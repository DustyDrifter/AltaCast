class NOVTABLE info_lookup_handler : public service_base {
public:
	enum {
		flag_album_lookup = 1 << 0,
		flag_track_lookup = 1 << 1,
	};
 
	virtual void get_name(pfc::string_base & p_out) = 0;
	virtual t_uint32 get_flags() = 0; // Returns one or more of flag_track_lookup, and flag_album_lookup.
	virtual HICON get_icon(int p_width, int p_height) = 0;
	virtual bool lookup(const pfc::list_base_const_t<metadb_handle_ptr> & p_items,HWND p_parent) = 0;
 
	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(info_lookup_handler);
};