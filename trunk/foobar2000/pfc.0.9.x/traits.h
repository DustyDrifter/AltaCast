namespace pfc {

	class traits_default {
	public:
		enum { realloc_safe = false, needs_destructor = true, needs_constructor = true, constructor_may_fail = true};
	};

	class traits_rawobject : public traits_default {
	public:
		enum { realloc_safe = true, needs_destructor = false, needs_constructor = false, constructor_may_fail = false};
	};

	template<typename T> class traits_t : public traits_default {};
	
	template<typename T> class traits_t<T*> : public traits_rawobject {};

	template<> class traits_t<char> : public traits_rawobject {};
	template<> class traits_t<unsigned char> : public traits_rawobject {};
	template<> class traits_t<wchar_t> : public traits_rawobject {};
	template<> class traits_t<short> : public traits_rawobject {};
	template<> class traits_t<unsigned short> : public traits_rawobject {};
	template<> class traits_t<int> : public traits_rawobject {};
	template<> class traits_t<unsigned int> : public traits_rawobject {};
	template<> class traits_t<long> : public traits_rawobject {};
	template<> class traits_t<unsigned long> : public traits_rawobject {};
	template<> class traits_t<long long> : public traits_rawobject {};
	template<> class traits_t<unsigned long long> : public traits_rawobject {};
	template<> class traits_t<bool> : public traits_rawobject {};

	template<> class traits_t<float> : public traits_rawobject {};
	template<> class traits_t<double> : public traits_rawobject {};
	
	template<> class traits_t<GUID> : public traits_rawobject {};

	template<typename T,t_size p_count>
	class traits_t<T[p_count]> : public traits_t<T> {};
}