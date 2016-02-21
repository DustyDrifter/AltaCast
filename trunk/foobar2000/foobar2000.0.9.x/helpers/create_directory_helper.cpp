#include "stdafx.h"
#include "create_directory_helper.h"

namespace create_directory_helper
{
	static void create_path_internal(const char * p_path,t_size p_base,abort_callback & p_abort) {
		pfc::string8_fastalloc temp;
		for(t_size walk = p_base; p_path[walk]; walk++) {
			if (p_path[walk] == '\\') {
				temp.set_string(p_path,walk);
				try {filesystem::g_create_directory(temp.get_ptr(),p_abort);} catch(exception_io_already_exists) {}
			}
		}
	}

	static bool test_localpath(const char * p_path) {
		if (pfc::strcmp_partial(p_path,"file://") == 0) p_path += strlen("file://");
		return pfc::char_is_ascii_alpha(p_path[0]) && 
			p_path[1] == ':' &&
			p_path[2] == '\\';
	}
	static bool test_netpath(const char * p_path) {
		if (pfc::strcmp_partial(p_path,"file://") == 0) p_path += strlen("file://");
		if (*p_path != '\\') return false;
		p_path++;
		if (*p_path != '\\') return false;
		p_path++;
		if (!pfc::char_is_ascii_alphanumeric(*p_path)) return false;
		p_path++;
		while(pfc::char_is_ascii_alphanumeric(*p_path)) p_path++;
		if (*p_path != '\\') return false;
		return true;
	}

	void create_path(const char * p_path,abort_callback & p_abort) {
		if (test_localpath(p_path)) {
			t_size walk = 0;
			if (pfc::strcmp_partial(p_path,"file://") == 0) walk += strlen("file://");
			create_path_internal(p_path,walk + 3,p_abort);
		} else if (test_netpath(p_path)) {
			t_size walk = 0;
			if (pfc::strcmp_partial(p_path,"file://") == 0) walk += strlen("file://");
			while(p_path[walk] == '\\') walk++;
			while(p_path[walk] != 0 && p_path[walk] != '\\') walk++;
			while(p_path[walk] == '\\') walk++;
			create_path_internal(p_path,walk,p_abort);
		} else {
			throw exception_io("Could not create directory structure; unknown path format");
		}
	}

	static bool is_bad_dirchar(char c)
	{
		return c==' ' || c=='.';
	}

	void make_path(const char * parent,const char * filename,const char * extension,bool allow_new_dirs,pfc::string8 & out,bool really_create_dirs,abort_callback & p_abort)
	{
		out.reset();
		if (parent && *parent)
		{
			out = parent;
			out.fix_dir_separator('\\');
		}
		bool last_char_is_dir_sep = true;
		while(*filename)
		{
#ifdef WIN32
			if (allow_new_dirs && is_bad_dirchar(*filename))
			{
				const char * ptr = filename+1;
				while(is_bad_dirchar(*ptr)) ptr++;
				if (*ptr!='\\' && *ptr!='/') out.add_string(filename,ptr-filename);
				filename = ptr;
				if (*filename==0) break;
			}
#endif
			if (pfc::is_path_bad_char(*filename))
			{
				if (allow_new_dirs && (*filename=='\\' || *filename=='/'))
				{
					if (!last_char_is_dir_sep)
					{
						if (really_create_dirs) try{filesystem::g_create_directory(out,p_abort);}catch(exception_io_already_exists){}
						out.add_char('\\');
						last_char_is_dir_sep = true;
					}
				}
				else
					out.add_char('_');
			}
			else
			{
				out.add_byte(*filename);
				last_char_is_dir_sep = false;
			}
			filename++;
		}
		if (out.length()>0 && out[out.length()-1]=='\\')
		{
			out.add_string("noname");
		}
		if (extension && *extension)
		{
			out.add_char('.');
			out.add_string(extension);
		}
	}
}

namespace {

	class titleformat_text_filter_impl_createdir : public titleformat_text_filter
	{
	public:
		void on_new_field() {}
		void write(const GUID & p_inputtype,pfc::string_receiver & p_out,const char * p_data,t_size p_data_length)
		{//not "UTF-8 aware" but coded not to clash with UTF-8, since only filtered chars are lower ASCII
			if (p_inputtype == titleformat_inputtypes::meta) {
				t_size index = 0;
				t_size good_bytes = 0;
				while(index < p_data_length && p_data[index] != 0) {
					unsigned char c = (unsigned char)p_data[index];
					if (c < ' ' || c == '\\' || c=='/' || c=='|' || c==':')
					{
						if (good_bytes > 0) {p_out.add_string(p_data+index-good_bytes,good_bytes);good_bytes=0;}
						p_out.add_string("_",1);
					}
					else good_bytes++;
					index++;
				}
				if (good_bytes > 0) {p_out.add_string(p_data+index-good_bytes,good_bytes);good_bytes=0;}
			} else {
				p_out.add_string(p_data,p_data_length);
			}
		}
	};
}

void create_directory_helper::format_filename(const metadb_handle_ptr & handle,titleformat_hook * p_hook,const char * spec,pfc::string8 & out)
{
	pfc::string8 temp;
	handle->format_title_legacy(p_hook,temp,spec,&titleformat_text_filter_impl_createdir());
	temp.replace_char('/','\\');
	temp.fix_filename_chars('_','\\');
	out = temp;
}
