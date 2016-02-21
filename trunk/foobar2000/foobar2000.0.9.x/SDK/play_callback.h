#ifndef _PLAY_CALLBACK_H_
#define _PLAY_CALLBACK_H_

/*!
Class receiving notifications about playback events. Note that all methods are called only from app's main thread.
*/
class NOVTABLE play_callback {
public:
	//! Playback process is being initialized. on_playback_new_track() should be called soon after this when first file is successfully opened for decoding.
	virtual void FB2KAPI on_playback_starting(play_control::t_track_command p_command,bool p_paused)=0;
	//! Playback advanced to new track.
	virtual void FB2KAPI on_playback_new_track(metadb_handle_ptr p_track) = 0;
	//! Playback stopped.
	virtual void FB2KAPI on_playback_stop(play_control::t_stop_reason p_reason)=0;
	//! User has seeked to specific time.
	virtual void FB2KAPI on_playback_seek(double p_time)=0;
	//! Called on pause/unpause.
	virtual void FB2KAPI on_playback_pause(bool p_state)=0;
	//! Called when currently played file gets edited.
	virtual void FB2KAPI on_playback_edited(metadb_handle_ptr p_track) = 0;
	//! Dynamic info (VBR bitrate etc) change.
	virtual void FB2KAPI on_playback_dynamic_info(const file_info & p_info) = 0;
	//! Per-track dynamic info (stream track titles etc) change. Happens less often than on_playback_dynamic_info().
	virtual void FB2KAPI on_playback_dynamic_info_track(const file_info & p_info) = 0;
	//! Called every second, for time display
	virtual void FB2KAPI on_playback_time(double p_time) = 0;
	//! User changed volume settings. Possibly called when not playing.
	//! @param p_new_val new volume level in dB; 0 for full volume.
	virtual void FB2KAPI on_volume_change(float p_new_val) = 0;

	enum {
		flag_on_playback_starting			= 1 << 0,
		flag_on_playback_new_track			= 1 << 1, 
		flag_on_playback_stop				= 1 << 2,
		flag_on_playback_seek				= 1 << 3,
		flag_on_playback_pause				= 1 << 4,
		flag_on_playback_edited				= 1 << 5,
		flag_on_playback_dynamic_info		= 1 << 6,
		flag_on_playback_dynamic_info_track	= 1 << 7,
		flag_on_playback_time				= 1 << 8,
		flag_on_volume_change				= 1 << 9,

		flag_on_playback_all = flag_on_playback_starting | flag_on_playback_new_track | 
			flag_on_playback_stop | flag_on_playback_seek | 
			flag_on_playback_pause | flag_on_playback_edited |
			flag_on_playback_dynamic_info | flag_on_playback_dynamic_info_track | flag_on_playback_time,
	};
protected:
	play_callback() {}
	~play_callback() {}
};

//! Standard API (always present); manages registrations of dynamic play_callbacks.
//! Usage: use static_api_ptr_t<play_callback_manager>.
//! Do not reimplement.
class NOVTABLE play_callback_manager : public service_base
{
public:
	//! Registers a play_callback object.
	//! @param p_callback Interface to register.
	//! @param p_flags Indicates which notifications are requested.
	virtual void FB2KAPI register_callback(play_callback * p_callback,unsigned p_flags,bool p_forward_status_on_register) = 0;
	//! Unregisters a play_callback object.
	//! @p_callback Previously registered interface to unregister.
	virtual void FB2KAPI unregister_callback(play_callback * p_callback) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(play_callback_manager);
};


//! Static (autoregistered) version of play_callback. Use play_callback_static_factory_t to register.
class play_callback_static : public service_base, public play_callback {
public:
	//! Controls which methods your callback wants called; returned value should not change in run time, you should expect it to be queried only once (on startup). See play_callback::flag_* constants.
	virtual unsigned get_flags() = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(play_callback_static);
};

template<typename T>
class play_callback_static_factory_t : public service_factory_single_t<T> {};


//! Gets notified about tracks being played. Notification occurs when at least 60s of the track has been played, or the track has reached its end after at least 1/3 of it has been played through.
//! Use playback_statistics_collector_factory_t to register.
class NOVTABLE playback_statistics_collector : public service_base {
public:
	virtual void on_item_played(metadb_handle_ptr p_item) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(playback_statistics_collector);
};

template<typename T>
class playback_statistics_collector_factory_t : public service_factory_single_t<T> {};
#endif