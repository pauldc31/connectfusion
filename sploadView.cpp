// sploadView.cpp : implementation of the CSploadView class
//
// Author    :  Grandhill Technology
//
//
//Credits  
//=======
//Sergey S. : WinInet: Implementing Resuming Feature 
//Alex Rest : Using MFC For Drag '& Drop, Cut, Copy, and Paste
//Eugene Pustovoyt  : CPPTooltip v2.0
//Bruno Podetti : Owner Drawn Menu with Icons, Titles and Shading

#include "stdafx.h"
#include "spload.h"


#include "sploadDoc.h"
#include "sploadView.h"
#include <Mmsystem.h>
//#include <afxtempl.h>

#include "AddURLDialog.h"
#include "FolderDlg.h"
#include <stdio.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




UINT DownloadThread(LPVOID pParam);
UINT DownloadSingleThread(LPVOID pParam);
UINT DetectDownloadThread(LPVOID pParam);
UINT WaitDownloadComplete(LPVOID lParam);
UINT restartSequentialWithResume(LPVOID lParam);


FILE* openFile(CString filename);
FILE* openFileINI(CString filename);
int closeFile(FILE* fx);
int seekFile(FILE* fx,long offset);
int readWriteFile(CStdioFile* pFile,FILE* fx,FILE* fy,DWORD startload,DWORD endload,THREADDATA* threadinfo);
int readWriteFileHttp(CHttpFile* pFile,FILE* fx,FILE* fy,DWORD startload,DWORD endload,THREADDATA* threadinfo);
int closeFileINI(FILE* fy);
int setMainINICompleted(CString delfile);
int updateStatistics(FILE* fy,DWORD startload,DWORD endload,DWORD nBytesLoad,THREADDATA* threadinfo);

CString GetModuleDirectory();
int copyStr(TCHAR *dst,CString src);

int createMainINI(CString filename,CString urlStr,THREADDATA* threadsinfo[],DWORD doclen,DWORD iscompleted);
DWORD verifyPart(CString filename, DWORD &r_startload,DWORD &r_endload, DWORD &ret_len);
DWORD GetFileLength(CString filename);
int mainVerifyCreate(CString filename,CString urlStr,THREADDATA* threadsinfo[],DWORD doclen,DWORD partsStatus[]);
int TestConnection(CString myUrlString);

#define  RELOAD_PART 0
#define  RETRY_PART 1
#define  RELOAD_RANGE 2 //use to load a single file (with a single thread)
#define  RELOAD_SINGLE 4 //same as reload range, but also set the savePath


#define RET_NO_ERROR 0
#define RET_PARTIAL_ERROR 1
#define RET_FULL_ERROR 2
#define RET_COMPLETED 4
#define RET_DO_NOTHING 8

#define SINGLE_THREAD 1
#define MULTIPLE_THREAD 2

int SendRequestTimeout(CHttpFile* pFile, int timeout, THREADDATA* threadinfo);
DWORD WINAPI WorkerSendRequest(IN LPVOID vThreadParm);

int PrepareDataTimeout(CSploadView*  parent, int timeout);
DWORD WINAPI WorkerPrepareData(LPVOID vThreadParm);

void DrawGradient (CRect rectangle, COLORREF Color1, COLORREF Color2, CDC* pDC);
void DrawGradientVert (CRect rectangle, COLORREF Color1, COLORREF Color2, CDC* pDC);
DWORD GetIpAddress(char *hostname);
DWORD GetMyIpAddress();
CString GetAddress();
int CheckConnection();
int verifyRedirect(CString filename);
int verifyRedirect2(CString filename);


int AddDownloadArrayItem(CString str);
int FreeDownloadArray();
int RemoveDownloadArrayItem(int n);
int SaveDownloadArray(FILE* pSaveFile);
int LoadDownloadArray(FILE* pSaveFile);
int InitDownloadArray();
int DrawDownloadArray(CDC* pDC, CDC* pBltDC, CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem, THREADDATA*  threadsinfo[],double downloadRate);
int verifyNameAble(CString nameString,CString& extension);
int verifyNameAble2(CString nameString,CString& extension);
int verifyNameAbleAdd(CString nameString,CString& extension);

CArray <CString,CString> downloadArray;
CArray <CString,CString> mainFileArray;
CArray <DWORD,DWORD> statusArray;
CArray <DWORD,DWORD> totalBytesArray;
CArray <DWORD,DWORD> loadedBytesArray;
CArray <DWORD,DWORD> resumableArray;
CString downloadFolder("");

#define ST_PENDING 0
#define ST_COMPLETED 1
#define ST_PARTIAL 2  
#define ST_ERROR 3
#define ST_DOWNLOADINPROGRESS 4


CRect gDelBox[100];
CRect gUpBox[100];
CRect gDownBox[100];
CRect gSaveBox[100];
CSize gExtent(0,0);

int gLightUpDelBox[100];
int gLightUpUpBox[100];
int gLightUpDownBox[100];
int gLightUpSaveBox[100];
int doInvalidateLightUp = 0;

CBrush lightUpBrush;
CBrush whiteBrush;

int currentActive = -1;
int currentRightClickActive = -1;
int downloadInProgress = 0;
int upDateDownloadRate = 0;

double rateGraphY[10];

CString redirectURL;
int progressStatus = 0; //Stopped, 1- Connecting (item n), 0-Downloading (item n), 2-Merging File (item n)

int threadRestarting = 0;
int userStopPressed = 0;
int forceAbortion = 0;
int forceStop(int i,THREADDATA* threadsinfo[]);

CWnd* mainView = NULL;
CArray<THREADDATA*,THREADDATA*> delArray;

int needDialog = 0;

ASYNCDATA* retTCResults = NULL; //only one results at a time
APDDATA*   retPdResults = NULL; //only one results at a time
int WaitDownloadCompleteStarted = 0;

int mouseMoveHit = -1;
int old_mouseMoveHit = -1;

int specialcase = 0;

HANDLE hgBkBitmap = NULL;
HANDLE hgUpBox1Bitmap = NULL;
HANDLE hgUpBox2Bitmap = NULL;
HANDLE hgDownBox1Bitmap = NULL;
HANDLE hgDownBox2Bitmap = NULL;
HANDLE hgDelBox1Bitmap = NULL;
HANDLE hgDelBox2Bitmap = NULL;
HANDLE hgSaveBox1Bitmap = NULL;
HANDLE hgSaveBox2Bitmap = NULL;
HANDLE hgOpensourceBitmap = NULL;


int DrawGradientBackground(CDC* pDC,CRect backgroundRect,int gradientType);
CSize GetBitmapDim(HANDLE hBitmap);

int exitStatus;

int urlThreadReady = 1;
UINT ADDURLThread(LPVOID pParam);

CFont font;
int upDateCount=0;
double index100 = 100;

extern int AddMode;
extern CString AddModeItem;
HANDLE hgHandIcon;

int firstUse = 0;
int m_tooltipCreated = 0;
int resetErrors = 0;

/////////////////////////////////////////////////////////////////////////////
// CSploadView

IMPLEMENT_DYNCREATE(CSploadView, CView)

BEGIN_MESSAGE_MAP(CSploadView, CView)
	//{{AFX_MSG_MAP(CSploadView)
	ON_COMMAND(ID_BUTTONDOWNLOAD, OnButtondownload)
	ON_WM_DESTROY()
	ON_COMMAND(ID_BUTTONPAUSE, OnButtonpause)
	ON_UPDATE_COMMAND_UI(ID_BUTTONDOWNLOAD, OnUpdateButtondownload)
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_CONTEXTMENU_DELETE, OnContextmenuDelete)
	ON_COMMAND(ID_CONTEXTMENU_MOVEUP, OnContextmenuMoveup)
	ON_COMMAND(ID_CONTEXTMENU_MOVEDOWN, OnContextmenuMovedown)
	ON_COMMAND(ID_CONTEXTMENU_SAVETO, OnContextmenuSaveto)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_BUTTONFORCESTOP, OnButtonforcestop)
	ON_COMMAND(ID_FILE_STARTDOWNLOAD, OnFileStartdownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_STARTDOWNLOAD, OnUpdateFileStartdownload)
	ON_COMMAND(ID_FILE_STOPDOWNLOAD, OnFileStopdownload)
	ON_UPDATE_COMMAND_UI(ID_FILE_STOPDOWNLOAD, OnUpdateFileStopdownload)
	ON_COMMAND(ID_FILE_REMOVEALLITEMS, OnFileRemoveallitems)
	ON_COMMAND(ID_FILE_SHOWDOWNLOADFOLDER, OnFileShowdownloadfolder)
	ON_COMMAND(ID_TRAY_RESTORE, OnTrayRestore)
	ON_COMMAND(ID_FILE_ADDURL, OnFileAddurl)
	ON_UPDATE_COMMAND_UI(ID_FILE_REMOVEALLITEMS, OnUpdateFileRemoveallitems)
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(ID_HELP_CONNECTFUSIONWEBSITE, OnHelpConnectfusionwebsite)
	ON_COMMAND(ID_HELP_CONNECTFUSIONSOURCE, OnHelpConnectfusionsource)
	ON_COMMAND(ID_HELP_CONNECTFUSIONFAQ, OnHelpConnectfusionfaq)
	ON_COMMAND(ID_FILE_STARTDOWNLOAD1, OnFileStartdownload1)
	ON_COMMAND(ID_FILE_STOPDOWNLOAD1, OnFileStopdownload1)
	ON_COMMAND(ID_FILE_SETDOWNLOADFOLDER, OnFileSetdownloadfolder)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	//ON_MESSAGE(WM_USER_RESTART,RestartDownload)
	ON_COMMAND(ID_CONTEXTMENU_CLEARERROR, OnContextmenuClearerror)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSploadView construction/destruction

CSploadView::CSploadView()
{
	// TODO: add construction code here

	username="user";
	password="password";
	//urlString="http://www.connectfusion.com/index.htm";	
	

	pFile = NULL;
	failCondition = 0;
	mySaveDir = "~temp.tmp";

	stopDownload = 0;
	downloadRate = 0.0;
	downloadInProgress = 0;
	restartCount = 0;

	restartLoad = 0;
	thread_restart_running = 1;
	thread_restart_ended = 0;
	thread_started = 0;

	downloadType = MULTIPLE_THREAD;
	myUrlString = "";
	

	SendTimeoutValue = 60000; //60 seconds

	colorgray = 180;

	colorredR =174;
	colorredG =192;
	colorredB =183;

	connectedState = 0;
	lightUpArrow = 0; //0==>no light up, else 1,2,3,4 indicates the arrow to highlight
	
	//statusText = _T("Ready");
	statusText.LoadString(IDS_READY);

	//Scrolling
	scrollbarTop = 0;
	scrollbarBottom = 0;;
	scrollbarRange = 0;;
	dstRange.cx = 0;
	dstRange.cy = 0;
	ScrollBarPos = 0;
	ScrollPosDst = 0;

	IsScrollable = 0;
	m_ScrollRect.left = 0;
	m_ScrollRect.top = 0;
	m_ScrollRect.right = 0;
	m_ScrollRect.bottom = 0;

	draggingEnabled = 0;
	dragPointOffset = 0;

	for (int i=0;i<8;i++)
		threadsinfo[i]=NULL;

	for (i=0;i<100;i++)
	{
		gLightUpDelBox[i]=0;
		gLightUpUpBox[i]=0;
		gLightUpDownBox[i]=0;
		gLightUpSaveBox[i]=0;

	}

	redirectURL = "";

	delArray.RemoveAll();

	exitStatus = 0;

	
}


CSploadView::~CSploadView()
{


}

BOOL CSploadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSploadView drawing

void CSploadView::OnDraw(CDC* pDC)
{
	//CSploadDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);

	
	if (!thread_started)
	{
		TRACE("\nStarting DetectDownload Thread");
		thread_restart_running = 1;
		CWinThread* pThread = AfxBeginThread(DetectDownloadThread,this);

		CString titleStr;
		titleStr.LoadString(IDS_SPEEDLOAD);
		AfxGetMainWnd()->SetWindowText(titleStr);
		
		
		thread_started = 1;

	}

	
	

	/////////////////// Drawing /////////////////
	int buffering =0 ;
	CRect clientrect;	
	GetClientRect(&clientrect);

	//Offscreen Buffer
	CDC dc;
	CBitmap bitmap;
	CBitmap* pOldBitmap;
	CDC* pOldDC = pDC;
	if (dc.CreateCompatibleDC(pDC))
	{
		if (bitmap.CreateCompatibleBitmap(pDC, clientrect.Width(), clientrect.Height()))
		{
			buffering =1;			
			pDC = &dc;
			pOldBitmap = dc.SelectObject(&bitmap);

		}
	}
	
	//Background
	
	COLORREF Color1 = RGB(255,255,255);	
	COLORREF Color2   = RGB(221,222,212);	
	DrawGradient (clientrect, Color1,  Color2,  pDC);

	//Fill physical paper white
	CRect listrect;
	listrect = clientrect;
	listrect.DeflateRect(15,15,15,15);

	CRect shadowrect = listrect;
	shadowrect.OffsetRect(5,3);
	shadowrect.left = listrect.right;
	::FillRect(pDC->m_hDC,&shadowrect, (HBRUSH)GetStockObject(LTGRAY_BRUSH)); //paper

	shadowrect = listrect;
	shadowrect.OffsetRect(5,3);
	shadowrect.top = listrect.bottom;
	::FillRect(pDC->m_hDC,&shadowrect, (HBRUSH)GetStockObject(LTGRAY_BRUSH)); //paper

	::FillRect(pDC->m_hDC,&listrect, (HBRUSH)GetStockObject(WHITE_BRUSH)); //paper

	
	

	//Draw a dark outline around the paper
	CPen *oldPen;
	CBrush *oldBrush = NULL;
	oldPen=pDC->SelectObject(&m_shadowPen);
	pDC->MoveTo(listrect.left-1,listrect.top+1);
	pDC->LineTo(listrect.right+1,listrect.top+1);
	pDC->LineTo(listrect.right+1,listrect.bottom-1);
	pDC->LineTo(listrect.left-1,listrect.bottom-1);
	pDC->LineTo(listrect.left-1,listrect.top+1);

	pDC->MoveTo(listrect.left-1,listrect.top+50);
	pDC->LineTo(listrect.right+1,listrect.top+50);

	//Scrollbars
	pDC->MoveTo(listrect.left-1,listrect.bottom-18);
	pDC->LineTo(listrect.right+1,listrect.bottom-18);

	pDC->MoveTo(listrect.right-18,listrect.bottom-1);
	pDC->LineTo(listrect.right-18,listrect.top+50);

	
	pDC->MoveTo(listrect.right-18,listrect.bottom-36);
	pDC->LineTo(listrect.right+1,listrect.bottom-36);

	pDC->MoveTo(listrect.right-18,listrect.top+68);
	pDC->LineTo(listrect.right+1,listrect.top+68);
		
	pDC->SelectObject(oldPen);


	//Scrolling Arrows
	oldBrush=pDC->SelectObject(&m_shadowBrush);

	CPoint triangle[3];

	if (lightUpArrow==3)
	{		
		pDC->SelectObject(&m_highlightBrush);		
	}
	else
		pDC->SelectObject(&m_shadowBrush);


	//right, right-18, bottom - 18, bottom - 36
	triangle[0].x = listrect.right - 18 + 4;
	triangle[0].y = listrect.bottom - 36 + 4;

	triangle[1].x = listrect.right - 4;
	triangle[1].y = listrect.bottom - 36 + 4;

	triangle[2].x = listrect.right - 9 ;
	triangle[2].y = listrect.bottom - 18 - 4;

	pDC->Polygon(triangle,3);


	if (lightUpArrow==4)
	{		
		pDC->SelectObject(&m_highlightBrush);		
	}
	else
		pDC->SelectObject(&m_shadowBrush);


	//right, right-18, top + 50, top +50 + 18
	triangle[0].x = listrect.right - 18 + 4;
	triangle[0].y = listrect.top + 50 + 18 - 4;

	triangle[1].x = listrect.right - 4;
	triangle[1].y = listrect.top + 50 + 18 - 4;

	triangle[2].x = listrect.right - 9 ;
	triangle[2].y = listrect.top  + 50 + 4;

	pDC->Polygon(triangle,3);
	pDC->SelectObject(oldBrush);
	
	//Download button
	CRect buttonrect;	
	buttonrect.left= 25;
	buttonrect.right= 100;
	buttonrect.top= 25;
	buttonrect.bottom= 40;	
	Color1   = RGB(221,222,212);
	Color2 = RGB(255,255,255);	


	if (!userStopPressed)
	{
		//Draw outline around download button
		pDC->Rectangle(buttonrect.left-3,buttonrect.top-3,buttonrect.right+3,buttonrect.top + (buttonrect.Height() * 2) );


	}

	DrawGradient (buttonrect, Color1,  Color2,  pDC);	
	buttonrect.left= 25;
	buttonrect.right= 100;
	buttonrect.top= 40;
	buttonrect.bottom= 52;	
	Color1 = RGB(255,255,255);	
	Color2   = RGB(colorgray,colorgray,colorgray);
	
	DrawGradient (buttonrect, Color1,  Color2,  pDC);
	buttonrect.left= 25;
	buttonrect.right= 100;
	buttonrect.top= 25;
	buttonrect.bottom= 52;	
	pDC->MoveTo(buttonrect.left-1,buttonrect.top-1);
	pDC->LineTo(buttonrect.right,buttonrect.top-1);
	pDC->LineTo(buttonrect.right,buttonrect.bottom);
	pDC->LineTo(buttonrect.left-1,buttonrect.bottom);
	pDC->LineTo(buttonrect.left-1,buttonrect.top-1);


	//Stop button
	buttonrect.left= 110;
	buttonrect.right= 185;
	buttonrect.top= 25;
	buttonrect.bottom= 40;		
	Color1 = RGB(212,222,217);	
	Color2 = RGB(255,255,255);	


	if (userStopPressed)
	{
		pDC->Rectangle(buttonrect.left-3,buttonrect.top-3,buttonrect.right+3,buttonrect.top + (buttonrect.Height() * 2));

	}	
	
	DrawGradient (buttonrect, Color1,  Color2,  pDC);	
	buttonrect.left= 110;
	buttonrect.right= 185;
	buttonrect.top= 40;
	buttonrect.bottom= 52;	
	Color1 = RGB(255,255,255);	
	//Color2 = RGB(255,255,255);	
	Color2   = RGB(colorredR,colorredG,colorredB);
	//Color2 = RGB(204,0,0);	
	
	
	DrawGradient (buttonrect, Color1,  Color2,  pDC);
	buttonrect.left= 110;
	buttonrect.right= 185;
	buttonrect.top= 25;
	buttonrect.bottom= 52;	
	pDC->MoveTo(buttonrect.left-1,buttonrect.top-1);
	pDC->LineTo(buttonrect.right,buttonrect.top-1);
	pDC->LineTo(buttonrect.right,buttonrect.bottom);
	pDC->LineTo(buttonrect.left-1,buttonrect.bottom);
	pDC->LineTo(buttonrect.left-1,buttonrect.top-1);


		
	HFONT oldFont = (HFONT) SelectObject(pDC->m_hDC,GetStockObject(ANSI_VAR_FONT));


	//Set the extent of the "Save" button
	if ((gExtent.cx ==0) && (gExtent.cy==0))
	{		
			CString savestr;
			//savestr.Format(_T("Save"));
			savestr.LoadString(IDS_SAVE);			
			CSize extent = GetBitmapDim(hgSaveBox1Bitmap);			
			gExtent = extent;
	}


	CString downloadStr;
	downloadStr.LoadString(IDS_DOWNLOAD);
	//downloadStr.Format(_T("Download"));	
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(37,32,downloadStr);
	


	CString stopStr;
	//stopStr.Format(_T("Stop"));	
	stopStr.LoadString(IDS_STOP);
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(135,32,stopStr);
	
	if (connectedState==0)
	{		
		CString errorStr;
		errorStr.LoadString(IDS_CDINC);
		//errorStr.Format(_T("Cannot detect internet connection !"));

		statusText = errorStr;

	
	}

	
	CRect ScrollRect;
	ScrollRect.left = listrect.left ; 	
	ScrollRect.top = listrect.top  + 50;
	ScrollRect.bottom = listrect.bottom - 18;
	ScrollRect.right = listrect.right - 18;
	int xscrollpos = 0;
	int yscrollpos = 0;

	
	int num = downloadArray.GetSize();
	int fullSizeX = ScrollRect.Width();
	int fullSizeY = num*80;


	//Need Scrolling
	if (fullSizeY > ScrollRect.Height())
	{
		CRect thumbrect;	
		thumbrect.left= listrect.right-18;
		thumbrect.right= listrect.right;
		thumbrect.top = listrect.top  + 50 + 18 + yscrollpos;
		thumbrect.bottom = thumbrect.top + 15;	//thumbrect height 15

		thumbrect.OffsetRect(0,ScrollBarPos);		

		Color1   = RGB(180,180,180);
		Color2 = RGB(255,255,255);	
		CRect thumbhalf1 = thumbrect;
		CRect thumbhalf2 = thumbrect;
		
		thumbhalf1.DeflateRect(1,0,1,0);
		thumbhalf2.DeflateRect(1,0,1,0);
		thumbhalf1.right = (thumbhalf1.left + thumbhalf1.right)/2;		
		thumbhalf2.left = (thumbhalf2.left + thumbhalf2.right)/2;

		DrawGradientVert (thumbhalf1, Color1,  Color2,  pDC);	
		DrawGradientVert (thumbhalf2, Color2,  Color1,  pDC);	

		pDC->MoveTo(thumbrect.left+1,thumbrect.top);
		pDC->LineTo(thumbrect.right-1,thumbrect.top);
		pDC->LineTo(thumbrect.right-1,thumbrect.bottom);
		pDC->LineTo(thumbrect.left+1,thumbrect.bottom);
		pDC->LineTo(thumbrect.left+1,thumbrect.top);		
		

		yscrollpos = ScrollPosDst;

	}
	else
	{
		xscrollpos = 0;
		yscrollpos = 0;

	}	

	
	pDC->TextOut(listrect.left+8,listrect.bottom-17,statusText);

	(HFONT) SelectObject(pDC->m_hDC,HFONT(font));

	
	
	CString statisticStr;	
	CString formattingStr;
	if (!downloadInProgress)
	{	
		double displayRate = 0.0;
		
		formattingStr.LoadString(IDS_STATSTR);
		statisticStr.Format(formattingStr,displayRate);
	}
	else
	{
		formattingStr.LoadString(IDS_STATSTR);
		statisticStr.Format(formattingStr,downloadRate);
	}
	pDC->TextOut(listrect.right-200,43,statisticStr);

	if (!downloadInProgress)
	{
		for (int l=9;l>0;l--)
		{		
			rateGraphY[l] = rateGraphY[l-1];
		}	
		rateGraphY[0] = 0.0;

		//TRACE("\ndownloadInProgress = 0");


	}
	else if (upDateDownloadRate)
	{
		
		for (int l=9;l>0;l--)
		{		
			rateGraphY[l] = rateGraphY[l-1];
		}	
		rateGraphY[0] = downloadRate;

		upDateDownloadRate = 0;

		if (upDateCount<5)
			upDateCount++;
		else if (upDateCount==5)
		{
			double ttRate=0;
			for (int l=2;l>=0;l--)
			{		
				ttRate += rateGraphY[l];
			}	
			ttRate /= 3;
			if (ttRate<7)
				 index100 = 12;	
			else if (ttRate>120)
				 index100 = ttRate * 1.2;

			upDateCount=6;

		}


	}

	
	CRect rateRect;	
	COLORREF Color2R   = RGB(104,104,153);	
	COLORREF Color1R = RGB(204,0,0);	
	int base = listrect.right-90;
	int bottombase = 55;
	int ww = 8;	
	double fullY = 24;

	for (int k=0;k<10;k++)
	{		
		
		rateRect.left = base + (9-k)*ww;
		rateRect.right = base + (9-k)*ww + (ww - 2);
		rateRect.bottom = bottombase;
		rateRect.top = bottombase - (long) (rateGraphY[k] * fullY / index100) ;

		if (rateRect.top < bottombase - fullY)
			rateRect.top = (long) (bottombase - fullY);		
		else if (rateRect.top>bottombase)
			rateRect.top = bottombase;
		
		DrawGradient (rateRect, Color1R,  Color2R,  pDC);

	}
	

	

	//Clipping, put at the end

	while (threadRestarting)
	{
		Sleep(80);
	}

	threadRestarting = 1;


	//put this before  IntersectClipRect
	CDC bltDC;
	bltDC.CreateCompatibleDC(pOldDC);
	HANDLE tempHandle = bltDC.SelectObject(hgBkBitmap);
	pDC->BitBlt(200,5,210,26,&bltDC,0,0, SRCCOPY);	//160, 39 is dimension of bitmap
	bltDC.SelectObject(tempHandle);	

	pDC->IntersectClipRect( &ScrollRect);
	

	{

		int num = downloadArray.GetSize();
		int desired = ScrollRect.Height()/80 + 1;

		if (num<desired)
		{
			

			Color1 = RGB(101,133,154);
			Color2 = RGB(255,255,255);
			CRect decohalf1 = ScrollRect;
			DrawGradient (decohalf1, Color1,  Color2,  pDC);	


		}

		
	}
	
	DrawDownloadArray(pDC,&bltDC, ScrollRect, xscrollpos, yscrollpos, -1, threadsinfo,downloadRate);

	threadRestarting = 0;
	
	
	
	SelectObject(pDC->m_hDC,oldFont);

	
	
	

	if (buffering)
	{
		pOldDC->BitBlt(clientrect.left, clientrect.top, clientrect.Width(), clientrect.Height(),
			&dc, 0, 0, SRCCOPY);

		dc.SelectObject(pOldBitmap);

		bitmap.DeleteObject();
		dc.DeleteDC();


	}


	
	if (firstUse)
	{

		
		CString mstr;
		CString ostr;
		mstr.LoadString(IDS_DEFAULTPATH);
		ostr.Format(mstr,downloadFolder);

		CString str1;
		str1.LoadString(IDS_INITIATE);			

		CString tip1;
		tip1.LoadString(IDS_TIP);			

		CPoint ptIcon;			

		ptIcon.x=80;
		ptIcon.y=220;
		ClientToScreen(&ptIcon);

		
		ShowMessage(str1,tip1,ptIcon);	
		firstUse = 0;
			

	}


}

