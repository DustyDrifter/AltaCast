/*$T MainWindow.cpp GC 1.140 10/23/05 09:48:26 */

/*
 * MainWindow.cpp : implementation file ;
 */
#include "stdafx.h"
#include "edcast.h"
#include "MainWindow.h"
#include "libedcast.h"
#include <process.h>
#include <bass.h>
#include <math.h>
#include <afxinet.h>

#include "About.h"
#include "SystemTray.h"

CMainWindow				*pWindow;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char				THIS_FILE[] = __FILE__;
#endif
#define WM_MY_NOTIFY	WM_USER + 10

int						edcast_init(edcastGlobals *g);

unsigned int			edcastThread = 0;

edcastGlobals			*g[MAX_ENCODERS];
edcastGlobals			gMain;

int						m_BASSOpen = 0;

bool					gLiveRecording = false;
HRECORD					inRecHandle;
static int				oldLeft = 0;
static int				oldRight = 0;
HDC						specdc = 0;
HBITMAP					specbmp = 0;
BYTE					*specbuf;
DWORD					timer = 0;

extern char    logPrefix[255];
char	currentConfigDir[MAX_PATH] = "";

/*
 * define SPECWIDTH 320 // display width ;
 * #define SPECHEIGHT 127 // height (changing requires palette adjustments too)
 */
#define SPECWIDTH	76						/* display width */
#define SPECHEIGHT	30						/* height (changing requires palette adjustments too) */

static UINT BASED_CODE	indicators[] = { ID_STATUSPANE };

extern "C"
{
int startedcastThread(void *obj) {
	CMainWindow *pWindow = (CMainWindow *) obj;
	pWindow->startedcast(-1);
	_endthread();
	return(1);
}
}extern "C"
{
int startSpecificedcastThread(void *obj) {
	int		enc = (int) obj;

	/*
	 * CMainWindow *pWindow = (CMainWindow *)obj;
	 */
	int		ret = pWindow->startedcast(enc);
	time_t	currentTime;
	currentTime = time(&currentTime);
	g[enc]->forcedDisconnectSecs = currentTime;
	_endthread();
	return(1);
}
}
VOID CALLBACK ReconnectTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	time_t	currentTime;

	currentTime = time(&currentTime);
	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if(g[i]->forcedDisconnect) {
			int timeout = getReconnectSecs(g[i]);
			time_t timediff = currentTime - g[i]->forcedDisconnectSecs;
			if(timediff > timeout) {
				g[i]->forcedDisconnect = false;
				_beginthreadex(NULL,
							   0,
							   (unsigned(_stdcall *) (void *)) startSpecificedcastThread,
							   (void *) i,
							   0,
							   &edcastThread);
			}
			else {
				char	buf[255] = "";
				sprintf(buf, "Connecting in %d seconds", timeout - timediff);
				pWindow->outputStatusCallback(i + 1, buf);
			}
		}
	}
}

VOID CALLBACK MetadataTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	if(!strcmp(gMain.externalMetadata, "FILE")) {
		FILE	*filep = fopen(gMain.externalFile, "r");
		if(!filep) {
			char	buf[1024] = "";
			sprintf(buf, "Cannot open metadata file (%s)", gMain.externalFile);
			pWindow->generalStatusCallback(buf);
		}
		else {
			char	buffer[1024];
			memset(buffer, '\000', sizeof(buffer));
			fgets(buffer, sizeof(buffer) - 1, filep);

			char	*p1;
			p1 = strstr(buffer, "\r\n");
			if(p1) {
				*p1 = '\000';
			}

			p1 = strstr(buffer, "\n");
			if(p1) {
				*p1 = '\000';
			}

			fclose(filep);
			setMetadata(buffer);
		}
	}

	if(!strcmp(gMain.externalMetadata, "URL")) {
		char	szCause[255];

		TRY
		{
			CInternetSession	session("edcast");
			CStdioFile			*file = NULL;
			file = session.OpenURL(gMain.externalURL, 1, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
			if(file == NULL) {
				char	buf[1024] = "";
				sprintf(buf, "Cannot open metadata URL (%s)", gMain.externalURL);
				pWindow->generalStatusCallback(buf);
			}
			else {
				CString metadata;
				file->ReadString(metadata);
				setMetadata((char *) LPCSTR(metadata));
				file->Close();
				delete file;
			}

			session.Close();
			delete session;
		}

		CATCH_ALL(error) {
			error->GetErrorMessage(szCause, 254, NULL);
			pWindow->generalStatusCallback(szCause);
		}

		END_CATCH_ALL;
	}
}

char	lastWindowTitleString[4096] = "";

#define UNICODE
VOID CALLBACK MetadataCheckTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	pWindow->KillTimer(5);
	if(gMain.metadataWindowClassInd) {
		if(strlen(gMain.metadataWindowClass) > 0) {
			HWND	winHandle = FindWindow(gMain.metadataWindowClass, NULL);
			if(winHandle) {
				char	buf[1024] = "";
				GetWindowText(winHandle, buf, sizeof(buf) - 1);
				if(!strcmp(buf, lastWindowTitleString)) {
					;
				}
				else {
					setMetadata(buf);
					strcpy(lastWindowTitleString, buf);
				}
			}
		}
	}

	pWindow->SetTimer(5, 1000, (TIMERPROC) MetadataCheckTimer);
}

#undef UNICODE
VOID CALLBACK AutoConnectTimer(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	pWindow->DoConnect();
	pWindow->generalStatusCallback("");
	pWindow->KillTimer(pWindow->autoconnectTimerId);
}

void freeComment() {
	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if (g[i]->numVorbisComments) {
			freeVorbisComments(g[i]);
		}
	}
	return;
}
void addComment(char *comment) {
	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if(g[i]->gOggFlag) {
			if(g[i]->weareconnected) {
				addVorbisComment(g[i], comment);
			}
		}
	}

	return;
}

int handleAllOutput(float *samples, int nsamples, int nchannels, int in_samplerate) {
	long	ileftMax = 0;
	long	irightMax = 0;
	long	leftMax = 0;
	long	rightMax = 0;
	double	sumLeft = 0.0;
	double	sumRight = 0.0;

	int		samplecounter = 0;
	if(nchannels == 2) {
		for(int i = 0; i < nsamples * 2; i = i + 2) {
			ileftMax = abs((int) ((float) samples[i] * 32767.f));
			irightMax = abs((int) ((float) samples[i + 1] * 32767.f));
			sumLeft = sumLeft + (ileftMax * ileftMax);
			sumRight = sumRight + (irightMax * irightMax);
			if(ileftMax > leftMax) {
				leftMax = ileftMax;
			}

			if(irightMax > rightMax) {
				rightMax = irightMax;
			}
		}
	}
	else {
		for(int i = 0; i < nsamples; i = i + 1) {
			ileftMax = abs((int) ((float) samples[i] * 32767.f));
			irightMax = ileftMax;
			sumLeft = sumLeft + (ileftMax * ileftMax);
			sumRight = sumRight + (irightMax * irightMax);
			if(ileftMax > leftMax) {
				leftMax = ileftMax;
			}

			if(irightMax > rightMax) {
				rightMax = irightMax;
			}
		}
	}

	/*
	 * if (nsamples > 0) { leftMax = leftMax/(nsamples);
	 * rightMax = rightMax/(nsamples);
	 * }
	 */
	double	RMSLeft = sqrt(sumLeft);
	double	RMSRight = sqrt(sumRight);

	double	newL = (double) 20 * log10((double) leftMax / 32768.0);
	double	newR = (double) 20 * log10((double) rightMax / 32768.0);

	/*
	 * double newL = (double)20 * log10((double)RMSLeft/32768.0);
	 * double newR = (double)20 * log10((double)RMSRight/32768.0);
	 */
	UpdatePeak((int) newL + 60, (int) newR + 60);
	for(int i = 0; i < gMain.gNumEncoders; i++) {
		handle_output(g[i], samples, nsamples, nchannels, in_samplerate);
	}

	return 1;
}

