#ifndef _METADB_H_
#define _METADB_H_

//! API for tag read/write operations. Legal to call from main thread only, except for hint_multi_async() / hint_async().\n
//! Implemented only by core, do not reimplement.\n
//! Use static_api_ptr_t template to access metadb_io methods.
class NOVTABLE metadb_io : public service_base
{
public:
	enum t_load_info_type {
		load_info_default,
		load_info_force,
		load_info_check_if_changed
	};

	enum t_update_info_state {
		update_info_success,
		update_info_aborted,
		update_info_errors,		
	};
	
	enum t_load_info_state {
		load_info_success,
		load_info_aborted,
		load_info_errors,		
	};

	//! Returns whether some tag I/O operation is currently running. Another one can't be started.
	virtual bool is_busy() = 0;
	//! Returns whether - in result of user settings - all update operations will fail.
	virtual bool is_updating_disabled() = 0;
	//! Returns whether - in result of user settings - all update operations will silently succeed but without actually modifying files.
	virtual bool is_file_updating_blocked() = 0;
	//! If another tag I/O operation is running, this call will give focus to its progress window.
	virtual void highlight_running_process() = 0;
	//! Loads tags from multiple items.
	virtual t_load_info_state load_info_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,t_load_info_type p_type,HWND p_parent_window,bool p_show_errors) = 0;
	//! Updates tags on multiple items.
	virtual t_update_info_state update_info_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,const pfc::list_base_const_t<file_info*> & p_new_info,HWND p_parent_window,bool p_show_errors) = 0;
	//! Rewrites tags on multiple items.
	virtual t_update_info_state rewrite_info_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,HWND p_parent_window,bool p_show_errors) = 0;
	//! Removes tags from multiple items.
	virtual t_update_info_state remove_info_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,HWND p_parent_window,bool p_show_errors) = 0;

	virtual void hint_multi(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,const pfc::list_base_const_t<const file_info*> & p_infos,const pfc::list_base_const_t<t_filestats> & p_stats,const bit_array & p_fresh_mask) = 0;

	virtual void hint_multi_async(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,const pfc::list_base_const_t<const file_info*> & p_infos,const pfc::list_base_const_t<t_filestats> & p_stats,const bit_array & p_fresh_mask) = 0;

	virtual void hint_reader(service_ptr_t<class input_info_reader> p_reader,const char * p_path,abort_callback & p_abort) = 0;

	virtual void path_to_handles_simple(const char * p_path,pfc::list_base_t<metadb_handle_ptr> & p_out) = 0;

	virtual void dispatch_refresh(const pfc::list_base_const_t<metadb_handle_ptr> & p_list) = 0;

	void hint_async(metadb_handle_ptr p_item,const file_info & p_info,const t_filestats & p_stats,bool p_fresh);

	t_load_info_state load_info(metadb_handle_ptr p_item,t_load_info_type p_type,HWND p_parent_window,bool p_show_errors);
	t_update_info_state update_info(metadb_handle_ptr p_item,file_info & p_info,HWND p_parent_window,bool p_show_errors);
	
	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(metadb_io);
};

class NOVTABLE metadb_io_callback : public service_base {
public:
	virtual void on_changed_sorted(const pfc::list_base_const_t<metadb_handle_ptr> & p_items_sorted, bool p_fromhook) = 0;//items are always sorted by pointer value

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(metadb_io_callback);
};

//! Entrypoint service for metadb_handle related operations.\n
//! Implemented only by core, do not reimplement.\n
//! Use static_api_ptr_t template to access it, e.g. static_api_ptr_t<metadb>()->handle_create(myhandle,mylocation);
class NOVTABLE metadb : public service_base
{
public:
	//! Locks metadb to prevent other threads from modifying it while you're working with some of its contents. Some functions (metadb_handle::get_info_locked(), metadb_handle::get_info_async_locked()) can be called only from inside metadb lock section.
	virtual void database_lock()=0;
	//! Unlocks metadb after database_lock(). Some functions (metadb_handle::get_info_locked(), metadb_handle::get_info_async_locked()) can be called only from inside metadb lock section.
	virtual void database_unlock()=0;
	
