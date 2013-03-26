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
#include "UserOptions.h"
#include "speedswitch.h"

#define TOOLTIP_ID 1

// CUserOptions dialog

IMPLEMENT_DYNAMIC(CUserOptions, CDialog)
CUserOptions::CUserOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CUserOptions::IDD, pParent)
  , m_iThrottlePolicyAC(0)
  , m_iThrottlePolicyDC(0)
  , m_iIdleTimeoutAC(0)
  , m_iIdleTimeoutDC(0)
  , m_iIdleSensAC(0)
  , m_iIdleSensDC(0)
  , m_iMaxSleepStateAC(0)
  , m_iMaxSleepStateDC(0)
  , m_iVideoTimeoutAC(0)
  , m_iVideoTimeoutDC(0)
  , m_iSpindownTimeoutAC(0)
  , m_iSpindownTimeoutDC(0)
  , m_bOptPowerAC(0)
  , m_bOptPowerDC(0)
  , m_iFanThrottleTolAC(0)
  , m_iFanThrottleTolDC(0)
  , m_iForcedThrottleAC(0)
  , m_iForcedThrottleDC(0)
{ }

CUserOptions::~CUserOptions()
{ }

void CUserOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_COMBO1, m_iThrottlePolicyAC);
  DDX_CBIndex(pDX, IDC_COMBO2, m_iThrottlePolicyDC);
  DDX_Text(pDX, IDC_EDIT1, m_iIdleTimeoutAC);
  DDX_Text(pDX, IDC_EDIT2, m_iIdleTimeoutDC);
  DDX_Text(pDX, IDC_EDIT3, m_iIdleSensAC);
  DDX_Text(pDX, IDC_EDIT4, m_iIdleSensDC);
  DDX_CBIndex(pDX, IDC_COMBO3, m_iMaxSleepStateAC);
  DDX_CBIndex(pDX, IDC_COMBO4, m_iMaxSleepStateDC);
  DDX_Text(pDX, IDC_EDIT5, m_iVideoTimeoutAC);
  DDX_Text(pDX, IDC_EDIT7, m_iVideoTimeoutDC);
  DDX_Text(pDX, IDC_EDIT8, m_iSpindownTimeoutAC);
  DDX_Text(pDX, IDC_EDIT6, m_iSpindownTimeoutDC);
  DDX_CBIndex(pDX, IDC_COMBO5, m_bOptPowerAC);
  DDX_CBIndex(pDX, IDC_COMBO6, m_bOptPowerDC);
  DDX_Text(pDX, IDC_EDIT10, m_iFanThrottleTolAC);
  DDX_Text(pDX, IDC_EDIT9, m_iFanThrottleTolDC);
  DDX_Text(pDX, IDC_EDIT11, m_iForcedThrottleAC);
  DDX_Text(pDX, IDC_EDIT12, m_iForcedThrottleDC);
}

BOOL CUserOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

  CString s1;

  s1.LoadStringW( IDS_CPUPOL1 );

  addToolTip( 0, IDC_EDIT1, s1 );
  addToolTip( 1, IDC_EDIT2, s1 );

  s1.LoadStringW( IDS_CPUPOL2 );
  addToolTip( 2, IDC_EDIT3, s1 );
  addToolTip( 3, IDC_EDIT4, s1 );

  s1.LoadStringW( IDS_CPUPOL3 );
  addToolTip( 4, IDC_COMBO1, s1 );
  addToolTip( 5, IDC_COMBO2, s1 );

  s1.LoadStringW( IDS_CPUPOL4 );
  addToolTip( 6, IDC_COMBO3, s1 );
  addToolTip( 7, IDC_COMBO4, s1 );

  s1.LoadStringW( IDS_CPUPOL5 );
  addToolTip( 8, IDC_EDIT5, s1 );
  addToolTip( 9, IDC_EDIT7, s1 );

  s1.LoadStringW( IDS_CPUPOL6 );
  addToolTip( 10, IDC_EDIT8, s1 );
  addToolTip( 11, IDC_EDIT6, s1 );

  s1.LoadStringW( IDS_CPUPOL7 );
  addToolTip( 12, IDC_COMBO5, s1 );
  addToolTip( 13, IDC_COMBO6, s1 );

  s1.LoadStringW( IDS_CPUPOL8 );
  addToolTip( 14, IDC_EDIT10, s1 );
  addToolTip( 15, IDC_EDIT9, s1 );

  s1.LoadStringW( IDS_CPUPOL9 );
  addToolTip( 16, IDC_EDIT11, s1 );
  addToolTip( 17, IDC_EDIT12, s1 );

  return TRUE;
}