void UpdatePeak(int peakL, int peakR) {
	pWindow->flexmeters.GetMeterInfoObject(0)->value = peakL;
	pWindow->flexmeters.GetMeterInfoObject(1)->value = peakR;
}

bool LiveRecordingCheck() {
	return gLiveRecording;
}

int getLastX() {
	return getLastXWindow(&gMain);
}

int getLastY() {
	return getLastYWindow(&gMain);
}

void setLastX(int x) {
	setLastXWindow(&gMain, x);
}

void setLastY(int y) {
	setLastYWindow(&gMain, y);
}

int getLastDummyX() {
	return getLastDummyXWindow(&gMain);
}

int getLastDummyY() {
	return getLastDummyYWindow(&gMain);
}

void setLastDummyX(int x) {
	setLastDummyXWindow(&gMain, x);
}

void setLastDummyY(int y) {
	setLastDummyYWindow(&gMain, y);
}

void setLiveRecFlag(int live) {
	gMain.gLiveRecordingFlag = live;
}

void setAuto(int flag) {
	setAutoConnect(&gMain, flag);
}

void writeMainConfig() {
	writeConfigFile(&gMain);
}

int initializeedcast() {
    char    currentlogFile[1024] = "";
    sprintf(currentlogFile, "%s\\%s", currentConfigDir, logPrefix);

    setDefaultLogFileName(currentlogFile);
    setgLogFile(&gMain, currentlogFile);
    setConfigFileName(&gMain, currentlogFile);

	addUISettings(&gMain);


	return edcast_init(&gMain);
}

void setMetadataFromMediaPlayer(char *metadata) {
	if(gMain.metadataWindowClassInd) {
		return;
	}

	if(!strcmp(gMain.externalMetadata, "DISABLED")) {
		setMetadata(metadata);
	}
}

void setMetadata(char *metadata) {
	char	modifiedSong[4096] = "";
	char	modifiedSongBuffer[4096] = "";
	char	*pData;

	strcpy(modifiedSong, metadata);
	if(strlen(gMain.metadataRemoveStringAfter) > 0) {
		char	*p1 = strstr(modifiedSong, gMain.metadataRemoveStringAfter);
		if(p1) {
			*p1 = '\000';
		}
	}

	if(strlen(gMain.metadataRemoveStringBefore) > 0) {
		char	*p1 = strstr(modifiedSong, gMain.metadataRemoveStringBefore);
		if(p1) {
			memset(modifiedSongBuffer, '\000', sizeof(modifiedSongBuffer));
			strcpy(modifiedSongBuffer, p1 + strlen(gMain.metadataRemoveStringBefore));
			strcpy(modifiedSong, modifiedSongBuffer);
		}
	}

	if(strlen(gMain.metadataAppendString) > 0) {
		strcat(modifiedSong, gMain.metadataAppendString);
	}

	pData = modifiedSong;

	pWindow->m_Metadata = modifiedSong;
	pWindow->inputMetadataCallback(0, (void *) pData);

	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if(getLockedMetadataFlag(&gMain)) {
			if(setCurrentSongTitle(g[i], (char *) getLockedMetadata(&gMain))) {
				pWindow->inputMetadataCallback(i, (void *) getLockedMetadata(&gMain));
			}
		}
		else {
			if(setCurrentSongTitle(g[i], (char *) pData)) {
				pWindow->inputMetadataCallback(i, (void *) pData);
			}
		}
	}
}

void LoadConfigs(char *currentDir, char *logFile) {
	char	configFile[1024] = "";
	char	currentlogFile[1024] = "";

    
	strcpy(currentConfigDir, currentDir);



	sprintf(configFile, "%s\\%s", currentConfigDir, logPrefix);
	sprintf(currentlogFile, "%s\\%s", currentConfigDir, logPrefix);

    setDefaultLogFileName(currentlogFile);
    setgLogFile(&gMain, currentlogFile);
    setConfigFileName(&gMain, configFile);
	addUISettings(&gMain);
	readConfigFile(&gMain);
}

BOOL CALLBACK BASSwaveInputProc(HRECORD handle, const void *buffer, DWORD length, DWORD user) {
	int			n;
	char		*name;
	static char currentDevice[1024] = "";

	if(gLiveRecording) {
		for(n = 0; name = (char *)BASS_RecordGetInputName(n); n++) {
			int s = BASS_RecordGetInput(n);
			if(!(s & BASS_INPUT_OFF)) {
				if(strcmp(currentDevice, name)) {
					strcpy(currentDevice, name);

					/*
					char	msg[255] = "";
					sprintf(msg, "Recording from %s", currentDevice);
					pWindow->generalStatusCallback((void *) msg);
					*/
				}

				/*
				 * setCurrentRecordingName(currentDevice);
				 */
			}
		}

		unsigned int	c_size = length;	/* in bytes. */
		short			*z = (short *) buffer;	/* signed short for pcm data. */

		int				numsamples = c_size / sizeof(short);

		int				nch = 2;
		int				srate = 44100;
		float			*samples;

		/*
		 * float samples[8196*16];
		 */
		samples = (float *) malloc(sizeof(float) * numsamples * 2);
		memset(samples, '\000', sizeof(float) * numsamples * 2);

		long	avgLeft = 0;
		long	avgRight = 0;
		int		flip = 0;

		for(int i = 0; i < numsamples; i++) {
			signed int	sample;
			sample = z[i];
			samples[i] = sample / 32767.f;

			/* clipping */
			if(samples[i] > 1.0) {
				samples[i] = 1.0;
			}

			if(samples[i] < -1.0) {
				samples[i] = -1.0;
			}
		}

		handleAllOutput((float *) samples, numsamples / nch, nch, srate);

		/*
		 * int ret;
		 * for (int j=0;
		 * j<gMain.gNumEncoders;
		 * j++) { ret = handle_output(g[j], (float *)samples, numsamples/nch, nch, srate);
		 * }
		 */
		free(samples);
		return 1;
	}
	else {
		return 0;
	}

	return 0;
}

