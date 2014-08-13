// sploadDoc.h : interface of the CSploadDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLOADDOC_H__2E0156A7_F74F_4AA4_A9F8_D2A39369847B__INCLUDED_)
#define AFX_SPLOADDOC_H__2E0156A7_F74F_4AA4_A9F8_D2A39369847B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSploadDoc : public CDocument
{
protected: // create from serialization only
	CSploadDoc();
	DECLARE_DYNCREATE(CSploadDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSploadDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSploadDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSploadDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLOADDOC_H__2E0156A7_F74F_4AA4_A9F8_D2A39369847B__INCLUDED_)
