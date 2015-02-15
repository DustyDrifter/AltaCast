#include "foobar2000.h"

void titleformat_compiler::remove_color_marks(const char * src,pfc::string_base & out)//helper
{
	out.reset();
	while(*src)
	{
		if (*src==3)
		{
			src++;
			while(*src && *src!=3) src++;
			if (*src==3) src++;
		}
		else out.add_byte(*src++);
	}
}

static bool test_for_bad_char(const char * source,t_size source_char_len,const char * reserved)
{
	return pfc::strstr_ex(reserved,(t_size)(-1),source,source_char_len) != (t_size)(-1);
}

void titleformat_compiler::remove_forbidden_chars(titleformat_text_out * p_out,const GUID & p_inputtype,const char * p_source,t_size p_source_len,const char * p_reserved_chars)
{
	if (p_reserved_chars == 0 || *p_reserved_chars == 0)
	{
		p_out->write(p_inputtype,p_source,p_source_len);
	}
	else
	{
		p_source_len = pfc::strlen_max(p_source,p_source_len);
		t_size index = 0;
		t_size good_byte_count = 0;
		while(index < p_source_len)
		{
			t_size delta = pfc::utf8_char_len(p_source + index,p_source_len - index);
			if (delta == 0) break;
			if (test_for_bad_char(p_source+index,delta,p_reserved_chars))
			{
				if (good_byte_count > 0) {p_out->write(p_inputtype,p_source+index-good_byte_count,good_byte_count);good_byte_count=0;}
				p_out->write(p_inputtype,"_",1);
			}
			else
			{
				good_byte_count += delta;
			}
			index += delta;
		}
		if (good_byte_count > 0) {p_out->write(p_inputtype,p_source+index-good_byte_count,good_byte_count);good_byte_count=0;}
	}
}

void titleformat_compiler::remove_forbidden_chars_string_append(pfc::string_receiver & p_out,const char * p_source,t_size p_source_len,const char * p_reserved_chars)
{
	remove_forbidden_chars(&titleformat_text_out_impl_string(p_out),pfc::guid_null,p_source,p_source_len,p_reserved_chars);
}

void titleformat_compiler::remove_forbidden_chars_string(pfc::string_base & p_out,const char * p_source,t_size p_source_len,const char * p_reserved_chars)
{
	p_out.reset();
	remove_forbidden_chars_string_append(p_out,p_source,p_source_len,p_reserved_chars);
}

void titleformat_hook_impl_file_info::process_codec(titleformat_text_out * p_out)
{
	pfc::string8 temp;
	const char * val = m_info->info_get("codec");
	if (val)
	{
		p_out->write(titleformat_inputtypes::meta,val);
	}
	else
	{
		val = m_info->info_get("referenced_file");
		if (val) uAddStringUpper(temp,pfc::string_extension(val));
		else uAddStringUpper(temp,pfc::string_extension(m_location.get_path()));
		p_out->write(titleformat_inputtypes::meta,temp);
	}
}

bool titleformat_hook_impl_file_info::remap_meta(t_size & p_meta_index, const char * p_name, t_size p_name_length)
{
	p_meta_index = infinite;
	if (!stricmp_utf8_ex(p_name, p_name_length, "album", infinite))
	{
		p_meta_index = m_info->meta_find("album");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("venue");
		if (p_meta_index != infinite) return true;
		return false;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "artist", infinite))
	{
		p_meta_index = m_info->meta_find("artist");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("album artist");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("composer");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("performer");
		if (p_meta_index != infinite) return true;
		return false;
	}
	else if (!stricmp_utf8_ex(p_name, p_name_length, "album artist", infinite))
	{
		p_meta_index = m_info->meta_find("album artist");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("artist");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("composer");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("performer");
		if (p_meta_index != infinite) return true;
		return false;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"track artist",infinite))
	{
		t_size index_artist, index_album_artist;
		
		index_artist = m_info->meta_find("artist");
		if (index_artist == infinite) return false;
		index_album_artist = m_info->meta_find("album artist");
		if (index_album_artist == infinite) return false;
		if (m_info->are_meta_fields_identical(index_artist, index_album_artist)) return false;

		p_meta_index = index_artist;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"track",infinite) || !stricmp_utf8_ex(p_name,p_name_length,"tracknumber",infinite))
	{
		p_meta_index = m_info->meta_find("tracknumber");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("track");
		if (p_meta_index != infinite) return true;
		return false;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"disc",infinite) || !stricmp_utf8_ex(p_name,p_name_length,"discnumber",infinite))
	{
		p_meta_index = m_info->meta_find("discnumber");
		if (p_meta_index != infinite) return true;
		p_meta_index = m_info->meta_find("disc");
		if (p_meta_index != infinite) return true;
		return false;
	}
	else
	{
		p_meta_index = m_info->meta_find_ex(p_name,p_name_length);
		return p_meta_index != infinite;
	}
}

