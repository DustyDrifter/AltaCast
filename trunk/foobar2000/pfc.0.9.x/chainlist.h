#ifndef _PFC_CHAINLIST_H_
#define _PFC_CHAINLIST_H_

namespace pfc {

	template<typename T>
	class chain_list_simple_t {
	private:
		typedef chain_list_simple_t<T> t_self;
	public:
		chain_list_simple_t() : m_first(0), m_last(0), m_count(0) {}
		chain_list_simple_t(const t_self & p_source) : m_first(0), m_last(0), m_count(0) {
			t_iter iter;
			for(iter = p_source.first();iter;iter = p_source.next(iter))
				insert_last(p_source.get_item(iter));
		}

		t_self const & operator=(t_self const & p_source) {
			remove_all();
			for(iter = p_source.first();iter;iter = p_source.next(iter))
				insert_last(p_source.get_item(iter));
			return *this;
		}


		typedef void* t_iter;

		inline t_size get_count() const {return m_count;}
		inline t_iter first() const {return reinterpret_cast<t_iter>(m_first);}
		inline t_iter last () const {return reinterpret_cast<t_iter>(m_last );}
		inline t_iter next(t_iter param) {return reinterpret_cast<elem*>(param)->m_next;}
		inline t_iter prev(t_iter param) {return reinterpret_cast<elem*>(param)->m_prev;}
		inline T & get_item(t_iter param) {return reinterpret_cast<elem*>(param)->m_data;}
		inline const T & get_item(t_iter param) const {return reinterpret_cast<elem*>(param)->m_data;}
		inline t_iter insert_first() {return _insert_first(new elem());}
		inline t_iter insert_first(const T& param) {return _insert_first(new elem(param));}
		inline t_iter insert_last() {return _insert_last(new elem());}
		inline t_iter insert_last(const T& param) {return _insert_last(new elem(param));}
		inline t_iter insert_before(t_iter p_iter) {return _insert_before(p_iter,new elem());}
		inline t_iter insert_before(t_iter p_iter,const T& param) {return _insert_before(p_iter,new elem(param));}
		inline t_iter insert_after(t_iter p_iter) {return _insert_after(p_iter,new elem());}
		inline t_iter insert_after(t_iter p_iter,const T& param) {return _insert_after(p_iter,new elem(param));}
		inline void remove(t_iter p_iter) {elem * item = reinterpret_cast<elem*>(p_iter); _unlink(item); delete item;}
		
		inline void unlink(t_iter p_iter) {_unlink(reinterpret_cast<elem*>(p_iter));}
		inline void link_first(t_iter p_iter) {_insert_first(reinterpret_cast<elem*>(p_iter));}
		inline void link_last(t_iter p_iter) {_insert_last(reinterpret_cast<elem*>(p_iter));}
		inline void link_before(t_iter p_next,t_iter p_iter) {_insert_before(p_next,reinterpret_cast<elem*>(p_iter));}
		inline void link_after(t_iter p_prev,t_iter p_iter) {_insert_after(p_prev,reinterpret_cast<elem*>(p_iter));}
		
		void remove_all() {while(m_count > 0) remove(first());}

		~chain_list_simple_t() {remove_all();}
	private:
		struct elem;

		t_iter _insert_first(elem * p_elem)
		{
			p_elem->m_prev = 0;
			p_elem->m_next = m_first;
			(m_first ? m_first->m_prev : m_last) = p_elem;
			m_first = p_elem;
			m_count++;
			return reinterpret_cast<t_iter>(p_elem);
		}

		t_iter _insert_last(elem * p_elem)
		{
			p_elem->m_prev = m_last;
			p_elem->m_next = 0;
			(m_last ? m_last->m_next : m_first) = p_elem;
			m_last = p_elem;
			m_count++;
			return reinterpret_cast<t_iter>(p_elem);
		}
		
