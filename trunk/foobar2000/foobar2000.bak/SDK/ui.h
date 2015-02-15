#ifndef _FOOBAR2000_UI_H_
#define _FOOBAR2000_UI_H_

#include "service.h"

#ifndef _WINDOWS
#error PORTME
#endif

//! Entrypoint service for user interface modules. Implement when registering an UI module. Do not call existing implementations; only core enumerates / dispatches calls. To control UI behaviors from other components, use ui_control API. \n
//! Use user_interface_factory_t<> to register, e.g static user_interface_factory_t<myclass> g_myclass_factory;
class NOVTABLE user_interface : public service_base {
public:
	//!HookProc usage: \n
	//! in your windowproc, call HookProc first, and if it returns true, return LRESULT value it passed to you
	typedef BOOL (WINAPI * HookProc_t)(HWND wnd,UINT msg,WPARAM wp,LPARAM lp,LRESULT * ret);

	//! Retrieves name (UTF-8 null-terminated string) of the UI module.
	virtual const char * get_name()=0;
	//! Initializes the UI module - creates the main app window, etc. Failure should be signaled by appropriate exception (std::exception or a derivative).
	virtual HWND init(HookProc_t hook)=0;
	//! Deinitializes the UI module - destroys the main app window, etc.
	virtual void shutdown()=0;
	//! Activates main app window.
	virtual void activate()=0;
	//! Minimizes/hides main app window.
	virtual void hide()=0;
	//! Returns whether main window is visible / not minimized. Used for activate/hide command.
	virtual bool is_visible() = 0;
	//! Retrieves GUID of your implementation, to be stored in configuration file etc.
	virtual GUID get_guid() = 0;

	//! Overrides statusbar text with specified string. The parameter is a null terminated UTF-8 string. The override is valid until another override_statusbar_text() call or revert_statusbar_text() call.
	virtual void override_statusbar_text(const char * p_text) = 0;
	//! Disables statusbar text override.
	virtual void revert_statusbar_text() = 0;

	//! Shows now-playing item somehow (e.g. system notification area popup).
	virtual void show_now_playing() = 0;

	static bool g_find(service_ptr_t<user_interface> & p_out,const GUID & p_guid);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(user_interface);
};

template<typename T>
class user_interface_factory : public service_factory_single_t<T> {};

//! Interface class allowing you to override UI statusbar text. There may be multiple callers trying to override statusbar text; backend decides which one succeeds so you will not always get what you want. Statusbar text override is automatically cancelled when the object is released.\n
//! Use ui_control::override_status_text_create() to instantiate.
//! Implemented by core. Do not reimplement.
class NOVTABLE ui_status_text_override : public service_base
{
public:
	//! Sets statusbar text to specified UTF-8 null-terminated string.
	virtual void override_text(const char * p_message) = 0;
	//! Cancels statusbar text override.
	virtual void revert_text() = 0;

	FB2K_MAKE_SERVICE_INTERFACE(ui_status_text_override,service_base);
};

//! Serivce providing various UI-related commands. Implemented by core; do not reimplement.
//! Instantiation: use static_api_ptr_t<ui_control>.
class NOVTABLE ui_control : public service_base {
public:
	//! Returns whether primary UI is visible/unminimized.
	virtual bool is_visible()=0;
	//! Activates/unminimizes main UI.
	virtual void activate()=0;
	//! Hides/minimizese main UI.
	virtual void hide()=0;
	//! Retrieves main GUI icon, to use as window icon etc. Returned handle does not need to be freed.
	virtual HICON get_main_icon()=0;
	//! Loads main GUI icon, version with specified width/height. Returned handle needs to be freed with DestroyIcon when you are done using it.
	virtual HICON load_main_icon(unsigned width,unsigned height) = 0;

	//! Activates preferences dialog and navigates to specified page. See also: preference_page API.
	virtual void show_preferences(const GUID & p_page) = 0;

	//! Instantiates ui_status_text_override service, that can be used to display status messages.
	//! @param p_out receives new ui_status_text_override instance.
	//! @returns true on success, false on failure (out of memory / no GUI loaded / etc)
	virtual bool override_status_text_create(service_ptr_t<ui_status_text_override> & p_out) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(ui_control);
};

//! Service called from the UI when some object is dropped into the UI. Usable for modifying drag&drop behaviors such as adding custom handlers for object types other than supported media files.\n
//! Implement where needed; use ui_drop_item_callback_factory_t<> template to register, e.g. static ui_drop_item_callback_factory_t<myclass> g_myclass_factory.
class NOVTABLE ui_drop_item_callback : public service_base {
public:
	//! Called when an object was dropped; returns true if the object was processed and false if not.
	virtual bool on_drop(interface IDataObject * pDataObject) = 0;
	//! Tests whether specified object type is supported by this ui_drop_item_callback implementation. Returns true and sets p_effect when it's supported; returns false otherwise. \n
	//! See IDropTarget::DragEnter() documentation for more information about p_effect values.
	virtual bool is_accepted_type(interface IDataObject * pDataObject, DWORD * p_effect)=0;

	//! Static helper, calls all existing implementations appropriately. See on_drop().
	static bool g_on_drop(interface IDataObject * pDataObject);
	//! Static helper, calls all existing implementations appropriately. See is_accepted_type().
	static bool g_is_accepted_type(interface IDataObject * pDataObject, DWORD * p_effect);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(ui_drop_item_callback);
};

template<class T>
class ui_drop_item_callback_factory_t : public service_factory_single_t<T> {};

#endif