bool titleformat_hook_impl_file_info::process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag)
{
	p_found_flag = false;

	//todo make this bsearch someday
	if (stricmp_utf8_ex(p_name,p_name_length,"filename",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_filename",infinite) == 0) {
		pfc::string8 temp;
		filesystem::g_get_display_path(m_location.get_path(),temp);
		p_out->write(titleformat_inputtypes::unknown,pfc::string_filename(temp),infinite);
		p_found_flag = true;
		return true;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"filename_ext",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_filename_ext",infinite) == 0) {
		pfc::string8 temp;
		filesystem::g_get_display_path(m_location.get_path(),temp);
		p_out->write(titleformat_inputtypes::unknown,pfc::string_filename_ext(temp),infinite);
		p_found_flag = true;
		return true;
	} else if (!stricmp_utf8_ex(p_name,p_name_length,"filename_sort",infinite)) {
		pfc::string8 temp;
		filesystem::g_get_display_path(m_location.get_path(),temp);
		p_out->write(titleformat_inputtypes::unknown,pfc::string_filename(temp),infinite);
		p_out->write(titleformat_inputtypes::unknown,"|",infinite);
		p_out->write(titleformat_inputtypes::unknown,pfc::format_uint(m_location.get_subsong(),10),infinite);
		p_found_flag = true;
		return true;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"path",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_path",infinite) == 0) {
		pfc::string8 temp;
		filesystem::g_get_display_path(m_location.get_path(),temp);
		p_out->write(titleformat_inputtypes::unknown,temp.is_empty() ? "n/a" : temp.get_ptr(),infinite);
		p_found_flag = true;
		return true;
	} else if (!stricmp_utf8_ex(p_name,p_name_length,"path_sort",infinite)) {
		pfc::string8_fastalloc temp;
		filesystem::g_get_display_path(m_location.get_path(),temp);
		p_out->write(titleformat_inputtypes::unknown,temp,infinite);
		p_out->write(titleformat_inputtypes::unknown,"|",infinite);
		p_out->write(titleformat_inputtypes::unknown,pfc::format_uint(m_location.get_subsong(),10),infinite);
		p_found_flag = true;
		return true;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"directoryname",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_directoryname",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"directory",infinite) == 0) {
		int count = 1;
		if (count > 0)
		{
			pfc::string8_fastalloc temp;
			filesystem::g_get_display_path(m_location.get_path(),temp);
			
			for(;count;count--)
			{
				t_size ptr = temp.scan_filename();
				if (ptr==0) {temp.reset();break;}
				ptr--;
				temp.truncate(ptr);
			}
			
			if (temp.is_empty())
			{
				p_found_flag = false;
			}
			else
			{
				p_out->write(titleformat_inputtypes::meta,temp + temp.scan_filename(),infinite);
				p_found_flag = true;
			}
		}
		return true;
	}
	else if (stricmp_utf8_ex(p_name,p_name_length,"subsong",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_subsong",infinite) == 0)
	{
		p_out->write_int(titleformat_inputtypes::unknown,m_location.get_subsong());
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"channels",infinite))
	{
		unsigned val = (unsigned)m_info->info_get_int("channels");
		switch(val)
		{
		case 0: p_out->write(titleformat_inputtypes::meta,"N/A",infinite); break;
		case 1: p_out->write(titleformat_inputtypes::meta,"mono",infinite); p_found_flag = true; break;
		case 2: p_out->write(titleformat_inputtypes::meta,"stereo",infinite); p_found_flag = true; break;
		default: p_out->write_int(titleformat_inputtypes::meta,val); p_out->write(titleformat_inputtypes::meta,"ch",infinite); p_found_flag = true; break;
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"bitrate",infinite))
	{
		const char * value = m_info->info_get("bitrate_dynamic");
		if (value == 0 || *value == 0) value = m_info->info_get("bitrate");
		if (value == 0 || *value == 0) return false;
		p_out->write(titleformat_inputtypes::meta,value);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"samplerate",infinite))
	{
		const char * value = m_info->info_get("samplerate");
		if (value == 0 || *value == 0) return false;
		p_out->write(titleformat_inputtypes::meta,value);
		p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"title",infinite))
	{
		if (process_meta(p_out,p_name,p_name_length,", ",2,", ",2))
		{
			p_found_flag = true;
			return true;
		}
		else
		{
			pfc::string8 temp;
			filesystem::g_get_display_path(m_location.get_path(),temp);
			pfc::string_filename fn(temp);
			if (fn.is_empty()) p_out->write(titleformat_inputtypes::meta,temp);
			else p_out->write(titleformat_inputtypes::meta,fn);
			p_found_flag = true;
			return true;
		}
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"codec",infinite))
	{
		process_codec(p_out);
		p_found_flag = true;
		return true;
	} else if (!stricmp_utf8_ex(p_name,p_name_length,"codec_profile",infinite)) {
		const char * profile = m_info->info_get("codec_profile");
		if (profile == NULL) return false;
		p_out->write(titleformat_inputtypes::meta,profile,infinite);
		p_found_flag = true;
		return true;		
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"track",infinite) || !stricmp_utf8_ex(p_name,p_name_length,"tracknumber",infinite))
	{
		const t_size pad = 2;
		const char * val = m_info->meta_get_ex("tracknumber",infinite,0);
		if (val == 0) m_info->meta_get_ex("track",infinite,0);
		if (val != 0)
		{
			p_found_flag = true;
			t_size val_len = strlen(val);
			if (val_len < pad)
			{
				t_size n = pad - val_len;
				do {
					p_out->write(titleformat_inputtypes::meta,"0",1);
					n--;
				} while(n > 0);
			}
			p_out->write(titleformat_inputtypes::meta,val);
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"disc",infinite) || !stricmp_utf8_ex(p_name,p_name_length,"discnumber",infinite))
	{
		const t_size pad = 1;
		const char * val = m_info->meta_get_ex("discnumber",infinite,0);
		if (val == 0) val = m_info->meta_get_ex("disc",infinite,0);
		if (val != 0)
		{
			p_found_flag = true;
			t_size val_len = strlen(val);
			if (val_len < pad)
			{
				t_size n = pad - val_len;
				do {
					p_out->write(titleformat_inputtypes::meta,"0");
					n--;
				} while(n > 0);
			}
			p_out->write(titleformat_inputtypes::meta,val);
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"totaltracks",infinite))	{
		const t_size pad = 2;
		const char * val = m_info->meta_get_ex("totaltracks",infinite,0);
		if (val != NULL)
		{
			p_found_flag = true;
			t_size val_len = strlen(val);
			if (val_len < pad)
			{
				t_size n = pad - val_len;
				do {
					p_out->write(titleformat_inputtypes::meta,"0",1);
					n--;
				} while(n > 0);
			}
			p_out->write(titleformat_inputtypes::meta,val);
		}
		return true;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"length",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_length",infinite) == 0) {
		double len = m_info->get_length();
		if (len>0)
		{
			p_out->write(titleformat_inputtypes::unknown,pfc::format_time(pfc::rint64(len)));
			p_found_flag = true;
			return true;
		}
		else return false;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"length_ex",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_length_ex",infinite) == 0) {
		double len = m_info->get_length();
		if (len>0)
		{
			p_out->write(titleformat_inputtypes::unknown,pfc::format_time_ex(len),infinite);
			p_found_flag = true;
			return true;
		}
		else return false;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"length_seconds",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_length_seconds",infinite) == 0) {
		double len = m_info->get_length();
		if (len>0) {
			p_out->write_int(titleformat_inputtypes::unknown,(t_uint64)len);
			p_found_flag = true;
			return true;
		} else return false;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"length_seconds_fp",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_length_seconds_fp",infinite) == 0) {
		double len = m_info->get_length();
		if (len>0) {
			p_out->write(titleformat_inputtypes::unknown,pfc::string_fixed_t<64>()<<len);
			p_found_flag = true;
			return true;
		} else return false;
	} else if (stricmp_utf8_ex(p_name,p_name_length,"length_samples",infinite) == 0 || stricmp_utf8_ex(p_name,p_name_length,"_length_samples",infinite) == 0) {
		t_int64 val = m_info->info_get_length_samples();
		if (val>0) {
			p_out->write_int(titleformat_inputtypes::unknown,val);
			p_found_flag = true;
			return true;
		} else return false;
	}
	else if (p_name_length > 2 && p_name[0] == '_' && p_name[1] == '_')
	{//info
		if (!stricmp_utf8_ex(p_name,p_name_length,"__replaygain_album_gain",infinite))
		{
			char rgtemp[replaygain_info::text_buffer_size];
			m_info->get_replaygain().format_album_gain(rgtemp);
			if (rgtemp[0] == 0) return false;
			p_out->write(titleformat_inputtypes::meta,rgtemp);
			p_found_flag = true;
			return true;
		}
		if (!stricmp_utf8_ex(p_name,p_name_length,"__replaygain_album_peak",infinite))
		{
			char rgtemp[replaygain_info::text_buffer_size];
			m_info->get_replaygain().format_album_peak(rgtemp);
			if (rgtemp[0] == 0) return false;
			p_out->write(titleformat_inputtypes::meta,rgtemp);
			p_found_flag = true;
			return true;
		}
		if (!stricmp_utf8_ex(p_name,p_name_length,"__replaygain_track_gain",infinite))
		{
			char rgtemp[replaygain_info::text_buffer_size];
			m_info->get_replaygain().format_track_gain(rgtemp);
			if (rgtemp[0] == 0) return false;
			p_out->write(titleformat_inputtypes::meta,rgtemp);
			p_found_flag = true;
			return true;
		}
		if (!stricmp_utf8_ex(p_name,p_name_length,"__replaygain_track_peak",infinite))
		{
			char rgtemp[replaygain_info::text_buffer_size];
			m_info->get_replaygain().format_track_peak(rgtemp);
			if (rgtemp[0] == 0) return false;
			p_out->write(titleformat_inputtypes::meta,rgtemp);
			p_found_flag = true;
			return true;
		}
		const char * value = m_info->info_get_ex(p_name+2,p_name_length-2);
		if (value == 0 || *value == 0) return false;
		p_out->write(titleformat_inputtypes::meta,value);
		p_found_flag = true;
		return true;
	}
	else if (p_name_length > 1 && p_name[0] == '_')
	{//special field
		bool found = process_extra(p_out,p_name+1,p_name_length-1);
		p_found_flag = found;
		return found;
	}
	else
	{//meta
		t_size index;
		if (remap_meta(index, p_name, p_name_length))
		{
			bool status = process_meta(p_out,index,", ",2,", ",2);
			p_found_flag = status;
			return status;
		}
		return false;
	}
}

