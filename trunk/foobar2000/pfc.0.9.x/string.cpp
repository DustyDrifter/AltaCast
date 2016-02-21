#include "pfc.h"

namespace pfc {

void string_receiver::add_char(t_uint32 p_char)
{
	char temp[8];
	t_size len = utf8_encode_char(p_char,temp);
	if (len>0) add_string(temp,len);
}

void string_base::skip_trailing_char(unsigned skip)
{
	const char * str = get_ptr();
	t_size ptr,trunc;
	bool need_trunc = false;
	for(ptr=0;str[ptr];)
	{
		unsigned c;
		t_size delta = utf8_decode_char(str+ptr,&c);
		if (delta==0) break;
		if (c==skip)
		{
			need_trunc = true;
			trunc = ptr;
		}
		else
		{
			need_trunc = false;
		}
		ptr += delta;
	}
	if (need_trunc) truncate(trunc);
}

format_time::format_time(t_int64 length)
{
	if (length<0) length=0;
	char * out = buffer;
	int weeks,days,hours,minutes,seconds;
	
	weeks = (int)( ( length / (60*60*24*7) ) );
	days = (int)( ( length / (60*60*24) ) % 7 );
	hours = (int) ( ( length / (60 * 60) ) % 24);
	minutes = (int) ( ( length / (60 ) ) % 60 );
	seconds = (int) ( ( length ) % 60 );

	if (weeks)
	{
		out += sprintf(out,"%uwk ",weeks);
	}
	if (days || weeks)
	{
		out += sprintf(out,"%ud ",days);
	}
	if (hours || days || weeks)
	{
		out += sprintf(out,"%u:%02u:%02u",hours,minutes,seconds);
	}
	else out += sprintf(out,"%u:%02u",minutes,seconds);
}

int strcmp_partial(const char * p_string,const char * p_substring) {return strcmp_partial_ex(p_string,infinite,p_substring,infinite);}

int strcmp_partial_ex(const char * p_string,t_size p_string_length,const char * p_substring,t_size p_substring_length) {
	p_string_length = strlen_max(p_string,p_string_length); p_substring_length = strlen_max(p_substring,p_substring_length);
	for(t_size walk=0;walk<p_substring_length;walk++) {
		char stringchar = (walk>=p_string_length ? 0 : p_string[walk]);
		char substringchar = p_substring[walk];
		int result = compare_t(stringchar,substringchar);
		if (result != 0) return result;
	}
	return 0;
}

bool is_path_separator(unsigned c)
{
	return c=='\\' || c=='/' || c=='|' || c==':';
}

bool is_path_bad_char(unsigned c)
{
#ifdef _WINDOWS
	return c=='\\' || c=='/' || c=='|' || c==':' || c=='*' || c=='?' || c=='\"' || c=='>' || c=='<';
#else
#error portme
#endif
}


void string_printf::g_run(string_base & out,const char * fmt,va_list list)
{
	out.reset();
	while(*fmt)
	{
		if (*fmt=='%')
		{
			fmt++;
			if (*fmt=='%')
			{
				out.add_char('%');
				fmt++;
			}
			else
			{
				bool force_sign = false;
				if (*fmt=='+')
				{
					force_sign = true;
					fmt++;
				}
				char padchar = (*fmt == '0') ? '0' : ' ';
				t_size pad = 0;
				while(*fmt>='0' && *fmt<='9')
				{
					pad = pad * 10 + (*fmt - '0');
					fmt++;
				}

				if (*fmt=='s' || *fmt=='S')
				{
					const char * ptr = va_arg(list,const char*);
					t_size len = strlen(ptr);
					if (pad>len) out.add_chars(padchar,pad-len);
					out.add_string(ptr);
					fmt++;

				}
				else if (*fmt=='i' || *fmt=='I' || *fmt=='d' || *fmt=='D')
				{
					char temp[8*sizeof(int)];
					int val = va_arg(list,int);
					if (force_sign && val>0) out.add_char('+');
					itoa(val,temp,10);
					t_size len = strlen(temp);
					if (pad>len) out.add_chars(padchar,pad-len);
					out.add_string(temp);
					fmt++;
				}
				else if (*fmt=='u' || *fmt=='U')
				{
					char temp[8*sizeof(int)];
					int val = va_arg(list,int);
					if (force_sign && val>0) out.add_char('+');
					_ultoa(val,temp,10);
					t_size len = strlen(temp);
					if (pad>len) out.add_chars(padchar,pad-len);
					out.add_string(temp);
					fmt++;
				}
				else if (*fmt=='x' || *fmt=='X')
				{
					char temp[8*sizeof(int)];
					int val = va_arg(list,int);
					if (force_sign && val>0) out.add_char('+');
					_ultoa(val,temp,16);
					if (*fmt=='X')
					{
						char * t = temp;
						while(*t)
						{
							if (*t>='a' && *t<='z')
								*t += 'A' - 'a';
							t++;
						}
					}
					t_size len = strlen(temp);
					if (pad>len) out.add_chars(padchar,pad-len);
					out.add_string(temp);
					fmt++;
				}
				else if (*fmt=='c' || *fmt=='C')
				{
					out.add_char(va_arg(list,char));
					fmt++;
				}
			}
		}
		else
		{
			out.add_char(*(fmt++));
		}
	}
}

string_printf::string_printf(const char * fmt,...)
{
	va_list list;
	va_start(list,fmt);
	run(fmt,list);
	va_end(list);
}



t_size strlen_max(const char * ptr,t_size max)
{
	if (ptr==0) return 0;
	t_size n = 0;
	while(n<max && ptr[n]) n++;
	return n;
}

t_size wcslen_max(const WCHAR * ptr,t_size max)
{
	if (ptr==0) return 0;
	t_size n = 0;
	while(n<max && ptr[n]) n++;
	return n;
}

char * strdup_n(const char * src,t_size len)
{
	len = strlen_max(src,len);
	char * ret = (char*)malloc(len+1);
	if (ret)
	{
		memcpy(ret,src,len);
		ret[len]=0;
	}
	return ret;
}

string_filename::string_filename(const char * fn)
{
	fn += pfc::scan_filename(fn);
	const char * ptr=fn,*dot=0;
	while(*ptr && *ptr!='?')
	{
		if (*ptr=='.') dot=ptr;
		ptr++;
	}

	if (dot && dot>fn) set_string(fn,dot-fn);
	else set_string(fn);
}

string_filename_ext::string_filename_ext(const char * fn)
{
	fn += pfc::scan_filename(fn);
	const char * ptr = fn;
	while(*ptr && *ptr!='?') ptr++;
	set_string(fn,ptr-fn);
}

string_extension::string_extension(const char * src)
{
	buffer[0]=0;
	const char * start = src + pfc::scan_filename(src);
	const char * end = start + strlen(start);
	const char * ptr = end-1;
	while(ptr>start && *ptr!='.')
	{
		if (*ptr=='?') end=ptr;
		ptr--;
	}

	if (ptr>=start && *ptr=='.')
	{
		ptr++;
		t_size len = end-ptr;
		if (len<tabsize(buffer))
		{
			memcpy(buffer,ptr,len*sizeof(char));
			buffer[len]=0;
		}
	}
}


bool has_path_bad_chars(const char * param)
{
	while(*param)
	{
		if (is_path_bad_char(*param)) return true;
		param++;
	}
	return false;
}

void float_to_string(char * out,t_size out_max,double val,unsigned precision,bool b_sign)
{
	char temp[64];
	t_size outptr;
	t_size temp_len;

	if (out_max == 0) return;
	out_max--;//for null terminator
	
	outptr = 0;	

	if (outptr == out_max) {out[outptr]=0;return;}

	if (val<0) {out[outptr++] = '-'; val = -val;}
	else if (b_sign) {out[outptr++] = '+';}

	if (outptr == out_max) {out[outptr]=0;return;}

	
	{
		double powval = pow((double)10.0,(double)precision);
		t_int64 blargh = (t_int64)floor(val * powval + 0.5);
		_i64toa(blargh,temp,10);
	}
	
	temp_len = strlen(temp);
	if (temp_len <= precision)
	{
		out[outptr++] = '0';
		if (outptr == out_max) {out[outptr]=0;return;}
		out[outptr++] = '.';
		if (outptr == out_max) {out[outptr]=0;return;}
		t_size d;
		for(d=precision-temp_len;d;d--)
		{
			out[outptr++] = '0';
			if (outptr == out_max) {out[outptr]=0;return;}
		}
		for(d=0;d<temp_len;d++)
		{
			out[outptr++] = temp[d];
			if (outptr == out_max) {out[outptr]=0;return;}
		}
	}
	else
	{
		t_size d = temp_len;
		const char * src = temp;
		while(*src)
		{
			if (d-- == precision)
			{
				out[outptr++] = '.';
				if (outptr == out_max) {out[outptr]=0;return;}
			}
			out[outptr++] = *(src++);
			if (outptr == out_max) {out[outptr]=0;return;}
		}
	}
	out[outptr] = 0;
}

static double pfc_string_to_float_internal(const char * src)
{
	bool neg = false;
	t_int64 val = 0;
	int div = 0;
	bool got_dot = false;

	while(*src==' ') src++;

	if (*src=='-') {neg = true;src++;}
	else if (*src=='+') src++;
	
	while(*src)
	{
		if (*src>='0' && *src<='9')
		{
			int d = *src - '0';
			val = val * 10 + d;
			if (got_dot) div--;
			src++;
		}
		else if (*src=='.' || *src==',')
		{
			if (got_dot) break;
			got_dot = true;
			src++;
		}
		else if (*src=='E' || *src=='e')
		{
			src++;
			div += atoi(src);
			break;
		}
		else break;
	}
	if (neg) val = -val;
	return (double) val * pow(10.0,(double)div);
}

double string_to_float(const char * src,t_size max)
{
	char blargh[128];
	strncpy(blargh,src,max>127?127:max);
	blargh[127]=0;
	return pfc_string_to_float_internal(blargh);
}



void string_base::convert_to_lower_ascii(const char * src,char replace)
{
	reset();
	PFC_ASSERT(replace>0);
	while(*src)
	{
		unsigned c;
		t_size delta = utf8_decode_char(src,&c);
		if (delta==0) {c = replace; delta = 1;}
		else if (c>=0x80) c = replace;
		add_byte((char)c);
		src += delta;
	}
}

void convert_to_lower_ascii(const char * src,t_size max,char * out,char replace)
{
	t_size ptr = 0;
	PFC_ASSERT(replace>0);
	while(ptr<max && src[ptr])
	{
		unsigned c;
		t_size delta = utf8_decode_char(src+ptr,&c,max-ptr);
		if (delta==0) {c = replace; delta = 1;}
		else if (c>=0x80) c = replace;
		*(out++) = (char)c;
		ptr += delta;
	}
	*out = 0;
}

t_size strstr_ex(const char * p_string,t_size p_string_len,const char * p_substring,t_size p_substring_len)
{
	p_string_len = strlen_max(p_string,p_string_len);
	p_substring_len = strlen_max(p_substring,p_substring_len);
	t_size index = 0;
	while(index + p_substring_len <= p_string_len)
	{
		if (memcmp(p_string+index,p_substring,p_substring_len) == 0) return index;
		t_size delta = utf8_char_len(p_string+index,p_string_len - index);
		if (delta == 0) break;
		index += delta;
	}
	return ~0;
}

unsigned atoui_ex(const char * p_string,t_size p_string_len)
{
	unsigned ret = 0; t_size ptr = 0;
	while(ptr<p_string_len)
	{
		char c = p_string[ptr];
		if (! ( c >= '0' && c <= '9' ) ) break;
		ret = ret * 10 + (unsigned)( c - '0' );
		ptr++;
	}
	return ret;
}

int strcmp_ex(const char* p1,t_size n1,const char* p2,t_size n2)
{
	t_size idx = 0;
	n1 = strlen_max(p1,n1); n2 = strlen_max(p2,n2);
	for(;;)
	{
		if (idx == n1 && idx == n2) return 0;
		else if (idx == n1) return -1;//end of param1
		else if (idx == n2) return 1;//end of param2

		char c1 = p1[idx], c2 = p2[idx];
		if (c1<c2) return -1;
		else if (c1>c2) return 1;
		
		idx++;
	}
}

t_uint64 atoui64_ex(const char * src,t_size len) {
	len = strlen_max(src,infinite);
	t_uint64 ret = 0, mul = 1;
	t_size ptr = len;
	t_size start = 0;
//	start += skip_spacing(src+start,len-start);
	
	while(ptr>start)
	{
		char c = src[--ptr];
		if (c>='0' && c<='9')
		{
			ret += (c-'0') * mul;
			mul *= 10;
		}
		else
		{
			ret = 0;
			mul = 1;
		}
	}
	return ret;
}


t_int64 atoi64_ex(const char * src,t_size len)
{
	len = strlen_max(src,infinite);
	t_int64 ret = 0, mul = 1;
	t_size ptr = len;
	t_size start = 0;
	bool neg = false;
//	start += skip_spacing(src+start,len-start);
	if (start < len && src[start] == '-') {neg = true; start++;}
//	start += skip_spacing(src+start,len-start);
	
	while(ptr>start)
	{
		char c = src[--ptr];
		if (c>='0' && c<='9')
		{
			ret += (c-'0') * mul;
			mul *= 10;
		}
		else
		{
			ret = 0;
			mul = 1;
		}
	}
	return neg ? -ret : ret;
}

int stricmp_ascii(const char * s1,const char * s2)
{
	for(;;)
	{
		char c1 = ascii_tolower(*s1), c2 = ascii_tolower(*s2);
		if (c1<c2) return -1;
		else if (c1>c2) return 1;
		else if (c1 == 0) return 0;
		s1++;
		s2++;
	}
}

format_float::format_float(double p_val,unsigned p_width,unsigned p_prec)
{
	char temp[64];
	float_to_string(temp,64,p_val,p_prec,false);
	temp[63] = 0;
	t_size len = strlen(temp);
	if (len < p_width)
		m_buffer.add_chars(' ',p_width-len);
	m_buffer += temp;
}

static char format_hex_char(unsigned p_val)
{
	PFC_ASSERT(p_val < 16);
	return (p_val < 10) ? p_val + '0' : p_val - 10 + 'A';
}

format_hex::format_hex(t_uint64 p_val,unsigned p_width)
{
	if (p_width > 16) p_width = 16;
	else if (p_width == 0) p_width = 1;
	char temp[16];
	unsigned n;
	for(n=0;n<16;n++)
	{
		temp[15-n] = format_hex_char((unsigned)(p_val & 0xF));
		p_val >>= 4;
	}

	for(n=0;n<16 && temp[n] == '0';n++) {}
	
	if (n > 16 - p_width) n = 16 - p_width;
	
	char * out = m_buffer;
	for(;n<16;n++)
		*(out++) = temp[n];
	*out = 0;
}

static char format_hex_char_lowercase(unsigned p_val)
{
	PFC_ASSERT(p_val < 16);
	return (p_val < 10) ? p_val + '0' : p_val - 10 + 'a';
}

format_hex_lowercase::format_hex_lowercase(t_uint64 p_val,unsigned p_width)
{
	if (p_width > 16) p_width = 16;
	else if (p_width == 0) p_width = 1;
	char temp[16];
	unsigned n;
	for(n=0;n<16;n++)
	{
		temp[15-n] = format_hex_char_lowercase((unsigned)(p_val & 0xF));
		p_val >>= 4;
	}

	for(n=0;n<16 && temp[n] == '0';n++) {}
	
	if (n > 16 - p_width) n = 16 - p_width;
	
	char * out = m_buffer;
	for(;n<16;n++)
		*(out++) = temp[n];
	*out = 0;
}

format_uint::format_uint(t_uint64 val,unsigned p_width,unsigned p_base)
{
	
	enum {max_width = tabsize(m_buffer) - 1};

	if (p_width > max_width) p_width = max_width;
	else if (p_width == 0) p_width = 1;

	char temp[max_width];
	
	unsigned n;
	for(n=0;n<max_width;n++)
	{
		temp[max_width-1-n] = format_hex_char((unsigned)(val % p_base));
		val /= p_base;
	}

	for(n=0;n<max_width && temp[n] == '0';n++) {}
	
	if (n > max_width - p_width) n = max_width - p_width;
	
	char * out = m_buffer;

	for(;n<max_width;n++)
		*(out++) = temp[n];
	*out = 0;
}

format_fixedpoint::format_fixedpoint(t_int64 p_val,unsigned p_point)
{
	unsigned div = 1;
	for(unsigned n=0;n<p_point;n++) div *= 10;

	if (p_val < 0) {m_buffer << "-";p_val = -p_val;}

	
	m_buffer << format_int(p_val / div) << "." << format_int(p_val % div, p_point);
}

format_int::format_int(t_int64 p_val,unsigned p_width,unsigned p_base)
{
	bool neg = false;
	t_uint64 val;
	if (p_val < 0) {neg = true; val = (t_uint64)(-p_val);}
	else val = (t_uint64)p_val;
	
	enum {max_width = tabsize(m_buffer) - 1};

	if (p_width > max_width) p_width = max_width;
	else if (p_width == 0) p_width = 1;

	if (neg && p_width > 1) p_width --;
	
	char temp[max_width];
	
	unsigned n;
	for(n=0;n<max_width;n++)
	{
		temp[max_width-1-n] = format_hex_char((unsigned)(val % p_base));
		val /= p_base;
	}

	for(n=0;n<max_width && temp[n] == '0';n++) {}
	
	if (n > max_width - p_width) n = max_width - p_width;
	
	char * out = m_buffer;

	if (neg) *(out++) = '-';

	for(;n<max_width;n++)
		*(out++) = temp[n];
	*out = 0;
}

format_hexdump_lowercase::format_hexdump_lowercase(const void * p_buffer,t_size p_bytes,const char * p_spacing)
{
	t_size n;
	const t_uint8 * buffer = (const t_uint8*)p_buffer;
	for(n=0;n<p_bytes;n++)
	{
		if (n > 0 && p_spacing != 0) m_formatter << p_spacing;
		m_formatter << format_hex_lowercase(buffer[n],2);
	}
}

format_hexdump::format_hexdump(const void * p_buffer,t_size p_bytes,const char * p_spacing)
{
	t_size n;
	const t_uint8 * buffer = (const t_uint8*)p_buffer;
	for(n=0;n<p_bytes;n++)
	{
		if (n > 0 && p_spacing != 0) m_formatter << p_spacing;
		m_formatter << format_hex(buffer[n],2);
	}
}



string_replace_extension::string_replace_extension(const char * p_path,const char * p_ext)
{
	m_data = p_path;
	t_size dot = m_data.find_last('.');
	if (dot < m_data.scan_filename())
	{//argh
		m_data += ".";
		m_data += p_ext;
	}
	else
	{
		m_data.truncate(dot+1);
		m_data += p_ext;
	}
}

string_directory::string_directory(const char * p_path)
{
	t_size ptr = scan_filename(p_path);
	if (ptr > 0) m_data.set_string(p_path,ptr-1);
}

t_size scan_filename(const char * ptr)
{
	t_size n;
	t_size _used = strlen(ptr);
	for(n=_used-1;n!=~0;n--)
	{
		if (is_path_separator(ptr[n])) return n+1;
	}
	return 0;
}



t_size string_find_first(const char * p_string,char p_tofind,t_size p_start) {
	return string_find_first_ex(p_string,infinite,&p_tofind,1,p_start);
}
t_size string_find_last(const char * p_string,char p_tofind,t_size p_start) {
	return string_find_last_ex(p_string,infinite,&p_tofind,1,p_start);
}
t_size string_find_first(const char * p_string,const char * p_tofind,t_size p_start) {
	return string_find_first_ex(p_string,infinite,p_tofind,infinite,p_start);
}
t_size string_find_last(const char * p_string,const char * p_tofind,t_size p_start) {
	return string_find_last_ex(p_string,infinite,p_tofind,infinite,p_start);
}

t_size string_find_first_ex(const char * p_string,t_size p_string_length,char p_tofind,t_size p_start) {
	return string_find_first_ex(p_string,p_string_length,&p_tofind,1,p_start);
}
t_size string_find_last_ex(const char * p_string,t_size p_string_length,char p_tofind,t_size p_start) {
	return string_find_last_ex(p_string,p_string_length,&p_tofind,1,p_start);
}
t_size string_find_first_ex(const char * p_string,t_size p_string_length,const char * p_tofind,t_size p_tofind_length,t_size p_start) {
	p_string_length = strlen_max(p_string,p_string_length); p_tofind_length = strlen_max(p_tofind,p_tofind_length);
	if (p_string_length >= p_tofind_length) {
		t_size max = p_string_length - p_tofind_length;
		for(t_size walk = p_start; walk <= max; walk++) {
			if (strcmp_partial_ex(p_string+walk,p_string_length-walk,p_tofind,p_tofind_length) == 0) return walk;
		}
	}
	return infinite;
}
t_size string_find_last_ex(const char * p_string,t_size p_string_length,const char * p_tofind,t_size p_tofind_length,t_size p_start) {
	p_string_length = strlen_max(p_string,p_string_length); p_tofind_length = strlen_max(p_tofind,p_tofind_length);
	if (p_string_length >= p_tofind_length) {
		t_size max = min_t<t_size>(p_string_length - p_tofind_length,p_start);
		for(t_size walk = max; walk != (t_size)(-1); walk--) {
			if (strcmp_partial_ex(p_string+walk,p_string_length-walk,p_tofind,p_tofind_length) == 0) return walk;
		}
	}
	return infinite;
}


bool string_is_numeric(const char * p_string,t_size p_length) {
	bool retval = false;
	for(t_size walk = 0; walk < p_length && p_string[walk] != 0; walk++) {
		if (!char_is_numeric(p_string[walk])) {retval = false; break;}
		retval = true;
	}
	return retval;
}


void string_base::fix_dir_separator(char p_char) {
	t_size length = get_length();
	if (length == 0 || get_ptr()[length-1] != p_char) add_byte(p_char);
}

}