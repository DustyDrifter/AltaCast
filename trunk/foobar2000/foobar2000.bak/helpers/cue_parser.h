namespace cue_parser
{
	struct cue_entry {
		pfc::string8 m_file;
		unsigned m_track_number;
		t_cuesheet_index_list m_indexes;
	};


	PFC_DECLARE_EXCEPTION(exception_bad_cuesheet,exception_io_data,"Invalid cuesheet");

	//! Throws exception_bad_cuesheet on failure.
	void parse(const char *p_cuesheet,pfc::chain_list_t<cue_entry> & p_out);
	//! Throws exception_bad_cuesheet on failure.
	void parse_info(const char *p_cuesheet,file_info & p_info,unsigned p_index);
	//! Throws exception_bad_cuesheet on failure.
	void parse_full(const char * p_cuesheet,cue_creator::t_entry_list & p_out);

	//! Throws exception_io_data and derivatives on failure.
	void extract_info(const file_info & p_baseinfo,file_info & p_info, t_uint32 p_subsong_index);
	void strip_cue_track_metadata(file_info & p_info);

	namespace input_wrapper_cue_base {
		void write_meta_create_field(pfc::string_base & p_out,const char * p_name,int p_index);
		void write_meta(file_info & p_baseinfo,const file_info & p_trackinfo,unsigned p_subsong_index);
	};

	class __decoder_wrapper {
	public:
		virtual bool run(audio_chunk & p_chunk,abort_callback & p_abort) = 0;
		virtual void seek(double p_seconds,abort_callback & p_abort) = 0;
		virtual bool get_dynamic_info(file_info & p_out, double & p_timestamp_delta) = 0;
		virtual bool get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) = 0;
		virtual void on_idle(abort_callback & p_abort) = 0;
		virtual ~__decoder_wrapper() {}
	};

	template<typename t_input>
	class __decoder_wrapper_simple : public __decoder_wrapper {
	public:
		void initialize(service_ptr_t<file> p_filehint,const char * p_path,abort_callback & p_abort) {
			m_input.open(p_filehint,p_path,input_open_decode,p_abort);
			m_input.decode_initialize(0,p_abort);
		}
		bool run(audio_chunk & p_chunk,abort_callback & p_abort) {return m_input.decode_run(p_chunk,p_abort);}
		void seek(double p_seconds,abort_callback & p_abort) {m_input.decode_seek(p_seconds,p_abort);}
		bool get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {return m_input.decode_get_dynamic_info(p_out,p_timestamp_delta);}
		bool get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {return m_input.decode_get_dynamic_info_track(p_out,p_timestamp_delta);}
		void on_idle(abort_callback & p_abort) {m_input.decode_on_idle(p_abort);}
	private:
		t_input m_input;
	};

	class __decoder_wrapper_cue : public __decoder_wrapper {
	public:
		void open(service_ptr_t<file> p_filehint,const playable_location & p_location,unsigned p_flags,abort_callback & p_abort,double p_start,double p_length) {
			m_input.open(p_filehint,p_location,p_flags,p_abort,p_start,p_length);
		}
		bool run(audio_chunk & p_chunk,abort_callback & p_abort) {return m_input.run(p_chunk,p_abort);}
		void seek(double p_seconds,abort_callback & p_abort) {m_input.seek(p_seconds,p_abort);}
		bool get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {return m_input.get_dynamic_info(p_out,p_timestamp_delta);}
		bool get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {return m_input.get_dynamic_info_track(p_out,p_timestamp_delta);}
		void on_idle(abort_callback & p_abort) {m_input.on_idle(p_abort);}

	private:
		input_helper_cue m_input;
	};