bool titleformat_hook_impl_file_info::process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag)
{
	p_found_flag = false;
	if (!stricmp_utf8_ex(p_name,p_name_length,"meta",infinite))
	{
		switch(p_params->get_param_count())
		{
		case 1:
			{
				const char * name;
				t_size name_length;
				p_params->get_param(0,name,name_length);
				bool status = process_meta(p_out,name,name_length,", ",2,", ",2);
				p_found_flag = status;
				return true;
			}
		case 2:
			{
				const char * name;
				t_size name_length;
				p_params->get_param(0,name,name_length);
				t_size index_val = p_params->get_param_uint(1);
				const char * value = m_info->meta_get_ex(name,name_length,index_val);
				if (value != 0)
				{
					p_found_flag = true;
					p_out->write(titleformat_inputtypes::meta,value,infinite);
				}
				return true;
			}
		default:
			return false;
		}
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"meta_sep",infinite))
	{
		switch(p_params->get_param_count())
		{
		case 2:
			{
				const char * name, * sep1;
				t_size name_length, sep1_length;
				p_params->get_param(0,name,name_length);
				p_params->get_param(1,sep1,sep1_length);
				bool status = process_meta(p_out,name,name_length,sep1,sep1_length,sep1,sep1_length);
				p_found_flag = status;
				return true;
			}
		case 3:
			{
				const char * name, * sep1, * sep2;
				t_size name_length, sep1_length, sep2_length;
				p_params->get_param(0,name,name_length);
				p_params->get_param(1,sep1,sep1_length);
				p_params->get_param(2,sep2,sep2_length);
				bool status = process_meta(p_out,name,name_length,sep1,sep1_length,sep2,sep2_length);
				p_found_flag = status;
				return true;
			}
		default:
			return false;
		}
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"meta_test",infinite))
	{
		t_size n, count = p_params->get_param_count();
		if (count == 0) return false;
		bool found_all = true;
		for(n=0;n<count;n++)
		{
			const char * name;
			t_size name_length;
			p_params->get_param(n,name,name_length);
			if (!m_info->meta_exists_ex(name,name_length))
			{
				found_all = false;
				break;
			}
		}
		if (found_all)
		{
			p_found_flag = true;
			p_out->write_int(titleformat_inputtypes::meta,1);
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"meta_num",infinite))
	{
		if (p_params->get_param_count() != 1) return false;
		const char * name;
		t_size name_length;
		p_params->get_param(0,name,name_length);
		t_size count = m_info->meta_get_count_by_name_ex(name,name_length);
		p_out->write_int(titleformat_inputtypes::meta,count);
		if (count > 0) p_found_flag = true;
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"info",infinite))
	{
		if (p_params->get_param_count() != 1) return false;
		const char * name;
		t_size name_length;
		p_params->get_param(0,name,name_length);
		const char * value = m_info->info_get_ex(name,name_length);
		if (value != 0)
		{
			p_found_flag = true;
			p_out->write(titleformat_inputtypes::meta,value);
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"extra",infinite))
	{
		if (p_params->get_param_count() != 1) return false;
		const char * name;
		t_size name_length;
		p_params->get_param(0,name,name_length);
		if (process_extra(p_out,name,name_length)) p_found_flag = true;
		return true;
	} else if (!stricmp_utf8_ex(p_name,p_name_length,"codec",infinite))
	{
		if (p_params->get_param_count() != 0) return false;
		process_codec(p_out);
		p_found_flag = true;
		return true;		
	} else if (!stricmp_utf8_ex(p_name,p_name_length,"channels",infinite))
	{
		if (p_params->get_param_count() != 0) return false;
		unsigned val = (unsigned)m_info->info_get_int("channels");
		switch(val)
		{
		case 0: p_out->write(titleformat_inputtypes::meta,"N/A",infinite); break;
		case 1: p_out->write(titleformat_inputtypes::meta,"mono",infinite); p_found_flag = true; break;
		case 2: p_out->write(titleformat_inputtypes::meta,"stereo",infinite); p_found_flag = true; break;
		default: p_out->write_int(titleformat_inputtypes::meta,val); p_out->write(titleformat_inputtypes::meta,"ch",infinite); p_found_flag = true; break;
		}
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"tracknumber",infinite))
	{
		t_size pad = 2;
		t_size param_count = p_params->get_param_count();
		if (param_count > 1) return false;
		if (param_count == 1) pad = (t_size)p_params->get_param_uint(0);
		const char * val = m_info->meta_get_ex("tracknumber",infinite,0);
		if (val != 0)
		{
			p_found_flag = true;
			t_size val_len = strlen(val);
			if (val_len < pad)
			{
				t_size n = pad - val_len;
				do {
					p_out->write(titleformat_inputtypes::meta,"0",1);
					n--;
				} while(n > 0);
			}
			p_out->write(titleformat_inputtypes::meta,val,infinite);
		}
		return true;
	}
	else return false;
}

