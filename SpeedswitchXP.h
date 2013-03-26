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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// Hauptsymbole

void log( TCHAR* msg, ... );

// CSpeedswitchXPApp:
// Siehe SpeedswitchXP.cpp für die Implementierung dieser Klasse
//

class CSpeedswitchXPApp : public CWinApp
{
public:
	CSpeedswitchXPApp();

// Überschreibungen
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()

private:
  HANDLE hMutex;
  BOOL activeMutex;
  HINSTANCE m_hInstResDLL;
};

extern CSpeedswitchXPApp theApp;
