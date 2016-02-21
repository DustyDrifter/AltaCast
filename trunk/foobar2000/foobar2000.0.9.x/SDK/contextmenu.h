#ifndef _FOOBAR2000_MENU_ITEM_H_
#define _FOOBAR2000_MENU_ITEM_H_

typedef void * t_glyph;


class NOVTABLE contextmenu_item_node {
public:
	enum t_flags {
		FLAG_CHECKED = 1,
		FLAG_DISABLED = 2,
		FLAG_GRAYED = 4,
		FLAG_DISABLED_GRAYED = FLAG_DISABLED|FLAG_GRAYED,
	};

	enum t_type {
		TYPE_POPUP,TYPE_COMMAND,TYPE_SEPARATOR
	};

	virtual bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) = 0;
	virtual t_type get_type() = 0;
	virtual void execute(const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) = 0;
	virtual t_glyph get_glyph(const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) {return 0;}//RESERVED
	virtual t_size get_children_count() = 0;
	virtual contextmenu_item_node * get_child(t_size p_index) = 0;
	virtual bool get_description(pfc::string_base & p_out) = 0;
	virtual GUID get_guid() = 0;
	virtual bool is_mappable_shortcut() = 0;

protected:
	contextmenu_item_node() {}
	~contextmenu_item_node() {}
};

class NOVTABLE contextmenu_item_node_root : public contextmenu_item_node
{
public:
	virtual ~contextmenu_item_node_root() {}
};

class NOVTABLE contextmenu_item_node_leaf : public contextmenu_item_node
{
public:
	t_type get_type() {return TYPE_COMMAND;}
	t_size get_children_count() {return 0;}
	contextmenu_item_node * get_child(t_size) {return NULL;}
};

class NOVTABLE contextmenu_item_node_root_leaf : public contextmenu_item_node_root
{
public:
	t_type get_type() {return TYPE_COMMAND;}
	t_size get_children_count() {return 0;}
	contextmenu_item_node * get_child(t_size) {return NULL;}
};

class NOVTABLE contextmenu_item_node_popup : public contextmenu_item_node
{
public:
	t_type get_type() {return TYPE_POPUP;}
	void execute(const pfc::list_base_const_t<metadb_handle_ptr> & data,const GUID & caller) {}
	bool get_description(pfc::string_base & p_out) {return false;}
};

class NOVTABLE contextmenu_item_node_root_popup : public contextmenu_item_node_root
{
public:
	t_type get_type() {return TYPE_POPUP;}
	void execute(const pfc::list_base_const_t<metadb_handle_ptr> & data,const GUID & caller) {}
	bool get_description(pfc::string_base & p_out) {return false;}
};

class contextmenu_item_node_separator : public contextmenu_item_node
{
public:
	t_type get_type() {return TYPE_SEPARATOR;}
	void execute(const pfc::list_base_const_t<metadb_handle_ptr> & data,const GUID & caller) {}
	bool get_description(pfc::string_base & p_out) {return false;}
	t_size get_children_count() {return 0;}
	bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) 
	{
		p_displayflags = 0;
		p_out = "---";
		return true;
	}
	contextmenu_item_node * get_child(t_size) {return NULL;}
};

/*!
Service class for declaring context menu commands.\n
See contextmenu_item_simple for implementation helper without dynamic menu generation features.\n
All methods are valid from main app thread only.
*/
class NOVTABLE contextmenu_item : public service_base {
public:
	enum t_enabled_state {
		FORCE_OFF,
		DEFAULT_OFF,
		DEFAULT_ON,
	};

	virtual unsigned get_num_items() = 0;
	virtual contextmenu_item_node_root * instantiate_item(unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) = 0;
	virtual GUID get_item_guid(unsigned p_index) = 0;
	virtual void get_item_name(unsigned p_index,pfc::string_base & p_out) = 0;
	virtual void get_item_default_path(unsigned p_index,pfc::string_base & p_out) = 0;
	virtual bool get_item_description(unsigned p_index,pfc::string_base & p_out) = 0;
	virtual t_enabled_state get_enabled_state(unsigned p_index) = 0;
	virtual void item_execute_simple(unsigned p_index,const GUID & p_node,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) = 0;