		t_iter _insert_before(t_iter p_iter,elem * p_elem)
		{
			elem * p_next = reinterpret_cast<elem*>(p_iter);
			p_elem->m_next = p_next;
			p_elem->m_prev = p_next->m_prev;
			(p_next->m_prev ? p_next->m_prev->m_next : m_first) = p_elem;
			p_next->m_prev = p_elem;			
			m_count++;
			return reinterpret_cast<t_iter>(p_elem);
		}

		t_iter _insert_after(t_iter p_iter,elem * p_elem)
		{
			elem * p_prev = reinterpret_cast<elem*>(p_iter);
			p_elem->m_next = p_prev->m_next;
			p_elem->m_prev = p_prev;
			(p_prev->m_next ? p_prev->m_next->m_prev : m_last) = p_elem;
			p_prev->m_next = p_elem;
			m_count++;
			return reinterpret_cast<t_iter>(p_elem);
		}

		void _unlink(elem * p_elem)
		{
			(p_elem->m_prev ? p_elem->m_prev->m_next : m_first) = p_elem->m_next;
			(p_elem->m_next ? p_elem->m_next->m_prev : m_last) = p_elem->m_prev;
			p_elem->m_next = p_elem->m_prev = 0;
			m_count--;
		}

		struct elem {
			elem() : m_prev(0), m_next(0) {}
			elem(const elem & p_src) {*this = p_src;}
			elem(const T& p_src) : m_data(p_src), m_prev(NULL), m_next(NULL) {}

			T m_data;
			elem * m_prev, * m_next;
		};
		elem * m_first, * m_last;
		t_size m_count;
	};

	template<typename T>
	class chain_list_t
	{
	public:
		class const_iterator;
		class iterator;
		friend class const_iterator;
		friend class iterator;

	private:
		class elem;

		typedef chain_list_t<T> t_self;
	public:

		class const_iterator
		{
		public:
			friend class chain_list_t<T>;

			inline const T & operator*() const {return m_ptr->m_data;}
			inline const T * operator->() const {return &m_ptr->m_data;}
			inline const_iterator() : m_owner(0), m_ptr(0) {}
			inline const_iterator(const const_iterator & p_source) : m_owner(0), m_ptr(0) {*this = p_source;}
			inline ~const_iterator() {if (m_owner) m_owner->_remove_iterator(this);}
			
			inline const_iterator(const chain_list_t<T> * p_owner,elem * p_ptr) : m_owner(p_owner), m_ptr(p_ptr)
			{
				if (m_owner != NULL) m_owner->_add_iterator(this);
			}


			inline const const_iterator & operator=(const const_iterator & p_source)
			{
				if (m_owner != NULL) m_owner->_remove_iterator(this);
				m_owner = p_source.m_owner;
				m_ptr = p_source.m_ptr;
				if (m_owner != NULL) m_owner->_add_iterator(this);
				return *this;
			}

			inline void next() {if (m_ptr != NULL) m_ptr = m_ptr->m_next;}
			inline void prev() {if (m_ptr != NULL) m_ptr = m_ptr->m_prev;}
			
			inline const const_iterator & operator++() {next();return *this;}
			inline const_iterator operator++(int) {const_iterator old = *this; next(); return old;}
			inline const const_iterator & operator--() {prev();return *this;}
			inline const_iterator operator--(int) {const_iterator old = *this; prev(); return old;}

			inline bool is_valid() const {return m_ptr != 0;}

			inline bool operator==(const const_iterator &  p_iter) const {return m_ptr == p_iter.m_ptr;}
			inline bool operator!=(const const_iterator &  p_iter) const {return m_ptr != p_iter.m_ptr;}

		protected:
			const chain_list_t<T> * m_owner;
			elem * m_ptr;

			void orphan() {m_ptr = 0;m_owner=0;}
		};

