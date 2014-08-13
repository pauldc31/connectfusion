// spload.h : main header file for the SPLOAD application
//

#if !defined(AFX_SPLOAD_H__1B7E4873_3B1B_4611_A5EB_F685F78743C5__INCLUDED_)
#define AFX_SPLOAD_H__1B7E4873_3B1B_4611_A5EB_F685F78743C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CSploadApp:
// See spload.cpp for the implementation of this class
//

class CSploadApp : public CWinApp
{
public:
	CSploadApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSploadApp)
	public:
	virtual BOOL InitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CSploadApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLOAD_H__1B7E4873_3B1B_4611_A5EB_F685F78743C5__INCLUDED_)
