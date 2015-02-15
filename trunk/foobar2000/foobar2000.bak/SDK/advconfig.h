#ifndef _FOOBAR2000_SDK_ADVCONFIG_H_
#define _FOOBAR2000_SDK_ADVCONFIG_H_

class advconfig_entry : public service_base {
public:
	virtual void get_name(pfc::string_base & p_out) = 0;
	virtual GUID get_guid() = 0;
	virtual GUID get_parent() = 0;
	virtual void reset() = 0;
	virtual double get_sort_priority() = 0;

	static const GUID guid_root;
	static const GUID guid_branch_tagging,guid_branch_decoding,guid_branch_tools,guid_branch_playback,guid_branch_display;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(advconfig_entry);
};

class advconfig_branch : public advconfig_entry {
public:
	FB2K_MAKE_SERVICE_INTERFACE(advconfig_branch,advconfig_entry);
};

class advconfig_entry_checkbox : public advconfig_entry {
public:
	virtual bool get_state() = 0;
	virtual void set_state(bool p_state) = 0;
	virtual bool is_radio() = 0;

	FB2K_MAKE_SERVICE_INTERFACE(advconfig_entry_checkbox,advconfig_entry);
};

class advconfig_entry_string : public advconfig_entry {
public:
	virtual void get_state(pfc::string_base & p_out) = 0;
	virtual void set_state(const char * p_string,t_size p_length = infinite) = 0;
	virtual t_uint32 get_flags() = 0;

	enum {
		flag_is_integer		= 1 << 0, 
		flag_is_signed		= 1 << 1,
	};

	FB2K_MAKE_SERVICE_INTERFACE(advconfig_entry_string,advconfig_entry);
};


class advconfig_branch_impl : public advconfig_branch {
public:
	advconfig_branch_impl(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority) : m_name(p_name), m_guid(p_guid), m_parent(p_parent), m_priority(p_priority) {}
	void get_name(pfc::string_base & p_out) {p_out = m_name;}
	GUID get_guid() {return m_guid;}
	GUID get_parent() {return m_parent;}
	void reset() {}
	double get_sort_priority() {return m_priority;}
private:
	pfc::string8 m_name;
	GUID m_guid,m_parent;
	const double m_priority;
};

template<bool p_is_radio = false>
class advconfig_entry_checkbox_impl : public advconfig_entry_checkbox {
public:
	advconfig_entry_checkbox_impl(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,bool p_initialstate)
		: m_name(p_name), m_initialstate(p_initialstate), m_state(p_guid,p_initialstate), m_parent(p_parent), m_priority(p_priority) {}
	
	void get_name(pfc::string_base & p_out) {p_out = m_name;}
	GUID get_guid() {return m_state.get_guid();}
	GUID get_parent() {return m_parent;}
	void reset() {m_state = m_initialstate;}
	bool get_state() {return m_state;}
	void set_state(bool p_state) {m_state = p_state;}
	bool is_radio() {return p_is_radio;}
	double get_sort_priority() {return m_priority;}
private:
	pfc::string8 m_name;
	const bool m_initialstate;
	cfg_bool m_state;
	GUID m_parent;
	const double m_priority;
};

class advconfig_branch_factory : public service_factory_single_t<advconfig_branch_impl> {
public:
	advconfig_branch_factory(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority)
		: service_factory_single_t<advconfig_branch_impl>(p_name,p_guid,p_parent,p_priority) {}
};

template<bool p_is_radio>
class advconfig_checkbox_factory_t : public service_factory_single_t<advconfig_entry_checkbox_impl<p_is_radio> > {
public:
	advconfig_checkbox_factory_t(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,bool p_initialstate) 
		: service_factory_single_t<advconfig_entry_checkbox_impl<p_is_radio> >(p_name,p_guid,p_parent,p_priority,p_initialstate) {}
};

typedef advconfig_checkbox_factory_t<false> advconfig_checkbox_factory;
typedef advconfig_checkbox_factory_t<true> advconfig_radio_factory;

class advconfig_entry_string_impl : public advconfig_entry_string {
public:
	advconfig_entry_string_impl(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,const char * p_initialstate)
		: m_name(p_name), m_parent(p_parent), m_priority(p_priority), m_initialstate(p_initialstate), m_state(p_guid,p_initialstate) {}
	void get_name(pfc::string_base & p_out) {p_out = m_name;}//{p_out = pfc::string_formatter() << m_name << " : " << m_state;}
	GUID get_guid() {return m_state.get_guid();}
	GUID get_parent() {return m_parent;}
	void reset() {core_api::ensure_main_thread();m_state = m_initialstate;}
	double get_sort_priority() {return m_priority;}
	void get_state(pfc::string_base & p_out) {core_api::ensure_main_thread();p_out = m_state;}
	void set_state(const char * p_string,t_size p_length = infinite) {core_api::ensure_main_thread();m_state.set_string(p_string,p_length);}
	t_uint32 get_flags() {return 0;}
private:
	const pfc::string8 m_initialstate, m_name;
	cfg_string m_state;
	const double m_priority;
	const GUID m_parent;
};

class advconfig_string_factory : public service_factory_single_t<advconfig_entry_string_impl> {
public:
	advconfig_string_factory(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,const char * p_initialstate) 
		: service_factory_single_t<advconfig_entry_string_impl>(p_name,p_guid,p_parent,p_priority,p_initialstate) {}
};


class advconfig_entry_integer_impl : public advconfig_entry_string {
public:
	advconfig_entry_integer_impl(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,t_uint64 p_initialstate,t_uint64 p_min,t_uint64 p_max)
		: m_name(p_name), m_parent(p_parent), m_priority(p_priority), m_initval(p_initialstate), m_min(p_min), m_max(p_max), m_state(p_guid,p_initialstate) {}
	void get_name(pfc::string_base & p_out) {p_out = m_name;}
	GUID get_guid() {return m_state.get_guid();}
	GUID get_parent() {return m_parent;}
	void reset() {m_state = m_initval;}
	double get_sort_priority() {return m_priority;}
	void get_state(pfc::string_base & p_out) {p_out = pfc::format_uint(m_state.get_value());}
	void set_state(const char * p_string,t_size p_length) {m_state = pfc::clip_t<t_uint64>(pfc::atoui64_ex(p_string,p_length),m_min,m_max);}
	t_uint32 get_flags() {return advconfig_entry_string::flag_is_integer;}

	t_uint64 get_state_int() const {return m_state;}
private:
	cfg_int_t<t_uint64> m_state;
	const double m_priority;
	const t_uint64 m_initval, m_min, m_max;
	const GUID m_parent;
	const pfc::string8 m_name;
};

class advconfig_integer_factory : public service_factory_single_t<advconfig_entry_integer_impl> {
public:
	advconfig_integer_factory(const char * p_name,const GUID & p_guid,const GUID & p_parent,double p_priority,t_uint64 p_initialstate,t_uint64 p_min,t_uint64 p_max) 
		: service_factory_single_t<advconfig_entry_integer_impl>(p_name,p_guid,p_parent,p_priority,p_initialstate,p_min,p_max) {}
};


class advconfig_entry_enum : public advconfig_entry {
public:
	virtual t_size get_value_count() = 0;
	virtual void enum_value(pfc::string_base & p_out,t_size p_index) = 0;
	virtual t_size get_state() = 0;
	virtual void set_state(t_size p_value) = 0;
	
	FB2K_MAKE_SERVICE_INTERFACE(advconfig_entry_enum,advconfig_entry);
};

#endif //_FOOBAR2000_SDK_ADVCONFIG_H_