#if 1
	template<typename t_base>
	class input_wrapper_cue_t {
	public:
		input_wrapper_cue_t() {}
		~input_wrapper_cue_t() {}

		void open(service_ptr_t<file> p_filehint,const char * p_path,t_input_open_reason p_reason,abort_callback & p_abort) {
			m_path = p_path;

			m_file = p_filehint;
			input_open_file_helper(m_file,p_path,p_reason,p_abort);

			{
				t_base instance;
				instance.open(m_file,p_path,p_reason,p_abort);
				instance.get_info(m_info,p_abort);
			}

			{
				const char *cue = m_info.meta_get("cuesheet",0);
				if (cue != 0 && cue[0] != 0)
				{
					m_cue_data.remove_all();
					try {
						cue_parser::parse(cue,m_cue_data);
					} catch(exception_bad_cuesheet const & e) {console::info(e.what());m_cue_data.remove_all();}
				}
			}
		}

		t_uint32 get_subsong_count() {
			return m_cue_data.get_count() > 0 ? pfc::downcast_guarded<t_uint32>(m_cue_data.get_count()) : 1;
		}

		t_uint32 get_subsong(t_uint32 p_index) {
			pfc::chain_list_t<cue_entry>::const_iterator iter = m_cue_data.by_index(p_index);
			if (iter.is_valid()) return iter->m_track_number;
			else return 0;
		}

		void get_info(t_uint32 p_subsong,file_info & p_info,abort_callback & p_abort)
		{
			if (p_subsong == 0) {
				p_info = m_info;
				p_info.info_set("cue_embedded","no");
			} else {
				extract_info(m_info,p_info, p_subsong);
			}
		}

		t_filestats get_file_stats(abort_callback & p_abort) {return m_file->get_stats(p_abort);}

		void decode_initialize(t_uint32 p_subsong,unsigned p_flags,abort_callback & p_abort) {
			m_decoder.release();
			if (p_subsong == 0) {
				pfc::rcptr_t<__decoder_wrapper_simple<t_base> > temp;
				temp.new_t();
				m_file->reopen(p_abort);
				temp->initialize(m_file,m_path,p_abort);
				m_decoder = temp;
			} else {
				double start = 0, end = m_info.get_length();
				
				if (p_subsong != 0)
				{
					pfc::chain_list_t<cue_entry>::const_iterator iter;
					for(iter = m_cue_data.first();iter.is_valid(); ++iter)
					{
						if (iter->m_track_number == p_subsong)
						{
							start = iter->m_indexes.start();
							++iter;
							if (iter.is_valid())
								end = iter->m_indexes.start();
							break;
						}
					}
				}
				
				if (end <= start) throw exception_io_data();

				pfc::rcptr_t<__decoder_wrapper_cue> temp;
				temp.new_t();
				m_file->reopen(p_abort);
				temp->open(m_file,make_playable_location(m_path,0),p_flags & ~input_flag_no_seeking,p_abort,start,end-start);
				m_decoder = temp;
			}
		}

		bool decode_run(audio_chunk & p_chunk,abort_callback & p_abort) {
			return m_decoder->run(p_chunk,p_abort);
		}
		
		void decode_seek(double p_seconds,abort_callback & p_abort) {
			m_decoder->seek(p_seconds,p_abort);
		}
		
		bool decode_can_seek() {return true;}

		bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {
			return m_decoder->get_dynamic_info(p_out,p_timestamp_delta);
		}

		bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {
			return m_decoder->get_dynamic_info_track(p_out,p_timestamp_delta);
		}

		void decode_on_idle(abort_callback & p_abort) {
			m_decoder->on_idle(p_abort);
		}

		void retag_set_info(t_uint32 p_subsong,const file_info & p_info,abort_callback & p_abort) {
			pfc::dynamic_assert(m_decoder.is_empty());
			pfc::chain_list_t<t_retag_entry>::iterator iter;
			iter = m_retag_entries.insert_last();
			iter->m_index = p_subsong;
			iter->m_info = p_info;
		}

		void retag_commit(abort_callback & p_abort) {
			pfc::dynamic_assert(m_decoder.is_empty());
			if (m_retag_entries.get_count() == 0) return;

			bool cue_modified = false;

			cue_creator::t_entry_list cue_entries;

			{
				const char * old_cue = m_info.meta_get("cuesheet",0);
				if (old_cue != 0)
				{
					try {
						parse_full(old_cue,cue_entries);
					} catch(exception_bad_cuesheet) {
						cue_entries.remove_all();
					}
				}
			}

			pfc::chain_list_t<t_retag_entry>::iterator iter;
			for(iter=m_retag_entries.first();iter.is_valid();++iter) {
				if (iter->m_index == 0) {
					m_info = iter->m_info;
				} else {
					cue_creator::t_entry_list::iterator cueiter;
					for(cueiter=cue_entries.first();cueiter.is_valid();++cueiter) {
						if (cueiter->m_track_number == iter->m_index) {
							cueiter->m_infos.copy_meta(iter->m_info);
							cueiter->m_infos.set_replaygain(iter->m_info.get_replaygain());

							input_wrapper_cue_base::write_meta(m_info,iter->m_info,iter->m_index);
							
							cue_modified = true;

							break;
						}
					}
				}
			}

			m_retag_entries.remove_all();

			if (cue_modified) {
				string_formatter temp;

				cue_creator::create(temp,cue_entries);

				m_info.meta_set("cuesheet",temp);
			}

			{
				t_base instance;
				m_file->reopen(p_abort);
				instance.open(m_file,m_path,input_open_info_write,p_abort);
				instance.retag(pfc::safe_cast<const file_info&>(m_info),p_abort);
				instance.get_info(m_info,p_abort);
			}
		}

		inline static bool g_is_our_content_type(const char * p_content_type) {return t_base::g_is_our_content_type(p_content_type);}
		inline static bool g_is_our_path(const char * p_path,const char * p_extension) {return t_base::g_is_our_path(p_path,p_extension);}

	private:
		pfc::rcptr_t<__decoder_wrapper> m_decoder;

		file_info_impl m_info;
		pfc::string8 m_path;
		service_ptr_t<file> m_file;
		
		pfc::chain_list_t<cue_entry> m_cue_data;
		
		struct t_retag_entry {t_uint32 m_index; file_info_impl m_info;};
		pfc::chain_list_t<t_retag_entry> m_retag_entries;
	};
