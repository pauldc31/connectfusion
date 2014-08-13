// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "spload.h"

#include "sploadDoc.h"
#include "MainFrm.h"
#include "sploadView.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CString GetModuleDirectory();
extern int userStopPressed;

#define WM_NOTIFY_TRAY (WM_USER + 10)
#define WM_USER_NOTIFY_ADDITEM 0x408

NOTIFYICONDATA	notifyData;
extern HANDLE hgOpensourceBitmap;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CNewFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CNewFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
//	ON_WM_INITMENUPOPUP()
//	ON_WM_MEASUREITEM()
//	ON_WM_MENUCHAR()
	ON_COMMAND(ID_VIEW_ADDRESSBAR, OnViewAddressbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ADDRESSBAR, OnUpdateViewAddressbar)
	ON_WM_SIZE()
	ON_COMMAND(ID_BUTTONGO, OnButtongo)
	ON_UPDATE_COMMAND_UI(ID_BUTTONGO, OnUpdateButtongo)
	//}}AFX_MSG_MAP
	ON_CBN_SELENDOK(AFX_IDW_TOOLBAR + 1,OnNewAddress)	
	ON_COMMAND(IDOK, OnNewAddressEnter)
	ON_MESSAGE(WM_USER_PASTECOMBO,PasteCombo)
	ON_MESSAGE(WM_USER_SETTRAYNOTIFY, SetTrayNotify)
	ON_MESSAGE(WM_NOTIFY_TRAY,OnNotifyTray)
	ON_MESSAGE(WM_USER_NOTIFY_ADDITEM,OnNotifyAdd)	
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CNewFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CNewMenu::SetMenuDrawMode(CNewMenu::STYLE_XP_2003_NOBORDER);

	if (!m_wndReBar.Create(this))
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}
	if (!m_wndAddress.Create(CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_TABSTOP | WS_VSCROLL | WS_CHILD , CRect(0, 0, 200, 120), this, AFX_IDW_TOOLBAR + 1))	
	{
		TRACE0("Failed to create combobox\n");
		return -1;      // fail to create
	}	
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME4))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	} 		

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}	

	if (!m_wndStatic.Create(NULL, SS_BITMAP | SS_CENTERIMAGE, CRect(0, 0, 95, 20), this, AFX_IDW_TOOLBAR + 10))
	{

		TRACE0("Failed to create static\n");
		return -1;      // fail to create
	}
	hgOpensourceBitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_OSBITMAP),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	m_wndStatic.SetBitmap((HBITMAP) hgOpensourceBitmap);	
	

	if (!m_wndToolBarGo.CreateEx(this) ||
		!m_wndToolBarGo.LoadToolBar(IDR_TOOLBAR1))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
	m_wndAddress.GetEditCtrl()->SetWindowText(_T("http://"));
	
	CString str;
	str.LoadString(IDS_ADDRESS);
		
	m_wndReBar.AddBar(&m_wndAddress, RGB(0,0,0),RGB(208,213,226),str, RBBS_FIXEDBMP | RBBS_NOGRIPPER );
	m_wndReBar.AddBar(&m_wndToolBarGo, RGB(0,0,0),RGB(208,213,226), NULL, RBBS_FIXEDSIZE );
	m_wndReBar.AddBar(&m_wndStatic, RGB(0,0,0),RGB(208,213,226), NULL, RBBS_FIXEDSIZE);
	
	
	//m_wndToolBar.ShowWindow(SW_HIDE);
	m_wndStatusBar.ShowWindow(SW_HIDE);

	m_DefaultNewMenu.LoadToolBar(IDR_MAINFRAME4);
	

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	

	if( !CNewFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.cx = 480;
	cs.cy = 400;
	
	cs.dwExStyle = WS_EX_TOPMOST;
	cs.lpszClass = "ConnectFusion Download Accelerator";
	

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CNewFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CNewFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::SetAddress(LPCTSTR lpszUrl)
{
	m_wndAddress.SetWindowText(lpszUrl);

}

void CMainFrame::OnNewAddress()
{
	CString str;

	m_wndAddress.GetLBText(m_wndAddress.GetCurSel(), str);
	((CSploadView *) GetActiveView())->OnAddItem(str);
	
}

void CMainFrame::OnNewAddressEnter()
{
	

	CString str;

	m_wndAddress.GetEditCtrl()->GetWindowText(str);
	BOOL ret = ((CSploadView *) GetActiveView())->OnAddItem(str);	


	if (ret)
	{

		COMBOBOXEXITEM item;

		item.mask = CBEIF_TEXT;
		item.iItem = -1;
		item.pszText = (LPTSTR)(LPCTSTR)str;
		m_wndAddress.InsertItem(&item);

	}
	
}

LRESULT CMainFrame::PasteCombo(WPARAM wParam, LPARAM lParam)
{	

	COleDataObject odo;
	odo.AttachClipboard();

	SetForegroundWindow();
	BOOL bRet = odo.IsDataAvailable( CF_TEXT, NULL );
	HGLOBAL hGlobal=odo.GetGlobalData(CF_TEXT);
	if (hGlobal)
	{
		LPCTSTR pData=(LPCTSTR)GlobalLock(hGlobal);
		m_wndAddress.SetWindowText(pData);				

		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
	}

	

	return 0;
	
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	
		CNewFrameWnd::OnSysCommand(nID, lParam);	
	
	
}

LRESULT CMainFrame::SetTrayNotify(WPARAM wParam, LPARAM lParam)
{

	TRACE("\n Notify");

	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(IDI_TRAY);
	if(hIcon)
	{
		notifyData.cbSize			= sizeof(NOTIFYICONDATA);	
		notifyData.hWnd				= 0;
		notifyData.uID				= 1;
		notifyData.uCallbackMessage	= WM_NOTIFY_TRAY;	
		notifyData.hIcon				= 0;
		notifyData.szTip[0]			= 0;	
		notifyData.uFlags			= NIF_MESSAGE;	
		notifyData.hWnd =this->m_hWnd;

		notifyData.hIcon = hIcon;
		notifyData.uFlags |= NIF_ICON;

		CString tooltip;
		tooltip.LoadString(IDS_TRAYTIP);
		_tcscpy(notifyData.szTip,tooltip);
		notifyData.uFlags |= NIF_TIP;	

		BOOL bSuccess = Shell_NotifyIcon(NIM_ADD,&notifyData);

		TRACE("\n Notify Status %d",bSuccess);
	}	
	
	return 0;

}



void CMainFrame::OnDestroy() 
{
	CNewFrameWnd::OnDestroy();
	
	if(notifyData.hWnd && notifyData.uID>0)
	{
		Shell_NotifyIcon(NIM_DELETE,&notifyData);
	}	
}


LRESULT CMainFrame::OnNotifyTray(WPARAM wParam, LPARAM lParam) 
{ 
    UINT Id,Message;     
 
    Id  = (UINT) wParam; 
    Message = (UINT) lParam; 
 
	if (Id != 1)
		return 0;
	
	CPoint point;	

    switch (Message ) 
	{ 
	
		case WM_LBUTTONDBLCLK:
			SetForegroundWindow(); 
			ShowWindow(SW_RESTORE);			
			break;
		
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			GetCursorPos(&point);
			SetForegroundWindow(); 	
			//CMenu traymenu;
			CNewMenu traymenu;
			traymenu.LoadMenu(IDR_TRAYMENU);				
			traymenu.GetSubMenu(0)->TrackPopupMenu(0,point.x, point.y, this); 
			break;
	
    } 
     return 0; 
} 


BOOL CMainFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	
	
	BOOL bSuccess =  CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);	

	return bSuccess;
}


