#ifndef _PFC_OTHER_H_
#define _PFC_OTHER_H_

namespace pfc {
	template<class T>
	class vartoggle_t {
		T oldval; T & var;
	public:
		vartoggle_t(T & p_var,const T & val) : var(p_var) {
			oldval = var;
			var = val;
		}
		~vartoggle_t() {var = oldval;}
	};

	typedef vartoggle_t<bool> booltoggle;
};

#ifdef _MSC_VER

class fpu_control
{
	unsigned old_val;
	unsigned mask;
public:
	inline fpu_control(unsigned p_mask,unsigned p_val)
	{
		mask = p_mask;
		old_val = _controlfp(p_val,mask);
	}
	inline ~fpu_control()
	{
		_controlfp(old_val,mask);
	}
};

class fpu_control_roundnearest : private fpu_control
{
public:
	fpu_control_roundnearest() : fpu_control(_MCW_RC,_RC_NEAR) {}
};

class fpu_control_flushdenormal : private fpu_control
{
public:
	fpu_control_flushdenormal() : fpu_control(_MCW_DN,_DN_FLUSH) {}
};

class fpu_control_default : private fpu_control
{
public:
	fpu_control_default() : fpu_control(_MCW_DN|_MCW_RC,_DN_FLUSH|_RC_NEAR) {}
};

#ifdef _M_IX86
class sse_control {
public:
	sse_control(unsigned p_mask,unsigned p_val) : m_mask(p_mask) {
		__control87_2(p_val,p_mask,NULL,&m_oldval);
	}
	~sse_control() {
		__control87_2(m_oldval,m_mask,NULL,&m_oldval);
	}
private:
	unsigned m_mask,m_oldval;
};
class sse_control_flushdenormal : private sse_control {
public:
	sse_control_flushdenormal() : sse_control(_MCW_DN,_DN_FLUSH) {}
};
#endif

#endif

namespace pfc {
	class refcounter {
	public:
		refcounter(long p_val = 0) : m_val(p_val) {}
		long operator++() {return InterlockedIncrement(&m_val);}
		long operator--() {return InterlockedDecrement(&m_val);}
	private:
		long m_val;
	};

	//! Assumes t_freefunc to never throw exceptions.
	template<typename T,void t_freefunc(T*) = pfc::delete_t<T> >
	class ptrholder_t {
	private:
		typedef ptrholder_t<T,t_freefunc> t_self;
	public:
		inline ptrholder_t(T* p_ptr) : m_ptr(p_ptr) {}
		inline ptrholder_t() : m_ptr(NULL) {}
		inline ~ptrholder_t() {if (m_ptr != NULL) t_freefunc(m_ptr);}
		inline bool is_valid() const {return m_ptr != NULL;}
		inline bool is_empty() const {return m_ptr == NULL;}
		inline T* operator->() const {return m_ptr;}
		inline T* get_ptr() const {return m_ptr;}
		inline void release() {if (m_ptr) t_freefunc(m_ptr); m_ptr = 0;}
		inline void set(T * p_ptr) {if (m_ptr) t_freefunc(m_ptr); m_ptr = p_ptr;}
		inline const ptrholder_t<T,t_freefunc> & operator=(T * p_ptr) {set(p_ptr);return *this;}
		inline T* detach() {return pfc::replace_t(m_ptr,(T*)NULL);}
		inline T& operator*() {return *m_ptr;}
		inline const T& operator*() const {return *m_ptr;}

		inline t_self & operator<<(t_self & p_source) {set(p_source.detach());return *this;}
		inline t_self & operator>>(t_self & p_dest) {p_dest.set(detach());return *this;}

	private:
		ptrholder_t(const t_self &) {throw pfc::exception_not_implemented();}
		const t_self & operator=(const t_self & ) {throw pfc::exception_not_implemented();}

		T* m_ptr;
	};

	//! Assumes t_freefunc to never throw exceptions.
	//! HACK: strip down methods where regular implementation breaks when used with void pointer
	template<void t_freefunc(void*)>
	class ptrholder_t<void,t_freefunc> {
	private:
		typedef void T;
		typedef ptrholder_t<T,t_freefunc> t_self;
	public:
		inline ptrholder_t(T* p_ptr) : m_ptr(p_ptr) {}
		inline ptrholder_t() : m_ptr(NULL) {}
		inline ~ptrholder_t() {if (m_ptr != NULL) t_freefunc(m_ptr);}
		inline bool is_valid() const {return m_ptr != NULL;}
		inline bool is_empty() const {return m_ptr == NULL;}
		inline T* get_ptr() const {return m_ptr;}
		inline void release() {if (m_ptr) t_freefunc(m_ptr); m_ptr = 0;}
		inline void set(T * p_ptr) {if (m_ptr) t_freefunc(m_ptr); m_ptr = p_ptr;}
		inline const ptrholder_t<T,t_freefunc> & operator=(T * p_ptr) {set(p_ptr);return *this;}
		inline T* detach() {return pfc::replace_t(m_ptr,(void*)NULL);}

		inline t_self & operator<<(t_self & p_source) {set(p_source.detach());return *this;}
		inline t_self & operator>>(t_self & p_dest) {p_dest.set(detach());return *this;}

	private:
		ptrholder_t(const t_self &) {throw pfc::exception_not_implemented();}
		t_self & operator=(const t_self & ) {throw pfc::exception_not_implemented();}

		T* m_ptr;
	};


	void crash();
}

#endif