/*
 =======================================================================================================================
    void CALLBACK UpdateSpectrum(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) { HDC dc;
    int x,y;
    float fft[512];
    // get the FFT data BASS_ChannelGetData(i nRecHandle,fft,BASS_DATA_FFT1024);
    int b0=0;
    memset(specbuf,0,SPECWIDTH*SPECHEIGHT);
    #define BANDS 15 for (x=0;
    x<BANDS;
    x++) { float sum=0;
    int sc,b1=pow(2,x*10.0/(BANDS-1));
    if (b1>511) b1=511;
    if (b1<=b0) b1=b0+1;
    // make sure it uses at least 1 FFT bin sc=10+b1-b0;
    for (;
    b0<b1;
    b0++) sum+=fft[1+b0];
    y=(sqrt(sum/log10(sc))*1.7*SPECHEIGHT)-4;
    // scale it if (y>SPECHEIGHT) y=SPECHEIGHT;
    // cap it while (--y>=0) memset(specbuf+y*SPECWIDTH+x*(SPECWIDTH/BANDS),y+1,0.9*(SPECWIDTH/BANDS));
    // draw bar } // update the display dc=GetDC(pWindow->m_hWnd);
    //int bmpPos = (int)(pWindow->gWindowHeight / 2.475);
    BitBlt(dc,315,70,SPECWIDTH,SPECHEIGHT,specdc,0,0,SRCCOPY);
    ReleaseDC(pWindow->m_hWnd,dc);
    }
 =======================================================================================================================
 */
void stopRecording() {
	BASS_ChannelStop(inRecHandle);
	m_BASSOpen = 0;
	BASS_RecordFree();
	gLiveRecording = false;
}

int startRecording(int m_CurrentInputCard) {
	char	buffer[1024] = "";
	char	buf[255] = "";

	int		ret = BASS_RecordInit(m_CurrentInputCard);
	m_BASSOpen = 1;

	if(!ret) {
		DWORD	errorCode = BASS_ErrorGetCode();
		switch(errorCode) {
			case BASS_ERROR_ALREADY:
				pWindow->generalStatusCallback((char *) "Recording device already opened!");
				return 0;

			case BASS_ERROR_DEVICE:
				pWindow->generalStatusCallback((char *) "Recording device invalid!");
				return 0;

			case BASS_ERROR_DRIVER:
				pWindow->generalStatusCallback((char *) "Recording device driver unavailable!");
				return 0;

			default:
				pWindow->generalStatusCallback((char *) "There was an error opening the preferred Digital Audio In device!");
				return 0;
		}
	}

	inRecHandle = BASS_RecordStart(44100, 2, MAKELONG(0, 25), BASSwaveInputProc, NULL);

	int		n = 0;
	char	*name;
	for(n = 0; name = (char *)BASS_RecordGetInputName(n); n++) {
		int s = BASS_RecordGetInput(n);
		if(!(s & BASS_INPUT_OFF)) {
			char	msg[255] = "";
			sprintf(msg, "Recording from %s", name);
			pWindow->generalStatusCallback((void *) msg);
		}
	}

	gLiveRecording = true;

	return 1;
}

/* CMainWindow dialog */
const char	*kpcTrayNotificationMsg_ = "edcast";

CMainWindow::CMainWindow(CWnd *pParent /* NULL */ ) :
	CDialog(CMainWindow::IDD, pParent),
	bMinimized_(false),
	pTrayIcon_(0),
	nTrayNotificationMsg_(RegisterWindowMessage(kpcTrayNotificationMsg_)) {

	//{{AFX_DATA_INIT(CMainWindow)
	m_Bitrate = _T("");
	m_Destination = _T("");
	m_Bandwidth = _T("");
	m_Metadata = _T("");
	m_ServerDescription = _T("");
	m_LiveRec = FALSE;
	m_RecDevices = _T("");
	m_RecCards = _T("");
	m_RecVolume = 0;
	m_AutoConnect = FALSE;
	m_StaticStatus = _T("");
	//}}AFX_DATA_INIT
	hIcon_ = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset(g, '\000', sizeof(g));
	m_BASSOpen = 0;
	pWindow = this;
	memset(m_currentDir, '\000', sizeof(m_currentDir));
	strcpy(m_currentDir, ".");
}

CMainWindow::~CMainWindow() {
	for(int i = 0; i < MAX_ENCODERS; i++) {
		if(g[i]) {
			free(g[i]);
		}
	}
}

void CMainWindow::InitializeWindow() {
	configDialog = 0;
	configDialog = new CConfig();
	configDialog->Create((UINT) IDD_CONFIG, this);
	configDialog->parentDialog = this;

	editMetadata = new CEditMetadata();
	editMetadata->Create((UINT) IDD_EDIT_METADATA, this);
	editMetadata->parentDialog = this;

	aboutBox = new CAbout();
	aboutBox->Create((UINT) IDD_ABOUT, this);
}

void CMainWindow::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainWindow)
	DDX_Control(pDX, IDC_RECCARDS, m_RecCardsCtrl);
	DDX_Control(pDX, IDC_VUONOFF, m_OnOff);
	DDX_Control(pDX, IDC_AUTOCONNECT, m_AutoConnectCtrl);
	DDX_Control(pDX, IDC_RECVOLUME, m_RecVolumeCtrl);
	DDX_Control(pDX, IDC_RECDEVICES, m_RecDevicesCtrl);
	DDX_Control(pDX, IDC_CONNECT, m_ConnectCtrl);
	DDX_Control(pDX, IDC_LIVEREC, m_LiveRecCtrl);
	DDX_Control(pDX, IDC_ENCODERS, m_Encoders);
	DDX_Text(pDX, IDC_METADATA, m_Metadata);
	DDX_Check(pDX, IDC_LIVEREC, m_LiveRec);
	DDX_CBString(pDX, IDC_RECDEVICES, m_RecDevices);
	DDX_CBString(pDX, IDC_RECCARDS, m_RecCards);
	DDX_Slider(pDX, IDC_RECVOLUME, m_RecVolume);
	DDX_Check(pDX, IDC_AUTOCONNECT, m_AutoConnect);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_StaticStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMainWindow, CDialog)
