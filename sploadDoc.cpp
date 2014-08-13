// sploadDoc.cpp : implementation of the CSploadDoc class
//

#include "stdafx.h"
#include "spload.h"

#include "sploadDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSploadDoc

IMPLEMENT_DYNCREATE(CSploadDoc, CDocument)

BEGIN_MESSAGE_MAP(CSploadDoc, CDocument)
	//{{AFX_MSG_MAP(CSploadDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSploadDoc construction/destruction

CSploadDoc::CSploadDoc()
{
	// TODO: add one-time construction code here

}

CSploadDoc::~CSploadDoc()
{
}

BOOL CSploadDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CSploadDoc serialization

void CSploadDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSploadDoc diagnostics

#ifdef _DEBUG
void CSploadDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSploadDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSploadDoc commands
