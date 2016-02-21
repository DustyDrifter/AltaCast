template<typename T>
class cfg_structlist_t : public cfg_var, public pfc::list_t<T>
{
public:
	void get_data_raw(stream_writer * p_stream,abort_callback & p_abort) {
		t_uint32 n, m = get_count();
		p_stream->write_lendian_t(m,p_abort);
		for(n=0;n<m;n++) {
			p_stream->write_object(&m_buffer[n],sizeof(T),p_abort);
		}
	}
	
	void set_data_raw(stream_reader * p_stream,abort_callback & p_abort) {
		t_uint32 n,count;
		p_stream->read_lendian_t(count,p_abort);
		m_buffer.set_size_e(count);
		for(n=0;n<count;n++) {
			try {
				p_stream->read_object(&m_buffer[n],sizeof(T),p_abort);
			} catch(...) {m_buffer.set_size(0); throw;}
		}
	}
public:
	cfg_structlist_t(const GUID & p_guid) : cfg_var(p_guid) {}
};