//{{AFX_MSG_MAP(CMainWindow)
	ON_BN_CLICKED(IDC_CONNECT, OnConnect)
	ON_BN_CLICKED(IDC_ADD_ENCODER, OnAddEncoder)
	ON_NOTIFY(NM_DBLCLK, IDC_ENCODERS, OnDblclkEncoders)
	ON_NOTIFY(NM_RCLICK, IDC_ENCODERS, OnRclickEncoders)
	ON_COMMAND(ID_POPUP_CONFIGURE, OnPopupConfigure)
	ON_COMMAND(ID_POPUP_CONNECT, OnPopupConnect)
	ON_BN_CLICKED(IDC_LIVEREC, OnLiverec)
	ON_COMMAND(ID_POPUP_DELETE, OnPopupDelete)
	ON_CBN_SELCHANGE(IDC_RECDEVICES, OnSelchangeRecdevices)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MANUALEDIT_METADATA, OnManualeditMetadata)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_ABOUT_ABOUT, OnAboutAbout)
	ON_COMMAND(ID_ABOUT_HELP, OnAboutHelp)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_METER, OnMeter)
	ON_NOTIFY(LVN_KEYDOWN, IDC_ENCODERS, OnKeydownEncoders)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_NOTIFY(NM_SETFOCUS, IDC_ENCODERS, OnSetfocusEncoders)
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_RECCARDS, OnSelchangeReccards)
	//}}AFX_MSG_MAP
	ON_WM_SYSCOMMAND()
	ON_COMMAND(IDI_RESTORE, OnSTRestore)
END_MESSAGE_MAP()
void CMainWindow::generalStatusCallback(void *pValue) {
	LogMessage(&gMain,LOG_INFO, "%s", (char *)pValue);
	SetDlgItemText(IDC_STATIC_STATUS, (char *) pValue);
}

void CMainWindow::inputMetadataCallback(int enc, void *pValue) {
	SetDlgItemText(IDC_METADATA, (char *) pValue);
	if (enc == 0) {
		LogMessage(&gMain,LOG_INFO, "%s", (char *)pValue);
	}
	else {
		LogMessage(g[enc-1],LOG_INFO, "%s", (char *)pValue);
	}
}

void CMainWindow::outputStatusCallback(int enc, void *pValue) {
	if(enc != 0) {
		LogMessage(g[enc-1],LOG_INFO, "%s", (char *)pValue);

		int numItems = m_Encoders.GetItemCount();
		if(enc - 1 >= numItems) {
			m_Encoders.InsertItem(enc - 1, (char *) "");
		}

		m_Encoders.SetItemText(enc - 1, 1, (char *) pValue);
	}
}

void CMainWindow::writeBytesCallback(int enc, void *pValue) {

	/* pValue is a long */
	static time_t startTime[MAX_ENCODERS];
	static time_t endTime[MAX_ENCODERS];
	static long bytesWrittenInterval[MAX_ENCODERS];
	static long totalBytesWritten[MAX_ENCODERS];
	static int	initted = 0;
	char		kBPSstr[255] = "";

	if(!initted) {
		initted = 1;
		memset(&startTime, '\000', sizeof(startTime));
		memset(&endTime, '\000', sizeof(endTime));
		memset(&bytesWrittenInterval, '\000', sizeof(bytesWrittenInterval));
		memset(&totalBytesWritten, '\000', sizeof(totalBytesWritten));
	}

	if(enc != 0) {
		int		enc_index = enc - 1;
		long	bytesWritten = (long) pValue;

		if(bytesWritten == -1) {
			strcpy(kBPSstr, "");
			outputStatusCallback(enc, kBPSstr);
			startTime[enc_index] = 0;
			return;
		}

		if(startTime[enc_index] == 0) {
			startTime[enc_index] = time(&(startTime[enc_index]));
			bytesWrittenInterval[enc_index] = 0;
		}

		bytesWrittenInterval[enc_index] += bytesWritten;
		totalBytesWritten[enc_index] += bytesWritten;
		endTime[enc_index] = time(&(endTime[enc_index]));
		if((endTime[enc_index] - startTime[enc_index]) > 4) {
			time_t		bytespersec = bytesWrittenInterval[enc_index] / (endTime[enc_index] - startTime[enc_index]);
			long	kBPS = (long)((bytespersec * 8) / 1000);
			if(strlen(g[enc_index]->gMountpoint) > 0) {
				sprintf(kBPSstr, "%ld Kbps (%s)", kBPS, g[enc_index]->gMountpoint);
			}
			else {
				sprintf(kBPSstr, "%ld Kbps", kBPS);
			}

			outputStatusCallback(enc, kBPSstr);
			startTime[enc_index] = 0;
		}
	}
}

void CMainWindow::outputServerNameCallback(int enc, void *pValue) {

	/*
	 * SetDlgItemText(IDC_SERVER_DESC, (char *)pValue);
	 */
	;
}

void CMainWindow::outputBitrateCallback(int enc, void *pValue) {
	if(enc != 0) {
		int numItems = m_Encoders.GetItemCount();
		if(enc - 1 >= numItems) {
			m_Encoders.InsertItem(enc - 1, (char *) pValue);
		}
		else {
			m_Encoders.SetItemText(enc - 1, 0, (char *) pValue);
		}
	}
}

void CMainWindow::outputStreamURLCallback(int enc, void *pValue) {

	/*
	 * SetDlgItemText(IDC_DESTINATION_LOCATION, (char *)pValue);
	 */
}

void CMainWindow::stopedcast() {
	for(int i = 0; i < gMain.gNumEncoders; i++) {
		setForceStop(g[i], 1);
		disconnectFromServer(g[i]);
		g[i]->forcedDisconnect = false;
	}
}

int CMainWindow::startedcast(int which) {
	if(which == -1) {
		for(int i = 0; i < gMain.gNumEncoders; i++) {
			if(!g[i]->weareconnected) {
				setForceStop(g[i], 0);
				if(!connectToServer(g[i])) {
					g[i]->forcedDisconnect = true;
					continue;
				}
			}
		}
	}
	else {
		if(!g[which]->weareconnected) {
			setForceStop(g[which], 0);

			int ret = connectToServer(g[which]);
			if(ret == 0) {
				g[which]->forcedDisconnect = true;
			}
		}
	}

	return 1;
}

void CMainWindow::DoConnect() {
	OnConnect();
}

void CMainWindow::OnConnect() {
	static bool connected = false;

	if(!connected) {
		_beginthreadex(NULL, 0, (unsigned(_stdcall *) (void *)) startedcastThread, (void *) this, 0, &edcastThread);
		connected = true;
		m_ConnectCtrl.SetWindowText("Disconnect");
		KillTimer(2);
		reconnectTimerId = SetTimer(2, 1000, (TIMERPROC) ReconnectTimer);

	}
	else {
		stopedcast();
		connected = false;
		m_ConnectCtrl.SetWindowText("Connect");
		KillTimer(2);
	}
}

void CMainWindow::OnAddEncoder() {

	/* TODO: Add your control notification handler code here */
	int orig_index = gMain.gNumEncoders;
	g[orig_index] = (edcastGlobals *) malloc(sizeof(edcastGlobals));

	memset(g[orig_index], '\000', sizeof(edcastGlobals));

	g[orig_index]->encoderNumber = orig_index + 1;


    char    currentlogFile[1024] = "";

	sprintf(currentlogFile, "%s\\%s_%d", currentConfigDir, logPrefix, g[orig_index]->encoderNumber);



	setDefaultLogFileName(currentlogFile);

	setgLogFile(g[orig_index], currentlogFile);
	setConfigFileName(g[orig_index], gMain.gConfigFileName);
	gMain.gNumEncoders++;
	initializeGlobals(g[orig_index]);
    addBasicEncoderSettings(g[orig_index]);


	edcast_init(g[orig_index]);
}