bool titleformat_hook_impl_file_info::process_meta(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,const char * p_sep1,t_size p_sep1_length,const char * p_sep2,t_size p_sep2_length)
{
	t_size index = m_info->meta_find_ex(p_name,p_name_length);
	return process_meta(p_out, index, p_sep1, p_sep1_length, p_sep2, p_sep2_length);
}

bool titleformat_hook_impl_file_info::process_meta(titleformat_text_out * p_out,t_size p_index,const char * p_sep1,t_size p_sep1_length,const char * p_sep2,t_size p_sep2_length)
{
	if (p_index == infinite) return false;

	t_size n, m = m_info->meta_enum_value_count(p_index);
	for(n=0;n<m;n++)
	{
		if (n>0)
		{
			if (n+1 == m) p_out->write(titleformat_inputtypes::meta,p_sep2,p_sep2_length);
			else p_out->write(titleformat_inputtypes::meta,p_sep1,p_sep1_length);
		}			
		p_out->write(titleformat_inputtypes::meta,m_info->meta_enum_value(p_index,n),infinite);
	}
	return true;
}

bool titleformat_hook_impl_file_info::process_extra(titleformat_text_out * p_out,const char * p_name,t_size p_name_length)
{
	if (!stricmp_utf8_ex(p_name,p_name_length,"PATH_RAW",infinite))
	{
		p_out->write(titleformat_inputtypes::unknown,m_location.get_path(),infinite);
		return true;
	}
	else if (!stricmp_utf8_ex(p_name,p_name_length,"FOOBAR2000_VERSION",infinite))
	{
		p_out->write(titleformat_inputtypes::unknown,core_version_info::g_get_version_string(),infinite);
		return true;
	}
	else return false;
}

