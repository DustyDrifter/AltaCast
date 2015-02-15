namespace pfc {

	class rc_container_base {
	public:
		long add_ref() {
			return ++m_counter;
		}
		long release() {
			long ret = --m_counter;
			if (ret == 0) delete this;
			return ret;
		}
	protected:
		virtual ~rc_container_base() {}
	private:
		refcounter m_counter;
	};

	template<typename t_object>
	class rc_container_t : public rc_container_base {
	public:
		TEMPLATE_CONSTRUCTOR_FORWARD_FLOOD(rc_container_t,m_object)

		t_object m_object;
	};

	template<typename t_object>
	class rcptr_const_t {
	protected:
		typedef rc_container_base t_container;
		typedef rc_container_t<t_object> t_container_impl;
	private:
		typedef rcptr_const_t<t_object> t_self;
	public:

		rcptr_const_t() : m_container(NULL), m_ptr(NULL) {}
		rcptr_const_t(const t_self & p_source) : m_container(NULL), m_ptr(NULL) {*this = p_source;}

		void __set_from_cast(t_container * p_container,t_object * p_ptr) {
			release();
			p_container->add_ref();
			m_container = p_container;
			m_ptr = p_ptr;
		}

		bool is_valid() const {return m_ptr != NULL;}
		bool is_empty() const {return m_ptr == NULL;}

		t_self const & operator=(const t_self & p_source) {
			release(); 
			if (p_source.is_valid()) {
				p_source.m_container->add_ref();
				m_container = p_source.m_container;
				m_ptr = p_source.m_ptr;
			}
			return *this;
		}

		~rcptr_const_t() {release();}

		void release() {
			t_container * temp = m_container;
			m_ptr = NULL;
			m_container = NULL;
			if (temp != NULL) temp->release();
		}

		const t_object & operator*() const {return *m_ptr;}
		const t_object * operator->() const {return m_ptr;}

		template<typename t_object_cast>
		operator rcptr_const_t<t_object_cast>() const {
			rcptr_const_t<t_object_cast> temp;
			if (is_valid()) temp.__set_from_cast(m_container,m_ptr);
			return temp;
		}

		template<typename t_object_cast>
		rcptr_const_t<t_object_cast> static_cast_t() const {
			rcptr_const_t<t_object_cast> temp;
			if (is_valid()) temp.__set_from_cast(m_container,static_cast<t_object_cast*>(m_ptr));
			return temp;
		}

		bool operator==(const t_self & p_other) const {
			return m_container == p_other.m_container;
		}

		bool operator!=(const t_self & p_other) const {
			return m_container != p_other.m_container;
		}

	protected:
		t_container * m_container;
		t_object * m_ptr;
	};

	template<typename t_object>
	class rcptr_t : public rcptr_const_t<t_object> {
	private:
		typedef rcptr_t<t_object> t_self;
	protected:
		typedef rc_container_base t_container;
		typedef rc_container_t<t_object> t_container_impl;
	public:
		t_self const & operator=(const t_self & p_source) {
			this->release(); 
			if (p_source.is_valid()) {
				p_source.m_container->add_ref();
				m_container = p_source.m_container;
				m_ptr = p_source.m_ptr;
			}
			return *this;
		}

		template<typename t_object_cast>
		operator rcptr_t<t_object_cast>() const {
			rcptr_t<t_object_cast> temp;
			if (is_valid()) temp.__set_from_cast(m_container,m_ptr);
			return temp;
		}

		template<typename t_object_cast>
		rcptr_t<t_object_cast> static_cast_t() const {
			rcptr_t<t_object_cast> temp;
			if (is_valid()) temp.__set_from_cast(m_container,static_cast<t_object_cast*>(m_ptr));
			return temp;
		}

		void new_t() {
			on_new(new t_container_impl());
		}

		template<typename t_param1>
		void new_t(t_param1 const & p_param1) {
			on_new(new t_container_impl(p_param1));
		}

		template<typename t_param1,typename t_param2>
		void new_t(t_param1 const & p_param1, t_param2 const & p_param2) {
			on_new(new t_container_impl(p_param1,p_param2));
		}

		template<typename t_param1,typename t_param2,typename t_param3>
		void new_t(t_param1 const & p_param1, t_param2 const & p_param2,t_param3 const & p_param3) {
			on_new(new t_container_impl(p_param1,p_param2,p_param3));
		}

		template<typename t_param1,typename t_param2,typename t_param3,typename t_param4>
		void new_t(t_param1 const & p_param1, t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4) {
			on_new(new t_container_impl(p_param1,p_param2,p_param3,p_param4));
		}
		
		template<typename t_param1,typename t_param2,typename t_param3,typename t_param4,typename t_param5>
		void new_t(t_param1 const & p_param1, t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4,t_param5 const & p_param5) {
			on_new(new t_container_impl(p_param1,p_param2,p_param3,p_param4,p_param5));
		}
		
		template<typename t_param1,typename t_param2,typename t_param3,typename t_param4,typename t_param5,typename t_param6>
		void new_t(t_param1 const & p_param1, t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4,t_param5 const & p_param5,t_param6 const & p_param6) {
			on_new(new t_container_impl(p_param1,p_param2,p_param3,p_param4,p_param5,p_param6));
		}

		static t_self g_new_t() {
			t_self temp;
			temp.new_t();
			return temp;
		}

		template<typename t_param1>
		static t_self g_new_t(t_param1 const & p_param1) {
			t_self temp;
			temp.new_t<t_param1>(p_param1);
			return temp;
		}

		template<typename t_param1,typename t_param2>
		static t_self g_new_t(t_param1 const & p_param1,t_param2 const & p_param2) {
			t_self temp;
			temp.new_t<t_param1,t_param2>(p_param1,p_param2);
			return temp;
		}

		template<typename t_param1,typename t_param2,typename t_param3>
		static t_self g_new_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3) {
			t_self temp;
			temp.new_t<t_param1,t_param2,t_param3>(p_param1,p_param2,p_param3);
			return temp;
		}

		template<typename t_param1,typename t_param2,typename t_param3,typename t_param4>
		static t_self g_new_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4) {
			t_self temp;
			temp.new_t<t_param1,t_param2,t_param3,t_param4>(p_param1,p_param2,p_param3,p_param4);
			return temp;
		}

		template<typename t_param1,typename t_param2,typename t_param3,typename t_param4,typename t_param5>
		static t_self g_new_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4,t_param5 const & p_param5) {
			t_self temp;
			temp.new_t<t_param1,t_param2,t_param3,t_param4>(p_param1,p_param2,p_param3,p_param4,p_param5);
			return temp;
		}

		t_object & operator*() const {return *m_ptr;}

		t_object * operator->() const {return m_ptr;}

	private:
		void on_new(t_container_impl * p_container) {
			this->release();
			p_container->add_ref();
			m_ptr = &p_container->m_object;
			m_container = p_container;
		}
	};

	template<typename t_object>
	rcptr_t<t_object> rcnew_t() {
		rcptr_t<t_object> temp;
		temp.new_t();
		return temp;		
	}

	template<typename t_object,typename t_param1>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1>(p_param1);
		return temp;		
	}

	template<typename t_object,typename t_param1,typename t_param2>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1,t_param2 const & p_param2) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1,t_param2>(p_param1,p_param2);
		return temp;		
	}

	template<typename t_object,typename t_param1,typename t_param2,typename t_param3>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1,t_param2,t_param3>(p_param1,p_param2,p_param3);
		return temp;		
	}

	template<typename t_object,typename t_param1,typename t_param2,typename t_param3,typename t_param4>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1,t_param2,t_param3,t_param4>(p_param1,p_param2,p_param3,p_param4);
		return temp;		
	}

	template<typename t_object,typename t_param1,typename t_param2,typename t_param3,typename t_param4,typename t_param5>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4,t_param5 const & p_param5) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1,t_param2,t_param3,t_param4>(p_param1,p_param2,p_param3,p_param4,p_param5);
		return temp;
	}

	template<typename t_object,typename t_param1,typename t_param2,typename t_param3,typename t_param4,typename t_param5,typename t_param6>
	rcptr_t<t_object> rcnew_t(t_param1 const & p_param1,t_param2 const & p_param2,t_param3 const & p_param3,t_param4 const & p_param4,t_param5 const & p_param5,t_param6 const & p_param6) {
		rcptr_t<t_object> temp;
		temp.new_t<t_param1,t_param2,t_param3,t_param4>(p_param1,p_param2,p_param3,p_param4,p_param5,p_param6);
		return temp;
	}

	class traits_rcptr : public traits_default {
	public:
		enum { realloc_safe = true, constructor_may_fail = false };
	};

	template<typename T> class traits_t<rcptr_const_t<T> > : public traits_rcptr {};
	template<typename T> class traits_t<rcptr_t<T> > : public traits_rcptr {};
}