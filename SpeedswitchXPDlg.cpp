/*
   SpeedswitchXP V1.5
   - Windows XP CPU Frequency Control for Notebooks -

   Copyright(c) 2002-2006 Christian Diefer
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as 
   published by the Free Software Foundation.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define _UNICODE

#include "stdafx.h"
#include "AtlBase.h"
#include "TOptions.h"
#include "SpeedswitchXP.h"
#include "SpeedswitchXPDlg.h"
#include <wtsapi32.h>
#include <stdarg.h>
#include "speedswitch.h"
#include "SpeedswitchXPOptions.h"
#include "Hyperlink.h"
#include "cpuid.h"
#include "speed.h"
#include "CPUData.h"
#include "PowerCapabilities.h"
#include "SystemInfo.h"
#include "htspeed.h"
#include <time.h>
#include "TrayIconManager.h"
#include "CPUSpeed.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDM_MAXPERF  300
#define IDM_BATTOPT  301
#define IDM_MAXBATT  302
#define IDM_DYNAMIC  303
#define IDM_SHOWMAIN 304
#define IDM_SHUTDOWN 305

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage( _T("TaskbarCreated") );

TOptions options;
static UINT timerID = 1;

static TTrayIconManager trayMan;

HWND glb_hWnd;

// CAboutDlg-Dialogfeld für Anwendungsbefehl 'Info'

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialogfelddaten
	enum { IDD = IDD_ABOUTBOX };
	CHyperLink	m_gplLink;
	CHyperLink	m_homeLink;
	CHyperLink	m_faqLink;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
  virtual BOOL OnInitDialog();

// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{ }

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_GPLLINK, m_gplLink);
  DDX_Control(pDX, IDC_URLTEXT1, m_homeLink);
  DDX_Control(pDX, IDC_URLTEXT4, m_faqLink);
}

//////////////////
// Initialize dialog: subclass static controls
BOOL CAboutDlg::OnInitDialog() 
{
  m_gplLink.SetURL( _T("http://www.fsf.org/licenses/gpl.txt") );
  m_gplLink.SetUnderline( TRUE );

  m_homeLink.SetURL( _T("http://www.diefer.de/speedswitchxp/index.html") );
  m_homeLink.SetUnderline( TRUE );

  m_faqLink.SetURL( _T("http://www.diefer.de/speedswitchxp/faq.html") );
  m_faqLink.SetUnderline( TRUE );

  return CDialog::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CSpeedswitchXPDlg Dialogfeld


//**************************************************************
// Constructor for main class
//**************************************************************
CSpeedswitchXPDlg::CSpeedswitchXPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpeedswitchXPDlg::IDD, pParent)
  , m_iACState(0)
  , m_iDCState(0)
  , m_szCPULoad(_T("---"))
  , m_szCPUSpeed(_T("---"))
  , m_szMaxCPUSpeed(_T(""))
{
  getRegKeys();   // read configuration from registry

  diagramOffset = 0;

  if( options.debugMode )
    DeleteFile( _T("SpeedswitchXP.log") );

  log( _T("Constructor: Starting") );
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  iBattOpt = AfxGetApp()->LoadIcon(IDR_BATTOPT);
  iMaxPerf = AfxGetApp()->LoadIcon(IDR_MAXPERF);
  iMaxBatt = AfxGetApp()->LoadIcon(IDR_MAXBATT);
  iDynamic = AfxGetApp()->LoadIcon(IDR_DYNAMIC);
  iTrayBattery = AfxGetApp()->LoadIcon(IDI_TRAYBATT);
  iBattCharge = AfxGetApp()->LoadIcon(IDI_CHARGEICON);

  cpuDataIcon = NULL;
  loadIndex = 0;
  speedIndex = 0;
  tickCounterCPU = 0;
  tickCounterCheck = 0;
  batteryIconActive = FALSE;
  batteryIconCharge = FALSE;
  batteryValue = -2;
  cpuDataIconActive = FALSE;
  cpuIconVal1 = -2;
  cpuIconVal2 = -2;
  curSpeed = 0;
  curLoad = 0;
  forceShutdown = false;

  for( int i=0; i<1200; i++ )   // init history arrays
    loadArr[i] = speedArr[i] = 0;

  trayIcon = FALSE;

  log( _T("Constructor: Creating dialog") );
  Create( CSpeedswitchXPDlg::IDD, pParent );

  // compute diagram offsets
  ((CWnd*)GetDlgItem(IDC_CPUFRAME))->GetWindowRect( &cpuFrame );
  ((CWnd*)GetDlgItem(IDC_LEGENDFRAME))->GetWindowRect( &legendFrame );
  ScreenToClient( &cpuFrame );
  ScreenToClient( &legendFrame );
  cpuFrame.DeflateRect( 1, 1 );
  legendFrame.DeflateRect( 1, 1 );

  log( _T("Constructor: Complete") );
}

void CSpeedswitchXPDlg::initStrings()
{
  CString s1;

  legend1.LoadStringW( IDS_MAIN21 );
  legend2.LoadStringW( IDS_MAIN22 );
  err.LoadStringW( IDS_MAINERR2 );
}

void CSpeedswitchXPDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_COMBO1, m_iACState);
  DDX_CBIndex(pDX, IDC_COMBO2, m_iDCState);
  DDX_Text(pDX, IDC_EDIT1, m_szCPULoad);
  DDX_Text(pDX, IDC_EDIT2, m_szCPUSpeed);
  DDX_Text(pDX, IDC_EDIT3, m_szMaxCPUSpeed);
  DDX_Control(pDX, IDC_COMBO3, m_cDisplayAC);
  DDX_Control(pDX, IDC_COMBO7, m_cDisplayDC);
  DDX_Control(pDX, IDC_COMBO4, m_cDiskAC);
  DDX_Control(pDX, IDC_COMBO8, m_cDiskDC);
  DDX_Control(pDX, IDC_COMBO5, m_cStandbyAC);
  DDX_Control(pDX, IDC_COMBO9, m_cStandbyDC);
  DDX_Control(pDX, IDC_COMBO6, m_cHibernationAC);
  DDX_Control(pDX, IDC_COMBO10, m_cHibernationDC);
}

BEGIN_MESSAGE_MAP(CSpeedswitchXPDlg, CDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_ICONNOTIFY, OnIconNotify)
  ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadCast)
  ON_BN_CLICKED(IDC_CLOSE, OnBnClickedClose)
  ON_BN_CLICKED(IDC_ABOUT, OnAbout)
  ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
  ON_CBN_SELCHANGE(IDC_COMBO2, OnCbnSelchangeCombo2)
  ON_BN_CLICKED(IDC_OPTBUTTON, OnBnClickedOptbutton)
  ON_COMMAND(IDM_MAXPERF, OnMaxPerfMenu)
  ON_COMMAND(IDM_BATTOPT, OnBattOptMenu)
  ON_COMMAND(IDM_MAXBATT, OnMaxBattMenu)
  ON_COMMAND(IDM_DYNAMIC, OnDynamicMenu)
  ON_COMMAND(IDM_SHOWMAIN, OnShowMainMenu)
  ON_COMMAND(IDM_SHUTDOWN, OnSelectShutdown)
  ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
  ON_WM_SETCURSOR()
  ON_CBN_SELCHANGE(IDC_COMBO3, OnCbnSelchangeDisplayAC)
  ON_CBN_SELCHANGE(IDC_COMBO7, OnCbnSelchangeDisplayDC)
  ON_CBN_SELCHANGE(IDC_COMBO4, OnCbnSelchangeDiskAC)
  ON_CBN_SELCHANGE(IDC_COMBO8, OnCbnSelchangeDiskDC)
  ON_CBN_SELCHANGE(IDC_COMBO5, OnCbnSelchangeStandbyAC)
  ON_CBN_SELCHANGE(IDC_COMBO9, OnCbnSelchangeStandbyDC)
  ON_CBN_SELCHANGE(IDC_COMBO6, OnCbnSelchangeHibAC)
  ON_CBN_SELCHANGE(IDC_COMBO10, OnCbnSelchangeHibDC)
  ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedCPUData)
  ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, OnTaskBarCreated)
  ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
END_MESSAGE_MAP()


// CSpeedswitchXPDlg Meldungshandler

//**************************************************************
// redisplay tray icon(s)
//**************************************************************
LRESULT CSpeedswitchXPDlg::OnTaskBarCreated( WPARAM wp, LPARAM lp )
{
  displayTrayIcon( NIM_ADD );

  if( batteryIconActive )
  {
    batteryIconActive = batteryIconCharge = FALSE;
    batteryDisplay( NULL );
  }

  if( cpuDataIconActive )
    displayCPUDataIcon( NIM_ADD );

  return 0;
}

//**************************************************************
// initialize dialog window
//**************************************************************
BOOL CSpeedswitchXPDlg::OnInitDialog()
{
  log( _T("InitDialog: Start") );
  CDialog::OnInitDialog();

  initStrings();

  fillHibernationTimes( &m_cDisplayAC, NULL, -1, 0 );
  fillHibernationTimes( &m_cDisplayDC, NULL, -1, 0 );
  fillHibernationTimes( &m_cDiskAC, NULL, -1, 1 );
  fillHibernationTimes( &m_cDiskDC, NULL, -1, 1 );
  fillHibernationTimes( &m_cStandbyAC, NULL, -1, 2 );
  fillHibernationTimes( &m_cStandbyDC, NULL, -1, 2 );

  fillPolicyStrings( (CComboBox*)GetDlgItem(IDC_COMBO1) );
  fillPolicyStrings( (CComboBox*)GetDlgItem(IDC_COMBO2) );

	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon( m_hIcon, TRUE );			// Großes Symbol verwenden
	SetIcon( m_hIcon, FALSE );		// Kleines Symbol verwenden

  glb_hWnd = m_hWnd;

	// TODO: Hier zusätzliche Initialisierung einfügen
	OSVERSIONINFO ovi;

	ovi.dwOSVersionInfoSize = sizeof( ovi );

	if( GetVersionEx(&ovi) == 0 )
		return FALSE;		// this should never happen !

  log( _T("Running on %s Version %d.%d Build %d (%s)"),
       ovi.dwPlatformId==VER_PLATFORM_WIN32_NT 
           ? _T("Windows NT")
           : (ovi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS
           ? _T("Windows 9x") : _T("Windows 3.1 (?)")),
       ovi.dwMajorVersion,
       ovi.dwMinorVersion,
       ovi.dwBuildNumber,
       ovi.szCSDVersion );

	// not running on XP or higher => terminate
	if( ovi.dwPlatformId!=VER_PLATFORM_WIN32_NT || ovi.dwBuildNumber<2600 )
  {
    log( _T("/aError") );
    CString s1;
    s1.LoadStringW( IDS_MAINERR1 );
    MessageBox( s1, err, MB_OK|MB_ICONEXCLAMATION ); 
    forceShutdown = true;
    PostMessage( WM_CLOSE );
    return FALSE;
  }

  if( !initPowerValues() )    // set box contents on main window
    return FALSE;

  UpdateData( FALSE );

  log( _T("InitDialog: Complete") );

	return TRUE;
}

//**************************************************************
// read keys from registry
//**************************************************************
void CSpeedswitchXPDlg::getRegKeys()
{
  if( !options.getRegKeys() )
  {
    CString s1;
    s1.LoadStringW( IDS_MAINERR3 );
    MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
  }
}

//**************************************************************
// write keys to registry
//**************************************************************
void CSpeedswitchXPDlg::setRegKeys()
{
  if( !options.setRegKeys() )
  {
    CString s1;
    s1.LoadStringW( IDS_MAINERR4 );
    MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
  }
}

//**************************************************************
// evaluate command line options (used here to indicate program 
// start, thus displaying the tray icon)
//**************************************************************
void CSpeedswitchXPDlg::setCmdLine( TCHAR* cl )
{
  log( _T("setCmdLine: Start") );
  displayTrayIcon( NIM_ADD );
  trayIcon = TRUE;
  minimizeToTray();
  log( _T("setCmdLine: Complete") );
}

//**************************************************************
// evaluate commands from the system menu
//**************************************************************
void CSpeedswitchXPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CSpeedswitchXPDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc( this ); // Gerätekontext zum Zeichnen

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0 );

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
  	drawDiagram();
		CDialog::OnPaint();
	}
}

// Das System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CSpeedswitchXPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//**************************************************************
// process events from the tray icon
//**************************************************************
LRESULT CSpeedswitchXPDlg::OnIconNotify( WPARAM w, LPARAM l )
{
  switch( l )
  {
    case WM_LBUTTONUP:
          ShowWindow( SW_SHOW );
          SetForegroundWindow();
          break;

    case WM_RBUTTONUP:
          {
            CMenu menu;
            menu.CreatePopupMenu();

            int cur = options.ac ? acThrottle : dcThrottle;

            for( int i=0; i<4; i++ )
              menu.AppendMenuW( MF_STRING | (i==cur ? MF_CHECKED : 0),
                               IDM_MAXPERF+i, 
                               throttleString(i) );

            menu.AppendMenu( MF_SEPARATOR );
            CString s1;
            s1.LoadStringW( IDS_MAIN23 );
            menu.AppendMenu( MF_STRING, IDM_SHOWMAIN, s1 );
            SetMenuDefaultItem( menu, IDM_SHOWMAIN, FALSE );
            menu.AppendMenu( MF_SEPARATOR );
            s1.LoadStringW( IDS_MAIN24 );
            menu.AppendMenu( MF_STRING, IDM_SHUTDOWN, s1 );

            POINT Point;
            GetCursorPos( &Point );
            SetForegroundWindow();

            menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_NONOTIFY,
                                 Point.x,
                                 Point.y,
                                 this );
            menu.Detach();
          }
          break;
  }

  return 0;
}

//**************************************************************
// enter autostart string in the user's autostart registry area
//**************************************************************
void CSpeedswitchXPDlg::setAutoStartRegistry()
{
  LONG lReturn = 0;
  TCHAR path[264];
  CRegKey key;

  if( GetModuleFileName(NULL,path,255) == 0 )
  {
    CString s1;
    s1.LoadStringW( IDS_MAINERR5 );
    MessageBox( s1, err, MB_OK|MB_ICONEXCLAMATION );
    return;
  }

  lReturn = key.Create( HKEY_CURRENT_USER,
                        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\") );

  if( lReturn == ERROR_SUCCESS )
  {
    if( key.SetValue(_T("SpeedswitchXP"),REG_SZ,path,(ULONG)((_tcslen(path)+1)*2)) != ERROR_SUCCESS )
    {
      CString s1;
      s1.LoadStringW( IDS_MAINERR6 );
      MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
      key.Close();
      return;
    }
  }
  else
  {
    CString s1;
    s1.LoadStringW( IDS_MAINERR6 );
    MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
  }

  key.Close();
}

//**************************************************************
// ... and delete it if the user asks for it
//**************************************************************
void CSpeedswitchXPDlg::deleteAutoStartRegistry()
{
  CRegKey key;
  LONG result = key.Open( HKEY_CURRENT_USER,
                          _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\") );

  if( result == ERROR_SUCCESS )
    key.DeleteValue( _T("SpeedswitchXP") );
  else
  {
    CString s1;
    s1.LoadStringW( IDS_MAINERR7 );
    MessageBox( s1, err, MB_ICONERROR );
  }

  key.Close();
}

//**************************************************************
// last method to be called during this class' lifetime
//**************************************************************
void CSpeedswitchXPDlg::PostNcDestroy()
{
	delete this;
}

//**************************************************************
// show the 'About' dialog
//**************************************************************
void CSpeedswitchXPDlg::OnAbout() 
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();	
}

//**************************************************************
// minimize to tray
//**************************************************************
void CSpeedswitchXPDlg::minimizeToTray()
{
  ShowWindow( SW_HIDE );
}

//**************************************************************
// handler for 'Close' button
//**************************************************************
void CSpeedswitchXPDlg::OnBnClickedClose()
{
  if( options.minimizeOnClose )
  {
    minimizeToTray();
    return;
  }

  OnClose();
}

//**************************************************************
// process the Close request
//**************************************************************
void CSpeedswitchXPDlg::OnClose()
{
  if( options.minimizeOnClose && !forceShutdown )
  {
    minimizeToTray();
    return;
  }

  forceShutdown = false;

  log( _T("--------------------------------") );
  log( _T("Begin termination") );

  if( closeQuery() )
  {
    setRegKeys();

    if( activeTimer )
    {
      log( _T("/bKilling timer (%d)... "), timerID );
      if( KillTimer(timerID) == 0 )
        log( _T("/aError") );
      else
        log( _T("/aOk") );
      tickCounterCPU = 0;
      tickCounterCheck = 0;
    }

    log( _T("/bClosing main window... ") );
    DestroyWindow();
    log( _T("/aOk") );

    if( options.reactivate )
    {
      log( _T("/bRestoring previous power scheme... ") );
      SetActivePwrScheme( options.originalScheme, NULL, NULL );
      log( _T("/aOk") );
    }
  }
}

//**************************************************************
// CloseQuery taken from I8kfanGUI; only used here to remove the
// tray icon
//**************************************************************
bool CSpeedswitchXPDlg::closeQuery()
{
  if( trayIcon )
  {
    log( _T("removing tray icon...") );
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof( NOTIFYICONDATA );
    nid.hWnd = m_hWnd;
    nid.uID = 700;
    nid.uCallbackMessage = WM_ICONNOTIFY;
    nid.hIcon = curIcon;
    Shell_NotifyIcon( NIM_DELETE, &nid );
  }

  if( batteryIconActive )
    removeBatteryIcon();

  if( cpuDataIconActive )
    removeCPUDataIcon();

  return true;
}

//**************************************************************
// display/modify/remove the tray icon
//**************************************************************
void CSpeedswitchXPDlg::displayTrayIcon( DWORD msg )
{
  NOTIFYICONDATA nid;
  static TCHAR tp[128];

  nid.cbSize = sizeof( NOTIFYICONDATA );
  nid.hWnd = m_hWnd;
  nid.uID = 700;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = WM_ICONNOTIFY;

  CString s1;
  s1.LoadStringW( IDS_MAIN25 );
  wsprintf( tp, s1,
                throttleString(acThrottle),
                options.ac ? _T(" <-") : _T(""),
                throttleString(dcThrottle),
                options.ac ? _T("") : _T(" <-") );
  
  if( options.readCPUSpeed )
  {
    CString s1;
    s1.LoadStringW( IDS_MAIN26 );
    wsprintf( &tp[_tcslen(tp)],
                s1,
                m_szCPUSpeed );
  }

  if( options.readCPULoad )
  {
    CString s1;
    s1.LoadStringW( IDS_MAIN27 );
    wsprintf( &tp[_tcslen(tp)],
                s1,
                m_szCPULoad );
  }

  lstrcpy( nid.szTip, tp );
  nid.hIcon = curIcon;

  Shell_NotifyIcon( msg, &nid );
}

//**************************************************************
// select the 'flag' icon corresponding to the current CPU policy
//**************************************************************
void CSpeedswitchXPDlg::setCPUIcon( int policy )
{
  switch( policy )
  {
    case PO_THROTTLE_NONE:      curIcon = iMaxPerf;
                                break;

    case PO_THROTTLE_ADAPTIVE:  curIcon = iDynamic;
                                break;
        
    case PO_THROTTLE_CONSTANT:  curIcon = iBattOpt;
                                break;

    case PO_THROTTLE_DEGRADE:   curIcon = iMaxBatt;
                                break;
  }
}

//**************************************************************
// process AC policy change by user
//**************************************************************
void CSpeedswitchXPDlg::OnCbnSelchangeCombo1()
{
  int i = acThrottle;

  UpdateData();
//  if( i != m_iACState )
  {
    log( _T("changing AC from %d to %d"), i, m_iACState );

    // Workaround for high load situations when switching from batt.opt/max.batt to
    // dynamic switching: the CPU speed stays at lower speed for duration of high
    // load 
    // => take a short step to max.performance to reset XP's internal idle timer
    if( options.ac )   // really on battery ?
    {
      if( i!=PO_THROTTLE_NONE && m_iACState==PO_THROTTLE_ADAPTIVE )
        setState( TRUE, PO_THROTTLE_NONE );    // take a short step to 'Max.Performance'
    }

    if( !setState(TRUE,m_iACState) )
    {
      log( _T("*** change failed ***") );
      m_iACState = i;
      UpdateData( FALSE );
      CString s1;
      s1.LoadStringW( IDS_MAINERR8 );
      MessageBox( s1, err, MB_OK|MB_ICONEXCLAMATION );
      return;
    }

    if( options.ac )
      setCPUIcon( m_iACState );

    displayTrayIcon( NIM_MODIFY );
  }
}

//**************************************************************
// process DC policy change by user
//**************************************************************
void CSpeedswitchXPDlg::OnCbnSelchangeCombo2()
{
  int i = dcThrottle;

  UpdateData();
//  if( i != m_iDCState )
  {
    log( _T("changing DC from %d to %d"), i, m_iDCState );

    // Workaround for high load situations when switching from batt.opt/max.batt to
    // dynamic switching: the CPU speed stays at lower speed for duration of high
    // load 
    // => take a short step to max.performance to reset XP's internal idle timer
    if( !options.ac )   // really on battery ?
    {
      if( i!=PO_THROTTLE_NONE && m_iDCState==PO_THROTTLE_ADAPTIVE )
        setState( FALSE, PO_THROTTLE_NONE );    // take a short step to 'Max.Performance'
    }

    if( !setState(FALSE,m_iDCState) )
    {
      log( _T("*** change failed ***") );
      m_iDCState = i;
      UpdateData( FALSE );
      CString s1;
      s1.LoadStringW( IDS_MAINERR8 );
      MessageBox( s1, err, MB_OK|MB_ICONEXCLAMATION );
      return;
    }

    if( !options.ac )
      setCPUIcon( m_iDCState );

    displayTrayIcon( NIM_MODIFY );
  }
}

//**************************************************************
// process power status change event from system
//**************************************************************
LRESULT CSpeedswitchXPDlg::OnPowerBroadCast( WPARAM wp, LPARAM lp )
{
  if( wp == PBT_APMPOWERSTATUSCHANGE )
  {
    log( _T("Powerstatus change detected !") );
    if( GetSystemPowerStatus(&pwrStatus) )
    {
      BOOL x = (pwrStatus.ACLineStatus!=0);
   
      if( x != options.ac )
      {
        log( _T("change from %s to %s"), options.ac?_T("AC"):_T("DC"), x?_T("AC"):_T("DC") );
        options.ac = x;
        setCPUIcon( options.ac ? acThrottle : dcThrottle );
        displayTrayIcon( NIM_MODIFY );

        if( options.showBattery )
          batteryDisplay( &pwrStatus );
      }
    }
  }
  else if( wp == PBT_APMRESUMESUSPEND )
  {
    log( _T("Returning from standby/hibernation !") );

    // switch to max performance for a short time and then back:
    int aktuell = options.ac ? acThrottle : dcThrottle;

    setState( options.ac, PO_THROTTLE_NONE );
    Sleep( 250 );                       // wait 1/2 second
    setState( options.ac, aktuell );    // return to previous setting
  }
  else if( wp == PBT_APMRESUMESTANDBY )
  {
    log( _T("Returning from standby !") );

    // switch to max performance for a short time and then back:
    int aktuell = options.ac ? acThrottle : dcThrottle;

    setState( options.ac, PO_THROTTLE_NONE );
    Sleep( 250 );                       // wait 1/2 second
    setState( options.ac, aktuell );    // return to previous setting
  }
  
  return 0;
}

//**************************************************************
// handler for 'Options' button
//**************************************************************
void CSpeedswitchXPDlg::OnBnClickedOptbutton()
{
  CSpeedswitchXPOptions dlg;

  dlg.setVars( options );

	if( dlg.DoModal() == IDOK )
  {
    dlg.getVars( options );

    if( dlg.power || dlg.cpu )
    {
      log( _T("writing new power policies:%d %d"), dlg.power, dlg.cpu );
      writePolicies( dlg.power, dlg.cpu );
    }

    setRegKeys();

    if( options.autoStart )
      setAutoStartRegistry();
    else
      deleteAutoStartRegistry();

    if( !options.readCPUSpeed )
      m_szCPUSpeed = _T("---");

    if( !options.readCPULoad )
      m_szCPULoad = _T("---");

    if( activeTimer 
    && !(options.checkStatus || options.readCPUSpeed || options.readCPULoad || options.showBattery) )
    {
      log( _T("Killing existing timer") );
      KillTimer( timerID );
      activeTimer = FALSE;
    }

    if( !activeTimer 
    && (options.checkStatus || options.readCPUSpeed || options.readCPULoad || options.showBattery) )
    {
      log( _T("Creating new timer") );
      SetTimer( timerID, 1000, NULL );
      activeTimer = TRUE;
    }

    displayTrayIcon( NIM_MODIFY );

    if( !GetSystemPowerStatus(&pwrStatus) )
    {
      CString s1;
      s1.LoadStringW( IDS_MAINERR9 );
      MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
    }
    else
    {
      // check battery options for changes:
      // 1. battery option off & icon shown => remove icon
      if( !options.showBattery && batteryIconActive )
        removeBatteryIcon();
      // 2. battery option on => show/update/remove icon
      else if( options.showBattery )
        batteryDisplay( &pwrStatus );
    }

    if( !options.speedIcon && !options.loadIcon && cpuDataIconActive )
      removeCPUDataIcon();
    else if( options.speedIcon || options.loadIcon )
      displayCPUDataIcon( cpuDataIconActive ? NIM_MODIFY : NIM_ADD );

    initPowerBoxes();
         
    UpdateData( FALSE );
  }
}


//**************************************************************
// handlers for tray icon menu
//**************************************************************
void CSpeedswitchXPDlg::OnMaxPerfMenu( void )
{ MenuSpeedHandler(PO_THROTTLE_NONE); }

void CSpeedswitchXPDlg::OnBattOptMenu( void )
{ MenuSpeedHandler(PO_THROTTLE_CONSTANT); }

void CSpeedswitchXPDlg::OnMaxBattMenu( void )
{ MenuSpeedHandler(PO_THROTTLE_DEGRADE); }

void CSpeedswitchXPDlg::OnDynamicMenu( void )
{ MenuSpeedHandler(PO_THROTTLE_ADAPTIVE); }

void CSpeedswitchXPDlg::OnShowMainMenu( void )
{ ShowWindow(SW_SHOW); }

void CSpeedswitchXPDlg::OnSelectShutdown( void )
{
  forceShutdown = true;
  PostMessage( WM_CLOSE );
}


void CSpeedswitchXPDlg::MenuSpeedHandler( int throttle )
{
  if( options.ac )
  {
    UpdateData();
    int x = m_iACState;
    m_iACState = throttle;
    UpdateData( FALSE );
    m_iACState = x;
    OnCbnSelchangeCombo1();
  }
  else
  {
    UpdateData();
    int x = m_iDCState;
    m_iDCState = throttle;
    UpdateData( FALSE );
    m_iDCState = x;
    OnCbnSelchangeCombo2();
  }
}

//**************************************************************
// perform user requested scrolling in the history diagram
//**************************************************************
void CSpeedswitchXPDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
  if( pScrollBar != ((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1)) )
    return;

  int curpos = pScrollBar->GetScrollPos();
  int newpos = curpos;

  switch( nSBCode )
  {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
           newpos = nPos;
           break;

    case SB_LEFT:
           newpos = 0;
           break;

    case SB_RIGHT:
           newpos = 90;
           break;

    case SB_LINELEFT:
    case SB_PAGELEFT:
           if( curpos > 0 )
             newpos = curpos-1;
           break;

    case SB_LINERIGHT:
    case SB_PAGERIGHT:
           if( curpos < 90 )
             newpos = curpos+1;
           break;

    case SB_ENDSCROLL:
           break;
  }

  if( newpos != curpos )
  {
    pScrollBar->SetScrollPos( newpos, TRUE );
    diagramOffset = (90-newpos) * 12;
    InvalidateRect( &cpuFrame, FALSE );
  }

  CDialog::OnHScroll( nSBCode, nPos, pScrollBar );
}

//**************************************************************
// perform timer dependend tasks (read CPU speed/load etc.)
//**************************************************************
void CSpeedswitchXPDlg::OnTimer( UINT nIDEvent ) 
{
	if( nIDEvent == timerID )
  {
    if( (options.readCPUSpeed || options.readCPULoad || options.showBattery) && (int)tickCounterCPU>=options.cpuInterval )
    {
      UpdateData();

      CString sp = m_szCPUSpeed;
      CString lo = m_szCPULoad;

      if( options.readCPUSpeed )
        readCPUSpeed();
      else
      {
        speedArr[speedIndex++] = 0;
        if( speedIndex >= 1200 )
          speedIndex = 0;
      }

      if( options.readCPULoad )
        readCPULoad();
      else
      {
        loadArr[loadIndex++] = 0;
        if( loadIndex >= 1200 )
          loadIndex = 0;
      }

      if( sp!=m_szCPUSpeed || lo!=m_szCPULoad )
      {
        displayTrayIcon( NIM_MODIFY );

        if( cpuDataIconActive )
          displayCPUDataIcon( NIM_MODIFY );
      }

      if( options.showBattery )
        batteryDisplay( NULL );

      tickCounterCPU = 0;

      UpdateData( FALSE );

      if( options.showDiagram )   // update history diagram
        InvalidateRect( &cpuFrame, FALSE );
    }

    if( (int)tickCounterCheck >= (options.checkInterval*60) ) 
    {
      UpdateData();
      
      checkProfile( 3 );

      tickCounterCheck = 0;
      UpdateData( FALSE );
    }

    tickCounterCPU++;
    tickCounterCheck++;
  }

	CDialog::OnTimer( nIDEvent );
}

//**************************************************************
// get the current CPU speed depending on the chosen method
//**************************************************************
void CSpeedswitchXPDlg::readCPUSpeed() 
{
  switch( options.speedMethod )
  {
    case 0: {
              DWORD speed;
              PROCESSOR_POWER_INFORMATION ppi;
              BOOL x = (CallNtPowerInformation(ProcessorInformation,
                                               NULL,
                                               0,
                                               &ppi,
                                               sizeof(ppi)) == ERROR_SUCCESS);
              if( x )
              {
/*
                log( "curspeed:%d maxspeed:%d limit:%d Maxidlestate:%d Currentidlestate:%d",
                     ppi.CurrentMhz, 
                     ppi.MaxMhz, 
                     ppi.MhzLimit, 
                     ppi.MaxIdleState, 
                     ppi.CurrentIdleState );
*/
                speed = ppi.CurrentMhz;
                m_szCPUSpeed.Format( _T("%d MHz"), speed );
              }
              else
              {
                m_szCPUSpeed = _T("???");
                speed = 0;
              }

              BYTE z = (BYTE)(speed / 100);
              if( speed%100 >= 90 )
                z++;

              speedArr[speedIndex++] = z;
              if( speedIndex >= 1200 )
                speedIndex = 0;

              curSpeed = speed;
            }
            break;

    case 1:
            {
              int x = cpuSpeedHT();
              if( x == 0 )
                m_szCPUSpeed = _T("???");
              else
                m_szCPUSpeed.Format( _T("%d MHz"), x );

              BYTE z = (BYTE)(x / 100);
              if( (x)%100 >= 90 )
                z++;

              speedArr[speedIndex++] = z;
              if( speedIndex >= 1200 )
                speedIndex = 0;
              
              curSpeed = x;
            }
            break;

    case 2:
            {
              CPUSpeed sp;
              int x = sp.CPUSpeedInMHz;
              if( x == 0 )
                m_szCPUSpeed = _T("???");
              else
                m_szCPUSpeed.Format( _T("%d MHz"), x );

              BYTE z = (BYTE)(x / 100);
              if( (x)%100 >= 90 )
                z++;

              speedArr[speedIndex++] = z;
              if( speedIndex >= 1200 )
                speedIndex = 0;
              
              curSpeed = x;
            }
            break;
  }
}

