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
#include "afxwin.h"


// CCPUOptions dialog

class CCPUOptions : public CDialog
{
	DECLARE_DYNAMIC(CCPUOptions)

public:
	CCPUOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCPUOptions();
  void setVars();
  void getVars();
  BOOL cpu;
  void applyChanges( int config );
  int m_iCStateConfig;

// Dialog Data
	enum { IDD = IDD_CPUOPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

private:
  void fillCStateArray( int config );
  void addToolTip( int no, int id, CString str );

  int m_iDynThrottleAC;
  int m_iDynThrottleDC;
  int m_iTimeCheck[3];
  int m_iDemoteLimit[3];
  int m_iPromoteLimit[3];
  int m_iDemotePercent[3];
  int m_iPromotePercent[3];
  int m_iAllowDemotion[3];
  int m_iAllowPromotion[3];
  PROCESSOR_POWER_POLICY polAC;
  PROCESSOR_POWER_POLICY polDC;
  CToolTipCtrl m_tToolTip[22];

public:
  afx_msg void OnCbnSelchangeCombo3();
};
