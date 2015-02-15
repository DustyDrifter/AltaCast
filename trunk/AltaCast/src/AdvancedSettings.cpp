// AdvancedSettings.cpp : implementation file
//

#include "stdafx.h"
#include "altacast.h"
#include "AdvancedSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettings dialog


CAdvancedSettings::CAdvancedSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvancedSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvancedSettings)
	m_ArchiveDirectory = _T("");
	m_Logfile = _T("");
	m_Loglevel = _T("");
	m_Savestream = FALSE;
	m_Savewav = FALSE;
	//}}AFX_DATA_INIT
}


void CAdvancedSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvancedSettings)
	DDX_Control(pDX, IDC_ARCHIVE_DIRECTORY, m_ArchiveDirectoryCtrl);
	DDX_Control(pDX, IDC_SAVEWAV, m_SavewavCtrl);
	DDX_Text(pDX, IDC_ARCHIVE_DIRECTORY, m_ArchiveDirectory);
	DDX_Text(pDX, IDC_LOGFILE, m_Logfile);
	DDX_Text(pDX, IDC_LOGLEVEL, m_Loglevel);
	DDX_Check(pDX, IDC_SAVESTREAM, m_Savestream);
	DDX_Check(pDX, IDC_SAVEWAV, m_Savewav);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvancedSettings, CDialog)
	//{{AFX_MSG_MAP(CAdvancedSettings)
	ON_BN_CLICKED(IDC_SAVESTREAM, OnSavestream)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettings message handlers

void CAdvancedSettings::EnableDisable()
{
    if (m_Savestream) {
        m_ArchiveDirectoryCtrl.EnableWindow(TRUE);
        m_SavewavCtrl.EnableWindow(TRUE);
    }
    else {
        m_ArchiveDirectoryCtrl.EnableWindow(FALSE);
        m_SavewavCtrl.EnableWindow(FALSE);
    }
}

BOOL CAdvancedSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvancedSettings::OnSavestream() 
{
    UpdateData(TRUE);
    EnableDisable();	
}
