#ifndef _APP_CLOSE_BLOCKER_H_
#define _APP_CLOSE_BLOCKER_H_


//! This service is used to signal whether something is currently preventing main window from being closed and app from being shut down.
class NOVTABLE app_close_blocker : public service_base
{
public:
	//! Checks whether this service is currently preventing main window from being closed and app from being shut down.
	virtual bool query() = 0;

	//! Static helper function, checks whether any of registered app_close_blocker services is currently preventing main window from being closed and app from being shut down.
	static bool g_query();

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(app_close_blocker);
};

#endif //_APP_CLOSE_BLOCKER_H_