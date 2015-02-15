#ifndef _COREVERSION_H_
#define _COREVERSION_H_

class NOVTABLE core_version_info : public service_base
{
public:
	virtual const char * get_version_string()=0;

	static const char * g_get_version_string() {return static_api_ptr_t<core_version_info>()->get_version_string();}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(core_version_info);
};

#endif //_COREVERSION_H_
