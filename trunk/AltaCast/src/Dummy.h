#if !defined(AFX_DUMMY_H__4D97E4E1_A53E_49E4_BFE5_B82833D30C74__INCLUDED_)
#define AFX_DUMMY_H__4D97E4E1_A53E_49E4_BFE5_B82833D30C74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Dummy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDummy dialog

class CDummy : public CDialog
{
// Construction
public:
	CDummy(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDummy)
	enum { IDD = IDD_DUMMY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDummy)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDummy)
	virtual BOOL OnInitDialog();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnShowhide();
	afx_msg void OnShowmain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUMMY_H__4D97E4E1_A53E_49E4_BFE5_B82833D30C74__INCLUDED_)
