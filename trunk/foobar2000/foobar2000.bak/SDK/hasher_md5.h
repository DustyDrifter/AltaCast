struct hasher_md5_state {
	char m_data[128];
};

struct hasher_md5_result {
	char m_data[16];
};

inline bool operator==(const hasher_md5_result & p_item1,const hasher_md5_result & p_item2) {return memcmp(&p_item1,&p_item2,sizeof(hasher_md5_result)) == 0;}
inline bool operator!=(const hasher_md5_result & p_item1,const hasher_md5_result & p_item2) {return memcmp(&p_item1,&p_item2,sizeof(hasher_md5_result)) != 0;}

namespace pfc {
	template<> class traits_t<hasher_md5_state> : public traits_rawobject {};
	template<> class traits_t<hasher_md5_result> : public traits_rawobject {};
}

class NOVTABLE hasher_md5 : public service_base
{
public:

	virtual void initialize(hasher_md5_state & p_state) = 0;
	virtual void process(hasher_md5_state & p_state,const void * p_buffer,t_size p_bytes) = 0;
	virtual hasher_md5_result get_result(const hasher_md5_state & p_state) = 0;

	
	static GUID guid_from_result(const hasher_md5_result & param);

	hasher_md5_result process_single(const void * p_buffer,t_size p_bytes);
	GUID process_single_guid(const void * p_buffer,t_size p_bytes);
	GUID get_result_guid(const hasher_md5_state & p_state) {return guid_from_result(get_result(p_state));}

	
	//! Helper
	void process_string(hasher_md5_state & p_state,const char * p_string,t_size p_length = infinite) {return process(p_state,p_string,pfc::strlen_max(p_string,p_length));}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(hasher_md5);
};