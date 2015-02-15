#include <winsock2.h>
#include <windows.h>
#undef _WINDOWS_
#include <afxwin.h>
#include <process.h>

#include "dsp.h"
#include "libaltacast.h"
#include <mad.h>
#include "frontend.h"
#include "altacast_radiodj.h"
#include "resource.h"

#include "MainWindow.h"


CMainWindow *mainWindow;
CWinApp			mainApp;

char    logPrefix[255] = "dsp_altacast";

int CALLBACK BigWindowHandler(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int CALLBACK EditMetadataHandler(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

#ifdef WIN32
#define PopupMessage MessageBox
#else
#define PopupMessage Alert
#endif

// Forward Funtion Declarations
winampDSPModule *getModule(int which);


int initaltacast(struct winampDSPModule *this_mod);
void config(struct winampDSPModule *this_mod);
void quitaltacast(struct winampDSPModule *this_mod);
int encode_samples(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
// These are needed by Winamp to be a DSP module...
// best not to mess with this unless you are winamp-experienced

HWND		ghwnd_winamp;
HWND		altacastHWND = 0;
int			timerId = 0;

winampDSPModule mod =
{
	"AltaCast DSP",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	initaltacast,
	encode_samples,
	quitaltacast
};
// Module header, includes version, description, and address of the module retriever function
winampDSPHeader hdr = { DSP_HDRVER, "AltaCast for RadioDJ DSP", getModule };

#ifdef __cplusplus
extern "C" {
#endif
// this is the only exported symbol. returns our main header.
__declspec( dllexport ) winampDSPHeader *winampDSPGetHeader2()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif


// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1 or mod2 depending on 'which'.
winampDSPModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &mod;
		default:return NULL;
	}
}


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
	int	printConfig = 0;
	

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
// configuration. Passed this_mod, as a "this" parameter. Allows you to make one configuration
// function that shares code for all your modules (you don't HAVE to use it though, you can make
// config1(), config2(), etc...)
void config(struct winampDSPModule *this_mod)
{
	int a = 1;
	
}

VOID CALLBACK getCurrentSongTitle(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
	char	songTitle[1024] = "";
	char	songTitle2[1024] = "";
	static	char currentTitle[1024] = "";
	int		windowTitle = 1;
	
	memset(songTitle2, '\000', sizeof(songTitle2));
	if (ghwnd_winamp) {
		if (GetWindowText(ghwnd_winamp, songTitle, sizeof(songTitle)) > 0) {
			if (!strncmp(songTitle, "MuchFX", strlen("MuchFX"))) {
				windowTitle = 0;
				int pos=SendMessage(ghwnd_winamp,WM_WA_IPC,0,IPC_GETLISTPOS);
				char *name = (char *)SendMessage(ghwnd_winamp,WM_WA_IPC,pos,IPC_GETPLAYLISTTITLE);
				if (name) {
					strcpy(songTitle, name);
				}
			}
			if (!windowTitle) {
				setMetadataFromMediaPlayer(songTitle);
			}
			else {
				if (strcmp(currentTitle, songTitle)) {
					strcpy(currentTitle, songTitle);
					char	*p1 = strrchr(songTitle, '-');
					if (p1) {
						char *p2 = strchr(songTitle, '.');
						if (p2) {
							p2++;
							if (*p2 == ' ') {
								p2++;
							}
							char *p3 = strstr(p2, "- Winamp");
							if (p3) {
								*p3 = '\000';
							}
							char *p4 = strstr(p2, "otslabs.com/ ");
							if (p4) {
								p2 = p2 + strlen("otslabs.com/ ");
							}
							setMetadataFromMediaPlayer(p2);
						}
						else {
							char *p4 = strstr(songTitle, "otslabs.com/ ");
							if (p4) {
								p2 = songTitle + strlen("otslabs.com/ ");
							}
							else {
								p2 = songTitle;
							}
							setMetadataFromMediaPlayer(p2);
						}
					}
				}
			}
		}
	}
}
// Here is the entry point for the Plugin..this gets called first.
int initaltacast(struct winampDSPModule *this_mod)
{
	char filename[512],*p;
	char	directory[1024] = "";
	char currentDir[1024] = "";
	

	memset(filename, '\000', sizeof(filename));
	GetModuleFileName(this_mod->hDllInstance,filename,sizeof(filename));
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
	char *p2 = strchr(p, '.');
	if (p2) {
		strncpy(logFile, p, p2-p);
	}
	else {
		strcpy(logFile, p);
	}

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


	ghwnd_winamp = this_mod->hwndParent;

    AfxWinInit( this_mod->hDllInstance, NULL, "", SW_HIDE);

    mainWindow = new CMainWindow();
    mainWindow->InitializeWindow();

    strcpy(mainWindow->m_currentDir, currentDir);
    mainWindow->Create((UINT)IDD_ALTACAST, AfxGetMainWnd());
    int x = getLastX();
    int y = getLastY();
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    mainWindow->SetWindowPos(NULL, (int)x, (int)y, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);

    mainWindow->SetIcon(mainApp.LoadIcon(IDR_MAINFRAME), TRUE);
    
    mainWindow->ShowWindow(SW_SHOW);

	initializealtacast();

	timerId = SetTimer(NULL, 1, 1000, (TIMERPROC)getCurrentSongTitle);

	return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quitaltacast(struct winampDSPModule *this_mod)
{
	KillTimer(NULL, timerId);
    
    AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();

    if (pState->m_pmapHWND) {
        mainWindow->DestroyWindow();
    }

    delete mainWindow;
	int a = 1;
}
static signed int scale(int sample)
{
	// round 
	sample += (1L << (MAD_F_FRACBITS - 16));

	// clip 
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	// quantize 
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}
int encode_samples(struct winampDSPModule *this_mod, short int *short_samples, int numsamples, int bps, int nch, int srate)
{
	float	samples[8196*16];
	static short lastSample = 0;
	static signed int sample;

  
	int	samplecount = 0;
	short int *psample = short_samples;

    if (!LiveRecordingCheck()) {
        for (int i=0;i<numsamples*nch;i++) {
            sample = *psample++;
            samples[i] = sample/32767.f;
        }
    
        int ret = handleAllOutput((float *)&samples, numsamples, nch, srate);
    }
	return numsamples;
}	