BOOL CMainWindow::OnInitDialog() {
	CDialog::OnInitDialog();

	SetWindowText("edcast");

	RECT	rect;

	rect.left = 340;
	rect.top = 190;

	m_Encoders.InsertColumn(0, "Encoder Settings");
	m_Encoders.InsertColumn(1, "Transfer Rate");

	m_Encoders.SetColumnWidth(0, 195);
	m_Encoders.SetColumnWidth(1, 200);

	m_Encoders.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_Encoders.SendMessage(LB_SETTABSTOPS, 0, NULL);
	liveRecOn.LoadBitmap(IDB_LIVE_ON);
	liveRecOff.LoadBitmap(IDB_LIVE_OFF);

#ifdef EDCASTSTANDALONE
	gMain.gLiveRecordingFlag = 1;
#endif
	m_LiveRec = gMain.gLiveRecordingFlag;
	if(m_LiveRec) {
		m_LiveRecCtrl.SetBitmap(HBITMAP(liveRecOn));
	}
	else {
		m_LiveRecCtrl.SetBitmap(HBITMAP(liveRecOff));
	}

	for(int i = 0; i < gMain.gNumEncoders; i++) {
		if(!g[i]) {
			g[i] = (edcastGlobals *) malloc(sizeof(edcastGlobals));
			memset(g[i], '\000', sizeof(edcastGlobals));
		}

		g[i]->encoderNumber = i + 1;
		char    currentlogFile[1024] = "";

		sprintf(currentlogFile, "%s\\%s_%d", currentConfigDir, logPrefix, g[i]->encoderNumber);
		setDefaultLogFileName(currentlogFile);
		setgLogFile(g[i], currentlogFile);
		setConfigFileName(g[i], gMain.gConfigFileName);
		initializeGlobals(g[i]);
	    addBasicEncoderSettings(g[i]);

		edcast_init(g[i]);
	}

	int		count = 0;	/* the device counter */
	char	*pDesc = (char *) 1;

	BASS_RecordInit(0);

	m_BASSOpen = 1;

	int		n;
	char	*name;
	int		currentInput = 0;


	for(n = 0; name = (char *)BASS_RecordGetDeviceDescription(n); n++) {
		m_RecCardsCtrl.AddString(name);
//		if (n == BASS_RecordGetDevice()) {
		if (!strcmp(getWindowsRecordingDevice(&gMain), "")) {
			m_RecCards = name;
			m_CurrentInputCard = n;
		}
		else {
			if (!strcmp(getWindowsRecordingDevice(&gMain), name)) {
				m_RecCards = name;
				m_CurrentInputCard = n;
			}
		}
	}

	for(n = 0; name = (char *)BASS_RecordGetInputName(n); n++) {
		int s = BASS_RecordGetInput(n);
		m_RecDevicesCtrl.AddString(name);
		if(s & BASS_INPUT_OFF) {
			;
		}
		else {
			m_RecDevices = name;
			m_RecVolume = LOWORD(s);
			m_CurrentInput = n;
		}
	}

	m_BASSOpen = 0;
	BASS_RecordFree();

	if(getLockedMetadataFlag(&gMain)) {
		m_Metadata = getLockedMetadata(&gMain);
	}

	m_AutoConnect = gMain.autoconnect;
	UpdateData(FALSE);
#ifdef EDCASTSTANDALONE
	m_LiveRecCtrl.SetBitmap(HBITMAP(liveRecOn));
	m_RecDevicesCtrl.EnableWindow(TRUE);
	m_RecCardsCtrl.EnableWindow(TRUE);
	m_RecVolumeCtrl.EnableWindow(TRUE);
	startRecording(m_CurrentInputCard);
#endif
	OnLiverec();
	reconnectTimerId = SetTimer(2, 1000, (TIMERPROC) ReconnectTimer);
	if(m_AutoConnect) {
		char	buf[255];
		sprintf(buf, "AutoConnecting in 5 seconds");
		generalStatusCallback(buf);
		autoconnectTimerId = SetTimer(3, 5000, (TIMERPROC) AutoConnectTimer);
	}

	int metadataInterval = atoi(gMain.externalInterval);
	metadataInterval = metadataInterval * 1000;
	metadataTimerId = SetTimer(4, metadataInterval, (TIMERPROC) MetadataTimer);

	SetTimer(5, 1000, (TIMERPROC) MetadataCheckTimer);

	FlexMeters_InitStruct	*fmis = flexmeters.Initialize_Step1();	/* returns a pointer to a struct, */

	/* with default values filled in. */
	fmis->max_x = 512;	/* buffer size. must be at least as big as the meter window */
	fmis->max_y = 512;	/* buffer size. must be at least as big as the meter window */

	fmis->hWndFrame = GetDlgItem(IDC_METER)->m_hWnd;	/* the window to grab coordinates from */

	fmis->border.left = 3;					/* borders. */
	fmis->border.right = 3;
	fmis->border.top = 3;
	fmis->border.bottom = 3;

	fmis->meter_count = 2;					/* number of meters */

	fmis->horizontal_meters = 1;			/* 0 = vertical */

	flexmeters.Initialize_Step2(fmis);		/* news meter info objects. after this, you must set them up. */

	int a = 0;

	for(a = 0; a < fmis->meter_count; a++) {
		CFlexMeters_MeterInfo	*pMeterInfo = flexmeters.GetMeterInfoObject(a);

		pMeterInfo->extra_spacing = 3;

		/* nice gradient */
		pMeterInfo->colour[0].colour = 0x00FF00;
		pMeterInfo->colour[0].at_value = 0;

		pMeterInfo->colour[1].colour = 0xFFFF00;
		pMeterInfo->colour[1].at_value = 55;

		pMeterInfo->colour[2].colour = 0xFF0000;
		pMeterInfo->colour[2].at_value = 58;

		pMeterInfo->colours_used = 3;

		pMeterInfo->value = 0;
		pMeterInfo->meter_width = 60;

		/*
		 * if(a & 1) pMeterInfo->direction=eMeterDirection_Backwards;
		 */
	}

	flexmeters.Initialize_Step3();			/* finalize init. */

	if(gMain.vuShow) {
		m_VUStatus = VU_ON;
		m_OnOff.ShowWindow(SW_HIDE);
	}
	else {
		m_VUStatus = VU_OFF;
		m_OnOff.ShowWindow(SW_SHOW);
	}

	SetTimer(73, 50, 0);					/* set up timer. 20ms=50hz - probably good. */

	return TRUE;							/* return TRUE unless you set the focus to a control */
	/* EXCEPTION: OCX Property Pages should return FALSE */
}

