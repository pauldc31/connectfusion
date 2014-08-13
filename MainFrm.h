// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__DF20A741_9BAE_4104_8C11_F4946A46255E__INCLUDED_)
#define AFX_MAINFRM_H__DF20A741_9BAE_4104_8C11_F4946A46255E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "NewToolBar.h"


class CMainFrame : public CNewFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	HMENU NewMenu();	
	CMenu m_menu;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:	
	virtual ~CMainFrame();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void SetAddress(LPCTSTR lpszUrl);

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	//CToolBar    m_wndToolBar;
	CNewToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CComboBoxEx m_wndAddress;			
	CAnimateCtrl m_wndAnimate;
	CStatic m_wndStatic;
	CBitmapButton m_wndButton;
	CNewToolBar    m_wndToolBarGo;
		
	//CEdit m_wndAddress;


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
//	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
//	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
//	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnViewAddressbar();
	afx_msg void OnUpdateViewAddressbar(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtongo();
	afx_msg void OnUpdateButtongo(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnNewAddress();
	afx_msg void OnNewAddressEnter();
	afx_msg LRESULT PasteCombo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT SetTrayNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotifyTray(WPARAM wParam, LPARAM lParam);		
	afx_msg LRESULT OnNotifyAdd(WPARAM wParam, LPARAM lParam);	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__DF20A741_9BAE_4104_8C11_F4946A46255E__INCLUDED_)
