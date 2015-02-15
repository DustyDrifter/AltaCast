#ifndef _PFC_STRING_H_
#define _PFC_STRING_H_

namespace pfc {
	class NOVTABLE string_receiver {
	public:
		virtual void add_string(const char * p_string,t_size p_string_size = infinite) = 0;

		void add_char(t_uint32 c);//adds unicode char to the string
		void add_byte(char c) {add_string(&c,1);}
		void add_chars(t_uint32 p_char,t_size p_count) {for(;p_count;p_count--) add_char(p_char);}
	protected:
		string_receiver() {}
		~string_receiver() {}
	};

	t_size scan_filename(const char * ptr);

	bool is_path_separator(unsigned c);
	bool is_path_bad_char(unsigned c);
	bool is_valid_utf8(const char * param);
	bool is_lower_ascii(const char * param);
	bool has_path_bad_chars(const char * param);
	void recover_invalid_utf8(const char * src,char * out,unsigned replace);//out must be enough to hold strlen(char) + 1, or appropiately bigger if replace needs multiple chars
	void convert_to_lower_ascii(const char * src,t_size max,char * out,char replace = '?');//out should be at least strlen(src)+1 long

	inline char ascii_tolower(char c) {if (c >= 'A' && c <= 'Z') c += 'a' - 'A'; return c;}
	inline char ascii_toupper(char c) {if (c >= 'a' && c <= 'z') c += 'A' - 'a'; return c;}

	t_size string_find_first(const char * p_string,char p_tofind,t_size p_start = 0);	//returns infinite if not found
	t_size string_find_last(const char * p_string,char p_tofind,t_size p_start = ~0);	//returns infinite if not found
	t_size string_find_first(const char * p_string,const char * p_tofind,t_size p_start = 0);	//returns infinite if not found
	t_size string_find_last(const char * p_string,const char * p_tofind,t_size p_start = ~0);	//returns infinite if not found

	t_size string_find_first_ex(const char * p_string,t_size p_string_length,char p_tofind,t_size p_start = 0);	//returns infinite if not found
	t_size string_find_last_ex(const char * p_string,t_size p_string_length,char p_tofind,t_size p_start = ~0);	//returns infinite if not found
	t_size string_find_first_ex(const char * p_string,t_size p_string_length,const char * p_tofind,t_size p_tofind_length,t_size p_start = 0);	//returns infinite if not found
	t_size string_find_last_ex(const char * p_string,t_size p_string_length,const char * p_tofind,t_size p_tofind_length,t_size p_start = ~0);	//returns infinite if not found

	bool string_is_numeric(const char * p_string,t_size p_length = infinite);
	inline bool char_is_numeric(char p_char) {return p_char >= '0' && p_char <= '9';}
	inline bool char_is_ascii_alpha_upper(char p_char) {return p_char >= 'A' && p_char <= 'Z';}
	inline bool char_is_ascii_alpha_lower(char p_char) {return p_char >= 'a' && p_char <= 'z';}
	inline bool char_is_ascii_alpha(char p_char) {return char_is_ascii_alpha_lower(p_char) || char_is_ascii_alpha_upper(p_char);}
	inline bool char_is_ascii_alphanumeric(char p_char) {return char_is_ascii_alpha(p_char) || char_is_numeric(p_char);}
	

	class NOVTABLE string_base : public pfc::string_receiver {
	public:
		virtual const char * get_ptr() const = 0;
		virtual void add_string(const char * p_string,t_size p_length = ~0) = 0;//same as string_receiver method
		virtual void set_string(const char * p_string,t_size p_length = ~0) {reset();add_string(p_string,p_length);}
		virtual void truncate(t_size len)=0;
		virtual t_size get_length() const {return strlen(get_ptr());}
		virtual char * lock_buffer(t_size p_requested_length) = 0;
		virtual void unlock_buffer() = 0;

		//! For compatibility with old conventions.
		inline t_size length() const {return get_length();}
		
		inline void reset() {truncate(0);}
		
		inline bool is_empty() const {return *get_ptr()==0;}
		
		void skip_trailing_char(unsigned c = ' ');

		bool is_valid_utf8() {return pfc::is_valid_utf8(get_ptr());}

		void convert_to_lower_ascii(const char * src,char replace = '?');

		inline const string_base & operator= (const char * src) {set_string(src);return *this;}
		inline const string_base & operator+= (const char * src) {add_string(src);return *this;}
		inline const string_base & operator= (const string_base & src) {set_string(src);return *this;}
		inline const string_base & operator+= (const string_base & src) {add_string(src);return *this;}

		inline operator const char * () const {return get_ptr();}

		t_size scan_filename() const {return pfc::scan_filename(get_ptr());}

