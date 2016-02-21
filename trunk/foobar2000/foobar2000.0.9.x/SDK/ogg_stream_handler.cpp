#include "foobar2000.h"

void ogg_stream_handler::g_open(service_ptr_t<ogg_stream_handler> & p_out,service_ptr_t<file> p_reader,t_input_open_reason p_reason,abort_callback & p_abort)
{
	service_enum_t<ogg_stream_handler> e;
	service_ptr_t<ogg_stream_handler> ptr;
	bool need_reset = false;
	while(e.next(ptr)) {
		p_abort.check_e();
		if (need_reset) {
			p_reader->reopen(p_abort);
			need_reset = false;
		}

		try {
			ptr->open(p_reader,p_reason,p_abort);
			p_out = ptr;
			return;
		} catch(exception_io_data) {//do nothing
		} catch(exception_io_object_not_seekable) { 
		}//other exceptions are caller's problem

		need_reset = true;
	}
	throw exception_io_data();//noone knows this format
}
