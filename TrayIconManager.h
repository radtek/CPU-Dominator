/*
   SpeedswitchXP V1.5
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

#ifndef __TRAYICONMANAGER_H
#define __TRAYICONMANAGER_H

extern HWND glb_hWnd;
extern TOptions options;

class TTrayIconManager {
public:
  HICON createIcon( bool upperStatus, int upperValue, bool lowerStatus, int lowerValue );

private:
  void createTempIcon( int temp, int where, BYTE* and );
};

#endif
