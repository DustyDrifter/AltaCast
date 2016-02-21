#include "foobar2000.h"



cfg_var * cfg_var::list=0;


static int cfg_var_guid_compare(const cfg_var * p_var1,const cfg_var * p_var2)
{
	return pfc::guid_compare(p_var1->get_guid(),p_var2->get_guid());
}

static int cfg_var_guid_compare_search(const cfg_var * p_var1,const GUID & p_var2)
{
	return pfc::guid_compare(p_var1->get_guid(),p_var2);
}

void cfg_var::config_read_file(stream_reader * p_stream,abort_callback & p_abort)
{
	for(;;)
	{
		GUID guid;
		t_uint32 size;

		if (p_stream->read(&guid,sizeof(guid),p_abort) != sizeof(guid)) break;
		guid = pfc::byteswap_if_be_t(guid);
		p_stream->read_lendian_t(size,p_abort);

		bool found = false;
		cfg_var * ptr;
		for(ptr = list; ptr; ptr=ptr->next) {
			if (ptr->get_guid() == guid) {
				stream_reader_limited_ref wrapper(p_stream,size);
				try {
					ptr->set_data_raw(&wrapper,size,p_abort);
				} catch(exception_io_data const & ) {}
				wrapper.flush_remaining(p_abort);
				found = true;
				break;
			}
		}
		if (!found)
			p_stream->skip_object(size,p_abort);
	}
}

void cfg_var::config_write_file(stream_writer * p_stream,abort_callback & p_abort) {
	cfg_var * ptr;
	pfc::array_t<t_uint8,pfc::alloc_fast_aggressive> temp;
	for(ptr = list; ptr; ptr=ptr->next) {
		temp.set_size(0);
		ptr->get_data_raw(&stream_writer_buffer_append_ref_t<pfc::array_t<t_uint8,pfc::alloc_fast_aggressive> >(temp),p_abort);
		p_stream->write_lendian_t(ptr->get_guid(),p_abort);
		p_stream->write_lendian_t(pfc::downcast_guarded<t_uint32>(temp.get_size()),p_abort);
		if (temp.get_size() > 0) {
			p_stream->write_object(temp.get_ptr(),temp.get_size(),p_abort);
		}
	}
}


void cfg_string::get_data_raw(stream_writer * p_stream,abort_callback & p_abort) {
	p_stream->write_object(get_ptr(),length(),p_abort);
}

void cfg_string::set_data_raw(stream_reader * p_stream,t_size p_sizehint,abort_callback & p_abort) {
	pfc::string8_fastalloc temp;
	p_stream->read_string_raw(temp,p_abort);
	set_string(temp);
}