//**************************************************************
// read the current CPU load
//**************************************************************
void CSpeedswitchXPDlg::readCPULoad() 
{
  DWORD t = cpuUsageNT();
  static DWORD oldUsage = 0;

  if( t > 100 )
    t = oldUsage;

  m_szCPULoad.Format( _T("%d %%"), t );

  loadArr[loadIndex++] = (BYTE)t;
  if( loadIndex >= 1200 )
    loadIndex = 0;

  curLoad = t;
}

//**************************************************************
// draw the history diagram
//**************************************************************
void CSpeedswitchXPDlg::drawDiagram()
{
  CPaintDC dc( this );
  CBrush bBlackBrush( RGB(0,0,0) );                     // Schwarz als Pinsel
  CPen bRed( PS_SOLID, 1, RGB(255,0,0) );               // Rot
  CPen bYellow( PS_SOLID, 1, RGB(255,255,0) );          // Gelb
  CPen bWhite( PS_SOLID, 1, RGB(255,255,255) );         // Weiß
  CPen bGreen( PS_SOLID,1,RGB(0,255,0) );               // Grün
  CPen bMagenta( PS_SOLID, 1, RGB(255,0,255) );         // Magenta
  CPen bCyan( PS_SOLID, 1, RGB(0,255,255) );            // Cyan
  CPen bWhiteDotted( PS_DOT, 1, RGB(255,255,255) );     // Weiß gestrichelt für Skala

#define XBASE (cpuFrame.left)
#define YBASE (cpuFrame.top)

  // Bereich anschwärzen ;-)
  dc.SetBkMode( TRANSPARENT );
  dc.FillRect( &cpuFrame, &bBlackBrush );

  // gestrichelte Linien im Diagramm bei 25er-, 50er- und 75er-Skala zeichnen
  CPen* old = dc.SelectObject( &bWhiteDotted );
  dc.MoveTo( XBASE+1, YBASE+50 );
  dc.LineTo( XBASE+351, YBASE+50 );
  dc.MoveTo( XBASE+1, YBASE+50+25 );
  dc.LineTo( XBASE+351, YBASE+50+25 );
  dc.MoveTo( XBASE+1, YBASE+50-25 );
  dc.LineTo( XBASE+351, YBASE+50-25 );

  if( options.showDiagram )
  {
    int x = loadIndex-117-diagramOffset;
    int run = 0;

    if( options.readCPULoad )
    {
      dc.SelectObject( &bYellow );
   
      while( run <= 348 )
      {
        while( x < 0 )
          x = 1200+x;

        int a=loadArr[x];
        if( run == 0 )
          dc.MoveTo( XBASE+1+run+1, YBASE+100-a );

        dc.LineTo( XBASE+1+run+1, YBASE+100-a );

        x++;
        run += 3;

        if( x >= 1200 )
          x = 0;
      }
    }

    if( options.readCPUSpeed )
    {
      x = loadIndex-117-diagramOffset;
      run = 0;

      dc.SelectObject( &bRed );
   
      while( run <= 348 )
      {
        while( x < 0 )
          x = 1200+x;

        int a=speedArr[x] * options.freqScaling;
        if( a > 100 )
          a = 100;

        if( run == 0 )
          dc.MoveTo( XBASE+1+run+1, YBASE+100-a );

        dc.LineTo( XBASE+1+run+1, YBASE+100-a );

        x++;
        run += 3;

        if( x >= 1200 )
          x = 0;
      }
    }
  }

  dc.SelectObject( GetFont() );
  dc.SetBkMode( OPAQUE );
  dc.SetBkColor( RGB(0,0,0) );

  dc.SetTextColor( RGB(255,0,0) );
  dc.TextOut( legendFrame.left+10, legendFrame.top+25, legend1 );

  dc.SetTextColor( RGB(255,255,0) );
  dc.TextOut( legendFrame.left+10, legendFrame.top+45, legend2 );

#undef XBASE
#undef YBASE
}

