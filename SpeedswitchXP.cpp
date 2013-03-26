/*
   SpeedswitchXP V1.5
   - Windows XP CPU Frequency Control for Notebooks -

   Copyright(c) 2002-2005 Christian Diefer
 
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
#include "SpeedswitchXP.h"
#include "TOptions.h"
#include "SpeedswitchXPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCHAR logPath[264];

// CSpeedswitchXPApp

BEGIN_MESSAGE_MAP(CSpeedswitchXPApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// CSpeedswitchXPApp-Erstellung
//---------------------------------------------------------------------------
CSpeedswitchXPApp::CSpeedswitchXPApp()
{
  activeMutex = FALSE;
}

// Das einzige CSSpeedswitchXPApp-Objekt
CSpeedswitchXPApp theApp;
CSpeedswitchXPDlg* dlg;
static CString error;

//---------------------------------------------------------------------------
// CSpeedswitchXPApp Initialisierung
//---------------------------------------------------------------------------
BOOL CSpeedswitchXPApp::InitInstance()
{
	// InitCommonControls() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	InitCommonControls();

//  m_hInstResDLL = LoadLibraryW( _T("SpeedswitchXPDEU.dll") );
//  ASSERT( m_hInstResDLL != NULL );

//  AfxSetResourceHandle( m_hInstResDLL );

	CWinApp::InitInstance();

	// make sure that only one instance is running
  BOOL	ret = FALSE;
	TCHAR* mutexName = _T("SpeedswitchXP_mutex");

  hMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, mutexName );
	if( hMutex == NULL )
	{
		hMutex = CreateMutex( 0, TRUE, mutexName );
		DWORD err = GetLastError();
		ret = !(hMutex==NULL || err==ERROR_ALREADY_EXISTS);
    if( hMutex != NULL )
      activeMutex = TRUE;
	}
	else
		CloseHandle( hMutex );

	if( ret == FALSE )
  {
    CString s1, s2;
    s1.LoadStringW( IDS_START1 );
    s2.LoadStringW( IDS_START2 );
    MessageBox( NULL, s1, s2, MB_ICONSTOP|MB_OK );
    return FALSE;
  }

  error.LoadStringW( IDS_MAINERR2 );
  
  // create log filename
  if( GetModuleFileName(NULL,logPath,255) == 0 )
  {
    CString s1;
    s1.LoadStringW( IDS_START3 );
    CloseHandle( hMutex );
    MessageBox( NULL, s1, error, MB_OK|MB_ICONSTOP );
    return FALSE;
  }

  int i = (int)_tcslen(logPath) - 1;
  while( i>=0 && logPath[i]!='.' )
    i--;

  if( i >= 0 )
    _tcscpy_s( &logPath[i], 264-i-3, _T(".log") );
  else
  {
    CString s1;
    s1.LoadStringW( IDS_START4 );
    CloseHandle( hMutex );
    MessageBox( NULL, s1, error, MB_ICONSTOP|MB_OK );
    return FALSE;
  }

  // create and display the main window
  dlg = new CSpeedswitchXPDlg();
  m_pMainWnd = dlg;

  if( m_pMainWnd )
  {
    log( _T("Mutex: 0x%08x"), hMutex );
    log( _T("Init complete") );
    log( _T("--------------------------------") );
    dlg->setCmdLine( m_lpCmdLine );
    return TRUE;
  }
  else
    return FALSE;
}

//---------------------------------------------------------------------------
// Terminate program instance
//---------------------------------------------------------------------------
int CSpeedswitchXPApp::ExitInstance() 
{
  if( activeMutex )  
  {
    log( _T("/bReleasing mutex (0x%08x)... "), hMutex );
    if( CloseHandle(hMutex) )
      log( _T("/aOk") );
    else
      log( _T("/aError") );
  }

  //FreeLibrary( m_hInstResDLL );

	return CWinApp::ExitInstance();
}

//---------------------------------------------------------------------------
// print out a debug message
//---------------------------------------------------------------------------
void log( TCHAR* msg, ... )
{
  if( !options.debugMode )
    return;

  BOOL noDate=FALSE, noCR=FALSE;
  static TCHAR debugBuf[32768];
  va_list ap;
  va_start( ap, msg );

  time_t zeit = time( NULL );
  struct tm ts;
  int err = localtime_s( &ts, &zeit );

  *debugBuf = '\0';

  if( *msg == '/' )
  {
    switch( msg[1] )
    {
      case 'a': noDate=TRUE;
                msg+=2;  
                break;

      case 'b': noCR=TRUE;
                msg+=2;  
                break;

      case 'c': noDate=TRUE;
                noCR=TRUE;
                msg+=2;  
                break;
    }
  }

  if( !noDate )
    wsprintf( debugBuf,
              _T("%02d:%02d:%02d "),
              ts.tm_hour,
              ts.tm_min,
              ts.tm_sec );

  _vstprintf_s( &debugBuf[_tcslen(debugBuf)], 32768-_tcslen(debugBuf)-1, msg, ap );
  va_end( ap );

  if( !noCR )
    _tcscat_s( debugBuf, _T("\r\n") );

  HANDLE hdl = CreateFile( logPath, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
  if( hdl == INVALID_HANDLE_VALUE )
  {
    options.debugMode = FALSE;
    CString s1;
    s1.LoadStringW( IDS_START5 );
    wsprintf( debugBuf, s1, GetLastError() );
    MessageBox( NULL, debugBuf, error, MB_ICONSTOP|MB_OK );
    return;
  }

  if( SetFilePointer(hdl,0,NULL,FILE_END) == INVALID_SET_FILE_POINTER )
  {
    CloseHandle( hdl );
    options.debugMode = FALSE;
    CString s1;
    s1.LoadStringW( IDS_START6 );
    wsprintf( debugBuf, s1, GetLastError() );
    MessageBox( NULL, debugBuf, error, MB_ICONSTOP|MB_OK );
    return;
  }

  DWORD written;
  if( WriteFile(hdl,debugBuf,(DWORD)_tcslen(debugBuf),&written,NULL) == 0 )
  {
    CloseHandle( hdl );
    options.debugMode = FALSE;
    CString s1;
    s1.LoadStringW( IDS_START7 );
    wsprintf( debugBuf, s1, GetLastError() );
    MessageBox( NULL, debugBuf, error, MB_ICONSTOP|MB_OK );
    return;
  }

  CloseHandle( hdl );
}

