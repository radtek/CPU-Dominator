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
#include "afxwin.h"

// CSpeedswitchXPOptions dialog

class CSpeedswitchXPOptions : public CDialog
{
	DECLARE_DYNAMIC(CSpeedswitchXPOptions)

public:
	CSpeedswitchXPOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpeedswitchXPOptions();
  void setVars( TOptions& options );
  void getVars( TOptions& options );
  BOOL power;
  BOOL cpu;

// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
  afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
  CTabCtrl tabCtrl;
  CDialog* tabs[4];
};
