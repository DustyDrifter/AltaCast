#include "stdafx.h"

static void _input_helper_io_filter_fullbuffer(service_ptr_t<file> & p_file,const char * p_path, t_filesize arg,abort_callback & p_abort) {
	if (arg > 0 && !filesystem::g_is_remote_or_unrecognized(p_path)) {
		fullFileBuffer a;
		p_file = a.open(p_path, p_abort, p_file, arg);
	}
}

static void _input_helper_io_filter_blockbuffer(service_ptr_t<file> & p_file,const char * p_path, t_filesize arg,abort_callback & p_abort) {
	if (arg > 0 && !filesystem::g_is_remote_or_unrecognized(p_path)) {
		if (p_file.is_empty()) filesystem::g_open_read( p_file, p_path, p_abort );
		if (!p_file->is_in_memory() && !p_file->is_remote() && p_file->can_seek()) {
			file_cached::g_create( p_file, p_file, p_abort, (size_t) arg );
		}
	}
}

void input_helper::open(service_ptr_t<file> p_filehint,metadb_handle_ptr p_location,unsigned p_flags,abort_callback & p_abort,bool p_from_redirect,bool p_skip_hints)
{
	open(p_filehint,p_location->get_location(),p_flags,p_abort,p_from_redirect,p_skip_hints);
}

void input_helper::process_fullbuffer(service_ptr_t<file> & p_file,const char * p_path,abort_callback & p_abort) {
	if (m_ioFilter != NULL) m_ioFilter(p_file, p_path, m_ioFilterArg, p_abort);
}

bool input_helper::need_file_reopen(const char * newPath) const {
	return m_input.is_empty() || metadb::path_compare(m_path, newPath) != 0;
}

bool input_helper::open_path(file::ptr p_filehint,const char * path,abort_callback & p_abort,bool p_from_redirect,bool p_skip_hints) {
	p_abort.check();

	if (!need_file_reopen(path)) return false;
	m_input.release();

	service_ptr_t<file> l_file = p_filehint;
	process_fullbuffer(l_file,path,p_abort);

	TRACK_CODE("input_entry::g_open_for_decoding",
		input_entry::g_open_for_decoding(m_input,l_file,path,p_abort,p_from_redirect)
		);

	
	if (!p_skip_hints) {
		try {
			static_api_ptr_t<metadb_io>()->hint_reader(m_input.get_ptr(),path,p_abort);
		} catch(exception_io_data) {
			//Don't fail to decode when this barfs, might be barfing when reading info from another subsong than the one we're trying to decode etc.
			m_input.release();
			if (l_file.is_valid()) l_file->reopen(p_abort);
			TRACK_CODE("input_entry::g_open_for_decoding",
				input_entry::g_open_for_decoding(m_input,l_file,path,p_abort,p_from_redirect)
				);
		}
	}

	m_path = path;
	return true;
}

void input_helper::open_decoding(t_uint32 subsong, t_uint32 flags, abort_callback & p_abort) {
	TRACK_CODE("input_decoder::initialize",m_input->initialize(subsong,flags,p_abort));
}

void input_helper::open(const playable_location & location, abort_callback & abort, decodeOpen_t const & other) {
	open_path(other.m_hint, location.get_path(), abort, other.m_from_redirect, other.m_skip_hints);

	set_logger( other.m_logger );

	open_decoding(location.get_subsong(), other.m_flags, abort);
}

void input_helper::open(service_ptr_t<file> p_filehint,const playable_location & p_location,unsigned p_flags,abort_callback & p_abort,bool p_from_redirect,bool p_skip_hints) {
	decodeOpen_t o;
	o.m_hint = p_filehint;
	o.m_flags = p_flags;
	o.m_from_redirect = p_from_redirect;
	o.m_skip_hints = p_skip_hints;
	this->open( p_location, p_abort, o );
}


void input_helper::close() {
	m_input.release();
}

bool input_helper::is_open() {
	return m_input.is_valid();
}