void CMainWindow::OnDblclkEncoders(NMHDR *pNMHDR, LRESULT *pResult) {

	/* TODO: Add your control notification handler code here */
	OnPopupConfigure();
	*pResult = 0;
}

void CMainWindow::OnRclickEncoders(NMHDR *pNMHDR, LRESULT *pResult) {

	/* TODO: Add your control notification handler code here */
	int iItem = m_Encoders.GetNextItem(-1, LVNI_SELECTED);

	if(iItem >= 0) {

		CMenu	menu;
		VERIFY(menu.LoadMenu(IDR_CONTEXT));

		/* Pop up sub menu 0 */
		CMenu	*popup = menu.GetSubMenu(0);

		if(popup) {
			if(g[iItem]->weareconnected) {
				popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Disconnect");
			}
			else {
				if(g[iItem]->forcedDisconnect) {
					popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Stop AutoConnect");
				}
				else {
					popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Connect");
				}
			}

			POINT	pt;
			GetCursorPos(&pt);
			popup->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
		}
	}

	*pResult = 0;
}

void CMainWindow::OnPopupConfigure() {

	/* TODO: Add your command handler code here */
	int iItem = m_Encoders.GetNextItem(-1, LVNI_SELECTED);

	if(iItem >= 0) {
		configDialog->GlobalsToDialog(g[iItem]);
		configDialog->ShowWindow(SW_SHOW);
	}
}

void CMainWindow::OnPopupConnect() {

	/* TODO: Add your command handler code here */
	int iItem = m_Encoders.GetNextItem(-1, LVNI_SELECTED);

	if(iItem >= 0) {
		if(!g[iItem]->weareconnected) {
			if(g[iItem]->forcedDisconnect) {
				g[iItem]->forcedDisconnect = 0;
				outputStatusCallback(iItem + 1, "AutoConnect stopped.");
			}
			else {
				m_SpecificEncoder = iItem;
				_beginthreadex(NULL,
							   0,
							   (unsigned(_stdcall *) (void *)) startSpecificedcastThread,
							   (void *) iItem,
							   0,
							   &edcastThread);
			}
		}
		else {
			disconnectFromServer(g[iItem]);
			setForceStop(g[iItem], 1);
			g[iItem]->forcedDisconnect = false;
		}
	}
}

void CMainWindow::OnLiverec()
{
#ifdef EDCASTSTANDALONE
	return;
#endif

	/* TODO: Add your control notification handler code here */
	UpdateData(TRUE);
	if(m_LiveRec) {
		m_LiveRecCtrl.SetBitmap(HBITMAP(liveRecOn));
		m_RecDevicesCtrl.EnableWindow(TRUE);
		m_RecCardsCtrl.EnableWindow(TRUE);
		m_RecVolumeCtrl.EnableWindow(TRUE);
		startRecording(m_CurrentInputCard);
	}
	else {
		m_LiveRecCtrl.SetBitmap(HBITMAP(liveRecOff));
		m_RecCardsCtrl.EnableWindow(FALSE);
		m_RecDevicesCtrl.EnableWindow(FALSE);
		m_RecVolumeCtrl.EnableWindow(FALSE);
		generalStatusCallback((void *) "Recording from DSP");
		stopRecording();
	}
}

void CMainWindow::ProcessConfigDone(int enc, CConfig *pConfig) {
	if(enc > 0) {
		pConfig->DialogToGlobals(g[enc - 1]);
		writeConfigFile(g[enc - 1]);
		edcast_init(g[enc - 1]);
	}

	SetFocus();
	m_Encoders.SetFocus();
}

void CMainWindow::ProcessEditMetadataDone(CEditMetadata *pConfig) {
	pConfig->UpdateData(TRUE);

	bool	ok = true;

	if(pConfig->m_ExternalFlag == 0) {
		strcpy(gMain.externalMetadata, "URL");
		ok = false;
	}

	if(pConfig->m_ExternalFlag == 1) {
		strcpy(gMain.externalMetadata, "FILE");
		ok = false;
	}

	if(pConfig->m_ExternalFlag == 2) {
		strcpy(gMain.externalMetadata, "DISABLED");
	}

	strcpy(gMain.externalInterval, LPCSTR(pConfig->m_MetaInterval));
	strcpy(gMain.externalFile, LPCSTR(pConfig->m_MetaFile));
	strcpy(gMain.externalURL, LPCSTR(pConfig->m_MetaURL));

	strcpy(gMain.metadataAppendString, LPCSTR(pConfig->m_AppendString));
	strcpy(gMain.metadataRemoveStringBefore, LPCSTR(pConfig->m_RemoveStringBefore));
	strcpy(gMain.metadataRemoveStringAfter, LPCSTR(pConfig->m_RemoveStringAfter));
	strcpy(gMain.metadataWindowClass, LPCSTR(pConfig->m_WindowClass));

	if(ok) {
		setLockedMetadata(&gMain, (char *) LPCSTR(pConfig->m_Metadata));
		setLockedMetadataFlag(&gMain, editMetadata->m_LockMetadata);
		if(strlen((char *) LPCSTR(pConfig->m_Metadata)) > 0) {
			setMetadata((char *) LPCSTR(pConfig->m_Metadata));
		}
	}

	gMain.metadataWindowClassInd = pConfig->m_WindowTitleGrab;

	KillTimer(metadataTimerId);

	int metadataInterval = atoi(gMain.externalInterval);
	metadataInterval = metadataInterval * 1000;
	metadataTimerId = SetTimer(4, metadataInterval, (TIMERPROC) MetadataTimer);

	SetFocus();
}

void CMainWindow::OnPopupDelete() {
	int i = 0;
	for(i = 0; i < gMain.gNumEncoders; i++) {
		if(g[i]->weareconnected) {
			MessageBox("You need to disconnect all the encoders before deleting one from the list", "Message", MB_OK);
			return;
		}
	}

	int iItem = m_Encoders.GetNextItem(-1, LVNI_SELECTED);

	if(iItem >= 0) {
		int ret = MessageBox("Delete this encoder ?", "Message", MB_YESNO);
		if(ret == IDYES) {
			if(g[iItem]) {
				deleteConfigFile(g[iItem]);
				free(g[iItem]);
			}

			m_Encoders.DeleteAllItems();
			for(i = iItem; i < gMain.gNumEncoders; i++) {
				if(g[i + 1]) {
					g[i] = g[i + 1];
					g[i + 1] = 0;
					deleteConfigFile(g[i]);
					g[i]->encoderNumber--;
					writeConfigFile(g[i]);
				}
			}

			gMain.gNumEncoders--;
			for(i = 0; i < gMain.gNumEncoders; i++) {
				edcast_init(g[i]);
			}
		}
	}
}

