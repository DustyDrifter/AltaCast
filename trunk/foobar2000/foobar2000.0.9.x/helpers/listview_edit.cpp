#include "stdafx.h"

using namespace listview_helper;

static bool test_key(unsigned k) {
	return (GetKeyState(k) & 0x8000) ? true : false;
}

namespace {
	static const TCHAR g_prop_oldproc[] = TEXT("{D85A395B-29B7-4d14-8C0F-E79756068D2B}");
	static const TCHAR g_prop_instance[] = TEXT("{98259F89-7D5F-41a3-ABEB-5EC1EEFC1FDC}");


	class CEditBoxScope {
	public:
		CEditBoxScope(HWND p_listview,unsigned p_item,unsigned p_column,unsigned p_height = 1) 
			: m_listview(p_listview), m_listview_item(p_item), m_listview_column(p_column), m_edit(NULL),
			m_killswitch(false), m_state(KListViewEditAborted), m_receiverclass_registered(false), m_receiver(NULL),
			m_edit_update_switch(false), m_height(p_height)
		{
			try {__init();} catch(...) {__cleanup();throw;}
		}

		~CEditBoxScope() {__cleanup();}

		bool Run(pfc::string_base & p_content,TListViewEditState & p_state);
	private:
		void OnEdited();
		void __cleanup();
		void __init();
		static LRESULT GEditHook_Default(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		static LRESULT CALLBACK GEditHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		void Done(TListViewEditState p_state) {
			if (!m_killswitch) {
				m_killswitch = true; m_state = p_state;
				ShowWindow(m_receiver,SW_HIDE);
			}
		}
		static LRESULT CALLBACK GReceiverProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp);
		static LRESULT CALLBACK GMouseProc(int nCode,WPARAM wParam,LPARAM lParam);
		enum {ID_MYEDIT = 666};


		HWND m_edit;
		HWND m_listview;
		HWND m_receiver;
		unsigned m_listview_item, m_listview_column;
		bool m_killswitch;
		TListViewEditState m_state;
		TCHAR m_receiverclass[64];
		bool m_receiverclass_registered;
		bool m_edit_update_switch;
		bool m_edited;
		const unsigned m_height;
		static CEditBoxScope * g_instance;
	};
	
	CEditBoxScope * CEditBoxScope::g_instance = NULL;