void titleformat_object::run_hook(const playable_location & p_location,const file_info * p_source,titleformat_hook * p_hook,pfc::string_base & p_out,titleformat_text_filter * p_filter)
{
	if (p_hook)
	{
		run(
			&titleformat_hook_impl_splitter(
			&titleformat_hook_impl_file_info(p_location,p_source),
			p_hook),
			p_out,p_filter);
	}
	else
	{
		run(
			&titleformat_hook_impl_file_info(p_location,p_source),
			p_out,p_filter);
	}
}

void titleformat_object::run_simple(const playable_location & p_location,const file_info * p_source,pfc::string_base & p_out)
{
	run(&titleformat_hook_impl_file_info(p_location,p_source),p_out,NULL);
}

t_size titleformat_hook_function_params::get_param_uint(t_size index)
{
	const char * str;
	t_size str_len;
	get_param(index,str,str_len);
	return pfc::atoui_ex(str,str_len);
}


void titleformat_text_out_impl_filter_chars::write(const GUID & p_inputtype,const char * p_data,t_size p_data_length)
{
	titleformat_compiler::remove_forbidden_chars(m_chain,p_inputtype,p_data,p_data_length,m_restricted_chars);
}

bool titleformat_hook_impl_splitter::process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag)
{
	p_found_flag = false;
	if (m_hook1 && m_hook1->process_field(p_out,p_name,p_name_length,p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook2 && m_hook2->process_field(p_out,p_name,p_name_length,p_found_flag)) return true;
	p_found_flag = false;
	return false;
}

bool titleformat_hook_impl_splitter::process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag)
{
	p_found_flag = false;
	if (m_hook1 && m_hook1->process_function(p_out,p_name,p_name_length,p_params,p_found_flag)) return true;
	p_found_flag = false;
	if (m_hook2 && m_hook2->process_function(p_out,p_name,p_name_length,p_params,p_found_flag)) return true;
	p_found_flag = false;
	return false;
}

