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
#include "TOptions.h"
#include "SpeedswitchXP.h"
#include "SpeedswitchXPOptions.h"
#include "GeneralOptions.h"
#include "UserOptions.h"
#include "MachOptions.h"
#include "CPUOptions.h"
#include "uxtheme.h"

CGeneralOptions genOpt;
CUserOptions userOpt;
CMachOptions machOpt;
CCPUOptions cpuOpt;

// CSpeedswitchXPOptions dialog

IMPLEMENT_DYNAMIC(CSpeedswitchXPOptions, CDialog)
CSpeedswitchXPOptions::CSpeedswitchXPOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CSpeedswitchXPOptions::IDD, pParent)
{ }

CSpeedswitchXPOptions::~CSpeedswitchXPOptions()
{ }

void CSpeedswitchXPOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TAB1, tabCtrl);
}


BEGIN_MESSAGE_MAP(CSpeedswitchXPOptions, CDialog)
  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
END_MESSAGE_MAP()


BOOL CSpeedswitchXPOptions::OnInitDialog()
{
  log( _T("OnInitDialog von Options") );
	CDialog::OnInitDialog();

	TC_ITEM TabCtrlItem;
	TabCtrlItem.mask = TCIF_TEXT;

  static WCHAR t[256];
  CString s1;

  s1.LoadStringW( IDS_OPT1 );
  _tcscpy_s( t, s1 );

	TabCtrlItem.pszText = t;
	tabCtrl.InsertItem( 0, &TabCtrlItem );

  s1.LoadStringW( IDS_OPT2 );
  _tcscpy_s( t, s1 );

	TabCtrlItem.pszText = t;
	tabCtrl.InsertItem( 1, &TabCtrlItem );

  s1.LoadStringW( IDS_OPT3 );
  _tcscpy_s( t, s1 );

	TabCtrlItem.pszText = t;
  tabCtrl.InsertItem( 2, &TabCtrlItem );
  
  s1.LoadStringW( IDS_OPT4 );
  _tcscpy_s( t, s1 );

	TabCtrlItem.pszText = t;
  tabCtrl.InsertItem( 3, &TabCtrlItem );

  tabs[0] = &genOpt;
  tabs[1] = &userOpt;
  tabs[2] = &machOpt;
  tabs[3] = &cpuOpt;

  genOpt.Create( IDD_GENERALOPT, this );
  userOpt.Create( IDD_USEROPT, this );
  machOpt.Create( IDD_MACHOPT, this );
  cpuOpt.Create( IDD_CPUOPT, this );

  genOpt.ShowWindow( SW_SHOW );
  userOpt.ShowWindow( SW_HIDE );
  machOpt.ShowWindow( SW_HIDE );
  cpuOpt.ShowWindow( SW_HIDE );

  // calculate overlay window position and size
  for( int i=0; i<4; i++ )
  {
    RECT rect;

    tabs[i]->GetClientRect( &rect );
    TabCtrl_AdjustRect( tabCtrl, 0, &rect );
    tabCtrl.ClientToScreen( &rect );
    ScreenToClient( &rect );
    tabs[i]->SetWindowPos( &wndTop,
                           rect.left,
                           rect.top,
                           rect.right - rect.left,
                           rect.bottom - rect.top + 22,
                           (i ? SWP_HIDEWINDOW : SWP_SHOWWINDOW) );

    EnableThemeDialogTexture( tabs[i]->m_hWnd, ETDT_ENABLETAB );
  }

  genOpt.UpdateData( FALSE );
  userOpt.UpdateData( FALSE );
  machOpt.UpdateData( FALSE );
  cpuOpt.UpdateData( FALSE );

  log( _T("OnInitDialog von Options zuende") );

  return TRUE;
}

// CSpeedswitchXPOptions message handlers

void CSpeedswitchXPOptions::OnTcnSelchangeTab1( NMHDR *pNMHDR, LRESULT *pResult )
{
  int i = tabCtrl.GetCurSel();

  for( int x=0; x<4; x++ )
    tabs[x]->ShowWindow( x==i ? SW_SHOW : SW_HIDE );

  *pResult = 0;
}

void CSpeedswitchXPOptions::OnOK()
{
  UpdateData();
  genOpt.UpdateData();
  userOpt.UpdateData();
  machOpt.UpdateData();
  cpuOpt.UpdateData();
  cpuOpt.applyChanges( cpuOpt.m_iCStateConfig );

  CDialog::OnOK();
}

void CSpeedswitchXPOptions::setVars( TOptions& options )
{
  genOpt.m_bAutostart = options.autoStart;
  genOpt.m_bDebugmode = options.debugMode;
  genOpt.m_bMinimizeOnClose = options.minimizeOnClose;
  genOpt.m_bCheckStatus = options.checkStatus;
  genOpt.m_iCheckInterval = options.checkInterval;
  genOpt.m_bReactivate = options.reactivate;
  genOpt.m_bReadCPUSpeed = options.readCPUSpeed;
  genOpt.m_iSpeedMethod = options.speedMethod;
  genOpt.m_bReadCPULoad = options.readCPULoad;
  genOpt.m_bShowDiagram = options.showDiagram;
  genOpt.m_iCPUInterval = options.cpuInterval;
  genOpt.m_iSpeedScaling = options.freqScaling-1;
  genOpt.m_bShowBattery = options.showBattery;
  genOpt.m_iBatteryIndicator = options.batteryIndicator;
  genOpt.m_iBattMethod = options.batteryMethod;
  genOpt.m_bChargeIndicator = options.chargeIndicator;
  genOpt.m_bSpeedIcon = options.speedIcon;
  genOpt.m_bLoadIcon = options.loadIcon;
  genOpt.m_iIconColor1 = options.iconColor1;
  genOpt.m_iIconColor2 = options.iconColor2;
  genOpt.m_bIconTransparent = options.iconTransparent;

  userOpt.setVars();
  machOpt.setVars();
  cpuOpt.setVars();
  power = cpu = FALSE;
}

void CSpeedswitchXPOptions::getVars( TOptions& options )
{
  options.autoStart = genOpt.m_bAutostart;
  options.debugMode = genOpt.m_bDebugmode;
  options.minimizeOnClose = genOpt.m_bMinimizeOnClose;
  options.checkStatus = genOpt.m_bCheckStatus;
  options.checkInterval = genOpt.m_iCheckInterval;
  options.reactivate = genOpt.m_bReactivate;
  options.readCPUSpeed = genOpt.m_bReadCPUSpeed;
  options.speedMethod = genOpt.m_iSpeedMethod;
  options.readCPULoad = genOpt.m_bReadCPULoad;
  options.showDiagram = genOpt.m_bShowDiagram;
  options.cpuInterval = genOpt.m_iCPUInterval;
  options.freqScaling = genOpt.m_iSpeedScaling+1;
  options.showBattery = genOpt.m_bShowBattery;
  options.batteryIndicator = genOpt.m_iBatteryIndicator;
  options.batteryMethod = genOpt.m_iBattMethod;
  options.chargeIndicator = genOpt.m_bChargeIndicator;
  options.speedIcon = genOpt.m_bSpeedIcon;
  options.loadIcon = genOpt.m_bLoadIcon;

  options.iconColor1 = genOpt.m_iIconColor1;
  options.iconColor2 = genOpt.m_iIconColor2;
  options.iconTransparent = genOpt.m_bIconTransparent;

  userOpt.getVars();
  machOpt.getVars();
  cpuOpt.getVars();

  power = userOpt.power || machOpt.power;
  cpu = cpuOpt.cpu;
}
