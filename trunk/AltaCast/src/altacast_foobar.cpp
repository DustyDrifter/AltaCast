//#include <windows.h>
#undef _WINDOWS_
#include <afxwin.h>
#include <process.h>
#include <winsock2.h>


#include "libaltacast.h"
#include <mad.h>
#include "altacast_winamp.h"
#include "resource.h"

#include "MainWindow.h"
#include <bass.h>
CMainWindow *mainWindow;
CWinApp			mainApp;

#define UNICODE
#include "../SDK/foobar2000.h"
#include "../shared/shared.h"

#include "config.h"
#include <math.h>
#include "resource.h"
#undef _WINDOWS_
#include <afxwin.h>
#include "MainWindow.h"
char    logPrefix[255] = "dsp_altacast";

extern void writeMainConfig();
uCallStackTracker *booga;

DECLARE_COMPONENT_VERSION(
	"AltaCast",
	"1.1",
	"altacast\n"
	"Written by admin@altacast.com\n"
	);
static int isShown = 0;
//static cfg_int altacast_show_on_startup("dsp_altacastv2",0);//in 0.1 dB

void inputMetadataCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->inputMetadataCallback(g->encoderNumber, pValue);
}
void outputStatusCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->outputStatusCallback(g->encoderNumber, pValue);
}
void writeBytesCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->writeBytesCallback(g->encoderNumber, pValue);
}
void outputServerNameCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->outputServerNameCallback(g->encoderNumber, pValue);
}
void outputBitrateCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->outputBitrateCallback(g->encoderNumber, pValue);
}
void outputStreamURLCallback(void *gbl, void *pValue) {
    altacastGlobals *g = (altacastGlobals *)gbl;
    mainWindow->outputStreamURLCallback(g->encoderNumber, pValue);
}


int altacast_init(altacastGlobals *g)
{

	setServerStatusCallback(g, outputStatusCallback);
	setGeneralStatusCallback(g, NULL);
	setWriteBytesCallback(g, writeBytesCallback);
	setBitrateCallback(g, outputBitrateCallback);
	setServerNameCallback(g, outputServerNameCallback);
	setDestURLCallback(g, outputStreamURLCallback);

	readConfigFile(g);
	setFrontEndType(g, FRONT_END_ALTACAST_PLUGIN);
	return 1;
}

void initializeIt()
{
		char filename[512],*p;
		char	directory[1024] = "";
		char currentDir[MAX_PATH] = "";


		memset(filename, '\000', sizeof(filename));
		GetModuleFileName(NULL,filename,sizeof(filename));
		strcpy(currentDir, filename);
		char *pend;
		pend = strrchr(currentDir, '\\');
		if (pend) {
			*pend = '\000';
		}
		p = filename+lstrlen(filename);
		while (p >= filename && *p != '\\') p--;
		p++;

		char	logFile[1024] = "";
		memset(logFile, '\000', sizeof(logFile));
		strcpy(logFile, "altacast_foo");

		char tmpfile[MAX_PATH] = "";
		sprintf(tmpfile, "%s\\.tmp", currentDir);

		FILE *filep = fopen(tmpfile, "w");
		if (filep == 0) {
			char path[MAX_PATH] = "";

			SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
			strcpy(currentDir, path);
		}
		else {
			fclose(filep);
		}
        LoadConfigs(currentDir, logFile);

        AfxWinInit(core_api::get_my_instance(), NULL, "", SW_HIDE);

        mainWindow = new CMainWindow();

        mainWindow->InitializeWindow();

        strcpy(mainWindow->m_currentDir, currentDir);

        //mainWindow->Create((UINT)IDD_ALTACAST, mainApp.GetMainWnd());    
        //mainWindow->Create((UINT)IDD_ALTACAST, CWnd::FromHandle(core_api::get_main_window()));    
        mainWindow->Create((UINT)IDD_ALTACAST, AfxGetMainWnd());
        int x = getLastX();
        int y = getLastY();
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }

        mainWindow->SetWindowPos(NULL, x, y, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);

        modeless_dialog_manager::g_add(mainWindow->m_hWnd);
        modeless_dialog_manager::g_add(mainWindow->configDialog->m_hWnd);   
        modeless_dialog_manager::g_add(mainWindow->editMetadata->m_hWnd);

        mainWindow->SetIcon(mainApp.LoadIcon(IDR_MAINFRAME), TRUE);
        mainWindow->ShowWindow(SW_HIDE);
        

        initializealtacast();
}
class dsp_altacast : public dsp_impl_base {
	bool m_chunk_valid;
	audio_chunk_impl m_chunk;

public:
	dsp_altacast() {
		// Mark buffer as empty.
		m_chunk_valid = false;
	}