#else
	template<typename t_base>
	class input_wrapper_cue_t
	{
	public:
		input_wrapper_cue_t() {}
		~input_wrapper_cue_t() {}

		void open(service_ptr_t<file> p_filehint,const char * p_path,t_input_open_reason p_reason,abort_callback & p_abort) {
			m_path = p_path;

			m_instance.open(p_filehint,p_path,p_reason,p_abort);
			m_instance.get_info(m_info,p_abort);

			{
				const char *cue = m_info.meta_get("cuesheet",0);
				if (cue != 0 && cue[0] != 0)
				{
					m_cue_data.remove_all();
					try {
						cue_parser::parse(cue,m_cue_data);
					} catch(exception_bad_cuesheet const & e) {console::info(e.what());m_cue_data.remove_all();}
				}
			}
		}

		t_uint32 get_subsong_count() {
			return m_cue_data.get_count() > 0 ? pfc::downcast_guarded<t_uint32>(m_cue_data.get_count()) : 1;
		}

		t_uint32 get_subsong(t_uint32 p_index) {
			pfc::chain_list_t<cue_entry>::const_iterator iter = m_cue_data.by_index(p_index);
			if (iter.is_valid()) return iter->m_track_number;
			else return 0;
		}

		void get_info(t_uint32 p_subsong,file_info & p_info,abort_callback & p_abort)
		{
			if (p_subsong == 0) {
				p_info = m_info;
				p_info.info_set("cue_embedded","no");
			} else {
				extract_info(m_info,p_info, p_subsong);
			}
		}

		t_filestats get_file_stats(abort_callback & p_abort) {return m_instance.get_file_stats(p_abort);}

		void decode_initialize(t_uint32 p_subsong,unsigned p_flags,abort_callback & p_abort)
		{
			m_decode_subsong = p_subsong;
			if (m_decode_subsong == 0) return m_instance.decode_initialize(p_flags,p_abort);
			else
			{
				double start = 0, end = m_info.get_length();
				
				if (p_subsong != 0)
				{
					pfc::chain_list_t<cue_entry>::const_iterator iter;
					for(iter = m_cue_data.first();iter.is_valid(); ++iter)
					{
						if (iter->m_track_number == p_subsong)
						{
							start = iter->m_indexes.start();
							++iter;
							if (iter.is_valid())
								end = iter->m_indexes.start();
							break;
						}
					}
				}
				
				if (end <= start) throw exception_io_data();

				m_input_cue.open(NULL/*FIXME*/,make_playable_location(m_path,0),p_flags & ~input_flag_no_seeking,p_abort,start,end-start);
			}
		}

		bool decode_run(audio_chunk & p_chunk,abort_callback & p_abort) {
			return m_decode_subsong > 0 ?
				m_input_cue.run(p_chunk,p_abort) :
				m_instance.decode_run(p_chunk,p_abort);
		}
		
		void decode_seek(double p_seconds,abort_callback & p_abort) {
			if (m_decode_subsong > 0)
				m_input_cue.seek(p_seconds,p_abort);
			else
				m_instance.decode_seek(p_seconds,p_abort);
		}
		
		bool decode_can_seek() {return true;}

		bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {
			return m_decode_subsong > 0 ? 
				m_input_cue.get_dynamic_info(p_out,p_timestamp_delta) : 
				m_instance.decode_get_dynamic_info(p_out,p_timestamp_delta);
		}

		bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {
			return m_decode_subsong > 0 ? 
				m_input_cue.get_dynamic_info_track(p_out,p_timestamp_delta) : 
				m_instance.decode_get_dynamic_info_track(p_out,p_timestamp_delta);
		}

		void decode_on_idle(abort_callback & p_abort) {
			if (m_decode_subsong > 0)
				m_input_cue.on_idle(p_abort);
			else
				m_instance.decode_on_idle(p_abort);
		}

		void retag_set_info(t_uint32 p_subsong,const file_info & p_info,abort_callback & p_abort) {
			pfc::chain_list_t<t_retag_entry>::iterator iter;
			iter = m_retag_entries.insert_last();
			iter->m_index = p_subsong;
			iter->m_info = p_info;
		}

		void retag_commit(abort_callback & p_abort) {
			if (m_retag_entries.get_count() == 0) return;

			bool cue_modified = false;

			cue_creator::t_entry_list cue_entries;

			{
				const char * old_cue = m_info.meta_get("cuesheet",0);
				if (old_cue != 0)
				{
					try {
						parse_full(old_cue,cue_entries);
					} catch(exception_bad_cuesheet) {
						cue_entries.remove_all();
					}
				}
			}

			pfc::chain_list_t<t_retag_entry>::iterator iter;
			for(iter=m_retag_entries.first();iter.is_valid();++iter) {
				if (iter->m_index == 0) {
					m_info = iter->m_info;
				} else {
					cue_creator::t_entry_list::iterator cueiter;
					for(cueiter=cue_entries.first();cueiter.is_valid();++cueiter) {
						if (cueiter->m_track_number == iter->m_index) {
							cueiter->m_infos.copy_meta(iter->m_info);
							cueiter->m_infos.set_replaygain(iter->m_info.get_replaygain());

							input_wrapper_cue_base::write_meta(m_info,iter->m_info,iter->m_index);
							
							cue_modified = true;

							break;
						}
					}
				}
			}

			m_retag_entries.remove_all();

			if (cue_modified) {
				string_formatter temp;

				cue_creator::create(temp,cue_entries);

				m_info.meta_set("cuesheet",temp);
			}

			m_instance.retag(m_info,p_abort);
		}

		inline static bool g_is_our_content_type(const char * p_content_type) {return t_base::g_is_our_content_type(p_content_type);}
		inline static bool g_is_our_path(const char * p_path,const char * p_extension) {return t_base::g_is_our_path(p_path,p_extension);}

	private:
		t_base m_instance;
		file_info_impl m_info;
		t_uint32 m_decode_subsong;
		input_helper_cue m_input_cue;
		pfc::string8 m_path;
		
		pfc::chain_list_t<cue_entry> m_cue_data;
		
		struct t_retag_entry {t_uint32 m_index; file_info_impl m_info;};
		pfc::chain_list_t<t_retag_entry> m_retag_entries;

	};
