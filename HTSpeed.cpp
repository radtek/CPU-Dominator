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

#define _UNICODE

#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "speed.h"
#include "cpuid.h"

static int firstrun = 1;
static ushort sproc;
static DWORD sfeat;


// compute CPU speed with TSC and performance counter
int cpuSpeedHT()
{
	UINT64 t0;      // ticks at beginning of test
  UINT64 t1;      // ticks at end of test
  UINT64 stamp0;  // cycles at beginning of test
  UINT64 stamp1;  // cycles at end of test
  UINT64 countFreq;
  UINT64 elapsedCycles;
  UINT64 elapsedTicks;
  double elapsedTime;
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	ULONG dwProcessMask;
  ULONG dwSystemMask;

  if( firstrun )    // init once (at first call only)
  {
    sproc = wincpuid();
    sfeat = wincpufeatures();
    firstrun = 0;
  }

  if( !(sfeat & TSC_SUPPORT) )    // no TSC support
		return 0;

	// get the frequency of the high-resolution performance counter
	if( !QueryPerformanceFrequency((LARGE_INTEGER*)&countFreq) )
		return 0;

	// get current process & thread priorities for later restoration
  ULONG dwCurPriorityClass = GetPriorityClass( hProcess );
	int iCurThreadPriority = GetThreadPriority( hThread );

  // get process & system affinity masks for later restoration
	GetProcessAffinityMask( hProcess, &dwProcessMask, &dwSystemMask );

	// set this process & thread to max priorities so we don't get interrupted too often
  if( dwCurPriorityClass != 0 )
    SetPriorityClass( hProcess, REALTIME_PRIORITY_CLASS );

  if( iCurThreadPriority != THREAD_PRIORITY_ERROR_RETURN )
    SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );

  // nail this process onto the first CPU
	SetProcessAffinityMask( hProcess, 1 );

  // get starting ticks
	QueryPerformanceCounter( (LARGE_INTEGER*)&t0 );

	// serialize for in-order-execution and get starting cycles 
  __asm { 
    push ebx
    xor eax,eax
    cpuid
    rdtsc
		mov dword ptr [stamp0],eax
		mov dword ptr [stamp0+4],edx
    pop ebx
	}

	Sleep( 50 );    // wait for 50msec to let the ticks and cycles counters advance

  // get ending ticks
  QueryPerformanceCounter( (LARGE_INTEGER*)&t1 );

	// serialize again for in-order-execution and get ending cycles 
	__asm 
	{
    push ebx
    xor eax,eax
    cpuid
    rdtsc
		mov dword ptr [stamp1],eax
		mov dword ptr [stamp1+4],edx
    pop ebx
	}

  // restore process affinity and process&thread priorities
	SetProcessAffinityMask( hProcess, dwProcessMask );
	SetThreadPriority( hThread, iCurThreadPriority );
	SetPriorityClass( hProcess, dwCurPriorityClass );

  // now compute the frequency...
	elapsedCycles = stamp1 - stamp0;
	elapsedTicks = t1 - t0;
  elapsedTime = (double)elapsedTicks / (double)countFreq;

  // MHz = elapsed cycles / elapsed time / 1 million
  return (int)((UINT64)(((double)elapsedCycles) / elapsedTime) / 1000000);
}
