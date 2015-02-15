#ifndef _FILE_OPERATION_CALLBACK_H_
#define _FILE_OPERATION_CALLBACK_H_

//! Interface to notify component system about files being deleted or moved. Operates in app's main thread only.

class file_operation_callback : public service_base
{
public:
	//! p_items is a metadb::path_compare sorted list of files that have been deleted.
	virtual void on_files_deleted_sorted(const pfc::list_base_const_t<const char *> & p_items) = 0;
	//! p_from is a metadb::path_compare sorted list of files that have been moved, p_to is a list of corresponding target locations.
	virtual void on_files_moved_sorted(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to) = 0;
	//! p_from is a metadb::path_compare sorted list of files that have been copied, p_to is a list of corresponding target locations.
	virtual void on_files_copied_sorted(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to) = 0;

	static void g_on_files_deleted(const pfc::list_base_const_t<const char *> & p_items);
	static void g_on_files_moved(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to);
	static void g_on_files_copied(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to);


	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(file_operation_callback);
};


#endif //_FILE_OPERATION_CALLBACK_H_