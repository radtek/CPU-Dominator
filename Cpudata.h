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
#include "afxcmn.h"

#ifndef CCPUDATA_H
#define CCPUDATA_H

// CCPUData dialog

class CCPUData : public CDialog
{
	DECLARE_DYNAMIC(CCPUData)

public:
	CCPUData(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCPUData();
  void setVars();

// Dialog Data
	enum { IDD = IDD_CPUDATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
  void init();
  void getCPUFeatures();
  void getCPUType();
  void identifyIntel();
  void identifyAMD();
  void identifyCyrix();
  void Message( LPCTSTR lpszMessage );
  void readCPUIDLevel( int level, int level2, ULONG* target );
  void readCacheInfo();
  void printCacheInfo( int value );

  BOOL supportsCPUID;
  WORD cpuType;
  TCHAR cpuStr[264];
  DWORD exlevel;
  int maxIDVal;
  TCHAR vendorID[16];
  int brandID;
  int family;
  DWORD features;
  DWORD exfeatures;
  DWORD exfeatures2;
  int vendor;
  int familyid;
  int modelid;
  int steppingid;
  int typid;
  TCHAR* brandStr;
  TCHAR* cacheSize;
  TCHAR* structureSize;
  TCHAR* coreType;
  CFont mf;
  LOGFONT lf;
  BOOL f_fpu;
  BOOL f_vme;
  BOOL f_de;
  BOOL f_pse;
  BOOL f_tsc;
  BOOL f_msr;
  BOOL f_pae;
  BOOL f_mce;
  BOOL f_c8x;
  BOOL f_apic;
  BOOL f_sep;
  BOOL f_mtrr;
  BOOL f_pge;
  BOOL f_mca;
  BOOL f_cmov;
  BOOL f_pse36;
  BOOL f_psn;
  BOOL f_mmx;
  BOOL f_pat;
  BOOL f_fxsr;
  BOOL f_fcmov;
  BOOL f_fsc;
  BOOL f_mmxext;
  BOOL f_cxmmx;
  BOOL f_3dnow;
  BOOL f_3dnowext;
  BOOL f_sse;
  BOOL f_sse2;
  BOOL f_ss;
  BOOL f_htt;
  BOOL f_tm1;
  BOOL f_acpi;
  BOOL f_ia64;
  BOOL f_pbe;
  BOOL f_sse3;
  BOOL f_mon;
  BOOL f_cpl;
  BOOL f_vmx;
  BOOL f_est;
  BOOL f_tm2;
  BOOL f_l1id;
  BOOL f_c16b; 

  CString m_szSupportsCPUID;
  CString m_szVendor;
  CString m_szFeatures;
  int m_iMaxIDVal;
  CString m_szManufacturer;
  int m_iFamily;
  int m_iModel;
  int m_iStepping;
  int m_iFType;
  CString m_szMaxExLevel;
  CString m_szExCPUName;
  CString m_szExFeatures;
  int m_iBrandID;
  CString m_szBrandStr;
  CString m_szExFeatures2;
  TCHAR flagText[65536];
  CString m_szFlags;
  CEdit m_eFeatures;
  CString m_szCacheInfo;
  CString m_szCoreType;
  CString m_szStructureSize;
  CListCtrl m_cList;
  CString cacheStr[32];
  int cacheStrNo;
  CString flag[64];
  int flagNo;

};

typedef struct cacheid {
  int id;
  TCHAR* text;
} CACHEID;

#endif
