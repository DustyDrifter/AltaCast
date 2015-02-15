namespace search_tools {

	PFC_DECLARE_EXCEPTION(exception_parse_error,pfc::exception,"Error parsing filter expression");

	class filter_node;
		
	typedef pfc::rcptr_const_t<filter_node> filter_node_cptr;
	typedef pfc::rcptr_t<filter_node> filter_node_ptr;

	class NOVTABLE filter_node {
	public:
		virtual bool test(const file_info * item,const metadb_handle_ptr & handle) const = 0;
		virtual ~filter_node() {}
		static filter_node_cptr g_create(const char * ptr,t_size len,bool b_allow_simple = false);
		static filter_node_cptr g_create(const class parser & p,bool b_allow_simple = false);
	};

	class search_filter {
		filter_node_cptr m_root;
	public:
		search_filter() {}
		~search_filter() {}
		bool init(const char * pattern) {
			try {
				m_root =  filter_node::g_create(pattern,strlen(pattern),true) ;
				return true;
			} catch(exception_parse_error) {return false;}
		}

		void deinit() {
			m_root.release();
		}

		bool test(const file_info * item,const metadb_handle_ptr & handle) const {
			return m_root.is_valid() ? m_root->test(item,handle) : false;
		}

		bool test(const metadb_handle_ptr & item) const {
			bool rv = false;
			const file_info * ptr;
			if (item->get_info_locked(ptr)) rv = test(ptr,item);
			return rv;
		}
	};


};