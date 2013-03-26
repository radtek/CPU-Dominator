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


// CUserOptions dialog

class CUserOptions : public CDialog
{
	DECLARE_DYNAMIC(CUserOptions)

public:
	CUserOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserOptions();
  void setVars();
  void getVars();
  BOOL power;

// Dialog Data
	enum { IDD = IDD_USEROPT };

protected:
	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage( MSG* pMsg );

	DECLARE_MESSAGE_MAP()

private:
  void addToolTip( int no, int id, CString str );

  int m_iThrottlePolicyAC;
  int m_iThrottlePolicyDC;
  int m_iIdleTimeoutAC;
  int m_iIdleTimeoutDC;
  int m_iIdleSensAC;
  int m_iIdleSensDC;
  int m_iMaxSleepStateAC;
  int m_iMaxSleepStateDC;
  int m_iVideoTimeoutAC;
  int m_iVideoTimeoutDC;
  int m_iSpindownTimeoutAC;
  int m_iSpindownTimeoutDC;
  int m_bOptPowerAC;
  int m_bOptPowerDC;
  int m_iFanThrottleTolAC;
  int m_iFanThrottleTolDC;
  int m_iForcedThrottleAC;
  int m_iForcedThrottleDC;
  CToolTipCtrl m_tToolTip[18];
};