//**************************************************************
// retrieve string for a particular windows error code
//**************************************************************
TCHAR* CSpeedswitchXPDlg::getWinErr( DWORD errcode )
{
  static TCHAR msg[256];
  
  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 errcode,
                 MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                 msg,
                 255,
                 NULL );

  TCHAR* t = msg + _tcslen(msg) - 1;

  while( t > msg )
  {
    if( isspace(*t) )
      t--;
    else
      break;
  }

  t[1]='\0';
  return msg;  
}

//**************************************************************
// handler for 'Detect max speed' button
//**************************************************************
void CSpeedswitchXPDlg::OnBnClickedButton2()
{
  AfxGetApp()->DoWaitCursor(1);

  int aktuell = options.ac ? acThrottle : dcThrottle;

  // switch to max performance:
  setState( options.ac, PO_THROTTLE_NONE );

  // wait for 2 seconds to make sure switching took place (this causes 100% CPU load)
  time_t x = time(NULL) + 2;
  while( time(NULL) < x )     
    ;

  int f = cpuSpeedHT();
  options.maxSpeed = f;

  m_szMaxCPUSpeed.Format( _T("~%d MHz"), options.maxSpeed );

  setRegKeys();

  // switch back to original policy:
  setState( options.ac, aktuell );

  UpdateData( FALSE );
  AfxGetApp()->DoWaitCursor(-1);
}

