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
#include "GeneralOptions.h"


// CGeneralOptions dialog

IMPLEMENT_DYNAMIC(CGeneralOptions, CDialog)

CGeneralOptions::CGeneralOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CGeneralOptions::IDD, pParent)
  , m_bAutostart(FALSE)
  , m_bDebugmode(FALSE)
  , m_bMinimizeOnClose(FALSE)
  , m_bCheckStatus(FALSE)
  , m_iCheckInterval(5)
  , m_bReactivate(FALSE)
  , m_bReadCPUSpeed(FALSE)
  , m_iSpeedMethod(1)
  , m_bReadCPULoad(FALSE)
  , m_bShowDiagram(FALSE)
  , m_iCPUInterval(2)
  , m_iSpeedScaling(0)
  , m_iBattMethod(0)
  , m_bShowBattery(FALSE)
  , m_iBatteryIndicator(0)
  , m_bChargeIndicator(FALSE)
  , m_bSpeedIcon(FALSE)
  , m_bLoadIcon(FALSE)
{ }

CGeneralOptions::~CGeneralOptions()
{ }

void CGeneralOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CHECK1, m_bAutostart);
  DDX_Check(pDX, IDC_CHECK2, m_bDebugmode);
  DDX_Check(pDX, IDC_CHECK3, m_bMinimizeOnClose);
  DDX_Check(pDX, IDC_CHECK4, m_bCheckStatus);
  DDX_Text(pDX, IDC_EDIT1, m_iCheckInterval);
  DDX_Check(pDX, IDC_CHECK5, m_bReactivate);
  DDX_Check(pDX, IDC_CHECK6, m_bReadCPUSpeed);
  DDX_CBIndex(pDX, IDC_COMBO2, m_iSpeedMethod);
  DDX_Check(pDX, IDC_CHECK7, m_bReadCPULoad);
  DDX_Check(pDX, IDC_CHECK8, m_bShowDiagram);
  DDX_Text(pDX, IDC_EDIT3, m_iCPUInterval);
  DDX_CBIndex(pDX, IDC_SCALING, m_iSpeedScaling);
  DDX_CBIndex(pDX, IDC_BATTMET, m_iBattMethod);
  DDX_Check(pDX, IDC_SHOWBATT, m_bShowBattery);
  DDX_CBIndex(pDX, IDC_BATTIND, m_iBatteryIndicator);
  DDX_Check(pDX, IDC_CHARGECHECK, m_bChargeIndicator);
  DDX_Check(pDX, IDC_CHECK10, m_bSpeedIcon);
  DDX_Check(pDX, IDC_CHECK11, m_bLoadIcon);
  DDX_Control(pDX, IDC_COMBO1, m_cIconColor1);
  DDX_Control(pDX, IDC_COMBO3, m_cIconColor2);
  DDX_CBIndex(pDX, IDC_COMBO1, m_iIconColor1);
  DDX_CBIndex(pDX, IDC_COMBO3, m_iIconColor2);
  DDX_Check(pDX, IDC_CHECK12, m_bIconTransparent);
}

BOOL CGeneralOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

  ((CWnd*)GetDlgItem(IDC_EDIT1))->EnableWindow( m_bCheckStatus );
  ((CWnd*)GetDlgItem(IDC_COMBO2))->EnableWindow( m_bReadCPUSpeed );
  ((CWnd*)GetDlgItem(IDC_CHECK10))->EnableWindow( m_bReadCPUSpeed );
  ((CWnd*)GetDlgItem(IDC_CHECK11))->EnableWindow( m_bReadCPULoad );
  ((CWnd*)GetDlgItem(IDC_STATIC8))->EnableWindow( m_bShowDiagram );
  ((CWnd*)GetDlgItem(IDC_SCALING))->EnableWindow( m_bShowDiagram );

  if( !m_bReadCPUSpeed && !m_bReadCPULoad )
  {
    m_bShowDiagram = FALSE;
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_STATIC8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_SCALING))->EnableWindow( FALSE );
  }
  else
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( TRUE );

  bool x = (m_bReadCPUSpeed || m_bReadCPULoad || m_bShowBattery);
  ((CWnd*)GetDlgItem(IDC_STATIC3))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_STATIC4))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_EDIT3))->EnableWindow( x );

  ((CWnd*)GetDlgItem(IDC_BATTIND))->EnableWindow( m_bShowBattery );
  ((CWnd*)GetDlgItem(IDC_BATTMET))->EnableWindow( m_bShowBattery );
  ((CWnd*)GetDlgItem(IDC_CHARGECHECK))->EnableWindow( m_bShowBattery );

  ((CWnd*)GetDlgItem(IDC_STATIC333))->EnableWindow( !m_bIconTransparent );
  ((CWnd*)GetDlgItem(IDC_COMBO3))->EnableWindow( !m_bIconTransparent );

  return TRUE;
}

