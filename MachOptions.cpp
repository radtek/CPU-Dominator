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
#include "MachOptions.h"
#include "speedswitch.h"

#define TOOLTIP_ID 1

// CMachOptions dialog

IMPLEMENT_DYNAMIC(CMachOptions, CDialog)
CMachOptions::CMachOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CMachOptions::IDD, pParent)
  , m_iMinSleepAC(0)
  , m_iMinSleepDC(0)
  , m_iRedLatSleepAC(0)
  , m_iRedLatSleepDC(0)
  , m_iDozeTimeoutAC(0)
  , m_iDozeTimeoutDC(0)
  , m_iDozeS4TimeoutAC(0)
  , m_iDozeS4TimeoutDC(0)
  , m_iMinThrottleAC(0)
  , m_iMinThrottleDC(0)
{ }

CMachOptions::~CMachOptions()
{ }

void CMachOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_COMBO3, m_iMinSleepAC);
  DDX_CBIndex(pDX, IDC_COMBO4, m_iMinSleepDC);
  DDX_CBIndex(pDX, IDC_COMBO7, m_iRedLatSleepAC);
  DDX_CBIndex(pDX, IDC_COMBO8, m_iRedLatSleepDC);
  DDX_Text(pDX, IDC_EDIT1, m_iDozeTimeoutAC);
  DDX_Text(pDX, IDC_EDIT2, m_iDozeTimeoutDC);
  DDX_Text(pDX, IDC_EDIT3, m_iDozeS4TimeoutAC);
  DDX_Text(pDX, IDC_EDIT7, m_iDozeS4TimeoutDC);
  DDX_Text(pDX, IDC_EDIT11, m_iMinThrottleAC);
  DDX_Text(pDX, IDC_EDIT12, m_iMinThrottleDC);
}

BOOL CMachOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

  CString s1;

  s1.LoadStringW( IDS_MACH1 );
  addToolTip( 0, IDC_COMBO3, s1 );
  addToolTip( 1, IDC_COMBO4, s1 );

  s1.LoadStringW( IDS_MACH2 );
  addToolTip( 2, IDC_COMBO7, s1 );
  addToolTip( 3, IDC_COMBO8, s1 );

  s1.LoadStringW( IDS_MACH3 );
  addToolTip( 4, IDC_EDIT1, s1 );
  addToolTip( 5, IDC_EDIT2, s1 );

  s1.LoadStringW( IDS_MACH4 );
  addToolTip( 6, IDC_EDIT3, s1 );
  addToolTip( 7, IDC_EDIT7, s1 );

  s1.LoadStringW( IDS_MACH5 );
  addToolTip( 8, IDC_EDIT11, s1 );
  addToolTip( 9, IDC_EDIT12, s1 );

  return TRUE;
}

BOOL CMachOptions::PreTranslateMessage( MSG* pMsg ) 
{
  for( int i=0; i<10; i++ )
    m_tToolTip[i].RelayEvent( pMsg );

  return CDialog::PreTranslateMessage( pMsg );
}

BEGIN_MESSAGE_MAP(CMachOptions, CDialog)
END_MESSAGE_MAP()


// CMachOptions message handlers
void CMachOptions::setVars()
{
  m_iMinSleepAC = internalPolicy.mach.MinSleepAc;
  m_iMinSleepDC = internalPolicy.mach.MinSleepDc;
  m_iRedLatSleepAC = internalPolicy.mach.ReducedLatencySleepAc;
  m_iRedLatSleepDC = internalPolicy.mach.ReducedLatencySleepDc;
  m_iDozeTimeoutAC = internalPolicy.mach.DozeTimeoutAc;
  m_iDozeTimeoutDC = internalPolicy.mach.DozeTimeoutDc;
  m_iDozeS4TimeoutAC = internalPolicy.mach.DozeS4TimeoutAc;
  m_iDozeS4TimeoutDC = internalPolicy.mach.DozeS4TimeoutDc;
  m_iMinThrottleAC = internalPolicy.mach.MinThrottleAc;
  m_iMinThrottleDC = internalPolicy.mach.MinThrottleDc;
}

void CMachOptions::getVars()
{
  if( m_iMinThrottleAC<0 || m_iMinThrottleAC>100 )
    m_iMinThrottleAC = internalPolicy.mach.MinThrottleAc;

  if( m_iMinThrottleDC<0 || m_iMinThrottleDC>100 )
    m_iMinThrottleDC = internalPolicy.mach.MinThrottleDc;

  if( m_iMinSleepAC != internalPolicy.mach.MinSleepAc
  ||  m_iMinSleepDC != internalPolicy.mach.MinSleepDc
  ||  m_iRedLatSleepAC != internalPolicy.mach.ReducedLatencySleepAc
  ||  m_iRedLatSleepDC != internalPolicy.mach.ReducedLatencySleepDc
  ||  m_iDozeTimeoutAC != internalPolicy.mach.DozeTimeoutAc
  ||  m_iDozeTimeoutDC != internalPolicy.mach.DozeTimeoutDc
  ||  m_iDozeS4TimeoutAC != internalPolicy.mach.DozeS4TimeoutAc
  ||  m_iDozeS4TimeoutDC != internalPolicy.mach.DozeS4TimeoutDc
  ||  m_iMinThrottleAC != internalPolicy.mach.MinThrottleAc
  ||  m_iMinThrottleDC != internalPolicy.mach.MinThrottleDc )
    power = TRUE;
  else
  {
    power = FALSE;
    return;
  }

  internalPolicy.mach.MinSleepAc = (SYSTEM_POWER_STATE)m_iMinSleepAC;
  internalPolicy.mach.MinSleepDc = (SYSTEM_POWER_STATE)m_iMinSleepDC;
  internalPolicy.mach.ReducedLatencySleepAc = (SYSTEM_POWER_STATE)m_iRedLatSleepAC;
  internalPolicy.mach.ReducedLatencySleepDc = (SYSTEM_POWER_STATE)m_iRedLatSleepDC;
  internalPolicy.mach.DozeTimeoutAc = m_iDozeTimeoutAC;
  internalPolicy.mach.DozeTimeoutDc = m_iDozeTimeoutDC;
  internalPolicy.mach.DozeS4TimeoutAc = m_iDozeS4TimeoutAC;
  internalPolicy.mach.DozeS4TimeoutDc = m_iDozeS4TimeoutDC;
  internalPolicy.mach.MinThrottleAc = m_iMinThrottleAC;
  internalPolicy.mach.MinThrottleDc = m_iMinThrottleDC;
}

void CMachOptions::addToolTip( int no, int id, CString str )
{
  CWnd* wnd = GetDlgItem( id );
  CRect rect; 
  wnd->GetClientRect( rect );
  m_tToolTip[no].Create( wnd );
  m_tToolTip[no].AddTool( wnd, str, rect, TOOLTIP_ID );
}
