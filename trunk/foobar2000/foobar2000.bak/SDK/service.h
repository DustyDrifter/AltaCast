#ifndef _SERVICE_H_
#define _SERVICE_H_

typedef const void* service_class_ref;

PFC_DECLARE_EXCEPTION(exception_service_not_found,pfc::exception,"Service not found");
PFC_DECLARE_EXCEPTION(exception_service_extension_not_found,pfc::exception,"Service extension not found");
PFC_DECLARE_EXCEPTION(exception_service_duplicated,pfc::exception,"Service duplicated");

#ifdef _MSC_VER
#define FOOGUIDDECL __declspec(selectany)
#else
#define FOOGUIDDECL
#endif


#define DECLARE_GUID(NAME,A,S,D,F,G,H,J,K,L,Z,X) FOOGUIDDECL const GUID NAME = {A,S,D,{F,G,H,J,K,L,Z,X}};
#define DECLARE_CLASS_GUID(NAME,A,S,D,F,G,H,J,K,L,Z,X) FOOGUIDDECL const GUID NAME::class_guid = {A,S,D,{F,G,H,J,K,L,Z,X}};

//! Special hack to ensure errors when someone tries to ->service_add_ref()/->service_release() on a service_ptr_t
template<typename T> class service_obscure_refcounting : public T {
private:
	int service_add_ref() throw();
	int service_release() throw();
	void service_release_safe() throw();
};

//! Converts a service interface pointer to a pointer that obscures service counter functionality.
template<typename T> static inline service_obscure_refcounting<T>* service_obscure_refcounting_cast(T * p_source) throw() {return static_cast<service_obscure_refcounting<T>*>(p_source);}

//! Autopointer class to be used with all services. Manages reference counter calls behind-the-scenes.
template<typename T>
class service_ptr_t {
private:
	typedef service_ptr_t<T> t_self;
public:
	inline service_ptr_t() throw() : m_ptr(NULL) {}
	inline service_ptr_t(T* p_ptr) throw() : m_ptr(NULL) {copy(p_ptr);}
	inline service_ptr_t(const t_self & p_source) throw() : m_ptr(NULL) {copy(p_source);}

	template<typename t_source>
	inline service_ptr_t(t_source * p_ptr) throw() : m_ptr(NULL) {copy(p_ptr);}

	template<typename t_source>
	inline service_ptr_t(const service_ptr_t<t_source> & p_source) throw() : m_ptr(NULL) {copy(p_source);}

	inline ~service_ptr_t() throw() {m_ptr->service_release_safe();}
	
	template<typename t_source>
	inline void copy(t_source * p_ptr) throw() {
		m_ptr->service_release_safe();
		m_ptr = p_ptr;
		m_ptr->service_add_ref_safe();
	}

	template<typename t_source>
	inline void copy(const service_ptr_t<t_source> & p_source) throw() {copy(p_source.get_ptr());}


	inline const t_self & operator=(const t_self & p_source) throw() {copy(p_source); return *this;}
	inline const t_self & operator=(T * p_ptr) throw() {copy(p_ptr); return *this;}

	template<typename t_source> inline t_self & operator=(const service_ptr_t<t_source> & p_source) throw() {copy(p_source); return *this;}
	template<typename t_source> inline t_self & operator=(t_source * p_ptr) throw() {copy(p_ptr); return *this;}
	
	inline void release() throw() {
		m_ptr->service_release_safe();
		m_ptr = NULL;
	}


	inline service_obscure_refcounting<T>* operator->() const throw() {PFC_ASSERT(m_ptr != NULL);return service_obscure_refcounting_cast(m_ptr);}

	inline T* get_ptr() const throw() {return m_ptr;}
	
	inline bool is_valid() const throw() {return m_ptr != NULL;}
	inline bool is_empty() const throw() {return m_ptr == NULL;}

	inline bool operator==(const t_self & p_item) const throw() {return m_ptr == p_item.get_ptr();}
	inline bool operator!=(const t_self & p_item) const throw() {return m_ptr != p_item.get_ptr();}
	inline bool operator>(const t_self & p_item) const throw() {return m_ptr > p_item.get_ptr();}
	inline bool operator<(const t_self & p_item) const throw() {return m_ptr < p_item.get_ptr();}

	template<typename t_other>
	inline t_self & operator<<(service_ptr_t<t_other> & p_source) throw() {__unsafe_set(p_source.__unsafe_detach());return *this;}
	template<typename t_other>
	inline t_self & operator>>(service_ptr_t<t_other> & p_dest) throw() {p_dest.__unsafe_set(__unsafe_detach());return *this;}


