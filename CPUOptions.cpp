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
#include "CPUOptions.h"
#include "speedswitch.h"

#define TOOLTIP_ID 1

// CCPUOptions dialog

IMPLEMENT_DYNAMIC(CCPUOptions, CDialog)
CCPUOptions::CCPUOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CCPUOptions::IDD, pParent)
  , m_iDynThrottleAC(0)
  , m_iDynThrottleDC(0)
  , m_iCStateConfig(0)
{
  m_iTimeCheck[0] = 0;
  m_iTimeCheck[1] = 0;
  m_iTimeCheck[2] = 0;
  m_iDemoteLimit[0] = 0;
  m_iDemoteLimit[1] = 0;
  m_iDemoteLimit[2] = 0;
  m_iPromoteLimit[0] = 0;
  m_iPromoteLimit[1] = 0;
  m_iPromoteLimit[2] = 0;
  m_iDemotePercent[0] = 0;
  m_iDemotePercent[1] = 0;
  m_iDemotePercent[2] = 0;
  m_iPromotePercent[0] = 0;
  m_iPromotePercent[1] = 0;
  m_iPromotePercent[2] = 0;
  m_iAllowDemotion[0] = 1;
  m_iAllowDemotion[1] = 1;
  m_iAllowDemotion[2] = 1;
  m_iAllowPromotion[0] = 1;
  m_iAllowPromotion[1] = 1;
  m_iAllowPromotion[2] = 1;
}

CCPUOptions::~CCPUOptions()
{ }

void CCPUOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_CBIndex(pDX, IDC_COMBO1, m_iDynThrottleAC);
  DDX_CBIndex(pDX, IDC_COMBO2, m_iDynThrottleDC);
  DDX_CBIndex(pDX, IDC_COMBO3, m_iCStateConfig);
  DDX_Text(pDX, IDC_EDIT1, m_iTimeCheck[0]);
  DDX_Text(pDX, IDC_EDIT2, m_iTimeCheck[1]);
  DDX_Text(pDX, IDC_EDIT3, m_iTimeCheck[2]);
  DDX_Text(pDX, IDC_EDIT4, m_iDemoteLimit[0]);
  DDX_Text(pDX, IDC_EDIT13, m_iDemoteLimit[1]);
  DDX_Text(pDX, IDC_EDIT14, m_iDemoteLimit[2]);
  DDX_Text(pDX, IDC_EDIT5, m_iPromoteLimit[0]);
  DDX_Text(pDX, IDC_EDIT6, m_iPromoteLimit[1]);
  DDX_Text(pDX, IDC_EDIT7, m_iPromoteLimit[2]);
  DDX_Text(pDX, IDC_EDIT8, m_iDemotePercent[0]);
  DDX_Text(pDX, IDC_EDIT15, m_iDemotePercent[1]);
  DDX_Text(pDX, IDC_EDIT16, m_iDemotePercent[2]);
  DDX_Text(pDX, IDC_EDIT9, m_iPromotePercent[0]);
  DDX_Text(pDX, IDC_EDIT17, m_iPromotePercent[1]);
  DDX_Text(pDX, IDC_EDIT18, m_iPromotePercent[2]);
  DDX_CBIndex(pDX, IDC_COMBO9, m_iAllowDemotion[0]);
  DDX_CBIndex(pDX, IDC_COMBO10, m_iAllowDemotion[1]);
  DDX_CBIndex(pDX, IDC_COMBO11, m_iAllowDemotion[2]);
  DDX_CBIndex(pDX, IDC_COMBO12, m_iAllowPromotion[0]);
  DDX_CBIndex(pDX, IDC_COMBO13, m_iAllowPromotion[1]);
  DDX_CBIndex(pDX, IDC_COMBO14, m_iAllowPromotion[2]);
}

