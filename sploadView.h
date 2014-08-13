// sploadView.h : interface of the CSploadView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLOADVIEW_H__86F86913_FC86_48C5_978F_CAEF5D9FB54D__INCLUDED_)
#define AFX_SPLOADVIEW_H__86F86913_FC86_48C5_978F_CAEF5D9FB54D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxinet.h>
#include <afxsock.h>		// MFC socket extensions
#include <afxole.h>
#include "DropFiles.h"

#include "PPTooltip.h"

#define WM_USER_PASTECOMBO 0x401
#define WM_USER_SETTRAYNOTIFY 0x402


class THREADDATA
{
public:
  void* parent;
  CHttpFile* pFile;
  DWORD startload; 
  DWORD endload;
  DWORD dwServiceType;
  TCHAR strServer[512];
  TCHAR strObject[512];
  INTERNET_PORT nPort;
  TCHAR filename[512];
  DWORD nByteLoad;  
  int isDone;
  int hasError;
  int threadNumber;
  int errorCode;
  int timeDelay;

  int aborted;
  FILE* saveFile; 
  FILE* iniFile;  
  int saveFileInUse; 
  int iniFileInUse; 
  int safeToDel;
 
};

class ASYNCDATA
{
	public:
		int testConnectionComplete;
		int testConnectionReturn;
		void* parent;
 
};


class APDDATA
{
	public:
		int pdComplete;
		int pdReturn;
		int pdForceAbortion;
		void* parent;
 
};




class CSploadView : public CView
{
protected: // create from serialization only
	CSploadView();
	DECLARE_DYNCREATE(CSploadView)

// Attributes
public:
	CSploadDoc* GetDocument();
	CPPToolTip m_tooltip;

	CString username;
	CString password;
	CString urlString;
	CHttpFile* pFile;
	THREADDATA* threadsinfo[8];
	int failCondition;
	CString mySaveDir;
	int stopDownload;
	double downloadRate;	
	int restartCount;
	int restartLoad;
	int thread_restart_running;
	int thread_restart_ended;
	int thread_started;
	int downloadType;
	CString myUrlString;

	int SendTimeoutValue;
	int StartedLaunchingThreads;
	int PreventLaunchingThreads;

	COleDropTarget m_dropTarget;
	CPen m_shadowPen;
	CBrush m_shadowBrush;
	CBrush m_highlightBrush;
	CBrush m_clickedBrush;

	//Graphics
	int colorgray;
	int colorredR;
	int colorredG;
	int colorredB;
	int connectedState;
	
	int lightUpArrow;

	//Status Text
	CString statusText;

	//Scrolling
	int scrollbarTop;
	int scrollbarBottom;
	int scrollbarRange;
	CSize dstRange;
	int ScrollBarPos; //varies from 0 to scrollbarRange
	int ScrollPosDst; //varies from 0 to (80 * number_of_urls - scrollrect height)
	int IsScrollable;
	CRect m_ScrollRect;

	int draggingEnabled;
	int dragPointOffset;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSploadView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:		
	void ShowMessage(CString msg, CString msgTitle); 
	void ShowMessage(CString msg, CString msgTitle, CPoint ptIcon) ;
	void StartDownload();
	void StopDownload();
	BOOL PrepareFileBuff(CDropFiles& DropFiles,CString filename);
	int PrepareDataTimeout(int timeout);
	int TestConnectionTimeout(int timeout);
	int ModifyDownloadArrayItem(CString str,int n);
	int AddDownloadArrayItem(CString str);
	int replaceThread(int i);
	int restartDownloadThread(int i);
	int CheckContext(CRect ScrollRect, int xscrollpos, int yscrollpos,CPoint pt);
	BOOL OnAddItem(CString pData);
	int MarkClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems,CPoint pt);
	int CheckClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems);
	int DetectClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems,CPoint pt);
	CRect PrepareScrollRect();
	int ComputeScrollRanges();
	int ForceExit();
	void RestartDownload();
	int MergeFile(CString savePath);		
	int PrepareData(CString urlStr,CString savePath,int* pdForceAbortion);
	BOOL SupportMultiple(CString urlStr,DWORD &doclen);	
	int StartDownloadThread(THREADDATA* threadinfo);
	int StartDownloadThreadSingle(THREADDATA* threadinfo);
	int PrepareDataSingle(CString urlStr,CString savePath);
	virtual ~CSploadView();	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSploadView)
	afx_msg void OnButtondownload();
	afx_msg void OnDestroy();
	afx_msg void OnButtonpause();
	afx_msg void OnUpdateButtondownload(CCmdUI* pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextmenuDelete();
	afx_msg void OnContextmenuMoveup();
	afx_msg void OnContextmenuMovedown();
	afx_msg void OnContextmenuSaveto();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnButtonforcestop();
	afx_msg void OnFileStartdownload();
	afx_msg void OnUpdateFileStartdownload(CCmdUI* pCmdUI);
	afx_msg void OnFileStopdownload();
	afx_msg void OnUpdateFileStopdownload(CCmdUI* pCmdUI);
	afx_msg void OnFileRemoveallitems();
	afx_msg void OnFileShowdownloadfolder();
	afx_msg void OnTrayRestore();
	afx_msg void OnFileAddurl();
	afx_msg void OnUpdateFileRemoveallitems(CCmdUI* pCmdUI);
	afx_msg void OnHelpContents();
	afx_msg void OnHelpConnectfusionwebsite();
	afx_msg void OnHelpConnectfusionsource();
	afx_msg void OnHelpConnectfusionfaq();
	afx_msg void OnFileStartdownload1();
	afx_msg void OnFileStopdownload1();
	afx_msg void OnFileSetdownloadfolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextmenuClearerror();
};

#ifndef _DEBUG  // debug version in sploadView.cpp
inline CSploadDoc* CSploadView::GetDocument()
   { return (CSploadDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLOADVIEW_H__86F86913_FC86_48C5_978F_CAEF5D9FB54D__INCLUDED_)
