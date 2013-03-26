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
#include "SpeedswitchXP.h"
#include "speedswitch.h"
#include "PowerCapabilities.h"


// CPowerCapabilities dialog

IMPLEMENT_DYNAMIC(CPowerCapabilities, CDialog)
CPowerCapabilities::CPowerCapabilities(CWnd* pParent /*=NULL*/)
	: CDialog(CPowerCapabilities::IDD, pParent)
  , m_szPowerButton(_T(""))
  , m_szSleepButton(_T(""))
  , m_szLidSwitch(_T(""))
  , m_szSStates(_T(""))
  , m_szHiberFile(_T(""))
  , m_szWakeSupport(_T(""))
  , m_szVideoDim(_T(""))
  , m_szAPMBios(_T(""))
  , m_szThermalZones(_T(""))
  , m_szThrottle(_T(""))
  , m_szUPSPresent(_T(""))
  , m_szThrottleMax(_T(""))
  , m_szThrottleMin(_T(""))
  , m_szSpinDown(_T(""))
  , m_szSystemBatteries(_T(""))
  , m_szShortTermBatteries(_T(""))
{ 
  yes.LoadStringW( IDS_PINF19 );
  no.LoadStringW( IDS_PINF20 );
}

CPowerCapabilities::~CPowerCapabilities()
{ }

void CPowerCapabilities::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST1, m_cList);
}


BEGIN_MESSAGE_MAP(CPowerCapabilities, CDialog)
END_MESSAGE_MAP()


// CPowerCapabilities message handlers
BOOL CPowerCapabilities::OnInitDialog()
{
  CDialog::OnInitDialog();

  yes.LoadStringW( IDS_PINF19 );
  no.LoadStringW( IDS_PINF20 );

  CRect rect;
  m_cList.GetClientRect( &rect );
  int nColInterval = rect.Width()/8;

  CString s1;
  s1.LoadStringW( IDS_PINF1 );

  m_cList.InsertColumn( 0, s1, LVCFMT_LEFT, nColInterval*3 );

  s1.LoadStringW( IDS_PINF2 );
  m_cList.InsertColumn( 1, s1, LVCFMT_LEFT, nColInterval*4+nColInterval/2 );

  s1.LoadStringW( IDS_PINF3 );
  int nIndex = m_cList.InsertItem( 0, s1 );
  m_cList.SetItemText( nIndex, 1, m_szPowerButton );

  s1.LoadStringW( IDS_PINF4 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szSleepButton );

  s1.LoadStringW( IDS_PINF5 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szLidSwitch );

  s1.LoadStringW( IDS_PINF6 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szSStates );

  s1.LoadStringW( IDS_PINF7 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szHiberFile );

  s1.LoadStringW( IDS_PINF8 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szWakeSupport );

  s1.LoadStringW( IDS_PINF9 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szVideoDim );

  s1.LoadStringW( IDS_PINF10 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szAPMBios );

  s1.LoadStringW( IDS_PINF11 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szUPSPresent );

  s1.LoadStringW( IDS_PINF12 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szThermalZones );

  s1.LoadStringW( IDS_PINF13 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szThrottle );

  if( m_szThrottle == yes )
  {
    s1.LoadStringW( IDS_PINF14 );
    nIndex = m_cList.InsertItem( nIndex+1, s1 );
    m_cList.SetItemText( nIndex, 1, m_szThrottleMin + _T("%") );

    s1.LoadStringW( IDS_PINF15 );
    nIndex = m_cList.InsertItem( nIndex+1, s1 );
    m_cList.SetItemText( nIndex, 1, m_szThrottleMax + _T("%") );
  }

  s1.LoadStringW( IDS_PINF16 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szSpinDown );

  s1.LoadStringW( IDS_PINF17 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szSystemBatteries );

  s1.LoadStringW( IDS_PINF18 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szShortTermBatteries );

  return TRUE;
}

void CPowerCapabilities::setVars()
{
  yes.LoadStringW( IDS_PINF19 );
  no.LoadStringW( IDS_PINF20 );

  BOOL x = (CallNtPowerInformation(SystemPowerCapabilities,
                                   NULL,
                                   0,
                                   &spc,
                                   sizeof(spc)) == ERROR_SUCCESS);
  if( x )
  {
    m_szPowerButton = (spc.PowerButtonPresent ? yes : no );
    m_szSleepButton = (spc.SleepButtonPresent ? yes : no );
    m_szLidSwitch = (spc.LidPresent ? yes : no );

    wchar_t text[128] = _T("");
    bool first = true;

    if( spc.SystemS1 ) 
    {
      _tcscat_s( text, _T("S1") );
      first = false;
    }

    if( spc.SystemS2 )
    {
      if( !first )
        _tcscat_s( text, _T(", ") );
      _tcscat_s( text, _T("S2") );
      first = false;
    }

    if( spc.SystemS3 )
    {
      if( !first )
        _tcscat_s( text, _T(", ") );
      _tcscat_s( text, _T("S3") );
      first = false;
    }

    if( spc.SystemS4 )
    {
      if( !first )
        _tcscat_s( text, _T(", ") );
      _tcscat_s( text, _T("S4") );
      first = false;
    }

    if( spc.SystemS5 )
    {
      if( !first )
        _tcscat_s( text, _T(", ") );
      _tcscat_s( text, _T("S5") );
      first = false;
    }
    
    m_szSStates = text;
    m_szHiberFile = (spc.HiberFilePresent ? yes : no );
    m_szWakeSupport = (spc.FullWake ? yes : no );
    m_szVideoDim = (spc.VideoDimPresent ? yes : no );
    m_szAPMBios = (spc.ApmPresent ? yes : no );
    m_szUPSPresent = (spc.UpsPresent ? yes : no );
    m_szThermalZones = (spc.ThermalControl ? yes : no );

    BOOLEAN throttle = spc.ProcessorThrottle;

    m_szThrottle = (throttle ? yes : no);

    if( throttle )
    {
      m_szThrottleMax.Format( _T("%d"), spc.ProcessorMinThrottle );
      m_szThrottleMin.Format( _T("%d"), spc.ProcessorMaxThrottle );
    }
    else
    {
      m_szThrottleMax = _T("---");
      m_szThrottleMin = _T("---");
    }

    m_szSpinDown = (spc.DiskSpinDown ? yes : no );
    m_szSystemBatteries = (spc.SystemBatteriesPresent ? yes : no );
    m_szShortTermBatteries = (spc.BatteriesAreShortTerm ? yes : no );
  }
}