		class iterator : public const_iterator
		{
		public:
			inline iterator() {}
			inline iterator(const iterator & p_source) : const_iterator(p_source) {}
			inline iterator(const chain_list_t<T> * p_owner,elem * p_ptr) : const_iterator(p_owner,p_ptr) {}
			inline const iterator & operator=(const iterator & p_source) { *(const_iterator*)this = *(const const_iterator*) & p_source; return *this;}
			inline T & operator*() const {return this->m_ptr->m_data;}
			inline T * operator->() const {return &this->m_ptr->m_data;}
			inline bool operator==(const iterator &  p_iter) const {return this->m_ptr == p_iter.m_ptr;}
			inline bool operator!=(const iterator &  p_iter) const {return this->m_ptr != p_iter.m_ptr;}
			inline const iterator & operator++() {next();return *this;}
			inline iterator operator++(int) {iterator old = *this; next(); return old;}
			inline const iterator & operator--() {prev();return *this;}
			inline iterator operator--(int) {iterator old = *this; prev(); return old;}
		};


		inline iterator first() {return iterator(this,m_first);}
		inline iterator last() {return iterator(this,m_last);}

		inline const_iterator first() const {return const_iterator(this,m_first);}
		inline const_iterator last() const {return const_iterator(this,m_last);}

		inline t_size get_count() const {return m_count;}

		inline chain_list_t() {_init();}
		inline chain_list_t(const t_self & p_source) { _init(); *this = p_source; }
		inline ~chain_list_t()
		{
			remove_all();
		}
		
		const t_self & operator=(const t_self & p_source)
		{
			remove_all();
			for(const_iterator iter = p_source.first();iter.is_valid();iter.next())
				insert_last(*iter);
			return *this;
		}

		template<typename t_insert> iterator insert_after(iterator const & p_iter,const t_insert & p_item) {
			bug_check_assert(p_iter.is_valid() && p_iter.m_owner == this);
			elem * ptr = p_iter.m_ptr;
			elem * new_elem = new elem(p_item,0);
			new_elem->m_prev = ptr;
			new_elem->m_next = ptr->m_next;
			(ptr->m_next ? ptr->m_next->m_prev : m_last) = new_elem;
			ptr->m_next = new_elem;
			m_count++;
			return iterator(this,new_elem);
		}

		template<typename t_insert> iterator insert_before(iterator const & p_iter,const t_insert & p_item) {
			bug_check_assert(p_iter.is_valid() && p_iter.m_owner == this);
			elem * ptr = p_iter.m_ptr;
			elem * new_elem = new elem(p_item,0);
			new_elem->m_next = ptr;
			new_elem->m_prev = ptr->m_prev;
			(ptr->m_prev ? ptr->m_prev->m_next : m_first) = new_elem;
			ptr->m_prev = new_elem;
			m_count++;
			return iterator(this,new_elem);
		}

		template<typename t_insert> iterator insert_last(const t_insert & p_item) {
			elem * new_elem = new elem(p_item,0);
			new_elem->m_prev = m_last;
			(m_last ? m_last->m_next : m_first) = new_elem;
			m_last = new_elem;
			m_count++;
			return iterator(this,new_elem);
		}

		template<typename t_insert> iterator insert_first(const t_insert & p_item) {
			elem * new_elem = new elem(p_item,0);
			new_elem->m_next = m_first;
			(m_first ? m_first->m_prev : m_last) = new_elem;
			m_first = new_elem;
			m_count++;
			return iterator(this,new_elem);
		}

		template<typename t_insert> void insert_last_multi(const chain_list_t<t_insert> & p_list) {
			typename chain_list_t<t_insert>::const_iterator iter;
			for(iter = p_list.first(); iter.is_valid(); ++iter) {
				insert_last(*iter);
			}
		}

		template<typename t_insert> void insert_first_multi(const chain_list_t<t_insert> & p_list) {
			typename chain_list_t<t_insert>::const_iterator iter;
			for(iter = p_list.last(); iter.is_valid(); --iter) {
				insert_first(*iter);
			}
		}

		iterator insert_after(iterator const & p_iter);
		iterator insert_before(iterator const & p_iter);
		iterator insert_last();
		iterator insert_first();

