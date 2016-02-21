#ifndef _foobar2000_sdk_library_manager_h_
#define _foobar2000_sdk_library_manager_h_

/*!
This service implements methods allowing you to interact with the Media Library.\n
All methods are valid from main thread only.\n
To avoid race conditions, methods that alter library contents should not be called from inside global callbacks.\n
Usage: Use static_api_ptr_t<library_manager> to instantiate.
*/

class NOVTABLE library_manager : public service_base {
public:
	//! Interface for use with library_manager::enum_items().
	class NOVTABLE enum_callback {
	public:
		//! Return true to continue enumeration, false to abort.
		virtual bool on_item(const metadb_handle_ptr & p_item) = 0;
	};

	//! Returns whether specified item is in the Media Library or not.
	virtual bool is_item_in_library(const metadb_handle_ptr & p_item) = 0;
	//! Returns whether current user settings allow specified item to be added to the Media Library or not.
	virtual bool is_item_addable(const metadb_handle_ptr & p_item) = 0;
	//! Returns whether current user settings allow specified item path to be added to the Media Library or not.
	virtual bool is_path_addable(const char * p_path) = 0;
	//! Retrieves path of specified item relative to Media Library directory it is in. Returns true on success, false when the item is not in the Media Library.
	virtual bool get_relative_path(const metadb_handle_ptr & p_item,pfc::string_base & p_out) = 0;
	//! Calls callback method for every item in the Media Library. Note that order of items in Media Library is undefined.
	virtual void enum_items(enum_callback & p_callback) = 0;
	//! Adds specified items to the Media Library (items actually added will be filtered according to user settings).
	virtual void add_items(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
	//! Removes specified items from the Media Library (does nothing if specific item is not in the Media Library).
	virtual void remove_items(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
	//! Adds specified items to the Media Library (items actually added will be filtered according to user settings). The difference between this and add_items() is that items are not added immediately; the operation is queued and executed later, so it is safe to call from e.g. global callbacks.
	virtual void add_items_async(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
	
	//! For internal use only; p_data must be sorted by metadb::path_compare; use file_operation_callback static methods instead of calling this directly.
	virtual void on_files_deleted_sorted(const pfc::list_base_const_t<const char *> & p_data) = 0;

	//! Retrieves entire Media Library content.
	virtual void get_all_items(pfc::list_base_t<metadb_handle_ptr> & p_out) = 0;

	//! Returns whether Media Library functionality is enabled or not, for e.g. notifying user to change settings when trying to use a Media Library viewer without having configured Media Library first.
	virtual bool is_library_enabled() = 0;
	//! Pops up Media Library preferences page.
	virtual void show_preferences() = 0;

	//! Rescans user-specified Media Library directories for new files and removes references to files that no longer exist from the Media Library.
	virtual void rescan() = 0;
	
	//! Hints Media Library about dead items detected externally.
	virtual void check_dead_entries(const pfc::list_base_t<metadb_handle_ptr> & p_list) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(library_manager);
};

//! Callback service receiving notifications about Media Library content changes. Methods called only from main thread.\n
//! Use library_callback_factory_t template to register.
class NOVTABLE library_callback : public service_base {
public:
	//! Called when new items are added to the Media Library.
	virtual void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
	//! Called when some items have been removed from the Media Library.
	virtual void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;
	//! Called when some items in the Media Library have been modified.
	virtual void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(library_callback);
};

template<typename T>
class library_callback_factory_t : public service_factory_single_t<T> {};

//! Implement this service to appear on "library viewers" list in Media Library preferences page.\n
//! Use library_viewer_factory_t to register.
class NOVTABLE library_viewer : public service_base {
public:
	//! Retrieves GUID of your preferences page (pfc::guid_null if you don't have one).
	virtual GUID get_preferences_page() = 0;
	//! Queries whether "activate" action is supported (relevant button will be disabled if it's not).
	virtual bool have_activate() = 0;
	//! Activates your Media Library viewer component (e.g. shows its window).
	virtual void activate() = 0;
	//! Retrieves GUID of your library_viewer implementation, for internal identification. Note that this not the same as preferences page GUID.
	virtual GUID get_guid() = 0;
	//! Retrieves name of your Media Library viewer, a null-terminated UTF-8 encoded string.
	virtual const char * get_name() = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(library_viewer);
};

template<typename T>
class library_viewer_factory_t : public service_factory_single_t<T> {};


#endif _foobar2000_sdk_library_manager_h_