/////////////////////////////////////////////////////////////////////////////
// CSploadView printing

BOOL CSploadView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSploadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

void CSploadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{

}

/////////////////////////////////////////////////////////////////////////////
// CSploadView diagnostics

#ifdef _DEBUG
void CSploadView::AssertValid() const
{
	CView::AssertValid();
}

void CSploadView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

/*
CSploadDoc* CSploadView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSploadDoc)));
	return (CSploadDoc*)m_pDocument;

	
}
*/
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSploadView message handlers

void CSploadView::OnButtondownload() 
{
	connectedState =  CheckConnection();
	if (connectedState)		
		restartLoad = 1;
	
}



int CSploadView::SupportMultiple(CString urlStr,DWORD &doclen)
{

	int support = 0;

	//need to parse and ensure the url is http first	
	HINTERNET hInternet = InternetOpen("IE6",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);

	if (hInternet)
	{	
		
				CString headersStr("");						
				HINTERNET hFile = InternetOpenUrl(hInternet,urlStr,headersStr,0,INTERNET_FLAG_RELOAD,0);									

				if (hFile)
				{
					 
						 
						 int retSet = InternetSetFilePointer(hFile,0,0,FILE_END,0);
						 if (retSet>0)
						 {

							 TRACE("\nSetFile Pointer returns %d",retSet);
							 doclen=retSet;
							 support = 0;
							 

						 }
						 else
						 {							 
							TRACE("\nServer does not support resume download");
							support = 1;

						 }
					 

					 InternetCloseHandle(hFile);

				}
				else
				{
					TRACE("\nError Opening Internet Connection to file");
					support = 2;
				}


		InternetCloseHandle(hInternet);

	}
	else
		support = 3;



	if (support)
		return TRUE;
	else
		return FALSE;



}

int DetectDownloadThread(THREADDATA* threadinfo,void * lparent, int option, DWORD sl, DWORD el, CString savePath,int timeDelay)
{

	if (!threadinfo)
	{
		TRACE("\nNo Thread info!");
		return 1;

	}

	if (!lparent)
	{
		TRACE("\nNo Parent info!");
		return 1;

	}

	CSploadView* parent = (CSploadView*) lparent;

	if ((parent->failCondition) || (parent->stopDownload))
	{

		TRACE("\nCannot Start a thread when failCondition  or stopDownload is not zero");
		return 1;

	}

	if (!(threadinfo->isDone))
	{

		TRACE("\nThread is still running!");
		return 1;

	}

	if (option==RELOAD_PART)
	{
		threadinfo->nByteLoad = 0;

	}
	else if (option==RETRY_PART)
	{
		//Do nothing
		//threadsinfo[i]->nByteLoad

	}
	else if (option==RELOAD_RANGE)
	{

		threadinfo->startload = sl;
		threadinfo->endload = el;
	
	
	}
	else if (option==RELOAD_SINGLE)
	{

		threadinfo->startload = sl;
		threadinfo->endload = el;

		copyStr(threadinfo->filename,savePath);	
	}
	else
	{

		TRACE("\nInvalid Restart Option");
		return 1;

	}

	
	threadinfo->isDone = 0;
	threadinfo->hasError = 0;
	threadinfo->errorCode = 0;
	threadinfo->timeDelay = timeDelay;	

	TRACE("\nRestarting thread %d after %d secs",threadinfo->threadNumber,timeDelay);

	parent->StartDownloadThread(threadinfo);		


	return 0;
}


int CSploadView::PrepareData(CString urlStr,CString savePath,int* pdForceAbortion)
{
	failCondition = 0;
	stopDownload = 0;
	downloadType = MULTIPLE_THREAD;
		
	
	int retStatus =0 ;	

	DWORD doclen;
	int retVal= SupportMultiple(urlStr,doclen);
	int supportMul =retVal;

	TRACE("\nSupportMultiple : %d",retVal);

	if (supportMul == 2)
	{
		retStatus = 3; //download aborted
		return retStatus;

	}
	
	if (retVal!=0)
	{
		retStatus = 2;
		return retStatus;
	}
	//else proceed
	

   DWORD dwServiceType;
   CString strServer;
   CString strObject;
   INTERNET_PORT nPort;
   retVal = AfxParseURL(urlStr, dwServiceType, strServer, strObject,nPort);

   if ((retVal==0) || (dwServiceType!=AFX_INET_SERVICE_HTTP) || (strServer==""))
   {
	   TRACE("\nUnable to parse URL");
	   return 3;

   }

   
	CInternetSession iss( "IE6", 1,  INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE );			
	CHttpConnection* htc = NULL;

	TRY
	{
		
		htc = iss.GetHttpConnection( strServer, nPort,username ,password );
	
	}
	CATCH(CException, excep)
	{
		TRACE("\nException thrown in getting connection");
		htc = NULL;

	}
	END_CATCH
	
	if (htc)
	{
		
		pFile =  htc->OpenRequest( CHttpConnection::HTTP_VERB_GET, strObject,  NULL, 1, NULL, NULL,  INTERNET_FLAG_RELOAD );
		

		if (pFile)
		{
			

			CString headersStr;
			headersStr.Format("Range: bytes=0-");
			pFile->AddRequestHeaders(headersStr);
			pFile->SendRequest();

			
			DWORD retReq = 0;
			pFile->QueryInfoStatusCode(retReq);
			if ((retReq>=200) && (retReq<=299))
			{
				
				//char querybuf[100];
				DWORD querybuf;
				DWORD len=4;
				pFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,&querybuf,&len, NULL);

				
				CString retString;
				pFile->QueryInfo(HTTP_QUERY_VERSION, retString, NULL);
				TRACE("\nHttp Version %s",retString);										


				char querybuf2[1000];
				DWORD len2=1000;
				pFile->QueryInfo(53 ,&querybuf2,&len2, NULL); //content range
				if ((len2<1000) && (len2>=0))
					TRACE("\n[QUERY] Content Range Returns :  %s",querybuf2);					
				else
					TRACE("\n[QUERY] Content Range Returns :  NULL");					

				len2=1000;
				pFile->QueryInfo(33 ,&querybuf2,&len2, NULL); //location
				if ((len2<1000) && (len2>=0))
					TRACE("\n[QUERY] Location Returns :  %s",querybuf2);					
				else
					TRACE("\n[QUERY] Location Returns : NULL");	


				len2=1000;
				pFile->QueryInfo(3 ,&querybuf2,&len2, NULL); //location
				if ((len2<1000) && (len2>=0))
					TRACE("\n[QUERY] Content-ID Returns :  %s",querybuf2);					
				else
					TRACE("\n[QUERY] Content-ID Returns : NULL");	

				len2=1000;
				pFile->QueryInfo(38 ,&querybuf2,&len2, NULL); //location
				if ((len2<1000) && (len2>=0))
					TRACE("\n[QUERY] Title Returns :  %s",querybuf2);					
				else
					TRACE("\n[QUERY] Title Returns : NULL");	


				len2=1000;
				pFile->QueryInfo(21 ,&querybuf2,&len2, NULL); //location
				if ((len2<1000) && (len2>=0))
				{			
					int lenx;
					lenx = len2;
					for (int i=0;i<lenx;i++)
					{
						if (querybuf2[i]==0)
							querybuf2[i]='\n';
					}
					
					TRACE("\n[QUERY] Raw Headers Returns :  %s",querybuf2);					

				}
				else
					TRACE("\n[QUERY] Raw Headers Returns : NULL");	
				
				
				

				TRACE("\nStatus Return : %d",querybuf);				
				if (querybuf==doclen)
				{
					
					//Partial Content Check
					{

						pFile->Close();
						pFile =  htc->OpenRequest( CHttpConnection::HTTP_VERB_GET, strObject,  NULL, 1, NULL, NULL,  INTERNET_FLAG_RELOAD );

		
						if (pFile)
						{

							CString headersStr;			
							headersStr.Format("Range: bytes=%d-%d",doclen/4,doclen/2);			
							
							TRY {	
							
								pFile->AddRequestHeaders(headersStr);							
								pFile->SendRequest();			
								
								char querybuf2[1000];
								len2=1000;
								pFile->QueryInfo(53 ,&querybuf2,&len2, NULL); //content range
								if ((len2>0) && (len2<1000))
								{
									TRACE("\n[NEW] Partial Content Range :  %s",querybuf2);					
									CString srcstr = querybuf2;
									CString findstr;
									findstr.Format("%d",doclen/4);
									int foundhead = srcstr.Find(findstr);
									findstr.Format("%d",doclen/2);
									int foundtail = srcstr.Find(findstr);
									if ((foundhead>0) && (foundtail>0))
									{
										//Proceed
										TRACE("\nPartial Content Match! Resumable Download Detected ! ");

									}
									else
									{					
										TRACE("\nPartial Content Does Not Match ");
										
										pFile->Close();
										htc->Close();
										iss.Close();
										
										return 2;
									}

								}
								else
									TRACE("\n[NEW] Partial Content Range : NULL");	

							}						
							CATCH (CException, excep)
							{
								TRACE("\nException thrown in Query Info");

							}
							END_CATCH
							
							if (len2==1000)
							{					
								TRACE("\nPartial Content Not Supported ");
								
								pFile->Close();
								htc->Close();
								iss.Close();
								
								return 2;
							}

						}
						else 
						{
							//Abort
							htc->Close();
							iss.Close();
							return 3;

						}

					}	
					
					if (querybuf>24) //break into several conenctions for download
					{

							if (pdForceAbortion)
							{
								if (*pdForceAbortion)
								{
									//Do not proceed to launch the threads
									//as this url is aborted

									TRACE("\nForce Abortion of PrepareData");
									
									pFile->Close();
									htc->Close();
									iss.Close();
									
									return 3;
								}

							}
							
							
							//setting the status array
							statusArray[currentActive] = ST_DOWNLOADINPROGRESS;
							resumableArray[currentActive] = 1;
							totalBytesArray[currentActive] = doclen;
							progressStatus = 0;
					

							DWORD partlen = querybuf/8;
							DWORD startload = 0;
							DWORD endload = 0;
							for (int i=0;i<8;i++)
							{	
								if (i<7)
									endload = startload+partlen-1;
								else
									endload = querybuf-1;						
														

								if (threadsinfo[i]==NULL)       
									threadsinfo[i]=new THREADDATA;
								else if ((threadsinfo[i]->aborted) || (threadsinfo[i]->saveFile!=NULL))    
								{
									if (threadsinfo[i]->safeToDel)
									{
										delete threadsinfo[i];
										threadsinfo[i]= NULL;
										TRACE("\nthreadsinfo[%d] deleted",i);
									}
									else
										delArray.Add(threadsinfo[i]);
									
									threadsinfo[i]=new THREADDATA;
								}
								threadsinfo[i]->parent=this;
								threadsinfo[i]->startload=startload;
								threadsinfo[i]->endload=endload;
								threadsinfo[i]->pFile = NULL;
								threadsinfo[i]->dwServiceType = dwServiceType;	
								copyStr(threadsinfo[i]->strServer,strServer);
								copyStr(threadsinfo[i]->strObject,strObject);								
								threadsinfo[i]->nPort = nPort;
								threadsinfo[i]->isDone = 0;
								threadsinfo[i]->nByteLoad = 0;							
								threadsinfo[i]->hasError = 0;
								threadsinfo[i]->errorCode = 0;
								threadsinfo[i]->threadNumber = i;
								threadsinfo[i]->timeDelay = 0;

								threadsinfo[i]->saveFile = NULL;
								threadsinfo[i]->iniFile = NULL;
								threadsinfo[i]->saveFileInUse = 0;
								threadsinfo[i]->iniFileInUse = 0;
								threadsinfo[i]->aborted = 0;
								threadsinfo[i]->safeToDel = 0;

								
								CString nameStr;							
								nameStr.Format("%d",i);
								//threadsinfo[i]->filename = savePath + nameStr;
								copyStr(threadsinfo[i]->filename,savePath + nameStr);	
								
								startload += partlen;
							}

					
							//will modify the threaddata if the mode is resume
							DWORD partsStatus[8];
							int isResume = mainVerifyCreate(savePath,urlStr,threadsinfo,doclen,partsStatus);	
							if ((isResume != RET_FULL_ERROR) && (isResume != RET_DO_NOTHING))
								TRACE("\nResuming Download");
							

							if (isResume == RET_FULL_ERROR)  
							{
								//Full Reload
								for (i=0;i<8;i++)
								{
									StartDownloadThread(threadsinfo[i]);	

								}

							}
							else if (isResume == RET_NO_ERROR)
							{
								//Some completed, some resume
								for (i=0;i<8;i++)
								{
									if (partsStatus[i]!=RET_COMPLETED)
										StartDownloadThread(threadsinfo[i]);
									else
										threadsinfo[i]->isDone = 1;

								}


							}
							else if (isResume == RET_PARTIAL_ERROR)
							{
								//some broken (rare)
								for (i=0;i<8;i++)
								{
									//Thosde parts with no errors will start with resume
									if ((partsStatus[i]==RET_PARTIAL_ERROR) || (partsStatus[i]==RET_NO_ERROR))
										StartDownloadThread(threadsinfo[i]);	
									else
										threadsinfo[i]->isDone = 1;

								}
							}						
							else if (isResume == RET_COMPLETED)
							{
								//all completed, except they are not merged (rare case)
								for (i=0;i<8;i++)
								{
									threadsinfo[i]->isDone = 1;
								}							
							

							}
							else if (isResume == RET_DO_NOTHING)
							{
								//TRACE("\nSet to Already Completed");							
								retStatus = 1; //already comlpeted
								return retStatus;

							}



					}	
					else
					{
						TRACE("\nTarget file is too small."); //revert to single part download
						retStatus = 2; //download single thread
						//return retStatus;
					}


				}
				else
				{
					TRACE("\nUnreliable content length. Multipart download aborted."); //revert to single part download
					retStatus = 2; //download single thread
					//return retStatus;

				}

			}						
			else
			{
				TRACE("\nError sending request to server : return code %d",retReq); //download aborted
				retStatus = 3; //Download Aborted
				//return retStatus;

			} 
			
			
			pFile->Close();

		}
		else
		{
				TRACE("\nError opening http request");
				retStatus = 3; //Download Aborted
				//return retStatus;

		} 


		htc->Close();

	}
	else
	{
				TRACE("\nError getting http connection");
				retStatus = 3; //Download Aborted
				return retStatus;

	} 

	
	iss.Close();


	return retStatus; 

}


int copyStr(TCHAR *dst,CString src)
{

	int len;
	len = src.GetLength();
	if (len>512)
	{
		TRACE("\nLength of string is too big");
		return 1;


	}

	_tcscpy(dst,LPCTSTR(src));

	return 0;

}


UINT WaitDownloadComplete(LPVOID lParam)
{
	TRACE("\nWaiting for download to complete");
	WaitDownloadCompleteStarted = 1;

	if (lParam==NULL)
	{
		TRACE("\nError launching monitor thread");
		WaitDownloadCompleteStarted = 0;
		return 1;

	}

	int numthreads = 8;
	CSploadView* parent = (CSploadView*) lParam;		
	if (parent->downloadType == SINGLE_THREAD)
	{
		numthreads = 1;

	}	

	DWORD prevLoad[8];
	for (int i=0;i<8;i++)
	{
		if (parent->threadsinfo[i])
			prevLoad[i] = parent->threadsinfo[i]->nByteLoad;
	}


	int allready = 0;
	int showupdate = 0;
	DWORD totalLoaded = 0;
	parent->downloadRate = 0;	
	DWORD prevtime = timeGetTime();
	DWORD currenttime = timeGetTime();
	DWORD timediff = 1;

	DWORD mainPrevTime = 0; 
	
	DWORD prevloadtime[8];
	for (int j=0;j<8;j++)
		prevloadtime[j] = 0;
	
	DWORD currentloadtime = currenttime;
	DWORD loadtimediff = 0;

	while ((!allready) && (!parent->failCondition))
	{

		
		Sleep(100);
		allready = 1;
		showupdate = 0;
		for (i=0;i<numthreads;i++)
		{	
			if (exitStatus) 
				return 0;
	

			if (parent->threadsinfo[i]->isDone == 0)
			{
				//TRACE("\nNot Done %d",i);
				allready = 0;
				//break;
			}
			
			if (exitStatus)
				return 0;
	
			if (parent->threadsinfo[i]->isDone == 0)
			{
				if (parent->threadsinfo[i]->aborted)
				{
					parent->threadsinfo[i]->isDone = 1;
					parent->threadsinfo[i]->hasError = 100;

				}
			}
			

			if (exitStatus)
				return 0;
	

			if (parent->threadsinfo[i]->nByteLoad > prevLoad[i])
			{				

				showupdate=1;
				int bytediff = (parent->threadsinfo[i]->nByteLoad - prevLoad[i]);

				if (bytediff>0)
					totalLoaded += bytediff;
				prevLoad[i] = parent->threadsinfo[i]->nByteLoad;									

				if (parent->threadsinfo[i]->isDone == 0)
						prevloadtime[i]= timeGetTime();

				mainPrevTime = timeGetTime();

			}
			else
			{	
			
			
			}


			if (mainPrevTime)
			{
				int oldloadtime =mainPrevTime;
				currentloadtime = timeGetTime();
				loadtimediff = currentloadtime - oldloadtime;

				if (loadtimediff > 15000) //if there is no response for more than 12  seconds
				{
					TRACE("\nThere is no data received from any threads for more than 15 seconds",i);
					for (int j=0;j<8;j++)
					{
						
						prevloadtime[j]= 0;
						prevLoad[j] = parent->threadsinfo[j]->nByteLoad;
					}
					mainPrevTime = 0;
					

					TRACE("\nFull Abort Initiated, Will Reload All Threads");
					
					forceStop(-1,parent->threadsinfo);
				
				}


			}


		} //while

		

		if (showupdate)
		{
			currenttime = timeGetTime();			
			timediff = currenttime - prevtime;
			
			if (timediff > 3000) //for the 1st 5 seconds do not updaye the download rate
			{				
				parent->downloadRate = (totalLoaded*0.9765625)/(timediff);
				prevtime = currenttime;
				totalLoaded = 0;
				upDateDownloadRate = 1;
			}
			
			
			parent->Invalidate();

		}

	}

	int downloadStatus = 0; //0 ==> successful, 100==>pause/resume , 1==>error


	if (parent->failCondition!=0)
	{
		downloadStatus = 2; //serious error

	}
	else
	{
		for (i=0;i<numthreads;i++)
		{			
			if ((parent->threadsinfo[i]->hasError >0) && (parent->threadsinfo[i]->hasError <100))
			{
				downloadStatus = 1; //thread(s) error
				break;

			}
			else if (parent->threadsinfo[i]->hasError ==100)
			{
				downloadStatus = 100;
			}

		}

	}


	if (downloadStatus ==0)
		TRACE("\nDownload completed."); //setting of status is deferred until merge file
	else if (downloadStatus ==100)
	{
		TRACE("\nDownload Stopped");
		if (currentActive>=0)
		{
			if (resumableArray[currentActive])
				statusArray[currentActive] = ST_PARTIAL;
			else
				statusArray[currentActive] = ST_PENDING;

		}

	}
	else if (downloadStatus ==2)
	{
		TRACE("\nDownload Error : failCondition %d",parent->failCondition);
		if (currentActive>=0)
			statusArray[currentActive] = ST_ERROR;

	}
	else if (downloadStatus ==1)
	{
		TRACE("\nDownload Thread(s) Error");
		

		if (resumableArray[currentActive])
			statusArray[currentActive] = ST_PARTIAL;
		else
			statusArray[currentActive] = ST_PENDING;

	}
	
	
	
	if (parent->downloadType == MULTIPLE_THREAD)
	{

		if (downloadStatus ==0)
		{
			progressStatus = 2;

			//Update loaded bytes
			DWORD totalLoaded = 0;
			for (int j=0;j<8;j++)
			{
				if (parent->threadsinfo[j])
				{
					int nByteLoad = parent->threadsinfo[j]->nByteLoad;							
					totalLoaded += nByteLoad;

				}				

			}
			loadedBytesArray[currentActive] = totalLoaded;			

			parent->Invalidate();
			
			parent->MergeFile(parent->mySaveDir);
			
		}

	}
	else		
	{
		if (downloadStatus ==0)
		{
			statusArray[currentActive] = ST_COMPLETED;
		}
		
	}
						
	progressStatus = 0;
	parent->Invalidate();	
	

	if (exitStatus)
		return 0;
	

	if (parent->downloadType == MULTIPLE_THREAD)
	{
		int allErrors10000 = 1; //all errors code received from the threads are 503
		int anysucceed10000 = 0; //if  allErrors503 is true, anysucceed503 determine whethr there is any threads that as succeeded

		int allErrors503 = 1; //all errors code received from the threads are 503
		int anysucceed503 = 0; //if  allErrors503 is true, anysucceed503 determine whethr there is any threads that as succeeded
		if (downloadStatus==1)
		{
			for (i=0;i<numthreads;i++)
			{	
				if ((parent->threadsinfo[i]->hasError) && (parent->threadsinfo[i]->errorCode!=503))
				{
					allErrors503 = 0;

				}
			}

			for (i=0;i<numthreads;i++)
			{	
				if ((parent->threadsinfo[i]->hasError) && (parent->threadsinfo[i]->errorCode!=10000))
				{
					allErrors10000 = 0;

				}
			}

			if (allErrors503)
			{
				for (i=0;i<numthreads;i++)
				{	
					int sl = parent->threadsinfo[i]->startload;
					int el = parent->threadsinfo[i]->endload;
					int bl = parent->threadsinfo[i]->nByteLoad;

					if (bl == el-sl+1)
						anysucceed503 = 1;

				}
			}
			else  if (allErrors10000)
			{
				for (i=0;i<numthreads;i++)
				{	
					int sl = parent->threadsinfo[i]->startload;
					int el = parent->threadsinfo[i]->endload;
					int bl = parent->threadsinfo[i]->nByteLoad;

					if (bl == el-sl+1)
						anysucceed10000 = 1;

				}
			}

			//2nd Pass Download
			if ((anysucceed503) || (anysucceed10000))
			{			
				
				TRACE("\n2nd Pass Sequential Reload");
				
				if ((parent->failCondition == 0) && (parent->stopDownload==0))
				{
					parent->restartCount--;
					if (parent->restartCount>0)			
						parent->restartLoad = 1; //RestartLoad
					else
						parent->restartCount = 0;
				}			


			}

		}

	}

	//Handle redirected case	
	if (parent->downloadType == SINGLE_THREAD) 	
	{
		if (downloadStatus ==0)  //success
		{
			
			//further checks on filesize etc.
			int redirect = verifyRedirect(parent->mySaveDir);
			if (redirect == 0)
				redirect = verifyRedirect2(parent->mySaveDir);
			
			TRACE("\nRedirect returns %d",redirect);
			
			if (redirect)
			{
				TRACE("\nRedirecting");
				if (currentActive>=0)
				{
					if (redirectURL!="")
					{
						
						int ret = parent->ModifyDownloadArrayItem(redirectURL,currentActive);
						if (ret==0)
						{
							parent->restartLoad = 1;

						}

					}


				}


			}


		}

	}
	
	WaitDownloadCompleteStarted = 0;
	downloadInProgress = 0;
	parent->Invalidate();

	return 0;

}

