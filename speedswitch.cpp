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

#include <stdafx.h>

#include "TOptions.h"
#include "speedswitch.h"
#include <stdio.h>
#include "SpeedswitchXP.h"

#define PROFILENAME "Speedswitch Control"

static BOOL comparePowerProfile( POWER_POLICY& pol1, POWER_POLICY& pol2 );
static BOOL compareCPUScheme( MACHINE_PROCESSOR_POWER_POLICY& pol1, MACHINE_PROCESSOR_POWER_POLICY& pol2 );
static BOOL comparePowerActionPolicy( POWER_ACTION_POLICY& pol1, POWER_ACTION_POLICY& pol2 );
static BOOL compareCPUPowerPolicy( PROCESSOR_POWER_POLICY& pol1, PROCESSOR_POWER_POLICY& pol2 );
static BOOL compareCPUPowerPolicyInfo( PROCESSOR_POWER_POLICY_INFO& pol1, PROCESSOR_POWER_POLICY_INFO& pol2 );

BOOLEAN __stdcall powerSchemeCallback(
  UINT uiIndex,      // power scheme index
  DWORD dwName,      // size of the sName string, in bytes
  LPTSTR sName,      // name of the power scheme
  DWORD dwDesc,      // size of the sDesc string, in bytes
  LPTSTR sDesc,      // description string
  PPOWER_POLICY pp,  // receives the power policy
  LPARAM lParam      // user-defined value
);

static TCHAR msg[2048];
static TCHAR dummy[256];
static UINT internalProfile;                  // the no of our own profile
static UINT lastChecked;
static POWER_POLICY toBeCopied;

int acThrottle;
int dcThrottle;
POWER_POLICY internalPolicy;           // our own profile policy
MACHINE_PROCESSOR_POWER_POLICY mach;   // our own cpu policy

//------------------------------------------------------------------
// Detect our own profile no
//
// return codes:
//   -1: profile enumeration failed
//   -2: profile not found (profile creation necessary)
//   -3: CPU policy reading from our own profile failed
//   -4: new profile activation failed
//   >=0: profile no (success)
//------------------------------------------------------------------
int detectProfile()
{
  internalProfile = 0xffffffff;
  lastChecked = 0xffffffff;

  if( !EnumPwrSchemes(&powerSchemeCallback,0) )
    return -1;      // enumeration failed

  if( internalProfile == 0xffffffff )
    return -2;      // profile not found

  // read the current CPU policy from our profile
  if( !ReadProcessorPwrScheme(internalProfile,&mach) )
    return -3;      // CPU policy reading failed

  if( !SetActivePwrScheme(internalProfile,NULL,NULL) )
    return -4;  

  updateStates();

  return internalProfile;   // profile found, return profile no
}

//------------------------------------------------------------------
// create new profile from current one
//
// return codes:
//   -1: active profile detection failed
//   -2: reading active profile policy failed
//   -3: writing our own profile failed
//   -4: reading active cpu policy failed
//   -5: writing our own cpu policy failed
//   -6: new profile activation failed
//   0: profile creation successful
//------------------------------------------------------------------
int createProfile()
{
  // get the # of the currently active profile
  UINT activeProfile;
  if( !GetActivePwrScheme(&activeProfile) )
    return -1;  

  // read the profile policy settings
  if( !ReadPwrScheme(activeProfile,&toBeCopied) )
    return -2;

  // copy this to the one we use later
  internalPolicy = toBeCopied;

  // create the # of our own profile
  lastChecked++;

  // disable throttling in this new profile
  internalPolicy.user.ForcedThrottleAc = 100;
  internalPolicy.user.ForcedThrottleDc = 100;

  // write our own profile policy settings to the registry
  if( !WritePwrScheme(&lastChecked,L"Speedswitch Control",L"\0\0",&internalPolicy) )
    return -3;

  // this is our profile #
  internalProfile = lastChecked;

  // read the CPU policy from the active profile
  if( !ReadProcessorPwrScheme(activeProfile,&mach) )
    return -4;

  // use this CPU policy for our own profile
  if( !WriteProcessorPwrScheme(internalProfile,&mach) )
    return -5;

  if( !SetActivePwrScheme(internalProfile,NULL,NULL) )
    return -6;

  updateStates();

  return 0;
}