//**************************************************************
// control battery icon display
//**************************************************************
void CSpeedswitchXPDlg::batteryDisplay( SYSTEM_POWER_STATUS* sps )
{
  if( sps == NULL )
  {
    if( !GetSystemPowerStatus(&pwrStatus) )
    {  
      options.showBattery = FALSE;
      setRegKeys();
      CString s1;
      s1.LoadStringW( IDS_MAINERR9 );
      MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
      return;
    }

    sps = &pwrStatus;
  }

  log( _T("AC-Status:%d batteryIcon:%d chargeIcon:%d method:%d chargeIndicator:%d batteryFlag:%d"),
       sps->ACLineStatus, batteryIconActive, batteryIconCharge, options.batteryMethod,
       options.chargeIndicator, sps->BatteryFlag );

  if( sps->ACLineStatus == 0 )    // running on battery
  {
    if( !batteryIconActive )
      showBatteryIcon( NIM_ADD, iTrayBattery, sps );
    else
      showBatteryIcon( NIM_MODIFY, iTrayBattery, sps );
  }
  else                            // running on AC
  {
    if( options.batteryMethod == 0 )    // show only on DC
    {
      if( !options.chargeIndicator && batteryIconActive )
        removeBatteryIcon();
      else if( options.chargeIndicator )    // show charging status
      {
        if( sps->BatteryFlag & 8 )   // it is charging
        {  
          if( !batteryIconActive )
            showBatteryIcon( NIM_ADD, iBattCharge, sps );
          else 
            showBatteryIcon( NIM_MODIFY, iBattCharge, sps );
        }
        else                          // not charging
        {
          if( batteryIconActive )
            removeBatteryIcon();
        }
      }
    }
    else        // show always
    {
      if( !options.chargeIndicator )      // no charge showing
      {
        if( !batteryIconActive )
          showBatteryIcon( NIM_ADD, iTrayBattery, sps );
        else
          showBatteryIcon( NIM_MODIFY, iTrayBattery, sps );
      }
      else        // show charging status
      {
        if( sps->BatteryFlag & 8 )     // now charging
        {
          if( !batteryIconActive )
            showBatteryIcon( NIM_ADD, iBattCharge, sps );
          else
            showBatteryIcon( NIM_MODIFY, iBattCharge, sps );
        }
        else                            // not charging now
        {
          if( !batteryIconActive )
            showBatteryIcon( NIM_ADD, iTrayBattery, sps );
          else
            showBatteryIcon( NIM_MODIFY, iTrayBattery, sps );
        }
      }
    }
  }
}

