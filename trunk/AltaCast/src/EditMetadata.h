#if !defined(AFX_EDITMETADATA_H__A2217C99_54F4_420E_BADE_E98054B96C07__INCLUDED_)
#define AFX_EDITMETADATA_H__A2217C99_54F4_420E_BADE_E98054B96C07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditMetadata.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditMetadata dialog

class CEditMetadata : public CDialog
{
// Construction
public:
	void GetWindowClass();
	CEditMetadata(CWnd* pParent = NULL);   // standard constructor

    void UpdateRadio();
// Dialog Data
	//{{AFX_DATA(CEditMetadata)
	enum { IDD = IDD_EDIT_METADATA };
	CEdit	m_RemoveStringBeforeCtrl;
	CEdit	m_RemoveStringAfterCtrl;
	CButton	m_GrabStaticCtrl;
	CButton	m_FindWindowCtrl;
	CButton	m_WindowTitleGrabCtrl;
	CEdit	m_WindowClassCtrl;
	CButton	m_ExternalFlagCtrl;
	CEdit	m_MetaFileCtrl;
	CEdit	m_MetaURLCtrl;
	CEdit	m_MetadataCtrl;
	CButton	m_LockMetadataCtrl;
	BOOL	m_LockMetadata;
	CString	m_Metadata;
	int		m_ExternalFile;
	int		m_ExternalURL;
	CString	m_MetaFile;
	CString	m_MetaURL;
	CString	m_MetaInterval;
	int		m_ExternalFlag;
	CString	m_WindowClass;
	BOOL	m_WindowTitleGrab;
	CString	m_AppendString;
	CString	m_RemoveStringAfter;
	CString	m_RemoveStringBefore;
	//}}AFX_DATA

    CDialog *parentDialog;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMetadata)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditMetadata)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnMetaurl();
	afx_msg void OnMetafile();
	afx_msg void OnMetadisable();
	afx_msg void OnFindwindow();
	afx_msg void OnWindowtitlegrab();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMETADATA_H__A2217C99_54F4_420E_BADE_E98054B96C07__INCLUDED_)