//------------------------------------------------------------------
// Callback function for power profile policy enumeration
//------------------------------------------------------------------
BOOLEAN __stdcall powerSchemeCallback(
  UINT uiIndex,      // power scheme index
  DWORD dwName,      // size of the sName string, in bytes
  LPTSTR sName,      // name of the power scheme
  DWORD dwDesc,      // size of the sDesc string, in bytes
  LPTSTR sDesc,      // description string
  PPOWER_POLICY pp,  // receives the power policy
  LPARAM lParam      // user-defined value
)
{
  lastChecked = uiIndex;

  CString s1;
  s1.LoadStringW( IDS_SW24 );
  wsprintf( &msg[_tcslen(msg)], s1, sName, uiIndex );
  if( !_tcsicmp(dummy,_T(PROFILENAME)) )
  {
    internalProfile = uiIndex;
    internalPolicy = *pp;
    return FALSE;
  }
  
  return TRUE;
}

//------------------------------------------------------------------
// get current ac/dc throttle from cpu policy
//------------------------------------------------------------------
void updateStates()
{
  acThrottle = mach.ProcessorPolicyAc.DynamicThrottle;
  dcThrottle = mach.ProcessorPolicyDc.DynamicThrottle;
}

//------------------------------------------------------------------
// set current ac/dc throttle and activate that
//------------------------------------------------------------------
BOOL setState( BOOL ac, BYTE policy )
{
  checkProfile( 1 );

  if( ac )
  {
    acThrottle = policy;
    mach.ProcessorPolicyAc.DynamicThrottle = policy;
  }
  else
  {
    dcThrottle = policy;
    mach.ProcessorPolicyDc.DynamicThrottle = policy;
  }

  if( !WriteProcessorPwrScheme(internalProfile,&mach) )
    return FALSE;

  if( !SetActivePwrScheme(internalProfile,NULL,NULL) )
    return FALSE;  

  return TRUE;
}

//------------------------------------------------------------------
// convert unicode string to ansi string
//------------------------------------------------------------------
char* uc2char( char* dest, void* s )
{
  char* t = (char*)s;
  int i=-1;
  int j=-1;
  
  do {
    dest[++i] = t[++j];
    t++;
  } while( dest[i] );
  
  return dest;      
}

//------------------------------------------------------------------
// convert ansi string to unicode string
//------------------------------------------------------------------
char* char2uc( char* dest, char* s )
{
  char* t = (char*)s;
  int i=-1;
  int j=-1;
  
  do {
    dest[++i] = t[++j];
    dest[++i] = '\0';
  } while( t[j] );
  
  return dest;      
}

//------------------------------------------------------------------
// convert CPU policy setting to string
//------------------------------------------------------------------
CString throttleString( UCHAR throttle )
{
  static CString s1, s2, s3, s4, s5;
  static boolean init = false;

  if( !init )
  {
    s1.LoadStringW( IDS_MAIN56 );
    s2.LoadStringW( IDS_MAIN57 );
    s3.LoadStringW( IDS_MAIN58 );
    s4.LoadStringW( IDS_MAIN59 );
    s5.LoadStringW( IDS_SW7 );
    init = true;
  }

  switch( throttle )
  {
    case PO_THROTTLE_NONE:      return s1;
    case PO_THROTTLE_CONSTANT:  return s2;
    case PO_THROTTLE_DEGRADE:   return s3;
    case PO_THROTTLE_ADAPTIVE:  return s4;
  }
  
  return s5;    
}

//------------------------------------------------------------------
// power state to string
//------------------------------------------------------------------
CString sysPwrState( SYSTEM_POWER_STATE state )
{
  static CString s1, s2, s3, s4, s5, s6, s7, s8, s9;
  static boolean init = false;

  if( !init )
  {
    s1.LoadStringW( IDS_SW8 );
    s2.LoadStringW( IDS_SW9 );
    s3.LoadStringW( IDS_SW10 );
    s4.LoadStringW( IDS_SW11 );
    s5.LoadStringW( IDS_SW12 );
    s6.LoadStringW( IDS_SW13 );
    s7.LoadStringW( IDS_SW14 );
    s8.LoadStringW( IDS_SW15 );
    s9.LoadStringW( IDS_SW7 );
    init = true;
  }

  switch( state )
  {
    case PowerSystemUnspecified: return s1; //_T("No wake up when lid opened");
    case PowerSystemWorking:     return s2; //_T("S0");
    case PowerSystemSleeping1:   return s3; //_T("S1");
    case PowerSystemSleeping2:   return s4; //_T("S2");
    case PowerSystemSleeping3:   return s5; //_T("S3");
    case PowerSystemHibernate:   return s6; //_T("S4 (Hibernate)");
    case PowerSystemShutdown:    return s7; //_T("S5 (Off)");
    case PowerSystemMaximum:     return s8; //_T("???");
  }
    
  return s9; //_T("(UNKNOWN)");
}