	LRESULT CALLBACK CEditBoxScope::GReceiverProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) {
		switch(msg) {
		case WM_CREATE:
			{
				const CREATESTRUCT * cs = (const CREATESTRUCT *) lp;
				SetWindowLongPtr(wnd,0,(LONG_PTR)cs->lpCreateParams);
			}
			return DefWindowProc(wnd,msg,wp,lp);
		case WM_MOUSEWHEEL:
			return 0;
		case WM_COMMAND:
			switch(wp) {
			case ID_MYEDIT | (EN_CHANGE << 16):
				reinterpret_cast<CEditBoxScope *>(GetWindowLongPtr(wnd,0))->OnEdited();
				return 0;
			default:
				return 0;	
			}
			
		default:
			return DefWindowProc(wnd,msg,wp,lp);
		}
		
	}

	LRESULT CEditBoxScope::GEditHook_Default(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) {
		WNDPROC proc = (WNDPROC) GetProp(wnd,g_prop_oldproc);
		if (proc == NULL) return DefWindowProc(wnd,msg,wp,lp);
		else return proc(wnd,msg,wp,lp);
	}
	
	LRESULT CALLBACK CEditBoxScope::GEditHook(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) {
		switch(msg) {
#if 0
		case WM_XBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
				POINT pt = {(short)LOWORD(lp), (short)HIWORD(lp)};
				ClientToScreen(wnd,&pt);
				HWND clicked = WindowFromPoint(pt);
				if (clicked != wnd) {
					CEditBoxScope * instance = reinterpret_cast<CEditBoxScope *>(GetProp(wnd,g_prop_instance));
					instance->Done(KListViewEditLostFocus);
					SetCapture(NULL);
					if (clicked == instance->m_listview) {
						ScreenToClient(clicked,&pt);
						return SendMessage(clicked,msg,wp,MAKELPARAM(pt.x,pt.y));
					}
				}
			}
			return GEditHook_Default(wnd,msg,wp,lp);
#endif
		case WM_KILLFOCUS:
			{
				CEditBoxScope * instance = reinterpret_cast<CEditBoxScope *>(GetProp(wnd,g_prop_instance));
				instance->Done(KListViewEditLostFocus);
			}
			return GEditHook_Default(wnd,msg,wp,lp);
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
		case WM_KEYDOWN:
			switch(wp) {
			case VK_TAB:
			case VK_RETURN:
				{
					CEditBoxScope * instance = reinterpret_cast<CEditBoxScope *>(GetProp(wnd,g_prop_instance));
					if (wp == VK_RETURN) {
						if (!test_key(VK_LCONTROL) && !test_key(VK_RCONTROL)) instance->Done(KListViewEditEnter);
					} else if (test_key(VK_SHIFT))
						instance->Done(KListViewEditShiftTab);
					else
						instance->Done(KListViewEditTab);
				}
				break;
			case VK_ESCAPE:
				{
					CEditBoxScope * instance = reinterpret_cast<CEditBoxScope *>(GetProp(wnd,g_prop_instance));
					instance->Done(KListViewEditAborted);
				}
				break;
			}
			return GEditHook_Default(wnd,msg,wp,lp);
		default:
			return GEditHook_Default(wnd,msg,wp,lp);
		case WM_DESTROY:
			{
				RemoveProp(wnd,g_prop_instance);
				WNDPROC proc = (WNDPROC) RemoveProp(wnd,g_prop_oldproc);
				if (proc == NULL) return DefWindowProc(wnd,msg,wp,lp);
				else return proc(wnd,msg,wp,lp);
			}
		}
	}


	void CEditBoxScope::OnEdited() {
		if (!m_edit_update_switch) m_edited = true;
	}

	class CMessageFilterScope {
	public:
		CMessageFilterScope(int p_id,HOOKPROC p_fn) {
			if (g_hook != NULL) throw pfc::exception_unexpected_recursion();
			g_hook = SetWindowsHookEx(p_id,p_fn,NULL,GetCurrentThreadId());
		}
		~CMessageFilterScope() {
			if (g_hook != NULL) {
				UnhookWindowsHookEx(g_hook);
				g_hook = NULL;
			}
		}
		static LRESULT GCallNextHook(int nCode,WPARAM wParam,LPARAM lParam) {
			return CallNextHookEx(g_hook,nCode,wParam,lParam);
		}
	private:
		static HHOOK g_hook;
	};
	HHOOK CMessageFilterScope::g_hook = NULL;

	LRESULT CALLBACK CEditBoxScope::GMouseProc(int nCode,WPARAM wParam,LPARAM lParam) {
		const MOUSEHOOKSTRUCT * mhs = (const MOUSEHOOKSTRUCT *) lParam;
		switch(wParam) {
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
		case WM_NCXBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			if (g_instance != NULL && mhs->hwnd != g_instance->m_edit) {
				g_instance->Done(listview_helper::KListViewEditLostFocus);
			}
			break;
		}
		return CMessageFilterScope::GCallNextHook(nCode,wParam,lParam);
	}

	bool CEditBoxScope::Run(pfc::string_base & p_content,TListViewEditState & p_state) {
		modal_dialog_scope l_scope( FindOwningPopup(m_listview) );
		{pfc::vartoggle_t<bool> bah(m_edit_update_switch,true); uSetWindowText(m_edit,p_content); }
		SendMessage(m_edit,EM_SETSEL,0,-1);
		m_edited = false;
		ShowWindow(m_receiver,SW_SHOW);
		SetFocus(m_edit);

		{
			pfc::vartoggle_t<CEditBoxScope*> l_instance_toggle(g_instance,this);
			CMessageFilterScope l_message_filter(WH_MOUSE,GMouseProc);
			while(!m_killswitch) {
				MSG msg;
				if (!GetMessage(&msg,NULL,0,0)) {
					PostThreadMessage(GetCurrentThreadId(),msg.message,msg.wParam,msg.lParam);
					p_state = KListViewEditAborted;
					return false;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		//if (GetFocus() == m_edit) SetFocus(m_listview);
		ShowWindow(m_receiver,SW_HIDE);
		p_state = m_state;
		if (m_state != KListViewEditAborted && m_edited) {
			uGetWindowText(m_edit,p_content);
			return true;
		} else return false;
	}

	void CEditBoxScope::__init() {
		ListView_EnsureVisible(m_listview,m_listview_item,FALSE);
		RECT rect;
		if (!ListView_GetSubItemRect(m_listview,m_listview_item,m_listview_column,LVIR_BOUNDS,&rect)) throw pfc::exception("ListView_GetSubItemRect failure");


		{
			static volatile unsigned increment;
			wsprintf(m_receiverclass,TEXT("{2BE5FC01-896D-47e4-B0E3-7314E1977D7F}/%u"),++increment);
			WNDCLASS wc;
			memset(&wc,0,sizeof(wc));
			wc.cbWndExtra = sizeof(void*);
			wc.hInstance = core_api::get_my_instance();
			wc.lpfnWndProc = GReceiverProc;
			wc.lpszClassName = m_receiverclass;
			if (RegisterClass(&wc) == NULL) throw pfc::exception("RegisterClass failure");
			m_receiverclass_registered = true;
		}

		const bool multiline = m_height > 1;
		if (multiline) {
			rect.bottom = rect.top + (rect.bottom - rect.top) * m_height;
		}

		AdjustWindowRect(&rect,WS_BORDER|WS_CHILD,FALSE);

		bool use_popup = false;

		{
			RECT client;
			GetClientRect(m_listview,&client);
			use_popup = rect.bottom > client.bottom || rect.right > client.right;
		}

		if (use_popup) {
			MapWindowPoints(m_listview,NULL,(POINT*)&rect,2);
		}

		m_receiver = CreateWindowEx(
			use_popup ? WS_EX_TOPMOST : 0,
			m_receiverclass,
			TEXT(""),
			WS_BORDER|(use_popup ? WS_POPUP : WS_CHILD),
			rect.left,rect.top,
			rect.right-rect.left,rect.bottom-rect.top,
			use_popup ? FindOwningPopup(m_listview) : m_listview,
			NULL,
			core_api::get_my_instance(),
			(LPVOID)this);

		if (m_receiver == NULL) throw pfc::exception("CreateWindow failure");

		RECT parent_client;
		GetClientRect(m_receiver,&parent_client);
		m_edit = CreateWindowEx(
			/*WS_EX_STATICEDGE*/ 0,
			WC_EDIT,TEXT(""),
			(multiline ? (WS_VSCROLL|ES_MULTILINE) : ES_AUTOHSCROLL) | WS_CHILD|ES_LEFT|WS_VISIBLE,//parent is invisible now
			0,0,
			parent_client.right,parent_client.bottom,
			m_receiver,
			(HMENU)ID_MYEDIT,
			core_api::get_my_instance(),
			0);
		if (m_edit == NULL) throw pfc::exception("CreateWindow failure");
		SendMessage(m_edit,WM_SETFONT,SendMessage(m_listview,WM_GETFONT,0,0),0);
		SetProp(m_edit,g_prop_instance,(HANDLE)this);
		SetProp(m_edit,g_prop_oldproc,(HANDLE)uHookWindowProc(m_edit,GEditHook));
		//SetCapture(m_edit);
	}

	void CEditBoxScope::__cleanup() {
		try {
			//SetCapture(NULL);
			if (m_edit != NULL) {DestroyWindow(m_edit);m_edit = NULL;}
			if (m_receiver != NULL) {DestroyWindow(m_receiver);m_receiver = NULL;}
			if (m_receiverclass_registered) {UnregisterClass(m_receiverclass,core_api::get_my_instance());m_receiverclass_registered = false;}
		} catch(...) {}
	}
}

namespace listview_helper {

bool RunListViewEdit(HWND p_listview,unsigned p_item,unsigned p_column,pfc::string_base & p_content,TListViewEditState &p_state) {
	return CEditBoxScope(p_listview,p_item,p_column).Run(p_content,p_state);
}

void ListViewEditCallback::Run(HWND p_listview,unsigned p_column) {
	t_size index, count;
	{
		int temp = ListView_GetFocusItem(p_listview);
		if (temp == -1) return;
		index = (t_size) temp;

		temp = ListView_GetItemCount(p_listview);
		if (temp == -1) return;
		count = (t_size) temp;
	}
	
	for(;;) {
		if (index >= count) break;
		
		pfc::string8 value;
		unsigned linecount = 1;
		if (!GetItemText(index,value,linecount)) break;
		TListViewEditState state;
		if (RunListViewEdit_MultiLine(p_listview,index,p_column,linecount,value,state)) {
			SetItemText(index,value);
		}
		if (state == KListViewEditAborted) break;
		else if (state == KListViewEditTab || state == KListViewEditEnter) {
			if (index + 1 >= count) break;
			index++;
		} else if (state == KListViewEditShiftTab) {
			if (index == 0) break;
			--index;
		} else {
			break;
		}

		listview_helper::select_single_item(p_listview,(unsigned)index);
	}
}

bool RunListViewEdit(HWND p_listview,unsigned p_item,unsigned p_column,TListViewEditState & p_state) {
	pfc::string8 content;
	listview_helper::get_item_text(p_listview,p_item,p_column,content);
	if (RunListViewEdit(p_listview,p_item,p_column,content,p_state)) {
		listview_helper::set_item_text(p_listview,p_item,p_column,content);
		return true;
	} else {
		return false;
	}
}

bool RunListViewEdit_MultiLine(HWND p_listview,unsigned p_item,unsigned p_column,unsigned p_height,pfc::string_base & p_content,TListViewEditState & p_state) {
	return CEditBoxScope(p_listview,p_item,p_column,p_height).Run(p_content,p_state);
}

}