BEGIN_MESSAGE_MAP(CGeneralOptions, CDialog)
  ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
  ON_BN_CLICKED(IDC_CHECK4, OnBnClickedCheck4)
  ON_BN_CLICKED(IDC_CHECK6, OnBnClickedCheck6)
  ON_BN_CLICKED(IDC_CHECK8, OnBnClickedCheck8)
  ON_BN_CLICKED(IDC_CHECK7, OnBnClickedCheck7)
  ON_BN_CLICKED(IDC_SHOWBATT, OnBnClickedShowbatt)
  ON_BN_CLICKED(IDC_CHECK12, &CGeneralOptions::OnBnClickedTransparent)
END_MESSAGE_MAP()


// CGeneralOptions message handlers

void CGeneralOptions::OnBnClickedCheck2()
{
  UpdateData();	

  if( m_bDebugmode )
  {
    CString s1, s2;
    s1.LoadStringW( IDS_GEN32 );
    s2.LoadStringW( IDS_GEN33 );

    if( MessageBox(s1,s2,MB_ICONEXCLAMATION|MB_YESNO) != IDYES )
    {
      m_bDebugmode = FALSE;
      UpdateData( FALSE );
    }
  }
}

// enable/disable check power scheme integrity
void CGeneralOptions::OnBnClickedCheck4()
{
  UpdateData();
  ((CWnd*)GetDlgItem(IDC_EDIT1))->EnableWindow( m_bCheckStatus );
}

// enable/disable CPU speed reading
void CGeneralOptions::OnBnClickedCheck6()
{
  UpdateData();

  ((CWnd*)GetDlgItem(IDC_COMBO2))->EnableWindow( m_bReadCPUSpeed );
  ((CWnd*)GetDlgItem(IDC_CHECK10))->EnableWindow( m_bReadCPUSpeed );

  if( !m_bReadCPUSpeed )
    m_bSpeedIcon = FALSE;

  if( !m_bReadCPUSpeed && !m_bReadCPULoad )
  {
    m_bShowDiagram = FALSE;
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_STATIC8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_SCALING))->EnableWindow( FALSE );
  }
  else
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( TRUE );

  bool x = (m_bReadCPUSpeed || m_bReadCPULoad || m_bShowBattery);
  ((CWnd*)GetDlgItem(IDC_STATIC3))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_STATIC4))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_EDIT3))->EnableWindow( x );

  UpdateData( FALSE );
}

// enable/disable history diagram
void CGeneralOptions::OnBnClickedCheck8()
{
  UpdateData();

  ((CWnd*)GetDlgItem(IDC_STATIC8))->EnableWindow( m_bShowDiagram );
  ((CWnd*)GetDlgItem(IDC_SCALING))->EnableWindow( m_bShowDiagram );
}

// enable/disable CPU load reading
void CGeneralOptions::OnBnClickedCheck7()
{
  UpdateData();

  ((CWnd*)GetDlgItem(IDC_CHECK11))->EnableWindow( m_bReadCPULoad );

  if( !m_bReadCPULoad )
    m_bLoadIcon = FALSE;

  if( !m_bReadCPUSpeed && !m_bReadCPULoad )
  {
    m_bShowDiagram = FALSE;
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_STATIC8))->EnableWindow( FALSE );
    ((CWnd*)GetDlgItem(IDC_SCALING))->EnableWindow( FALSE );
  }
  else
    ((CWnd*)GetDlgItem(IDC_CHECK8))->EnableWindow( TRUE );

  bool x = (m_bReadCPUSpeed || m_bReadCPULoad || m_bShowBattery);
  ((CWnd*)GetDlgItem(IDC_STATIC3))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_STATIC4))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_EDIT3))->EnableWindow( x );

  UpdateData( FALSE );
}

void CGeneralOptions::OnBnClickedShowbatt()
{
  UpdateData();

  ((CWnd*)GetDlgItem(IDC_BATTIND))->EnableWindow( m_bShowBattery );
  ((CWnd*)GetDlgItem(IDC_BATTMET))->EnableWindow( m_bShowBattery );
  ((CWnd*)GetDlgItem(IDC_CHARGECHECK))->EnableWindow( m_bShowBattery );

  if( !m_bShowBattery )
    m_bChargeIndicator = FALSE;

  bool x = (m_bReadCPUSpeed || m_bReadCPULoad || m_bShowBattery);
  ((CWnd*)GetDlgItem(IDC_STATIC3))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_STATIC4))->EnableWindow( x );
  ((CWnd*)GetDlgItem(IDC_EDIT3))->EnableWindow( x );

  UpdateData( FALSE );
}

void CGeneralOptions::OnBnClickedTransparent()
{
  UpdateData();

  ((CWnd*)GetDlgItem(IDC_STATIC333))->EnableWindow( !m_bIconTransparent );
  ((CWnd*)GetDlgItem(IDC_COMBO3))->EnableWindow( !m_bIconTransparent );
}