		t_size find_first(char p_char,t_size p_start = 0) {return pfc::string_find_first(get_ptr(),p_char,p_start);}
		t_size find_last(char p_char,t_size p_start = ~0) {return pfc::string_find_last(get_ptr(),p_char,p_start);}
		t_size find_first(const char * p_string,t_size p_start = 0) {return pfc::string_find_first(get_ptr(),p_string,p_start);}
		t_size find_last(const char * p_string,t_size p_start = ~0) {return pfc::string_find_last(get_ptr(),p_string,p_start);}

		void fix_dir_separator(char p_char);
	protected:
		string_base() {}
		~string_base() {}
	};
}

namespace pfc {
	template<template<typename> class t_alloc>
	class string8_t : public pfc::string_base {
	private:
		typedef string8_t<t_alloc> t_self;
	protected:
		pfc::array_t<char,t_alloc> m_data;
		t_size used;

		inline void makespace(t_size s)
		{
			t_size old_size = m_data.get_size();
			if (old_size < s)
				m_data.set_size(s+16);
			else if (old_size > s + 32)
				m_data.set_size(s);
		}

		inline const char * __get_ptr() const throw() {return used > 0 ? m_data.get_ptr() : "";}

	public:
		inline const t_self & operator= (const char * src) {set_string(src);return *this;}
		inline const t_self & operator+= (const char * src) {add_string(src);return *this;}
		inline const t_self & operator= (const string_base & src) {set_string(src);return *this;}
		inline const t_self & operator+= (const string_base & src) {add_string(src);return *this;}
		inline const t_self & operator= (const t_self & src) {set_string(src);return *this;}
		inline const t_self & operator+= (const t_self & src) {add_string(src);return *this;}

		inline operator const char * () const throw() {return __get_ptr();}

		string8_t() : used(0) {}
		string8_t(const char * p_string) : used(0) {set_string(p_string);}
		string8_t(const char * p_string,t_size p_length) : used(0) {set_string(p_string,p_length);}
		string8_t(const t_self & p_string) : used(0) {set_string(p_string);}
		string8_t(const string_base & p_string) : used(0) {set_string(p_string);}

		void prealloc(t_size p_size) {m_data.prealloc(p_size+1);}

		const char * get_ptr() const throw() {return __get_ptr();}

		void add_string(const char * p_string,t_size p_length = ~0);
		void set_string(const char * p_string,t_size p_length = ~0);

		void truncate(t_size len)
		{
			if (used>len) {used=len;m_data[len]=0;makespace(used+1);}
		}

		t_size get_length() const throw() {return used;}


		void set_char(unsigned offset,char c);

		t_size replace_nontext_chars(char p_replace = '_');
		t_size replace_char(unsigned c1,unsigned c2,t_size start = 0);
		t_size replace_byte(char c1,char c2,t_size start = 0);
		void fix_filename_chars(char def = '_',char leave=0);//replace "bad" characters, leave can be used to keep eg. path separators
		void remove_chars(t_size first,t_size count); //slow
		void insert_chars(t_size first,const char * src, t_size count);//slow
		void insert_chars(t_size first,const char * src);
		bool truncate_eol(t_size start = 0);
		bool fix_eol(const char * append = " (...)",t_size start = 0);
		bool limit_length(t_size length_in_chars,const char * append = " (...)");

		//for string_buffer class
		char * lock_buffer(t_size n)
		{
			makespace(n+1);
			pfc::memset_t(m_data,(char)0);
			return m_data.get_ptr();;
		}

		void unlock_buffer() {
			used=strlen(m_data.get_ptr());
			makespace(used+1);
		}

		void force_reset() {used=0;data.force_reset();}

		inline static void g_swap(t_self & p_item1,t_self & p_item2) {
			pfc::swap_t(p_item1.m_data,p_item2.m_data);
			pfc::swap_t(p_item1.used,p_item2.used);
		}
	};

	typedef string8_t<pfc::alloc_standard> string8;
	typedef string8_t<pfc::alloc_fast> string8_fast;
	typedef string8_t<pfc::alloc_fast_aggressive> string8_fast_aggressive;
	//for backwards compatibility
	typedef string8_t<pfc::alloc_fast_aggressive> string8_fastalloc;
}



#include "string8_impl.h"

namespace pfc {

	class string_buffer {
	private:
		string_base & m_owner;
		char * m_buffer;
	public:
		explicit string_buffer(string_base & p_string,t_size p_requeted_length) : m_owner(p_string) {m_buffer = m_owner.lock_buffer(p_requeted_length);}
		~string_buffer() {m_owner.unlock_buffer();}
		operator char* () {return m_buffer;}
	};

	class string_printf : public string8_fastalloc {
	public:
		static void g_run(string_base & out,const char * fmt,va_list list);
		inline void run(const char * fmt,va_list list) {g_run(*this,fmt,list);}