#endif

	template<class I>
	class chapterizer_impl_t : public chapterizer
	{
	public:
		bool is_our_file(const char * p_path,abort_callback & p_abort) 
		{
			return I::g_is_our_path(p_path,string_extension(p_path));
		}

		void set_chapters(const char * p_path,chapter_list const & p_list,abort_callback & p_abort) {
			file_info_impl info;
			
			I instance;
			instance.open(0,p_path,input_open_info_write,p_abort);

			instance.get_info(info,p_abort);

			info.meta_remove_all();

			string_formatter cuesheet;
						
			{
				cue_creator::t_entry_list entries;
				t_size n, m = p_list.get_chapter_count();
								
				double offset_acc = 0;
				for(n=0;n<m;n++)
				{
					cue_creator::t_entry_list::iterator entry;
					entry = entries.insert_last();
					entry->m_infos = p_list.get_info(n);
					entry->m_file = "CDImage.wav";
					entry->m_track_number = (unsigned)(n+1);
					entry->m_index_list.from_infos(entry->m_infos,offset_acc);
//					entry->set_simple_index( offset_acc );
					offset_acc += entry->m_infos.get_length();
					
					input_wrapper_cue_base::write_meta(info,p_list.get_info(n),(unsigned)(n+1));
				}
				cue_creator::create(cuesheet,entries);
			}

			
			info.meta_set("cuesheet",cuesheet);
			
			instance.retag(info,p_abort);
		}

		void get_chapters(const char * p_path,chapter_list & p_list,abort_callback & p_abort) {
			file_info_impl info;


			{
				I instance;
				instance.open(0,p_path,input_open_info_read,p_abort);
				instance.get_info(info,p_abort);
			}

			const char *cue = info.meta_get("cuesheet",0);
			if (cue == 0 || cue[0] == 0) {
				p_list.set_chapter_count(1);
				p_list.set_info(0,info);
				return;
			}

			pfc::chain_list_t<cue_entry> cue_data;
			cue_parser::parse(cue,cue_data);

			p_list.set_chapter_count(cue_data.get_count());
			unsigned chapterptr = 0;

			pfc::chain_list_t<cue_entry>::const_iterator iter;
			for(iter=cue_data.first();iter.is_valid();iter++)
			{
				unsigned index = iter->m_track_number;
				if (index > 0 && index < 100) {
					file_info_impl info_subsong;
					extract_info(info,info_subsong,index);
					p_list.set_info(chapterptr++, info_subsong);
				}
			}
			if (chapterptr < cue_data.get_count())
				p_list.set_chapter_count(chapterptr);
		}
	};

};

template<class I>
class input_cuesheet_factory_t
{
public:
	input_factory_t<cue_parser::input_wrapper_cue_t<I> > m_input_factory;
	service_factory_single_t<cue_parser::chapterizer_impl_t<I> > m_chapterizer_factory;	
};