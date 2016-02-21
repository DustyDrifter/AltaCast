namespace listview_helper {

enum TListViewEditState {
	KListViewEditAborted,
	KListViewEditTab,
	KListViewEditShiftTab,
	KListViewEditEnter,
	KListViewEditLostFocus,
};

bool RunListViewEdit(HWND p_listview,unsigned p_item,unsigned p_column,pfc::string_base & p_content,TListViewEditState & p_state);
bool RunListViewEdit(HWND p_listview,unsigned p_item,unsigned p_column,TListViewEditState & p_state);

bool RunListViewEdit_MultiLine(HWND p_listview,unsigned p_item,unsigned p_column,unsigned p_height,pfc::string_base & p_content,TListViewEditState & p_state);


class ListViewEditCallback {
public:
	virtual bool GetItemText(t_size p_item,pfc::string_base & p_string,unsigned & p_linecount) = 0;
	virtual void SetItemText(t_size p_item,const char * p_string) = 0;
	void Run(HWND p_listview,unsigned p_column);
protected:
	ListViewEditCallback() {}
	~ListViewEditCallback() {}
};

}