BOOL CCPUOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

  fillCStateArray( m_iCStateConfig );

  CString s1;

  s1.LoadStringW( IDS_CPOL1 );
  addToolTip( 0, IDC_COMBO3, s1 );

  s1.LoadStringW( IDS_CPOL2 );
  addToolTip( 1, IDC_EDIT1, s1 );
  addToolTip( 2, IDC_EDIT2, s1 );
  addToolTip( 3, IDC_EDIT3, s1 );

  s1.LoadStringW( IDS_CPOL3 );
  addToolTip( 4, IDC_EDIT4, s1 );
  addToolTip( 5, IDC_EDIT13, s1 );
  addToolTip( 6, IDC_EDIT14, s1 );

  s1.LoadStringW( IDS_CPOL4 );
  addToolTip( 7, IDC_EDIT5, s1 );
  addToolTip( 8, IDC_EDIT6, s1 );
  addToolTip( 9, IDC_EDIT7, s1 );

  s1.LoadStringW( IDS_CPOL5 );
  addToolTip( 10, IDC_EDIT8, s1 );
  addToolTip( 11, IDC_EDIT15, s1 );
  addToolTip( 12, IDC_EDIT16, s1 );

  s1.LoadStringW( IDS_CPOL6 );
  addToolTip( 13, IDC_EDIT9, s1 );
  addToolTip( 14, IDC_EDIT17, s1 );
  addToolTip( 15, IDC_EDIT18, s1 );

  s1.LoadStringW( IDS_CPOL7 );
  addToolTip( 16, IDC_COMBO9, s1 );
  addToolTip( 17, IDC_COMBO10, s1 );
  addToolTip( 18, IDC_COMBO11, s1 );

  s1.LoadStringW( IDS_CPOL8 );
  addToolTip( 19, IDC_COMBO12, s1 );
  addToolTip( 20, IDC_COMBO13, s1 );
  addToolTip( 21, IDC_COMBO14, s1 );

  return TRUE;
}

BOOL CCPUOptions::PreTranslateMessage( MSG* pMsg ) 
{
  for( int i=0; i<22; i++ )
    m_tToolTip[i].RelayEvent( pMsg );

  return CDialog::PreTranslateMessage( pMsg );
}


BEGIN_MESSAGE_MAP(CCPUOptions, CDialog)
  ON_CBN_SELCHANGE(IDC_COMBO3, OnCbnSelchangeCombo3)
END_MESSAGE_MAP()


// CCPUOptions message handlers
void CCPUOptions::setVars()
{
  polAC = mach.ProcessorPolicyAc;
  polDC = mach.ProcessorPolicyDc;
//  polAC.PolicyCount = 2;
  m_iCStateConfig = 0;
  m_iDynThrottleAC = mach.ProcessorPolicyAc.DynamicThrottle;
  m_iDynThrottleDC = mach.ProcessorPolicyDc.DynamicThrottle;
  cpu = FALSE;
}

void CCPUOptions::getVars()
{
  if( cpu )
  {
    mach.ProcessorPolicyAc = polAC;
    mach.ProcessorPolicyDc = polDC;
  }
}


void CCPUOptions::OnCbnSelchangeCombo3()
{
  UpdateData();
  applyChanges( m_iCStateConfig==0 ? 1 : 0 );   // invertieren, da schon geändert
  fillCStateArray( m_iCStateConfig );
  UpdateData( FALSE );
}