void CMainWindow::OnSelchangeRecdevices() {
	char	*name;
	char	selectedDevice[1024] = "";
	bool	opened = false;

	int		index = m_RecDevicesCtrl.GetCurSel();
	memset(selectedDevice, '\000', sizeof(selectedDevice));
	m_RecDevicesCtrl.GetLBText(index, selectedDevice);

	m_RecDevices = selectedDevice;


	if(!m_BASSOpen) {
		int ret = BASS_RecordInit(m_CurrentInputCard);
		m_BASSOpen = 1;
		opened = true;
	}

	for(int n = 0; name = (char *)BASS_RecordGetInputName(n); n++) {
		int		s = BASS_RecordGetInput(n);
		CString description = name;

		if(m_RecDevices == description) {
			BASS_RecordSetInput(n, BASS_INPUT_ON);
			m_CurrentInput = n;
			m_RecVolume = LOWORD(s);
		}
	}

	if(opened) {
		m_BASSOpen = 0;
		BASS_RecordFree();
	}

	UpdateData(FALSE);
}

void CMainWindow::CleanUp() {
	timeKillEvent(timer);
	Sleep(100);
	if(specbmp) {
		DeleteObject(specbmp);
	}

	if(specdc) {
		DeleteDC(specdc);
	}

	if(gLiveRecording) {
		stopRecording();
	}
}

