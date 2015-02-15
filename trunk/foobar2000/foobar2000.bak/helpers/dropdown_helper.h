#ifndef _DROPDOWN_HELPER_H_
#define _DROPDOWN_HELPER_H_


class cfg_dropdown_history
{
	enum {separator = '\n'};
	cfg_string data;
	unsigned max;
	void build_list(pfc::ptr_list_t<char> & out);
	void parse_list(const pfc::ptr_list_t<char> & src);
public:
	cfg_dropdown_history(const GUID & p_guid,unsigned p_max = 10,const char * init_vals = "") : data(p_guid,init_vals) {max = p_max;}
	void setup_dropdown(HWND wnd);
	void setup_dropdown(HWND wnd,UINT id) {setup_dropdown(GetDlgItem(wnd,id));}
	void add_item(const char * item);
	bool is_empty();
	void on_context(HWND wnd,LPARAM coords);
};


#endif //_DROPDOWN_HELPER_H_