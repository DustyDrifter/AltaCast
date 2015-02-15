#if !defined(AFX_FINDWINDOW_H__EE156CA9_2C89_4328_B73C_6B5DAEA68F6C__INCLUDED_)
#define AFX_FINDWINDOW_H__EE156CA9_2C89_4328_B73C_6B5DAEA68F6C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindWindow dialog

class CFindWindow : public CDialog
{
// Construction
public:
	CFindWindow(CWnd* pParent = NULL);   // standard constructor

    CString m_Selected;
// Dialog Data
	//{{AFX_DATA(CFindWindow)
	enum { IDD = IDD_FINDWINDOW_DLG };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindWindow)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkFindwindow(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedFindwindow(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDWINDOW_H__EE156CA9_2C89_4328_B73C_6B5DAEA68F6C__INCLUDED_)
