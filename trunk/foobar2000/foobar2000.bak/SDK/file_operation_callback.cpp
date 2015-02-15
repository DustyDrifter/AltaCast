#include "foobar2000.h"


static void g_on_files_deleted_sorted(const pfc::list_base_const_t<const char *> & p_items)
{
	static_api_ptr_t<library_manager>()->on_files_deleted_sorted(p_items);
	static_api_ptr_t<playlist_manager>()->on_files_deleted_sorted(p_items);

	service_ptr_t<file_operation_callback> ptr;
	service_enum_t<file_operation_callback> e;
	while(e.next(ptr))
	{
		ptr->on_files_deleted_sorted(p_items);
	}
}

static void g_on_files_moved_sorted(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to)
{
	static_api_ptr_t<playlist_manager>()->on_files_moved_sorted(p_from,p_to);
	static_api_ptr_t<playlist_manager>()->on_files_deleted_sorted(p_from);

	service_ptr_t<file_operation_callback> ptr;
	service_enum_t<file_operation_callback> e;
	while(e.next(ptr))
	{
		ptr->on_files_moved_sorted(p_from,p_to);
	}
}

static void g_on_files_copied_sorted(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to)
{
	service_ptr_t<file_operation_callback> ptr;
	service_enum_t<file_operation_callback> e;
	while(e.next(ptr))
	{
		ptr->on_files_copied_sorted(p_from,p_to);
	}
}

void file_operation_callback::g_on_files_deleted(const pfc::list_base_const_t<const char *> & p_items)
{
	core_api::ensure_main_thread();
	t_size count = p_items.get_count();
	if (count > 0)
	{
		if (count == 1) g_on_files_deleted_sorted(p_items);
		else
		{
			pfc::array_t<t_size> order; order.set_size(count);
			order_helper::g_fill(order);
			p_items.sort_get_permutation_t(metadb::path_compare,order.get_ptr());
			g_on_files_deleted_sorted(pfc::list_permutation_t<const char*>(p_items,order.get_ptr(),count));
		}
	}
}

void file_operation_callback::g_on_files_moved(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to)
{
	core_api::ensure_main_thread();
	pfc::dynamic_assert(p_from.get_count() == p_to.get_count());
	t_size count = p_from.get_count();
	if (count > 0)
	{
		if (count == 1) g_on_files_moved_sorted(p_from,p_to);
		else
		{
			pfc::array_t<t_size> order; order.set_size(count);
			order_helper::g_fill(order);
			p_from.sort_get_permutation_t(metadb::path_compare,order.get_ptr());
			g_on_files_moved_sorted(pfc::list_permutation_t<const char*>(p_from,order.get_ptr(),count),pfc::list_permutation_t<const char*>(p_to,order.get_ptr(),count));
		}
	}
}

void file_operation_callback::g_on_files_copied(const pfc::list_base_const_t<const char *> & p_from,const pfc::list_base_const_t<const char *> & p_to)
{
	if (core_api::assert_main_thread())
	{
		assert(p_from.get_count() == p_to.get_count());
		t_size count = p_from.get_count();
		if (count > 0)
		{
			if (count == 1) g_on_files_copied_sorted(p_from,p_to);
			else
			{
				pfc::array_t<t_size> order; order.set_size(count);
				order_helper::g_fill(order);
				p_from.sort_get_permutation_t(metadb::path_compare,order.get_ptr());
				g_on_files_copied_sorted(pfc::list_permutation_t<const char*>(p_from,order.get_ptr(),count),pfc::list_permutation_t<const char*>(p_to,order.get_ptr(),count));
			}
		}
	}
}