//**************************************************************
// remove battery icon from tray area
//**************************************************************
void CSpeedswitchXPDlg::removeBatteryIcon()
{
  NOTIFYICONDATA nid;
  nid.cbSize = sizeof( NOTIFYICONDATA );
  nid.hWnd = m_hWnd;
  nid.uID = 701;
  nid.uCallbackMessage = WM_ICONNOTIFY;
  nid.hIcon = modBatteryIcon;
  Shell_NotifyIcon( NIM_DELETE, &nid );
  DestroyIcon( modBatteryIcon );
  batteryIconActive = FALSE;
  batteryIconCharge = FALSE;
  batteryValue = -2;
}

// 10 digits (0-9) à 16 bytes: 2 bytes per row (16 pixels) * 8 rows = 16x8 matrix
// + 2 digits for a compressed '100' (special case for CPU usage)
// + 1 digit for '-' as unreported fan speed digits
static unsigned char digits[17][16] = 
{
  // digits 0-9:
  { 0xf0,0x0f,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xf0,0x0f },
  { 0xfc,0xff,0xf0,0xff,0xc0,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xc0,0x0f },
  { 0xf0,0x0f,0xcf,0xf3,0xff,0xf3,0xff,0xcf,0xff,0x3f,0xfc,0xff,0xf3,0xff,0xc0,0x03 },
  { 0xf0,0x0f,0xcf,0xf3,0xff,0xf3,0xff,0x0f,0xff,0xf3,0xff,0xf3,0xcf,0xf3,0xf0,0x0f },
  { 0xff,0x3f,0xfc,0x3f,0xf3,0x3f,0xcf,0x3f,0x00,0x03,0xff,0x3f,0xff,0x3f,0xf0,0x03 },
  { 0xc0,0x03,0xcf,0xff,0xcf,0xff,0xc0,0x0f,0xff,0xf3,0xff,0xf3,0xcf,0xf3,0xf0,0x0f },
  { 0xfc,0x0f,0xf3,0xff,0xcf,0xff,0xc0,0x0f,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xf0,0x0f },
  { 0xc0,0x0f,0xcf,0xcf,0xff,0x3f,0xff,0x3f,0xfc,0xff,0xfc,0xff,0xf3,0xff,0xf3,0xff },
  { 0xf0,0x0f,0xcf,0xf3,0xcf,0xf3,0xf0,0x0f,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xf0,0x0f },
  { 0xf0,0x0f,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xf0,0x03,0xff,0xf3,0xff,0xcf,0xf0,0x3f },

  // 2 additional digits for '100':
  { 0xf3,0xf0,0xc3,0xcf,0x03,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0x00,0x30 },
  { 0x3f,0x03,0xcc,0xfc,0xcc,0xfc,0xcc,0xfc,0xcc,0xfc,0xcc,0xfc,0xcc,0xfc,0x3f,0x03 },

  // 1 additional character for '-':
  { 0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0x03,0xc0,0x03,0xff,0xff,0xff,0xff,0xff,0xff },

  // 4 additional characters for 'Sl' and 'Hi':
  { 0xf0,0x03,0xcf,0xff,0xcf,0xff,0xcf,0xff,0xf0,0x0f,0xff,0xf3,0xff,0xf3,0xc0,0x0f },
  { 0xcf,0xff,0xcf,0xff,0xcf,0xff,0xcf,0xff,0xcf,0xff,0xcf,0xff,0xcf,0xff,0xf0,0x0f },
  { 0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3,0xc0,0x03,0xcf,0xf3,0xcf,0xf3,0xcf,0xf3 },
  { 0xff,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xfc,0xff,0xfc,0xff,0xf0,0x3f }
};

// 10 digits (0-9) à 8 bytes: 1 byte per row (8 pixels) * 8 rows = 8x8 matrix
static unsigned char digits2[10][8] = 
{
  { 0x3c,0x42,0x42,0x42,0x42,0x42,0x42,0x3c },
  { 0x10,0x30,0x70,0x10,0x10,0x10,0x10,0x7c },
  { 0x3c,0x42,0x02,0x04,0x08,0x10,0x20,0x7e },
  { 0x3c,0x42,0x02,0x0c,0x02,0x02,0x42,0x3c },
  { 0x0c,0x1c,0x2c,0x4c,0xfe,0x0c,0x0c,0x3e },
  { 0x7e,0x40,0x40,0x7c,0x02,0x02,0x42,0x3c },
  { 0x1c,0x20,0x40,0x7c,0x42,0x42,0x42,0x3c },
  { 0x7c,0x44,0x08,0x08,0x10,0x10,0x20,0x20 },
  { 0x3c,0x42,0x42,0x3c,0x42,0x42,0x42,0x3c },
  { 0x3c,0x42,0x42,0x42,0x3e,0x02,0x04,0x38 }
};