	// Every DSP type is identified by a GUID.
	static GUID g_get_guid() {
        static const GUID guid = 
        { 0xceee844a, 0xce07, 0x4d0a, { 0x95, 0x87, 0x34, 0x7e, 0x54, 0x1, 0xf2, 0x4c } };
		return guid;
	}

	// We also need a name, so the user can identify the DSP.
	// The name we use here does not describe what the DSP does,
	// so it would be a bad name. We can excuse this, because it
	// doesn't do anything useful anyway.
	static void g_get_name(pfc::string_base & p_out) {
		p_out = "AltaCast 1.1";
	}

	// The framework feeds input to our DSP using this method.
	// Each chunk contains a number of samples with the same
	// stream characteristics, i.e. same sample rate, channel count
	// and channel configuration.
	virtual bool on_chunk(audio_chunk * chunk,abort_callback & p_abort) {
		if (!isShown) {
			mainWindow->ShowWindow(SW_SHOW);
        //    initializeIt();
			isShown = 1;
		}
        if (!LiveRecordingCheck()) {
            int nsamples  = chunk->get_sample_count();
            int nch = chunk->get_channels();
            int srate = chunk->get_srate();
            float *samples = (float *)malloc(nsamples*nch*sizeof(float));
            audio_sample *psample = chunk->get_data();
            for (int i=0;i<nsamples*nch;i++) {
                audio_sample sample = *psample++;
                samples[i] = (sample);
            }
            handleAllOutput((float *)samples, nsamples, nch, srate);
            if (samples) {
                free(samples);
            }
        }
  	    return true;
	}

	virtual void flush() {

	}
	virtual void on_endoftrack(abort_callback & p_abort) {
		// This method is called on end of playback instead of flush().
		// We need to do the same thing as flush(), so we just call it.
		flush();
	}

	virtual void on_endofplayback(abort_callback & p_abort) {
		// This method is called on end of playback instead of flush().
		// We need to do the same thing as flush(), so we just call it.
		flush();
	}

	virtual double get_latency() {
		// If the buffered chunk is valid, return its length.
		// Otherwise return 0.
		return m_chunk_valid ? m_chunk.get_duration() : 0.0;
	}

	virtual bool need_track_change_mark() {
		// Return true if you need to know exactly when a new track starts.
		// Beware that this may break gapless playback, as at least all the
		// DSPs before yours have to be flushed.
		// To picture this, consider the case of a reverb DSP which outputs
		// the sum of the input signal and a delayed copy of the input signal.
		// In the case of a single track:

		// Input signal:   01234567
		// Delayed signal:   01234567

		// For two consecutive tracks with the same stream characteristics:

		// Input signal:   01234567abcdefgh
		// Delayed signal:   01234567abcdefgh

		// If the DSP chain contains a DSP that requires a track change mark,
		// the chain will be flushed between the two tracks:

		// Input signal:   01234567  abcdefgh
		// Delayed signal:   01234567  abcdefgh
		return false;
	}
};

/*
class dsp_altacast : public dsp_impl_base
{
public:
	public:
	dsp_altacast() {
		;
	}

	static GUID g_get_guid() {
        // {CEEE844A-CE07-4d0a-9587-347E5401F24C}
        static const GUID guid = 
        { 0xceee844a, 0xce07, 0x4d0a, { 0x95, 0x87, 0x34, 0x7e, 0x54, 0x1, 0xf2, 0x4c } };
		return guid;
	}
	virtual double get_latency() {
		// We have no buffer, so latency is 0.
		return 0.0;
	}

	virtual bool need_track_change_mark() {
		return false;
	}
	virtual void flush() {
		// Nothing to flush.
	}
	static bool g_get_default_preset(dsp_preset & p_out) {
		return false;
	}
	static void g_get_name(pfc::string_base & p_out) {
		p_out = "altacast V3";
	}

	virtual void on_endoftrack(abort_callback & p_abort) {
		return;
	}

	virtual void on_endofplayback(abort_callback & p_abort) {
		return;
	}
	static bool g_have_config_popup() {
		return false;
	}
	static bool g_show_config_popup(const dsp_preset & p_data,HWND p_parent,dsp_preset_edit_callback & p_callback) {
		return false;
	}
//	void show_config_popup_v2(const dsp_preset & p_data,HWND p_parent,dsp_preset_edit_callback & p_callback) {
//		return f;
//	}


	virtual bool on_chunk(audio_chunk * chunk,abort_callback & p_abort) {
		if (!isShown) {
			mainWindow->ShowWindow(SW_SHOW);
        //    initializeIt();
			isShown = 1;
		}
        if (!LiveRecordingCheck()) {
            int nsamples  = chunk->get_sample_count();
            int nch = chunk->get_channels();
            int srate = chunk->get_srate();
            float *samples = (float *)malloc(nsamples*nch*sizeof(float));
            audio_sample *psample = chunk->get_data();
            for (int i=0;i<nsamples*nch;i++) {
                audio_sample sample = *psample++;
                samples[i] = (sample);
            }
            handleAllOutput((float *)samples, nsamples, nch, srate);
            if (samples) {
                free(samples);
            }
        }
  	    return true;
	}

};
*/


