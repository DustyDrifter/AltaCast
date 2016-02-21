#ifndef _PFC_STRING_FIXED_H_
#define _PFC_STRING_FIXED_H_

namespace pfc {

template<t_size t_length>
class string_fixed_t : public pfc::string_base
{
public:
	inline string_fixed_t() {init();}
	inline string_fixed_t(const string_fixed_t<t_length> & p_source) {init(); *this = p_source;}
	inline string_fixed_t(const char * p_source) {init(); set_string(p_source);}
	
	inline const string_fixed_t<t_length> & operator=(const string_fixed_t<t_length> & p_source) {set_string(p_source);return *this;}
	inline const string_fixed_t<t_length> & operator=(const char * p_source) {set_string(p_source);return *this;}

	char * lock_buffer(t_size p_requested_length)
	{
		if (p_requested_length >= t_length) return NULL;
		memset(m_data,0,sizeof(m_data));
		return m_data;
	}
	void unlock_buffer()
	{
		m_length = strlen(m_data);
	}

	inline operator const char * () const {return m_data;}
	
	const char * get_ptr() const {return m_data;}

	void add_string(const char * ptr,t_size len) {
		len = strlen_max(ptr,len);
		if (m_length + len > max_length) throw pfc::exception_overflow();
		for(t_size n=0;n<len;n++) {
			m_data[m_length++] = ptr[n];
		}
		m_data[m_length] = 0;
	}
	void truncate(t_size len)
	{
		if (len > max_length) len = max_length;
		if (m_length > len)
		{
			m_length = len;
			m_data[len] = 0;
		}
	}
	t_size get_length() const {return m_length;}

private:
	inline void init() {m_length = 0; m_data[0] = 0;}
	enum {max_length = t_length - 1};
	t_size m_length;
	char m_data[t_length];
};

}
#endif