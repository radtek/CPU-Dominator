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


// CMachOptions dialog

class CMachOptions : public CDialog
{
	DECLARE_DYNAMIC(CMachOptions)

public:
	CMachOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMachOptions();
  void setVars();
  void getVars();
  BOOL power;

// Dialog Data
	enum { IDD = IDD_MACHOPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
  virtual BOOL PreTranslateMessage( MSG* pMsg );

	DECLARE_MESSAGE_MAP()

private:
  void addToolTip( int no, int id, CString str );

  int m_iMinSleepAC;
  int m_iMinSleepDC;
  int m_iRedLatSleepAC;
  int m_iRedLatSleepDC;
  int m_iDozeTimeoutAC;
  int m_iDozeTimeoutDC;
  int m_iDozeS4TimeoutAC;
  int m_iDozeS4TimeoutDC;
  int m_iMinThrottleAC;
  int m_iMinThrottleDC;
  CToolTipCtrl m_tToolTip[10];
};