	inline T* __unsafe_duplicate() const throw() {//should not be used ! temporary !
		m_ptr->service_add_ref_safe();
		return m_ptr;
	}

	inline T* __unsafe_detach() throw() {//should not be used ! temporary !
		return pfc::replace_t(m_ptr,(T*)NULL);
	}

	inline void __unsafe_set(T * p_ptr) throw() {//should not be used ! temporary !
		m_ptr->service_release_safe();
		m_ptr = p_ptr;
	}
private:
	T* m_ptr;
};

namespace pfc {
	template<typename T>
	class traits_t<service_ptr_t<T> > : public traits_default {
	public:
		enum { realloc_safe = true, constructor_may_fail = false};
	};
}


template<typename T, template<typename> class t_alloc = pfc::alloc_fast>
class service_list_t : public pfc::list_t<service_ptr_t<T>, t_alloc >
{
};

//! Helper macro for use when defining a service class. Generates standard features of a service, without ability to register using service_factory / enumerate using service_enum_t. \n
//! This is used for declaring services that are meant to be instantiated by means other than service_enum_t (or non-entrypoint services), or extensions of services (including extension of entrypoint services).	\n
//! Sample non-entrypoint declaration: class myclass : public service_base {...; FB2K_MAKE_SERVICE_INTERFACE(myclass, service_base); };	\n
//! Sample extension declaration: class myclass : public myotherclass {...; FB2K_MAKE_SERVICE_INTERFACE(myclass, myotherclass); };	\n
//! This macro is intended for use ONLY WITH INTERFACE CLASSES, not with implementation classes.
#define FB2K_MAKE_SERVICE_INTERFACE(THISCLASS,PARENTCLASS) \
	public:	\
		typedef THISCLASS t_interface;	\
		typedef PARENTCLASS t_interface_parent;	\
			\
		static const GUID class_guid;	\
			\
		virtual bool service_query(service_ptr_t<service_base> & p_out,const GUID & p_guid) {	\
			if (p_guid == class_guid) {p_out = this; return true;}	\
			else return PARENTCLASS::service_query(p_out,p_guid);	\
		}	\
	protected:	\
		THISCLASS() {}	\
		~THISCLASS() {}	\
	private:	\
		const THISCLASS & operator=(const THISCLASS &) {throw pfc::exception_not_implemented();}	\
		THISCLASS(const THISCLASS &) {throw pfc::exception_not_implemented();}	\
	private:	\
		void __private__service_declaration_selftest() {	\
			pfc::assert_same_type<PARENTCLASS,PARENTCLASS::t_interface>(); /*parentclass must be an interface*/	\
			__validate_service_class_helper<THISCLASS>(); /*service_base must be reachable by walking t_interface_parent*/	\
			pfc::safe_cast<service_base*>(this); /*this class must derive from service_base, directly or indirectly, and be implictly castable to it*/ \
		}

//! Helper macro for use when defining an entrypoint service class. Generates standard features of a service, including ability to register using service_factory and enumerate using service_enum.	\n
//! Sample declaration: class myclass : public service_base {...; FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(myclass); };	\n
//! Note that entrypoint service classes must directly derive from service_base, and not from another service class.
//! This macro is intended for use ONLY WITH INTERFACE CLASSES, not with implementation classes.
#define FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(THISCLASS)	\
	public:	\
		typedef THISCLASS t_interface_entrypoint;	\
	FB2K_MAKE_SERVICE_INTERFACE(THISCLASS,service_base)
		
	

		

//! Base class for all service classes.\n
//! Provides interfaces for reference counter and querying for different interfaces supported by the object.\n
class NOVTABLE service_base
{
public:	
	//! Decrements reference count; deletes the object if reference count reaches zero. This is normally not called directly but managed by service_ptr_t<> template.
	//! @returns New reference count. For debug purposes only, in certain conditions return values may be unreliable.
	virtual int service_release() throw() = 0;
	//! Increments reference count. This is normally not called directly but managed by service_ptr_t<> template.
	//! @returns New reference count. For debug purposes only, in certain conditions return values may be unreliable.
	virtual int service_add_ref() throw() = 0;
	//! Queries whether the object supports specific interface and retrieves a pointer to that interface. This is normally not called directly but managed by service_query_t<> function template.
	//! Typical implementation checks the parameter against GUIDs of interfaces supported by this object, if the GUID is one of supported interfaces, p_out is set to service_base pointer that can be static_cast<>'ed to queried interface and the method returns true; otherwise the method returns false.
	virtual bool service_query(service_ptr_t<service_base> & p_out,const GUID & p_guid) {return false;}