		void remove_single(iterator const & p_iter);
		void remove_range(iterator const & p_from,iterator const & p_to);
		void remove_all();

		inline iterator find_forward(iterator const & p_iter,const T & p_item) const {return iterator(this,_find_forward(p_iter.m_ptr,p_item));}
		inline const_iterator find_forward(const_iterator const & p_iter,const T & p_item) const {return const_iterator(this,_find_forward(p_iter.m_ptr,p_item));}
		inline iterator find_back(iterator const & p_iter,const T & p_item) const {return iterator(this,_find_back(p_iter.m_ptr,p_item));}
		inline const_iterator find_back(const_iterator const & p_iter,const T & p_item) const {return const_iterator(this,_find_back(p_iter.m_ptr,p_item));}

		inline iterator find_first(const T & p_item) {return find_forward(first(),p_item);}
		inline const_iterator find_first(const T & p_item) const {return find_forward(first(),p_item);}
		inline iterator find_next(iterator p_iter,const T & p_item) const {p_iter++; return find_forward(p_iter,p_item);}
		inline const_iterator find_next(const_iterator p_iter,const T & p_item) const {p_iter++; return find_forward(p_iter,p_item);}

		inline iterator find_last(const T & p_item) {return find_back(last(),p_item);}
		inline const_iterator find_last(const T & p_item) const {return find_back(last(),p_item);}
		inline iterator find_prev(iterator p_iter,const T & p_item) const {p_iter++; return find_back(p_iter,p_item);}
		inline const_iterator find_prev(const_iterator p_iter,const T & p_item) const {p_iter++; return find_back(p_iter,p_item);}

		inline bool find_first_and_remove(const T & p_item) {
			iterator iter = find_first(p_item);
			if (iter.is_valid()) {remove_single(iter); return true;}
			else return false;
		}

		const_iterator by_index(unsigned p_idx) const
		{
			const_iterator iter = first();
			while(iter.is_valid() && p_idx)
			{
				++iter;
				p_idx--;
			}
			return iter;
		}

		iterator by_index(unsigned p_idx)
		{
			iterator iter = first();
			while(iter.is_valid() && p_idx)
			{
				++iter;
				p_idx--;
			}
			return iter;
		}

	private:
		class elem
		{
		public:
			elem() : m_prev(0), m_next(0) {}
			elem(const elem & p_source) {*this = p_source;}
			template<typename t_param>
			elem(const t_param & p_source,int) : m_data(p_source), m_prev(0), m_next(0) {}

			T m_data;
			elem * m_prev, * m_next;
		};

		elem * m_first, * m_last;

		t_size m_count;

		typedef typename chain_list_simple_t<const_iterator*>::t_iter t_iter_entry;

		mutable chain_list_simple_t<const_iterator*> m_iterators,m_iterators_recycle;
		enum {iterators_recycle_max = 32};

		elem * _find_forward(elem * p_base,const T & p_item) const
		{
			elem * walk = p_base;
			while(walk)
			{
				if (walk->m_data == p_item) return walk;
				walk = walk->m_next;
			}
			return 0;
		}

		elem * _find_back(elem * p_base,const T & p_item) const
		{
			elem * walk = p_base;
			while(walk)
			{
				if (walk->m_data == p_item) return walk;
				walk = walk->m_prev;
			}
			return 0;
		}

		void _init()
		{
			m_first = m_last = 0;
			m_count = 0;
		}

		void _delete_iter_entry(t_iter_entry p_iter) const
		{
			if (m_iterators_recycle.get_count() < iterators_recycle_max)
			{
				m_iterators.unlink(p_iter);
				m_iterators_recycle.link_last(p_iter);
			}
			else
			{
				m_iterators.remove(p_iter);
			}
		}

