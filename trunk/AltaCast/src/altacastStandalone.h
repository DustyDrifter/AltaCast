// altacastStandalone.h : main header file for the altacastSTANDALONE application
//

#if !defined(AFX_ALTACASTSTANDALONE_H__30572DD7_0B96_48C1_9D02_9FB68A7C8BDA__INCLUDED_)
#define AFX_ALTACASTSTANDALONE_H__30572DD7_0B96_48C1_9D02_9FB68A7C8BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CaltacastStandaloneApp:
// See altacastStandalone.cpp for the implementation of this class
//

class CaltacastStandaloneApp : public CWinApp
{
public:
	CaltacastStandaloneApp();

    void SetMainAfxWin(CWnd *pwnd);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CaltacastStandaloneApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CaltacastStandaloneApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALTACASTSTANDALONE_H__30572DD7_0B96_48C1_9D02_9FB68A7C8BDA__INCLUDED_)