	//! Queries whether the object supports specific interface and retrieves a pointer to that interface.
	//! @param p_out Receives pointer to queried interface on success.
	//! returns true on success, false on failure (interface not supported by the object).
	template<class T>
	bool service_query_t(service_ptr_t<T> & p_out)
	{
		pfc::assert_same_type<T,T::t_interface>();
		service_ptr_t<service_base> temp;
		if (!service_query(temp,T::class_guid)) return false;
		p_out.__unsafe_set(static_cast<T*>(temp.__unsafe_detach()));
		return true;
	}

	//! This is safe to call on null this pointer. Calls service_release when this pointer is not null.
	void service_release_safe() throw();
	//! This is safe to call on null this pointer. Calls service_add_ref when this pointer is not null.
	void service_add_ref_safe() throw();

	typedef service_base t_interface;
	
protected:
	service_base() {}
	~service_base() {}
private:
	service_base(const service_base&) {throw pfc::exception_not_implemented();}
	const service_base & operator=(const service_base&) {throw pfc::exception_not_implemented();}
};

template<typename T>
static void __validate_service_class_helper() {
	__validate_service_class_helper<T::t_interface_parent>();
}

template<>
static void __validate_service_class_helper<service_base>() {}


#include "service_impl.h"

class NOVTABLE service_factory_base {
protected:
	inline service_factory_base(const GUID & p_guid) : m_guid(p_guid) {PFC_ASSERT(!core_api::are_services_available());__internal__next=__internal__list;__internal__list=this;}
	inline ~service_factory_base() {PFC_ASSERT(!core_api::are_services_available());}
public:
	inline const GUID & get_class_guid() const {return m_guid;}

	static service_class_ref enum_find_class(const GUID & p_guid);
	static bool enum_create(service_ptr_t<service_base> & p_out,service_class_ref p_class,t_size p_index);
	static t_size enum_get_count(service_class_ref p_class);

	inline static bool is_service_present(const GUID & g) {return enum_get_count(enum_find_class(g))>0;}

	//! Throws std::bad_alloc or another exception on failure.
	virtual void instance_create(service_ptr_t<service_base> & p_out) = 0;

	//! FOR INTERNAL USE ONLY
	static service_factory_base *__internal__list;
	//! FOR INTERNAL USE ONLY
	service_factory_base * __internal__next;
private:
	const GUID & m_guid;
};


template<typename B>
class service_factory_base_t : public service_factory_base {
public:
	service_factory_base_t() : service_factory_base(B::class_guid) {
		pfc::assert_same_type<B,B::t_interface_entrypoint>();
	}

};




template<class T> static bool service_enum_create_t(service_ptr_t<T> & p_out,t_size p_index) {
	pfc::assert_same_type<T,T::t_interface_entrypoint>();
	service_ptr_t<service_base> ptr;
	if (service_factory_base::enum_create(ptr,service_factory_base::enum_find_class(T::class_guid),p_index)) {
		p_out = static_cast<T*>(ptr.get_ptr());
		return true;
	} else {
		p_out.release();
		return false;
	}
}

template<typename T> class service_class_helper_t {
public:
	service_class_helper_t() : m_class(service_factory_base::enum_find_class(T::class_guid)) {
		pfc::assert_same_type<T,T::t_interface_entrypoint>();
	}
	t_size get_count() const {
		return service_factory_base::enum_get_count(m_class);
	}

	bool create(service_ptr_t<T> & p_out,t_size p_index) const {
		service_ptr_t<service_base> temp;
		if (!service_factory_base::enum_create(temp,m_class,p_index)) return false;
		p_out.__unsafe_set(static_cast<T*>(temp.__unsafe_detach()));
		return true;
	}

	service_ptr_t<T> create(t_size p_index) const {
		service_ptr_t<T> temp;
		if (!create(temp,p_index)) throw pfc::exception_bug_check();
		return temp;
	}
private:
	service_class_ref m_class;
};

template<typename T> static service_ptr_t<T> standard_api_create_t() {
	service_class_helper_t<T> helper;
	switch(helper.get_count()) {
	case 0:
		throw exception_service_not_found();
	case 1:
		return helper.create(0);
	default:
		throw exception_service_duplicated();
	}
}

//! Helper template used to easily access core services. \n
//! Usage: static_api_ptr_t<myclass> api; api->dosomething();
//! Can be used at any point of code, WITH EXCEPTION of static objects that are initialized during DLL load before service system is initialized. That includes static static_api_ptr_t objects, as well as having static_api_ptr_t as members of statically created objects.
//! Throws exception_service_not_found if service could not be reached (which can be ignored for core APIs that are always present unless there is some kind of bug in the code).
template<typename T>
class static_api_ptr_t {
public:
	static_api_ptr_t() : m_ptr(standard_api_create_t<T>()) {}
	service_obscure_refcounting<T>* operator->() const {return service_obscure_refcounting_cast(m_ptr.get_ptr());}
	T* get_ptr() const {return m_ptr.get_ptr();}
private:
	service_ptr_t<T> m_ptr;
};

