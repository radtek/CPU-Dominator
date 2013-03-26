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

#pragma once
#include "afxcmn.h"


// CPowerCapabilities dialog

class CPowerCapabilities : public CDialog
{
	DECLARE_DYNAMIC(CPowerCapabilities)

public:
	CPowerCapabilities(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPowerCapabilities();
  virtual BOOL OnInitDialog();
  void setVars();

// Dialog Data
	enum { IDD = IDD_POWERCAPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
  SYSTEM_POWER_CAPABILITIES spc;

	DECLARE_MESSAGE_MAP()
  CString m_szPowerButton;
  CString m_szSleepButton;
  CString m_szLidSwitch;
  CString m_szSStates;
  CString m_szHiberFile;
  CString m_szWakeSupport;
  CString m_szVideoDim;
  CString m_szAPMBios;
  CString m_szThermalZones;
  CString m_szThrottle;
  CString m_szUPSPresent;
  CString m_szThrottleMax;
  CString m_szThrottleMin;
  CString m_szSpinDown;
  CString m_szSystemBatteries;
  CString m_szShortTermBatteries;
  CListCtrl m_cList;
  CString yes;
  CString no;
};
