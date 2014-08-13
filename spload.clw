; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSploadView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "spload.h"
LastPage=0

ClassCount=6
Class1=CSploadApp
Class2=CSploadDoc
Class3=CSploadView
Class4=CMainFrame

ResourceCount=14
Resource1=IDR_MAINFRAME7
Class5=CAboutDlg
Resource2=IDR_CONTEXTMENU_UNSEL
Resource3=IDR_CONTEXTMENU
Resource4=IDR_MAINFRAME1
Resource5=IDR_TRAYMENU
Resource6=IDD_DIALOG1
Class6=AddURLDialog
Resource7=IDR_MAINFRAME5
Resource8=IDR_TOOLBAR1
Resource9=IDR_MAINFRAME4
Resource10=IDR_MAINFRAME8
Resource11=IDR_MAINFRAME
Resource12=IDR_MAINFRAME6
Resource13=IDD_ABOUTBOX
Resource14=IDD_URLDIALOG

[CLS:CSploadApp]
Type=0
HeaderFile=spload.h
ImplementationFile=spload.cpp
Filter=N
LastObject=CSploadApp
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CSploadDoc]
Type=0
HeaderFile=sploadDoc.h
ImplementationFile=sploadDoc.cpp
Filter=N

[CLS:CSploadView]
Type=0
HeaderFile=sploadView.h
ImplementationFile=sploadView.cpp
Filter=C
BaseClass=CView
VirtualFilter=VWC
LastObject=CSploadView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=W
LastObject=ID_FILE_SETDOWNLOADFOLDER
BaseClass=CFrameWnd
VirtualFilter=fWC




[CLS:CAboutDlg]
Type=0
HeaderFile=spload.cpp
ImplementationFile=spload.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,button,1342177287

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_STARTDOWNLOAD
Command2=ID_FILE_STOPDOWNLOAD
Command3=ID_FILE_ADDURL
Command4=ID_FILE_REMOVEALLITEMS
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_FILE_SETDOWNLOADFOLDER
Command7=ID_APP_EXIT
Command8=ID_VIEW_TOOLBAR
Command9=ID_VIEW_ADDRESSBAR
Command10=ID_HELP_CONTENTS
Command11=ID_HELP_CONNECTFUSIONWEBSITE
Command12=ID_HELP_CONNECTFUSIONFAQ
Command13=ID_HELP_CONNECTFUSIONSOURCE
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[MNU:IDR_CONTEXTMENU]
Type=1
Class=CSploadView
Command1=ID_CONTEXTMENU_DELETE
Command2=ID_CONTEXTMENU_MOVEUP
Command3=ID_CONTEXTMENU_MOVEDOWN
Command4=ID_CONTEXTMENU_SAVETO
Command5=ID_FILE_ADDURL
Command6=ID_FILE_SHOWDOWNLOADFOLDER
CommandCount=6

[MNU:IDR_MAINFRAME1]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_SEND_MAIL
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_APP_ABOUT
CommandCount=17

[MNU:IDR_TRAYMENU]
Type=1
Class=CSploadView
Command1=ID_TRAY_RESTORE
Command2=ID_APP_ABOUT
Command3=ID_FILE_SHOWDOWNLOADFOLDER
Command4=ID_FILE_ADDURL
Command5=ID_APP_EXIT
CommandCount=5

[DLG:IDD_DIALOG1]
Type=1
Class=?
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,button,1342177287

[DLG:IDD_URLDIALOG]
Type=1
Class=AddURLDialog
ControlCount=6
Control1=IDC_EDIT1,edit,1350631552
Control2=IDOK,button,1342242816
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,button,1342177287
Control6=IDC_STATIC,static,1342177294

[CLS:AddURLDialog]
Type=0
HeaderFile=AddURLDialog.h
ImplementationFile=AddURLDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=AddURLDialog
VirtualFilter=dWC

[MNU:IDR_CONTEXTMENU_UNSEL]
Type=1
Class=?
Command1=ID_FILE_ADDURL
Command2=ID_FILE_SHOWDOWNLOADFOLDER
CommandCount=2

[TB:IDR_TOOLBAR1]
Type=1
Class=CMainFrame
Command1=ID_BUTTONGO
CommandCount=1

[TB:IDR_MAINFRAME5]
Type=1
Class=?
Command1=ID_FILE_ADDURL
Command2=ID_EDIT_PASTE
Command3=ID_FILE_STARTDOWNLOAD1
Command4=ID_FILE_STOPDOWNLOAD1
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_APP_ABOUT
CommandCount=6

[TB:IDR_MAINFRAME6]
Type=1
Class=?
Command1=ID_FILE_ADDURL
Command2=ID_EDIT_PASTE
Command3=ID_FILE_STARTDOWNLOAD1
Command4=ID_FILE_STOPDOWNLOAD1
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_APP_ABOUT
CommandCount=6

[TB:IDR_MAINFRAME8]
Type=1
Command1=ID_FILE_ADDURL
Command2=ID_EDIT_PASTE
Command3=ID_FILE_STARTDOWNLOAD1
Command4=ID_FILE_STOPDOWNLOAD1
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_APP_ABOUT
CommandCount=6

[TB:IDR_MAINFRAME4]
Type=1
Class=?
Command1=ID_FILE_ADDURL
Command2=ID_EDIT_PASTE
Command3=ID_FILE_STARTDOWNLOAD1
Command4=ID_FILE_STOPDOWNLOAD1
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_APP_ABOUT
CommandCount=6

[TB:IDR_MAINFRAME7]
Type=1
Class=?
Command1=ID_FILE_ADDURL
Command2=ID_EDIT_PASTE
Command3=ID_FILE_STARTDOWNLOAD1
Command4=ID_FILE_STOPDOWNLOAD1
Command5=ID_FILE_SHOWDOWNLOADFOLDER
Command6=ID_APP_ABOUT
CommandCount=6