//------------------------------------------------------------------
// power action to string
//------------------------------------------------------------------
CString pwrAction( POWER_ACTION action )
{
  static CString s1, s2, s3, s4, s5, s6, s7, s8, s9;
  static boolean init = false;

  if( !init )
  {
    s1.LoadStringW( IDS_SW16 );
    s2.LoadStringW( IDS_SW17 );
    s3.LoadStringW( IDS_SW18 );
    s4.LoadStringW( IDS_SW19 );
    s5.LoadStringW( IDS_SW20 );
    s6.LoadStringW( IDS_SW21 );
    s7.LoadStringW( IDS_SW22 );
    s8.LoadStringW( IDS_SW23 );
    s9.LoadStringW( IDS_SW7 );
    init = true;
  }

  switch( action )
  {
    case PowerActionNone:          return s1; //_T("No action");
    case PowerActionReserved:      return s2; //_T("(reserved setting)");
    case PowerActionSleep:         return s3; //_T("Sleep");
    case PowerActionHibernate:     return s4; //_T("Hibernate");
    case PowerActionShutdown:      return s5; //_T("Shutdown");
    case PowerActionShutdownReset: return s6; //_T("Shutdown and reset");
    case PowerActionShutdownOff:   return s7; //_T("Shutdown and power off");
    case PowerActionWarmEject:     return s8; //_T("Warm eject");
  }
    
  return s9; //_T("(UNKNOWN)");
}

//------------------------------------------------------------------
// get CPU usage on NT-style operating systems
//------------------------------------------------------------------
DWORD cpuUsageNT()
{
  static SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
  static SYSTEM_TIME_INFORMATION SysTimeInfo;
  static SYSTEM_BASIC_INFORMATION SysBaseInfo;
  static double dbIdleTime;
  static double dbSystemTime;
  static LONG status;
  static LARGE_INTEGER liOldIdleTime = {0,0};
  static LARGE_INTEGER liOldSystemTime = {0,0};
  static BOOL init = FALSE;
  static PROCNTQSI NtQuerySystemInformation = NULL;
  DWORD retVal = 0xffffffff;

  if( !init )
  {
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress( GetModuleHandle(_T("ntdll")), "NtQuerySystemInformation" );

    if( !NtQuerySystemInformation )
      return 0xffffffff;

    // get number of processors in the system
    status = NtQuerySystemInformation( SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL );
    if( status != NO_ERROR )
      return 0xffffffff;

    init = TRUE;
  }

  // get new system time
  status = NtQuerySystemInformation( SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0 );

  if( status != NO_ERROR )
    return 0xffffffff;

  // get new CPU's idle time
  status = NtQuerySystemInformation( SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL );
  if( status != NO_ERROR )
    return 0xffffffff;

  // if it's a first call - skip it
  if( liOldIdleTime.QuadPart != 0 )
  {
    // CurrentValue = NewValue - OldValue
    dbIdleTime = Li2Double( SysPerfInfo.liIdleTime ) - Li2Double( liOldIdleTime );
    dbSystemTime = Li2Double( SysTimeInfo.liKeSystemTime ) - Li2Double( liOldSystemTime );

    // CurrentCpuIdle = IdleTime / SystemTime
    dbIdleTime = dbIdleTime / dbSystemTime;

    // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
    dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
    
    retVal = (DWORD)dbIdleTime;
  }

  // store new CPU's idle and system time
  liOldIdleTime = SysPerfInfo.liIdleTime;
  liOldSystemTime = SysTimeInfo.liKeSystemTime;

  return retVal;
}

//------------------------------------------------------------------
// write changed power policies
//
// return codes:
//   -1: power policy writing failure
//   -2: cpu policy writing failure
//   0: success
//------------------------------------------------------------------
int writePolicies( BOOL power, BOOL cpu )
{
  if( power && !WritePwrScheme(&internalProfile,L"Speedswitch Control",L"\0\0",&internalPolicy) )
  {
    ReadPwrScheme( internalProfile, &internalPolicy );
    return -1;
  }

  if( cpu && !WriteProcessorPwrScheme(internalProfile,&mach) )
  {
    ReadProcessorPwrScheme( internalProfile, &mach );

    if( power )
      SetActivePwrScheme( internalProfile, NULL, NULL );

    return -2;
  }

  SetActivePwrScheme( internalProfile, NULL, NULL );
  return 0;
}