void input_helper::set_pause(bool state) {
	input_decoder_v3::ptr v3;
	if (m_input->service_query_t(v3)) v3->set_pause(state);
}
bool input_helper::flush_on_pause() {
	input_decoder_v3::ptr v3;
	if (m_input->service_query_t(v3)) return v3->flush_on_pause();
	else return false;
}


void input_helper::set_logger(event_logger::ptr ptr) {
	input_decoder_v2::ptr v2;
	if (m_input->service_query_t(v2)) v2->set_logger(ptr);
}

bool input_helper::run_raw(audio_chunk & p_chunk, mem_block_container & p_raw, abort_callback & p_abort) {
	input_decoder_v2::ptr v2;
	if (!m_input->service_query_t(v2)) throw pfc::exception_not_implemented();
	return v2->run_raw(p_chunk, p_raw, p_abort);
}

bool input_helper::run(audio_chunk & p_chunk,abort_callback & p_abort) {
	TRACK_CODE("input_decoder::run",return m_input->run(p_chunk,p_abort));
}

void input_helper::seek(double seconds,abort_callback & p_abort) {
	TRACK_CODE("input_decoder::seek",m_input->seek(seconds,p_abort));
}

bool input_helper::can_seek() {
	return m_input->can_seek();
}

void input_helper::set_full_buffer(t_filesize val) {
	m_ioFilter = _input_helper_io_filter_fullbuffer;
	m_ioFilterArg = val;
}

void input_helper::set_block_buffer(size_t val) {
	m_ioFilter = _input_helper_io_filter_blockbuffer;
	m_ioFilterArg = val;
}

void input_helper::on_idle(abort_callback & p_abort) {
	if (m_input.is_valid()) {
		TRACK_CODE("input_decoder::on_idle",m_input->on_idle(p_abort));
	}
}

bool input_helper::get_dynamic_info(file_info & p_out,double & p_timestamp_delta) {
	TRACK_CODE("input_decoder::get_dynamic_info",return m_input->get_dynamic_info(p_out,p_timestamp_delta));
}

bool input_helper::get_dynamic_info_track(file_info & p_out,double & p_timestamp_delta) {
	TRACK_CODE("input_decoder::get_dynamic_info_track",return m_input->get_dynamic_info_track(p_out,p_timestamp_delta));
}

void input_helper::get_info(t_uint32 p_subsong,file_info & p_info,abort_callback & p_abort) {
	TRACK_CODE("input_decoder::get_info",m_input->get_info(p_subsong,p_info,p_abort));
}

const char * input_helper::get_path() const {
	return m_path;
}


input_helper::input_helper() : m_ioFilter(), m_ioFilterArg()
{
}


void input_helper::g_get_info(const playable_location & p_location,file_info & p_info,abort_callback & p_abort,bool p_from_redirect) {
	service_ptr_t<input_info_reader> instance;
	input_entry::g_open_for_info_read(instance,0,p_location.get_path(),p_abort,p_from_redirect);
	instance->get_info(p_location.get_subsong_index(),p_info,p_abort);
}

void input_helper::g_set_info(const playable_location & p_location,file_info & p_info,abort_callback & p_abort,bool p_from_redirect) {
	service_ptr_t<input_info_writer> instance;
	input_entry::g_open_for_info_write(instance,0,p_location.get_path(),p_abort,p_from_redirect);
	instance->set_info(p_location.get_subsong_index(),p_info,p_abort);
	instance->commit(p_abort);
}


