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

#pragma once

#include "ColorComboEx.h"

// CGeneralOptions dialog

class CGeneralOptions : public CDialog
{
	DECLARE_DYNAMIC(CGeneralOptions)
public:
	CGeneralOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGeneralOptions();
  BOOL m_bAutostart;
  BOOL m_bDebugmode;
  BOOL m_bMinimizeOnClose;
  BOOL m_bCheckStatus;
  int m_iCheckInterval;
  BOOL m_bReactivate;
  BOOL m_bReadCPUSpeed;
  int m_iSpeedMethod;
  BOOL m_bReadCPULoad;
  BOOL m_bShowDiagram;
  int m_iCPUInterval;
  int m_iSpeedScaling;
  int m_iBattMethod;
  BOOL m_bShowBattery;
  int m_iBatteryIndicator;
  BOOL m_bChargeIndicator;
  BOOL m_bSpeedIcon;
  BOOL m_bLoadIcon;
  CColorComboEx m_cIconColor1;
  CColorComboEx m_cIconColor2;
  int m_iIconColor1;
  int m_iIconColor2;
  BOOL m_bIconTransparent;

// Overrides

// Dialog Data
  enum { IDD = IDD_GENERALOPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedCheck2();
  afx_msg void OnBnClickedCheck4();
  afx_msg void OnBnClickedCheck6();
  afx_msg void OnBnClickedCheck8();
  afx_msg void OnBnClickedCheck7();
  afx_msg void OnBnClickedShowbatt();

	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedTransparent();
};