//! Helper template used to easily access extensions of core services; works in similar way to static_api_ptr_t<>, but queries for specific extension of the interface.\n
//! Usage: static_api_ptr_ex_t<myclass,myclass_extension> api; api->dosomething();
//! Can be used at any point of code, WITH EXCEPTION of static objects that are initialized during DLL load before service system is initialized. That includes static static_api_ptr_ex_t objects, as well as having static_api_ptr_ex_t as members of statically created objects.
//! Throws exception_service_not_found if service could not be reached (which can be ignored for core APIs that are always present unless there is some kind of bug in the code).
template<typename t_extension>
class static_api_ptr_ex_t {
public:
	static_api_ptr_ex_t() {
		if (!static_api_ptr_t<t_extension::t_interface_entrypoint>()->service_query_t(m_ptr)) throw exception_service_extension_not_found();
	}

	service_obscure_refcounting<t_extension>* operator->() const {return service_obscure_refcounting_cast(m_ptr.get_ptr());}
	t_extension* get_ptr() const {return m_ptr.get_ptr();}
private:
	service_ptr_t<t_extension> m_ptr;
};


//! Helper; simulates array with instance of each available implementation of given service class.
template<typename T> class service_instance_array_t {
public:
	typedef service_ptr_t<T> t_ptr;
	service_instance_array_t() {
		service_class_helper_t<T> helper;
		const t_size count = helper.get_count();
		m_data.set_size(count);
		for(t_size n=0;n<count;n++) m_data[n] = helper.create(n);
	}

	t_size get_size() const {return m_data.get_size();}
	const t_ptr & operator[](t_size p_index) const {return m_data[p_index];}

	//nonconst version to allow sorting/bsearching; do not abuse
	t_ptr & operator[](t_size p_index) {return m_data[p_index];}
private:
	pfc::array_t<t_ptr> m_data;
};

template<typename t_interface>
class service_enum_t {
public:
	service_enum_t() : m_index(0) {}
	void reset() {m_index = 0;}
	bool first(service_ptr_t<t_interface> & p_out) {
		reset();
		return next(p_out);
	}
	bool next(service_ptr_t<t_interface> & p_out) {
		return m_helper.create(p_out,m_index++);
	}
private:
	unsigned m_index;
	service_class_helper_t<t_interface> m_helper;
};


template<typename T>
class service_factory_t : public service_factory_base_t<typename T::t_interface_entrypoint> {
public:
	void instance_create(service_ptr_t<service_base> & p_out) {
		p_out = pfc::safe_cast<service_base*>(pfc::safe_cast<typename T::t_interface_entrypoint*>(pfc::safe_cast<T*>(  new service_impl_t<T>  )));
	}
};

template<typename T>
class service_factory_single_t : public service_factory_base_t<typename T::t_interface_entrypoint> {
	service_impl_single_t<T> g_instance;
public:
	TEMPLATE_CONSTRUCTOR_FORWARD_FLOOD(service_factory_single_t,g_instance)

	void instance_create(service_ptr_t<service_base> & p_out) {
		p_out = pfc::safe_cast<service_base*>(pfc::safe_cast<typename T::t_interface_entrypoint*>(pfc::safe_cast<T*>(&g_instance)));
	}

	inline T& get_static_instance() {return g_instance;}
};

template<typename T>
class service_factory_single_ref_t : public service_factory_base_t<typename T::t_interface_entrypoint>
{
private:
	T & instance;
public:
	service_factory_single_ref_t(T& param) : instance(param) {}

	void instance_create(service_ptr_t<service_base> & p_out) {
		p_out = pfc::safe_cast<service_base*>(pfc::safe_cast<typename T::t_interface_entrypoint*>(pfc::safe_cast<T*>(&instance)));
	}

	inline T& get_static_instance() {return instance;}
};


template<typename T>
class service_factory_single_transparent_t : public service_factory_base_t<typename T::t_interface_entrypoint>, public service_impl_single_t<T>
{	
public:
	TEMPLATE_CONSTRUCTOR_FORWARD_FLOOD(service_factory_single_transparent_t,service_impl_single_t<T>)

	void instance_create(service_ptr_t<service_base> & p_out) {
		p_out = pfc::safe_cast<service_base*>(pfc::safe_cast<typename T::t_interface_entrypoint*>(pfc::safe_cast<T*>(this)));
	}

	inline T& get_static_instance() {return *(T*)this;}
};

#endif