void CMainWindow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {
	bool	opened = false;

	if(pScrollBar->m_hWnd == m_RecVolumeCtrl.m_hWnd) {
		UpdateData(TRUE);
		if(!m_BASSOpen) {
			int ret = BASS_RecordInit(m_CurrentInputCard);
			m_BASSOpen = 1;
			opened = true;
		}

		BASS_RecordSetInput(m_CurrentInput, BASS_INPUT_LEVEL | m_RecVolume);
		if(opened) {
			m_BASSOpen = 0;
			BASS_RecordFree();
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMainWindow::OnManualeditMetadata() {
	editMetadata->m_LockMetadata = gMain.gLockSongTitle;
	editMetadata->m_Metadata = gMain.gManualSongTitle;

	if(!strcmp("DISABLED", gMain.externalMetadata)) {
		editMetadata->m_ExternalFlag = 2;
	}

	if(!strcmp("FILE", gMain.externalMetadata)) {
		editMetadata->m_ExternalFlag = 1;
	}

	if(!strcmp("URL", gMain.externalMetadata)) {
		editMetadata->m_ExternalFlag = 0;
	}

	editMetadata->m_MetaFile = gMain.externalFile;
	editMetadata->m_MetaURL = gMain.externalURL;
	editMetadata->m_MetaInterval = gMain.externalInterval;

	editMetadata->m_AppendString = gMain.metadataAppendString;
	editMetadata->m_RemoveStringAfter = gMain.metadataRemoveStringAfter;
	editMetadata->m_RemoveStringBefore = gMain.metadataRemoveStringBefore;
	editMetadata->m_WindowClass = gMain.metadataWindowClass;

	editMetadata->m_WindowTitleGrab = gMain.metadataWindowClassInd;

	editMetadata->UpdateRadio();
	editMetadata->UpdateData(FALSE);
	editMetadata->ShowWindow(SW_SHOW);
}

void CMainWindow::OnClose()
{
	/* TODO: Add your message handler code here and/or call default */
#ifndef EDCASTSTANDALONE
	bMinimized_ = true;
	SetupTrayIcon();
	SetupTaskBarButton();
#else
	OnDestroy();
	EndModalLoop(1);
	exit(1);
#endif
}

void CMainWindow::OnDestroy() {
	RECT	pRect;

	bMinimized_ = false;
	SetupTrayIcon();

	GetWindowRect(&pRect);
	UpdateData(TRUE);
	setLastX(pRect.left);
	setLastY(pRect.top);
	setLiveRecFlag(m_LiveRec);
	setAuto(m_AutoConnect);

	stopedcast();
	CleanUp();
	if(configDialog) {
		configDialog->DestroyWindow();
		delete configDialog;
	}

	if(editMetadata) {
		editMetadata->DestroyWindow();
		delete editMetadata;
	}

	if(aboutBox) {
		aboutBox->DestroyWindow();
		delete aboutBox;
	}

	writeMainConfig();

	/*
	 * DestroyWindow();
	 */
	CDialog::OnDestroy();
}

void CMainWindow::OnAboutAbout() {

	/* TODO: Add your command handler code here */
	aboutBox->m_Version = "Built on : "__DATE__ " "__TIME__;
	aboutBox->UpdateData(FALSE);
	aboutBox->ShowWindow(SW_SHOW);
}

void CMainWindow::OnAboutHelp() {

	/* TODO: Add your command handler code here */
	char	loc[2046] = "";
	sprintf(loc, "%s\\%s", m_currentDir, "edcast.chm");

	HINSTANCE	ret = ShellExecute(NULL, "open", loc, NULL, NULL, SW_SHOWNORMAL);
}

void CMainWindow::OnPaint() {
	CPaintDC	dc(this);					/* device context for painting */

	/*
	 * TODO: Add your message handler code here ;
	 * Do not call CDialog::OnPaint() for painting messages
	 */
}

void CMainWindow::OnTimer(UINT nIDEvent) {

	/* TODO: Add your message handler code here and/or call default */
	if(nIDEvent == 73) {
		int			a = 0;
		static int	oldL = 0;
		static int	oldR = 0;
		static int	oldCounter = 0;

		if(m_VUStatus == VU_OFF) {
			return;
		}

		if((m_VUStatus == VU_ON) || (m_VUStatus == VU_SWITCHOFF)) {
			HWND	hWnd = GetDlgItem(IDC_METER)->m_hWnd;

			HDC		hDC = ::GetDC(hWnd);	/* get the DC for the window. */

			if((flexmeters.GetMeterInfoObject(0)->value == -1) && (flexmeters.GetMeterInfoObject(1)->value == -1)) {
				if(oldCounter > 10) {
					flexmeters.GetMeterInfoObject(0)->value = 0;
					flexmeters.GetMeterInfoObject(1)->value = 0;
					oldCounter = 0;
				}
				else {
					flexmeters.GetMeterInfoObject(0)->value = oldL;
					flexmeters.GetMeterInfoObject(1)->value = oldR;
					oldCounter++;
				}
			}
			else {
				oldCounter = 0;
			}

			if(m_VUStatus == VU_SWITCHOFF) {
				flexmeters.GetMeterInfoObject(0)->value = 0;
				flexmeters.GetMeterInfoObject(1)->value = 0;
				m_VUStatus = VU_OFF;
			}

			flexmeters.RenderMeters(hDC);	/* render */
			oldL = flexmeters.GetMeterInfoObject(0)->value;
			oldR = flexmeters.GetMeterInfoObject(1)->value;

			flexmeters.GetMeterInfoObject(0)->value = -1;
			flexmeters.GetMeterInfoObject(1)->value = -1;

			::ReleaseDC(hWnd, hDC);			/* release the DC */
		}
		else {
			HWND	hWnd = GetDlgItem(IDC_METER)->m_hWnd;

			HDC		hDC = ::GetDC(hWnd);	/* get the DC for the window. */
			flexmeters.GetMeterInfoObject(0)->value = 0;
			flexmeters.GetMeterInfoObject(1)->value = 0;
			flexmeters.RenderMeters(hDC);	/* render */
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CMainWindow::OnMeter() {

	/* TODO: Add your control notification handler code here */
	if(m_VUStatus == VU_ON) {
		m_VUStatus = VU_SWITCHOFF;
		gMain.vuShow = 0;
		m_OnOff.ShowWindow(SW_SHOW);
	}
	else {
		m_VUStatus = VU_ON;
		gMain.vuShow = 1;
		m_OnOff.ShowWindow(SW_HIDE);
	}
}

void CMainWindow::OnSTExit() {
	OnCancel();
}

void CMainWindow::OnSTRestore() {
	ShowWindow(SW_RESTORE);
	bMinimized_ = false;
	SetupTrayIcon();
	SetupTaskBarButton();
}

void CMainWindow::SetupTrayIcon() {
	if(bMinimized_ && (pTrayIcon_ == 0)) {
		pTrayIcon_ = new CSystemTray;
		pTrayIcon_->Create(0, nTrayNotificationMsg_, "edcast Restore", hIcon_, IDR_SYSTRAY);
		pTrayIcon_->SetMenuDefaultItem(IDI_RESTORE, false);
		pTrayIcon_->SetNotifier(this);
	}
	else {
		delete pTrayIcon_;
		pTrayIcon_ = 0;
	}
}

/*
 =======================================================================================================================
    SetupTaskBarButton Show or hide the taskbar button for this app, depending on whether ;
    we're minimized right now or not.
 =======================================================================================================================
 */
void CMainWindow::SetupTaskBarButton() {

	/* Show or hide this window appropriately */
	if(bMinimized_) {
		ShowWindow(SW_HIDE);
	}
	else {
		ShowWindow(SW_SHOW);
	}
}

void CMainWindow::OnSysCommand(UINT nID, LPARAM lParam) {

	/* Decide if minimize state changed */
	bool	bOldMin = bMinimized_;
	if(nID == SC_MINIMIZE)
	{
#ifndef EDCASTSTANDALONE
		bMinimized_ = false;
#else
		bMinimized_ = true;
		SetupTrayIcon();
		ShowWindow(SW_HIDE);
		return;
#endif
	}
	else if(nID == SC_RESTORE) {
		bMinimized_ = false;
		if(bOldMin != bMinimized_) {

			/*
			 * Minimize state changed. Create the systray icon and do ;
			 * custom taskbar button handling.
			 */
			SetupTrayIcon();
			SetupTaskBarButton();
		}
	}

	CDialog::OnSysCommand(nID, lParam);
}

void CMainWindow::OnKeydownEncoders(NMHDR *pNMHDR, LRESULT *pResult) {
	LV_KEYDOWN	*pLVKeyDow = (LV_KEYDOWN *) pNMHDR;

	/* TODO: Add your control notification handler code here */
	if (pLVKeyDow->wVKey == 32) {
		OnPopupConfigure();
	}

	if (pLVKeyDow->wVKey == 46) {
		OnPopupDelete();
	}

	if (pLVKeyDow->wVKey == 93) {
		int iItem = m_Encoders.GetNextItem(-1, LVNI_SELECTED);

		CMenu	menu;
		VERIFY(menu.LoadMenu(IDR_CONTEXT));

		/* Pop up sub menu 0 */
		CMenu	*popup = menu.GetSubMenu(0);

		if(popup) {
			if(g[iItem]->weareconnected) {
				popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Disconnect");
			}
			else {
				if(g[iItem]->forcedDisconnect) {
					popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Stop AutoConnect");
				}
				else {
					popup->ModifyMenu(ID_POPUP_CONNECT, MF_BYCOMMAND, ID_POPUP_CONNECT, "Connect");
				}
			}

			RECT	pt;
			WINDOWPLACEMENT wp;
			WINDOWPLACEMENT wp2;
			GetWindowPlacement(&wp);
			m_Encoders.GetWindowPlacement(&wp2);

			//GetCursorPos(&pt);
			pt.bottom = wp.rcNormalPosition.bottom + wp2.rcNormalPosition.bottom;
			pt.left = wp.rcNormalPosition.left + wp2.rcNormalPosition.left;
			pt.right = wp.rcNormalPosition.right + wp2.rcNormalPosition.right;
			pt.top = wp.rcNormalPosition.top + wp2.rcNormalPosition.top;

			popup->TrackPopupMenu(TPM_LEFTALIGN, pt.left, pt.top, m_Encoders.GetActiveWindow());
		}
	}

	*pResult = 0;
}

void CMainWindow::OnButton1() {

	/* TODO: Add your control notification handler code here */
	OnPopupConfigure();
}

void CMainWindow::OnCancel() {

	/* TODO: Add extra cleanup here */
	;
}

void CMainWindow::OnSetfocusEncoders(NMHDR *pNMHDR, LRESULT *pResult) {

	/* TODO: Add your control notification handler code here */
	int mark = m_Encoders.GetSelectionMark();

	if(mark == -1) {
		if(m_Encoders.GetItemCount() > 0) {
			m_Encoders.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
			m_Encoders.EnsureVisible(0, FALSE);
		}
	}
	else {
		m_Encoders.SetCheck(m_Encoders.GetSelectionMark(), true);
	}

	*pResult = 0;
}

void CMainWindow::OnSelchangeReccards() 
{
	char	*name;
	char	selectedCard[1024] = "";
	bool	opened = false;

	// TODO: Add your control notification handler code here
	int		index = m_RecCardsCtrl.GetCurSel();
	memset(selectedCard, '\000', sizeof(selectedCard));
	m_RecCardsCtrl.GetLBText(index, selectedCard);

	m_RecCards = selectedCard;

	setWindowsRecordingDevice(&gMain, selectedCard);


	int n = 0;
	for(n = 0; name = (char *)BASS_RecordGetDeviceDescription(n); n++) {
		if (!strcmp(selectedCard, name)) {
			BASS_RecordSetDevice(n);
			m_CurrentInputCard = n;
		}
	}

	if(m_BASSOpen) {
		m_BASSOpen = 0;
		BASS_RecordFree();
	}

	if(!m_BASSOpen) {
		int ret = BASS_RecordInit(m_CurrentInputCard);
		m_BASSOpen = 1;
		opened = true;
	}

	m_RecDevicesCtrl.ResetContent();

	for(n = 0; name = (char *)BASS_RecordGetInputName(n); n++) {
		int s = BASS_RecordGetInput(n);
		m_RecDevicesCtrl.AddString(name);
		if(s & BASS_INPUT_OFF) {
			;
		}
		else {
			m_RecDevices = name;
			m_RecVolume = LOWORD(s);
			m_CurrentInput = n;
		}
	}

	if(m_BASSOpen) {
		m_BASSOpen = 0;
		BASS_RecordFree();
	}
	startRecording(m_CurrentInputCard);

	UpdateData(FALSE);

	
}
