#ifndef _MODELESS_DIALOG_H_
#define _MODELESS_DIALOG_H_


//! Service for plugging your nonmodal dialog windows into main app loop to receive IsDialogMessage()-translated messages.\n
//! Note that all methods are valid from main app thread only.\n
//! Usage: static_api_ptr_t<modeless_dialog_manager> or modeless_dialog_manager::g_add / modeless_dialog_manager::g_remove.
class NOVTABLE modeless_dialog_manager : public service_base
{
public:
	//! Adds specified window to global list of windows to receive IsDialogMessage().
	virtual void add(HWND p_wnd) = 0;
	//! Removes specified window from global list of windows to receive IsDialogMessage().
	virtual void remove(HWND p_wnd) = 0;

	//! Static helper; see add().
	static void g_add(HWND p_wnd);
	//! Static helper; see remove().
	static void g_remove(HWND p_wnd);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(modeless_dialog_manager);
};

#endif //_MODELESS_DIALOG_H_