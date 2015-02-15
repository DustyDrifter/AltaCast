#include "foobar2000.h"

bool preferences_page::get_help_url(pfc::string_base & p_out)
{
	p_out = "http://help.foobar2000.org/";
	p_out += core_version_info::g_get_version_string();
	p_out += "/preferences/";
	p_out += (const char*) pfc::print_guid(get_guid());
	p_out += "/";
	p_out += get_name();
	return true;
}