bool dead_item_filter::run(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,bit_array_var & p_mask) {
	file_list_helper::file_list_from_metadb_handle_list path_list;
	path_list.init_from_list(p_list);
	metadb_handle_list valid_handles;
	static_api_ptr_t<metadb> l_metadb;
	for(t_size pathidx=0;pathidx<path_list.get_count();pathidx++)
	{
		if (is_aborting()) return false;
		on_progress(pathidx,path_list.get_count());

		const char * path = path_list[pathidx];

		if (filesystem::g_is_remote_safe(path)) {
			metadb_handle_ptr temp;
			l_metadb->handle_create(temp,make_playable_location(path,0));
			valid_handles.add_item(temp);
		} else {
			try {
				service_ptr_t<input_info_reader> reader;
				
				input_entry::g_open_for_info_read(reader,0,path,*this);
				t_uint32 count = reader->get_subsong_count();
				for(t_uint32 n=0;n<count && !is_aborting();n++) {
					metadb_handle_ptr ptr;
					t_uint32 index = reader->get_subsong(n);
					l_metadb->handle_create(ptr,make_playable_location(path,index));
					valid_handles.add_item(ptr);
				}
			} catch(...) {}
		}
	}

	if (is_aborting()) return false;

	valid_handles.sort_by_pointer();
	for(t_size listidx=0;listidx<p_list.get_count();listidx++) {
		bool dead = valid_handles.bsearch_by_pointer(p_list[listidx]) == ~0;
		if (dead) console::formatter() << "Dead item: " << p_list[listidx];
		p_mask.set(listidx,dead);
	}
	return !is_aborting();
}

namespace {

class dead_item_filter_impl_simple : public dead_item_filter
{
public:
	inline dead_item_filter_impl_simple(abort_callback & p_abort) : m_abort(p_abort) {}
	bool is_aborting() const {return m_abort.is_aborting();}
	abort_callback_event get_abort_event() const {return m_abort.get_abort_event();}
	void on_progress(t_size p_position,t_size p_total) {}
private:
	abort_callback & m_abort;
};

}

bool input_helper::g_mark_dead(const pfc::list_base_const_t<metadb_handle_ptr> & p_list,bit_array_var & p_mask,abort_callback & p_abort)
{
	dead_item_filter_impl_simple filter(p_abort);
	return filter.run(p_list,p_mask);
}

void input_info_read_helper::open(const char * p_path,abort_callback & p_abort) {
	if (m_input.is_empty() || metadb::path_compare(m_path,p_path) != 0)
	{
		TRACK_CODE("input_entry::g_open_for_info_read",input_entry::g_open_for_info_read(m_input,0,p_path,p_abort));

		m_path = p_path;
	}
}

void input_info_read_helper::get_info(const playable_location & p_location,file_info & p_info,t_filestats & p_stats,abort_callback & p_abort) {
	open(p_location.get_path(),p_abort);

	TRACK_CODE("input_info_reader::get_file_stats",p_stats = m_input->get_file_stats(p_abort));

	TRACK_CODE("input_info_reader::get_info",m_input->get_info(p_location.get_subsong_index(),p_info,p_abort));
}

void input_info_read_helper::get_info_check(const playable_location & p_location,file_info & p_info,t_filestats & p_stats,bool & p_reloaded,abort_callback & p_abort) {
	open(p_location.get_path(),p_abort);
	t_filestats newstats;
	TRACK_CODE("input_info_reader::get_file_stats",newstats = m_input->get_file_stats(p_abort));
	if (metadb_handle::g_should_reload(p_stats,newstats,true)) {
		p_stats = newstats;
		TRACK_CODE("input_info_reader::get_info",m_input->get_info(p_location.get_subsong_index(),p_info,p_abort));
		p_reloaded = true;
	} else {
		p_reloaded = false;
	}
}






void input_helper_cue::open(service_ptr_t<file> p_filehint,const playable_location & p_location,unsigned p_flags,abort_callback & p_abort,double p_start,double p_length) {
	p_abort.check();

	m_start = p_start;
	m_position = 0;
	m_dynamic_info_trigger = false;
	m_dynamic_info_track_trigger = false;
	
	m_input.open(p_filehint,p_location,p_flags,p_abort,true,true);
	
	if (!m_input.can_seek()) throw exception_io_object_not_seekable();

	if (m_start > 0) {
		m_input.seek(m_start,p_abort);
	}

	if (p_length > 0) {
		m_length = p_length;
	} else {
		file_info_impl temp;
		m_input.get_info(0,temp,p_abort);
		double ref_length = temp.get_length();
		if (ref_length <= 0) throw exception_io_data();
		m_length = ref_length - m_start + p_length /* negative or zero */;
		if (m_length <= 0) throw exception_io_data();
	}
}

