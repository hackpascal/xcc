// XCC INI Editor.h : main header file for the XCC INI EDITOR application
//

#if !defined(AFX_XCCINIEDITOR_H__CBA4B643_0846_11D5_B606_0000B4936994__INCLUDED_)
#define AFX_XCCINIEDITOR_H__CBA4B643_0846_11D5_B606_0000B4936994__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CXIEApp:
// See XCC INI Editor.cpp for the implementation of this class
//

class CXIEApp : public CWinApp
{
public:
	CXIEApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXIEApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CXIEApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XCCINIEDITOR_H__CBA4B643_0846_11D5_B606_0000B4936994__INCLUDED_)