//------------------------------------------------------------------
// check power scheme integrity
//
// check: 0=no consistency check
//        1=check power scheme only
//        2=check processor scheme only
//        3=check both power and processor scheme
//------------------------------------------------------------------
BOOL checkProfile( int check )
{
  log( _T("Checking power scheme integrity ...") );
  
  CString err;
  err.LoadStringW( IDS_MAINERR2 );

  POWER_POLICY policyTemp;
  MACHINE_PROCESSOR_POWER_POLICY machTemp;
  static TCHAR msg[1024];   // für diverse Fehlertexte
  BOOL forceActivate = FALSE;

  if( !ReadPwrScheme(internalProfile,&policyTemp) )
  {
    log( _T("Recreating power scheme ...") );
    UINT x = internalProfile;

    if( !WritePwrScheme(&x,L"Speedswitch Control",L"\0\0",&internalPolicy) )
    {
      CString s1;
      s1.LoadStringW( IDS_SW1 );
      wsprintf( msg, s1, GetLastError() );
      MessageBox( NULL, msg, err, MB_ICONEXCLAMATION|MB_OK );
      return FALSE;
    }

    forceActivate = TRUE;
  }
  else if( check&1 )
  {
    if( !comparePowerProfile(policyTemp,internalPolicy) )
    {
      log( _T("Restoring power scheme data ...") );
      UINT x = internalProfile;

      if( !WritePwrScheme(&x,L"Speedswitch Control",L"\0\0",&internalPolicy) )
      {
        CString s1;
        s1.LoadStringW( IDS_SW2 );
        wsprintf( msg, s1, GetLastError() );
        MessageBox( NULL, msg, err, MB_ICONEXCLAMATION|MB_OK );
        return FALSE;
      }

      forceActivate = TRUE;
    }
  }

  if( !ReadProcessorPwrScheme(internalProfile,&machTemp) )
  {
    log( _T("Recreating processor scheme ...") );
    if( !WriteProcessorPwrScheme(internalProfile,&mach) )
    {
      CString s1;
      s1.LoadStringW( IDS_SW3 );
      wsprintf( msg, s1, GetLastError() );
      MessageBox( NULL, msg, err, MB_ICONEXCLAMATION|MB_OK );
      return FALSE;
    }   

    forceActivate = TRUE;
  }
  else if( check&2 )
  {
    if( !compareCPUScheme(machTemp,mach) )
    {
      log( _T("Restoring processor scheme data ...") );
      if( !WriteProcessorPwrScheme(internalProfile,&mach) )
      {
        CString s1;
        s1.LoadStringW( IDS_SW4 );
        wsprintf( msg, s1, GetLastError() );
        MessageBox( NULL, msg, err, MB_ICONEXCLAMATION|MB_OK );
        return FALSE;
      }  

      forceActivate = TRUE;
    }
  }

  UINT profile;

  if( !GetActivePwrScheme(&profile) )
  {
    CString s1;
    s1.LoadStringW( IDS_SW5 );
    wsprintf( msg, s1, GetLastError() );
    MessageBox( NULL, msg, err, MB_OK|MB_ICONEXCLAMATION );
    return FALSE;
  }

  if( forceActivate || profile!=internalProfile )
  {
    log( _T("Reactivating power scheme (%d, %d) ..."), profile, internalProfile );
    if( !SetActivePwrScheme(internalProfile,NULL,NULL) )
    {
      CString s1;
      s1.LoadStringW( IDS_SW6 );
      wsprintf( msg, s1, GetLastError() );
      MessageBox( NULL, msg, err, MB_ICONEXCLAMATION|MB_OK );
      return FALSE;
    }
  }

  return TRUE;
}

