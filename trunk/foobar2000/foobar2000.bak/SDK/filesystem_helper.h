#ifndef _FILESYSTEM_HELPER_H_
#define _FILESYSTEM_HELPER_H_

//helper
class file_path_canonical {
public:
	file_path_canonical(const char * src) {filesystem::g_get_canonical_path(src,m_data);}
	operator const char * () const {return m_data.get_ptr();}
	const char * get_ptr() const {return m_data.get_ptr();}
	t_size get_length() const {return m_data.get_length();}
private:
	pfc::string8 m_data;
};

class file_path_display {
public:
	file_path_display(const char * src) {filesystem::g_get_display_path(src,m_data);}
	operator const char * () const {return m_data.get_ptr();}
	const char * get_ptr() const {return m_data.get_ptr();}
	t_size get_length() const {return m_data.get_length();}
private:
	pfc::string8 m_data;
};


class NOVTABLE reader_membuffer_base : public file_readonly {
public:
	reader_membuffer_base() : m_offset(0) {}

	t_size read(void * p_buffer,t_size p_bytes,abort_callback & p_abort);

	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort) {throw exception_io_sharing_violation();}

	t_filesize get_size(abort_callback & p_abort) {return get_buffer_size();}
	t_filesize get_position(abort_callback & p_abort) {return m_offset;}
	void seek(t_filesize position,abort_callback & p_abort);
	void reopen(abort_callback & p_abort) {seek(0,p_abort);}
	
	bool can_seek() {return true;}
	bool is_in_memory() {return true;}
		
protected:
	virtual const void * get_buffer() = 0;
	virtual t_size get_buffer_size() = 0;
	virtual t_filetimestamp get_timestamp(abort_callback & p_abort) = 0;
	virtual bool get_content_type(pfc::string_base &) {return false;}
	inline void seek_internal(t_size p_offset) {if (p_offset > get_buffer_size()) throw exception_io_seek_out_of_range(); m_offset = p_offset;}
private:
	t_size m_offset;
};

class reader_membuffer_mirror : public reader_membuffer_base
{
public:
	t_filetimestamp get_timestamp(abort_callback & p_abort) {return m_timestamp;}
	bool is_remote() {return m_remote;}

	//! Returns false when the object could not be mirrored (too big) or did not need mirroring.
	static bool g_create(service_ptr_t<file> & p_out,const service_ptr_t<file> & p_src,abort_callback & p_abort) {
		service_ptr_t<reader_membuffer_mirror> ptr = new service_impl_t<reader_membuffer_mirror>();
		if (!ptr->init(p_src,p_abort)) return false;
		p_out = ptr.get_ptr();
		return true;
	}

private:
	const void * get_buffer() {return m_buffer.get_ptr();}
	t_size get_buffer_size() {return m_buffer.get_size();}

	bool init(const service_ptr_t<file> & p_src,abort_callback & p_abort) {
		if (p_src->is_in_memory()) return false;//already buffered
		m_remote = p_src->is_remote();
		
		t_size size = pfc::downcast_guarded<t_size>(p_src->get_size(p_abort));

		m_buffer.set_size(size);

		p_src->reopen(p_abort);
		
		p_src->read_object(m_buffer.get_ptr(),size,p_abort);

		m_timestamp = p_src->get_timestamp(p_abort);

		return true;
	}


	t_filetimestamp m_timestamp;
	pfc::array_t<char> m_buffer;
	bool m_remote;

};

class reader_limited : public file_readonly {
	service_ptr_t<file> r;
	t_filesize begin;
	t_filesize end;
	
public:
	reader_limited() {begin=0;end=0;}
	reader_limited(const service_ptr_t<file> & p_r,t_filesize p_begin,t_filesize p_end,abort_callback & p_abort) {
		r = p_r;
		begin = p_begin;
		end = p_end;
		r->seek(begin,p_abort);
	}

	void init(const service_ptr_t<file> & p_r,t_filesize p_begin,t_filesize p_end,abort_callback & p_abort) {
		r = p_r;
		begin = p_begin;
		end = p_end;
		r->seek(begin,p_abort);
	}

	t_filetimestamp get_timestamp(abort_callback & p_abort) {return r->get_timestamp(p_abort);}

	t_size read(void *p_buffer, t_size p_bytes,abort_callback & p_abort) {
		t_filesize pos;
		pos = r->get_position(p_abort);
		if (p_bytes > end - pos) p_bytes = (t_size)(end - pos);
		return r->read(p_buffer,p_bytes,p_abort);
	}

