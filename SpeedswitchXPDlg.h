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

#pragma once
#include "afxwin.h"

#define WM_ICONNOTIFY (WM_APP+100)

// for detecting XP session changes
typedef BOOL (WINAPI *SESSIONREG) ( HWND hWnd, DWORD dwFlags );
typedef BOOL (WINAPI *SESSIONUNREG) ( HWND hWnd );

extern TOptions options;

// CSpeedswitchXPDlg Dialogfeld
class CSpeedswitchXPDlg : public CDialog
{
// Konstruktion
public:
	CSpeedswitchXPDlg(CWnd* pParent = NULL);	// Standardkonstruktor
  void setCmdLine( TCHAR* cl );

// Dialogfelddaten
	enum { IDD = IDD_SPEEDSWITCHXP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	virtual void PostNcDestroy();


// Implementierung
protected:
	HICON m_hIcon;
  HICON iBattOpt;
  HICON iMaxPerf;
  HICON iMaxBatt;
  HICON iDynamic;
  HICON curIcon;
  HICON iTrayBattery;
  HICON iBattCharge;
  HICON modBatteryIcon;
  HICON cpuDataIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnBnClickedClose();
	afx_msg void OnClose();
	afx_msg void OnAbout();
  LRESULT OnIconNotify( WPARAM wp, LPARAM lp );
  LRESULT OnWtsSessionChange( WPARAM wp, LPARAM lp );
  LRESULT OnPowerBroadCast( WPARAM wp, LPARAM lp );
  LRESULT OnTaskBarCreated( WPARAM wp, LPARAM lp );
  afx_msg void OnMaxPerfMenu( void );
  afx_msg void OnBattOptMenu( void );
  afx_msg void OnMaxBattMenu( void );
  afx_msg void OnDynamicMenu( void );
  afx_msg void OnShowMainMenu( void );
  afx_msg void OnSelectShutdown( void );
	afx_msg void OnTimer( UINT nIDEvent );
  afx_msg void OnCbnSelchangeCombo1();
  afx_msg void OnCbnSelchangeCombo2();
  afx_msg void OnBnClickedOptbutton();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnBnClickedButton2();
  afx_msg void OnCbnSelchangeDisplayAC();
  afx_msg void OnCbnSelchangeDisplayDC();
  afx_msg void OnCbnSelchangeDiskAC();
  afx_msg void OnCbnSelchangeDiskDC();
  afx_msg void OnCbnSelchangeStandbyAC();
  afx_msg void OnCbnSelchangeStandbyDC();
  afx_msg void OnCbnSelchangeHibAC();
  afx_msg void OnCbnSelchangeHibDC();
  afx_msg void OnBnClickedCPUData();
  afx_msg void OnBnClickedButton4();
	DECLARE_MESSAGE_MAP()

private:
  void getRegKeys();
  void setRegKeys();
  void setAutoStartRegistry();
  void deleteAutoStartRegistry();
  void minimizeToTray();
  void displayTrayIcon( DWORD msg );
  bool closeQuery();
  void setCPUIcon( int policy );
  void MenuSpeedHandler( int throttle );
  void readCPUSpeed();
  void readCPULoad();
  void drawDiagram();
  TCHAR* getWinErr( DWORD errcode );
  void batteryDisplay( SYSTEM_POWER_STATUS* sps );
  void removeBatteryIcon();
  void displayCPUDataIcon( int msg );
  void removeCPUDataIcon();
  void showBatteryIcon( int msg, HICON icon, SYSTEM_POWER_STATUS* sps );
  int findDefaultIndex( int val );
  int findDiskIndex( int val );
  int fillHibernationTimes( CComboBox* box, int* hibernation, int idx, int what );
  void fillPolicyStrings( CComboBox* box );
  int findHibernationIndex( int* hibernation, int val, int maxIndex );
  void initPowerBoxes();
  BOOL initPowerValues();
  void initStrings();

  CRect cpuFrame;
  CRect legendFrame;
  CString m_szCPULoad;
  CString m_szCPUSpeed;
  BOOL trayIcon;
  SYSTEM_POWER_STATUS pwrStatus;
  int m_iACState;
  int m_iDCState;
  BYTE speedArr[1204];
  BYTE loadArr[1204];
  int diagramOffset;
  int loadIndex;
  int speedIndex;
  int ybaseoffset;
  int xbaseoffset;
  int tickCounterCPU;
  int tickCounterCheck;
  CWnd* frameStart;
  CWnd* frameEnd;
  int legendLine;
  BOOL activeTimer;
  CString m_szMaxCPUSpeed;
  UCHAR bitmap[128];
  BOOL batteryIconActive;
  BOOL batteryIconCharge;
  int batteryValue;
  BOOL cpuDataIconActive;
  int cpuIconVal1;
  int cpuIconVal2;
  CComboBox m_cStandbyAC;
  CComboBox m_cStandbyDC;
  CComboBox m_cHibernationAC;
  CComboBox m_cHibernationDC;
  CComboBox m_cDisplayAC;
  CComboBox m_cDisplayDC;
  CComboBox m_cDiskAC;
  CComboBox m_cDiskDC;
  int defaultTimes[18];
  int diskTimes[18];
  int hibernationTimesAC[18];
  int hibernationTimesDC[18];
  int maxHibIndexAC;
  int maxHibIndexDC;
  int curSpeed;   // for the CPU data tray icon
  int curLoad;    // for the CPU data tray icon
  bool forceShutdown;
  CString legend1;
  CString legend2;
  CString err;
};

typedef struct _NOTIFYICONDATA_CD { 
    DWORD cbSize; 
    HWND hWnd; 
    UINT uID; 
    UINT uFlags; 
    UINT uCallbackMessage; 
    HICON hIcon; 
    TCHAR szTip[128];
    DWORD dwState; 
    DWORD dwStateMask; 
    TCHAR szInfo[256]; 
    union {
      UINT  uTimeout; 
      UINT  uVersion; 
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64]; 
    DWORD dwInfoFlags; 
} NOTIFYICONDATA_CD, *PNOTIFYICONDATA_CD; 