void CCPUOptions::fillCStateArray( int config )
{
  PROCESSOR_POWER_POLICY* pol;

  if( config == 0 )   // AC
    pol = &polAC;
  else
    pol = &polDC;

  for( int i=0; i<(int)pol->PolicyCount; i++ )
  {
    m_iTimeCheck[i] = pol->Policy[i].TimeCheck;
    m_iDemoteLimit[i] = pol->Policy[i].DemoteLimit;
    m_iPromoteLimit[i] = pol->Policy[i].PromoteLimit;
    m_iDemotePercent[i] = pol->Policy[i].DemotePercent;
    m_iPromotePercent[i] = pol->Policy[i].PromotePercent;
    m_iAllowDemotion[i] = pol->Policy[i].AllowDemotion ? 0 : 1;
    m_iAllowPromotion[i] = pol->Policy[i].AllowPromotion ? 0 : 1;
  }

  for( int i=pol->PolicyCount; i<3; i++ )
  {
    m_iTimeCheck[i] = 0;
    m_iDemoteLimit[i] = 0;
    m_iPromoteLimit[i] = 0;
    m_iDemotePercent[i] = 0;
    m_iPromotePercent[i] = 0;
    m_iAllowDemotion[i] = 1;
    m_iAllowPromotion[i] = 1;
  }

  ((CWnd*)GetDlgItem(IDC_EDIT1))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_EDIT4))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_EDIT5))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_EDIT8))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_EDIT9))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_COMBO9))->EnableWindow( pol->PolicyCount>0 );
  ((CWnd*)GetDlgItem(IDC_COMBO12))->EnableWindow( pol->PolicyCount>0 );

  ((CWnd*)GetDlgItem(IDC_EDIT2))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_EDIT13))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_EDIT6))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_EDIT15))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_EDIT17))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_COMBO10))->EnableWindow( pol->PolicyCount>1 );
  ((CWnd*)GetDlgItem(IDC_COMBO13))->EnableWindow( pol->PolicyCount>1 );

  ((CWnd*)GetDlgItem(IDC_EDIT3))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_EDIT14))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_EDIT7))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_EDIT16))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_EDIT18))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_COMBO11))->EnableWindow( pol->PolicyCount>2 );
  ((CWnd*)GetDlgItem(IDC_COMBO14))->EnableWindow( pol->PolicyCount>2 );
}

void CCPUOptions::applyChanges( int config )
{
  PROCESSOR_POWER_POLICY* pol;

  if( config == 0 )   // AC
    pol = &polAC;
  else
    pol = &polDC;

  for( int i=0; i<(int)pol->PolicyCount; i++ )
  {
    if( m_iDemotePercent[i]<0 || m_iDemotePercent[i]>100 )
      m_iDemotePercent[i] = pol->Policy[i].DemotePercent;

    if( m_iPromotePercent[i]<0 || m_iPromotePercent[i]>100 )
      m_iPromotePercent[i] = pol->Policy[i].PromotePercent;

    if( m_iTimeCheck[i] != pol->Policy[i].TimeCheck
    ||  m_iDemoteLimit[i] != pol->Policy[i].DemoteLimit
    ||  m_iPromoteLimit[i] != pol->Policy[i].PromoteLimit
    ||  m_iDemotePercent[i] != pol->Policy[i].DemotePercent
    ||  m_iPromotePercent[i] != pol->Policy[i].PromotePercent
    ||  (m_iAllowDemotion[i]==0?1:0) != pol->Policy[i].AllowDemotion
    ||  (m_iAllowPromotion[i]==0?1:0) != pol->Policy[i].AllowPromotion )
    {
      cpu = TRUE;
      pol->Policy[i].TimeCheck = m_iTimeCheck[i];
      pol->Policy[i].DemoteLimit = m_iDemoteLimit[i];
      pol->Policy[i].PromoteLimit = m_iPromoteLimit[i];
      pol->Policy[i].DemotePercent = m_iDemotePercent[i];
      pol->Policy[i].PromotePercent = m_iPromotePercent[i];
      pol->Policy[i].AllowDemotion = (m_iAllowDemotion[i]==0?1:0);
      pol->Policy[i].AllowPromotion = (m_iAllowPromotion[i]==0?1:0);
    }    
  }
}

void CCPUOptions::addToolTip( int no, int id, CString str )
{
  CWnd* wnd = GetDlgItem( id );
  CRect rect; 
  wnd->GetClientRect( rect );
  m_tToolTip[no].Create( wnd );
  m_tToolTip[no].AddTool( wnd, str, rect, TOOLTIP_ID );
}