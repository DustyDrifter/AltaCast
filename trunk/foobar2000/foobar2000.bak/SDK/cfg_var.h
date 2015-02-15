#ifndef _FOOBAR2000_SDK_CFG_VAR_H_
#define _FOOBAR2000_SDK_CFG_VAR_H_

//! Base class for configuration variable classes; provides self-registration mechaisms and methods to set/retrieve configuration data; those methods are automatically called for all registered instances by backend when configuration file is being read or written.\n
//! Note that cfg_var class and its derivatives may be only instantiated statically (as static objects or members of other static objects), NEVER dynamically (operator new, local variables, members of objects instantiated as such).
class NOVTABLE cfg_var {
protected:
	//! @param p_guid GUID of the variable, used to identify variable implementations owning specific configuration file entries when reading the configuration file back. You must generate a new GUID every time you declare a new cfg_var.
	cfg_var(const GUID & p_guid) : m_guid(p_guid) {PFC_ASSERT(!core_api::are_services_available());/*imperfect check for nonstatic instantiation*/next=list;list=this;};
	~cfg_var() {PFC_ASSERT(!core_api::are_services_available());/*imperfect check for nonstatic instantiation*/}
public:
	//! Retrieves state of the variable. Called only from main thread, when writing configuration file.
	//! @param p_stream Stream receiving state of the variable.
	virtual void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) = 0;
	//! Sets state of the variable. Called only from main thread, when reading configuration file.
	//! @param p_stream Stream containing new state of the variable.
	//! @param p_sizehint Number of bytes contained in the stream; reading past p_sizehint bytes will fail (EOF).
	virtual void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) = 0;

	//! For internal use only, do not call.
	inline const GUID & get_guid() const {return m_guid;}

	//! For internal use only, do not call.
	static void config_read_file(stream_reader * p_stream,abort_callback & p_abort);
	//! For internal use only, do not call.
	static void config_write_file(stream_writer * p_stream,abort_callback & p_abort);
private:
	GUID m_guid;
	static cfg_var * list;
	cfg_var * next;

	cfg_var(const cfg_var& ) {throw pfc::exception_not_implemented();}
	const cfg_var & operator=(const cfg_var& ) {throw pfc::exception_not_implemented();}
};

//! Generic integer config variable class. Template parameter can be used to specify integer type to use.\n
//! Note that cfg_var class and its derivatives may be only instantiated statically (as static objects or members of other static objects), NEVER dynamically (operator new, local variables, members of objects instantiated as such).
template<typename t_inttype>
class cfg_int_t : public cfg_var {
private:
	t_inttype m_val;
protected:
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) {p_stream->write_lendian_t(m_val,p_abort);}
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) {
		t_inttype temp;
		p_stream->read_lendian_t(temp,p_abort);//alter member data only on success, this will throw an exception when something isn't right
		m_val = temp;
	}

public:
	//! @param p_guid GUID of the variable, used to identify variable implementations owning specific configuration file entries when reading the configuration file back. You must generate a new GUID every time you declare a new cfg_var.
	//! @param p_default Default value of the variable.
	explicit inline cfg_int_t(const GUID & p_guid,t_inttype p_default) : cfg_var(p_guid), m_val(p_default) {}
	
	inline const cfg_int_t<t_inttype> & operator=(const cfg_int_t<t_inttype> & p_val) {m_val=p_val.m_val;return *this;}
	inline t_inttype operator=(t_inttype p_val) {m_val=p_val;return m_val;}

	inline operator t_inttype() const {return m_val;}

	inline t_inttype get_value() const {return m_val;}
};

typedef cfg_int_t<t_int32> cfg_int;
typedef cfg_int_t<t_uint32> cfg_uint;
//! Since relevant byteswapping functions also understand GUIDs, this can be abused to declare a cfg_guid.
typedef cfg_int_t<GUID> cfg_guid;
typedef cfg_int_t<bool> cfg_bool;

//! String config variable. Stored in the stream with int32 header containing size in bytes, followed by non-null-terminated UTF-8 data.\n
//! Note that cfg_var class and its derivatives may be only instantiated statically (as static objects or members of other static objects), NEVER dynamically (operator new, local variables, members of objects instantiated as such).
class cfg_string : public cfg_var, public pfc::string8
{
protected:
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort);
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort);

public:
	//! @param p_guid GUID of the variable, used to identify variable implementations owning specific configuration file entries when reading the configuration file back. You must generate a new GUID every time you declare a new cfg_var.
	//! @param p_defaultval Default/initial value of the variable.
	explicit inline cfg_string(const GUID & p_guid,const char * p_defaultval) : cfg_var(p_guid), pfc::string8(p_defaultval) {}

	inline const cfg_string& operator=(const cfg_string & p_val) {set_string(p_val);return *this;}
	inline const cfg_string& operator=(const char* p_val) {set_string(p_val);return *this;}

	inline operator const char * () const {return get_ptr();}

};

//! Struct config variable template. Warning: not endian safe, should be used only for nonportable code.\n
//! Note that cfg_var class and its derivatives may be only instantiated statically (as static objects or members of other static objects), NEVER dynamically (operator new, local variables, members of objects instantiated as such).
template<typename t_struct>
class cfg_struct_t : public cfg_var {
private:
	t_struct m_val;
protected:

	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) {p_stream->write_object(&m_val,sizeof(m_val),p_abort);}
	void set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) {
		t_struct temp;
		p_stream->read_object(&temp,sizeof(temp),p_abort);
		m_val = temp;
	}
public:
	//! @param p_guid GUID of the variable, used to identify variable implementations owning specific configuration file entries when reading the configuration file back. You must generate a new GUID every time you declare a new cfg_var.
	inline cfg_struct_t(const GUID & p_guid,const t_struct & p_val) : cfg_var(p_guid), m_val(p_val) {}
	//! @param p_guid GUID of the variable, used to identify variable implementations owning specific configuration file entries when reading the configuration file back. You must generate a new GUID every time you declare a new cfg_var.
	inline cfg_struct_t(const GUID & p_guid,int filler) : cfg_var(p_guid) {memset(&m_val,filler,sizeof(t_struct));}
	
	inline const cfg_struct_t<t_struct> & operator=(const cfg_struct_t<t_struct> & p_val) {m_val = p_val.get_value();return *this;}
	inline const cfg_struct_t<t_struct> & operator=(const t_struct & p_val) {m_val = p_val;return *this;}

	inline const t_struct& get_value() const {return m_val;}
	inline t_struct& get_value() {return m_val;}
	inline operator t_struct() const {return m_val;}
};


#endif
