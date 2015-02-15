#include "foobar2000.h"


void metadb::handle_create_replace_path_canonical(metadb_handle_ptr & p_out,const metadb_handle_ptr & p_source,const char * p_new_path) {
	handle_create(p_out,make_playable_location(p_new_path,p_source->get_subsong_index()));
}

void metadb::handle_create_replace_path(metadb_handle_ptr & p_out,const metadb_handle_ptr & p_source,const char * p_new_path) {
	pfc::string8 path;
	filesystem::g_get_canonical_path(p_new_path,path);
	handle_create_replace_path_canonical(p_out,p_source,path);
}

void metadb::handle_replace_path_canonical(metadb_handle_ptr & p_out,const char * p_new_path) {
	metadb_handle_ptr temp;
	handle_create_replace_path_canonical(temp,p_out,p_new_path);
	p_out = temp;
}


metadb_io::t_load_info_state metadb_io::load_info(metadb_handle_ptr p_item,t_load_info_type p_type,HWND p_parent_window,bool p_show_errors) {
	return load_info_multi(pfc::list_single_ref_t<metadb_handle_ptr>(p_item),p_type,p_parent_window,p_show_errors);
}

metadb_io::t_update_info_state metadb_io::update_info(metadb_handle_ptr p_item,file_info & p_info,HWND p_parent_window,bool p_show_errors)
{
	file_info * blah = &p_info;
	return update_info_multi(pfc::list_single_ref_t<metadb_handle_ptr>(p_item),pfc::list_single_ref_t<file_info*>(blah),p_parent_window,p_show_errors);
}

file_info_update_helper::file_info_update_helper(metadb_handle_ptr p_item)
{
	const t_size count = 1;
	m_data.add_item(p_item);

	m_infos.set_size(count);
	m_mask.set_size(count);
	for(t_size n=0;n<count;n++) m_mask[n] = false;
}

file_info_update_helper::file_info_update_helper(const pfc::list_base_const_t<metadb_handle_ptr> & p_data)
{
	const t_size count = p_data.get_count();
	m_data.add_items(p_data);

	m_infos.set_size(count);
	m_mask.set_size(count);
	for(t_size n=0;n<count;n++) m_mask[n] = false;
}

bool file_info_update_helper::read_infos(HWND p_parent,bool p_show_errors)
{
	static_api_ptr_t<metadb_io> api;
	if (api->load_info_multi(m_data,metadb_io::load_info_default,p_parent,p_show_errors) != metadb_io::load_info_success) return false;
	t_size n; const t_size m = m_data.get_count();
	t_size loaded_count = 0;
	for(n=0;n<m;n++)
	{
		bool val = m_data[n]->get_info(m_infos[n]);
		if (val) loaded_count++;
		m_mask[n] = val;
	}
	return loaded_count == m;
}

file_info_update_helper::t_write_result file_info_update_helper::write_infos(HWND p_parent,bool p_show_errors)
{
	t_size n, outptr = 0; const t_size count = m_data.get_count();
	pfc::array_t<metadb_handle_ptr> items_to_update;
	pfc::array_t<file_info *> infos_to_write;
	items_to_update.set_size(count);
	infos_to_write.set_size(count);
	
	for(n=0;n<count;n++)
	{
		if (m_mask[n])
		{
			items_to_update[outptr] = m_data[n];
			infos_to_write[outptr] = &m_infos[n];
			outptr++;
		}
	}

	if (outptr == 0) return write_ok;
	else
	{
		static_api_ptr_t<metadb_io> api;
		switch(api->update_info_multi(
			pfc::list_const_array_t<metadb_handle_ptr,const pfc::array_t<metadb_handle_ptr>&>(items_to_update,outptr),
			pfc::list_const_array_t<file_info*,const pfc::array_t<file_info*>&>(infos_to_write,outptr),
			p_parent,
			true
			))
		{
		case metadb_io::update_info_success:
			return write_ok;
		case metadb_io::update_info_aborted:
			return write_aborted;
		default:
		case metadb_io::update_info_errors:
			return write_error;
		}
	}
}

t_size file_info_update_helper::get_item_count() const
{
	return m_data.get_count();
}

bool file_info_update_helper::is_item_valid(t_size p_index) const
{
	return m_mask[p_index];
}
	
file_info & file_info_update_helper::get_item(t_size p_index)
{
	return m_infos[p_index];
}

metadb_handle_ptr file_info_update_helper::get_item_handle(t_size p_index) const
{
	return m_data[p_index];
}

void file_info_update_helper::invalidate_item(t_size p_index)
{
	m_mask[p_index] = false;
}


void metadb_io::hint_async(metadb_handle_ptr p_item,const file_info & p_info,const t_filestats & p_stats,bool p_fresh)
{
	const file_info * blargh = &p_info;
	hint_multi_async(pfc::list_single_ref_t<metadb_handle_ptr>(p_item),pfc::list_single_ref_t<const file_info *>(blargh),pfc::list_single_ref_t<t_filestats>(p_stats),bit_array_val(p_fresh));
}


bool metadb::g_get_random_handle(metadb_handle_ptr & p_out) {
	if (static_api_ptr_t<playback_control>()->get_now_playing(p_out)) return true;

	{
		static_api_ptr_t<playlist_manager> api;	

		t_size playlist_count = api->get_playlist_count();
		t_size active_playlist = api->get_active_playlist();
		if (active_playlist != infinite) {
			if (api->playlist_get_focus_item_handle(p_out,active_playlist)) return true;
		}

		for(t_size n = 0; n < playlist_count; n++) {
			if (api->playlist_get_focus_item_handle(p_out,n)) return true;
		}

		if (active_playlist != infinite) {
			t_size item_count = api->playlist_get_item_count(active_playlist);
			if (item_count > 0) {
				if (api->playlist_get_item_handle(p_out,active_playlist,0)) return true;
			}
		}
		
		for(t_size n = 0; n < playlist_count; n++) {
			t_size item_count = api->playlist_get_item_count(n);
			if (item_count > 0) {
				if (api->playlist_get_item_handle(p_out,n,0)) return true;
			}
		}
	}

	return false;
}