	bool item_get_display_data_root(pfc::string_base & p_out,unsigned & displayflags,unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller);
	bool item_get_display_data(pfc::string_base & p_out,unsigned & displayflags,unsigned p_index,const GUID & p_node,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller);
	
	static const GUID caller_now_playing;
	static const GUID caller_playlist;
	static const GUID caller_undefined;
	static const GUID caller_keyboard_shortcut_list;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(contextmenu_item);
};

//! contextmenu_item implementation helper for implementing non-dynamically-generated context menu items; derive from this instead of from contextmenu_item directly if your menu items are static.
class NOVTABLE contextmenu_item_simple : public contextmenu_item
{
private:
	class contextmenu_item_node_impl : public contextmenu_item_node_root_leaf
	{
	public:
		contextmenu_item_node_impl(contextmenu_item_simple * p_owner,unsigned p_index) : m_owner(p_owner), m_index(p_index) {}
		bool get_display_data(pfc::string_base & p_out,unsigned & p_displayflags,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) {return m_owner->get_display_data(m_index,p_data,p_out,p_displayflags,p_caller);}
		void execute(const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller) {m_owner->context_command(m_index,p_data,p_caller);}
		bool get_description(pfc::string_base & p_out) {return m_owner->get_item_description(m_index,p_out);}
		GUID get_guid() {return pfc::guid_null;}
		bool is_mappable_shortcut() {return m_owner->item_is_mappable_shortcut(m_index);}
	private:
		service_ptr_t<contextmenu_item_simple> m_owner;
		unsigned m_index;
	};

	contextmenu_item_node_root * instantiate_item(unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller)
	{
		return new contextmenu_item_node_impl(this,p_index);
	}


	void item_execute_simple(unsigned p_index,const GUID & p_node,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID & p_caller)
	{
		if (p_node == pfc::guid_null)
			context_command(p_index,p_data,p_caller);
	}

	virtual bool item_is_mappable_shortcut(unsigned p_index)
	{
		return true;
	}


	virtual bool get_display_data(unsigned n,const pfc::list_base_const_t<metadb_handle_ptr> & data,pfc::string_base & p_out,unsigned & displayflags,const GUID & caller)
	{
		bool rv = false;
		assert(n>=0 && n<get_num_items());
		if (data.get_count()>0)
		{
			rv = context_get_display(n,data,p_out,displayflags,caller);
		}
		return rv;
	}
public:
	//! Same as contextmenu_item_node::t_flags.
	enum t_flags
	{
		FLAG_CHECKED = 1,
		FLAG_DISABLED = 2,
		FLAG_GRAYED = 4,
		FLAG_DISABLED_GRAYED = FLAG_DISABLED|FLAG_GRAYED,
	};

	
	virtual t_enabled_state get_enabled_state(unsigned p_index) {return contextmenu_item::DEFAULT_ON;}
	virtual unsigned get_num_items()=0;
	virtual void get_item_name(unsigned p_index,pfc::string_base & p_out)=0;
	virtual void get_item_default_path(unsigned p_index,pfc::string_base & p_out) = 0;
	virtual void context_command(unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID& p_caller)=0;
	virtual bool context_get_display(unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,pfc::string_base & p_out,unsigned & p_displayflags,const GUID & p_caller) {
		PFC_ASSERT(p_index>=0 && p_index<get_num_items());
		get_item_name(p_index,p_out);
		return true;
	}
	virtual GUID get_item_guid(unsigned p_index) = 0;
	virtual bool get_item_description(unsigned p_index,pfc::string_base & p_out) = 0;

};


template<typename T>
class contextmenu_item_factory_t : public service_factory_single_t<T> {};


#endif //_FOOBAR2000_MENU_ITEM_H_