int CSploadView::MergeFile(CString savePath)
{
	CString fileout;

	FILE* fo = fopen(savePath,"wb");
	if (fo==NULL)
	{
		TRACE("\nError creating save file");
		return 1;

	}

	TRACE("\nMerging Files");


	char buffer[2048];
	int bytesread = 0;
	int byteswritten = 0;

	CString filename;
	for (int i=0;i<8;i++)
	{			

		filename = threadsinfo[i]->filename;
		FILE* fi =fopen(filename,"rb");
		if (fi==NULL)
		{
			TRACE("\nError opening partial file");
			return 2;

		}
		bytesread = fread(buffer,1,2048,fi);
		while (bytesread)
		{
			byteswritten = fwrite(buffer,1,bytesread,fo);
			if (byteswritten != bytesread)
			{
				TRACE("\nError merging the partial files");
				fclose(fi);				
				fclose(fo);
				return 3;

			}

			bytesread = fread(buffer,1,2048,fi);
		}


		fclose(fi);
		
	}

	//Delete the partial files
	for (i=0;i<8;i++)
	{	
		filename = threadsinfo[i]->filename;
		DeleteFile(filename);
		DeleteFile(filename+".ini");

	}

	
	
	CString delfile = savePath+".main.ini";


	setMainINICompleted(delfile);

	if (currentActive>=0)
	{
		statusArray[currentActive] = ST_COMPLETED;		

	}


	TRACE("\nCompleted : %s ",delfile);
	


	fclose(fo);

	return 0;

}

int CSploadView::StartDownloadThread(THREADDATA* threadinfo)
{

	CWinThread* pThread = AfxBeginThread(DownloadThread,threadinfo);	


	return 0;
}

int CSploadView::StartDownloadThreadSingle(THREADDATA* threadinfo)
{

	CWinThread* pThread = AfxBeginThread(DownloadSingleThread,threadinfo);	


	return 0;
}

FILE* openFileINI(CString filename)
{

	FILE* fy = fopen(LPCTSTR(filename),"r+");
	if (fy==NULL)
		fy = fopen(LPCTSTR(filename),"w+");
	return fy;

}

int closeFileINI(FILE* fy)
{
	fclose(fy);
	return 0;

}

//bytesLoad refers to the number of bytes loaded, it is not a number to refer to a point in the file
int updateStatistics(FILE* fy,DWORD startload,DWORD endload,DWORD nBytesLoad,THREADDATA* threadinfo)
{

	char updateStr[100];
	sprintf(updateStr,"%20d,%20d,%20d\n",startload,endload,nBytesLoad);
	
	int percent = (nBytesLoad*100)/(endload-startload+1);
	
	if (threadinfo)
		threadinfo->nByteLoad = nBytesLoad;
	
	if (fy)
	{
		rewind(fy);
		//fsetpos(fy, 0 );
		//fprintf(fy,updateStr);
		fwrite(updateStr,1,63,fy);

	}
		
	return 0;

}



FILE* openFile(CString filename)
{	
	FILE* fx = fopen(LPCTSTR(filename),"r+b");
	if (fx==NULL)
		fx = fopen(LPCTSTR(filename),"w+b");
	return fx;

}

int closeFile(FILE* fx)
{
	fclose(fx);
	return 0;

}

int seekFile(FILE* fx,long offset)
{
		
	return fseek(fx,offset,SEEK_SET);


}

int readWriteFile(CStdioFile* pFile,FILE* fx,FILE* fy,DWORD startload,DWORD endload,THREADDATA* threadinfo)
{


	int wroteBytes = 0;
	CSploadView* parent= (CSploadView*) threadinfo->parent;
	DWORD nByteLoad = threadinfo->nByteLoad;

	int buffertransfer = 2048; //make buffer smaller for slower connection
	char buffer[2048];
	DWORD verifyCount = 0;
	DWORD expectedCount = endload-startload+1;					
	
	
	int retBytes;
		


	TRY {

		retBytes = pFile->Read(buffer,buffertransfer);

	}
	CATCH (CException, excep)
	{
		TRACE("\nException thrown in reading data (1)");


	}
	END_CATCH

	TRACE("\nPostread %d",threadinfo->threadNumber);
	
	
	verifyCount += retBytes;
	
	while(retBytes)
	{	

		while (threadinfo->saveFileInUse != 0) 
		{
			Sleep(100);

		}
		

		if (threadinfo->saveFile)
		{
			threadinfo->saveFileInUse = 1;
			threadinfo->iniFileInUse = 1;

			wroteBytes = fwrite(buffer,1,retBytes,fx);
			if (wroteBytes != retBytes)
			{
				TRACE("\nError writing partial files");
				return 1;
			}
			updateStatistics(fy,startload,endload,verifyCount+nByteLoad,threadinfo);

			threadinfo->saveFileInUse = 0;
			threadinfo->iniFileInUse = 0;

		}

		if (parent->stopDownload)
		{
			//TRACE("\nStop Downlaod Thread %d",startload);
			threadinfo->hasError = 100;
			break;

		}
		else if (threadinfo->aborted)
		{
			//TRACE("\nForce Stop Download Thread %d",startload);
			threadinfo->hasError = 100;
			

			break;

		}
		else
		{
			//TRACE("\nretBytes %d",retBytes);

			TRY {

				retBytes = pFile->Read(buffer,buffertransfer);

			}
			CATCH (CException, excep)
			{

				TRACE("Exception thrown in reading data (2)");


			}
			END_CATCH
			
			verifyCount += retBytes;

		}
		
	}


	TRACE("\nVerify count %d Expected %d startload %d",verifyCount,expectedCount,startload);

	return 0;

}



int readWriteFileHttp(CHttpFile* pFile,FILE* fx,FILE* fy,DWORD startload,DWORD endload,THREADDATA* threadinfo)
{


	int wroteBytes = 0;
	CSploadView* parent= (CSploadView*) threadinfo->parent;
	DWORD nByteLoad = threadinfo->nByteLoad;

	int buffertransfer = 2048; //make buffer smaller for slower connection
	char buffer[2048];
	DWORD verifyCount = 0;
	DWORD expectedCount = endload-startload+1;					
	
	
	int retBytes;
	DWORD allowexceed = expectedCount+buffertransfer*3;

	TRY {

		retBytes = pFile->Read(buffer,buffertransfer);

	}
	CATCH (CException, excep)
	{
		TRACE("\nException thrown in reading data (1)");


	}
	END_CATCH

	TRACE("\nPostread %d",threadinfo->threadNumber);
	
	
	verifyCount += retBytes;
	
	//pFile->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT,5000); //may works only on new IE , will ensure when user break is clicked, the download will stopped with 5 seconds becuase of the exceptions
	TRACE("\nSetting Timeout for reading");
	
	while ((retBytes) && (allowexceed>threadinfo->nByteLoad))
	//while (retBytes) 
	{
		//TRACE("\nAllowed Exceed %d   current %d    vn %d",allowexceed,threadinfo->nByteLoad,verifyCount+nByteLoad);
		
		while ((threadinfo->saveFileInUse != 0) || (threadinfo->iniFileInUse != 0))
		{
			Sleep(100);

		}
		


		//TRACE("\nthread %d,sf %d,if %d",threadinfo->threadNumber,threadinfo->saveFile,threadinfo->iniFile);

		if ((threadinfo->saveFile) && (threadinfo->iniFile))
		{

			threadinfo->saveFileInUse = 1;
			threadinfo->iniFileInUse = 1;

			wroteBytes = fwrite(buffer,1,retBytes,fx);
			if (wroteBytes != retBytes)
			{
				TRACE("\nError writing partial files");
				return 1;
			}
			updateStatistics(fy,startload,endload,verifyCount+nByteLoad,threadinfo);

			threadinfo->saveFileInUse = 0;
			threadinfo->iniFileInUse = 0;


		}

		if (parent->stopDownload)
		{
			//TRACE("\nStop Downlaod Thread %d",startload);
			threadinfo->hasError = 100;
			break;

		}
		else if (threadinfo->aborted)  //force abort
		{
			TRACE("\nAbort Download Thread %d",threadinfo->threadNumber);
			threadinfo->hasError = 100;			
			break;

		}
		else
		{
			//TRACE("\nretBytes %d",retBytes);

			TRY {

				retBytes = pFile->Read(buffer,buffertransfer);

			}
			CATCH (CException, excep)
			{

				TRACE("Exception thrown in reading data (2)");


			}
			END_CATCH
			
			verifyCount += retBytes;

		}
		
	}

	if (allowexceed<threadinfo->nByteLoad)
		TRACE("\nBreak on allowed exceed%d Expected %d startload %d thread %d",verifyCount,expectedCount,startload,threadinfo->threadNumber);
	else
		TRACE("\nVerify count %d Expected %d startload %d thread %d",verifyCount,expectedCount,startload,threadinfo->threadNumber);

	return 0;

}




UINT DownloadThread(LPVOID pParam)
{	

	THREADDATA* threadinfo = (THREADDATA*) pParam;

	DWORD dwServiceType = threadinfo->dwServiceType;
	CString strServer = threadinfo->strServer;
	CString strObject = threadinfo->strObject;
	INTERNET_PORT nPort = threadinfo->nPort;
	DWORD startload = threadinfo->startload;
	DWORD endload = threadinfo->endload;
	CString username = ((CSploadView*) (threadinfo->parent))->username;
	CString password = ((CSploadView*) (threadinfo->parent))->password;
	int SendTimeoutValue = ((CSploadView*) (threadinfo->parent))->SendTimeoutValue;
	CString filename = threadinfo->filename;
	DWORD nByteLoad = threadinfo->nByteLoad;
	CHttpFile* pFile = NULL;

	if (nByteLoad==endload-startload+1)
	{
		threadinfo->isDone = 1;
		return 0; // completed
	}	
   
	//TRACE("\n%s",LPCTSTR(filename));
	
	
	//DeleteFile(filename);
	FILE* fx = openFile(filename);
	FILE* fy = openFileINI(filename+".ini");

	//TRACE("\nfilename %s",filename);
	if (fx==NULL)
	{
		TRACE("\nUnable to open temporary file. Download aborted");
		((CSploadView*)(threadinfo->parent))->failCondition = 1;
		return 2;

	}

	if (fy==NULL)
	{
		TRACE("Unable to open tracking file. Download aborted");
		((CSploadView*)(threadinfo->parent))->failCondition = 1;
		return 3;

	}

	if (threadinfo->saveFile == NULL)	
	{
		threadinfo->saveFile = fx;
		threadinfo->saveFileInUse = 0;

	}
	
	if (threadinfo->iniFile == NULL)
	{
		threadinfo->iniFile = fx;
		threadinfo->iniFileInUse = 0;

	}


	if (nByteLoad>0)
	{
		seekFile(fx,nByteLoad);

	}

	if (threadinfo->timeDelay > 0)
	{
		Sleep(threadinfo->timeDelay);

	}

	if (threadinfo->aborted)
	{
		return 0;
	}

		
	CInternetSession iss( "IE6", 1,  INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE );

	CHttpConnection* htc = NULL;

	TRY
	{
		if (threadinfo->aborted)
		{
			return 0;

		}
		htc = iss.GetHttpConnection( strServer, nPort,username ,password );

	}
	CATCH( CException, e)
	{
		TRACE("\nException thrown in getting connection in download thread");
		htc = NULL;

	}
	END_CATCH

	if (htc)
	{

		
		if (threadinfo->aborted)
				return 0;
		pFile =  htc->OpenRequest( CHttpConnection::HTTP_VERB_GET, strObject,  NULL, 1, NULL, NULL,  INTERNET_FLAG_RELOAD );
		
		

		if (pFile)
		{		
			threadinfo->pFile = pFile;

			CString headersStr;			
			headersStr.Format("Range: bytes=%d-%d",startload+nByteLoad,endload);			
			DWORD retReq = 0;

			TRY {	
				
				
				pFile->AddRequestHeaders(headersStr);
				
				
				//will throw exception if fails
				if (threadinfo->aborted)
						return 0;

				int retval = SendRequestTimeout(pFile,SendTimeoutValue,threadinfo);
				if (retval==10000)
				{
					retReq=10000;
				}
				else if (retval==100)
				{
					TRACE("\nUser break in SendRequest in thread %d",threadinfo->threadNumber);
					//user break;
					threadinfo->hasError = 100;
					retReq=0; //bypass queryinfostatus checks and exit

				}
				else
				{



					if (threadinfo->aborted)
						return 0;	
									
					
					
					pFile->QueryInfoStatusCode(retReq);

				}

			}			
			CATCH (CException, excep)
			{
				
				retReq=10000; //indicates error
				TRACE("\nException thrown in Sending Requests");
			

			}
			END_CATCH


			
			
			if ((retReq>=200) && (retReq<=299))
			{
				//if (retReq==206)
				//{

					readWriteFileHttp(pFile,fx,fy,startload,endload,threadinfo);

					

				//}		
				
	
			}	
			else if (retReq==416) //Requested range not satisfiable
			{

				threadinfo->hasError = 1;
				threadinfo->errorCode = retReq;


			}
			else
			{
				TRACE("\nError sending request to server : return code %d",retReq);
				threadinfo->hasError = 1;
				threadinfo->errorCode = retReq;


			}
			
			
			pFile->Close();

		}

		htc->Close();


	}

	iss.Close();

	//if (threadinfo)
	{
		//if (((CSploadView *)threadinfo->parent)->thread_restart_running!=0)
		{
			
			if (threadinfo->saveFile)
			{
					fclose(threadinfo->saveFile);
					threadinfo->saveFile = NULL;
			}

			if (threadinfo->iniFile)
			{
					fclose(threadinfo->iniFile);
					threadinfo->iniFile = NULL;
			}

		}
	}
	
	//closeFile(fx);
	//closeFile(fy);
	
	threadinfo->isDone = 1;

	threadinfo->safeToDel = 1;

	return 0;

}




void CSploadView::OnDestroy() 
{
	CView::OnDestroy();

	/*
	if (downloadInProgress)
	{
		stopDownload = 1;
		Sleep(1000);

	}
	*/


	thread_restart_running = 0;	
	restartLoad = 0;
	

	CString moduleDir = GetModuleDirectory();
	CString savePath = moduleDir + "\\savedata.dat";
	FILE* saveFile = fopen(savePath,"wb");
	if (saveFile)
	{
		SaveDownloadArray(saveFile);
		fclose(saveFile);
	}

	//prevent crash in waitdownloadcomplete on exit ?
	exitStatus = 1;
	Sleep(100);
	
	
	for (int i=0;i<8;i++)
	{
	
		if (threadsinfo[i])
		{	

			//can cause threads to crash on exit because the resources are freed prematurely !?
			if (threadsinfo[i]->safeToDel)
			{
				free(threadsinfo[i]);
				threadsinfo[i]=NULL;
			}

		}

	}

	m_shadowPen.DeleteObject();
	m_shadowBrush.DeleteObject();
	m_highlightBrush.DeleteObject();

	lightUpBrush.DeleteObject();
	whiteBrush.DeleteObject();

	

	int delnum = 0;
	delnum = delArray.GetSize();
	for (int j=0;j<delnum;j++)
	{
		THREADDATA* tdata = delArray[j];
		if (tdata)
		{
			if (tdata->safeToDel)
			{
				TRACE("\nDeleted THREADDATA %d",j);
				delete tdata;

			}
		}

	}

	//Sleep(5000);

	
	
}


CString GetModuleDirectory()
{
	
	TCHAR    strPath[800];
	CString retPath;
	
	GetModuleFileName( NULL, strPath, 800 );
	
	CString modPath(strPath);	
	int fval = modPath.ReverseFind('\\');	
	retPath=modPath.Left(fval);
	
	return retPath;	

}

void CSploadView::OnButtonpause() 
{
	stopDownload = 1;
	
}

int setMainINICompleted(CString filename)
{

	FILE* fmain = fopen(filename,"r+b");
	if (fmain)
	{
		
		fseek(fmain,12,SEEK_SET); //completion flag 

		DWORD val = 1;
		fwrite(&val,1,4,fmain);

		fclose(fmain);

	}
	else
	{
		TRACE("\nError setting completion info");
		return 1;

	}

	return 0;

}

int createMainINI(CString filename,CString urlStr,THREADDATA* threadsinfo[],DWORD doclen,DWORD iscompleted)
{

	CString filenameMain = filename + ".main.ini";

	FILE* fmain = fopen(filenameMain,"wb");
	if (fmain)
	{

		DWORD reserved = 0;
		DWORD reserved1 = 0;
		DWORD reserved2 = 0;
		
		DWORD val = 0;
		val = urlStr.GetLength();
		fwrite(&val,1,4,fmain);
		val = filename.GetLength();
		fwrite(&val,1,4,fmain);
		val = doclen;
		fwrite(&val,1,4,fmain);
		val = iscompleted;
		fwrite(&val,1,4,fmain);

		fwrite(&reserved1,1,4,fmain);
		fwrite(&reserved2,1,4,fmain);

		for (int i=0;i<50;i++)
			fwrite(&reserved,1,4,fmain);
		
		
		for (i=0;i<8;i++)
		{
			val = strlen(threadsinfo[i]->filename);
			fwrite(&val,1,4,fmain);
			//TRACE("len = %d",val);

		}
		
		fwrite(urlStr,1,urlStr.GetLength(),fmain);
		fwrite(filename,1,filename.GetLength(),fmain);		
		for (i=0;i<8;i++)
		{
			fwrite(threadsinfo[i]->filename,1,strlen(threadsinfo[i]->filename),fmain);

		}

		fclose(fmain);

	}
	else
	{
		TRACE("\nError creating file info");
		return 1;

	}

	return 0;

}


//returns FULL,PARTIAL or NO_ERROR
//return 1 to indicate resume
int verifyResume(CString filename, CString urlStr, THREADDATA* threadsinfo[],DWORD doclen,DWORD partsStatus[])
{

	CString filenameMain = filename + ".main.ini";

	FILE* fmain = fopen(filenameMain,"rb");
	if (fmain)
	{
		BYTE buffer[512];
		DWORD partsLen[8];

		DWORD reserved;
		DWORD reserved1 = 0;
		DWORD reserved2 = 0;
		DWORD urlLen;
		DWORD fileLen;
		DWORD documentLen;
		DWORD iscompleted = 0;
				
		DWORD val = 0;		
		fread(&val,1,4,fmain);
		urlLen = val;
				
		fread(&val,1,4,fmain);
		fileLen = val;

		fread(&val,1,4,fmain);
		documentLen = val;	

		fread(&val,1,4,fmain);
		iscompleted = val;			

		
		
		fread(&reserved1,1,4,fmain);
		fread(&reserved2,1,4,fmain);

		for (int i=0;i<50;i++)
			fread(&reserved,1,4,fmain);

		if (documentLen!=doclen)
		{
			TRACE("\nFile size does not match");
			fclose(fmain);
			return RET_FULL_ERROR;

		}
		else
			TRACE("\nFile size match!");


			
		if ((reserved1!=0) || (reserved2!=0))
		{

			TRACE("\nInvalid Resumption File");
			fclose(fmain);
			return RET_FULL_ERROR;

		}

		if ((urlLen > 512) || (fileLen>512) || (urlLen<=0) || (fileLen<=0))
		{

			TRACE("\nInvalid URL or File Length");
			fclose(fmain);
			return RET_FULL_ERROR;

		}		


		for (i=0;i<8;i++)
		{
			
			fread(&val,1,4,fmain);
			if ((val > 512) || (val<=0))
			{
				TRACE("\nInvalid Partial File Length (%d)",i);
				fclose(fmain);
				return RET_FULL_ERROR;


			}
			partsLen[i]=val;
			

		}

		CString urlRead,fileRead;
		fread(buffer,1,urlLen,fmain);
		if (urlLen<512)
			buffer[urlLen]=0;
		urlRead = buffer;

		if (urlRead!=urlStr)
		{
			
			TRACE("\nURL does not match. Creating a new download session.");
			fclose(fmain);
			return RET_FULL_ERROR;
		}		

		fread(buffer,1,fileLen,fmain);		
		fileRead = buffer;

	
		if (fileLen<512)
			buffer[fileLen]=0;
		if (fileRead!=urlStr)
		{
			//TRACE("\nFilename does not match.");
			
		}

		if (iscompleted!=0)
		{
			DWORD xlen = GetFileLength(filename);

			TRACE("\nxlen %d",xlen);

			if (xlen != doclen)
			{
				TRACE("\nCompleted file size does not match!");				
				//ignore this flag .. and continue

			}		
			else
			{
			
				TRACE("\nFile Download Already Completed !");
				fclose(fmain);
				return RET_DO_NOTHING;
			}

		}
		
		
		
		for (i=0;i<8;i++)
		{

			partsStatus[i]=RET_NO_ERROR;

			CString partsStr;
			fread(buffer,1,partsLen[i],fmain);
			if (partsLen[i]<512)
				buffer[partsLen[i]]=0;
			partsStr = buffer;

	
			//verify part
			DWORD sl,el,vp ;
			partsStatus[i] = verifyPart(partsStr,sl,el,vp);	 //no error or completed ==> no error, partial error => error
			if ((partsStatus[i]==RET_NO_ERROR) || (partsStatus[i]==RET_COMPLETED))
			{
				if ((sl!=threadsinfo[i]->startload) ||
					(el != threadsinfo[i]->endload))
				{

					TRACE("\nStartLoad and Endload does not match");					
					fclose(fmain);
					return RET_FULL_ERROR;
					
				}

				threadsinfo[i]->nByteLoad = vp;				
				strcpy(threadsinfo[i]->filename,(char *) buffer);

			}			


		}

		for (i=0;i<8;i++)
		{

			if (partsStatus[i]==RET_PARTIAL_ERROR)
			{

				TRACE("\nUnable to verify Part %d.",i);
				fclose(fmain);
				return RET_PARTIAL_ERROR;

			}

		}

		int completed  = 1;
		for (i=0;i<8;i++)
		{
			if (partsStatus[i]!=RET_COMPLETED)
			{
				completed  = 0;

			}

		}

		fclose(fmain);

		//verification success
		if (completed)
			return RET_COMPLETED;
		else
			return RET_NO_ERROR;

	}

	return RET_FULL_ERROR;

}