		void _add_iterator(const_iterator * p_iter) const
		{
			if (m_iterators_recycle.get_count() > 0)
			{
				t_iter_entry temp = m_iterators_recycle.first();
				m_iterators_recycle.unlink(temp);
				m_iterators.link_last(temp);
				m_iterators.get_item(temp) = p_iter;
			}
			else
			{
				m_iterators.insert_last(p_iter);
			}
		}

		void _remove_iterator(const_iterator * p_iter) const
		{
			t_iter_entry entry = m_iterators.first();
			while(entry)
			{
				if (m_iterators.get_item(entry) == p_iter)
				{
					_delete_iter_entry(entry);
					break;
				}
				entry = m_iterators.next(entry);
			}
		}

		void _update_iterators_on_removed(const elem * ptr)
		{
			t_iter_entry entry = m_iterators.first();
			while(entry)
			{
				t_iter_entry entry_next = m_iterators.next(entry);
				const_iterator * iter = m_iterators.get_item(entry);
				if (iter->m_ptr == ptr)
				{
					iter->orphan();
					_delete_iter_entry(entry);
				}
				entry = entry_next;
			}
		}
	};


	template<typename T>
	typename chain_list_t<T>::iterator chain_list_t<T>::insert_after(iterator const & p_iter)
	{
		bug_check_assert(p_iter.is_valid() && p_iter.m_owner == this);
		elem * ptr = p_iter.m_ptr;
		elem * new_elem = new elem();
		new_elem->m_prev = ptr;
		new_elem->m_next = ptr->m_next;
		(ptr->m_next ? ptr->m_next->m_prev : m_last) = new_elem;
		ptr->m_next = new_elem;
		m_count++;

		return iterator(this,new_elem);
	}

	template<typename T>
	typename chain_list_t<T>::iterator chain_list_t<T>::insert_before(iterator const & p_iter)
	{
		bug_check_assert(p_iter.is_valid() && p_iter.m_owner == this);
		elem * ptr = p_iter.m_ptr;
		elem * new_elem = new elem();
		new_elem->m_next = ptr;
		new_elem->m_prev = ptr->m_prev;
		(ptr->m_prev ? ptr->m_prev->m_next : m_first) = new_elem;
		ptr->m_prev = new_elem;
		m_count++;
		
		return iterator(this,new_elem);			
	}

	template<typename T>
	typename chain_list_t<T>::iterator chain_list_t<T>::insert_last()
	{
		elem * new_elem = new elem();
		new_elem->m_prev = m_last;
		(m_last ? m_last->m_next : m_first) = new_elem;
		m_last = new_elem;
		m_count++;
		
		return iterator(this,new_elem);
	}

	template<typename T>
	typename chain_list_t<T>::iterator chain_list_t<T>::insert_first()
	{
		elem * new_elem = new elem();
		new_elem->m_next = m_first;
		(m_first ? m_first->m_prev : m_last) = new_elem;
		m_first = new_elem;
		m_count++;

		return iterator(this,new_elem);
	}


	template<typename T>
	void chain_list_t<T>::remove_single(iterator const & p_iter)
	{
		elem * ptr = p_iter.m_ptr;
		(ptr->m_prev ? ptr->m_prev->m_next : m_first) = ptr->m_next;
		(ptr->m_next ? ptr->m_next->m_prev : m_last) = ptr->m_prev;

		m_count--;

		_update_iterators_on_removed(ptr);

		delete ptr;
	}

	template<typename T>
	void chain_list_t<T>::remove_range(iterator const & p_from,iterator const & p_to)
	{
		if (p_from.is_valid() && p_to.is_valid())
		{
			bug_check_assert(p_from.m_owner == this && p_to.m_owner == this);

			iterator walk = p_from;
			while(walk != p_to)
			{
				iterator axeme = walk;
				walk.next();
				bug_check_assert(walk.is_valid());
				remove_single(axeme);
			}

			remove_single(walk);
		}
	}
	template<typename T>
	void chain_list_t<T>::remove_all() {remove_range(first(),last());}


}
#endif //_PFC_CHAINLIST_H_