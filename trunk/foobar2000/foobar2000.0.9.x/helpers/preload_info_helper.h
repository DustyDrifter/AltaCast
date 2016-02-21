namespace preload_info_helper {
	bool preload_info(metadb_handle_ptr p_item,HWND p_parent_window,bool p_showerror);
	bool preload_info_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_items,HWND p_parent_window,bool p_showerror);
	bool preload_info_multi_modalcheck(const pfc::list_base_const_t<metadb_handle_ptr> & p_items,HWND p_parent_window,bool p_showerror);
	bool are_all_loaded(const pfc::list_base_const_t<metadb_handle_ptr> & p_items);
};