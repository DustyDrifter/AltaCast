#ifndef _FOOBAR2000_SDK_FILE_INFO_IMPL_H_
#define _FOOBAR2000_SDK_FILE_INFO_IMPL_H_


class info_storage
{
public:
	t_size add_item(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
	void remove_mask(const bit_array & p_mask);	
	inline t_size get_count() const {return m_info.get_count();}
	inline const char * get_name(t_size p_index) const {return m_info[p_index].get_name();}
	inline const char * get_value(t_size p_index) const {return m_info[p_index].get_value();}
	void copy_from(const file_info & p_info);
	~info_storage();
private:
	struct info_entry
	{
	
		void init(const char * p_name,t_size p_name_len,const char * p_value,t_size p_value_len);
		void deinit();
		inline const char * get_name() const {return m_name;}
		inline const char * get_value() const {return m_value;}
		
	
		char * m_name;
		char * m_value;
	};
	pfc::list_t<info_entry> m_info;
};

class meta_storage
{
public:
	meta_storage();
	~meta_storage();

	t_size add_entry(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
	void insert_value(t_size p_index,t_size p_value_index,const char * p_value,t_size p_value_length);
	void modify_value(t_size p_index,t_size p_value_index,const char * p_value,t_size p_value_length);
	void remove_values(t_size p_index,const bit_array & p_mask);
	void remove_mask(const bit_array & p_mask);
	void copy_from(const file_info & p_info);

	inline void reorder(const t_size * p_order);

	inline t_size get_count() const {return m_data.get_size();}
	
	inline const char * get_name(t_size p_index) const {assert(p_index < m_data.get_size()); return m_data[p_index].get_name();}
	inline const char * get_value(t_size p_index,t_size p_value_index) const {assert(p_index < m_data.get_size()); return m_data[p_index].get_value(p_value_index);}
	inline t_size get_value_count(t_size p_index) const {assert(p_index < m_data.get_size()); return m_data[p_index].get_value_count();}

	struct meta_entry
	{
		meta_entry() {}
		meta_entry(const char * p_name,t_size p_name_len,const char * p_avlue,t_size p_value_len);

		void remove_values(const bit_array & p_mask);
		void insert_value(t_size p_value_index,const char * p_value,t_size p_value_length);
		void modify_value(t_size p_value_index,const char * p_value,t_size p_value_length);

		inline const char * get_name() const {return m_name;}
		inline const char * get_value(t_size p_index) const {return m_values[p_index];}
		inline t_size get_value_count() const {return m_values.get_size();}
		

		pfc::string_simple m_name;
		pfc::array_hybrid_t<pfc::string_simple,1,pfc::alloc_fast > m_values;

	};
private:
	

	pfc::array_hybrid_t<meta_entry,10, pfc::alloc_fast > m_data;
};

namespace pfc
{
	template<>
	inline void swap_t(meta_storage::meta_entry & p_item1,meta_storage::meta_entry & p_item2)
	{
		swap_t(p_item1.m_name,p_item2.m_name);
		swap_t(p_item1.m_values,p_item2.m_values);
	}
};

//! Implements file_info.
class file_info_impl : public file_info
{
public:
	file_info_impl(const file_info_impl & p_source);
	file_info_impl(const file_info & p_source);
	file_info_impl();
	~file_info_impl();

	double		get_length() const;
	void		set_length(double p_length);

	void		copy_meta(const file_info & p_source);//virtualized for performance reasons, can be faster in two-pass
	void		copy_info(const file_info & p_source);//virtualized for performance reasons, can be faster in two-pass
	
	t_size	meta_get_count() const;
	const char*	meta_enum_name(t_size p_index) const;
	t_size	meta_enum_value_count(t_size p_index) const;
	const char*	meta_enum_value(t_size p_index,t_size p_value_number) const;
	t_size	meta_set_ex(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
	void		meta_insert_value_ex(t_size p_index,t_size p_value_index,const char * p_value,t_size p_value_length);
	void		meta_remove_mask(const bit_array & p_mask);
	void		meta_reorder(const t_size * p_order);
	void		meta_remove_values(t_size p_index,const bit_array & p_mask);
	void		meta_modify_value_ex(t_size p_index,t_size p_value_index,const char * p_value,t_size p_value_length);

	t_size	info_get_count() const;
	const char*	info_enum_name(t_size p_index) const;
	const char*	info_enum_value(t_size p_index) const;
	t_size	info_set_ex(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
	void		info_remove_mask(const bit_array & p_mask);

	const file_info_impl & operator=(const file_info_impl & p_source);

	replaygain_info	get_replaygain() const;
	void			set_replaygain(const replaygain_info & p_info);

protected:
	t_size	meta_set_nocheck_ex(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
	t_size	info_set_nocheck_ex(const char * p_name,t_size p_name_length,const char * p_value,t_size p_value_length);
private:


	meta_storage m_meta;
	info_storage m_info;
	

	double m_length;

	replaygain_info m_replaygain;
};

typedef file_info_impl file_info_i;//for compatibility

#endif