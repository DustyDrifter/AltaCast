#ifndef _INITQUIT_H_
#define _INITQUIT_H_

#include "service.h"

//init/quit callback, on_init is called after main window has been created, on_quit is called before main window is destroyed
class NOVTABLE initquit : public service_base
{
public:
	virtual void on_init() {}
	virtual void on_quit() {}

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(initquit);
};

template<typename T>
class initquit_factory_t : public service_factory_single_t<T> {};

#endif