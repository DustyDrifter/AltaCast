#include "stdafx.h"


#define uTEXT(blah) TEXT(blah)
#define uLVM_SETITEM LVM_SETITEM
#define uLVM_INSERTITEM LVM_INSERTITEM
#define uLVM_INSERTCOLUMN LVM_INSERTCOLUMN
#define uLVM_GETITEM LVM_GETITEM

namespace listview_helper {

	unsigned insert_item(HWND p_listview,unsigned p_index,const char * p_name,LPARAM p_param)
	{
		if (p_index == infinite) p_index = ListView_GetItemCount(p_listview);
		LVITEM item;
		memset(&item,0,sizeof(item));

		pfc::stringcvt::string_os_from_utf8 os_string_temp(p_name);
		os_string_temp.convert(p_name);

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = p_index;
		item.lParam = p_param;
		item.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());
		
		LRESULT ret = uSendMessage(p_listview,uLVM_INSERTITEM,0,(LPARAM)&item);
		if (ret < 0) return infinite;
		else return (unsigned) ret;
	}



	unsigned insert_column(HWND p_listview,unsigned p_index,const char * p_name,unsigned p_width_dlu)
	{
		pfc::stringcvt::string_os_from_utf8 os_string_temp;
		os_string_temp.convert(p_name);

		RECT rect = {0,0,p_width_dlu,0};
		MapDialogRect(GetParent(p_listview),&rect);

		LVCOLUMN data;
		memset(&data,0,sizeof(data));
		data.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		data.fmt = LVCFMT_LEFT;
		data.cx = rect.right;
		data.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());
		
		LRESULT ret = uSendMessage(p_listview,uLVM_INSERTCOLUMN,p_index,(LPARAM)&data);
		if (ret < 0) return infinite;
		else return (unsigned) ret;
	}

	void get_item_text(HWND p_listview,unsigned p_index,unsigned p_column,pfc::string_base & p_out) {
		enum {buffer_length = 4096};
		TCHAR buffer[buffer_length];
		ListView_GetItemText(p_listview,p_index,p_column,buffer,buffer_length);
		p_out = pfc::stringcvt::string_utf8_from_os(buffer,buffer_length);
	}

	bool set_item_text(HWND p_listview,unsigned p_index,unsigned p_column,const char * p_name)
	{
		LVITEM item;
		memset(&item,0,sizeof(item));

		pfc::stringcvt::string_os_from_utf8 os_string_temp;
		os_string_temp.convert(p_name);

		item.mask = LVIF_TEXT;
		item.iItem = p_index;
		item.iSubItem = p_column;
		item.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());
		return uSendMessage(p_listview,uLVM_SETITEM,0,(LPARAM)&item) ? true : false;
	}

	bool is_item_selected(HWND p_listview,unsigned p_index)
	{
		LVITEM item;
		memset(&item,0,sizeof(item));
		item.mask = LVIF_STATE;
		item.iItem = p_index;
		item.stateMask = LVIS_SELECTED;
		if (!uSendMessage(p_listview,uLVM_GETITEM,0,(LPARAM)&item)) return false;
		return (item.state & LVIS_SELECTED) ? true : false;
	}

	bool set_item_selection(HWND p_listview,unsigned p_index,bool p_state)
	{
		LVITEM item;
		memset(&item,0,sizeof(item));
		item.mask = LVIF_STATE;
		item.iItem = p_index;
		item.stateMask = LVIS_SELECTED;
		item.state = p_state ? LVIS_SELECTED : 0;
		return uSendMessage(p_listview,uLVM_SETITEM,0,(LPARAM)&item) ? true : false;
	}

	bool select_single_item(HWND p_listview,unsigned p_index)
	{
		LRESULT temp = SendMessage(p_listview,LVM_GETITEMCOUNT,0,0);
		if (temp < 0) return false;
		ListView_SetSelectionMark(p_listview,p_index);
		unsigned n; const unsigned m = pfc::downcast_guarded<unsigned>(temp);
		for(n=0;n<m;n++) {
			enum {mask = LVIS_FOCUSED | LVIS_SELECTED};
			ListView_SetItemState(p_listview,n,n == p_index ? mask : 0, mask);
		}
		return ensure_visible(p_listview,p_index);
	}

	bool ensure_visible(HWND p_listview,unsigned p_index)
	{
		return uSendMessage(p_listview,LVM_ENSUREVISIBLE,p_index,FALSE) ? true : false;
	}
}



bool ListView_GetContextMenuPoint(HWND p_list,LPARAM p_coords,POINT & p_point,int & p_selection) {
	if ((DWORD)p_coords == (DWORD)infinite) {
		int firstsel = ListView_GetFirstSelection(p_list);
		if (firstsel >= 0) {
			RECT rect;
			if (!ListView_GetItemRect(p_list,firstsel,&rect,LVIR_BOUNDS)) return false;
			p_point.x = (rect.left + rect.right) / 2;
			p_point.y = (rect.top + rect.bottom) / 2;
			if (!ClientToScreen(p_list,&p_point)) return false;
		} else {
			RECT rect;
			if (!GetClientRect(p_list,&rect)) return false;
			p_point.x = (rect.left + rect.right) / 2;
			p_point.y = (rect.top + rect.bottom) / 2;
			if (!ClientToScreen(p_list,&p_point)) return false;
		}
		p_selection = firstsel;
		return true;
	} else {
		POINT pt = {(short)LOWORD(p_coords),(short)HIWORD(p_coords)};
		p_point = pt;
		POINT client = pt;
		if (!ScreenToClient(p_list,&client)) return false;
		LVHITTESTINFO info;
		memset(&info,0,sizeof(info));
		info.pt = client;
		p_selection = ListView_HitTest(p_list,&info);
		return true;
	}
}
