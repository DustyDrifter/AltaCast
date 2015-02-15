template<unsigned blocksize>
class file_cached : public file {
public:
	static void g_create(service_ptr_t<file> & p_out,service_ptr_t<file> p_base,abort_callback & p_abort) {
		service_ptr_t<file_cached<blocksize> > temp;
		temp = new service_impl_t<file_cached<blocksize> >();
		temp->initialize(p_base,p_abort);
		p_out = temp.get_ptr();
	}
private:
	void initialize(service_ptr_t<file> p_base,abort_callback & p_abort) {
		m_base = p_base;
		m_position = 0;
		m_can_seek = m_base->can_seek();
		if (m_can_seek) {
			m_position_base = m_base->get_position(p_abort);
		} else {
			m_position_base = 0;
		}

		m_size = m_base->get_size(p_abort);

		flush_buffer();
	}
public:

	t_size read(void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		t_uint8 * outptr = (t_uint8*)p_buffer;
		t_size done = 0;
		while(done < p_bytes && m_position < m_size) {
			p_abort.check_e();

			if (m_position >= m_buffer_position && m_position < m_buffer_position + m_buffer_status) {				
				t_size delta = pfc::min_t<t_size>((t_size)(m_buffer_position + m_buffer_status - m_position),p_bytes - done);
				t_size bufptr = (t_size)(m_position - m_buffer_position);
				memcpy(outptr+done,m_buffer+bufptr,delta);
				done += delta;
				m_position += delta;
				if (m_buffer_status != sizeof(m_buffer) && done < p_bytes) break;//EOF before m_size is hit
			} else {
				m_buffer_position = m_position - m_position % blocksize;
				adjust_position(m_buffer_position,p_abort);

				m_buffer_status = m_base->read(m_buffer,sizeof(m_buffer),p_abort);
				m_position_base += m_buffer_status;

				if (m_buffer_status <= (t_size)(m_position - m_buffer_position)) break;
			}
		}

		return done;
	}

	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		p_abort.check_e();
		adjust_position(m_position,p_abort);
		m_base->write(p_buffer,p_bytes,p_abort);
		m_position_base = m_position = m_position + p_bytes_written;
		if (m_size < m_position) m_size = m_position;
		flush_buffer();
	}

	t_filesize get_size(abort_callback & p_abort) {
		p_abort.check_e();
		return m_size;
	}
	t_filesize get_position(abort_callback & p_abort) {
		p_abort.check_e();
		return m_position;
	}
	void set_eof(abort_callback & p_abort) {
		p_abort.check_e();
		adjust_position(m_position,p_abort);
		m_base->set_eof(p_abort);
		flush_buffer();
	}
	void seek(t_filesize p_position,abort_callback & p_abort) {
		p_abort.check_e();
		if (!m_can_seek) throw exception_io_object_not_seekable();
		if (p_position > m_size) throw exception_io_seek_out_of_range();
		m_position = p_position;
	}
	bool can_seek() {return m_can_seek;}
	bool get_content_type(pfc::string_base & out) {return m_base->get_content_type(out);}
	void on_idle(abort_callback & p_abort) {p_abort.check_e();m_base->on_idle(p_abort);}
	t_filetimestamp get_timestamp(abort_callback & p_abort) {p_abort.check_e(); return m_base->get_timestamp(p_abort);}
private:
	void adjust_position(t_filesize p_target,abort_callback & p_abort) {
		if (p_target != m_position_base) {
			m_base->seek(p_target,p_abort);
			m_position_base = p_target;
		}
	}

	void flush_buffer() {
		m_buffer_status = 0;
		m_buffer_position = 0;
	}

	service_ptr_t<file> m_base;
	t_filesize m_position,m_position_base,m_size;
	bool m_can_seek;
	t_filesize m_buffer_position;
	t_size m_buffer_status;
	t_uint8 m_buffer[blocksize];
	
};
