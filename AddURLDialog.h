#if !defined(AFX_ADDURLDIALOG_H__7582EB3F_7266_4451_AF71_5099C705EE82__INCLUDED_)
#define AFX_ADDURLDIALOG_H__7582EB3F_7266_4451_AF71_5099C705EE82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddURLDialog.h : header file
//

#include "DialogHeader.h"


/////////////////////////////////////////////////////////////////////////////
// AddURLDialog dialog

class AddURLDialog : public CDialog
{
// Construction
public:
	AddURLDialog(CWnd* pParent = NULL);   // standard constructor
	CString returnURL;
	CDialogHeader m_Header;  


// Dialog Data
	//{{AFX_DATA(AddURLDialog)
	enum { IDD = IDD_URLDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddURLDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AddURLDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDURLDIALOG_H__7582EB3F_7266_4451_AF71_5099C705EE82__INCLUDED_)
