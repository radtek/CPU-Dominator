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
#include "AtlBase.h"
#include "TOptions.h"

#define WRITEDW(a,b)      \
    if( key.SetValue(b,REG_DWORD,&(a),size) != ERROR_SUCCESS ) \
    { \
      key.Close(); \
      return false; \
    }

#define READDW(a,b) \
    if( key.QueryValue(b,NULL,&(a),&size) != ERROR_SUCCESS ) \
    { \
      WRITEDW(a,b) \
    }


TOptions::TOptions()
{
  // initial (default) settings:
  autoStart = true;     // autostart enabled
  debugMode = false;    // use only on erratic behaviour for debugging
  minimizeOnClose = false;
  checkStatus = true;
  checkInterval = 5;
  reactivate = false;
  originalScheme = 0;
  readCPUSpeed = false;
  speedMethod = 1;
  readCPULoad = false;
  showDiagram = false;
  cpuInterval = 2;
  freqScaling = 1;
  showBattery = true;
  batteryMethod = 0;      // show only when on DC
  batteryIndicator = 0;   // show percentage (0); 1= show time left
  maxSpeed = 0;
  ac = true;
  chargeIndicator = true; // show charging status
  speedIcon = false;
  loadIcon = false;
  iconColor1 = 0;
  iconColor2 = 15;
  iconTransparent = TRUE;
}

// read options from registry
bool TOptions::getRegKeys()
{
  LONG lReturn = 0;
  CRegKey key;
  ULONG size = sizeof DWORD;   // DWORD size


  lReturn = key.Create( HKEY_CURRENT_USER, _T("Software\\CD\\SpeedswitchXP\\") );

  if( lReturn == ERROR_SUCCESS )
  {
    READDW(autoStart,_T("Autostart"))
    READDW(debugMode,_T("DebugMode"))
    READDW(minimizeOnClose,_T("MinimizeOnClose"))
    READDW(checkStatus,_T("CheckStatus"))
    READDW(checkInterval,_T("CheckInterval"))
    READDW(reactivate,_T("ReactivatePreviousScheme"))
    READDW(originalScheme,_T("OriginalScheme"))
    READDW(readCPUSpeed,_T("ReadCPUSpeed"))
    READDW(speedMethod,_T("SpeedMethod"))
    READDW(readCPULoad,_T("ReadCPULoad"))
    READDW(showDiagram,_T("ShowDiagram"))
    READDW(cpuInterval,_T("UpdateInterval"))
    READDW(freqScaling,_T("FrequenceScaling"))
    READDW(maxSpeed,_T("CalculatedMaxSpeed"))
    READDW(showBattery,_T("ShowBattery"))
    READDW(batteryMethod,_T("BatteryMethod"))
    READDW(batteryIndicator,_T("BatteryIndicator"))
    READDW(chargeIndicator,_T("ChargeIndicator"))
    READDW(speedIcon,_T("TraySpeed"))
    READDW(loadIcon,_T("TrayLoad"))
    READDW(iconColor1,_T("IconColor1"))
    READDW(iconColor2,_T("IconColor2"))
    READDW(iconTransparent,_T("IconTransparent"))
  }
  else
    return false;

  key.Close();
  return true;
}

// save options in registry
bool TOptions::setRegKeys()
{
  LONG lReturn = 0;
  CRegKey key;
  ULONG size = sizeof DWORD;

  lReturn = key.Create( HKEY_CURRENT_USER, _T("Software\\CD\\SpeedswitchXP\\") );

  if( lReturn == ERROR_SUCCESS )
  {
    WRITEDW(autoStart,_T("Autostart"))
    WRITEDW(debugMode,_T("DebugMode"))
    WRITEDW(minimizeOnClose,_T("MinimizeOnClose"))
    WRITEDW(checkStatus,_T("CheckStatus"))
    WRITEDW(checkInterval,_T("CheckInterval"))
    WRITEDW(reactivate,_T("ReactivatePreviousScheme"))
    WRITEDW(originalScheme,_T("OriginalScheme"))
    WRITEDW(readCPUSpeed,_T("ReadCPUSpeed"))
    WRITEDW(speedMethod,_T("SpeedMethod"))
    WRITEDW(readCPULoad,_T("ReadCPULoad"))
    WRITEDW(showDiagram,_T("ShowDiagram"))
    WRITEDW(cpuInterval,_T("UpdateInterval"))
    WRITEDW(freqScaling,_T("FrequenceScaling"))
    WRITEDW(maxSpeed,_T("CalculatedMaxSpeed"))
    WRITEDW(showBattery,_T("ShowBattery"))
    WRITEDW(batteryMethod,_T("BatteryMethod"))
    WRITEDW(batteryIndicator,_T("BatteryIndicator"))
    WRITEDW(chargeIndicator,_T("ChargeIndicator"))
    WRITEDW(speedIcon,_T("TraySpeed"))
    WRITEDW(loadIcon,_T("TrayLoad"))
    WRITEDW(iconColor1,_T("IconColor1"))
    WRITEDW(iconColor2,_T("IconColor2"))
    WRITEDW(iconTransparent,_T("IconTransparent"))
  }
  else
    return false;

  key.Close();

  return true;
}

