//! Todo documentme
class NOVTABLE ogg_stream_handler : public service_base
{
public:
	virtual void open(service_ptr_t<file> p_reader,t_input_open_reason p_reason,abort_callback & p_abort) = 0;
	virtual void get_info(file_info & p_info,abort_callback & p_abort) = 0;
	virtual void decode_initialize(abort_callback & p_abort) = 0;
	virtual bool decode_run(audio_chunk & p_chunk,abort_callback & p_abort) = 0;
	virtual void decode_seek(double p_seconds,abort_callback & p_abort) = 0;
	virtual void retag(const file_info & p_info,stream_writer * p_out,abort_callback & p_abort) = 0;
	//! @returns false when in-place retag was not possible, true on success.
	virtual bool retag_inplace(const file_info & p_info,abort_callback & p_abort) = 0;
	virtual bool decode_can_seek() = 0;
	virtual bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) = 0;
	virtual bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) = 0;

	static void g_open(service_ptr_t<ogg_stream_handler> & p_out,service_ptr_t<file> p_reader,t_input_open_reason p_reason,abort_callback & p_abort);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(ogg_stream_handler);
};


template<typename T>
class ogg_stream_handler_factory_t : public service_factory_t<T> {};