DWORD verifyPart(CString filename, DWORD &r_startload,DWORD &r_endload, DWORD &ret_len)
{

	CString xname = filename+".ini";
	FILE* fv = fopen(xname,"rb");
	if (fv)
	{
		DWORD startload,endload,nBytesLoad;

		char updateStr[100];
		fread(updateStr,1,63,fv);
		sscanf(updateStr,"%20d,%20d,%20d\n",&startload,&endload,&nBytesLoad);

		DWORD xlen = GetFileLength(filename);

		if (xlen < nBytesLoad)
		{

			TRACE("\nPart length not enough");
			fclose(fv);
			return RET_PARTIAL_ERROR;

		}		
		else
		{
			if (xlen>=endload-startload+1)
				TRACE("\nPart length OK (Completed) %d %d %d %d",xlen,nBytesLoad,startload,endload);
			else
				TRACE("\nPart length OK %d %d %d %d",xlen,nBytesLoad,startload,endload);


		}

		r_startload = startload;
		r_endload = endload;
		ret_len = xlen;

		fclose(fv);

		if (xlen>=endload-startload+1)
		{
			return RET_COMPLETED;
			
		}		

		return NO_ERROR;

	}
	else
	{

		TRACE("\nPart INI file does not exist! %s",xname);
		return RET_PARTIAL_ERROR;

	}

	return RET_PARTIAL_ERROR;

}


DWORD GetFileLength(CString filename)
{
	DWORD xlen = 0; 

	CFile  xfile;
	BOOL ret = xfile.Open(filename,CFile::shareDenyNone |CFile::modeRead,NULL);
	if (ret)
		xlen = (DWORD) xfile.GetLength();

	return  xlen;


}

//filename refers to the main name
int mainVerifyCreate(CString filename,CString urlStr,THREADDATA* threadsinfo[],DWORD doclen,DWORD partsStatus[])
{

	
	int isResume = RET_NO_ERROR;
	isResume = verifyResume(filename,  urlStr, threadsinfo,doclen,partsStatus);

	//tracing
	TRACE("\n isResume = %d",isResume);
	if ((isResume != RET_FULL_ERROR) && (isResume != RET_DO_NOTHING))
	{
		for (int j=0;j<8;j++)
		{
			TRACE("\n partsStatus = %d",partsStatus[j]);

		}

	}
	//else , the values may be undefined and result in negative numbers


	if (isResume==RET_FULL_ERROR)
	{

		TRACE("\n FULL ERROR");
		CString filenameMain = filename + ".main.ini";
		DeleteFile(filenameMain);

		createMainINI(filename,urlStr,threadsinfo,doclen,0);
		for (int i=0;i<8;i++)
		{
			DeleteFile(threadsinfo[i]->filename);

			CString fstr;
			fstr = threadsinfo[i]->filename;
			DeleteFile(fstr  + ".ini");

			threadsinfo[i]->nByteLoad = 0;

		}

	}
	else if (isResume==RET_PARTIAL_ERROR)
	{
		TRACE("\n PARTIAL ERROR");
		for (int i=0;i<8;i++)		
		{
			if (partsStatus[i]==RET_PARTIAL_ERROR)
			{

				DeleteFile(threadsinfo[i]->filename);

				CString fstr;
				fstr = threadsinfo[i]->filename;
				DeleteFile(fstr  + ".ini");

				threadsinfo[i]->nByteLoad = 0;


			}

		}

	}
	else if (isResume==RET_NO_ERROR)
	{
		//Do nothing
		TRACE("\n NO ERROR -- Resume");


	}
	else if (isResume==RET_COMPLETED)
	{

		//Do nothing
		TRACE("\n NO ERROR -- Completed");


	}

	return isResume;

}


void CSploadView::OnUpdateButtondownload(CCmdUI* pCmdUI) 
{	
	pCmdUI->Enable(!downloadInProgress);
}

void CSploadView::RestartDownload()
{

	if (downloadInProgress)
	{	
		TRACE("\nReturn due to restart download");
		//downloadInProgress = 0;

		return;
	}

	downloadInProgress = 1 ;
	int retStatus = 1;	    
		
		

		int num = downloadArray.GetSize();
		if (num<=0)
		{
			downloadInProgress = 0;
			Invalidate();
			return;
		}

		currentActive = -1;		
		int startdownload = -1;
		for (int i=0;i<num;i++)
		{
			if ((statusArray[i]==ST_PENDING) || (statusArray[i]==ST_PARTIAL))
			{
				startdownload = i;
				break;

			}

		}		

		if (startdownload<0)
		{
			//Download error items only after all pending/partial items are completed
			for (int i=0;i<num;i++)
			{
				if (statusArray[i]==ST_ERROR)
				{
					startdownload = i;
					break;

				}
			}
		
			

		}

		if (startdownload<0)
		{
			downloadInProgress = 0;
			Invalidate();
			return;

		}

		
		currentActive = startdownload;
		upDateCount = 0;
		index100 = 120;
		//TRACE("\nCurrent Active (2) :%d",currentActive);
		

		CString mydir;

		urlString = downloadArray[startdownload];
		myUrlString = urlString;
		mydir = mainFileArray[startdownload];
		mySaveDir = mydir;
			


		TRACE("\nStart Downloading %s",urlString);


		//Readies URL and saveFile data	
		


		//Perform a Simple Check
		DWORD dwServiceType;
	    CString strServer, strObject;
	    INTERNET_PORT nPort;
	    int retVal = AfxParseURL(urlString, dwServiceType, strServer, strObject,nPort);

		progressStatus = 1;
		int old_status = ST_PENDING;
		old_status = statusArray[currentActive];

		statusArray[currentActive] = ST_DOWNLOADINPROGRESS;
		Invalidate();

		forceAbortion = 0;


		//Special Cases		
		specialcase = 0;
		if (retVal!=0) 				
		{
			CString compareStr;
			compareStr = strServer;
			compareStr.MakeLower();
		
		} 
		TRACE("\nSpecial Case : %d",specialcase);
		

		//If not using HTTP, proceed to single thread download with no resume
		if ((specialcase == 1) || ((retVal!=0) && (strServer!="") && (dwServiceType!=AFX_INET_SERVICE_HTTP)))
		{
			TRACE("\nProceed to single download directly");	
			retStatus=2;		
			
						
			int testconnect = TestConnectionTimeout(30000);			
			if (!testconnect)
			{
				TRACE("\nFailure connecting to URL (1)");
				statusArray[currentActive] = ST_ERROR;
				downloadInProgress = 0;
				Invalidate();
				return ;		

			}

		}
		else		
		{	
			
			retStatus = PrepareDataTimeout(30000);
			TRACE("\nPrepare Data returns %d",retStatus);

		}		

		if (!retStatus)
		{		
			downloadType = MULTIPLE_THREAD;
			restartCount = 10;
			mySaveDir = mydir;
			CWinThread* pMonitorThread = AfxBeginThread(WaitDownloadComplete,this);		

		}
		else
		{

			if (retStatus==1)
			{
				TRACE("\nAlready Completed");

				statusArray[currentActive] = ST_COMPLETED;
				loadedBytesArray[currentActive] = totalBytesArray[currentActive];
				downloadInProgress = 0;
				Invalidate();


			}
			else if (retStatus==2)
			{
				TRACE("\nDoes not support multiple");						
				
				//launch single thread				
				retStatus = PrepareDataSingle(urlString,mydir);
				
				if (!retStatus)
				{	
					TRACE("\nStarting single thread download");				
					downloadType = SINGLE_THREAD;
					restartCount = 1;
					mySaveDir = mydir;
					downloadInProgress = 1;
					CWinThread* pMonitorThread = AfxBeginThread(WaitDownloadComplete,this);		
					

				}				

			}
			else if (retStatus==3)
			{
	
				statusArray[currentActive] = old_status;
				downloadInProgress = 0;


			}

		}

	//}
}


UINT DetectDownloadThread(LPVOID pParam)
{

	if (pParam==NULL)
	{
		TRACE("\nError launching restart thread");
		return 1;

	}

	CSploadView* parent = (CSploadView*) pParam;		

	while (parent->thread_restart_running)
	{

		if (parent->restartLoad)
		{

			parent->RestartDownload();
			parent->restartLoad = 0;

		}
		else
		{
			//Auto restart download
			if ((!downloadInProgress) && (!userStopPressed))
			{

				int num = downloadArray.GetSize();
				if (num>100) 
					num = 100;
				
				if (num>0)
				{

					int allatrest = 1;
					for (int i=0;i<num;i++)
					{
						if (statusArray[i]==ST_DOWNLOADINPROGRESS)
						{
							allatrest = 0;
							break;
						}

					}		

					//TRACE("\nallrest %d",allatrest);						

					if (allatrest)
					{

							int startdownload = -1;
							for (i=0;i<num;i++)
							{
								if ((statusArray[i]==ST_PENDING) || (statusArray[i]==ST_PARTIAL))
								{
									startdownload = i;
									break;
								}

							}		


							if (startdownload<0)
							{
								//Do nothing	
							

							}
							else
							{
								
								if (parent->connectedState)
								{				
									parent->RestartDownload();
									parent->restartLoad = 0;

								}

							}


					}
				}
			}



		}

		Sleep(100);

	
	}

	parent->thread_restart_ended = 1;

	return 0;

}


int CSploadView::PrepareDataSingle(CString urlStr,CString savePath)
{

	   failCondition = 0;	
	   stopDownload = 0;
	   downloadType = SINGLE_THREAD;
	   

	   int retStatus =0 ;	

	   DWORD dwServiceType;
	   CString strServer;
	   CString strObject;
	   INTERNET_PORT nPort;
	   int retVal = AfxParseURL(urlStr, dwServiceType, strServer, strObject,nPort);

	   if ((retVal==0) || (strServer==""))
	   {
		   TRACE("\nUnable to parse URL");
		   return 1;

	   }

	
		if (threadsinfo[0]==NULL)
			threadsinfo[0]=new THREADDATA;
		else if ((threadsinfo[0]->aborted) || (threadsinfo[0]->saveFile!=NULL))  
		{
			threadsinfo[0]=new THREADDATA;
		}
		threadsinfo[0]->parent=this;
		threadsinfo[0]->startload=0;
		threadsinfo[0]->endload=0;
		threadsinfo[0]->pFile = NULL;
		threadsinfo[0]->dwServiceType = dwServiceType;							
		copyStr(threadsinfo[0]->strServer,strServer);
		copyStr(threadsinfo[0]->strObject,strObject);
		threadsinfo[0]->nPort = nPort;
		threadsinfo[0]->isDone = 0;
		threadsinfo[0]->nByteLoad = 0;							
		threadsinfo[0]->hasError = 0;
		threadsinfo[0]->errorCode = 0;
		threadsinfo[0]->threadNumber = 0;
		threadsinfo[0]->timeDelay = 0;				
		copyStr(threadsinfo[0]->filename,savePath);	

		threadsinfo[0]->saveFile = NULL;
		threadsinfo[0]->iniFile = NULL;
		threadsinfo[0]->saveFileInUse = 0;
		threadsinfo[0]->iniFileInUse = 0;
		threadsinfo[0]->aborted = 0;
		threadsinfo[0]->safeToDel = 0;

		//setting the status array
		statusArray[currentActive] = ST_DOWNLOADINPROGRESS;
		resumableArray[currentActive] = 0;
		progressStatus = 0;
				
			
		StartDownloadThreadSingle(threadsinfo[0]);


	return 0; 

}



UINT DownloadSingleThread(LPVOID pParam)
{
	TRACE("\nSingle Thread Download Started");

	THREADDATA* threadinfo = (THREADDATA*) pParam;

	DWORD dwServiceType = threadinfo->dwServiceType;
	CString strServer = threadinfo->strServer;
	CString strObject = threadinfo->strObject;
	INTERNET_PORT nPort = threadinfo->nPort;
	DWORD startload = threadinfo->startload;
	DWORD endload = threadinfo->endload;
	CString username = ((CSploadView*) (threadinfo->parent))->username;
	CString password = ((CSploadView*) (threadinfo->parent))->password;
	CString filename = threadinfo->filename;
	DWORD nByteLoad = threadinfo->nByteLoad;
	

	if (nByteLoad==endload-startload+1)
	{
		threadinfo->isDone = 1;
		return 0; // completed
	}	
   
	FILE* fx = openFile(filename);
	
	if (fx==NULL)
	{
		TRACE("\nUnable to open temporary file. Download aborted (Single Thread)");
		((CSploadView*)(threadinfo->parent))->failCondition = 1;
		return 2;

	}

	if (threadinfo->saveFile == NULL)	
	{
		threadinfo->saveFile = fx;
		threadinfo->saveFileInUse = 0;
		threadinfo->iniFileInUse = 0;

	}
	
	
	if (threadinfo->timeDelay > 0)
	{
		Sleep(threadinfo->timeDelay);

	}

		
	CInternetSession iss( "IE6", 1,  INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE );
	CStdioFile* pFile = NULL;

	TRY
	{		
		 //this line can cause runtime crash!
		 pFile = iss.OpenURL(((CSploadView*) (threadinfo->parent))->myUrlString,1, INTERNET_FLAG_TRANSFER_BINARY,0,0);

	}
	CATCH( CException, e)
	{
		TRACE("\nException thrown in opening URL");
		pFile= NULL;

		((CSploadView*)(threadinfo->parent))->failCondition = 1;

		
	}
	END_CATCH

	
	if (pFile)
	{	
		
		//threadinfo->pFile = pFile; //no abort available for CStdioFile
		
		readWriteFile(pFile,fx,NULL,startload,endload,threadinfo);

		if (threadinfo->saveFile)	
		{		
			closeFile(fx);	
			threadinfo->saveFile = NULL;

		}
		threadinfo->isDone = 1;

	}
	

	return 0;

}





int TestConnection(CString myUrlString)
{
	TRACE("\nTesting Connection");
	
	HINTERNET hInternet = InternetOpen("IE6",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);

	if (hInternet)
	{		
		HINTERNET hFile = InternetOpenUrl(hInternet,myUrlString,_T(""),0,INTERNET_FLAG_RELOAD,0);
											

		if (hFile)
		{
			
			TRACE("\nPass");			
			InternetCloseHandle(hFile);

		}
		else		
		{
			InternetCloseHandle(hInternet);
			TRACE("\nNot Pass");
			return 0;
			

		}
				
		InternetCloseHandle(hInternet);

	}

	return 1;

}


int SendRequestTimeout(CHttpFile* pFile, int timeout, THREADDATA* threadinfo)
{
	   HANDLE   hThread; 
       DWORD    dwThreadID;

	   if (threadinfo==NULL)
		   return 100;

	   CSploadView*  parent = (CSploadView* ) threadinfo->parent;

       hThread = CreateThread(
                     NULL,            // Pointer to thread security attributes 
                     0,               // Initial thread stack size, in bytes 
                     WorkerSendRequest,  // Pointer to thread function 
                     pFile,     // The argument for the new thread
                     0,               // Creation flags 
                     &dwThreadID      // Pointer to returned thread identifier 
                 );    


	   DWORD statusWait = WAIT_TIMEOUT;
	   int timewait =500;
	   int haswaited = 0;
	   while ((statusWait == WAIT_TIMEOUT) && (haswaited<timeout))
	   {

		   if (parent->stopDownload)
			   return 100; //indicates user break

		    statusWait = WaitForSingleObject ( hThread, timewait );
			haswaited += timewait;
	   }   

	   if (haswaited>=timeout)
	   {
		   TRACE("\nTimeout in sending requests for thread %d",threadinfo->threadNumber);
           return 10000;

	   }
	   

	   DWORD dwExitCode = 10000;
       if ( !GetExitCodeThread( hThread, &dwExitCode ) )
	   {
		   TRACE("\nError in sending requests");
		   return 10000;
	   }


	   TRACE("\nSend Request Completed in thread %d, exitcode : %d ",threadinfo->threadNumber,dwExitCode);

	   if (dwExitCode==0)	   
			return 0;
	   else
			return 10000;
}



//build thread wrappers around send and read
DWORD WINAPI WorkerSendRequest(IN LPVOID vThreadParm)
{
	if (vThreadParm==NULL)
		return 1;

	CHttpFile* pFile = (CHttpFile*) vThreadParm;
	

	TRY
	{	
		pFile->SendRequest();

	}
	CATCH (CException, excep)
	{

		return 10000;

	}
	END_CATCH

	return 0;

}






int CSploadView::ForceExit()
{
	

	return 0;
}



void CSploadView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	if (!AfxOleInit())
	  {
		::MessageBox(NULL,"AfxOleInit Error!","Error",MB_OK | MB_ICONEXCLAMATION);
		exit(1);
	  }


	if (!m_dropTarget.Register(this))
	{
		::MessageBox(NULL,"Cannot register drop target","Error",MB_OK | MB_ICONEXCLAMATION);	
		exit(1);

	}

	connectedState =  CheckConnection();

	m_shadowPen.CreatePen(PS_SOLID, 2, RGB(64,64,64));
	m_shadowBrush.CreateSolidBrush(RGB(200,200,200));
	m_highlightBrush.CreateSolidBrush(RGB(255,180,180));

	lightUpBrush.CreateSolidBrush(RGB(255,180,180));
	whiteBrush.CreateSolidBrush(RGB(255,255,255));


	CString moduleDir = GetModuleDirectory();
	CString savePath = moduleDir + "\\savedata.dat";
	FILE* saveFile = fopen(savePath,"rb");
	if (saveFile)
	{
		LoadDownloadArray(saveFile);
		fclose(saveFile);
		
		IsScrollable =	ComputeScrollRanges();

		int num=downloadArray.GetSize();	
		if (num>0)
		{			//connectedState =  CheckConnection();
			if (connectedState)
				restartLoad = 1;

		}
	}


	mainView = this;
	TRACE("\nMainview %d",mainView);

	//hgBkBitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgBkBitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BITMAP5),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgUpBox1Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_UPBOX1),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgUpBox2Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_UPBOX2),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgDownBox1Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_DOWNBOX1),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgDownBox2Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_DOWNBOX2),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgDelBox1Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_DELBOX1),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgDelBox2Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_DELBOX2),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgSaveBox1Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_SAVEBOX3),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgSaveBox2Bitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_SAVEBOX4),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	//hgOpensourceBitmap =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_OSBITMAP),IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
	hgHandIcon =  LoadImage(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_HANDICON),IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);

	(AfxGetMainWnd())->PostMessage(WM_USER_SETTRAYNOTIFY,0,0);

	

	LOGFONT logfont;
	ZeroMemory(&logfont, sizeof(LOGFONT));
	strcpy(logfont.lfFaceName,"Arial");		
	logfont.lfHeight = 15;	
	logfont.lfWeight = FW_NORMAL;
	logfont.lfWidth = 0;		
	
	font.CreateFontIndirect(&logfont);	

	if (AddMode)
	{
		TRACE("\nAddModeString : %s",AddModeItem);
		OnAddItem(AddModeItem);
		AddMode = 0;
	}

	

	m_tooltip.Create(this);
	m_tooltip.SetSize(CPPToolTip::PPTTSZ_MARGIN_CX, 4);
	m_tooltip.SetSize(CPPToolTip::PPTTSZ_MARGIN_CY, 4);
	//m_tooltip.SetDirection(	PPTOOLTIP_TOPEDGE_RIGHT);
	//m_tooltip.SetColorBk(RGB(204,0,0), RGB(255,255,255), RGB(217,217,211));
	//m_tooltip.SetEffectBk(CPPDrawManager::EFFECT_VBUMP  ,  5) ;
	m_tooltipCreated = 1;

	firstUse = 1;
	int num=downloadArray.GetSize();	
	if (num>0)
		firstUse = 0;
	
}

DROPEFFECT CSploadView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	
	BOOL bRet = pDataObject->IsDataAvailable( CF_TEXT, NULL );
	if (bRet)	
		return DROPEFFECT_COPY;
	else
		return DROPEFFECT_NONE;
	return CView::OnDragOver(pDataObject, dwKeyState, point);

	
}

BOOL CSploadView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	
	//AfxGetMainWnd()->SetActiveWindow();
	SetForegroundWindow();
	BOOL bRet = pDataObject->IsDataAvailable( CF_TEXT, NULL );
	HGLOBAL hGlobal=pDataObject->GetGlobalData(CF_TEXT);
	if (hGlobal)
	{
		LPCTSTR pData=(LPCTSTR)GlobalLock(hGlobal);


		TRACE("\n Text :%s",pData);
				
		int ret = AddDownloadArrayItem(pData);
		IsScrollable = ComputeScrollRanges();  //this function should always follow AddDownloadArrayItem and RemoveItem

		GlobalUnlock(hGlobal);
		GlobalFree(hGlobal);
		
		Invalidate();
		
		if (ret==0)
		{
			connectedState =  CheckConnection();
			if ((connectedState) && (!userStopPressed))
				restartLoad = 1;
		}
		
		
		return DROPEFFECT_COPY;

	}
	
	
	return FALSE;	
	//return CView::OnDrop(pDataObject, dropEffect, point);
}

BOOL CSploadView::OnAddItem(CString pData)
{

	int ret = AddDownloadArrayItem(pData);
	IsScrollable = ComputeScrollRanges();  //this function should always follow AddDownloadArrayItem and RemoveItem
	Invalidate();

	if (ret==0)
	{
		connectedState =  CheckConnection();
		if ((connectedState) && (!userStopPressed))
			restartLoad = 1;
		return TRUE;

	}
	else
		return FALSE;


}


void DrawGradient (CRect rectangle, COLORREF Color1, COLORREF Color2, CDC* pDC)
{    
	float division = 128;

	float red1 = GetRValue(Color1);
    float green1 = GetGValue(Color1);
    float blue1 = GetBValue(Color1);
    
    float red2 = GetRValue(Color2);
    float green2 = GetGValue(Color2);
    float blue2 = GetBValue(Color2);
    
    float deltaR = (red2 - red1) / division;
    float deltaG = (green2 - green1) / division;
    float deltaB = (blue2 - blue1) / division;

	float Red = red1;
	float Green = green1;
	float Blue = blue1;
	
	float xx = (float) rectangle.left;
	float yy = (float) rectangle.top;
	float ww = ((float) rectangle.Width())/division;
	float hh = ((float) rectangle.Height())/division;

	int n=0;
	CRect fillrect;	
	while (n < division)
	{		
		CBrush brush (RGB(Red, Green, Blue));        		
        
		fillrect.left = rectangle.left;
		fillrect.right = rectangle.right;
		fillrect.top = (int) yy;            
        fillrect.bottom = (int) (yy + hh);  
        pDC->FillRect(fillrect, &brush);
 
	    Red += deltaR;
    	Green += deltaG;
    	Blue += deltaB;

		yy = yy + hh;
		n++;
	}    
}



void DrawGradientVert (CRect rectangle, COLORREF Color1, COLORREF Color2, CDC* pDC)
{    
	float division = 128;

	float red1 = GetRValue(Color1);
    float green1 = GetGValue(Color1);
    float blue1 = GetBValue(Color1);
    
    float red2 = GetRValue(Color2);
    float green2 = GetGValue(Color2);
    float blue2 = GetBValue(Color2);
    
    float deltaR = (red2 - red1) / division;
    float deltaG = (green2 - green1) / division;
    float deltaB = (blue2 - blue1) / division;

	float Red = red1;
	float Green = green1;
	float Blue = blue1;
	
	float xx = (float) rectangle.left;
	float yy = (float) rectangle.top;
	float ww = ((float) rectangle.Width())/division;
	float hh = ((float) rectangle.Height())/division;

	int n=0;
	CRect fillrect;	
	while (n < division)
	{		
		CBrush brush (RGB(Red, Green, Blue));        		
        
		fillrect.left = (long) xx;
		fillrect.right = (long) (xx+ww);
		fillrect.top = rectangle.top;            
        fillrect.bottom = rectangle.bottom;  
        pDC->FillRect(fillrect, &brush);
 
	    Red += deltaR;
    	Green += deltaG;
    	Blue += deltaB;

		xx = xx + ww;

		n++;
	}
    
}