class initquit_altacast : public initquit
{
public:
	virtual void on_init()
	{
		int a = 1;
        initializeIt();
	}
    virtual void on_system_shutdown()
    {
        ;
    }
	virtual void on_quit()
	{
		writeMainConfig();
        modeless_dialog_manager::g_remove(mainWindow->m_hWnd);
        modeless_dialog_manager::g_remove(mainWindow->configDialog->m_hWnd);
        modeless_dialog_manager::g_remove(mainWindow->editMetadata->m_hWnd);
	}

	static GUID get_class_guid();
};



class altacast_play_callback_ui : public play_callback_static
{
	virtual unsigned get_flags() {
		return play_callback::flag_on_playback_all;
	}
	virtual void on_playback_starting(play_control::t_track_command p_command,bool p_paused)
	{
	}
	virtual void on_playback_new_track(metadb_handle_ptr track)
	{
		pfc::string8		out = "";
		pfc::string8		fullTitle = "";
		pfc::string8		artist = "";
		pfc::string8		title = "";

       	char	songTitle[1024] = "";
	    char	songTitle2[1024] = "";
	    static	char currentTitle[1024] = "";
	    int		windowTitle = 1;
        char    newTitle[2046] = "";


        if (track != NULL) {
            track->metadb_lock();
			const file_info * info;
			track->get_info_async_locked(info);
            if (info) {
				freeComment();
                int numInfos = info->meta_get_count();
                for (int i=0;i<numInfos;i++) {
                    const char *pName = info->meta_enum_name(i);
                    const char *pValue = info->meta_enum_value(i,0);
                    char *buffer;
                    int bufferlen = 0;
                    bufferlen = strlen(pName) + strlen(pValue) + strlen("=") + 1;
                    buffer = (char *)calloc(1, bufferlen);
                    sprintf(buffer, "%s=%s", pName, pValue);
                    pfc::string8 theout = "";
                    theout.add_string(buffer);
					addComment((char *)(const char *)pfc::stringcvt::string_ansi_from_utf8(theout));
                    free(buffer);
					if (pName) {
						if (!stricmp(pName, "ARTIST")) {
							if (pValue) {
								artist.add_string(pValue);
							}
						}
						if (!stricmp(pName, "TITLE")) {
							if (pValue) {
								title.add_string(" - ");
								title.add_string(pValue);
							}
						}
					}
                }
				if (numInfos == 0) {
			        static const char * TITLE_FORMAT = "%_filename%";
					track->format_title_legacy(0,fullTitle,TITLE_FORMAT,0);
				}
				else {
					fullTitle.add_string(artist);
					fullTitle.add_string(title);
					/*
					track->handle_query_meta_field("ARTIST", 0, out);
					fullTitle.add_string(out);
					track->handle_query_meta_field("TITLE", 0, out);
					fullTitle.add_string(" - ");
					fullTitle.add_string(out);
					*/
					;
				}
	    	    setMetadataFromMediaPlayer((char *)(const char *)pfc::stringcvt::string_ansi_from_utf8(fullTitle));
            }
			track->metadb_unlock();
        }
	}
	virtual void on_playback_stop(play_control::t_stop_reason reason)
	{
		;
	}
	virtual void on_playback_seek(double time)
	{
		;
	}
	virtual void on_playback_pause(bool state)
	{
		;
	}
	virtual void on_playback_edited(metadb_handle_ptr track)
	{
		;
	}
	virtual void on_playback_dynamic_info(const file_info & p_info)
	{
		;
	}
	virtual void on_playback_dynamic_info_track(const file_info & p_info)
	{
		;
	}
	virtual void on_playback_time(double p_time)
	{
		;
	}
	
	virtual void on_volume_change(float new_val) {};
};
static dsp_factory_nopreset_t<dsp_altacast> odd_foo;

//static dsp_factory_t<dsp_altacast> odd_foo;

//static service_factory_single_t<dsp_altacast> odd_foo;
static service_factory_single_t<initquit_altacast> odd_foo_init;
static service_factory_single_t<altacast_play_callback_ui> odd_foo_play_callback;