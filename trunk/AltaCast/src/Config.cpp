// Config.cpp : implementation file
//

#include "stdafx.h"
#include "altacast.h"
#include "Config.h"
#include "MainWindow.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfig dialog


CConfig::CConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    basicSettings = new CBasicSettings();
    ypSettings = new CYPSettings();
    advSettings = new CAdvancedSettings();
    currentEnc = 0;
}

CConfig::~CConfig()
{
    if (basicSettings) {
        delete basicSettings;
    }
    if (ypSettings) {
        delete ypSettings;
    }
    if (advSettings) {
        delete advSettings;
    }
}
void CConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfig)
	DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfig, CDialog)
	//{{AFX_MSG_MAP(CConfig)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfig message handlers

BOOL CConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_TabCtrl.InsertItem(0, _T("Basic Settings"));
	m_TabCtrl.InsertItem(1, _T("YP Settings"));
	m_TabCtrl.InsertItem(2, _T("Advanced Settings"));

    basicSettings->Create((UINT)IDD_PROPPAGE_LARGE, this);    
    ypSettings->Create((UINT)IDD_PROPPAGE_LARGE1, this);    
    advSettings->Create((UINT)IDD_PROPPAGE_LARGE2, this);    

    basicSettings->ShowWindow(SW_SHOW);
    ypSettings->ShowWindow(SW_HIDE);
    advSettings->ShowWindow(SW_HIDE);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfig::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int selected = m_TabCtrl.GetCurSel();
    if (selected == 0) {
        basicSettings->ShowWindow(SW_SHOW);
        ypSettings->ShowWindow(SW_HIDE);
        advSettings->ShowWindow(SW_HIDE);
    }
    if (selected == 1) {
        ypSettings->ShowWindow(SW_SHOW);
        basicSettings->ShowWindow(SW_HIDE);
        advSettings->ShowWindow(SW_HIDE);
    }
    if (selected == 2) {
        basicSettings->ShowWindow(SW_HIDE);
        ypSettings->ShowWindow(SW_HIDE);
        advSettings->ShowWindow(SW_SHOW);
    }

	*pResult = 0;
}
void CConfig::GlobalsToDialog(altacastGlobals *g) {
    char    buf[255] = "";
    /*Basic Settings 
	CString	m_Bitrate;
	CString	m_Channels;
	CString	m_EncoderType;
	CString	m_Mountpoint;
	CString	m_Password;
	CString	m_Quality;
	CString	m_ReconnectSecs;
	CString	m_Samplerate;
	CString	m_ServerIP;
	CString	m_ServerPort;
	CString	m_ServerType;
    */
    currentEnc = g->encoderNumber;

    sprintf(buf, "%d", getCurrentBitrate(g));
    basicSettings->m_Bitrate = buf;
    sprintf(buf, "%d", getCurrentChannels(g));
    basicSettings->m_Channels = buf;
    sprintf(buf, "%d", getCurrentSamplerate(g));
    basicSettings->m_Samplerate = buf;

	if (g->gOggBitQualFlag == 0) { // Quality
		basicSettings->m_UseBitrate = false;
	}
	else {
		basicSettings->m_UseBitrate = true;
	}

    if (g->gAACFlag) {
        basicSettings->m_EncoderType = "AAC";
        basicSettings->m_Quality = g->gAACQuality;
    }
    if (g->gAACPFlag) {
        basicSettings->m_EncoderType = "AAC Plus";
        basicSettings->m_Quality = "";
    }
    if (g->gLAMEFlag) {
        basicSettings->m_EncoderType = "MP3 Lame";
        basicSettings->m_Quality = g->gOggQuality;
    }
    if (g->gOggFlag) {
        basicSettings->m_EncoderType = "OggVorbis";
        basicSettings->m_Quality = g->gOggQuality;
    }
    if (g->gFLACFlag) {
        basicSettings->m_EncoderType = "Ogg FLAC";
        basicSettings->m_Quality = g->gOggQuality;
    }
    basicSettings->m_EncoderTypeCtrl.SelectString(0, basicSettings->m_EncoderType);
    basicSettings->m_Mountpoint = g->gMountpoint;
    basicSettings->m_Password = g->gPassword;
    sprintf(buf, "%d", g->gReconnectSec);

    basicSettings->m_ReconnectSecs = buf;
    basicSettings->m_ServerIP = g->gServer;
    basicSettings->m_ServerPort = g->gPort;

    if (g->gShoutcastFlag) {
        basicSettings->m_ServerType = "Shoutcast";
    }
    if (g->gIcecast2Flag) {
        basicSettings->m_ServerType = "Icecast2";
    }

    basicSettings->m_ServerTypeCtrl.SelectString(0, basicSettings->m_ServerType);

	if (g->LAMEJointStereoFlag) {
		basicSettings->m_JointStereo = true;
	}
    basicSettings->UpdateData(FALSE);
    basicSettings->UpdateFields();
    /* YP Settings
    	BOOL	m_Public;
	CString	m_StreamDesc;
	CString	m_StreamGenre;
	CString	m_StreamName;
	CString	m_StreamURL;
    */
    if (g->gPubServ) {
        ypSettings->m_Public = true;
    }
    else {
        ypSettings->m_Public = false;
    }
    ypSettings->m_StreamDesc = g->gServDesc;
    ypSettings->m_StreamName = g->gServName;
    ypSettings->m_StreamGenre = g->gServGenre;
    ypSettings->m_StreamURL = g->gServURL;
    ypSettings->m_StreamAIM = g->gServAIM;
    ypSettings->m_StreamICQ = g->gServICQ;
    ypSettings->m_StreamIRC = g->gServIRC;
    ypSettings->UpdateData(FALSE);
    ypSettings->EnableDisable();
    /* Advanced Settings
    	CString	m_ArchiveDirectory;
	CString	m_Logfile;
	CString	m_Loglevel;
	BOOL	m_Savestream;
	BOOL	m_Savewav;
    */
    advSettings->m_Savestream = g->gSaveDirectoryFlag;
    advSettings->m_ArchiveDirectory = g->gSaveDirectory;
    sprintf(buf, "%d", g->gLogLevel);
    advSettings->m_Loglevel = buf;
	advSettings->m_Logfile = g->gLogFile;

    advSettings->m_Savewav = g->gSaveAsWAV;
    advSettings->UpdateData(FALSE);
    advSettings->EnableDisable();
}

