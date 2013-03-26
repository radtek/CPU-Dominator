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
#include "CPUData.h"
#include "PowerCapabilities.h"
#include "SystemInfo.h"
#include "uxtheme.h"

CCPUData cpuData;
CPowerCapabilities powerCaps;

// CSystemInfo dialog

IMPLEMENT_DYNAMIC(CSystemInfo, CDialog)
CSystemInfo::CSystemInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemInfo::IDD, pParent)
{ }

CSystemInfo::~CSystemInfo()
{ }

void CSystemInfo::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TAB3, tabCtrl);
}


BEGIN_MESSAGE_MAP(CSystemInfo, CDialog)
  ON_NOTIFY(TCN_SELCHANGE, IDC_TAB3, OnTcnSelchangeTab3)
END_MESSAGE_MAP()


// CSystemInfo message handlers

BOOL CSystemInfo::OnInitDialog()
{
  CDialog::OnInitDialog();

  CString s1;
  wchar_t t[256];

  s1.LoadStringW( IDS_INFO3 );
  _tcscpy_s( t, s1 );

	TC_ITEM TabCtrlItem;
	TabCtrlItem.mask = TCIF_TEXT;
	TabCtrlItem.pszText = t;
	tabCtrl.InsertItem( 0, &TabCtrlItem );

  s1.LoadStringW( IDS_INFO4 );
  _tcscpy_s( t, s1 );
	TabCtrlItem.pszText = t;
	tabCtrl.InsertItem( 1, &TabCtrlItem );

  tabs[0] = &cpuData;
  tabs[1] = &powerCaps;

  cpuData.Create( IDD_CPUDATA, this );
  powerCaps.Create( IDD_POWERCAPS, this );

  cpuData.ShowWindow( SW_SHOW );
  powerCaps.ShowWindow( SW_HIDE );

  // calculate overlay window position and size
  RECT rect;

  tabCtrl.GetClientRect( &rect );
  ScreenToClient( &rect );

  for( int i=0; i<2; i++ )
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

  cpuData.UpdateData( FALSE );
  powerCaps.UpdateData( FALSE );

  log( _T("OnInitDialog von Sysinfo zuende") );

  return TRUE;
}

void CSystemInfo::OnTcnSelchangeTab3( NMHDR *pNMHDR, LRESULT *pResult )
{
  int i = tabCtrl.GetCurSel();

  for( int x=0; x<2; x++ )
    tabs[x]->ShowWindow( x==i ? SW_SHOW : SW_HIDE );

  *pResult = 0;
}

void CSystemInfo::OnOK()
{
  CDialog::OnOK();
}

void CSystemInfo::setVars()
{
  cpuData.setVars();
  powerCaps.setVars();
}


