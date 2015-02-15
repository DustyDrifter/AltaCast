#if !defined(AFX_ADVANCEDSETTINGS_H__08B61D97_80EA_4C27_87D7_C0276A2932B8__INCLUDED_)
#define AFX_ADVANCEDSETTINGS_H__08B61D97_80EA_4C27_87D7_C0276A2932B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvancedSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvancedSettings dialog

class CAdvancedSettings : public CDialog
{
// Construction
public:
	CAdvancedSettings(CWnd* pParent = NULL);   // standard constructor

    void EnableDisable();

// Dialog Data
	//{{AFX_DATA(CAdvancedSettings)
	enum { IDD = IDD_PROPPAGE_LARGE2 };
	CEdit	m_ArchiveDirectoryCtrl;
	CButton	m_SavewavCtrl;
	CString	m_ArchiveDirectory;
	CString	m_Logfile;
	CString	m_Loglevel;
	BOOL	m_Savestream;
	BOOL	m_Savewav;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvancedSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvancedSettings)
	virtual BOOL OnInitDialog();
	afx_msg void OnSavestream();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEDSETTINGS_H__08B61D97_80EA_4C27_87D7_C0276A2932B8__INCLUDED_)
