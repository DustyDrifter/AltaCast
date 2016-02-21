namespace pfc {


//this is windows-only
//update me to new conventions
template<class T>
class com_ptr_t
{
public:
	inline com_ptr_t() : m_ptr(0) {}
	inline com_ptr_t(T* p_ptr) : m_ptr(p_ptr) {if (m_ptr) m_ptr->AddRef();}
	inline com_ptr_t(const com_ptr_t<T> & p_source) : m_ptr(p_source.m_ptr) {if (m_ptr) m_ptr->AddRef();}

	inline ~com_ptr_t() {if (m_ptr) m_ptr->Release();}
	
	inline void copy(T * p_ptr)
	{
		if (m_ptr) m_ptr->Release();
		m_ptr = p_ptr;
		if (m_ptr) m_ptr->AddRef();
	}

	inline void copy(const com_ptr_t<T> & p_source) {copy(p_source.m_ptr);}

	inline void set(T * p_ptr)//should not be used ! temporary !
	{
		if (m_ptr) m_ptr->Release();
		m_ptr = p_ptr;
	}

	inline const com_ptr_t<T> & operator=(const com_ptr_t<T> & p_source) {copy(p_source); return *this;}
	inline const com_ptr_t<T> & operator=(T * p_ptr) {copy(p_ptr); return *this;}

	inline void release()
	{
		if (m_ptr) m_ptr->Release();
		m_ptr = 0;
	}


	inline T* operator->() const {assert(m_ptr);return m_ptr;}

	inline T* get_ptr() const {return m_ptr;}
	
	inline T* duplicate_ptr() const//should not be used ! temporary !
	{
		if (m_ptr) m_ptr->AddRef();
		return m_ptr;
	}

	inline T* duplicate_ptr_release() {
		T* ret = m_ptr;
		m_ptr = 0;
		return ret;
	}

	inline bool is_valid() const {return m_ptr != 0;}
	inline bool is_empty() const {return m_ptr == 0;}

	inline bool operator==(const com_ptr_t<T> & p_item) const {return m_ptr == p_item.m_ptr;}
	inline bool operator!=(const com_ptr_t<T> & p_item) const {return m_ptr != p_item.m_ptr;}
	inline bool operator>(const com_ptr_t<T> & p_item) const {return m_ptr > p_item.m_ptr;}
	inline bool operator<(const com_ptr_t<T> & p_item) const {return m_ptr < p_item.m_ptr;}

	inline static void g_swap(com_ptr_t<T> & item1, com_ptr_t<T> & item2) {
		pfc::swap_t(item1.m_ptr,item2.m_ptr);
	}

	inline T** receive_ptr() {release();return &m_ptr;}

private:
	T* m_ptr;
};

}
