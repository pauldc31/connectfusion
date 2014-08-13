// AddURLDialog.cpp : implementation file
//

#include "stdafx.h"
#include "spload.h"
#include "AddURLDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AddURLDialog dialog


AddURLDialog::AddURLDialog(CWnd* pParent /*=NULL*/)
	: CDialog(AddURLDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddURLDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void AddURLDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddURLDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddURLDialog, CDialog)
	//{{AFX_MSG_MAP(AddURLDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddURLDialog message handlers

void AddURLDialog::OnOK() 
{
	// TODO: Add extra validation here
	((CEdit*) GetDlgItem(IDC_EDIT1))->GetWindowText(returnURL);
	
	CDialog::OnOK();
}

BOOL AddURLDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	
	// TODO: Add extra initialization here

	//Set initial header data
    m_Header.SetTitleText(_T("Enter URL"));
    m_Header.SetDescText(_T("Type or paste your download link into the text box"));	
    m_Header.SetIconHandle(AfxGetApp()->LoadIcon(IDR_MAINFRAME));    
	m_Header.SetHeaderHeight(50);
    m_Header.Init(this);    
    m_Header.MoveCtrls(this);	
	
	
	((CEdit*) GetDlgItem(IDC_EDIT1))->SetWindowText(_T("http://"));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void AddURLDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