		explicit string_printf(const char * fmt,...);
	};

	class string_printf_va : public string8_fastalloc {
	public:
		explicit string_printf_va(const char * fmt,va_list list) {string_printf::g_run(*this,fmt,list);}
	};

	#pragma deprecated(string_printf, string_printf_va)

	class format_time
	{
	protected:
		char buffer[128];
	public:
		format_time(t_int64 s);
		operator const char * () const {return buffer;}
	};


	class format_time_ex : private format_time
	{
	public:
		format_time_ex(double s,unsigned extra = 3) : format_time((t_int64)s)
		{
			if (extra>0)
			{
				unsigned mul = 1, n;
				for(n=0;n<extra;n++) mul *= 10;

				
				unsigned val = (unsigned)((t_int64)(s*mul) % mul);
				char fmt[16];
				sprintf(fmt,".%%0%uu",extra);
				sprintf(buffer + strlen(buffer),fmt,val);
			}
		}
		const char * get_ptr() const {return buffer;}
		operator const char * () const {return buffer;}
	};


	unsigned atoui_ex(const char * ptr,t_size max);
	t_int64 atoi64_ex(const char * ptr,t_size max);
	t_uint64 atoui64_ex(const char * ptr,t_size max);

	t_size strlen_max(const char * ptr,t_size max);
	t_size wcslen_max(const wchar_t * ptr,t_size max);


	#ifdef UNICODE
	#define tcslen_max wcslen_max
	#else
	#define tcslen_max strlen_max
	#endif

	t_size strlen_utf8(const char * s,t_size num = ~0);//returns number of characters in utf8 string; num - no. of bytes (optional)
	t_size utf8_char_len(const char * s,t_size max = ~0);//returns size of utf8 character pointed by s, in bytes, 0 on error
	t_size utf8_char_len_from_header(char c);
	t_size utf8_chars_to_bytes(const char * string,t_size count);

	t_size strcpy_utf8_truncate(const char * src,char * out,t_size maxbytes);

	t_size utf8_decode_char(const char * src,unsigned * out,t_size src_bytes = ~0);//returns length in bytes
	t_size utf8_encode_char(unsigned c,char * out);//returns used length in bytes, max 6
	t_size utf16_decode_char(const wchar_t * p_source,unsigned * p_out,t_size p_source_length = ~0);
	t_size utf16_encode_char(unsigned c,WCHAR * out);


	t_size strstr_ex(const char * p_string,t_size p_string_len,const char * p_substring,t_size p_substring_len);

	int strcmp_partial(const char * p_string,const char * p_substring);
	int strcmp_partial_ex(const char * p_string,t_size p_string_length,const char * p_substring,t_size p_substring_length);

	t_size skip_utf8_chars(const char * ptr,t_size count);
	char * strdup_n(const char * src,t_size len);
	int stricmp_ascii(const char * s1,const char * s2);

	int strcmp_ex(const char* p1,t_size n1,const char* p2,t_size n2);

	unsigned utf8_get_char(const char * src);

	inline bool utf8_advance(const char * & var)
	{
		t_size delta = utf8_char_len(var);
		var += delta;
		return delta>0;
	}

	inline bool utf8_advance(char * & var)
	{
		t_size delta = utf8_char_len(var);
		var += delta;
		return delta>0;
	}

	inline const char * utf8_char_next(const char * src) {return src + utf8_char_len(src);}
	inline char * utf8_char_next(char * src) {return src + utf8_char_len(src);}



	class string_filename : public string8 {
	public:
		explicit string_filename(const char * fn);
	};

	class string_filename_ext : public string8 {
	public:
		explicit string_filename_ext(const char * fn);
	};

	class string_extension
	{
		char buffer[32];
	public:
		inline const char * get_ptr() const {return buffer;}
		inline t_size length() const {return strlen(buffer);}
		inline operator const char * () const {return buffer;}
		explicit string_extension(const char * src);
	};


	class string_replace_extension
	{
	public:
		string_replace_extension(const char * p_path,const char * p_ext);
		inline operator const char*() const {return m_data;}
	private:
		string8 m_data;
	};

	class string_directory
	{
	public:
		string_directory(const char * p_path);
		inline operator const char*() const {return m_data;}
	private:
		string8 m_data;
	};

	void float_to_string(char * out,t_size out_max,double val,unsigned precision,bool force_sign = false);//doesnt add E+X etc, has internal range limits, useful for storing float numbers as strings without having to bother with international coma/dot settings BS
	double string_to_float(const char * src,t_size len = infinite);

	template<>
	inline void swap_t(string8 & p_item1,string8 & p_item2)
	{
		string8::g_swap(p_item1,p_item2);
	}