void titleformat_text_out::write_int_padded(const GUID & p_inputtype,t_int64 val,t_int64 maxval)
{
	const t_size bufsize = 64;
	char temp[bufsize+1];
	t_size len = 0;
	while(maxval) {maxval/=10;len++;}
	if (len == 0) len = 1;
	t_size n;
	for(n=0;n<bufsize;n++) temp[n] = '0';
	temp[n] = 0;
	_i64toa(val,temp+bufsize/2,10);
	write(p_inputtype,temp + strlen(temp) - len,infinite);
}

void titleformat_text_out::write_int(const GUID & p_inputtype,t_int64 val)
{
	char temp[32];
	_i64toa(val,temp,10);
	write(p_inputtype,temp,32);
}
void titleformat_text_filter_impl_reserved_chars::write(const GUID & p_inputtype,pfc::string_receiver & p_out,const char * p_data,t_size p_data_length)
{
	if (p_inputtype == titleformat_inputtypes::meta) titleformat_compiler::remove_forbidden_chars_string_append(p_out,p_data,p_data_length,m_reserved_chars);
	else p_out.add_string(p_data,p_data_length);
}

void titleformat_compiler::run(titleformat_hook * p_source,pfc::string_base & p_out,const char * p_spec)
{
	service_ptr_t<titleformat_object> ptr;
	if (!compile(ptr,p_spec)) p_out = "[COMPILATION ERROR]";
	else ptr->run(p_source,p_out,NULL);
}

void titleformat_compiler::compile_safe(service_ptr_t<titleformat_object> & p_out,const char * p_spec)
{
	if (!compile(p_out,p_spec)) {
		if (!compile(p_out,"%filename%"))
			throw pfc::exception_bug_check();
	}
}


namespace titleformat_inputtypes {
	const GUID meta = { 0xcd839c8e, 0x5c66, 0x4ae1, { 0x8d, 0xad, 0x71, 0x1f, 0x86, 0x0, 0xa, 0xe3 } };
	const GUID unknown = { 0x673aa1cd, 0xa7a8, 0x40c8, { 0xbf, 0x9b, 0x34, 0x37, 0x99, 0x29, 0x16, 0x3b } };
};