//**************************************************************
// create battery icon and show/modify it
//**************************************************************
void CSpeedswitchXPDlg::showBatteryIcon( int msg, HICON icon, SYSTEM_POWER_STATUS* sps )
{
  int newval;
  int chargeLeft;
  int timeLeft;
  BOOL chargeIconNow = FALSE;

  chargeLeft = sps->BatteryLifePercent;
  if( chargeLeft<0 || chargeLeft>100 )
    chargeLeft = 999;

  if( sps->ACLineStatus == 0 )    // running on battery
  {
    timeLeft = sps->BatteryLifeTime;
    if( timeLeft < 0 )
      timeLeft = 999;
    else
      timeLeft /= 60;
  }
  else                            // on AC
    timeLeft = 999;

  if( icon == iBattCharge )
    chargeIconNow = TRUE;

  if( options.batteryIndicator==0 || chargeIconNow )     // show percentage
    newval = (chargeLeft==999 ? -3 : chargeLeft);
  else                                    // show time left
    newval = (timeLeft==999 ? -3 : timeLeft);

  CString s1, s2, s3;
  s1.LoadStringW( IDS_MAIN28 );
  s2.LoadStringW( IDS_MAIN30 );
  s3.LoadStringW( IDS_MAIN29 );

  if( msg==NIM_MODIFY && newval==batteryValue && batteryIconCharge==chargeIconNow )
  {
    NOTIFYICONDATA nid;
    static TCHAR tp[128];

    nid.cbSize = sizeof( NOTIFYICONDATA );
    nid.hWnd = m_hWnd;
    nid.uID = 701;
    nid.uFlags = NIF_TIP;

    CString temp;
    temp.Format( _T("%d%%"), chargeLeft );

    wsprintf( tp, s1,
              chargeLeft==999 ? s2 : temp );

    if( timeLeft != 999 )
    {
      CString s1;
      s1.LoadStringW( IDS_MAIN29 );
      wsprintf( &tp[_tcslen(tp)], s3, timeLeft );
    }

    lstrcpy( nid.szTip, tp );

    Shell_NotifyIcon( msg, &nid );
    return;
  }

  if( msg == NIM_MODIFY )
    DestroyIcon( modBatteryIcon );

  // create new icon:
  ICONINFO iconinfo;
  ICONINFO d2;

  GetIconInfo( icon, &d2 );

  iconinfo.hbmMask = d2.hbmMask;
  iconinfo.hbmColor = d2.hbmColor;

  CBitmap* mask = CBitmap::FromHandle( iconinfo.hbmMask );
  BITMAP maskMap;
  mask->GetBitmap( &maskMap );
  int size = maskMap.bmHeight * maskMap.bmWidthBytes;   // 128 Bytes (32x32 monochrom)
  log( _T("Bitmap size: %d"), size );
  mask->GetBitmapBits( size, bitmap );

  int num0 = (newval/100)%10;   // first decimal digit
  int num1 = (newval/10)%10;    // second decimal digit
  int num2 = newval%10;         // third decimal digit
  int j=0;

  if( newval == -3 )     // special value for deactivated ("--")
  {
    num0 = 0;
    num1 = 12;
    num2 = 12;
  }

  if( num0 != 0 )   // display three digits
  {
    for( int i=16; i<31; i+=2 )
    {
      for( int m=0; m<2; m++ )   // each loop = 1 icon row
      {
        bitmap[(i+m)*4] = 0xff ^ (digits2[num0][j]>>2);
        bitmap[(i+m)*4+1] = 0xff ^ (digits2[num0][j]<<6 | digits2[num1][j]>>4);
        bitmap[(i+m)*4+2] = 0xff ^ (digits2[num1][j]<<4 | digits2[num2][j]>>6); 
        bitmap[(i+m)*4+3] = 0xff ^ (digits2[num2][j]<<2);
      }
 
      j++;   // advance in digits2 array
    }
  }
  else              // only two digits
  {
    for( int i=16; i<=31; i+=2 )    // fill icon region
    {
      for( int m=0; m<2; m++ )   // each loop = 1 icon row
      {
        bitmap[(i+m)*4] = digits[num1][j];
        bitmap[(i+m)*4+1] = digits[num1][j+1];
        bitmap[(i+m)*4+2] = digits[num2][j];
        bitmap[(i+m)*4+3] = digits[num2][j+1];
      }
 
      j+=2;   // advance in digits array
    }
  }

  maskMap.bmBits = bitmap;

  CBitmap newMask;
  newMask.CreateBitmapIndirect( &maskMap );

  iconinfo.hbmMask = (HBITMAP)newMask.m_hObject;
  iconinfo.fIcon = TRUE;

  modBatteryIcon = ::CreateIconIndirect( &iconinfo );    

  NOTIFYICONDATA nid;
  static TCHAR tp[128];

  nid.cbSize = sizeof( NOTIFYICONDATA );
  nid.hWnd = m_hWnd;
  nid.uID = 701;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = WM_ICONNOTIFY;

  CString temp;
  temp.Format( _T("%d%%"), chargeLeft );

  wsprintf( tp, s1,
            chargeLeft==999 ? s2 : temp );

  if( timeLeft != 999 )
    wsprintf( &tp[_tcslen(tp)], s3, timeLeft );

  lstrcpy( nid.szTip, tp );
  nid.hIcon = modBatteryIcon;

  Shell_NotifyIcon( msg, &nid );

  batteryIconActive = TRUE;
  batteryIconCharge = chargeIconNow;
  batteryValue = newval;
}

//**************************************************************
// fill combo boxes on main window with current values
//**************************************************************
void CSpeedswitchXPDlg::initPowerBoxes()
{
  m_cDiskAC.SetCurSel( findDiskIndex(internalPolicy.user.SpindownTimeoutAc) );
  m_cDiskDC.SetCurSel( findDiskIndex(internalPolicy.user.SpindownTimeoutDc) );
  m_cDisplayAC.SetCurSel( findDefaultIndex(internalPolicy.user.VideoTimeoutAc) );
  m_cDisplayDC.SetCurSel( findDefaultIndex(internalPolicy.user.VideoTimeoutDc) );

  log( _T("ACAction: %d"), internalPolicy.user.IdleAc.Action );
  log( _T("ACIdletimeout:%d min"), internalPolicy.user.IdleTimeoutAc/60 );
  log( _T("ACHibtimeout:%d min"), internalPolicy.mach.DozeS4TimeoutAc );

  if( internalPolicy.user.IdleAc.Action == PowerActionHibernate )
  {
    m_cStandbyAC.SetCurSel( 15 );
    maxHibIndexAC = fillHibernationTimes( &m_cHibernationAC, hibernationTimesAC, 15, 3 );
    m_cHibernationAC.SetCurSel( 
                      findHibernationIndex(hibernationTimesAC,
                                           internalPolicy.user.IdleTimeoutAc,
                                           maxHibIndexAC) );
  }
  else
  {
    int sIdxAC = findDefaultIndex( internalPolicy.user.IdleTimeoutAc );
    m_cStandbyAC.SetCurSel( sIdxAC );
    maxHibIndexAC = fillHibernationTimes( &m_cHibernationAC, hibernationTimesAC, sIdxAC, 3 );
    int timeout = internalPolicy.mach.DozeS4TimeoutAc;
    if( timeout > 0 )
      timeout += internalPolicy.user.IdleTimeoutAc;

    m_cHibernationAC.SetCurSel( 
                      findHibernationIndex(hibernationTimesAC,
                                           timeout,
                                           maxHibIndexAC) );
  }

  log( _T("DCAction: %d"), internalPolicy.user.IdleDc.Action );
  log( _T("DCIdletimeout:%d min"), internalPolicy.user.IdleTimeoutDc/60 );
  log( _T("DCHibtimeout:%d min"), internalPolicy.mach.DozeS4TimeoutDc/60 );

  if( internalPolicy.user.IdleDc.Action == PowerActionHibernate )
  {
    m_cStandbyDC.SetCurSel( 15 );
    maxHibIndexDC = fillHibernationTimes( &m_cHibernationDC, hibernationTimesDC, 15, 3 );
    m_cHibernationDC.SetCurSel( 
                      findHibernationIndex(hibernationTimesDC,
                                           internalPolicy.user.IdleTimeoutDc,
                                           maxHibIndexDC) );
  }
  else
  {
    int sIdxDC = findDefaultIndex( internalPolicy.user.IdleTimeoutDc );
    m_cStandbyDC.SetCurSel( sIdxDC );
    maxHibIndexDC = fillHibernationTimes( &m_cHibernationDC, hibernationTimesDC, sIdxDC, 3 );

    int timeout = internalPolicy.mach.DozeS4TimeoutDc;
    if( timeout > 0 )
      timeout += internalPolicy.user.IdleTimeoutDc;

    m_cHibernationDC.SetCurSel( 
                      findHibernationIndex(hibernationTimesDC,
                                           timeout,
                                           maxHibIndexDC) );

  }
}

int CSpeedswitchXPDlg::findDefaultIndex( int val )
{
  for( int i=0; i<16; i++ )
    if( defaultTimes[i] == val )
      return i;

  return -1;    // return -1 as default if not found
}

int CSpeedswitchXPDlg::findDiskIndex( int val )
{
  for( int i=0; i<14; i++ )
    if( diskTimes[i] == val )
      return i;

  return -1;    // return -1 as default if not found
}

int CSpeedswitchXPDlg::findHibernationIndex( int* hibernation, int val, int maxIndex )
{
  for( int i=0; i<maxIndex; i++ )
    if( hibernation[i] == val )
      return i;

  return -1;    // return -1 as default if not found
}

