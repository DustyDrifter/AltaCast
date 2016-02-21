#include "foobar2000.h"
#include "component.h"

foobar2000_api * g_api;

service_class_ref service_factory_base::enum_find_class(const GUID & p_guid)
{
	PFC_ASSERT(core_api::are_services_available() && g_api);
	return g_api->service_enum_find_class(p_guid);
}

bool service_factory_base::enum_create(service_ptr_t<service_base> & p_out,service_class_ref p_class,t_size p_index)
{
	PFC_ASSERT(core_api::are_services_available() && g_api);
	return g_api->service_enum_create(p_out,p_class,p_index);
}

t_size service_factory_base::enum_get_count(service_class_ref p_class)
{
	PFC_ASSERT(core_api::are_services_available() && g_api);
	return g_api->service_enum_get_count(p_class);
}

service_factory_base * service_factory_base::__internal__list = NULL;




void service_base::service_release_safe() throw() {
	if (this != NULL) service_release();
}

void service_base::service_add_ref_safe() throw() {
	if (this != NULL) service_add_ref();
}