// AltaCastStandalone.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "altacastStandalone.h"
#include "MainWindow.h"
//#include "Dummy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern CMainWindow *mainWindow;
CMainWindow *mainWindow;
CWnd *myWin;

char    logPrefix[255] = "altacaststandalone";

/////////////////////////////////////////////////////////////////////////////
// CaltacastStandalone

BEGIN_MESSAGE_MAP(CaltacastStandaloneApp, CWinApp)
	//{{AFX_MSG_MAP(CaltacastStandaloneApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CaltacastStandaloneApp construction

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
    //strcpy(g->gConfigFileName, ".\\altacast_standalone");

	readConfigFile(g);
	
	setFrontEndType(g, FRONT_END_ALTACAST_PLUGIN);
	
	return 1;
}


CaltacastStandaloneApp::CaltacastStandaloneApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CaltacastStandaloneApp object

CaltacastStandaloneApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CaltacastStandaloneApp initialization
/*
bool done;
INT  nResult;

int RunModalWindow(HWND hwndDialog,HWND hwndParent)
{
  if(hwndParent != NULL)
    EnableWindow(hwndParent,FALSE);

  MSG msg;
  
  for(done=false;done==false;WaitMessage())
  {
    while(PeekMessage(&msg,0,0,0,PM_REMOVE))
    {
      if(msg.message == WM_QUIT)
      {
        done = true;
        PostMessage(NULL,WM_QUIT,0,0);
        break;
      }

      if(!IsDialogMessage(hwndDialog,&msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  if(hwndParent != NULL)
    EnableWindow(hwndParent,TRUE);

  DestroyWindow(hwndDialog);

  return nResult;
}
*/

void CaltacastStandaloneApp::SetMainAfxWin(CWnd *pwnd) {
    m_pMainWnd = pwnd;
}

BOOL CaltacastStandaloneApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	char currentDir[MAX_PATH] = ".";
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

    LoadConfigs(currentDir, "altacaststandalone");

    mainWindow = new CMainWindow(m_pMainWnd);

    theApp.SetMainAfxWin(mainWindow);

    mainWindow->InitializeWindow();

    
    //mainWindow->Create((UINT)IDD_altacast, this);
	mainWindow->DoModal();

	return FALSE;
}