	t_filesize get_size(abort_callback & p_abort) {return end-begin;}

	t_filesize get_position(abort_callback & p_abort) {
		return r->get_position(p_abort) - begin;
	}

	void seek(t_filesize position,abort_callback & p_abort) {
		r->seek(position+begin,p_abort);
	}
	bool can_seek() {return r->can_seek();}
	bool is_remote() {return r->is_remote();}
	
	bool get_content_type(pfc::string_base &) {return false;}

	void reopen(abort_callback & p_abort) {seek(0,p_abort);}
};

class stream_reader_memblock_ref : public stream_reader
{
public:
	stream_reader_memblock_ref(const void * p_data,t_size p_data_size) : m_data((const unsigned char*)p_data), m_data_size(p_data_size), m_pointer(0) {}
	t_size read(void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		t_size remaining = m_data_size - m_pointer;
		t_size toread = p_bytes;
		if (toread > remaining) toread = remaining;
		if (toread > 0) {
			memcpy(p_buffer,m_data+m_pointer,toread);
			m_pointer += toread;
		}

		return toread;
	}
private:
	const unsigned char * m_data;
	t_size m_data_size,m_pointer;
};

template<class t_storage>
class stream_writer_buffer_append_ref_t : public stream_writer
{
public:
	stream_writer_buffer_append_ref_t(t_storage & p_output) : m_output(p_output) {}
	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		pfc::static_assert< sizeof(m_output[0]) == 1>();
		t_size base = m_output.get_size();
		if (base + p_bytes < base) throw std::bad_alloc();
		m_output.set_size(base + p_bytes);
		memcpy( (t_uint8*) m_output.get_ptr() + base, p_buffer, p_bytes );
	}
private:
	t_storage & m_output;
};

class stream_reader_limited_ref : public stream_reader
{
public:
	stream_reader_limited_ref(stream_reader * p_reader,t_size p_limit) : m_reader(p_reader), m_remaining(p_limit) {}
	
	t_size read(void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		if (p_bytes > m_remaining) p_bytes = m_remaining;

		t_size done = m_reader->read(p_buffer,p_bytes,p_abort);
		m_remaining -= done;
		return done;
	}

	inline t_size get_remaining() const {return m_remaining;}

	void flush_remaining(abort_callback & p_abort)
	{
		if (m_remaining > 0) return skip_object(m_remaining,p_abort);
	}

private:
	stream_reader * m_reader;
	t_size m_remaining;
};

class stream_writer_chunk_dwordheader : public stream_writer
{
public:
	stream_writer_chunk_dwordheader(const service_ptr_t<file> & p_writer) : m_writer(p_writer) {}

	void initialize(abort_callback & p_abort) {
		m_headerposition = m_writer->get_position(p_abort);
		m_written = 0;
		m_writer->write_lendian_t((t_uint32)0,p_abort);
	}

	void finalize(abort_callback & p_abort) {
		t_filesize end_offset;
		end_offset = m_writer->get_position(p_abort);
		m_writer->seek(m_headerposition,p_abort);
		m_writer->write_lendian_t(pfc::downcast_guarded<t_uint32>(m_written),p_abort);
		m_writer->seek(end_offset,p_abort);
	}

	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort) {
		m_writer->write(p_buffer,p_bytes,p_abort);
		m_written += p_bytes;
	}

private:
	service_ptr_t<file> m_writer;
	t_filesize m_headerposition;
	t_filesize m_written;
};

class stream_writer_chunk : public stream_writer
{
public:
	stream_writer_chunk(stream_writer * p_writer) : m_writer(p_writer), m_buffer_state(0) {}

	void write(const void * p_buffer,t_size p_bytes,abort_callback & p_abort);

	void flush(abort_callback & p_abort);//must be called after writing before object is destroyed
	
private:
	stream_writer * m_writer;
	unsigned m_buffer_state;
	unsigned char m_buffer[255];
};

class stream_reader_chunk : public stream_reader
{
public:
	stream_reader_chunk(stream_reader * p_reader) : m_reader(p_reader), m_buffer_state(0), m_buffer_size(0), m_eof(false) {}

	t_size read(void * p_buffer,t_size p_bytes,abort_callback & p_abort);

	void flush(abort_callback & p_abort);//must be called after reading before object is destroyed

	static void g_skip(stream_reader * p_stream,abort_callback & p_abort);

private:
	stream_reader * m_reader;
	t_size m_buffer_state, m_buffer_size;
	bool m_eof;
	unsigned char m_buffer[255];
};

#endif//_FILESYSTEM_HELPER_H_