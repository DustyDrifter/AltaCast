// FindWindow.cpp : implementation file
//

#include "stdafx.h"
#include "altacast.h"
#include "FindWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFindWindow *findWindow = 0;
BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
{
	char	windowTitle[2046];
	char	windowClass[2046];
	char	*pMatch = (char *)lParam;
	memset(windowTitle, '\000', sizeof(windowTitle));
    memset(windowClass, '\000', sizeof(windowClass));
	GetWindowText(hwnd, windowTitle, sizeof(windowTitle)-1);

    GetClassName(hwnd, windowClass, sizeof(windowClass)-1);

    if (strlen(windowTitle) > 0) {
        LVITEM pItem;
        LVITEM pItem2;
        int numItems = findWindow->m_ListCtrl.GetItemCount();

        pItem.iItem = numItems;
        pItem.iSubItem = 0;
        pItem.pszText = windowClass;
        pItem.mask = LVIF_TEXT;

        findWindow->m_ListCtrl.InsertItem(&pItem);
        pItem2.iItem = numItems;
        pItem2.iSubItem = 1;
        pItem2.pszText = windowTitle;
        pItem2.mask = LVIF_TEXT;
        findWindow->m_ListCtrl.SetItem(&pItem2);
    }


	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFindWindow dialog


CFindWindow::CFindWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CFindWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFindWindow)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFindWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindWindow)
	DDX_Control(pDX, IDC_FINDWINDOW, m_ListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindWindow, CDialog)
	//{{AFX_MSG_MAP(CFindWindow)
	ON_NOTIFY(NM_DBLCLK, IDC_FINDWINDOW, OnDblclkFindwindow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindWindow message handlers

BOOL CFindWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    findWindow = this;
	// TODO: Add extra initialization here
	m_ListCtrl.DeleteAllItems();
    m_ListCtrl.InsertColumn(0, "Window Class");
    m_ListCtrl.InsertColumn(1, "Window Title");
    m_ListCtrl.SetColumnWidth(0, 100);
    m_ListCtrl.SetColumnWidth(1, 200);

    ::SendMessage(m_ListCtrl.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    
    EnumWindows(EnumWindowsProc, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindWindow::OnDblclkFindwindow(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here

    
	*pResult = 0;
    POSITION p = m_ListCtrl.GetFirstSelectedItemPosition();
    char    itemData[255];
    memset(itemData, '\000', sizeof(itemData));
    while (p)
    {
	    int nSelected = m_ListCtrl.GetNextSelectedItem(p);
        m_ListCtrl.GetItemText(nSelected, 0, itemData, sizeof(itemData));
    }
    m_Selected = itemData;
    EndModalLoop(IDOK);
}
