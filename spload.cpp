// spload.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "spload.h"

#include "MainFrm.h"
#include "sploadDoc.h"
#include "sploadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int AddMode = 0;
CString AddModeItem;

/////////////////////////////////////////////////////////////////////////////
// CSploadApp

BEGIN_MESSAGE_MAP(CSploadApp, CWinApp)
	//{{AFX_MSG_MAP(CSploadApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSploadApp construction

CSploadApp::CSploadApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSploadApp object

CSploadApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSploadApp initialization

BOOL CSploadApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	CWnd *pWnd ;
	pWnd = CWnd::FindWindow("ConnectFusion Download Accelerator",NULL);
	if (pWnd!=NULL)
	{			
		return FALSE;
	}


	//TRACE("\npWnd %d",pWnd);

	WNDCLASS wc;
	HICON appIcon = LoadIcon(IDR_MAINFRAME);
    ZeroMemory(&wc, sizeof(WNDCLASS));   // start with NULL defaults    
    wc.lpfnWndProc = ::DefWindowProc;
    wc.hInstance = AfxGetInstanceHandle();  
	wc.hIcon = appIcon;
	wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; 
    wc.lpszClassName = "ConnectFusion Download Accelerator";

    // Register new class and exit if it fails
	// Remember to assign the mainform (in precreatewindow) the registered classname 
    if(!AfxRegisterClass(&wc))
    {	  
      TRACE("Fail To Register Class\n");
      return FALSE;
    }

	SetRegistryKey(_T("ConnectFusion"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSploadDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CSploadView));
	AddDocTemplate(pDocTemplate);

	
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	//ParseCommandLine(cmdInfo);

	int commandlen = strlen(m_lpCmdLine);
	if (commandlen>0)
	{		
			AddMode = 1;
			AddModeItem = m_lpCmdLine;
			cmdInfo.m_strFileName = "";
			strcpy(m_lpCmdLine,"");

	}

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CSploadApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CSploadApp message handlers


CDocument* CSploadApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CWinApp::OpenDocumentFile(lpszFileName);
	
}