BOOL CUserOptions::PreTranslateMessage( MSG* pMsg ) 
{
  for( int i=0; i<18; i++ )
    m_tToolTip[i].RelayEvent( pMsg );

  return CDialog::PreTranslateMessage( pMsg );
}


BEGIN_MESSAGE_MAP(CUserOptions, CDialog)
END_MESSAGE_MAP()

// CUserOptions message handlers
void CUserOptions::setVars()
{
  m_iIdleTimeoutAC = internalPolicy.user.IdleTimeoutAc;
  m_iIdleTimeoutDC = internalPolicy.user.IdleTimeoutDc;
  m_iIdleSensAC = internalPolicy.user.IdleSensitivityAc;
  m_iIdleSensDC = internalPolicy.user.IdleSensitivityDc;
  m_iThrottlePolicyAC = internalPolicy.user.ThrottlePolicyAc;
  m_iThrottlePolicyDC = internalPolicy.user.ThrottlePolicyDc;
  m_iMaxSleepStateAC = internalPolicy.user.MaxSleepAc;
  m_iMaxSleepStateDC = internalPolicy.user.MaxSleepDc;
  m_iVideoTimeoutAC = internalPolicy.user.VideoTimeoutAc;
  m_iVideoTimeoutDC = internalPolicy.user.VideoTimeoutDc;
  m_iSpindownTimeoutAC = internalPolicy.user.SpindownTimeoutAc;
  m_iSpindownTimeoutDC = internalPolicy.user.SpindownTimeoutDc;
  m_bOptPowerAC = internalPolicy.user.OptimizeForPowerAc ? 0 : 1;
  m_bOptPowerDC = internalPolicy.user.OptimizeForPowerDc ? 0 : 1;
  m_iFanThrottleTolAC = internalPolicy.user.FanThrottleToleranceAc;
  m_iFanThrottleTolDC = internalPolicy.user.FanThrottleToleranceDc;
  m_iForcedThrottleAC = internalPolicy.user.ForcedThrottleAc;
  m_iForcedThrottleDC = internalPolicy.user.ForcedThrottleDc;
}

