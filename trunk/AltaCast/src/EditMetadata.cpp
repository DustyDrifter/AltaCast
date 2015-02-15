// EditMetadata.cpp : implementation file
//

#include "stdafx.h"
#include "altacast.h"
#include "EditMetadata.h"
#include "MainWindow.h"
#include "FindWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditMetadata dialog


CEditMetadata::CEditMetadata(CWnd* pParent /*=NULL*/)
	: CDialog(CEditMetadata::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMetadata)
	m_LockMetadata = FALSE;
	m_Metadata = _T("");
	m_ExternalFile = -1;
	m_ExternalURL = -1;
	m_MetaFile = _T("");
	m_MetaURL = _T("");
	m_MetaInterval = _T("");
	m_ExternalFlag = -1;
	m_WindowClass = _T("");
	m_WindowTitleGrab = FALSE;
	m_AppendString = _T("");
	m_RemoveStringAfter = _T("");
	m_RemoveStringBefore = _T("");
	//}}AFX_DATA_INIT
}


void CEditMetadata::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMetadata)
	DDX_Control(pDX, IDC_REMOVESTRINGBEFORE, m_RemoveStringBeforeCtrl);
	DDX_Control(pDX, IDC_REMOVESTRINGAFTER, m_RemoveStringAfterCtrl);
	DDX_Control(pDX, IDC_GRABSTATIC, m_GrabStaticCtrl);
	DDX_Control(pDX, IDC_FINDWINDOW, m_FindWindowCtrl);
	DDX_Control(pDX, IDC_WINDOWTITLEGRAB, m_WindowTitleGrabCtrl);
	DDX_Control(pDX, IDC_WINDOWCLASS, m_WindowClassCtrl);
	DDX_Control(pDX, IDC_METAURL, m_ExternalFlagCtrl);
	DDX_Control(pDX, IDC_META_FILE, m_MetaFileCtrl);
	DDX_Control(pDX, IDC_META_URL, m_MetaURLCtrl);
	DDX_Control(pDX, IDC_METADATA, m_MetadataCtrl);
	DDX_Control(pDX, IDC_LOCK_METADATA, m_LockMetadataCtrl);
	DDX_Check(pDX, IDC_LOCK_METADATA, m_LockMetadata);
	DDX_Text(pDX, IDC_METADATA, m_Metadata);
	DDX_Text(pDX, IDC_META_FILE, m_MetaFile);
	DDX_Text(pDX, IDC_META_URL, m_MetaURL);
	DDX_Text(pDX, IDC_METADATA_INTERVAL, m_MetaInterval);
	DDX_Radio(pDX, IDC_METAURL, m_ExternalFlag);
	DDX_Text(pDX, IDC_WINDOWCLASS, m_WindowClass);
	DDX_Check(pDX, IDC_WINDOWTITLEGRAB, m_WindowTitleGrab);
	DDX_Text(pDX, IDC_APPENDSTRING, m_AppendString);
	DDX_Text(pDX, IDC_REMOVESTRINGAFTER, m_RemoveStringAfter);
	DDX_Text(pDX, IDC_REMOVESTRINGBEFORE, m_RemoveStringBefore);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditMetadata, CDialog)
	//{{AFX_MSG_MAP(CEditMetadata)
	ON_BN_CLICKED(IDC_METAURL, OnMetaurl)
	ON_BN_CLICKED(IDC_METAFILE, OnMetafile)
	ON_BN_CLICKED(IDC_METADISABLE, OnMetadisable)
	ON_BN_CLICKED(IDC_FINDWINDOW, OnFindwindow)
	ON_BN_CLICKED(IDC_WINDOWTITLEGRAB, OnWindowtitlegrab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMetadata message handlers

void CEditMetadata::OnOK() 
{
	// TODO: Add extra validation here
    UpdateData(TRUE);
    CMainWindow *pwin = (CMainWindow *)parentDialog;
    pwin->ProcessEditMetadataDone(this);
	CDialog::OnOK();
}

void CEditMetadata::OnCancel() 
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL CEditMetadata::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
#if 0
    m_WindowTitleGrabCtrl.EnableWindow(FALSE);
    m_FindWindowCtrl.EnableWindow(FALSE);
    m_GrabStaticCtrl.SetWindowText("Grab Metadata From Window Title - DISABLED");
#endif
    UpdateRadio();
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditMetadata::UpdateRadio()
{
    if (m_ExternalFlag == 2) {
        m_MetadataCtrl.EnableWindow(TRUE);
        m_LockMetadataCtrl.EnableWindow(TRUE);
        m_MetaURLCtrl.EnableWindow(FALSE);
        m_MetaFileCtrl.EnableWindow(FALSE);
    }
    else {
        m_MetadataCtrl.EnableWindow(FALSE);
        m_LockMetadataCtrl.EnableWindow(FALSE);
        m_MetaFileCtrl.EnableWindow(FALSE);
        m_MetaURLCtrl.EnableWindow(FALSE);
        if (m_ExternalFlag == 1) {
            m_MetaFileCtrl.EnableWindow(TRUE);
        }
        if (m_ExternalFlag == 0) {
            m_MetaURLCtrl.EnableWindow(TRUE);
        }
    }
    if (m_WindowTitleGrab) {
        m_WindowClassCtrl.EnableWindow(TRUE);
        m_RemoveStringBeforeCtrl.EnableWindow(TRUE);
        m_RemoveStringAfterCtrl.EnableWindow(TRUE);
    }
    else {
        m_WindowClassCtrl.EnableWindow(FALSE);
        m_RemoveStringBeforeCtrl.EnableWindow(FALSE);
        m_RemoveStringAfterCtrl.EnableWindow(FALSE);
    }
}
void CEditMetadata::OnMetaurl() 
{
	UpdateData(TRUE);

    UpdateRadio();	
}

void CEditMetadata::OnMetafile() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

    UpdateRadio();	
	
}

void CEditMetadata::OnMetadisable() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData(TRUE);

    UpdateRadio();	
}

void CEditMetadata::OnFindwindow() 
{

	GetWindowClass();

    UpdateData(FALSE);
    
	SetActiveWindow();
}

void CEditMetadata::OnWindowtitlegrab() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
    UpdateRadio();	
	
}

void CEditMetadata::GetWindowClass()
{

#ifndef ALTACASTSTANDALONE
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

    CFindWindow fwindow(AfxGetMainWnd()); 
    int ret = fwindow.DoModal();
    if (ret == IDOK) {
        m_WindowClass = fwindow.m_Selected;
    }
#else
    CFindWindow fwindow; 
    int ret = fwindow.DoModal();
    if (ret == IDOK) {
        m_WindowClass = fwindow.m_Selected;
    }
#endif
}
