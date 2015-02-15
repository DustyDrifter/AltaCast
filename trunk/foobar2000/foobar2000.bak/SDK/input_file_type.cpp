#include "foobar2000.h"

void input_file_type::build_openfile_mask(pfc::string_base & out, bool b_include_playlists)
{
	pfc::string8_fastalloc name,mask,mask_alltypes,out_temp;
	
	if (b_include_playlists)
	{
		service_enum_t<playlist_loader> e;
		service_ptr_t<playlist_loader> ptr;
		if (e.first(ptr)) do {
			if (!mask.is_empty()) mask += ";";
			mask += "*.";
			mask += ptr->get_extension();
		} while(e.next(ptr));
		out_temp += "Playlists";
		out_temp += "|";
		out_temp += mask;
		out_temp += "|";

		if (!mask_alltypes.is_empty())
		{
			if (mask_alltypes[mask_alltypes.length()-1]!=';')
				mask_alltypes += ";";
		}
		mask_alltypes += mask;
	}

	{
		service_enum_t<input_file_type> e;
		service_ptr_t<input_file_type> ptr;
		if (e.first(ptr)) do {
			unsigned n,m = ptr->get_count();
			for(n=0;n<m;n++)
			{
				name.reset();
				mask.reset();
				if (ptr->get_name(n,name) && ptr->get_mask(n,mask))
				{
					if (!strchr(name,'|') && !strchr(mask,'|'))
					{
						out_temp += name;
						out_temp += "|";
						out_temp += mask;
						out_temp += "|";
						if (!mask_alltypes.is_empty())
						{
							if (mask_alltypes[mask_alltypes.length()-1]!=';')
								mask_alltypes += ";";
						}
						mask_alltypes += mask;
					}
				}
			}
		} while(e.next(ptr));
	}
	out.reset();
	out += "All files|*.*|";
	if (!mask_alltypes.is_empty())
	{
		out += "All supported types|";
		out += mask_alltypes;
		out += "|";
	}
	out += out_temp;
}

