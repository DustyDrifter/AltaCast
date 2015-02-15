// Dummy.cpp : implementation file
//

#include "stdafx.h"
#include "altacast.h"
#include "Dummy.h"
#include "MainWindow.h"
#include "altacastStandalone.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CaltacastStandaloneApp theApp;

#if 0

CMainWindow *mainWindow;
bool done;
INT  nResult;
static bool shown = true;

/////////////////////////////////////////////////////////////////////////////
// CDummy dialog
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


CDummy::CDummy(CWnd* pParent /*=NULL*/)
	: CDialog(CDummy::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDummy)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDummy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDummy)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDummy, CDialog)
	//{{AFX_MSG_MAP(CDummy)
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SHOWHIDE, OnShowhide)
	ON_BN_CLICKED(IDC_SHOWMAIN, OnShowmain)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDummy message handlers

BOOL CDummy::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
    LoadConfigs(".", "altacaststandalone");

    mainWindow = new CMainWindow(this);


    theApp.SetMainAfxWin(mainWindow);

    mainWindow->InitializeWindow();

    
    mainWindow->Create((UINT)IDD_ALTACAST, this);

    int x = getLastX();
    int y = getLastY();
	int dummyx = getLastDummyX();
	int dummyy = getLastDummyY();
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    
    RECT wsize;

    GetWindowRect(&wsize);

    SetWindowPos(NULL, dummyx, dummyy, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
    mainWindow->SetWindowPos(NULL, (int)x, (int)y, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);

    SetIcon(theApp.LoadIcon(IDR_MAINFRAME), TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDummy::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
    RECT wsize;
    if (shown) {
        if (mainWindow) {
            GetWindowRect(&wsize);
			setLastDummyX(wsize.left);
			setLastDummyY(wsize.top);
            mainWindow->SetWindowPos(NULL, (int)wsize.left, (int)wsize.bottom, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
        }	
    }
	// TODO: Add your message handler code here
	
}

void CDummy::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
    if (mainWindow) {
        if (bShow) {
            mainWindow->ShowWindow(SW_SHOW);
        }
        else {
            mainWindow->ShowWindow(SW_HIDE);
        }
    }

	// TODO: Add your message handler code here
	
}

void CDummy::OnShowhide() 
{

	
}

void CDummy::OnShowmain() 
{
	// TODO: Add your control notification handler code here
    if (shown) {
        mainWindow->ShowWindow(SW_HIDE);
        ShowWindow(SW_SHOW);
        SetActiveWindow();
        shown = false;
    }
    else {
        RECT wsize;
        GetWindowRect(&wsize);
        mainWindow->SetWindowPos(NULL, (int)wsize.left, (int)wsize.bottom, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
        mainWindow->ShowWindow(SW_SHOW);
        SetActiveWindow();
        shown = true;
    }
	
}
#endif