void CSploadView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//Save Old State

	

	int oldLightUpArrow = lightUpArrow;
	int oldcolorgray = colorgray;
	int oldcolorredR = colorredR;
	int oldcolorredG = colorredG;
	int oldcolorredB = colorredB;
	


	POINT pt;
	GetCursorPos(&pt);	
	ScreenToClient(&pt);
	
	CRect clientrect;
	GetClientRect(&clientrect);
	clientrect.DeflateRect(15,15,15,15);
	
	if (connectedState)
	{
		if (userStopPressed)
		{
			//statusText = _T("Download Stopped");
			statusText.LoadString(IDS_DOWNLOADSTOPPED);
		}
		else
		{
			//statusText = _T("Ready");
			statusText.LoadString(IDS_READY);
		}

	}
	else
	{
		statusText.LoadString(IDS_CDINC);
		//statusText = _T("Cannot detect internet connection");

	}


	

	if (draggingEnabled) 
	{		

		ScrollBarPos = point.y - dragPointOffset;
		//TRACE("\nDragging Scroll Bar %d %d %d",point.y,dragPointOffset,ScrollBarPos);
		//ScrollBarPos = point.y + dragPointOffset - listrect.top - 68;
		if (ScrollBarPos < 0)
			ScrollBarPos = 0;

		if (ScrollBarPos > scrollbarRange)
			ScrollBarPos = scrollbarRange;

		
		if ((dstRange.cy > 0) && (scrollbarRange > 0))
		{			

			//TRACE("\nInvalidate");

			ScrollPosDst = (ScrollBarPos * dstRange.cy) / scrollbarRange;

			CRect inRect = 	m_ScrollRect;
			inRect.right += 18;
			InvalidateRect(inRect);

			CView::OnMouseMove(nFlags, point);

			return;

		}

	}
	
	if ((pt.x > 25)  &&
		(pt.x < 100)  &&
		(pt.y > 25)  &&
		(pt.y < 55))
	{	
		colorgray =210;

		//statusText = _T("Start/Resume Download");
		statusText.LoadString(IDS_STARTRESUME);
		SetCursor((HICON) hgHandIcon);
		
		
	}
	else
		colorgray =180;



	if ((pt.x > 110)  &&
		(pt.x < 185)  &&
		(pt.y > 25)  &&
		(pt.y < 55))
	{	
		colorredR =212;
		colorredG =222;
		colorredB =217;
		SetCursor((HICON) hgHandIcon);
	
		//statusText = _T("Stop Download");
		statusText.LoadString(IDS_STOPDOWNLOAD);
		
	}
	else
	{

		
		colorredR =174;
		colorredG =192;
		colorredB =183;
		

	}


	if ((pt.x > clientrect.right - 18)  &&
		(pt.x < clientrect.right )  &&
		(pt.y > clientrect.bottom - 36)  &&
		(pt.y < clientrect.bottom - 18))
	{		


		
		lightUpArrow = 3;
		//statusText = _T("Click to Scroll");
		statusText.LoadString(IDS_CLICKTOSCROLL);

	}
	else if ((pt.x > clientrect.right - 18)  &&
		(pt.x < clientrect.right )  &&
		(pt.y > clientrect.top + 50)  &&
		(pt.y < clientrect.top + 50 + 18))
	{		

		
		lightUpArrow = 4;
		//statusText = _T("Click to Scroll");
		statusText.LoadString(IDS_CLICKTOSCROLL);

	}
	else		
		lightUpArrow = 0;

	int numItems = 0;
	CRect ScrollRect = PrepareScrollRect();
	CheckClickableRects(ScrollRect, 0, ScrollPosDst, -1,numItems);
	doInvalidateLightUp = MarkClickableRects(ScrollRect, 0, ScrollPosDst, -1,numItems,pt);


	int itemHit = CheckContext(ScrollRect, 0, ScrollPosDst, pt);
	if ((itemHit>=0) && (itemHit<=99))
	{
		old_mouseMoveHit = mouseMoveHit;
		mouseMoveHit = itemHit;

		if (statusArray[itemHit] == ST_COMPLETED)
		{
			//statusText = _T("Drag to destination folder to copy file");
			statusText.LoadString(IDS_DRAGDEST);

		}
		
	}
	else
	{	
		old_mouseMoveHit = mouseMoveHit;
		mouseMoveHit = -1;

	}

	if (old_mouseMoveHit != mouseMoveHit)
	{
		doInvalidateLightUp = 1;
	}


	
	
	//if there is a chnage in state...invalidate
	if ((oldLightUpArrow != lightUpArrow) ||
		(oldcolorgray != colorgray) ||
		(oldcolorredR != colorredR) ||
		(oldcolorredG != colorredG) ||
		(oldcolorredB != colorredB) ||
		(doInvalidateLightUp))
	{

		Invalidate();

	}
	
	CView::OnMouseMove(nFlags, point);
}

int CSploadView::MarkClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems, CPoint pt)
{

	int doInvalidateLightUp = 0;
	
	for (int i=0;i<numItems;i++)
	{
		int old_gLightUpDelBox = gLightUpDelBox[i];
		int old_gLightUpUpBox = gLightUpUpBox[i];
		int old_gLightUpDownBox = gLightUpDownBox[i];
		int old_gLightUpSaveBox = gLightUpSaveBox[i];			
	
		if (gDelBox[i].PtInRect(pt))
		{			
			gLightUpDelBox[i] = 1;
			//statusText = _T("Delete Download");

			if (!downloadInProgress)
			{
				statusText.LoadString(IDS_DELETEDOWNLOAD);
				SetCursor((HICON) hgHandIcon);
			}


		}
		else
			gLightUpDelBox[i] = 0;

		if (gDownBox[i].PtInRect(pt))
		{			
			gLightUpDownBox[i] = 1;
			//statusText = _T("Move Down");
			if (!downloadInProgress)
			{
				statusText.LoadString(IDS_MOVEDOWN);
				SetCursor((HICON) hgHandIcon);
			}
			
		}
		else
			gLightUpDownBox[i] = 0;

	
		if (gUpBox[i].PtInRect(pt))
		{			
			gLightUpUpBox[i] = 1;
			//statusText = _T("Move Up");
			if (!downloadInProgress)
			{
				statusText.LoadString(IDS_MOVEUP);
				SetCursor((HICON) hgHandIcon);
			}
			
		}
		else
			gLightUpUpBox[i] = 0;

		if (gSaveBox[i].PtInRect(pt))
		{			
			gLightUpSaveBox[i] = 1;
			//statusText = _T("Save Download As");
			
			if (statusArray[i]==ST_COMPLETED)
			{
				statusText.LoadString(IDS_SAVEDOWNLOADAS);
				SetCursor((HICON) hgHandIcon);
			}
			
		}
		else
			gLightUpSaveBox[i] = 0;		
		
		if (old_gLightUpDelBox != gLightUpDelBox[i])
		{
				doInvalidateLightUp = 1;
		}

		if (old_gLightUpDownBox != gLightUpDownBox[i])
		{
				doInvalidateLightUp = 1;
		}

		if (old_gLightUpUpBox != gLightUpUpBox[i])
		{
				doInvalidateLightUp = 1;
		}

		if (old_gLightUpSaveBox != gLightUpSaveBox[i])
		{
				doInvalidateLightUp = 1;
		}
		

	}

	return doInvalidateLightUp;

}


int CSploadView::DetectClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems, CPoint pt)
{
	int updateClient = 0;
	
	for (int i=0;i<numItems;i++)
	{	
		if (gDelBox[i].PtInRect(pt))
		{			
			if (statusArray[i]==ST_DOWNLOADINPROGRESS)
			{
				return 0;
			}

			CString delmsg;
			CString delconfirm;
			delmsg.LoadString(IDS_DELMSG);
			delconfirm.LoadString(IDS_DELCONFIRM);
			int ret = MessageBox(delmsg,delconfirm,MB_YESNO | MB_ICONQUESTION);
			if (ret == IDYES)
			{

				mainFileArray[i].TrimLeft();
				mainFileArray[i].TrimRight();
				if (mainFileArray[i]!=_T(""))
				{

					

					DeleteFile(mainFileArray[i]);
					DeleteFile(mainFileArray[i] + ".main.ini");

					CString filename,numstr;
					for (int j=0;j<8;j++)
					{
						numstr.Format("%d",j);
						filename = mainFileArray[i] + numstr;
						DeleteFile(filename);
						DeleteFile(filename+".ini");

					}
				}

				downloadArray.RemoveAt(i);
				mainFileArray.RemoveAt(i);
				statusArray.RemoveAt(i);
				totalBytesArray.RemoveAt(i);
				loadedBytesArray.RemoveAt(i);
				resumableArray.RemoveAt(i);

				return 1;

			}			
			
			return 0;

		}

		if (gDownBox[i].PtInRect(pt))		
		{			
			if (i!=numItems-1)
			{

				CString tempStr;
				tempStr = downloadArray[i];
				downloadArray[i] = downloadArray[i+1];
				downloadArray[i+1] = tempStr;

				tempStr = mainFileArray[i];
				mainFileArray[i] = mainFileArray[i+1];
				mainFileArray[i+1] = tempStr;

				DWORD tempDW;
				tempDW = statusArray[i];
				statusArray[i] = statusArray[i+1];
				statusArray[i+1] = tempDW;

				tempDW = totalBytesArray[i];
				totalBytesArray[i] = totalBytesArray[i+1];
				totalBytesArray[i+1] = tempDW;

				tempDW = loadedBytesArray[i];
				loadedBytesArray[i] = loadedBytesArray[i+1];
				loadedBytesArray[i+1] = tempDW;		

				tempDW = resumableArray[i];
				resumableArray[i] = resumableArray[i+1];
				resumableArray[i+1] = tempDW;

				
			}			
			
			return 1;
			
		}
	
	
		if (gUpBox[i].PtInRect(pt))
		{						
			if (i!=0)
			{

				CString tempStr;
				tempStr = downloadArray[i];
				downloadArray[i] = downloadArray[i-1];
				downloadArray[i-1] = tempStr;

				tempStr = mainFileArray[i];
				mainFileArray[i] = mainFileArray[i-1];
				mainFileArray[i-1] = tempStr;

				DWORD tempDW;
				tempDW = statusArray[i];
				statusArray[i] = statusArray[i-1];
				statusArray[i-1] = tempDW;

				tempDW = totalBytesArray[i];
				totalBytesArray[i] = totalBytesArray[i-1];
				totalBytesArray[i-1] = tempDW;

				tempDW = loadedBytesArray[i];
				loadedBytesArray[i] = loadedBytesArray[i-1];
				loadedBytesArray[i-1] = tempDW;		

				tempDW = resumableArray[i];
				resumableArray[i] = resumableArray[i-1];
				resumableArray[i-1] = tempDW;

			}			
			return 1;
			
		}
	
		if (gSaveBox[i].PtInRect(pt))
		{		
			
			//need to test for completeness of downloads
			if (statusArray[i] == ST_COMPLETED)
			{
				CString strFilter = "*.*";
				CString defaultName("");
				CString extension("");
				int nameable = verifyNameAble(mainFileArray[i],extension);				
				if (nameable)
				{
						

					int foundslash = mainFileArray[i].ReverseFind('\\');
					if (foundslash>0)	
					{	
						int strlength = mainFileArray[i].GetLength();
						defaultName = mainFileArray[i].Right(strlength-foundslash-1);
						strFilter = "*" + extension;

					}			


				}
								
				//Set default directory to "C:" and/or default name to....
				CFileDialog nameDlg(FALSE,extension,defaultName,OFN_LONGNAMES | OFN_OVERWRITEPROMPT,strFilter,this);	
							
				if(nameDlg.DoModal() == IDOK)
				{				
					CString nameStr = nameDlg.GetFileName();
					if (mainFileArray[i] !=nameStr)
					CopyFile(mainFileArray[i],nameStr,TRUE);
						
				}
				else {				
					
					return 0;

				}
				
				return 0;

			}
			
		}
		
		

	}

	return 0;

}

BOOL CSploadView::OnEraseBkgnd(CDC* pDC) 
{
	
	//return CView::OnEraseBkgnd(pDC);
	return TRUE;
}



//For click detection in spbrowser
int verifyDownloadable(CString urlString)
{
	int len=urlString.GetLength();


	if (len>2)
	{
		CString ext =  urlString.Right(2);
		ext.MakeLower();

		if (ext==".z")
				return 1;

		if (len>3)
		{

			ext =  urlString.Right(3);
			ext.MakeLower();

			if (ext==".gz")
				return 1;
			else if (ext==".tz")
				return 1;
			else if (ext==".uu")
				return 1;
			

			if (len>4)
			{
				ext =  urlString.Right(4);
				ext.MakeLower();
				if (ext==".htm")
					return 0;
				else if (ext==".cgi")
					return 0;
				else if (ext==".exe")
					return 1;
				else if (ext==".zip")
					return 1;
				else if (ext==".tar")
					return 1;
				else if (ext==".rar")
					return 1;
				else if (ext==".arj")
					return 1;
				else if (ext==".b64")
					return 1;
				else if (ext==".bhx")
					return 1;
				else if (ext==".arc")
					return 1;
				else if (ext==".cab")
					return 1;
				else if (ext==".hqx")
					return 1;
				else if (ext==".lha")
					return 1;
				else if (ext==".lzh")
					return 1;
				else if (ext==".mim")
					return 1;
				else if (ext==".hqx")
					return 1;
				else if (ext==".taz")
					return 1;
				else if (ext==".tgz")
					return 1;
				else if (ext==".xxe")
					return 1;

				if (len>5)
				{

					ext =  urlString.Right(5);
					ext.MakeLower();
					
					if (ext==".html")
						return 0;
					else if (ext==".gzip")
						return 1;
					else if (ext==".bzip")
						return 1;

				}



				//if (urlString
			
				

			}

		}

	}

	return 0;
}


//For checking whether autonaming is possible
int verifyNameAble(CString nameString,CString& extension)
{


	int len=nameString.GetLength();

	CString filterString = "\\/:*?\"<>|";

	if (len>2)
	{
		CString ext =  nameString.Right(2);
		ext.MakeLower();

		if (ext==".z")
		{		
			extension = ".z";
			return 1;

		}

		if (len>3)
		{

			ext =  nameString.Right(3);
			ext.MakeLower();

			if (ext==".gz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".tz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".uu")
			{
				extension = ext;
				return 1;
			}
			else if (ext[0]=='.')
			{
				extension = ext;
				return 1;
			}
			

			if (len>4)
			{
				ext =  nameString.Right(4);
				ext.MakeLower();
				if (ext==".htm")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".cgi")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".exe")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".zip")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".rar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arj")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".b64")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".bhx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arc")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".cab")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lha")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lzh")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".mim")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".taz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tgz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".xxe")
				{
					extension = ext;
					return 1;
				}
				else if (ext[0]=='.')
				{
					extension = ext;
					return 1;
				}

				if (len>5)
				{

					ext =  nameString.Right(5);
					ext.MakeLower();
					
					if (ext==".html")
					{
						extension = ext;
						return 1;
					}
					else if (ext==".gzip")
					{
						extension = ext;
						return 1;
					}
					else if (ext==".bzip")
					{
						extension = ext;
						return 1;
					}

				}

			}

		}

	}




	extension = "";

	return 0;
}


int verifyNameAbleAdd(CString nameString,CString& extension)
{


	int len=nameString.GetLength();

	CString filterString = "\\/:*?\"<>|";


	int ret = nameString.FindOneOf(filterString);	
	if (ret>=0)
	{
		extension = "*.*";
		return 0;
	}
	else if (len>70)
	{
		extension = "*.*";
		return 0;

	}


	if (len>2)
	{
		CString ext =  nameString.Right(2);
		ext.MakeLower();

		if (ext==".z")
		{		
			extension = ".z";
			return 1;

		}

		if (len>3)
		{

			ext =  nameString.Right(3);
			ext.MakeLower();

			if (ext==".gz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".tz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".uu")
			{
				extension = ext;
				return 1;
			}
			else if (ext[0]=='.')
			{
				extension = ext;
				return 1;
			}
			

			if (len>4)
			{
				ext =  nameString.Right(4);
				ext.MakeLower();
				if (ext==".htm")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".cgi")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".exe")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".zip")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".rar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arj")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".b64")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".bhx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arc")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".cab")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lha")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lzh")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".mim")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".taz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tgz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".xxe")
				{
					extension = ext;
					return 1;
				}
				else if (ext[0]=='.')
				{
					extension = ext;
					return 1;
				}

				if (len>5)
				{

					ext =  nameString.Right(5);
					ext.MakeLower();
					
					if (ext==".html")
					{
						extension = ext;
						return 1;
					}
					else if (ext==".gzip")
					{
						extension = ext;
						return 1;
					}
					else if (ext==".bzip")
					{
						extension = ext;
						return 1;
					}

				}



				//if (nameString
			
				

			}

		}

	}

	return 0;
}


//more restrictive form of verifyNamable
//this function does not returns true for html,htm type of files
int verifyNameAble2(CString nameString,CString& extension)
{
	int len=nameString.GetLength();

	CString filterString = "\\/:*?\"<>|";

	if (len>2)
	{
		CString ext =  nameString.Right(2);
		ext.MakeLower();

		if (ext==".z")
		{		
			extension = ".z";
			return 1;

		}

		if (len>3)
		{

			ext =  nameString.Right(3);
			ext.MakeLower();

			if (ext==".gz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".tz")
			{
				extension = ext;
				return 1;
			}
			else if (ext==".uu")
			{
				extension = ext;
				return 1;
			}

			if (len>4)
			{
				ext =  nameString.Right(4);
				ext.MakeLower();
				if (ext==".exe")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".zip")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".rar")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arj")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".b64")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".bhx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".arc")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".cab")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lha")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".lzh")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".mim")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".hqx")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".taz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".tgz")
				{
					extension = ext;
					return 1;
				}
				else if (ext==".xxe")
				{
					extension = ext;
					return 1;
				}

				if (len>5)
				{

					ext =  nameString.Right(5);
					ext.MakeLower();
					
					if (ext==".gzip")
					{
						extension = ext;
						return 1;
					}
					else if (ext==".bzip")
					{
						extension = ext;
						return 1;
					}

				}



				//if (nameString
			
				

			}

		}

	}


	return 0;
}


int CheckConnection()
{

	CString ipAddress = GetAddress();
	if (ipAddress == "127.0.0.1")
	{

		DWORD dwFlags;
		BOOL ret = InternetGetConnectedState(&dwFlags,0);
		if (ret)
			return 1;
		else
			return 0;

	}


	return 1;

}




CString GetAddress()
{

	CString retString;
	DWORD myIP = GetMyIpAddress();
	retString.Format("%d.%d.%d.%d",myIP>>24,(myIP<<8)>>24,(myIP<<16)>>24,myIP & 255);
	
	return retString;


}


DWORD GetIpAddress(char *hostname)
{
 DWORD dwIP = 0;

 TCHAR    msg[128];
 HOSTENT *lpHost=NULL;
 struct sockaddr_in   dest;
 
 lpHost = gethostbyname(hostname);
 if (lpHost == NULL)
 {
  wsprintf(msg, _T("gethostbyname failed: %d"),
    WSAGetLastError());
  MessageBox(NULL, msg, NULL, MB_OK);
 }
 else
 {
  for(int i=0; lpHost->h_addr_list[i] != NULL ;i++)
  {
             memcpy(&(dest.sin_addr),
     lpHost->h_addr_list[i],
     lpHost->h_length);
      dwIP = ntohl( dest.sin_addr.S_un.S_addr );
  } 
 }

 return dwIP;
}



DWORD GetMyIpAddress()
{
 DWORD dwIP = 0;
 char szBuffer[100];

 WSADATA WSAData;
 WORD wVersionRequested = MAKEWORD( 2, 0 );

 if ( 0 == WSAStartup( wVersionRequested, &WSAData ) )
 {
  if ( 0 == gethostname(szBuffer, sizeof(szBuffer)))
  {
   dwIP = GetIpAddress( szBuffer );
  
  }

  WSACleanup();

 }

 return dwIP;

}


//verify Meta refresh case
int verifyRedirect(CString filename)
{

	redirectURL = "";

	DWORD len = GetFileLength(filename);
	char filecontent[120000];

		//if (((len>0) && (len< 70000)) || ((specialcase==2) && (len< 120000)))
		if ((len>0) && (len< 70000))
		{
			FILE * pvfile = fopen(filename,"rb");
			if (pvfile)
			{
				DWORD count = fread(filecontent,1,len,pvfile);
				
				if (count == len)
				{
					filecontent[len] = 0;
					CString verifyStr = filecontent;
					CString verifyOrg = filecontent;
					
					//Parsing : Determine need to redirect
					verifyStr.MakeLower();
					int findtail = -1;
					int findhead = -1;
					int findval = verifyStr.Find("html");
					if (findval >= 0)
					{
						findhead = verifyStr.Find("head");
						findtail = verifyStr.Find("/head");

						if ((findhead >= 0) && (findtail>= 0) && (findtail >  findhead))
						{
							
							int length;
							CString verifyHeader;
							verifyHeader = verifyStr;

							length = verifyHeader.GetLength(); 							

							if (length > 5)
							{
								int findrefresh = verifyHeader.Find("refresh");
								TRACE("\nRefresh Found");
								if (findrefresh >= 0)
								{
									CString verifyMeta = verifyHeader;
									int findgreater = verifyHeader.Find(">",findrefresh);
									verifyMeta = verifyMeta.Left(findgreater+1);									
									verifyOrg = verifyOrg.Left(findgreater+1);
							
							
									
									//find the 1st '<' before the 1st '>' after 'refresh'
									int findsmaller = verifyMeta.Find("<"); 									

									if (findsmaller >= 0) 
									{
										length = verifyMeta.GetLength(); 
										verifyMeta = verifyMeta.Right(length - findsmaller);										
										verifyOrg = verifyOrg.Right(length - findsmaller);	

										
										//TRACE("\nMeta : %s",verifyMeta);
										int findmeta = verifyMeta.Find("meta");
										int findhteq = verifyMeta.Find("http-equiv");

										if ((findmeta >=0) && (findhteq >=0))
										{
											int findcontent = verifyMeta.Find("content");
											//TRACE("\ncontent %d",findcontent);
											if (findcontent >=0)
											{
												int firstquote  = verifyMeta.Find("\"",findcontent);
												int secondquote  = verifyMeta.Find("\"",firstquote+1);

												//TRACE("\n%d %d",firstquote,secondquote);
												//TRACE("\nMeta : %s",verifyMeta);
												if ((firstquote >= 0) && (secondquote>=0) && (secondquote > firstquote))
												{
													verifyMeta = verifyMeta.Left(secondquote);
													verifyOrg = verifyOrg.Left(secondquote);
													
													length = verifyMeta.GetLength(); 

													verifyMeta = verifyMeta.Right(length - firstquote);
													verifyOrg = verifyOrg.Right(length - firstquote);

													length = verifyMeta.GetLength(); 


													//TRACE("\nRedirect URL 1: %s",verifyMeta);

													

													if (length > 5)
													{
														int findurl  = verifyMeta.Find("http://");
														if (findurl >= 0)
														{

															TRACE("\n%d ",findurl);

															length = verifyMeta.GetLength(); 
															verifyMeta = verifyMeta.Right(length - findurl);
															verifyOrg = verifyOrg.Right(length - findurl);
															
															length = verifyMeta.GetLength(); 
															
															if (length > 1)
															{

																//verifyMeta = "http://" + verifyMeta;
																TRACE("\nRedirect URL : %s",verifyMeta);
																//redirectURL = verifyMeta;
																
																//preserve case															
																redirectURL = verifyOrg;
																
																return 1;



 
															}

														}

													}

												}

											}
										}

									}


								}








							}

						}

					}


				}


			}
			else 
				return 0;
			

		}
	

	return 0;
}


