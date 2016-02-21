class order_helper
{
	pfc::array_t<t_size> m_data;
public:
	order_helper(t_size p_size) {
		m_data.set_size(p_size);
		for(t_size n=0;n<p_size;n++) m_data[n]=n;
	}

	order_helper(const order_helper & p_order) {*this = p_order;}


	template<typename t_int>
	static void g_fill(t_int * p_order,const t_size p_count) {
		t_size n; for(n=0;n<p_count;n++) p_order[n] = (t_int)n;
	}

	template<typename t_array>
	static void g_fill(t_array & p_array) {
		t_size n; const t_size max = pfc::array_size_t(p_array);
		for(n=0;n<max;n++) p_array[n] = n;
	}


	t_size get_item(t_size ptr) const {return m_data[ptr];}

	t_size & operator[](t_size ptr) {return m_data[ptr];}
	t_size operator[](t_size ptr) const {return m_data[ptr];}

	static void g_swap(t_size * p_data,t_size ptr1,t_size ptr2);
	inline void swap(t_size ptr1,t_size ptr2) {pfc::swap_t(m_data[ptr1],m_data[ptr2]);}

	const t_size * get_ptr() const {return m_data.get_ptr();}

	static t_size g_find_reverse(const t_size * order,t_size val);
	inline t_size find_reverse(t_size val) {return g_find_reverse(m_data.get_ptr(),val);}

	static void g_reverse(t_size * order,t_size base,t_size count);
	inline void reverse(t_size base,t_size count) {g_reverse(m_data.get_ptr(),base,count);}

	t_size get_count() const {return m_data.get_size();}
};
