#include "stdafx.h"

static bool is_spacing(char c) {return c == ' ' || c==10 || c==13;}
static bool is_alpha(char c) {return (c>='a' && c<='z') || (c>='A' && c<='Z');}
static bool is_alphanumeric(char c) {return is_alpha(c) || (c>='0' && c<='9');}
static bool is_spacing_or_null(char c) {return c == 0 || is_spacing(c);}

static bool is_format_spec(const char * ptr) {return strchr(ptr,'%') || strchr(ptr,'#') || strchr(ptr,'$');}

namespace search_tools {

	class substring_filter {
		pfc::ptr_list_t<char> data;
		mutable pfc::string8_fastalloc temp;
	public:
		substring_filter(const char * src) {
			while(*src) {
				while(*src && is_spacing(*src)) src++;
				t_size ptr = 0;
				while(src[ptr] && !is_spacing(src[ptr])) ptr++;
				if (ptr) {
					uStringLower(temp,src,ptr);
					data.add_item(strdup(temp));
				}
				src+=ptr;
			}
		}
		bool test(const char * src) const 		{
			t_size n,m = data.get_count();
			if (m==0) return false;
			uStringLower(temp,src);
			for(n=0;n<m;n++) {
				if (!strstr(temp,data[n])) return false;
			}
			return true;
		}
		~substring_filter() {data.free_all();}
	};


	class filter_node_and : public filter_node 	{
		filter_node_cptr n1, n2;
	public:
		filter_node_and(filter_node_cptr p1,filter_node_cptr p2) : n1(p1), n2(p2) {}
		virtual bool test(const file_info * item,const metadb_handle_ptr & handle) const {return n1->test(item,handle) && n2->test(item,handle);}
	};

	class filter_node_or : public filter_node {
		filter_node_cptr n1, n2;
	public:
		filter_node_or(filter_node_cptr p1,filter_node_cptr p2) : n1(p1), n2(p2) {}
		virtual bool test(const file_info * item,const metadb_handle_ptr & handle) const {return n1->test(item,handle) || n2->test(item,handle);}
	};

	class filter_node_not : public filter_node {
		filter_node_cptr n;
	public:
		filter_node_not(filter_node_cptr p) : n(p) {}
		virtual bool test(const file_info * item,const metadb_handle_ptr & handle) const {return !n->test(item,handle);}
	};

	class filter_node_missing : public filter_node {
		pfc::string8 m_field;
	public:
		filter_node_missing(const char * p_string,t_size p_len) {
			while(p_len > 0 && p_string[0] == ' ') {p_string++;p_len--;}
			while(p_len > 0 && p_string[p_len - 1] == ' ') p_len--;
			m_field.set_string(p_string,p_len);
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			return item->meta_find(m_field) == infinite;
		}
	};

	class filter_node_is_format : public filter_node {
		pfc::string_simple param;
		service_ptr_t<titleformat_object> m_script;
		mutable pfc::string8_fastalloc temp;
		bool is_wildcard;
		bool test_internal(const char * src) const {
			if (is_wildcard) return wildcard_helper::test(src,param,false);
			else return !stricmp_utf8(src,param);
		}
	public:
		filter_node_is_format(const char * p_field,const char * p_param)
			: param(p_param), is_wildcard(wildcard_helper::has_wildcards(p_param))
		{
			if (!static_api_ptr_t<titleformat_compiler>()->compile(m_script,p_field))
				throw exception_parse_error();
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			handle->format_title(0,temp,m_script,0);
			return test_internal(temp);
		}
	};

	class filter_node_is : public filter_node {
		pfc::string_simple field,param;
		mutable pfc::string8_fastalloc temp;
		bool is_wildcard;
		bool test_internal(const char * src) const {
			if (is_wildcard) return wildcard_helper::test(src,param,false);
			else return !stricmp_utf8(src,param);
		}
	public:
		filter_node_is(const char * p_field,const char * p_param)
			: field(p_field), param(p_param), is_wildcard(wildcard_helper::has_wildcards(p_param))
		{
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			t_size index = item->meta_find(field);
			if (index == infinite) return false;
			t_size n,m = item->meta_enum_value_count(index);
			for(n=0;n<m;n++)
			{
				if (test_internal(item->meta_enum_value(index,n))) return true;
			}
			return false;
		}
	};

	class filter_node_has : public filter_node {
		pfc::string_simple field;
		mutable pfc::string8_fastalloc temp;
		substring_filter m_filter;
	public:
		filter_node_has(const char * p_field,const char * p_param)
			: field(p_field), m_filter(p_param)
		{
		}