//verify iFrameCase
int verifyRedirect2(CString filename)
{

		redirectURL = "";

		DWORD len = GetFileLength(filename);
		char filecontent[120000];

		if ((len>0) && (len< 70000))
		//if (((len>0) && (len< 70000)) || ((specialcase==2) && (len< 120000)))
		{
			FILE * pvfile = fopen(filename,"rb");
			if (pvfile)
			{
				DWORD count = fread(filecontent,1,len,pvfile);
				
				if (count == len)
				{
					filecontent[len] = 0;
					CString verifyStr = filecontent;
					CString verifyOrg = filecontent;		
					
					//Parsing : Determine need to redirect
					verifyStr.MakeLower();
					int findtail = -1;
					int findhead = -1;
					int findval = verifyStr.Find("html");
					if (findval >= 0)
					{
						findhead = verifyStr.Find("<iframe");
						findtail = verifyStr.Find("/iframe");

						if ((findhead >= 0) && (findtail>= 0) && (findtail >  findhead))
						{
							
							CString verifyHeader;
							verifyHeader = verifyStr;
							verifyHeader = verifyHeader.Left(findtail);
							verifyOrg = verifyOrg.Left(findtail);
							
							
							int length = verifyHeader.GetLength(); 
							verifyHeader = verifyHeader.Right(length - findhead);	
							verifyOrg = verifyOrg.Right(length - findhead);	
							
							length = verifyHeader.GetLength(); 

							//TRACE("\nRedirect URL 2: %s",verifyOrg);
							

							if (length > 5)
							{
								int findsrc = verifyHeader.Find("src");
								if (findsrc >= 0)
								{
									CString verifyMeta = verifyHeader;
									//find the 2 quotes
									int findsmaller = verifyMeta.Find("\"",findsrc);
									int findgreater = verifyHeader.Find("\"",findsmaller+1);

									if ((findsmaller >= 0) && (findgreater >=0) && (findgreater > findsmaller))
									{
										verifyMeta = verifyMeta.Left(findgreater);
										verifyOrg = verifyOrg.Left(findgreater);

										length = verifyMeta.GetLength(); 
										
										verifyMeta = verifyMeta.Right(length - findsmaller);
										verifyOrg = verifyOrg.Right(length - findsmaller);
										
										length = verifyMeta.GetLength(); 

																				
										

										//TRACE("\nRedirect URL 1: %s",verifyOrg);

											if (length > 5)
											{
												int findurl  = verifyMeta.Find("http://");
												if (findurl >= 0)
												{

													TRACE("\n%d ",findurl);

													length = verifyMeta.GetLength(); 
													verifyMeta = verifyMeta.Right(length - findurl);													
													verifyOrg = verifyOrg.Right(length - findurl);

													length = verifyMeta.GetLength(); 

													if (length > 1)
													{
														CString ext;
														int ret = verifyNameAble2(verifyMeta,ext);

														if (ret)
														{															
															TRACE("\nRedirect URL : %s",verifyOrg);
															
															//redirectURL = verifyMeta;
															//preserve case
															redirectURL = verifyOrg;

															return 1;

														}


													}

												}

											}

										}

									}

								}

							}

						}

					}


				}


			}

		
		return 0;
}



int LoadDownloadArray(FILE* pSaveFile)
{
	downloadArray.RemoveAll();

	if (pSaveFile)
	{
		
		int numItems = 0;
		int itemSize[100];
		int itemSizeMain[100];

		int retBytes = 0;

		DWORD val = 0;
		char buf[1000];
		retBytes = fread(&val,1,4,pSaveFile);
		if (retBytes != 4) 
			return 1;

		numItems = val;

		if (numItems > 0) 
		{
			if (numItems > 100) //limit to 100 urls
				numItems = 100; 

			for (int i=0;i<numItems;i++)
			{
				retBytes = fread(&itemSize[i],1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;
			}


			for (i=0;i<numItems;i++)
			{
				if ((itemSize[i]>0) && (itemSize[i]<1000))
				{
					retBytes = fread(buf,1,itemSize[i],pSaveFile);
					if (retBytes != itemSize[i]) 
						return 1;

					buf[itemSize[i]] = 0;
					CString urlString = buf;
					downloadArray.Add(urlString);

				}
			}


			for (i=0;i<numItems;i++)
			{
				retBytes = fread(&itemSizeMain[i],1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;
			}


			for (i=0;i<numItems;i++)
			{
				if ((itemSizeMain[i]>0) && (itemSizeMain[i]<1000))
				{
					retBytes = fread(buf,1,itemSizeMain[i],pSaveFile);
					if (retBytes != itemSizeMain[i]) 
						return 1;

					buf[itemSizeMain[i]] = 0;
					CString mainString = buf;
					mainFileArray.Add(mainString);

				}
			}


			for (i=0;i<numItems;i++)
			{
				retBytes = fread(&val,1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;

				statusArray.Add(val);
			}


			for (i=0;i<numItems;i++)
			{
				retBytes = fread(&val,1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;			

				loadedBytesArray.Add(val);
			}


			for (i=0;i<numItems;i++)
			{
				retBytes = fread(&val,1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;			

				totalBytesArray.Add(val);
			}


			for (i=0;i<numItems;i++)
			{
				retBytes = fread(&val,1,4,pSaveFile);
				if (retBytes != 4) 
					return 1;	
				
				resumableArray.Add(val);
			}

		}


		//Download Folder
		{

			retBytes = fread(&val,1,4,pSaveFile);
			if (retBytes != 4) 
					return 1;			

			retBytes = fread(buf,1,val,pSaveFile);
			if (retBytes != val) 
					return 1;
			buf[val] = 0;
			downloadFolder = buf;			

			TRACE("\nP Download Folder %s",downloadFolder);

			if (downloadFolder != "")
			{
				CString testfile = downloadFolder + "\\testfile.txt";
				FILE* tf = fopen(testfile,"wt");
				if (tf == NULL)
				{
					downloadFolder = "";

				}
				else
				{
					fclose(tf);

				}

			}
			
			TRACE("\nL Download Folder %s",downloadFolder);


		}
		

	}


	return 0;
}


int SaveDownloadArray(FILE* pSaveFile)
{
	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	DWORD val = num;
	
	int itemSize[100];
	int itemSizeMain[100];
	int retBytes = 0;
	retBytes = fwrite(&val,1,4,pSaveFile);
	if (retBytes != 4) 
		return 1;

	for (int i=0;i<num;i++)
	{
		val = downloadArray[i].GetLength();
		if ((val<1000) && (val>0))
		{
			retBytes = fwrite(&val,1,4,pSaveFile);			
			itemSize[i]=val;

		}
		else
			itemSize[i]=0;

	}

	for (i=0;i<num;i++)
	{
		if (itemSize[i] > 0)
			retBytes = fwrite(LPCTSTR(downloadArray[i]),1,itemSize[i],pSaveFile);

	}


	for (i=0;i<num;i++)
	{
		val = mainFileArray[i].GetLength();
		if ((val<1000) && (val>0))
		{
			retBytes = fwrite(&val,1,4,pSaveFile);			
			itemSizeMain[i]=val;

		}
		else
			itemSizeMain[i]=0;

	}

	for (i=0;i<num;i++)
	{

		if (itemSizeMain[i] > 0)
			retBytes = fwrite(LPCTSTR(mainFileArray[i]),1,itemSizeMain[i],pSaveFile);

	}

	for (i=0;i<num;i++)
	{
		val = statusArray[i];		
		if (val==ST_DOWNLOADINPROGRESS)
		{
			if (resumableArray[i])
				val=ST_PARTIAL;
			else
				val=ST_PENDING;

		}

		retBytes = fwrite(&val,1,4,pSaveFile);	
	}

	for (i=0;i<num;i++)
	{
		val =loadedBytesArray[i];
		retBytes = fwrite(&val,1,4,pSaveFile);
		
	}


	for (i=0;i<num;i++)
	{
		val = totalBytesArray[i];
		retBytes = fwrite(&val,1,4,pSaveFile);
		
	}


	for (i=0;i<num;i++)
	{
		val = resumableArray[i];
		retBytes = fwrite(&val,1,4,pSaveFile);
		
	}

	
	if (downloadFolder=="")
	{
		val = 0;
		retBytes = fwrite(&val,1,4,pSaveFile);

	}
	else
	{
		val = downloadFolder.GetLength();
		retBytes = fwrite(&val,1,4,pSaveFile);
		retBytes = fwrite(LPCTSTR(downloadFolder),1,val,pSaveFile);		

	}	


	return 0;

}


int FreeDownloadArray()
{
	downloadArray.RemoveAll();
	mainFileArray.RemoveAll();
	statusArray.RemoveAll();
	totalBytesArray.RemoveAll();
	loadedBytesArray.RemoveAll();
	resumableArray.RemoveAll();

	return 0;
}


int InitDownloadArray()
{
	FreeDownloadArray();

	return 0;

}


int RemoveDownloadArrayItem(int n)
{
	int num = downloadArray.GetSize();
	if ((n>=0) && (n<num))
	{
		downloadArray.RemoveAt(n,1);
		downloadArray.FreeExtra();

		mainFileArray.RemoveAt(n,1);
		mainFileArray.FreeExtra();
		
		statusArray.RemoveAt(n,1);
		statusArray.FreeExtra();
		
		totalBytesArray.RemoveAt(n,1);
		totalBytesArray.FreeExtra();
		
		loadedBytesArray.RemoveAt(n,1);
		loadedBytesArray.FreeExtra();

		resumableArray.RemoveAt(n,1);
		resumableArray.FreeExtra();

	}

	return 0;
			
}

int CSploadView::AddDownloadArrayItem(CString str)
{

	int num = downloadArray.GetSize();
	if (num<100)
	{		
		DWORD dwServiceType;
		CString strServer;
		CString strObject;
		INTERNET_PORT nPort;
		int retVal = AfxParseURL(str, dwServiceType, strServer, strObject,nPort);

		if (retVal==0)
		{

			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_INVALIDURL);			
			ShowMessage(errStr,errNote);
			return 1;
		}

		if 	((dwServiceType!=AFX_INET_SERVICE_HTTP) && (dwServiceType!=AFX_INET_SERVICE_FTP))
		{
			//MessageBox("This URL cannot be accessed with HTTP or FTP protocol","Error",MB_OK | MB_ICONEXCLAMATION);
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_URLACCESS);
			ShowMessage(errStr,errNote);
			
			return 1;

		}			
		
		strServer.TrimLeft();
		strServer.TrimRight();	
		if (strServer=="")
		{
			//MessageBox("Unable to parse the server. Please provide a valid domain name.","Error",MB_OK | MB_ICONEXCLAMATION);
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_PARSE);
			ShowMessage(errStr,errNote);
			
			return 1;


		}

		strObject.TrimLeft();
		strObject.TrimRight();
		if (strObject=="")
		{
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_REMOTE);
			ShowMessage(errStr,errNote);		
			//MessageBox("Unable to parse the remote object. Please provide a valid file name.","Error",MB_OK | MB_ICONEXCLAMATION);
			return 1;

		}
		
		
		CString moduleDir;
		if (downloadFolder=="")
			moduleDir =  GetModuleDirectory() + "\\Download";
		else
			moduleDir = downloadFolder ;

		CString filePath;	
	
		int strlength = str.GetLength();
		int foundslash = 0;
		if ((strlength>0) && (str[strlength-1]=='/'))
		{			
			filePath=moduleDir + "\\" + "index.html";
		
		}	
		else
		{
		
			foundslash =str.ReverseFind('/');	
			if (foundslash>0)	
			{	
				filePath=str.Right(strlength-foundslash-1);	

			}	
			else
			{
				foundslash = str.ReverseFind('\\');
				if (foundslash>0)	
				{	
					filePath=str.Right(strlength-foundslash-1);

				}			

			}

			needDialog = 0;
			if (filePath!="")	
			{	
				
				//CString exten;
				//int nameableAdd = verifyNameAbleAdd(filePath,exten);		


				//moddir
				CString filePathTest = moduleDir + "\\" + filePath;
				TRACE("\nfilePathTest %s",filePathTest);
				FILE* temp = fopen(filePathTest,"wb");
				if (temp)
				{
					fclose(temp); 
					DeleteFile(filePath);
					filePath = filePathTest;

				}
				else				
				{
						CString strFilter = "*.*";
						CString extension("");
						CString defaultName("*.*"); 
						int nameable = verifyNameAble(str,extension);
						if (nameable)
						{
							strFilter = extension;
							defaultName = "*" + extension;

						}
						CFileDialog nameDlg(FALSE,_T(""),defaultName,OFN_LONGNAMES | OFN_OVERWRITEPROMPT,strFilter,this);	
									
						if(nameDlg.DoModal() == IDOK)
						{				
							filePath = nameDlg.GetPathName();						
								
						}
						else						
							return 1;

					//}
					

				}
				

			}	

		}

		//Check for repeat case
		for (int j=0;j<num;j++)
		{
			if (downloadArray[j]==str)
				return 1;

		}
		

		if (filePath !="")
		{
			int insertIndex = num;

			if (!downloadInProgress)
			{
				insertIndex = 0;

			}
			else
			{
				for (int j=0;j<num;j++)
				{
					//if ((statusArray[j]==ST_COMPLETED) || (statusArray[j]==ST_ERROR) || (statusArray[j]==ST_PARTIAL))
					if (statusArray[j]==ST_DOWNLOADINPROGRESS)
					{
						insertIndex = j+1;
						break;
					}
					else 
					{
						//Do nothing

					}


				}

			}

			TRACE("\nInsertNum = %d",insertIndex);
			//TRACE("\nDxinProgress = %d",downloadInProgress);
			if (insertIndex >= num)
			{	
	
				downloadArray.Add(str);

				//all other arrays are filled in by PrepareData
				filePath.MakeLower();
				mainFileArray.Add(filePath);		
				statusArray.Add(ST_PENDING);		
				totalBytesArray.Add(0);		
				loadedBytesArray.Add(0);
				resumableArray.Add(0);
				

			}
			else
			{

				//insert at index
				downloadArray.InsertAt(insertIndex,str,1);

				//all other arrays are filled in by PrepareData
				filePath.MakeLower();
				mainFileArray.InsertAt(insertIndex,filePath,1);		
				statusArray.InsertAt(insertIndex,ST_PENDING,1);		
				totalBytesArray.InsertAt(insertIndex,0,1);		
				loadedBytesArray.InsertAt(insertIndex,0,1);
				resumableArray.InsertAt(insertIndex,0,1);


			}
			
			
		}


	}

	return 0;

}


int CSploadView::ModifyDownloadArrayItem(CString str,int n)
{

	int num = downloadArray.GetSize();
	if (num<100)
	{		
		DWORD dwServiceType;
		CString strServer;
		CString strObject;
		INTERNET_PORT nPort;
		int retVal = AfxParseURL(str, dwServiceType, strServer, strObject,nPort);

		if (retVal==0)
		{
			
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_INVALIDURL);
			ShowMessage(errStr,errNote);			
			//MessageBox("Invalid URL","Error",MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}

		if 	((dwServiceType!=AFX_INET_SERVICE_HTTP) && (dwServiceType!=AFX_INET_SERVICE_FTP))
		{
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_URLACCESS);
			ShowMessage(errStr,errNote);		
			//MessageBox("This URL cannot be accessed with HTTP or FTP protocol","Error",MB_OK | MB_ICONEXCLAMATION);
			return 1;

		}			
		
		strServer.TrimLeft();
		strServer.TrimRight();	
		if (strServer=="")
		{
			
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_PARSE);
			ShowMessage(errStr,errNote);			
			//MessageBox("Unable to parse the server. Please provide a valid domain name.","Error",MB_OK | MB_ICONEXCLAMATION);
			return 1;

		}

		strObject.TrimLeft();
		strObject.TrimRight();
		if (strObject=="")
		{
			CString errStr;
			CString errNote;
			errNote.LoadString(IDS_ERRNOTE);
			errStr.LoadString(IDS_REMOTE);
			ShowMessage(errStr,errNote);			
			//MessageBox("Unable to parse the remote object. Please provide a valid file name.","Error",MB_OK | MB_ICONEXCLAMATION);
			return 1;

		}		

		CString moduleDir;
		if (downloadFolder=="")
			moduleDir =  GetModuleDirectory() + "\\Download";
		else
			moduleDir = downloadFolder ;

		CString filePath;	
	
		int strlength = str.GetLength();
		int foundslash = 0;
		if ((strlength>0) && (str[strlength-1]=='/'))
		{			
			filePath=moduleDir + "\\" + "index.html";
		
		}	
		else
		{
		
			foundslash =str.ReverseFind('/');	
			if (foundslash>0)	
			{	
				filePath=str.Right(strlength-foundslash-1);	

			}	
			else
			{
				foundslash = str.ReverseFind('\\');
				if (foundslash>0)	
				{	
					filePath=str.Right(strlength-foundslash-1);

				}			

			}
	
			if (filePath!="")	
			{			
				CString filePathTest = moduleDir + "\\" + filePath;


				FILE* temp = fopen(filePathTest,"wb");
				if (temp)
				{
					fclose(temp); 
					DeleteFile(filePath);
					filePath = filePathTest;

				}
				else
				{					

					CString strFilter = "*.*";
					CString extension("");
					CString defaultName("*.*"); 
					int nameable = verifyNameAble(str,extension);
					if (nameable)
					{
						strFilter = extension;
						defaultName = "*" + extension;

					}
					CFileDialog nameDlg(FALSE,_T(""),defaultName,OFN_LONGNAMES | OFN_OVERWRITEPROMPT,strFilter,mainView);	
								
					if(nameDlg.DoModal() == IDOK)
					{				
						filePath = nameDlg.GetPathName();						
							
					}
					else						
						return 1;
					

				}
				

			}	

		}

		//Check for repeat case
		for (int j=0;j<num;j++)
		{
			if (downloadArray[j]==str)
				return 1;

		}
		

		if (filePath !="")
		{
		
			downloadArray[n] = str;
		
			filePath.MakeLower();
			mainFileArray[n] =filePath;		
			statusArray[n] = ST_PENDING;		
			totalBytesArray[n] = 0;		
			loadedBytesArray[n] = 0;
			resumableArray[n] = 0;
			
		}


	}

	return 0;

}


CRect CSploadView::PrepareScrollRect()
{

	CDC* pDC = GetDC();

	CRect clientrect;	
	GetClientRect(&clientrect);
	CRect listrect;
	listrect = clientrect;
	listrect.DeflateRect(15,15,15,15);

	CRect ScrollRect;
	ScrollRect.left = listrect.left ; 	
	ScrollRect.top = listrect.top  + 50;
	ScrollRect.bottom = listrect.bottom - 18;
	ScrollRect.right = listrect.right - 18;
	
	ReleaseDC(pDC);

	return ScrollRect;

}
	

int CSploadView::CheckClickableRects(CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem,int& numItems)
{
	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	int xoffset,yoffset;

	int boxtop;
	int boxleft = ScrollRect.right - 2 - 13;


	//Compute Full Size
	int fullSizeX = ScrollRect.Width();
	int fullSizeY = num*80;

	//Need Scrolling
	if (fullSizeY > ScrollRect.Height())
	{
	

	}
	else
	{

		xscrollpos = 0;
		yscrollpos = 0;

	}
	
	for (int i=0;i<num;i++)
	{

		xoffset=ScrollRect.left + 10;
		yoffset=ScrollRect.top + i * 80 + 10 - yscrollpos;
		boxtop = yoffset - 8;
		yoffset+=12;

		CRect delbox, upbox, downbox, savebox;

		delbox.right = boxleft + 13;
		delbox.left = boxleft;
		delbox.top = boxtop;
		delbox.bottom = boxtop + 14;

		gDelBox[i] = delbox;
		
		upbox.right = boxleft + 13;
		upbox.left = boxleft;
		upbox.top = boxtop + 13;
		upbox.bottom = boxtop + 14 + 13;

		gUpBox[i] = upbox;
		
		downbox.left = boxleft;
		downbox.right = boxleft + 13;
		downbox.top = boxtop + 26;
		downbox.bottom = boxtop + 14 + 26;

		gDownBox[i] = downbox;

		
		savebox.right = ScrollRect.right - 2;
		savebox.left = ScrollRect.right - 2 - gExtent.cx;		
		savebox.bottom = yoffset + 58 - 2;
		savebox.top = savebox.bottom - gExtent.cy ;		

		gSaveBox[i] = savebox;



	}

	numItems = num;

	return 0;

}

int DrawGradientBackground(CDC* pDC,CRect backgroundRect,int gradientType)
{
	//Draw Decorations
	COLORREF Color1,Color2,Color3,Color4;

	if (gradientType>=1024)
	{
		gradientType -=1024;

		if (gradientType==0)
		{		
			Color1 = RGB(101,133,154);
			Color2 = RGB(255,255,255);	
			Color3 = RGB(255,255,200);			
			Color4 = RGB(255,255,255);	
		}
		else if (gradientType==1)
		{
			
			Color1 = RGB(174,192,183);						
			Color2 = RGB(215,223,219);				
			Color3 = RGB(204,0,0);		
			Color4 = RGB(230,127,127);

			
		}
		else if (gradientType==2)
		{
			
			Color1 = RGB(215,223,219);			
			Color2 = RGB(255,255,255);				
			Color3 = RGB(230,127,127);		
			Color4 = RGB(255,255,255);	
			
			
			
		}
		

	}
	else
	{
		if (gradientType==0)
		{
			Color1 = RGB(212,222,217);				
			Color2 = RGB(255,255,255);	
			Color3 = RGB(212,222,217);	
			Color4 = RGB(255,255,255);	
		
		}
		else if (gradientType==1)
		{

			Color1 = RGB(255,255,255);			
			Color2 = RGB(255,255,255);	
			Color3 = RGB(255,255,255);
			Color4 = RGB(255,255,255);	

		
		}		
		else if (gradientType==2)
		{

			Color1 = RGB(212,222,217);				
			Color2 = RGB(255,255,255);	
			Color3 = RGB(212,222,217);	
			Color4 = RGB(255,255,255);	

		
		}
		else if (gradientType==3)
		{
			Color1 = RGB(255,255,255);			
			Color2 = RGB(255,255,255);	
			Color3 = RGB(255,255,255);
			Color4 = RGB(255,255,255);	

		
		}
		
	}
		
	CRect decohalf1 = backgroundRect;
	CRect decohalf2 = backgroundRect;		
		
	decohalf1.right = (decohalf1.left + decohalf1.right)/2;		
	decohalf2.left = (decohalf2.left + decohalf2.right)/2;
	
	DrawGradient (decohalf1, Color1,  Color2,  pDC);	
	DrawGradient (decohalf2, Color3,  Color4,  pDC);	


	return 0;

}