//----------------------------------------------------------------
// what: 0=Display timeout
//       1=Disk timeout
//       2=Standby timeout
//       3=Hibernation timeout
//----------------------------------------------------------------
int CSpeedswitchXPDlg::fillHibernationTimes( CComboBox* box, int* hibernation, int idx, int what )
{
  int val;
  CString str, s1;
  int j=0;

  if( idx == 15 )
    idx = -1;   // start at 0

  box->ResetContent();

  for( int i=idx+1; i<17; i++ )
  {
    boolean nix = false;

    switch( i )
    {
      case 0: if( what == 1 )
                nix = true;
              else
              {
                val = 60;
                s1.LoadStringW( IDS_MAIN39 );
                str = s1;
              }
              break;

      case 1: if( what == 1 )
                nix = true;
              else
              {
                val = 120;
                s1.LoadStringW( IDS_MAIN40 );
                str = s1;
              }
              break;

      case 2: val = 180;
              s1.LoadStringW( IDS_MAIN41 );
              str = s1;
              break;

      case 3: val = 300;
              s1.LoadStringW( IDS_MAIN42 );
              str = s1;
              break;

      case 4: val = 600;
              s1.LoadStringW( IDS_MAIN43 );
              str = s1;
              break;

      case 5: val = 900;
              s1.LoadStringW( IDS_MAIN44 );
              str = s1;
              break;

      case 6: val = 1200;
              s1.LoadStringW( IDS_MAIN45 );
              str = s1;
              break;

      case 7: val = 1500;
              s1.LoadStringW( IDS_MAIN46 );
              str = s1;
              break;

      case 8: val = 1800;
              s1.LoadStringW( IDS_MAIN47 );
              str = s1;
              break;

      case 9: val = 2700;
              s1.LoadStringW( IDS_MAIN48 );
              str = s1;
              break;

      case 10:val = 3600;
              s1.LoadStringW( IDS_MAIN49 );
              str = s1;
              break;

      case 11:val = 3600*2;
              s1.LoadStringW( IDS_MAIN50 );
              str = s1;
              break;

      case 12:val = 3600*3;
              s1.LoadStringW( IDS_MAIN51 );
              str = s1;
              break;

      case 13:val = 3600*4;
              s1.LoadStringW( IDS_MAIN52 );
              str = s1;
              break;

      case 14:val = 3600*5;
              s1.LoadStringW( IDS_MAIN53 );
              str = s1;
              break;

      case 15:if( what != 3 )
                nix = true;
              else
              {
                val = 3600*6;
                s1.LoadStringW( IDS_MAIN54 );
                str = s1;
              }
              break;

      case 16:val = 0;
              s1.LoadStringW( IDS_MAIN55 );
              str = s1;
              break;
    }

    if( !nix )
    {
      if( hibernation != NULL )
        hibernation[j++] = val;

      box->AddString( str );
    }
    else
      nix = false;
  }

  return j;
}

//**************************************************************
// handlers for selecting values from power combo boxes on main 
// window
//**************************************************************
void CSpeedswitchXPDlg::OnCbnSelchangeDisplayAC()
{
  internalPolicy.user.VideoTimeoutAc = defaultTimes[m_cDisplayAC.GetCurSel()];
  writePolicies( true, false );
}

void CSpeedswitchXPDlg::OnCbnSelchangeDisplayDC()
{
  internalPolicy.user.VideoTimeoutDc = defaultTimes[m_cDisplayDC.GetCurSel()];
  writePolicies( true, false );
}

void CSpeedswitchXPDlg::OnCbnSelchangeDiskAC()
{
  internalPolicy.user.SpindownTimeoutAc = diskTimes[m_cDiskAC.GetCurSel()];
  writePolicies( true, false );
}

void CSpeedswitchXPDlg::OnCbnSelchangeDiskDC()
{
  internalPolicy.user.SpindownTimeoutDc = diskTimes[m_cDiskDC.GetCurSel()];
  writePolicies( true, false );
}

void CSpeedswitchXPDlg::OnCbnSelchangeHibAC()
{
  log( _T("------------------------------") );
  log( _T("Start of 'OnCbnSelchangeHibAC'") );

  int n = hibernationTimesAC[m_cHibernationAC.GetCurSel()];

  log( _T("new ACHib val: %d min"), n/60 );
  log( _T("current ACIdletimeout: %d min"), internalPolicy.user.IdleTimeoutAc/60 );
  log( _T("GUI AC Standby selection: %d min"), defaultTimes[m_cStandbyAC.GetCurSel()]/60 );

  if( n!=0 && internalPolicy.user.IdleAc.Action!=PowerActionHibernate )
    n -= internalPolicy.user.IdleTimeoutAc;

  if( defaultTimes[m_cStandbyAC.GetCurSel()] == 0 )
  {
    internalPolicy.user.IdleTimeoutAc = n;
    internalPolicy.mach.DozeS4TimeoutAc = 0;
    internalPolicy.user.IdleAc.Action = PowerActionHibernate;
  }
  else
    internalPolicy.mach.DozeS4TimeoutAc = n;

  log( _T("new ACIdletimeout: %d min"), internalPolicy.user.IdleTimeoutAc/60 );
  log( _T("new ACHibtimeout: %d min"), internalPolicy.mach.DozeS4TimeoutAc/60 );

  writePolicies( true, false );
  log( _T("End of 'OnCbnSelchangeHibAC'") );
}

void CSpeedswitchXPDlg::OnCbnSelchangeHibDC()
{
  log( _T("------------------------------") );
  log( _T("Start of 'OnCbnSelchangeHibDC'") );

  int n = hibernationTimesDC[m_cHibernationDC.GetCurSel()];

  log( _T("new DCHib val: %d min"), n/60 );
  log( _T("current DCIdletimeout: %d min"), internalPolicy.user.IdleTimeoutDc/60 );
  log( _T("GUI DC Standby selection: %d min"), defaultTimes[m_cStandbyDC.GetCurSel()]/60 );

  if( n!=0 && internalPolicy.user.IdleDc.Action!=PowerActionHibernate )
    n -= internalPolicy.user.IdleTimeoutDc;

  if( defaultTimes[m_cStandbyDC.GetCurSel()] == 0 )
  {
    internalPolicy.user.IdleTimeoutDc = n;
    internalPolicy.mach.DozeS4TimeoutDc = 0;
    internalPolicy.user.IdleDc.Action = PowerActionHibernate;
  }
  else
    internalPolicy.mach.DozeS4TimeoutDc = n;

  log( _T("new DCIdletimeout: %d min"), internalPolicy.user.IdleTimeoutDc/60 );
  log( _T("new DCHibtimeout: %d min"), internalPolicy.mach.DozeS4TimeoutDc/60 );

  writePolicies( true, false );
  log( _T("End of 'OnCbnSelchangeHibDC'") );
}

void CSpeedswitchXPDlg::OnCbnSelchangeStandbyAC()
{
  int last;

  if( internalPolicy.user.IdleAc.Action == PowerActionHibernate )
    last = internalPolicy.user.IdleTimeoutAc;
  else
  {
    last = internalPolicy.mach.DozeS4TimeoutAc;
    if( last != 0 )
      last += internalPolicy.user.IdleTimeoutAc;
  }

  int n = defaultTimes[m_cStandbyAC.GetCurSel()];

  internalPolicy.user.IdleTimeoutAc = n;

  maxHibIndexAC = fillHibernationTimes( &m_cHibernationAC, hibernationTimesAC, m_cStandbyAC.GetCurSel(), 3 );
  log( _T("Searching for %d (%d) (max:%d)"), last, last/60, maxHibIndexAC );
  int x = findHibernationIndex( hibernationTimesAC, last, maxHibIndexAC );
  log( _T("Found:%d"), x );
  if( x == -1 )
    x = 0;

  n = hibernationTimesAC[x];
  if( n != 0 )
    n -= internalPolicy.user.IdleTimeoutAc;

  internalPolicy.mach.DozeS4TimeoutAc = n;

  m_cHibernationAC.SetCurSel( x );

  if( internalPolicy.user.IdleTimeoutAc==0 && internalPolicy.mach.DozeS4TimeoutAc!=0 )
  {
    internalPolicy.user.IdleTimeoutAc = internalPolicy.mach.DozeS4TimeoutAc;
    internalPolicy.mach.DozeS4TimeoutAc = 0;
    internalPolicy.user.IdleAc.Action = PowerActionHibernate;
  }
  else
    internalPolicy.user.IdleAc.Action = PowerActionSleep;

  writePolicies( true, false );
}

void CSpeedswitchXPDlg::OnCbnSelchangeStandbyDC()
{
  int last;

  if( internalPolicy.user.IdleDc.Action == PowerActionHibernate )
    last = internalPolicy.user.IdleTimeoutDc;
  else
  {
    last = internalPolicy.mach.DozeS4TimeoutDc;
    if( last != 0 )
      last += internalPolicy.user.IdleTimeoutDc;
  }

  int n = defaultTimes[m_cStandbyDC.GetCurSel()];

  internalPolicy.user.IdleTimeoutDc = n;

  maxHibIndexDC = fillHibernationTimes( &m_cHibernationDC, hibernationTimesDC, m_cStandbyDC.GetCurSel(), 3 );
  int x = findHibernationIndex( hibernationTimesDC, last, maxHibIndexDC );
  if( x == -1 )
    x = 0;

  n = hibernationTimesDC[x];
  if( n != 0 )
    n -= internalPolicy.user.IdleTimeoutDc;

  internalPolicy.mach.DozeS4TimeoutDc = n;

  m_cHibernationDC.SetCurSel( x );

  if( internalPolicy.user.IdleTimeoutDc==0 && internalPolicy.mach.DozeS4TimeoutDc!=0 )
  {
    internalPolicy.user.IdleTimeoutDc = internalPolicy.mach.DozeS4TimeoutDc;
    internalPolicy.mach.DozeS4TimeoutDc = 0;
    internalPolicy.user.IdleDc.Action = PowerActionHibernate;
  }
  else
    internalPolicy.user.IdleDc.Action = PowerActionSleep;

  writePolicies( true, false );
}

