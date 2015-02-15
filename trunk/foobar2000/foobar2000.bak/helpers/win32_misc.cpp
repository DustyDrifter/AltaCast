#include "stdafx.h"

void registerclass_scope_delayed::toggle_on(UINT p_style,WNDPROC p_wndproc,int p_clsextra,int p_wndextra,HICON p_icon,HCURSOR p_cursor,HBRUSH p_background,const TCHAR * p_class_name,const TCHAR * p_menu_name) {
	toggle_off();
	WNDCLASS wc;
	memset(&wc,0,sizeof(wc));
	wc.style = p_style;
	wc.lpfnWndProc = p_wndproc;
	wc.cbClsExtra = p_clsextra;
	wc.cbWndExtra = p_wndextra;
	wc.hInstance = core_api::get_my_instance();
	wc.hIcon = p_icon;
	wc.hCursor = p_cursor;
	wc.hbrBackground = p_background;
	wc.lpszMenuName = p_menu_name;
	wc.lpszClassName = p_class_name;
	SetLastError(NO_ERROR);
	m_class = RegisterClass(&wc);
	if (m_class == 0) throw ::exception_win32(GetLastError());
}

void registerclass_scope_delayed::toggle_off() {
	if (m_class != 0) {
		UnregisterClass((LPCTSTR)m_class,core_api::get_my_instance());
		m_class = 0;
	}
}