		~filter_node_has() {}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			t_size index = item->meta_find(field);
			if (index == infinite) return false;
			t_size n, m = item->meta_enum_value_count(index);
			temp.reset();
			for(n=0;n<m;n++)
			{
				temp += " ";
				temp += item->meta_enum_value(index,n);
			}
			return m_filter.test(temp);
		}
	};

	class filter_node_has_ex : public filter_node {
		pfc::string_simple field;
		substring_filter m_filter;			
		mutable pfc::string8_fastalloc temp;
	public:
		filter_node_has_ex(const char * p_field,const char * p_param)
			: field(p_field), m_filter(p_param)
		{
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			handle->format_title_legacy(0,temp,field,0);
			return m_filter.test(temp);
		}
	};

	class filter_node_simple : public filter_node {
		substring_filter m_filter;
		mutable pfc::string8_fastalloc temp;
	public:
		filter_node_simple(const char * src) : m_filter(src) {}
		bool test(const file_info * info,const metadb_handle_ptr & handle) const {
			t_size n, m = info->meta_get_count();
			temp = handle->get_path();
			for(n=0;n<m;n++) {
				t_size n1, m1 = info->meta_enum_value_count(n);
				for(n1=0;n1<m1;n1++)
				{
					temp += " ";
					temp += info->meta_enum_value(n,n1);
				}
			}
			m = info->info_get_count();
			for(n=0;n<m;n++) {
				temp += " ";
				temp += info->info_enum_value(n);
			}

			return m_filter.test(temp);
		}
	};

	class parser {
		const char * base;
		t_size len,ptr;
	public:
		inline parser() : base(0), len(0), ptr(0) {}
		inline parser(const char * p_ptr,t_size p_len) : base(p_ptr), len(p_len), ptr(0) {}
		inline parser(const parser & param) : base(param.get_ptr()), len(param.get_remaining()), ptr(0) {}
		inline parser(const parser & param,t_size len) : base(param.get_ptr()), len(len), ptr(0) {assert(len<=param.get_remaining());}
		inline const char * get_ptr() const {return base+ptr;}
		inline t_size get_remaining() const {return len-ptr;}
		inline t_size advance(t_size n) {ptr+=n; assert(ptr<=len);return n;}
		inline char get_char() const {return get_ptr()[0];}
		inline t_size get_offset() const {return ptr;}
		inline void reset(t_size offset=0) {ptr=offset;}
		
		t_size test_spacing() const {
			t_size n = 0, m = get_remaining();
			const char * src = get_ptr();
			while(n<m && is_spacing(src[n])) n++;
			return n;
		}

		t_size skip_spacing() {
			return advance(test_spacing());
		}

		bool is_empty() const {return test_spacing() == get_remaining();}

		t_size test_token() const {
			parser p(*this);
			if (p.get_remaining()==0) return 0;
			if (p.get_char() == '\"')
			{
				p.advance(1);
				for(;;)
				{
					if (p.get_remaining()==0) return 0;
					if (p.get_char()=='\"')
					{
						p.advance(1);
						return p.get_offset();
					}
					else p.advance(1);
				}
			}
			else if (p.get_char()=='(')
			{
				p.advance(1);
				p.skip_spacing();
				do {
					if (p.skip_token()==0) return 0;
					p.skip_spacing();
					if (p.get_remaining()==0) return 0;
				} while(p.get_char()!=')');
				p.advance(1);
				return p.get_offset();
			}
			else if (p.get_char()==')') return 0;
			else
			{
				do { p.advance(1); } while(p.get_remaining()>0 && p.get_char()!=')' && !is_spacing(p.get_char()));
				return p.get_offset();
			}
			return 0;
		}

		t_size skip_token() {
			return advance(test_token());
		}
	};

	typedef filter_node_cptr (*operator_handler)(const parser & token_left,const parser & token_right);

	static filter_node_cptr operator_handler_and(const parser & token_left,const parser & token_right) {
		return pfc::rcnew_t<filter_node_and>(filter_node::g_create(token_left),filter_node::g_create(token_right));
	}

	static filter_node_cptr operator_handler_or(const parser & token_left,const parser & token_right) {
		return pfc::rcnew_t<filter_node_or>(filter_node::g_create(token_left),filter_node::g_create(token_right));
	}

	static filter_node_cptr operator_handler_not(const parser & token_left,const parser & token_right) {
		if (!token_left.is_empty()) throw exception_parse_error();
		return pfc::rcnew_t<filter_node_not>(filter_node::g_create(token_right));
	}
	
	static filter_node_cptr operator_handler_missing(const parser & token_left,const parser & token_right)
	{
		if (!token_right.is_empty()) throw exception_parse_error();
		return pfc::rcnew_t<filter_node_missing>(token_left.get_ptr(),token_left.get_remaining());
	}

	static void parse_string(const parser & src,pfc::string_base & out)
	{
		parser p(src);
		p.skip_spacing();
		out.reset();
		if (p.get_remaining()>0)
		{
			if (p.get_char()=='\"')
			{
				p.advance(1);
				while(p.get_remaining()>0 && p.get_char()!='\"') {out.add_byte(p.get_char());p.advance(1);}
				if (p.get_remaining() == 0) throw exception_parse_error();
				return;
			}
			else
			{
				out.add_string(p.get_ptr(),p.get_remaining());
				t_size trunc = out.length();
				while(trunc>0 && is_spacing(out[trunc-1])) trunc--;
				out.truncate(trunc);
				if (trunc == 0) throw exception_parse_error();
				return;
			}
		}
		else throw exception_parse_error();
	}

	static filter_node_cptr operator_handler_has(const parser & token_left,const parser & token_right) {
		pfc::string8 name,value;
		parse_string(token_left,name); parse_string(token_right,value);
		if (!strcmp(name,"*")) return pfc::rcnew_t<filter_node_simple>(value);
		else if (is_format_spec(name)) return pfc::rcnew_t<filter_node_has_ex>(name,value);
		else return pfc::rcnew_t<filter_node_has>(name,value);
	}

	static filter_node_cptr operator_handler_is(const parser & token_left,const parser & token_right)
	{
		pfc::string8 name,value;
		parse_string(token_left,name); parse_string(token_right,value);
		if (is_format_spec(name)) return pfc::rcnew_t<filter_node_is_format>(name,value);
		else return pfc::rcnew_t<filter_node_is>(name,value);
	}

	class filter_node_mathop : public filter_node
	{
		pfc::string_simple left;
		service_ptr_t<titleformat_object> left_script;
		t_int64 rval;
		bool left_ex;
		int type;
		mutable pfc::string8_fastalloc ltemp;

		static t_int64 parse_int_or_time(const char * ptr)
		{
			bool neg = false;
			t_int64 a = 0;
			// whitespace
			while (ptr[0] == ' ' || ptr[0] == '\t')
				ptr++;
			// sign
			neg = ptr[0] == '-';
			if (ptr[0] == '+' || ptr[0] == '-')
				ptr++;
			// digits
			if (ptr[0] >= '0' && ptr[0] <= '9')
			{
				do
				{
					a = a * 10 + (ptr[0] - '0');
					ptr++;
				}
				while (*ptr >= '0' && *ptr <= '9');
			}
			else
				return 0;
			if (ptr[0] != ':')
				return neg ? -a : a;
			ptr++;
			// minutes or seconds
			if ((ptr[0] >= '0' && ptr[0] <= '5') && (ptr[1] >= '0' && ptr[1] <= '9'))
			{
				a = a * 60 + (ptr[0] - '0') * 10 + (ptr[1] - '0');
				if (ptr[2] == ':')
				{
					// seconds
					if ((ptr[3] >= '0' && ptr[3] <= '5') && (ptr[4] >= '0' && ptr[4] <= '9') && !(ptr[5] >= '0' && ptr[5] <= '9'))
					{
						a = a * 60 + (ptr[3] - '0') * 10 + (ptr[4] - '0');
					}
				}
			}
			return neg ? -a : a;
		}

	public:
		explicit filter_node_mathop(const char * p_left,const char * p_right,int p_type) : left(p_left), rval(parse_int_or_time(p_right)), type(p_type),
			left_ex(is_format_spec(p_left))
		{
			if (left_ex)
				static_api_ptr_t<titleformat_compiler>()->compile_safe(left_script,left);
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const 
		{
			t_int64 lval;
			if (left_ex)
			{
				handle->format_title(0,ltemp,left_script,0);
				lval = parse_int_or_time(ltemp);
			}
			else
			{
				const char * ptr = item->meta_get(left,0);
				if (!ptr) return false;
				lval = parse_int_or_time(ptr);
			}

			switch(type)
			{
			case -1: return lval <  rval;
			case 0:  return lval == rval;
			case 1:  return lval >  rval;
			default: assert(0); return false;
			}
		}
	};

	static filter_node_cptr operator_handler_mathop(const parser & token_left,const parser & token_right,int type) {
		pfc::string8 left,right;
		parse_string(token_left,left); parse_string(token_right,right);
		return pfc::rcnew_t<filter_node_mathop>(left,right,type);
	}

	static filter_node_cptr operator_handler_greater(const parser & token_left,const parser & token_right)
	{
		return operator_handler_mathop(token_left,token_right,1);
	}

	static filter_node_cptr operator_handler_less(const parser & token_left,const parser & token_right)
	{
		return operator_handler_mathop(token_left,token_right,-1);
	}

	static filter_node_cptr operator_handler_equal(const parser & token_left,const parser & token_right)
	{
		return operator_handler_mathop(token_left,token_right,0);
	}

	struct operator_desc
	{
		const char * name;
		bool right_to_left;
		operator_handler handler;
		
		bool test_name(const char * ptr,t_size len) const
		{
			const char * ptr2 = name;
			while(len)
			{
				if (*ptr != *ptr2) return false;
				ptr++;
				ptr2++;
				len--;
			}
			return *ptr2==0;
		}
	};

	static operator_desc g_operators[] =
	{
		{"OR",false,operator_handler_or},
		{"AND",false,operator_handler_and},
		{"NOT",false,operator_handler_not},
		{"HAS",false,operator_handler_has},
		{"IS",false,operator_handler_is},
		{"IST",false,operator_handler_is},
		{"EQUAL",false,operator_handler_equal},
		{"GREATER",false,operator_handler_greater},
		{"LESS",false,operator_handler_less},
		{"MISSING",false,operator_handler_missing},
	};


	static bool has_operators(const char * src) {
		t_size n;
		for(n=0;n<tabsize(g_operators);n++) {
			const char * name = g_operators[n].name;
			t_size namelen = strlen(name);
			const char * ptr = src;
			for(;;) {
				ptr = strstr(ptr,name);
				if (ptr) {
					if (ptr && (ptr==src || is_spacing(ptr[-1])) && is_spacing_or_null(ptr[namelen])) return true;
					ptr++;
				}
				else break;
			}
		}
		return false;
	}

	struct token_info {
		t_size ptr,len;
		token_info() {}
		token_info(t_size p_ptr,t_size p_len) : ptr(p_ptr), len(p_len) {}
	};

	static t_size test_operator(const char * ptr,t_size len) {
		t_size n;
		for(n=0;n<tabsize(g_operators);n++) {
			if (g_operators[n].test_name(ptr,len)) return n;
		}
		return ~0;
	}

	static bool is_operator(const char * ptr,t_size len) {return test_operator(ptr,len)!=~0;}

	filter_node_cptr filter_node::g_create(const parser & p_parser,bool b_allow_simple)
	{
		parser p(p_parser);

		if (b_allow_simple)
		{
			if (!has_operators(pfc::string8(p.get_ptr(),p.get_remaining())))
			{
				pfc::string8 temp;
				parse_string(p,temp);
				return pfc::rcnew_t<filter_node_simple>(temp);
			}
		}

		pfc::list_hybrid_t<token_info,8> operators;
		
		p.skip_spacing();
		while(p.get_remaining()>0)
		{
			t_size delta = p.test_token();
			if (delta==0) throw exception_parse_error();
			if (is_operator(p.get_ptr(),delta)) operators.add_item(token_info(p.get_offset(),delta));
			p.advance(delta);
			p.skip_spacing();
		}

		p.reset();

		if (operators.get_count()>0)
		{
			t_size n;
			for(n=0;n<tabsize(g_operators);n++)
			{
				if (g_operators[n].right_to_left)
				{
					t_size m;
					for(m=operators.get_count()-1;m!=infinite;m--)
					{
						if (g_operators[n].test_name(p.get_ptr()+operators[m].ptr,operators[m].len))
						{
							t_size right_base = operators[m].ptr + operators[m].len;
							return g_operators[n].handler(parser(p.get_ptr(),operators[m].ptr),parser(p.get_ptr()+right_base,p.get_remaining()-right_base));
						}
					}
				}
				else
				{
					t_size m;
					for(m=0;m<operators.get_count();m++)
					{
						if (g_operators[n].test_name(p.get_ptr()+operators[m].ptr,operators[m].len))
						{
							t_size right_base = operators[m].ptr + operators[m].len;
							return g_operators[n].handler(parser(p.get_ptr(),operators[m].ptr),parser(p.get_ptr()+right_base,p.get_remaining()-right_base));
						}
					}
				}
			}
			throw exception_parse_error();
		}
		else
		{
			p.skip_spacing();
			if (p.get_remaining()>0 && p.get_char()=='(')
			{
				t_size base = p.get_offset();
				t_size len = p.skip_token();
				if (len==0 || len<2) throw exception_parse_error();
				if (!p.is_empty()) throw exception_parse_error();
				p.reset(base+1);
				return g_create(parser(p,len-2));
			}
			else if (b_allow_simple)
			{
				pfc::string8 temp;
				parse_string(p,temp);
				return pfc::rcnew_t<filter_node_simple>(temp);
			}
			throw exception_parse_error();
		}
	}

	filter_node_cptr filter_node::g_create(const char * ptr,t_size len,bool b_allow_simple) {
		return g_create(parser(ptr,len),b_allow_simple);
	}
}