static BOOL comparePowerProfile( POWER_POLICY& pol1, POWER_POLICY& pol2 )
{
  return pol1.user.Revision == pol2.user.Revision
    &&   comparePowerActionPolicy(pol1.user.IdleAc,pol2.user.IdleAc)
    &&   comparePowerActionPolicy(pol1.user.IdleDc,pol2.user.IdleDc)
    &&   pol1.user.IdleTimeoutAc == pol2.user.IdleTimeoutAc
    &&   pol1.user.IdleTimeoutDc == pol2.user.IdleTimeoutDc
    &&   pol1.user.IdleSensitivityAc == pol2.user.IdleSensitivityAc
    &&   pol1.user.IdleSensitivityDc == pol2.user.IdleSensitivityDc
    &&   pol1.user.ThrottlePolicyAc == pol2.user.ThrottlePolicyAc
    &&   pol1.user.ThrottlePolicyDc == pol2.user.ThrottlePolicyDc
    &&   pol1.user.MaxSleepAc == pol2.user.MaxSleepAc
    &&   pol1.user.MaxSleepDc == pol2.user.MaxSleepDc
    &&   pol1.user.VideoTimeoutAc == pol2.user.VideoTimeoutAc
    &&   pol1.user.VideoTimeoutDc == pol2.user.VideoTimeoutDc
    &&   pol1.user.SpindownTimeoutAc == pol2.user.SpindownTimeoutAc
    &&   pol1.user.SpindownTimeoutDc == pol2.user.SpindownTimeoutDc
    &&   pol1.user.OptimizeForPowerAc == pol2.user.OptimizeForPowerAc
    &&   pol1.user.OptimizeForPowerDc == pol2.user.OptimizeForPowerDc
    &&   pol1.user.FanThrottleToleranceAc == pol2.user.FanThrottleToleranceAc
    &&   pol1.user.FanThrottleToleranceDc == pol2.user.FanThrottleToleranceDc
    &&   pol1.user.ForcedThrottleAc == pol2.user.ForcedThrottleAc
    &&   pol1.user.ForcedThrottleDc == pol2.user.ForcedThrottleDc
    &&   pol1.mach.Revision == pol2.mach.Revision
    &&   pol1.mach.MinSleepAc == pol2.mach.MinSleepAc
    &&   pol1.mach.MinSleepDc == pol2.mach.MinSleepDc
    &&   pol1.mach.ReducedLatencySleepAc == pol2.mach.ReducedLatencySleepAc
    &&   pol1.mach.ReducedLatencySleepDc == pol2.mach.ReducedLatencySleepDc
    &&   pol1.mach.DozeTimeoutAc == pol2.mach.DozeTimeoutAc
    &&   pol1.mach.DozeTimeoutDc == pol2.mach.DozeTimeoutDc
    &&   pol1.mach.DozeS4TimeoutAc == pol2.mach.DozeS4TimeoutAc
    &&   pol1.mach.DozeS4TimeoutDc == pol2.mach.DozeS4TimeoutDc
    &&   pol1.mach.MinThrottleAc == pol2.mach.MinThrottleAc
    &&   pol1.mach.MinThrottleDc == pol2.mach.MinThrottleDc
    &&   comparePowerActionPolicy(pol1.mach.OverThrottledAc,pol2.mach.OverThrottledAc)
    &&   comparePowerActionPolicy(pol1.mach.OverThrottledDc,pol2.mach.OverThrottledDc);
}

static BOOL comparePowerActionPolicy( POWER_ACTION_POLICY& pol1, POWER_ACTION_POLICY& pol2 )
{
  return pol1.Action == pol2.Action
    &&   pol1.EventCode == pol2.EventCode
    &&   pol1.Flags == pol2.Flags;
}

static BOOL compareCPUScheme( MACHINE_PROCESSOR_POWER_POLICY& pol1, MACHINE_PROCESSOR_POWER_POLICY& pol2 )
{
  return pol1.Revision == pol2.Revision
    &&   compareCPUPowerPolicy(pol1.ProcessorPolicyAc,pol2.ProcessorPolicyAc)
    &&   compareCPUPowerPolicy(pol1.ProcessorPolicyDc,pol2.ProcessorPolicyDc);
}

static BOOL compareCPUPowerPolicy( PROCESSOR_POWER_POLICY& pol1, PROCESSOR_POWER_POLICY& pol2 )
{
  return pol1.Revision == pol2.Revision
    &&   pol1.DynamicThrottle == pol2.DynamicThrottle
    &&   pol1.PolicyCount == pol2.PolicyCount
    &&   ((pol1.PolicyCount>0) ? compareCPUPowerPolicyInfo(pol1.Policy[0],pol2.Policy[0]) : TRUE)
    &&   ((pol1.PolicyCount>1) ? compareCPUPowerPolicyInfo(pol1.Policy[1],pol2.Policy[1]) : TRUE)
    &&   ((pol1.PolicyCount>2) ? compareCPUPowerPolicyInfo(pol1.Policy[2],pol2.Policy[2]) : TRUE);
}

static BOOL compareCPUPowerPolicyInfo( PROCESSOR_POWER_POLICY_INFO& pol1, PROCESSOR_POWER_POLICY_INFO& pol2 )
{
  return pol1.TimeCheck == pol2.TimeCheck
    &&   pol1.DemoteLimit == pol2.DemoteLimit
    &&   pol1.PromoteLimit == pol2.PromoteLimit
    &&   pol1.DemotePercent == pol2.DemotePercent
    &&   pol1.PromotePercent == pol2.PromotePercent
    &&   pol1.AllowDemotion == pol2.AllowDemotion
    &&   pol1.AllowPromotion == pol2.AllowPromotion;
}