	//! Returns metadb_handle object referencing specified location (attempts to find existing one, creates new one if doesn't exist).
	//! @param p_out Receives metadb_handle pointer.
	//! @param p_location Location to create metadb_handle for.
	virtual void handle_create(metadb_handle_ptr & p_out,const playable_location & p_location)=0;

	void handle_create_replace_path_canonical(metadb_handle_ptr & p_out,const metadb_handle_ptr & p_source,const char * p_new_path);
	void handle_replace_path_canonical(metadb_handle_ptr & p_out,const char * p_new_path);
	void handle_create_replace_path(metadb_handle_ptr & p_out,const metadb_handle_ptr & p_source,const char * p_new_path);

	//! Helper function; attempts to retrieve a handle to any known playable location to be used for e.g. titleformatting script preview.\n
	//! @returns True on success; false on failure (no known playable locations).
	static bool g_get_random_handle(metadb_handle_ptr & p_out);

	enum {case_sensitive = true};

	inline static int path_compare_ex(const char * p1,unsigned len1,const char * p2,unsigned len2) {return case_sensitive ? pfc::strcmp_ex(p1,len1,p2,len2) : stricmp_utf8_ex(p1,len1,p2,len2);}
	inline static int path_compare(const char * p1,const char * p2) {return case_sensitive ? strcmp(p1,p2) : stricmp_utf8(p1,p2);}
	inline static int path_compare_metadb_handle(const metadb_handle_ptr & p1,const metadb_handle_ptr & p2) {return path_compare(p1->get_path(),p2->get_path());}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(metadb);
};


class in_metadb_sync {
public:
	in_metadb_sync() {
		m_api->database_lock();
	}
	~in_metadb_sync() {
		m_api->database_unlock();
	}
private:
	static_api_ptr_t<metadb> m_api;
};

class in_metadb_sync_fromptr {
public:
	in_metadb_sync_fromptr(const service_ptr_t<metadb> & p_api) : m_api(p_api) {m_api->database_lock();}
	~in_metadb_sync_fromptr() {m_api->database_unlock();}
private:
	service_ptr_t<metadb> m_api;
};

class in_metadb_sync_fromhandle {
public:
	in_metadb_sync_fromhandle(const service_ptr_t<metadb_handle> & p_api) : m_api(p_api) {m_api->metadb_lock();}
	~in_metadb_sync_fromhandle() {m_api->metadb_unlock();}
private:
	service_ptr_t<metadb_handle> m_api;
};

class file_info_update_helper
{
public:
	file_info_update_helper(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
	file_info_update_helper(metadb_handle_ptr p_item);

	bool read_infos(HWND p_parent,bool p_show_errors);

	enum t_write_result
	{
		write_ok,
		write_aborted,
		write_error
	};
	t_write_result write_infos(HWND p_parent,bool p_show_errors);

	t_size get_item_count() const;
	bool is_item_valid(t_size p_index) const;//returns false where info reading failed
	
	file_info & get_item(t_size p_index);
	metadb_handle_ptr get_item_handle(t_size p_index) const;

	void invalidate_item(t_size p_index);

private:
	metadb_handle_list m_data;
	pfc::array_t<file_info_impl> m_infos;
	pfc::array_t<bool> m_mask;
};

class titleformat_text_out;
class titleformat_hook_function_params;


/*
	Implementing this service installs a global hook for metadb_handle::format_title field processing. \n
	This should be implemented only where absolutely necessary, for safety and performance reasons. \n
	metadb_display_hook methods should NEVER make any other API calls (other than possibly querying information from passed metadb_handle pointer), only read implementation-specific private data and return as soon as possible. Since those are called from metadb_handle::format_title, no assumptions should be made about calling context (threading etc). \n
	Both methods are called from inside metadb lock, so no additional locking is required to use *_locked metadb_handle methods.
	See titleformat_hook for more info about methods/parameters. \n
	If there are multiple metadb_display_hook implementations registered, call order is undefined.
*/

class metadb_display_hook : public service_base {
public:
	virtual bool process_field(metadb_handle * p_handle,titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) = 0;
	virtual bool process_function(metadb_handle * p_handle,titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(metadb_display_hook);
};


#endif