void input_helper_cue::close() {m_input.close();}
bool input_helper_cue::is_open() {return m_input.is_open();}

bool input_helper_cue::_m_input_run(audio_chunk & p_chunk, mem_block_container * p_raw, abort_callback & p_abort) {
	if (p_raw == NULL) {
		return m_input.run(p_chunk, p_abort);
	} else {
		return m_input.run_raw(p_chunk, *p_raw, p_abort);
	}
}
bool input_helper_cue::_run(audio_chunk & p_chunk, mem_block_container * p_raw, abort_callback & p_abort) {
	p_abort.check();
	
	if (m_length > 0) {
		if (m_position >= m_length) return false;

		if (!_m_input_run(p_chunk, p_raw, p_abort)) return false;

		m_dynamic_info_trigger = true;
		m_dynamic_info_track_trigger = true;

		t_uint64 max = (t_uint64) audio_math::time_to_samples(m_length - m_position, p_chunk.get_sample_rate());
		if (max == 0)
		{//handle rounding accidents, this normally shouldn't trigger
			m_position = m_length;
			return false;
		}

		t_size samples = p_chunk.get_sample_count();
		if ((t_uint64)samples > max)
		{
			p_chunk.set_sample_count((unsigned)max);
			if (p_raw != NULL) {
				const t_size rawSize = p_raw->get_size();
				PFC_ASSERT( rawSize % samples == 0 );
				p_raw->set_size( (t_size) ( (t_uint64) rawSize * max / samples ) );
			}
			m_position = m_length;
		}
		else
		{
			m_position += p_chunk.get_duration();
		}
		return true;
	}
	else
	{
		if (!_m_input_run(p_chunk, p_raw, p_abort)) return false;
		m_position += p_chunk.get_duration();
		return true;
	}
}
bool input_helper_cue::run_raw(audio_chunk & p_chunk, mem_block_container & p_raw, abort_callback & p_abort) {
	return _run(p_chunk, &p_raw, p_abort);
}

bool input_helper_cue::run(audio_chunk & p_chunk,abort_callback & p_abort) {
	return _run(p_chunk, NULL, p_abort);
}

void input_helper_cue::seek(double p_seconds,abort_callback & p_abort)
{
	m_dynamic_info_trigger = false;
	m_dynamic_info_track_trigger = false;
	if (m_length <= 0 || p_seconds < m_length) {
		m_input.seek(p_seconds + m_start,p_abort);
		m_position = p_seconds;
	} else {
		m_position = m_length;
	}
}

bool input_helper_cue::can_seek() {return true;}

void input_helper_cue::set_full_buffer(t_filesize val) {m_input.set_full_buffer(val);}

void input_helper_cue::on_idle(abort_callback & p_abort) {m_input.on_idle(p_abort);}

bool input_helper_cue::get_dynamic_info(file_info & p_out,double & p_timestamp_delta) {
	if (m_dynamic_info_trigger) {
		m_dynamic_info_trigger = false;
		return m_input.get_dynamic_info(p_out,p_timestamp_delta);
	} else {
		return false;
	}
}

bool input_helper_cue::get_dynamic_info_track(file_info & p_out,double & p_timestamp_delta) {
	if (m_dynamic_info_track_trigger) {
		m_dynamic_info_track_trigger = false;
		return m_input.get_dynamic_info_track(p_out,p_timestamp_delta);
	} else {
		return false;
	}
}

const char * input_helper_cue::get_path() const {return m_input.get_path();}
	
void input_helper_cue::get_info(t_uint32 p_subsong,file_info & p_info,abort_callback & p_abort) {m_input.get_info(p_subsong,p_info,p_abort);}


