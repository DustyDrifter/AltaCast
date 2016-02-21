namespace pfc {

	class NOVTABLE refcounted_object_root
	{
	public:
		void refcount_add_ref() {++m_counter;}
		void refcount_release() {if (--m_counter == 0) delete this;}
	protected:
		refcounted_object_root() {}
		virtual ~refcounted_object_root() {}
	private:
		refcounter m_counter;
	};

	template<typename T>
	class refcounted_object_ptr_t {
	private:
		typedef refcounted_object_ptr_t<T> t_self;
	public:
		inline refcounted_object_ptr_t() : m_ptr(NULL) {}
		inline refcounted_object_ptr_t(T* p_ptr) : m_ptr(NULL) {copy(p_ptr);}
		inline refcounted_object_ptr_t(const t_self & p_source) : m_ptr(NULL) {copy(p_source);}

		template<typename t_source>
		inline refcounted_object_ptr_t(t_source * p_ptr) : m_ptr(NULL) {copy(p_ptr);}

		template<typename t_source>
		inline refcounted_object_ptr_t(const refcounted_object_ptr_t<t_source> & p_source) : m_ptr(NULL) {copy(p_source);}

		inline ~refcounted_object_ptr_t() {if (m_ptr != NULL) m_ptr->refcount_release();}
		
		template<typename t_source>
		inline void copy(t_source * p_ptr) {
			T* torel = pfc::replace_t(m_ptr,p_ptr);
			if (m_ptr != NULL) m_ptr->refcount_add_ref();
			if (torel != NULL) torel->refcount_release();
			
		}

		template<typename t_source>
		inline void copy(const refcounted_object_ptr_t<t_source> & p_source) {copy(p_source.get_ptr());}


		inline const t_self & operator=(const t_self & p_source) {copy(p_source); return *this;}
		inline const t_self & operator=(T * p_ptr) {copy(p_ptr); return *this;}

		template<typename t_source> inline t_self & operator=(const refcounted_object_ptr_t<t_source> & p_source) {copy(p_source); return *this;}
		template<typename t_source> inline t_self & operator=(t_source * p_ptr) {copy(p_ptr); return *this;}
		
		inline void release() {
			T * temp = pfc::replace_t(m_ptr,(T*)NULL);
			if (temp != NULL) temp->refcount_release();
		}


		inline T* operator->() const {PFC_ASSERT(m_ptr != NULL);return m_ptr;}

		inline T* get_ptr() const {return m_ptr;}
		
		inline bool is_valid() const {return m_ptr != NULL;}
		inline bool is_empty() const {return m_ptr == NULL;}

		inline bool operator==(const t_self & p_item) const {return m_ptr == p_item.get_ptr();}
		inline bool operator!=(const t_self & p_item) const {return m_ptr != p_item.get_ptr();}
		inline bool operator>(const t_self & p_item) const {return m_ptr > p_item.get_ptr();}
		inline bool operator<(const t_self & p_item) const {return m_ptr < p_item.get_ptr();}


		inline T* __unsafe_duplicate() const//should not be used ! temporary !
		{
			if (m_ptr) m_ptr->refcount_add_ref();
			return m_ptr;
		}

		inline T* __unsafe_detach() {
			T* ret = m_ptr;
			m_ptr = 0;
			return ret;
		}

		inline void __unsafe_set(T * p_ptr) {//should not be used ! temporary !
			release();
			m_ptr = p_ptr;
		}
	private:
		T* m_ptr;
	};

	template<typename T>
	class traits_t<refcounted_object_ptr_t<T> > : public traits_default {
	public:
		enum { realloc_safe = true, constructor_may_fail = false};
	};

};