void CConfig::DialogToGlobals(altacastGlobals *g) {
    char    buf[255] = "";
    /*Basic Settings 
	CString	m_Bitrate;
	CString	m_Channels;
	CString	m_EncoderType;
	CString	m_Mountpoint;
	CString	m_Password;
	CString	m_Quality;
	CString	m_ReconnectSecs;
	CString	m_Samplerate;
	CString	m_ServerIP;
	CString	m_ServerPort;
	CString	m_ServerType;
    */
    currentEnc = g->encoderNumber;

//    basicSettings->UpdateData(TRUE);

    g->currentBitrate = atoi(LPCSTR(basicSettings->m_Bitrate));
    g->currentChannels = atoi(LPCSTR(basicSettings->m_Channels));
    g->currentSamplerate = atoi(LPCSTR(basicSettings->m_Samplerate));

    if (basicSettings->m_EncoderType == "AAC Plus") {
        g->gAACPFlag = 1;
        g->gAACFlag = 0;
        g->gOggFlag = 0;
        g->gLAMEFlag = 0;
		g->gFLACFlag = 0;
    }
    if (basicSettings->m_EncoderType == "AAC") {
        g->gAACPFlag = 0;
        g->gAACFlag = 1;
        g->gOggFlag = 0;
        g->gLAMEFlag = 0;
		g->gFLACFlag = 0;
    }
    if (basicSettings->m_EncoderType == "MP3 Lame") {
        g->gAACPFlag = 0;
        g->gLAMEFlag = 1;
        g->gAACFlag = 0;
        g->gOggFlag = 0;
		g->gFLACFlag = 0;
    }
    if (basicSettings->m_EncoderType == "OggVorbis") {
        g->gAACPFlag = 0;
        g->gOggFlag = 1;
        g->gLAMEFlag = 0;
        g->gAACFlag = 0;
		g->gFLACFlag = 0;
    }
    if (basicSettings->m_EncoderType == "Ogg FLAC") {
        g->gAACPFlag = 0;
        g->gOggFlag = 0;
        g->gLAMEFlag = 0;
        g->gAACFlag = 0;
		g->gFLACFlag = 1;
    }

	if (basicSettings->m_UseBitrate) {
		g->gOggBitQualFlag = 1;
	}
	else {
		g->gOggBitQualFlag = 0;
	}
	if (basicSettings->m_JointStereo) {
		g->LAMEJointStereoFlag = 1;
	}
	else {
		g->LAMEJointStereoFlag = 0;
	}

    strcpy(g->gEncodeType, LPCSTR(basicSettings->m_EncoderType));

    if (g->gAACFlag) {
        strcpy(g->gAACQuality, LPCSTR(basicSettings->m_Quality));
    }
    if (g->gLAMEFlag) {
        strcpy(g->gOggQuality, LPCSTR(basicSettings->m_Quality));
    }
    if (g->gOggFlag) {
        strcpy(g->gOggQuality, LPCSTR(basicSettings->m_Quality));
    }
    strcpy(g->gMountpoint, LPCSTR(basicSettings->m_Mountpoint));
    strcpy(g->gPassword, LPCSTR(basicSettings->m_Password));

    g->gReconnectSec = atoi(LPCSTR(basicSettings->m_ReconnectSecs));
    strcpy(g->gServer, LPCSTR(basicSettings->m_ServerIP));
    strcpy(g->gPort, LPCSTR(basicSettings->m_ServerPort));

    if (basicSettings->m_ServerType == "Shoutcast") {
        g->gShoutcastFlag = 1;
        g->gIcecast2Flag = 0;
    }
    if (basicSettings->m_ServerType == "Icecast2") {
        g->gShoutcastFlag = 0;
        g->gIcecast2Flag = 1;
    }
    strcpy(g->gServerType, LPCSTR(basicSettings->m_ServerType));

    ypSettings->UpdateData(TRUE);
    if (ypSettings->m_Public) {
        g->gPubServ = 1;
    }
    else {
        g->gPubServ = 0;
    }

    strcpy(g->gServDesc, LPCSTR(ypSettings->m_StreamDesc));
    strcpy(g->gServName, LPCSTR(ypSettings->m_StreamName));
    strcpy(g->gServGenre, LPCSTR(ypSettings->m_StreamGenre));
    strcpy(g->gServURL, LPCSTR(ypSettings->m_StreamURL));
    strcpy(g->gServAIM, LPCSTR(ypSettings->m_StreamAIM));
    strcpy(g->gServICQ, LPCSTR(ypSettings->m_StreamICQ));
    strcpy(g->gServIRC, LPCSTR(ypSettings->m_StreamIRC));

    /* Advanced Settings
    	CString	m_ArchiveDirectory;
	CString	m_Logfile;
	CString	m_Loglevel;
	BOOL	m_Savestream;
	BOOL	m_Savewav;
    */
    advSettings->UpdateData(TRUE);

    g->gSaveDirectoryFlag = advSettings->m_Savestream;
    strcpy(g->gSaveDirectory, LPCSTR(advSettings->m_ArchiveDirectory));
    g->gLogLevel = atoi(LPCSTR(advSettings->m_Loglevel));
	strcpy(g->gLogFile, LPCSTR(advSettings->m_Logfile));
    g->gSaveAsWAV = advSettings->m_Savewav;

}

void CConfig::OnClose() 
{
    CDialog::OnClose();
}

void CConfig::OnOK() 
{
	// TODO: Add extra validation here
	basicSettings->UpdateData(TRUE);
	ypSettings->UpdateData(TRUE);
	advSettings->UpdateData(TRUE);
    CMainWindow *pwin = (CMainWindow *)parentDialog;
    pwin->ProcessConfigDone(currentEnc, this);
    CDialog::OnOK();
}

void CConfig::OnCancel() 
{
	// TODO: Add extra cleanup here
    CMainWindow *pwin = (CMainWindow *)parentDialog;
    pwin->ProcessConfigDone(-1, this);
    CDialog::OnCancel();
}
