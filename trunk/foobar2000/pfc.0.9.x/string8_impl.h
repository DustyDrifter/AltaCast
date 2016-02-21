namespace pfc {

template<template<typename> class t_alloc>
void string8_t<t_alloc>::add_string(const char * ptr,t_size len)
{
	if (len > 0 && ptr >= m_data.get_ptr() && ptr <= m_data.get_ptr() + m_data.get_size()) {
		add_string(string8(ptr,len));
	} else {
		len = strlen_max(ptr,len);
		makespace(used+len+1);
		pfc::memcpy_t(m_data.get_ptr() + used,ptr,len);
		used+=len;
		m_data[used]=0;
		true;
	}
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::set_string(const char * ptr,t_size len) {
	if (len > 0 && ptr >= m_data.get_ptr() && ptr < m_data.get_ptr() + m_data.get_size()) {
		set_string(string8(ptr,len));
	} else {
		len = strlen_max(ptr,len);
		makespace(len+1);
		pfc::memcpy_t(m_data.get_ptr(),ptr,len);
		used=len;
		m_data[used]=0;
	}
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::set_char(unsigned offset,char c)
{
	if (!c) truncate(offset);
	else if (offset<used) m_data[offset]=c;
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::fix_filename_chars(char def,char leave)//replace "bad" characters, leave can be used to keep eg. path separators
{
	t_size n;
	for(n=0;n<used;n++)
		if (m_data[n]!=leave && pfc::is_path_bad_char(m_data[n])) m_data[n]=def;
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::remove_chars(t_size first,t_size count)
{
	if (first>used) first = used;
	if (first+count>used) count = used-first;
	if (count>0)
	{
		t_size n;
		for(n=first+count;n<=used;n++)
			m_data[n-count]=m_data[n];
		used -= count;
		makespace(used+1);
	}
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::insert_chars(t_size first,const char * src, t_size count)
{
	if (first > used) first = used;

	makespace(used+count+1);
	t_size n;
	for(n=used;(int)n>=(int)first;n--)
		m_data[n+count] = m_data[n];
	for(n=0;n<count;n++)
		m_data[first+n] = src[n];

	used+=count;
}

template<template<typename> class t_alloc>
void string8_t<t_alloc>::insert_chars(t_size first,const char * src) {insert_chars(first,src,strlen(src));}

template<template<typename> class t_alloc>
bool string8_t<t_alloc>::truncate_eol(t_size start)
{
	t_size n;
	const char * ptr = m_data.get_ptr() + start;
	for(n=start;n<used;n++)
	{
		if (*ptr==10 || *ptr==13)
		{
			truncate(n);
			return true;
		}
		ptr++;
	}
	return false;
}

template<template<typename> class t_alloc>
bool string8_t<t_alloc>::fix_eol(const char * append,t_size start)
{
	bool rv = truncate_eol(start);
	if (rv) add_string(append);
	return rv;
}


template<template<typename> class t_alloc>
bool string8_t<t_alloc>::limit_length(t_size length_in_chars,const char * append)
{
	bool rv = false;
	const char * base = get_ptr(), * ptr = base;
	while(length_in_chars && utf8_advance(ptr)) length_in_chars--;
	if (length_in_chars==0)
	{
		truncate(ptr-base);
		add_string(append);
		rv = true;
	}
	return rv;
}

template<template<typename> class t_alloc>
t_size string8_t<t_alloc>::replace_nontext_chars(char p_replace)
{
	t_size ret = 0;
	for(t_size n=0;n<used;n++)
	{
		if ((unsigned char)m_data[n] < 32) {m_data[n] = p_replace; ret++; }
	}
	return ret;
}

template<template<typename> class t_alloc>
t_size string8_t<t_alloc>::replace_byte(char c1,char c2,t_size start)
{
	PFC_ASSERT(c1 != 0); PFC_ASSERT(c2 != 0);
	t_size n, ret = 0;
	for(n=start;n<used;n++)
	{
		if (m_data[n] == c1) {m_data[n] = c2; ret++;}
	}
	return ret;
}

template<template<typename> class t_alloc>
t_size string8_t<t_alloc>::replace_char(unsigned c1,unsigned c2,t_size start)
{
	if (c1 < 128 && c2 < 128) return replace_byte((char)c1,(char)c2,start);

	string8 temp(get_ptr()+start);
	truncate(start);
	const char * ptr = temp;
	t_size rv = 0;
	while(*ptr)
	{
		unsigned test;
		t_size delta = utf8_decode_char(ptr,&test);
		if (delta==0 || test==0) break;
		if (test == c1) {test = c2;rv++;}
		add_char(test);
		ptr += delta;
	}
	return rv;
}

}