LRESULT CMainFrame::OnNotifyAdd(WPARAM wParam, LPARAM lParam) 
{ 

	TRACE("\nOn Notify Add");

	int rd = wParam;
	int len = lParam;

	CString passPath = GetModuleDirectory() + "\\passdata";
	CString urlStr;

	CString numstr;	
	numstr.Format("%d.tmp",rd);
	passPath = passPath + numstr;
		
	TRACE("\nPassParam %s",passPath);
	FILE* pf = fopen(passPath,"rb");
	if (pf)
	{
		if (len<2000)
		{
			char strbuf[2000];
			fread(strbuf,1,len,pf);		
			strbuf[len] = 0;
			urlStr = strbuf;
			TRACE("\nPassURL %s",urlStr);


			userStopPressed = 0;

			ShowWindow(SW_RESTORE);
			SetForegroundWindow(); 
			((CSploadView *) GetActiveView())->OnAddItem(urlStr);			

		}
		fclose(pf);
		DeleteFile(passPath);
	}

	return 0; 

} 


HMENU CMainFrame::NewMenu()
{
  return(m_menu.Detach());
}


void CMainFrame::OnViewAddressbar() 
{
	// TODO: Add your command handler code here

	if (m_wndReBar.IsVisible())
	{
		m_wndReBar.ShowWindow(SW_HIDE);
		RecalcLayout();
		
	}
	else
	{
		m_wndReBar.ShowWindow(SW_SHOW);
		RecalcLayout();

	}
	
}

void CMainFrame::OnUpdateViewAddressbar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_wndReBar.IsVisible());
	
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{	
	CNewFrameWnd::OnSize(nType, cx, cy);
	
	
}

void CMainFrame::OnButtongo() 
{
	CString str;

	m_wndAddress.GetEditCtrl()->GetWindowText(str);
	BOOL ret = ((CSploadView *) GetActiveView())->OnAddItem(str);	

	if (ret)
	{

		COMBOBOXEXITEM item;

		item.mask = CBEIF_TEXT;
		item.iItem = -1;
		item.pszText = (LPTSTR)(LPCTSTR)str;
		m_wndAddress.InsertItem(&item);

	}
	
}

void CMainFrame::OnUpdateButtongo(CCmdUI* pCmdUI) 
{
	CString str;
	m_wndAddress.GetEditCtrl()->GetWindowText(str);
	str.TrimLeft();
	str.TrimRight();
	BOOL bEnable = TRUE;
	if (str=="")
		bEnable = FALSE;

	pCmdUI->Enable(bEnable);
	
}
