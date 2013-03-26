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

#ifndef CPUSPEED_H
#define CPUSPEED_H

typedef	void (*DELAY_FUNC)(unsigned int uiMS);

class CPUSpeed {
public:
	CPUSpeed ();
	~CPUSpeed ();

	// Variables.
	int CPUSpeedInMHz;

	// Functions.
	__int64 __cdecl GetCyclesDifference (DELAY_FUNC, unsigned int);
		
private:
	// Functions.
	static void Delay (unsigned int);
	static void DelayOverhead (unsigned int);

protected:
	
};


#endif