	class format_float
	{
	public:
		format_float(double p_val,unsigned p_width,unsigned p_prec);
		format_float(const format_float & p_source) {*this = p_source;}

		inline const char * get_ptr() const {return m_buffer.get_ptr();}
		inline operator const char*() const {return m_buffer.get_ptr();}
	private:
		string8 m_buffer;
	};

	class format_int
	{
	public:
		format_int(t_int64 p_val,unsigned p_width = 0,unsigned p_base = 10);
		format_int(const format_int & p_source) {*this = p_source;}
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		char m_buffer[64];
	};


	class format_uint
	{
	public:
		format_uint(t_uint64 p_val,unsigned p_width = 0,unsigned p_base = 10);
		format_uint(const format_uint & p_source) {*this = p_source;}
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		char m_buffer[64];
	};

	class format_hex
	{
	public:
		format_hex(t_uint64 p_val,unsigned p_width = 0);
		format_hex(const format_hex & p_source) {*this = p_source;}
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		char m_buffer[17];
	};

	class format_hex_lowercase
	{
	public:
		format_hex_lowercase(t_uint64 p_val,unsigned p_width = 0);
		format_hex_lowercase(const format_hex_lowercase & p_source) {*this = p_source;}
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		char m_buffer[17];
	};


	typedef string8_fastalloc string_formatter;

	class format_hexdump
	{
	public:
		format_hexdump(const void * p_buffer,t_size p_bytes,const char * p_spacing = " ");

		inline const char * get_ptr() const {return m_formatter;}
		inline operator const char * () const {return m_formatter;}

	private:
		string_formatter m_formatter;
	};

	class format_hexdump_lowercase
	{
	public:
		format_hexdump_lowercase(const void * p_buffer,t_size p_bytes,const char * p_spacing = " ");

		inline const char * get_ptr() const {return m_formatter;}
		inline operator const char * () const {return m_formatter;}

	private:
		string_formatter m_formatter;
	};

	class format_fixedpoint
	{
	public:
		format_fixedpoint(t_int64 p_val,unsigned p_point);
		format_fixedpoint(const format_fixedpoint & p_source) {*this = p_source;}
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		string_formatter m_buffer;
	};

	class format_char {
	public:
		format_char(char p_char) {m_buffer[0] = p_char; m_buffer[1] = 0;}
		format_char(const format_char & p_source) { *this = p_source; }
		inline const char * get_ptr() const {return m_buffer;}
		inline operator const char*() const {return m_buffer;}
	private:
		char m_buffer[2];
	};
}

inline pfc::string_base & operator<<(pfc::string_base & p_fmt,const char * p_source) {p_fmt.add_string(p_source); return p_fmt;}
inline pfc::string_base & operator<<(pfc::string_base & p_fmt,t_int32 p_val) {return p_fmt << pfc::format_int(p_val);}
inline pfc::string_base & operator<<(pfc::string_base & p_fmt,t_uint32 p_val) {return p_fmt << pfc::format_uint(p_val);}
inline pfc::string_base & operator<<(pfc::string_base & p_fmt,t_int64 p_val) {return p_fmt << pfc::format_int(p_val);}
inline pfc::string_base & operator<<(pfc::string_base & p_fmt,t_uint64 p_val) {return p_fmt << pfc::format_uint(p_val);}
inline pfc::string_base & operator<<(pfc::string_base & p_fmt,double p_val) {return p_fmt << pfc::format_float(p_val,0,7);}

inline pfc::string_base & operator<<(pfc::string_base & p_fmt,std::exception const & p_exception) {return p_fmt << p_exception.what();}






namespace pfc {
	template<typename t_char>
	class string_simple_t {
	private:
		typedef string_simple_t<t_char> t_self;
	public:
		t_size length(t_size p_limit = infinite) const {return pfc::strlen_t(get_ptr(),infinite);}
		bool is_empty() const {return length(1) == 0;}
		void set_string(const t_char * p_source,t_size p_length = infinite) {
			t_size length = pfc::strlen_t(p_source,p_length);
			m_buffer.set_size(length + 1);
			pfc::memcpy_t(m_buffer.get_ptr(),p_source,length);
			m_buffer[length] = 0;
		}
		string_simple_t() {}
		string_simple_t(const t_char * p_source,t_size p_length = infinite) {set_string(p_source,p_length);}
		const t_self & operator=(const t_char * p_source) {set_string(p_source);return *this;}
		operator const t_char* () const {return get_ptr();}
		const t_char * get_ptr() const {return m_buffer.get_size() > 0 ? m_buffer.get_ptr() : pfc::empty_string_t<t_char>();}
	private:
		pfc::array_t<t_char> m_buffer;
	};

	typedef string_simple_t<char> string_simple;

}


#endif //_PFC_STRING_H_