//The scroll positions refers to the coordinates of the destination image (consisting of the list of URLs)  
//It does not refers to the coordinates of the scrollbar
int DrawDownloadArray(CDC* pDC,CDC* pBltDC, CRect ScrollRect, int xscrollpos, int yscrollpos, int selItem, THREADDATA*  threadsinfo[],double downloadRate)
{

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	int xoffset,yoffset,adjustedxoffset;

	int boxtop;
	int boxleft = ScrollRect.right - 2 - 13;


	//Compute Full Size
	int fullSizeX = ScrollRect.Width();
	int fullSizeY = num*80;
	

	//All drawings should be based on y-offsets for vertical alignment
	for (int i=0;i<num;i++)
	{	

		xoffset=ScrollRect.left + 10;
		yoffset=ScrollRect.top + i * 80 + 10 - yscrollpos;
		boxtop = yoffset - 8;

		
		//if (mouseMoveHit!=i)
		{

			CRect backgroundRect;
			backgroundRect = ScrollRect;
			backgroundRect.top = ScrollRect.top + i * 80 + 1 - yscrollpos;
			backgroundRect.bottom = backgroundRect.top + 79;

			CRect testRect;
			BOOL intersect = testRect.IntersectRect(&ScrollRect,&backgroundRect);
			if (intersect)
			{
				int gradientType = i % 4;
				DrawGradientBackground(pDC,backgroundRect,gradientType);

			}
		}
		

		
		CString entryStr;
		entryStr.Format("%d",i+1);
		int lenx = entryStr.GetLength();
		int shiftright = 9*lenx-5;
		pDC->Rectangle(xoffset-9,boxtop,xoffset+shiftright,boxtop + 15);
		pDC->TextOut(xoffset-6,yoffset-8,entryStr);

		adjustedxoffset = xoffset + shiftright + 2;
		
		CString urlStr;
		CString textStr;
		urlStr = "URL : ";
		pDC->TextOut(adjustedxoffset,yoffset,urlStr);
		
		urlStr.Format("%s",downloadArray[i]);
		pDC->TextOut(adjustedxoffset+60,yoffset,urlStr);
		
		yoffset+=12;
		CString fileStr;
		//fileStr.Format("Save To : %s",mainFileArray[i]);
		//fileStr.Format("Save To : ");
		fileStr.LoadString(IDS_SAVETO);
		pDC->TextOut(adjustedxoffset,yoffset,fileStr);		
		
		
		fileStr.Format("%s",mainFileArray[i]);
		pDC->TextOut(adjustedxoffset+60,yoffset,fileStr);

		//At this point yoffset+12 is no longer incremented
		if (statusArray[i]==ST_PENDING)
		{
			//textStr.Format("Status :");
			textStr.LoadString(IDS_STATUSS);

			pDC->TextOut(adjustedxoffset,yoffset+12,textStr);
			//textStr.Format("Pending");
			textStr.LoadString(IDS_PENDING);
			
			pDC->TextOut(adjustedxoffset+60,yoffset+12,textStr);
			

		}
		else if (statusArray[i]==ST_COMPLETED)
		{
			//textStr.Format("Status :");
			textStr.LoadString(IDS_STATUSS);
			pDC->TextOut(adjustedxoffset,yoffset+12,textStr);

			//textStr.Format("Completed");
			textStr.LoadString(IDS_COMPLETED);
			pDC->TextOut(adjustedxoffset+60,yoffset+12,textStr);

			if ((totalBytesArray[i]>0) && (resumableArray[i]))
			{

				double finishRate = (loadedBytesArray[i]*100.0)/(totalBytesArray[i]);			

				//textStr.Format("Percent :");
				textStr.LoadString(IDS_PERCENT);
				pDC->TextOut(adjustedxoffset,yoffset+24,textStr);	

				double sizeLB = loadedBytesArray[i] * 1.0 / 1024;
				double sizeTB = totalBytesArray[i] * 1.0 / 1024;
				
				CString formattingStr;
				formattingStr.LoadString(IDS_STATFORMAT);
				textStr.Format(formattingStr,finishRate,sizeLB,sizeTB);
				
				//TRACE("\nloadedBytesArray[i] = %d",loadedBytesArray[i]);
				pDC->TextOut(adjustedxoffset+60,yoffset+24,textStr);	


			}


		}
		else if (statusArray[i]==ST_ERROR)
		{
			//textStr.Format("Status :");
			textStr.LoadString(IDS_STATUSS);
			pDC->TextOut(adjustedxoffset,yoffset+12,textStr);

			//textStr.Format("Error");
			textStr.LoadString(IDS_ERRORS);
			pDC->TextOut(adjustedxoffset+60,yoffset+12,textStr);

			//textStr.Format("Unable to download this file.");
			textStr.LoadString(IDS_UNABLETOLOAD);
			pDC->TextOut(adjustedxoffset,yoffset+24,textStr);

		}
		else if (statusArray[i]==ST_PARTIAL)
		{
			//textStr.Format("Status :");
			textStr.LoadString(IDS_STATUSS);
			pDC->TextOut(adjustedxoffset,yoffset+12,textStr);			

			//textStr.Format("Partially Completed");
			textStr.LoadString(IDS_PARTIAL);
			pDC->TextOut(adjustedxoffset+60,yoffset+12,textStr);			

			if ((totalBytesArray[i]>0) && (resumableArray[i]))
			{
				double finishRate = (loadedBytesArray[i]*100.0)/(totalBytesArray[i]);			

				//textStr.Format("Percent :");
				textStr.LoadString(IDS_PERCENT);
				pDC->TextOut(adjustedxoffset,yoffset+24,textStr);	

				double sizeLB = loadedBytesArray[i] * 1.0 / 1024;
				double sizeTB = totalBytesArray[i] * 1.0 / 1024;
				
				CString formattingStr;
				formattingStr.LoadString(IDS_STATFORMAT);
				textStr.Format(formattingStr,finishRate,sizeLB,sizeTB);
				//textStr.Format("%.2f%%      Size : %.1fK / %.1fK",finishRate,sizeLB,sizeTB);
				//TRACE("\nloadedBytesArray[i] = %d",loadedBytesArray[i]);
				pDC->TextOut(adjustedxoffset+60,yoffset+24,textStr);	

			}


		}
		else if (statusArray[i]==ST_DOWNLOADINPROGRESS)
		{
			//textStr.Format("Status :");
			textStr.LoadString(IDS_STATUSS);
			pDC->TextOut(adjustedxoffset,yoffset+12,textStr);			

			if (progressStatus==1)
			{
				//textStr.Format("Connecting ...");
				textStr.LoadString(IDS_CONNECTING);

			}
			else if (progressStatus==2)
			{
				//textStr.Format("Merging File ...");
				textStr.LoadString(IDS_MERGING);

			}
			else if (resumableArray[i])
			{
				//textStr.Format("Download In Progress (can resume)");
				textStr.LoadString(IDS_DOWNLOADCANRESUME);

			}
			else
			{
				//textStr.Format("Download In Progress (cannot resume)");
				textStr.LoadString(IDS_DOWNLOADCANNOTRESUME);
			}
			pDC->TextOut(adjustedxoffset+60,yoffset+12,textStr);			

			if ((totalBytesArray[i]>0) && (resumableArray[i]))
			{
				if (currentActive>=0)
				{	
					//textStr.Format("Completed :");
					textStr.LoadString(IDS_COMPLETEDS);
					pDC->TextOut(adjustedxoffset,yoffset+36,textStr);	
					
					
					int leftbase = adjustedxoffset+60;
					int topbase = yoffset+38;
					int fullPart = 20;

					CRect completionRect;
					completionRect.left = leftbase+11 ;	
					completionRect.right = completionRect.left + (fullPart*8) + 1;
					completionRect.top = topbase+1;
					completionRect.bottom = topbase+11;
					pDC->Rectangle(&completionRect);


					double finishRate = 0;			
					DWORD totalLoaded = 0;
					for (int j=0;j<8;j++)
					{
						if (threadsinfo[j])
						{
							int nByteLoad = threadsinfo[j]->nByteLoad;		
							totalLoaded += nByteLoad;

							
							int st = threadsinfo[j]->startload;
							int ed = threadsinfo[j]->endload;
							int range =ed-st+1;
							double completedPercent = 0;
							if ((range>0) && (nByteLoad>=0))
							{
								if (nByteLoad>range)
									nByteLoad = range;

								completedPercent = (nByteLoad * 1.0) / (range * 1.0);


								COLORREF Color1R = RGB(175,221,157);	
								COLORREF Color2R   = RGB(104,133,154);									
								
								//COLORREF Color1R = RGB(0,255,0);	
								//COLORREF Color2R   = RGB(0,0,255);									
								
								completionRect.left = leftbase+12 + (j*fullPart);								
								completionRect.right = completionRect.left + (int) (completedPercent*(fullPart));
								completionRect.top = topbase+2;
								completionRect.bottom = topbase+10;

								if (progressStatus!=1)
									DrawGradientVert (completionRect, Color1R,  Color2R,  pDC);

							}
							

							//TRACE("\nByteLoad %d totalloaded %d",nByteLoad,totalLoaded);

						}
					}
					

					//loadedBytesArray[currentActive] = totalLoaded;
					loadedBytesArray[i] = totalLoaded;

					
					finishRate = (totalLoaded*100.0)/(totalBytesArray[i]);			

					//textStr.Format("Percent :");
					textStr.LoadString(IDS_PERCENT);
					pDC->TextOut(adjustedxoffset,yoffset+24,textStr);	

					double sizeLB = loadedBytesArray[i] * 1.0 / 1024;
					double sizeTB = totalBytesArray[i] * 1.0 / 1024;
					DWORD timeSecRemaining = 0;
					if (downloadRate > 0.1)
					{
						timeSecRemaining = (DWORD) (((totalBytesArray[i]-totalLoaded)/1024) / downloadRate);


						int hours = 0;
						int mins = 0;
						int secs = 0;

						secs = timeSecRemaining % 60;
						mins = (timeSecRemaining/60) % 60;
						hours = timeSecRemaining/3600;						

						CString minStr,secStr,hourStr;
						if (mins<=9)
							minStr.Format(":0%d",mins);
						else
							minStr.Format(":%d",mins);

						if (secs<=9)
							secStr.Format(":0%d",secs);
						else
							secStr.Format(":%d",secs);

						hourStr.Format("%d",hours);

						CString fxStr;
						fxStr.LoadString(IDS_FORMAT1);
						//CString fmtStr = "%.2f%%      Size : %.1fK / %.1fK        Est. Time   " + hourStr + minStr + secStr;
						CString fmtStr = fxStr + hourStr + minStr + secStr;
						textStr.Format(fmtStr,finishRate,sizeLB,sizeTB);

					}
					else
					{
						CString fxStr;
						fxStr.LoadString(IDS_FORMAT2);
						//textStr.Format("%.2f%%      Size : %.1fK / %.1fK",finishRate,sizeLB,sizeTB);
						textStr.Format(fxStr,finishRate,sizeLB,sizeTB);

					}
					//TRACE("\nloadedBytesArray[i] = %d",loadedBytesArray[i]);
					pDC->TextOut(adjustedxoffset+60,yoffset+24,textStr);	
					
					
					

				}
				

			}
			else
			{
				//Do not display if at the connecting phase
				if (progressStatus==0)
				{
					
					if (threadsinfo[0])
						loadedBytesArray[i] = threadsinfo[0]->nByteLoad;
					double sizeLB = loadedBytesArray[i] * 1.0 / 1024;
				
					CString fxStr;
					fxStr.LoadString(IDS_FORMAT3);
					textStr.Format(fxStr,sizeLB);	
					//textStr.Format("Opened so far : %.1f K ",sizeLB);					
					pDC->TextOut(adjustedxoffset,yoffset+24,textStr);	

				}

			}

		}


		CBrush* old_brush = pDC->SelectObject(&whiteBrush);	

		CRect delbox, upbox, downbox, savebox;		

		delbox.right = boxleft + 14;
		delbox.left = boxleft;
		delbox.top = boxtop;
		delbox.bottom = boxtop + 14;

		
		HANDLE oldBitmap = pBltDC->SelectObject(hgDelBox1Bitmap);

		if (statusArray[i]!=ST_DOWNLOADINPROGRESS)
		{
			if ((downloadInProgress) && (i<=currentActive))
			{

				//Do not draw del boxes for items <= currentActive

			}
			else
			{

				if (gLightUpDelBox[i])
				{
					//pDC->SelectObject(&lightUpBrush);		
					pBltDC->SelectObject(hgDelBox2Bitmap);
				}
				else
				{
					//pDC->SelectObject(&whiteBrush);	
					pBltDC->SelectObject(hgDelBox1Bitmap);
					
				}

				pDC->BitBlt(delbox.left, delbox.top,delbox.Width(),delbox.Height(),pBltDC,0,0,SRCCOPY);


			}

		}
		
		upbox.right = boxleft + 14;
		upbox.left = boxleft;
		upbox.top = boxtop + 13;
		upbox.bottom = boxtop + 14 + 13;

		if (!downloadInProgress)
		{

			if (gLightUpUpBox[i])
			{
				//pDC->SelectObject(&lightUpBrush);		
				pBltDC->SelectObject(hgUpBox2Bitmap);
				
			}
			else
			{
				//pDC->SelectObject(&whiteBrush);		
				pBltDC->SelectObject(hgUpBox1Bitmap);
				
			}
			
			
			
			pDC->BitBlt(upbox.left, upbox.top,upbox.Width(),upbox.Height(),pBltDC,0,0,SRCCOPY);

		
		}
		


		downbox.left = boxleft;
		downbox.right = boxleft + 14;
		downbox.top = boxtop + 26;
		downbox.bottom = boxtop + 14 + 26;

		if (!downloadInProgress)
		{

			if (gLightUpDownBox[i])
			{
				//pDC->SelectObject(&lightUpBrush);		
				pBltDC->SelectObject(hgDownBox2Bitmap);
			}
			else
			{
			
				pBltDC->SelectObject(hgDownBox1Bitmap);
				
			}
			

			pDC->BitBlt(downbox.left, downbox.top,downbox.Width(),downbox.Height(),pBltDC,0,0,SRCCOPY);

		}

		CString savestr;
		//savestr.Format(_T("Save"));
		savestr.LoadString(IDS_SAVE);
		savebox.right = ScrollRect.right - 2;
		savebox.left = ScrollRect.right - 2 - gExtent.cx;		
		savebox.bottom = yoffset + 58 - 2;
		savebox.top = savebox.bottom - gExtent.cy ;		
		

		if (statusArray[i] == ST_COMPLETED)
		{
			if (gLightUpSaveBox[i])
			{
				//pDC->SelectObject(&lightUpBrush);		
				pBltDC->SelectObject(hgSaveBox2Bitmap);
			}
			else
			{			
				pBltDC->SelectObject(hgSaveBox1Bitmap);
				
			}
			
			pDC->BitBlt(savebox.left, savebox.top,savebox.Width(),savebox.Height(),pBltDC,0,0,SRCCOPY);

		}
	
		pDC->SelectObject(old_brush);	

		pDC->MoveTo(ScrollRect.left,yoffset+58);
		pDC->LineTo(ScrollRect.right,yoffset+58);

		pBltDC->SelectObject(oldBitmap);

	}

	return 0;


}

//Returns 1 if scrollable
int CSploadView::ComputeScrollRanges()
{
	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;	

	CRect clientrect;	
	GetClientRect(&clientrect);
	CRect listrect;
	listrect = clientrect;
	listrect.DeflateRect(15,15,15,15);

	CRect ScrollRect;
	ScrollRect.left = listrect.left ; 	
	ScrollRect.top = listrect.top  + 50;
	ScrollRect.bottom = listrect.bottom - 18;
	ScrollRect.right = listrect.right - 18;
	m_ScrollRect = ScrollRect;

	int fullSizeX = ScrollRect.Width();
	int fullSizeY = num*80;
	

	scrollbarTop = listrect.top + 50;  //measured at top of thumb
	scrollbarBottom = scrollbarTop + ScrollRect.Height() - 15 - 36 - 1; //measured at top of thumb (15 is the thumb size)
	scrollbarRange = scrollbarBottom - scrollbarTop;

	dstRange.cx = fullSizeX;
	dstRange.cy = fullSizeY - ScrollRect.Height();  //50 is toolbar, 18 is status bar

	if (dstRange.cy <= 0)
	{
		dstRange.cy = 0;
		ScrollPosDst = 0;
	}	
	else if (ScrollPosDst>dstRange.cy)
	{
		ScrollPosDst = dstRange.cy;

	}

	if (ScrollPosDst<0)
		ScrollPosDst = 0;


	if ((dstRange.cy > 0) && (scrollbarRange > 0))
	{
		ScrollBarPos = (ScrollPosDst * scrollbarRange) / dstRange.cy;

	}

	if (fullSizeY > ScrollRect.Height())
	{
		return 1;

	}

	return 0;	

}


void CSploadView::StartDownload()
{
		while (threadRestarting)
		{
			Sleep(50);
		}

		threadRestarting = 1;

		connectedState =  CheckConnection();
		if (connectedState)
			restartLoad = 1;

		if (downloadInProgress==0)
			resetErrors = 1;

		userStopPressed = 0;
		forceAbortion = 0;

		threadRestarting = 0;


}

void CSploadView::StopDownload()
{
		while (threadRestarting)
		{
			Sleep(50);
		}

		threadRestarting = 1;

		//stopDownload = 1;
		userStopPressed = 1;		
		forceAbortion = 1;
		if (retPdResults)
			retPdResults->pdForceAbortion = 1;
		forceStop(-1, threadsinfo);
		
		
		//statusText = _T("Download Stopped");
		statusText.LoadString(IDS_DOWNLOADSTOPPED);

		threadRestarting = 0;



}


void CSploadView::OnLButtonDown(UINT nFlags, CPoint point) 
{

	if ((point.x > 25)  &&
		(point.x < 100)  &&
		(point.y > 25)  &&
		(point.y < 55))
	{	

		StartDownload();
		Invalidate();
		return;
		
	}
	else if ((point.x > 110)  &&
		(point.x < 185)  &&
		(point.y > 25)  &&
		(point.y < 55))
	{		

		
		StopDownload();
		Invalidate();
		return;
	}

	//Assume the scroll range is already properly computed 
	//int IsScrollable = ComputeScrollRanges();
	if (IsScrollable)
	{
		CRect clientrect;	
		GetClientRect(&clientrect);
		CRect listrect;
		listrect = clientrect;
		listrect.DeflateRect(15,15,15,15);

		//TRACE("\nScrollable");
		if ((point.y > listrect.bottom - 36) &&
			(point.y < listrect.bottom - 18) &&
			(point.x < listrect.right) &&
			(point.x > listrect.right-18))
		{
			//Scroll Down
			
			ScrollPosDst += 20;
			if (ScrollPosDst > dstRange.cy)
				ScrollPosDst = dstRange.cy;

			//TRACE("\nScroll Down %d %d %d %d %d",ScrollPosDst,dstRange.cy,scrollbarBottom,scrollbarTop,scrollbarRange);

			ScrollBarPos = (ScrollPosDst * scrollbarRange) / dstRange.cy;
			
			CRect inRect = 	m_ScrollRect;
			inRect.right += 18;			
			InvalidateRect(inRect);

		}
		else 
		if ((point.y > listrect.top + 50) &&
			(point.y < listrect.top + 50 + 18) &&
			(point.x < listrect.right) &&
			(point.x > listrect.right-18))
		{
			//Scroll Up
			
			ScrollPosDst -= 20;
			if (ScrollPosDst < 0)
				ScrollPosDst = 0;

			//TRACE("\nScroll Up %d %d %d %d %d",ScrollPosDst,dstRange.cy,scrollbarBottom,scrollbarTop,scrollbarRange);

			ScrollBarPos = (ScrollPosDst * scrollbarRange) / dstRange.cy;
			CRect inRect = 	m_ScrollRect;
			inRect.right += 18;
			InvalidateRect(inRect);

		}
		else 
		if ((point.y > listrect.top + 50 + 18 + ScrollBarPos) &&
			(point.y < listrect.top + 50 + 18 + ScrollBarPos + 15) &&
			(point.x < listrect.right) &&
			(point.x > listrect.right-18))
		{
			
			draggingEnabled = 1;
			dragPointOffset = point.y-ScrollBarPos;
			//TRACE("\ndraggingEnabled %d %d %d",point.y,dragPointOffset,ScrollBarPos);

		}
		else 
		if ((point.y > listrect.top + 50 + 18) &&
			(point.y < listrect.bottom - 36) &&
			(point.x < listrect.right) &&
			(point.x > listrect.right-18))
		{
			
			int incr = 0;
			if (point.y > ScrollBarPos + listrect.top + 50 + 18 + 15)
			{
				incr = 10;
				//ScrollBarPos += incr;
				ScrollBarPos = point.y - listrect.top - 50 - 18 - 7;
			
			}
			else if (point.y < ScrollBarPos + listrect.top + 50 + 18)
			{
				incr = 10;
				ScrollBarPos = point.y - listrect.top - 50 - 18 - 7;
				//ScrollBarPos -= incr;

			}
			
			if (incr>0)
			{
		
				if (ScrollBarPos < 0)
					ScrollBarPos = 0;

				if (ScrollBarPos > scrollbarRange)
					ScrollBarPos = scrollbarRange;

				
				if ((dstRange.cy > 0) && (scrollbarRange > 0))
				{			
					ScrollPosDst = (ScrollBarPos * dstRange.cy) / scrollbarRange;

					CRect inRect = 	m_ScrollRect;
					inRect.right += 18;
					InvalidateRect(inRect);

					draggingEnabled = 1;
					dragPointOffset = point.y-ScrollBarPos;
				
				}

			}

		}

	}


	int numItems = 0;
	CRect ScrollRect = PrepareScrollRect();
	CheckClickableRects(ScrollRect, 0, ScrollPosDst, -1,numItems);
	int doInvalidate = DetectClickableRects(ScrollRect, 0, ScrollPosDst, -1,numItems,point);

	int itemHit = CheckContext(ScrollRect, 0, ScrollPosDst, point);
	if ((itemHit>=0) && (itemHit<=99))
	{	
		
		if (statusArray[itemHit] == ST_COMPLETED)
		{

			CDropFiles DropFiles;
			if(!PrepareFileBuff(DropFiles,mainFileArray[itemHit])){
				ASSERT(0);
			}

			COleDropSource DropSource;
			COleDataSource DropData;

			HGLOBAL hMem = ::GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, DropFiles.GetBuffSize());		
			memcpy( (char*)::GlobalLock(hMem), DropFiles.GetBuffer(), DropFiles.GetBuffSize() );
			::GlobalUnlock(hMem);

			DropData.CacheGlobalData( CF_HDROP, hMem );
			DROPEFFECT de = DropData.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE,NULL);


		}

	}

	if (doInvalidate)
		Invalidate();

	
	CView::OnLButtonDown(nFlags, point);

}

 void CSploadView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	IsScrollable = ComputeScrollRanges();
	Invalidate();
	
}




void CSploadView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (draggingEnabled) 	
		draggingEnabled = 0;
	
	CView::OnLButtonUp(nFlags, point);
}

BOOL CSploadView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	
	int incr = (zDelta*5)/WHEEL_DELTA;

	if (IsScrollable)	
	{	
		ScrollBarPos -= incr;
		
		if (ScrollBarPos < 0)
			ScrollBarPos = 0;

		if (ScrollBarPos > scrollbarRange)
			ScrollBarPos = scrollbarRange;

		
		if ((dstRange.cy > 0) && (scrollbarRange > 0))
		{			
			ScrollPosDst = (ScrollBarPos * dstRange.cy) / scrollbarRange;

			CRect inRect = 	m_ScrollRect;
			inRect.right += 18;
			InvalidateRect(inRect);
		
		}

	}
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CSploadView::OnRButtonDown(UINT nFlags, CPoint point) 
{

	CRect ScrollRect = PrepareScrollRect();
	int itemHit = CheckContext(ScrollRect, 0, ScrollPosDst, point);

	if ((itemHit>=0) && (itemHit<=99))
	{
		old_mouseMoveHit = mouseMoveHit;
		mouseMoveHit = itemHit;
		
	}
	else
	{	
		old_mouseMoveHit = mouseMoveHit;
		mouseMoveHit = -1;

	}
	Invalidate();

	if ((itemHit>=0) && (itemHit<=99))
	{
		currentRightClickActive = -1;
		CNewMenu contextmenu;	
		if (contextmenu.LoadMenu(IDR_CONTEXTMENU))
		{
			CMenu* pContext = contextmenu.GetSubMenu(0);

			if (statusArray[itemHit]!=ST_DOWNLOADINPROGRESS)			
				pContext->EnableMenuItem(ID_CONTEXTMENU_DELETE,MF_ENABLED|MF_BYCOMMAND);			
			else				
				pContext->EnableMenuItem(ID_CONTEXTMENU_DELETE,MF_GRAYED|MF_DISABLED|MF_BYCOMMAND);

			if (!downloadInProgress)
			{			
				pContext->EnableMenuItem(ID_CONTEXTMENU_MOVEUP,MF_ENABLED|MF_BYCOMMAND);
				pContext->EnableMenuItem(ID_CONTEXTMENU_MOVEDOWN,MF_ENABLED|MF_BYCOMMAND);
			}
			else
			{
				pContext->EnableMenuItem(ID_CONTEXTMENU_MOVEUP,MF_GRAYED|MF_DISABLED|MF_BYCOMMAND);
				pContext->EnableMenuItem(ID_CONTEXTMENU_MOVEDOWN,MF_GRAYED|MF_DISABLED|MF_BYCOMMAND);

			}

			if (statusArray[itemHit] == ST_COMPLETED)
			{
				pContext->EnableMenuItem(ID_CONTEXTMENU_SAVETO,MF_ENABLED|MF_BYCOMMAND);

			}
			else
				pContext->EnableMenuItem(ID_CONTEXTMENU_SAVETO,MF_GRAYED|MF_DISABLED|MF_BYCOMMAND);


			//statusArray[itemHit] = ST_ERROR;
			if (statusArray[itemHit] == ST_ERROR)
			{
				pContext->EnableMenuItem(ID_CONTEXTMENU_CLEARERROR,MF_ENABLED|MF_BYCOMMAND);
			}
			else
				pContext->EnableMenuItem(ID_CONTEXTMENU_CLEARERROR,MF_GRAYED|MF_DISABLED|MF_BYCOMMAND);
				

			currentRightClickActive = itemHit;

			POINT screenpos;
			GetCursorPos(&screenpos);
			pContext->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, screenpos.x, screenpos.y, this); 

		
		}

	}
	else
	{

		CNewMenu contextmenu;	
		if (contextmenu.LoadMenu(IDR_CONTEXTMENU_UNSEL))
		{
			CMenu* pContext = contextmenu.GetSubMenu(0);
			POINT screenpos;
			GetCursorPos(&screenpos);
			pContext->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, screenpos.x, screenpos.y, this); 

		}


	}
	
	CView::OnRButtonDown(nFlags, point);
}


