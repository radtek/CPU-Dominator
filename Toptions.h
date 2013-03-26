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

#ifndef TOPTIONS_H
#define TOPTIONS_H

class TOptions {
public:
  TOptions();
  bool setRegKeys();
  bool getRegKeys();

  BOOL autoStart;
  BOOL debugMode;
  BOOL minimizeOnClose;
  BOOL checkStatus;
  int checkInterval;
  BOOL reactivate;
  int originalScheme;
  BOOL readCPUSpeed;
  int speedMethod;
  BOOL readCPULoad;
  BOOL showDiagram;
  int cpuInterval;
  int freqScaling;
  int maxSpeed;
  BOOL showBattery;
  int batteryMethod;    // 0=DC only, 1=Show always
  int batteryIndicator;
  BOOL ac;              // not saved in registry
  BOOL chargeIndicator;
  BOOL speedIcon;
  BOOL loadIcon;
  int iconColor1;
  int iconColor2;
  BOOL iconTransparent;
};

#endif