//**************************************************************
// handler for 'System Info' button
//**************************************************************
void CSpeedswitchXPDlg::OnBnClickedCPUData()
{
  CSystemInfo si;
  si.setVars();
  si.DoModal();
}

//**************************************************************
// handler for hidden 'Update' button (only used during development)
//**************************************************************
void CSpeedswitchXPDlg::OnBnClickedButton4()
{
  initPowerValues();
}

//**************************************************************
// read current power and CPU policy from registry
//**************************************************************
BOOL CSpeedswitchXPDlg::initPowerValues()
{
  static TCHAR msg[1024];   // für diverse Fehlertexte

  if( !CanUserWritePwrScheme() )
  {
    DWORD x = GetLastError();
    CString s1;
    s1.LoadStringW( IDS_MAIN31 );
    wsprintf( msg, s1, x, getWinErr(x) );
    MessageBox( msg, err, MB_OK|MB_ICONEXCLAMATION );
    PostMessage( WM_CLOSE );
    return FALSE;
  }

  UINT origScheme;
  if( !GetActivePwrScheme(&origScheme) )
  {
    DWORD x = GetLastError();
    CString s1;
    s1.LoadStringW( IDS_MAIN32 );
    wsprintf( msg, s1, x, getWinErr(x) );
    MessageBox( msg, err, MB_OK|MB_ICONEXCLAMATION );
    PostMessage( WM_CLOSE );
    return FALSE;
  }

  log( _T("Current power scheme no: %d"), origScheme );

  options.originalScheme = origScheme;
  options.setRegKeys();

  int x = detectProfile();

  log( _T("profile detect rc: %d"), x );

  if( x == -2 )
  {
    int y = createProfile();
    if( y != 0 )
    {
      DWORD x = GetLastError();
      CString s1;
      s1.LoadStringW( IDS_MAIN33 );
      wsprintf( msg, s1, y, x, getWinErr(x) );
      MessageBox( msg, err, MB_OK|MB_ICONEXCLAMATION );
      PostMessage( WM_CLOSE );
      return FALSE;
    }
  }
  else if( x < 0 )
  {
    DWORD z = GetLastError();
    CString s1;
    s1.LoadStringW( IDS_MAIN34 );
    wsprintf( msg, s1, x, z, getWinErr(z) );
    MessageBox( msg, err, MB_OK|MB_ICONEXCLAMATION );
    PostMessage( WM_CLOSE );
    return FALSE;
  }

  if( !GetSystemPowerStatus(&pwrStatus) )
  {
    CString s1;
    s1.LoadStringW( IDS_MAIN35 );
    MessageBox( s1, err, MB_ICONEXCLAMATION|MB_OK );
  }
  else
    options.ac = (pwrStatus.ACLineStatus!=0);

  setCPUIcon( options.ac ? acThrottle : dcThrottle );

  if( options.showBattery )
    batteryDisplay( &pwrStatus );

  if( options.speedIcon || options.loadIcon )
    displayCPUDataIcon( NIM_ADD );

  m_iACState = acThrottle;
  m_iDCState = dcThrottle;

  if( options.autoStart )
    setAutoStartRegistry();

  ((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange( 0, 90, FALSE );
  ((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollPos( 90, TRUE );

  if( options.checkStatus || options.readCPUSpeed || options.readCPULoad || options.showBattery )
  {
    if( SetTimer(timerID,1000,NULL) == timerID )
      activeTimer = TRUE;
    else
    {
      CString s1;
      s1.LoadStringW( IDS_MAIN36 );
      MessageBox( s1, err, MB_ICONERROR|MB_OK );
      PostMessage( WM_CLOSE );
      return FALSE;
    }
  }

  cpuUsageNT();     // let it run once to initialize itself (?)

  if( options.maxSpeed <= 0 )
    m_szMaxCPUSpeed = _T("???");
  else
    m_szMaxCPUSpeed.Format( _T("~%d MHz"), options.maxSpeed );

  defaultTimes[0] = 60;                       // 1 min
  defaultTimes[1] = 120;                      // 2 min
  defaultTimes[2] = diskTimes[0] = 180;       // 3 min
  defaultTimes[3] = diskTimes[1] = 300;       // 5 min
  defaultTimes[4] = diskTimes[2] = 600;       // 10 min
  defaultTimes[5] = diskTimes[3] = 900;       // 15 min
  defaultTimes[6] = diskTimes[4] = 1200;      // 20 min
  defaultTimes[7] = diskTimes[5] = 1500;      // 25 min
  defaultTimes[8] = diskTimes[6] = 1800;      // 30 min
  defaultTimes[9] = diskTimes[7] = 2700;      // 45 min
  defaultTimes[10] = diskTimes[8] = 3600;     // 1 hr
  defaultTimes[11] = diskTimes[9] = 7200;     // 2 hrs
  defaultTimes[12] = diskTimes[10] = 10800;   // 3 hrs
  defaultTimes[13] = diskTimes[11] = 14400;   // 4 hrs
  defaultTimes[14] = diskTimes[12] = 18000;   // 5 hrs
  defaultTimes[15] = diskTimes[13] = 0;       // Never

  initPowerBoxes();

  return TRUE;
}

//**************************************************************
// show/modify the CPU data tray icon
//**************************************************************
void CSpeedswitchXPDlg::displayCPUDataIcon( int msg )
{
  NOTIFYICONDATA_CD nid;
  BOOL prev = FALSE;
  TCHAR tp[192];
  int val1, val2, val3;

  if( !options.speedIcon || !options.readCPUSpeed )
    val1 = -3;
  else
  {
    val1 = curSpeed/100;

    int t = curSpeed%100;
    if( t >= 90 )
      val1++;
  }

  val3 = val1;    // use val3 for tooltip only
  if( val3 != -3 )
    val3 = curSpeed;

  if( !options.loadIcon || !options.readCPULoad )
    val2 = -3;
  else
    val2 = curLoad;

  if( msg==NIM_MODIFY && cpuIconVal1==val1 && cpuIconVal2==val2 )
    return;
  
  nid.cbSize = sizeof( NOTIFYICONDATA );
  nid.hWnd = m_hWnd;
  nid.uID = 702;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  nid.uCallbackMessage = WM_ICONNOTIFY;

  *tp = '\0';

  if( val1 != -3 )
  {
    prev = true;
    CString s1;
    s1.LoadStringW( IDS_MAIN37 );
    wsprintf( tp, s1, val3 );
  }

  if( val2 != -3 )
  {
    if( prev )
      _tcscat_s( tp, _T("\n") );

    prev = true;
    CString s1;
    s1.LoadStringW( IDS_MAIN38 );
    wsprintf( &tp[_tcslen(tp)], s1, val2 );
  } 

  lstrcpy( nid.szTip, tp );

  DestroyIcon( cpuDataIcon );    // we're not going to waste resources, so destroy old icon resource !

  if( options.speedIcon && options.loadIcon )
    cpuDataIcon = trayMan.createIcon( true, val1, true, val2 );
  else if( options.speedIcon )
    cpuDataIcon = trayMan.createIcon( true, val1, false, 0 );
  else
    cpuDataIcon = trayMan.createIcon( true, val2, false, 0 );

  nid.hIcon = cpuDataIcon;

  if( msg==NIM_MODIFY && !cpuDataIconActive )
    msg = NIM_ADD;

  Shell_NotifyIcon( msg, (PNOTIFYICONDATA)&nid );

  cpuDataIconActive = TRUE;
  cpuIconVal1 = val1; 
  cpuIconVal2 = val2;
}

//**************************************************************
// remove the CPU data tray icon
//**************************************************************
void CSpeedswitchXPDlg::removeCPUDataIcon()
{
  NOTIFYICONDATA_CD nid;
  nid.cbSize = sizeof( NOTIFYICONDATA );
  nid.hWnd = m_hWnd;
  nid.uID = 702;
  nid.uCallbackMessage = WM_ICONNOTIFY;
  nid.hIcon = cpuDataIcon;
  Shell_NotifyIcon( NIM_DELETE, (PNOTIFYICONDATA)&nid );
  DestroyIcon( cpuDataIcon );
  cpuDataIconActive = FALSE;
  cpuIconVal1 = -2;
  cpuIconVal2 = -2;
}

//**************************************************************
//**************************************************************
void CSpeedswitchXPDlg::fillPolicyStrings( CComboBox* box )
{
  CString s1;
  
  box->ResetContent();

  s1.LoadStringW( IDS_MAIN56 ); box->AddString( s1 );
  s1.LoadStringW( IDS_MAIN57 ); box->AddString( s1 );
  s1.LoadStringW( IDS_MAIN58 ); box->AddString( s1 );
  s1.LoadStringW( IDS_MAIN59 ); box->AddString( s1 );
}