//returns selected item or -1
int CSploadView::CheckContext(CRect ScrollRect, int xscrollpos, int yscrollpos,CPoint pt)
{

	int num = downloadArray.GetSize();

	if (num>100) 
		num = 100;

	if (num<=0)
		return -1;

	//Compute Full Size
	int fullSizeX = ScrollRect.Width();
	int fullSizeY = num*80;
	
	if (ScrollRect.PtInRect(pt))
	{	
		//Need Scrolling
		if (fullSizeY > ScrollRect.Height())
		{
		

		}
		else
		{

			xscrollpos = 0;
			yscrollpos = 0;

			if (pt.y-ScrollRect.top > fullSizeY)
				return -1;

		}


		
		//Convert Point to DownloadArray Canvas Coordinates
		int canvasY = pt.y - ScrollRect.top + yscrollpos;

		int itemHit = canvasY/80; //80 is height of each item


		if (itemHit < num)
			return itemHit;
		else
			return -1;

		
	}
	else 
		return -1;

	
	return 0;

}




void CSploadView::OnContextmenuDelete() 
{
	int i = currentRightClickActive;

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	if ((i>=0) && (i<num))
	{


		//int ret = MessageBox("Are you sure you want to delete this file ?","Confirm Delete",MB_YESNO | MB_ICONQUESTION);
		CString delmsg;
		CString delconfirm;
		delmsg.LoadString(IDS_DELMSG);
		delconfirm.LoadString(IDS_DELCONFIRM);
		int ret = MessageBox(delmsg,delconfirm,MB_YESNO | MB_ICONQUESTION);
		if (ret == IDYES)
		{

			mainFileArray[i].TrimLeft();
			mainFileArray[i].TrimRight();
			if (mainFileArray[i]!=_T(""))
			{	

				DeleteFile(mainFileArray[i]);
				DeleteFile(mainFileArray[i] + ".main.ini");

				CString filename,numstr;
				for (int j=0;j<8;j++)
				{
					numstr.Format("%d",j);
					filename = mainFileArray[i] + numstr;
					DeleteFile(filename);
					DeleteFile(filename+".ini");

				}
			}

			downloadArray.RemoveAt(i);
			mainFileArray.RemoveAt(i);
			statusArray.RemoveAt(i);
			totalBytesArray.RemoveAt(i);
			loadedBytesArray.RemoveAt(i);
			resumableArray.RemoveAt(i);

			Invalidate();

			return;

		}			

	}
	
}

void CSploadView::OnContextmenuMoveup() 
{
	int i = currentRightClickActive;

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	if ((i>=0) && (i<num))
	{
		if (i!=0)
		{
				CString tempStr;
				tempStr = downloadArray[i];
				downloadArray[i] = downloadArray[i-1];
				downloadArray[i-1] = tempStr;

				tempStr = mainFileArray[i];
				mainFileArray[i] = mainFileArray[i-1];
				mainFileArray[i-1] = tempStr;

				DWORD tempDW;
				tempDW = statusArray[i];
				statusArray[i] = statusArray[i-1];
				statusArray[i-1] = tempDW;

				tempDW = totalBytesArray[i];
				totalBytesArray[i] = totalBytesArray[i-1];
				totalBytesArray[i-1] = tempDW;

				tempDW = loadedBytesArray[i];
				loadedBytesArray[i] = loadedBytesArray[i-1];
				loadedBytesArray[i-1] = tempDW;		

				tempDW = resumableArray[i];
				resumableArray[i] = resumableArray[i-1];
				resumableArray[i-1] = tempDW;

				Invalidate();

		}			

	}
	
}

void CSploadView::OnContextmenuMovedown() 
{
	int i = currentRightClickActive;

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	if ((i>=0) && (i<num))
	{
		if (i!=num-1)
		{

				CString tempStr;
				tempStr = downloadArray[i];
				downloadArray[i] = downloadArray[i+1];
				downloadArray[i+1] = tempStr;

				tempStr = mainFileArray[i];
				mainFileArray[i] = mainFileArray[i+1];
				mainFileArray[i+1] = tempStr;

				DWORD tempDW;
				tempDW = statusArray[i];
				statusArray[i] = statusArray[i+1];
				statusArray[i+1] = tempDW;

				tempDW = totalBytesArray[i];
				totalBytesArray[i] = totalBytesArray[i+1];
				totalBytesArray[i+1] = tempDW;

				tempDW = loadedBytesArray[i];
				loadedBytesArray[i] = loadedBytesArray[i+1];
				loadedBytesArray[i+1] = tempDW;		

				tempDW = resumableArray[i];
				resumableArray[i] = resumableArray[i+1];
				resumableArray[i+1] = tempDW;

				Invalidate();

				
			}			

	}
	
}

void CSploadView::OnContextmenuSaveto() 
{
	int i = currentRightClickActive;

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	if ((i>=0) && (i<num))
	{
		if (statusArray[i] == ST_COMPLETED)
		{
				CString strFilter = "*.*";
				CString defaultName;
				CString extension;
				int nameable = verifyNameAble(mainFileArray[i],extension);				
				if (nameable)
				{
				}
								
				
				CFileDialog nameDlg(FALSE,_T(""),_T("*.*"),OFN_LONGNAMES,strFilter,this);	
							
				if(nameDlg.DoModal() == IDOK)
				{				
					CString nameStr = nameDlg.GetFileName();
					if (mainFileArray[i] !=nameStr)
					CopyFile(mainFileArray[i],nameStr,TRUE);
						
				}
				else
					return;
				
				return;

		}

	}
	
}

void CSploadView::OnEditCopy() 
{

}

void CSploadView::OnEditPaste() 
{
		
	(AfxGetMainWnd())->PostMessage(WM_USER_PASTECOMBO,0,0);
	
	
}

int CSploadView::restartDownloadThread(int i)
{

	while (threadRestarting)
	{
		Sleep(50);
	}

	threadRestarting = 1;

	if (threadsinfo[i]==NULL)      
	{
		threadRestarting = 0;
		return 1;

	}

	if (!(threadsinfo[i]->aborted))
	{
		threadRestarting = 0;
		return 1;

	}

	int retStatus = 0;
										

	THREADDATA* newThreadData = NULL;
	newThreadData = new THREADDATA;
	memcpy(newThreadData,threadsinfo[i],sizeof(THREADDATA));		
		
	  		
	newThreadData->parent = threadsinfo[i]->parent;
	newThreadData->startload = threadsinfo[i]->startload;
	newThreadData->endload = threadsinfo[i]->endload;
	newThreadData->pFile = NULL;
	newThreadData->dwServiceType = threadsinfo[i]->dwServiceType;	
	
	newThreadData->nPort = threadsinfo[i]->nPort;
	
	newThreadData->isDone = 0;
	newThreadData->nByteLoad = 0; 
	newThreadData->hasError = 0;
	newThreadData->errorCode = 0;
	newThreadData->threadNumber = i;
	newThreadData->timeDelay = 0;

	newThreadData->saveFile = NULL;
	newThreadData->iniFile = NULL;
	newThreadData->saveFileInUse = 0;
	newThreadData->iniFileInUse = 0;
	newThreadData->aborted = 0;
	newThreadData->safeToDel = 0;		
	
	DWORD sl,el,vp ;	
	int isResume = verifyPart(newThreadData->filename,sl,el,vp);	 //no error or completed ==> no error, partial error => error
	if ((isResume==RET_NO_ERROR) || (isResume==RET_COMPLETED))
	{
		if ((sl !=newThreadData->startload) ||
			(el != newThreadData->endload))
		{
			TRACE("\nStartLoad and Endload does not match");					
			threadRestarting = 0;			
			return 1; //No restart
			
		}

		newThreadData->nByteLoad = vp;				
	

	}			
	else
	{	
		newThreadData->nByteLoad = 0;

	}

	if (isResume==RET_COMPLETED)
	{
		
		newThreadData->isDone = 1;
		delete newThreadData;
		threadRestarting = 0;
		return 1;

	}	

	if ((isResume == RET_PARTIAL_ERROR) || (isResume==RET_NO_ERROR))
	{	
		THREADDATA* oldThreadData = threadsinfo[i];

		threadsinfo[i] = newThreadData;		
		
		StartDownloadThread(newThreadData);			

		if (oldThreadData->safeToDel)
		{
			delete oldThreadData;			
			TRACE("\nthreadsinfo[%d] deleted",i);

		}
		else
			delArray.Add(oldThreadData);

	}	

	threadRestarting = 0;


	return 0; 

}

int CSploadView::replaceThread(int i)
{
	if ((i>=0) && (i<8))	
	{

		if ((userStopPressed==0) && (downloadInProgress))
		{
			forceStop(i, threadsinfo);
			Sleep(100);
			return restartDownloadThread(i);

		}

	}

	return 1;

}

int forceStop(int i,THREADDATA* threadsinfo[])
{

	if (threadsinfo==NULL)
		return 0;

	//force stop will proceed only if WaitDownloadComplete Thread is started
	if (WaitDownloadCompleteStarted == 0)
		return 0;

	if ((i>=0) && (i<8))	
	{
		
		if (threadsinfo[i]==NULL)
			return 0;

		if (threadsinfo[i]->isDone == 0)
		{

			while ((threadsinfo[i]->saveFileInUse != 0) || (threadsinfo[i]->iniFileInUse != 0))
			{
				Sleep(100);

			}

			threadsinfo[i]->saveFileInUse = 1;
			threadsinfo[i]->iniFileInUse = 1;


			threadsinfo[i]->aborted = 1;

			if (threadsinfo[i]->saveFile)
			{
				fclose(threadsinfo[i]->saveFile);
				threadsinfo[i]->saveFile = NULL;
			}
			
			if (threadsinfo[i]->iniFile)
			{
				fclose(threadsinfo[i]->iniFile);
				threadsinfo[i]->iniFile = NULL;

			}

			

			threadsinfo[i]->saveFileInUse = 0;
			threadsinfo[i]->iniFileInUse = 0;

			TRACE("\nSuccessfully Force Close Thread %d",i);

		}
		
	}
	else if (i==-1)
	{
		for (int j=0;j<8;j++)
		{
			if (threadsinfo[j]!=NULL)
			{

				if (threadsinfo[j]->isDone == 0)
				{
					
					while ((threadsinfo[j]->saveFileInUse != 0) || (threadsinfo[j]->iniFileInUse != 0))
					{
						Sleep(100);
					}


					threadsinfo[j]->saveFileInUse = 1;
					threadsinfo[j]->iniFileInUse = 1;


					threadsinfo[j]->aborted = 1;

					if (threadsinfo[j]->saveFile)
					{
						fclose(threadsinfo[j]->saveFile);
						threadsinfo[j]->saveFile = NULL;
					}
					
					if (threadsinfo[j]->iniFile)
					{
						fclose(threadsinfo[j]->iniFile);
						threadsinfo[j]->iniFile = NULL;

					}

					if (threadsinfo[j]->pFile)
					{
						threadsinfo[j]->pFile->Abort();
					}


					threadsinfo[j]->saveFileInUse = 0;
					threadsinfo[j]->iniFileInUse = 0;
					
				}

			}


		}

	}


	return 0;

}

void CSploadView::OnButtonforcestop() 
{	
	userStopPressed = 1;
	forceStop(-1, threadsinfo);	
	
}

UINT TestConnectionThread(LPVOID lParam)
{

	ASYNCDATA* asyncData = (ASYNCDATA*) lParam; 

	if (asyncData)
	{	
		asyncData->testConnectionComplete = 0;
		
		CSploadView* parent = (CSploadView*) asyncData->parent;
		if (parent)
		{
			
			asyncData->testConnectionReturn = TestConnection(parent->myUrlString);
		}

		asyncData->testConnectionComplete = 1;

	}

	return 0;

}


int CSploadView::TestConnectionTimeout(int timeout)
{
	if (retTCResults)
	{
		if (retTCResults->testConnectionComplete == 0)
		{
			retTCResults = new ASYNCDATA;
		}

	}
	else
		retTCResults = new ASYNCDATA;

	retTCResults->testConnectionComplete = 0;
	retTCResults->testConnectionReturn = 0;
	retTCResults->parent = this;

	
	int stx = timeGetTime();
	int timeElapsed = 0;
	CWinThread* pThread = AfxBeginThread(TestConnectionThread,retTCResults);
	while ((retTCResults->testConnectionComplete==0) && (timeElapsed<timeout) && (forceAbortion==0))
	{
		Sleep(200);
		int ctx = timeGetTime();
	}

	if ((timeElapsed>timeout) || (forceAbortion))
	{
		TRACE("\nTimeOut or Abort on TestConnectAsync");

		retTCResults->testConnectionReturn = 0;

	}

	return retTCResults->testConnectionReturn;

}


UINT PrepareDataThread(LPVOID lParam)
{
	APDDATA* asyncData = (APDDATA*) lParam; 

	if (asyncData)
	{	
		asyncData->pdComplete = 0;
		
		CSploadView* parent = (CSploadView*) asyncData->parent;
		if (parent)
		{			
		
			asyncData->pdReturn = parent->PrepareData(parent->urlString,parent->mySaveDir,&(asyncData->pdForceAbortion));
		}

		asyncData->pdComplete = 1;

	}

	return 0;
}



int CSploadView::PrepareDataTimeout(int timeout)
{

	if (retPdResults)
	{
		if (retPdResults->pdComplete == 0)
		{
			
			retPdResults = new APDDATA;
		}

	}
	else
		retPdResults = new APDDATA;

	retPdResults->pdComplete = 0;
	retPdResults->pdReturn = 3;
	retPdResults->pdForceAbortion = 0;
	retPdResults->parent = this;


	int stx = timeGetTime();
	int timeElapsed = 0;
	CWinThread* pThread = AfxBeginThread(PrepareDataThread,retPdResults);
	while ((retPdResults->pdComplete==0) && (timeElapsed<timeout) && (forceAbortion==0))
	{
		Sleep(200);
		int ctx = timeGetTime();
	}

	if ((timeElapsed>timeout) || (forceAbortion))
	{
		TRACE("\nTimeOut or Abort on PrepareDataAsync");

		retPdResults->pdReturn = 3;
		
	}

	return retPdResults->pdReturn;

}

BOOL CSploadView::PrepareFileBuff(CDropFiles& DropFiles,CString filename) 
{
	
	DropFiles.AddFile(filename);
	
	DropFiles.CreateBuffer();

	return TRUE;
}

void CSploadView::OnFileStartdownload() 
{
	StartDownload();
	Invalidate();
	
	
}

void CSploadView::OnUpdateFileStartdownload(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(userStopPressed);
	
}

void CSploadView::OnFileStopdownload() 
{
	StopDownload();
	Invalidate();
	
	
}

void CSploadView::OnUpdateFileStopdownload(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(!(userStopPressed));
	
	
}

CSize GetBitmapDim(HANDLE hBitmap)
{

	CSize retSize(0,0);

	if (hBitmap)
	{
		BITMAP bitmap;
		GetObject(hBitmap, sizeof(bitmap), &bitmap);
		retSize.cx = bitmap.bmWidth;
		retSize.cy = bitmap.bmHeight;

	}

	return retSize;

}

void CSploadView::OnFileRemoveallitems() 
{

	CString delconfirm;
	delconfirm.LoadString(IDS_DELCONFIRM);
	CString msgStr;
	msgStr.LoadString(IDS_REMOVE);
	int ret = MessageBox(msgStr,delconfirm,MB_YESNO | MB_ICONQUESTION);
	if (ret == IDYES)
	{

		int num = downloadArray.GetSize();		
		
		//must decrement to delete
		for (int i=num-1;i>=0;i--)
		{
				TRACE("\ni %d num %d",i,num);
				mainFileArray[i].TrimLeft();
				mainFileArray[i].TrimRight();
				if (mainFileArray[i]!=_T(""))
				{				

					DeleteFile(mainFileArray[i]);
					DeleteFile(mainFileArray[i] + ".main.ini");

					CString filename,numstr;
					for (int j=0;j<8;j++)
					{
						numstr.Format("%d",j);
						filename = mainFileArray[i] + numstr;
						DeleteFile(filename);
						DeleteFile(filename+".ini");


					}
				}

				downloadArray.RemoveAt(i);
				mainFileArray.RemoveAt(i);
				statusArray.RemoveAt(i);
				totalBytesArray.RemoveAt(i);
				loadedBytesArray.RemoveAt(i);
				resumableArray.RemoveAt(i);

		}


		CString  folderPath;
		folderPath = GetModuleDirectory()	+ "\\Download\\*.ini";  


		if (downloadFolder=="")
		{	
			BOOL nextAvail = FALSE;
			CFileFind fileFind;
			nextAvail = fileFind.FindFile(folderPath,0);
			while (nextAvail)
			{
				nextAvail = fileFind.FindNextFile();
				CString  nextFile = fileFind.GetFilePath();
				TRACE("\nFile %s",nextFile);
				DeleteFile(nextFile);

			}	

		}
		

		Invalidate();


	}
			

	
}

void CSploadView::OnFileShowdownloadfolder() 
{

  CString  folderPath;
  folderPath = GetModuleDirectory()	+ "\\Download";  
  if (downloadFolder!="")
  {
	  folderPath = downloadFolder;
  }
  
  HINSTANCE hRun = ShellExecute (GetSafeHwnd (), _T ("open"), folderPath, NULL, NULL, SW_SHOW);	
}

void CSploadView::OnTrayRestore() 
{
	AfxGetMainWnd()->SetForegroundWindow(); 
	AfxGetMainWnd()->ShowWindow(SW_RESTORE);			
	
}


void CSploadView::OnFileAddurl() 
{

	if (urlThreadReady == 0)
		return;

	urlThreadReady = 0;
	CWinThread* pThread = AfxBeginThread(ADDURLThread,this);
	
}

UINT ADDURLThread(LPVOID pParam)
{
	if (pParam==NULL)
		return 0;
	
	CSploadView* parent = (CSploadView*) pParam;
	

	AddURLDialog aud;
	if (aud.DoModal()==IDOK)
	{
		CString url = aud.returnURL;
		parent->OnAddItem(url);

	}

	urlThreadReady = 1;

	return 0;

}


void CSploadView::OnUpdateFileRemoveallitems(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!downloadInProgress);
	
}

void CSploadView::OnHelpContents() 
{
	
  CString  folderPath;
  folderPath = GetModuleDirectory()	+ "\\Help.chm";  
  HINSTANCE hRun = ShellExecute (GetSafeHwnd (), _T ("open"), folderPath, NULL, NULL, SW_SHOW);	
	
}

void CSploadView::OnHelpConnectfusionwebsite() 
{
	// TODO: Add your command handler code here
  CString  remotePath;  
  remotePath = "http://www.connectfusion.com";
  HINSTANCE hRun = ShellExecute (GetSafeHwnd (), _T ("open"), remotePath, NULL, NULL, SW_SHOW);	
	
}

void CSploadView::OnHelpConnectfusionsource() 
{

  CString  remotePath;  
  remotePath = "http://www.connectfusion.com/source.htm";
  HINSTANCE hRun = ShellExecute (GetSafeHwnd (), _T ("open"), remotePath, NULL, NULL, SW_SHOW);	
	
}

void CSploadView::OnHelpConnectfusionfaq() 
{
  CString  remotePath;  
  remotePath = "http://www.connectfusion.com/faq.htm";
  HINSTANCE hRun = ShellExecute (GetSafeHwnd (), _T ("open"), remotePath, NULL, NULL, SW_SHOW);	
	
}

void CSploadView::OnFileStartdownload1() 
{
	StartDownload();
	Invalidate();
	
}

void CSploadView::OnFileStopdownload1() 
{
	StopDownload();
	Invalidate();
	
}

BOOL CSploadView::PreTranslateMessage(MSG* pMsg) 
{

	if (m_tooltipCreated)
		m_tooltip.RelayEvent(pMsg);
	
	return CView::PreTranslateMessage(pMsg);
}



void CSploadView::ShowMessage(CString msg, CString msgTitle, CPoint ptIcon) 
{		


	CString str = _T("<table><tr><td><right></right><br><center><font color=black size=5><B>" + msgTitle +"<br><hr color=gray></center><br>");
	CString str1;
	str1 = msg;
	str += "</B></font><BR><font size=4>" + str1 + "  <BR></font>";
	
	str += _T("</td></tr></table>");
	m_tooltip.ShowHelpTooltip(&ptIcon, str);

}


void CSploadView::ShowMessage(CString msg, CString msgTitle) 
{		

	CPoint ptIcon;
	GetCursorPos(&ptIcon);


	ShowMessage(msg,  msgTitle, ptIcon) ;


}


void CSploadView::OnFileSetdownloadfolder() 
{
	LPCTSTR lpszTitle = _T( "Select the default folder for download:" );
	UINT	uFlags	  = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;	
	
	CString m_strRoot("");
	
	if (downloadFolder == "")
	{
		m_strRoot = GetModuleDirectory() + "\\Download";
	}
	else	
		m_strRoot = downloadFolder;


	TRACE("m_strRoot %s",m_strRoot);
	

	CFolderDialog dlgRoot( lpszTitle, m_strRoot, this, uFlags );	
	//dlgRoot.SetRootFolder( m_strRoot );	

	if( dlgRoot.DoModal() == IDOK )
	{
		m_strRoot = dlgRoot.GetFolderPath();				
		downloadFolder = m_strRoot;
		TRACE("new m_strRoot %s",m_strRoot);

		CString tstr;
		CString mstr;
		CString ostr;
		mstr.LoadString(IDS_DEFAULTPATH);
		ostr.Format(mstr,downloadFolder);
		tstr.LoadString(IDS_NOTE);
		ShowMessage(ostr,tstr);
	}
	
	
}

void CSploadView::OnContextmenuClearerror()
{
	
	int i = currentRightClickActive;

	int num = downloadArray.GetSize();
	if (num>100) 
		num = 100;

	if (num<0)
		num = 0;

	if ((i>=0) && (i<num))
	{
		if (statusArray[i]==ST_ERROR)
			statusArray[i]=ST_PENDING;

		Invalidate();

	}

}