void CUserOptions::getVars()
{
  if( m_iIdleSensAC<0 || m_iIdleSensAC>100 )
    m_iIdleSensAC = internalPolicy.user.IdleSensitivityAc;

  if( m_iIdleSensDC<0 || m_iIdleSensDC>100 )
    m_iIdleSensDC = internalPolicy.user.IdleSensitivityDc;

  if( m_iFanThrottleTolAC<0 || m_iFanThrottleTolAC>100 )
    m_iFanThrottleTolAC = internalPolicy.user.FanThrottleToleranceAc;

  if( m_iFanThrottleTolDC<0 || m_iFanThrottleTolDC>100 )
    m_iFanThrottleTolDC = internalPolicy.user.FanThrottleToleranceDc;

  if( m_iForcedThrottleAC<0 || m_iForcedThrottleAC>100 )
    m_iForcedThrottleAC = internalPolicy.user.ForcedThrottleAc;

  if( m_iForcedThrottleDC<0 || m_iForcedThrottleDC>100 )
    m_iForcedThrottleDC = internalPolicy.user.ForcedThrottleDc;

  if( m_iIdleTimeoutAC != internalPolicy.user.IdleTimeoutAc
  ||  m_iIdleTimeoutDC != internalPolicy.user.IdleTimeoutDc
  ||  m_iIdleSensAC != internalPolicy.user.IdleSensitivityAc
  ||  m_iIdleSensDC != internalPolicy.user.IdleSensitivityDc
  ||  m_iThrottlePolicyAC != internalPolicy.user.ThrottlePolicyAc
  ||  m_iThrottlePolicyDC != internalPolicy.user.ThrottlePolicyDc
  ||  ((SYSTEM_POWER_STATE)m_iMaxSleepStateAC) != internalPolicy.user.MaxSleepAc
  ||  ((SYSTEM_POWER_STATE)m_iMaxSleepStateDC) != internalPolicy.user.MaxSleepDc
  ||  m_iVideoTimeoutAC != internalPolicy.user.VideoTimeoutAc
  ||  m_iVideoTimeoutDC != internalPolicy.user.VideoTimeoutDc
  ||  m_iSpindownTimeoutAC != internalPolicy.user.SpindownTimeoutAc
  ||  m_iSpindownTimeoutDC != internalPolicy.user.SpindownTimeoutDc
  ||  m_bOptPowerAC != (internalPolicy.user.OptimizeForPowerAc ? 0 : 1)
  ||  m_bOptPowerDC != (internalPolicy.user.OptimizeForPowerDc ? 0 : 1)
  ||  m_iFanThrottleTolAC != internalPolicy.user.FanThrottleToleranceAc
  ||  m_iFanThrottleTolDC != internalPolicy.user.FanThrottleToleranceDc
  ||  m_iForcedThrottleAC != internalPolicy.user.ForcedThrottleAc
  ||  m_iForcedThrottleDC != internalPolicy.user.ForcedThrottleDc )
    power = TRUE;
  else
  {
    power = FALSE;
    return;
  }

  internalPolicy.user.IdleTimeoutAc = m_iIdleTimeoutAC;
  internalPolicy.user.IdleTimeoutDc = m_iIdleTimeoutDC;
  internalPolicy.user.IdleSensitivityAc = m_iIdleSensAC;
  internalPolicy.user.IdleSensitivityDc = m_iIdleSensDC;
  internalPolicy.user.ThrottlePolicyAc = m_iThrottlePolicyAC;
  internalPolicy.user.ThrottlePolicyDc = m_iThrottlePolicyDC;
  internalPolicy.user.MaxSleepAc = (SYSTEM_POWER_STATE)m_iMaxSleepStateAC;
  internalPolicy.user.MaxSleepDc = (SYSTEM_POWER_STATE)m_iMaxSleepStateDC;
  internalPolicy.user.VideoTimeoutAc = m_iVideoTimeoutAC;
  internalPolicy.user.VideoTimeoutDc = m_iVideoTimeoutDC;
  internalPolicy.user.SpindownTimeoutAc = m_iSpindownTimeoutAC;
  internalPolicy.user.SpindownTimeoutDc = m_iSpindownTimeoutDC;
  internalPolicy.user.OptimizeForPowerAc = m_bOptPowerAC==0;
  internalPolicy.user.OptimizeForPowerDc = m_bOptPowerDC==0;
  internalPolicy.user.FanThrottleToleranceAc = m_iFanThrottleTolAC;
  internalPolicy.user.FanThrottleToleranceDc = m_iFanThrottleTolDC;
  internalPolicy.user.ForcedThrottleAc = m_iForcedThrottleAC;
  internalPolicy.user.ForcedThrottleDc = m_iForcedThrottleDC;
}

void CUserOptions::addToolTip( int no, int id, CString str )
{
  CWnd* wnd = GetDlgItem( id );
  CRect rect; 
  wnd->GetClientRect( rect );
  m_tToolTip[no].Create( wnd );
  m_tToolTip[no].AddTool( wnd, str, rect, TOOLTIP_ID );
}
