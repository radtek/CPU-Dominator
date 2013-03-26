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

#define _UNICODE

#include "stdafx.h"
#include "SpeedswitchXP.h"
#include "CPUData.h"
#include "cpuid.h"
#include "speed.h"
#include "speedswitch.h"
#include <ctype.h>

// CCPUData dialog

IMPLEMENT_DYNAMIC(CCPUData, CDialog)
CCPUData::CCPUData(CWnd* pParent /*=NULL*/)
	: CDialog(CCPUData::IDD, pParent)
  , m_szSupportsCPUID(_T(""))
  , m_szVendor(_T(""))
  , m_szFeatures(_T(""))
  , m_iMaxIDVal(0)
  , m_szManufacturer(_T(""))
  , m_iFamily(0)
  , m_iModel(0)
  , m_iStepping(0)
  , m_iFType(0)
  , m_szMaxExLevel(_T(""))
  , m_szExCPUName(_T(""))
  , m_szExFeatures(_T(""))
  , m_iBrandID(0)
  , m_szBrandStr(_T(""))
  , m_szExFeatures2(_T(""))
  , m_szFlags(_T(""))
  , m_szCacheInfo(_T(""))
  , m_szCoreType(_T(""))
  , m_szStructureSize(_T(""))
{ }

CCPUData::~CCPUData()
{ }

void CCPUData::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LIST2, m_cList);
}

BEGIN_MESSAGE_MAP(CCPUData, CDialog)
END_MESSAGE_MAP()

// CCPUData message handlers

BOOL CCPUData::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  CRect rect;
  m_cList.GetClientRect( &rect );
  int nColInterval = rect.Width()/8;

  CString s1;
  s1.LoadStringW( IDS_CPU1 );
  m_cList.InsertColumn( 0, s1, LVCFMT_LEFT, nColInterval*2 );
  s1.LoadStringW( IDS_CPU2 );
  m_cList.InsertColumn( 1, s1, LVCFMT_LEFT, nColInterval*5+nColInterval/2 );

  s1.LoadStringW( IDS_CPU3 );
  int nIndex = m_cList.InsertItem( 0, s1 );
  m_cList.SetItemText( nIndex, 1, cpuStr );

  s1.LoadStringW( IDS_CPU4 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szManufacturer );

  s1.LoadStringW( IDS_CPU5 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, vendorID );

  s1.LoadStringW( IDS_CPU6 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szCoreType );

  s1.LoadStringW( IDS_CPU7 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szStructureSize );

  if( cacheStrNo != 0 )
  {
    for( int i=0; i<cacheStrNo; i++ )
    {
      s1.LoadStringW( IDS_CPU8 );
      nIndex = m_cList.InsertItem( nIndex+1, i==0 ? s1 : _T("") );
      m_cList.SetItemText( nIndex, 1, cacheStr[i] );
    }
  }

  CString szFamily, szModel, szStepping, szType, szBrand, szMaxLevel;
  szFamily.Format( _T("%d"), familyid );
  szModel.Format( _T("%d"), modelid );
  szStepping.Format( _T("%d"), steppingid );
  szType.Format( _T("%d"), typid );
  szBrand.Format( _T("%d"), brandID & 255 );
  szMaxLevel.Format( _T("%d"), maxIDVal );

  nIndex = m_cList.InsertItem( nIndex+1, _T("") );
  m_cList.SetItemText( nIndex, 1, _T("") );

  s1.LoadStringW( IDS_CPU9 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szFamily );

  s1.LoadStringW( IDS_CPU10 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szModel );

  s1.LoadStringW( IDS_CPU11 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szStepping );

  s1.LoadStringW( IDS_CPU12 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szType );

  s1.LoadStringW( IDS_CPU13 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szBrand );

  s1.LoadStringW( IDS_CPU14 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, brandStr );

  s1.LoadStringW( IDS_CPU15 );
  CString s2;
  s2.LoadStringW( IDS_CPU16 );
  m_szSupportsCPUID = (supportsCPUID ? s1 : s2);

  s1.LoadStringW( IDS_CPU17 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szSupportsCPUID );

  s1.LoadStringW( IDS_CPU18 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, szMaxLevel );

  s1.LoadStringW( IDS_CPU19 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szMaxExLevel );

  s1.LoadStringW( IDS_CPU20 );
  nIndex = m_cList.InsertItem( nIndex+1, s1 );
  m_cList.SetItemText( nIndex, 1, m_szFeatures + _T("  ") + m_szExFeatures + _T("  ") + m_szExFeatures2 );

  s1.LoadStringW( IDS_CPU21 );

  if( flagNo != 0 )
  {
    nIndex = m_cList.InsertItem( nIndex+1, _T("") );
    m_cList.SetItemText( nIndex, 1, _T("") );

    for( int i=0; i<flagNo; i++ )
    {
      nIndex = m_cList.InsertItem( nIndex+1, i==0 ? s1 : _T("") );
      m_cList.SetItemText( nIndex, 1, flag[i] );
    }
  }

  UpdateData( FALSE );
  return TRUE;
}

void CCPUData::setVars()
{
  init();
  getCPUType();

  m_szExCPUName = cpuStr;
  m_szVendor = vendorID;
  m_iFType = typid;
  m_iFamily = familyid;
  m_iModel = modelid;
  m_iStepping = steppingid;
  m_iMaxIDVal = maxIDVal;
  m_iBrandID = brandID & 255;
  m_szBrandStr = brandStr;

  if( coreType == NULL )
    m_szCoreType = "?";
  else
    m_szCoreType = coreType;
  
  if( structureSize == NULL )
    m_szStructureSize = "?";
  else
    m_szStructureSize = structureSize;

  CString s1;
  s1.LoadStringW( IDS_CPU22 );

  if( supportsCPUID && maxIDVal>=2 )
    readCacheInfo();
  else if( cacheSize != NULL )
    cacheStr[cacheStrNo++].Format( s1, cacheSize );

  m_szFeatures.Format( _T("0x%08x"), features );
  m_szExFeatures.Format( _T("0x%08x"), exfeatures );
  m_szExFeatures2.Format( _T("0x%08x"), exfeatures2 );

  if( exlevel >= 0x80000000 )
    m_szMaxExLevel.Format( _T("0x%08x"), exlevel );
  else
    m_szMaxExLevel = _T("---");

  memset( &lf, 0, sizeof(LOGFONT) ); 

  *flagText = '\0';

  if( f_fpu )   Message( _T("FPU   - Floating Point Unit") );
  if( f_vme )   Message( _T("VME   - V86 Mode Extensions") );
  if( f_de )    Message( _T("DE    - Debug Extensions") );
  if( f_pse )   Message( _T("PSE   - Page Size Extensions") );
  if( f_tsc )   Message( _T("TSC   - Timestamp Counter") );
  if( f_msr )   Message( _T("MSR   - Model Specific Registers") );
  if( f_pae )   Message( _T("PAE   - Physical Address Extensions") );
  if( f_mce )   Message( _T("MCE   - Machine Check Exception") );
  if( f_c8x )   Message( _T("C8X   - CMPXCHG8B Instruction") );
  if( f_apic )  Message( _T("APIC  - Local APIC") );
  if( f_sep )   Message( _T("SEP   - Fast System Call") );
  if( f_mtrr )  Message( _T("MTRR  - Memory Type Range Registers") );
  if( f_pge )   Message( _T("PGE   - Page Global Enable") );
  if( f_mca )   Message( _T("MCA   - Machine Check Architecture") );
  if( f_cmov )  Message( _T("CMOV  - CMOV Instructions") );
  if( f_pse36 ) Message( _T("PSE36 - 36bit Page Size Extensions") );
  if( f_psn )   Message( _T("PSN   - Processor Serial Number") );
  if( f_mmx )   Message( _T("MMX   - MMX Instructions") );

  switch( vendor )
  {
    case 0: m_szManufacturer = _T("Intel");
            if( f_acpi ) Message( _T("ACPI  - ACPI Support") );
            if( f_pat )  Message( _T("PAT   - Page Attribute Table") );
            if( f_fxsr ) Message( _T("FXSR  - FXSAVE/FXRSTOR Instructions") );
            if( f_sse )  Message( _T("SSE   - SSE Instructions") );
            if( f_sse2 ) Message( _T("SSE2  - SSE2 Instructions") );
            if( f_ss )   Message( _T("SS    - Self Snoop") );
            if( f_htt )  Message( _T("HTMC  - Hyperthreading/Multicore Technology") );
            if( f_tm1 )  Message( _T("TM1   - Thermal Monitor 1") );
            if( f_pbe )  Message( _T("PBE   - Pending Break Enable") );
            if( f_ia64 ) Message( _T("IA64  - IA64 Instructions") );
            if( f_sse3 ) Message( _T("SSE3  - SSE3 Instructions") );
            if( f_mon )  Message( _T("MMW   - MONITOR/MWAIT") );
            if( f_cpl )  Message( _T("CPL   - CPL Qualified Debug Store") );
            if( f_vmx )  Message( _T("VMX   - Virtual Machine Extensions") );
            if( f_est )  Message( _T("EST   - Enhanced Intel Speed Step") );
            if( f_tm2 )  Message( _T("TM2   - Thermal Monitor 2") );
            if( f_l1id ) Message( _T("L1ID  - L1 Context ID") );
            if( f_c16b ) Message( _T("C16X  - CMPXCHG16B Support") );
            //if( f_xtpr ) Message( _T("XTPR  - xTPR Disable") );
            break;

    case 1: m_szManufacturer = _T("AMD");
            if( f_fsc )      Message( _T("FSC   - Fast System Call") );
            if( f_fcmov )    Message( _T("FCMOV - FCMOV Instruction") );
            if( f_mmxext )   Message( _T("MMXE  - Extended MMX Instructions") );
            if( f_sse )      Message( _T("SSE   - SSE Instructions") );
            if( f_sse2 )     Message( _T("SSE2  - SSE2 Instructions") );
            if( f_3dnowext ) Message( _T("3DNE  - Extended 3DNow! Instructions") );
            if( f_3dnow )    Message( _T("3DNOW - 3DNow! Instructions") );
            break;

    case 2: m_szManufacturer = _T("Cyrix");
            if( f_cxmmx ) Message( _T("CXMMX - Cyrix MMX Instructions") );
            break;

    case 3: m_szManufacturer = _T("Transmeta");
            break;

    case 4: m_szManufacturer = _T("Centaur");
            if( f_3dnow ) Message( _T("3DNOW - 3DNow! Instructions") );
            break;

    case 5: m_szManufacturer = _T("Rise");
            break;

    case 6: m_szManufacturer = _T("UMC");
            break;

    case 7: m_szManufacturer = _T("NexGen");
            break;

    default:m_szManufacturer = _T("Unknown");
            break;
  }

  m_szFlags = flagText;
}

void CCPUData::getCPUType()
{
  cpuType = wincpuid();
  supportsCPUID = wincpuidsupport();

  switch( cpuType & (CLONE_MASK-1) )
  {
    case 0: _tcscpy_s( cpuStr, _T("8086") );
            break;

    case 1: _tcscpy_s( cpuStr, _T("80186") );
            break;

    case 2: _tcscpy_s( cpuStr, _T("80286") );
            break;

    case 3: _tcscpy_s( cpuStr, _T("80386") );
            break;

    case 4: _tcscpy_s( cpuStr, _T("80486") );
            break;

    case 5: _tcscpy_s( cpuStr, _T("Pentium") );
            break;

    case 6: _tcscpy_s( cpuStr, _T("Pentium Pro") );
            break;

    case 15:_tcscpy_s( cpuStr, _T("Pentium 4") );
            break;

    default:_tcscpy_s( cpuStr, _T("Unknown") );
            break;
  }

  familyid = cpuType & (CLONE_MASK-1);

  if( !supportsCPUID )
    return;

  getCPUFeatures();

  typid = (family & (3<<12))>>12;
  familyid = (family&(15<<8))>>8;
  modelid = (family&(15<<4))>>4;
  steppingid = family&15;

  f_fpu = features & 1;
  f_vme = features & 2;
  f_de = features & 4;
  f_pse = features & 8;
  f_tsc = features & 16;
  f_msr = features & 32;
  f_pae = features & 64;
  f_mce = features & 128;
  f_c8x = features & (1<<8);
  f_apic = features & (1<<9);
  f_sep = features & (1<<11);
  f_mtrr = features & (1<<12);
  f_pge = features & 8192;
  f_mca = features & 16384;
  f_cmov = features & 32768;
  f_pse36 = features & (1<<17);
  f_psn = features & (1<<18);
  f_mmx = features & (1<<23);

  if( !_tcsicmp(vendorID,_T("GenuineIntel")) )
  {
    vendor = 0;   // Intel
    f_pat = features & (1<<16);
    f_acpi = features & (1<<22);
    f_fxsr = features & (1<<24);
    f_sse = features & (1<<25);
    f_sse2 = features & (1<<26);
    f_ss = features & (1<<27);
    f_htt = features & (1<<28);
    f_tm1 = features & (1<<29);
    f_ia64 = features & (1<<30);
    f_pbe = features & (1<<31);
    f_sse3 = exfeatures & 1;
    f_mon = exfeatures & (1<<3);
    f_cpl = exfeatures & (1<<4);
    f_vmx = exfeatures & (1<<5);
    f_est = exfeatures & (1<<7);
    f_tm2 = exfeatures & (1<<8);
    f_l1id = exfeatures & (1<<10);
    f_c16b = exfeatures & (1<<13);
    identifyIntel();
  }
  else if( !_tcsicmp(vendorID,_T("AuthenticAMD")) )
  {
    vendor = 1;   // AMD
    f_fsc = features & (1<<10);
    f_fcmov = features & (1<<16);
    f_mmxext = features & (1<<22);
    f_sse = features & (1<<25);
    f_sse2 = features & (1<<26);
//    f_3dnowext = features & (1<<30);
//    f_3dnow = features & (1<<31);
    identifyAMD();
  }
  else if( !_tcsicmp(vendorID,_T("CyrixInstead")) )
  {
    vendor = 2;   // Cyrix
    f_cxmmx = features & (1<<24);
  }
  else if( !_tcsicmp(vendorID,_T("GenuineTMx86")) )
  {
    vendor = 3;   // Transmeta
  }
  else if( !_tcsicmp(vendorID,_T("CentaurHauls")) )
  {
    vendor = 4;   // Centaur
    f_3dnow = features & (1<<31);
  }
  else if( !_tcsicmp(vendorID,_T("RISERISERISE")) )
  {
    vendor = 5;   // Rise
  }
  else if( !_tcsicmp(vendorID,_T("UMC UMC UMC ")) )
  {
    vendor = 6;   // UMC
  }
  else if( !_tcsicmp(vendorID,_T("NexGenDriven")) )
  {
    vendor = 7;   // NexGen
  }

  DWORD exl;

  __asm {
    pushad
    mov eax,0x80000000
    CPU_ID
    mov dword ptr exl,eax
    popad
  }

  exlevel = exl;

  if( exlevel>=0x80000001 && vendor==1 )  // nur bei AMD
  {
    DWORD amdflags;

    __asm {
      pushad
      mov eax, 0x80000001
      CPU_ID
      mov dword ptr amdflags, edx
      popad
    }

    exfeatures2 = amdflags;

    f_3dnow = amdflags & (1<<31);
    f_3dnowext = amdflags & (1<<30);
  }

  if( exlevel >= 0x80000004 )
  {
    DWORD pstr[12];

    __asm {
      pushad
      mov eax, 0x80000002
      CPU_ID
      mov dword ptr pstr, eax
      mov dword ptr pstr+4, ebx
      mov dword ptr pstr+8, ecx
      mov dword ptr pstr+12, edx
      mov eax, 0x80000003
      CPU_ID
      mov dword ptr pstr+16, eax
      mov dword ptr pstr+20, ebx
      mov dword ptr pstr+24, ecx
      mov dword ptr pstr+28, edx
      mov eax, 0x80000004
      CPU_ID
      mov dword ptr pstr+32, eax
      mov dword ptr pstr+36, ebx
      mov dword ptr pstr+40, ecx
      mov dword ptr pstr+44, edx
      popad
    }

    char* s = (char*)pstr;
    while( *s && isspace(*s) )
      s++;

#ifdef _UNICODE
    wchar_t pstrw[48];
    char2uc( (char*)pstrw, s );
    _tcscpy_s( cpuStr, pstrw );
#else
    strcpy_s( cpuStr, s );
#endif
  }
}

void CCPUData::identifyIntel()
{
  boolean noFamily = false;
  TCHAR* cpuString = NULL;

  switch( familyid )
  {
    case 4: switch( modelid )
            {
              case 0: cpuString = _T("80486DX-25/33");
                      break;

              case 1: cpuString = _T("80486DX-50");
                      break;

              case 2: cpuString = _T("80486SX");
                      break;

              case 3: cpuString = _T("80487/DX2/DX2 Overdrive");
                      break;

              case 4: cpuString = _T("80486SL");
                      break;

              case 5: cpuString = _T("80486SX2");
                      break;

              case 6: cpuString = _T("80486DX (write-back)");
                      break;

              case 7: cpuString = _T("80486DX4/DX4 Overdrive");
                      break;

              case 8: cpuString = _T("80486DX4");
                      break;

              case 9: cpuString = _T("80486DX4 (write-back)");
                      break;

              default:cpuString = _T("Unknown 80486");
                      noFamily = true;
                      break;
            }
            break;

    case 5: switch( modelid )
            {
              case 1: if( typid == 1 )
                        cpuString = _T("Pentium OverDrive (60,66)");
                      else
                        cpuString = _T("Pentium (60,66)");
                      break;

              case 2: if( typid == 1 )
                        cpuString = _T("Pentium (75,90,100,120,133,150,166,200)");
                      else
                        cpuString = _T("Pentium OverDrive (75,90,100,120,133,150,166,200)");
                      break;

              case 3: cpuString = _T("Pentium Overdrive (for 80486)");
                      break;

              case 4: if( typid == 1 )
                        cpuString = _T("Pentium OverDrive MMC (75,90,100,120,133)");
                      else
                        cpuString = _T("Pentium MMX (166,200)");
                      break;

              case 7: cpuString = _T("P54C");
                      break;

              case 8: cpuString = _T("P55C");
                      structureSize = _T("0.25µm");
                      break;

              default:cpuString = _T("Unknown Pentium");
                      noFamily = true;
                      break;
            }
            break;

    case 6: switch( modelid )
            {
              case 0: cpuString = _T("Pentium Pro (A-Step)");
                      break;

              case 1: cpuString = _T("Pentium Pro");
                      break;

              case 3: if( typid == 1 )
                      {
                        cpuString = _T("Pentium II OverDrive");
                        structureSize = _T("0.28µm");
                      }
                      else
                      {
                        cpuString = _T("Intel Pentium II");
                        coreType = _T("Klamath");
                        structureSize = _T("0.28µm");
                      }
                      break;

              case 5: cpuString = _T("Intel Pentium II/II Xeon/Celeron");
                      structureSize = _T("0.25µm");
                      break;

              case 6: cpuString = _T("Intel Pentium II/Celeron w/ on-die-L2");
                      break;

              case 7: cpuString = _T("Intel Pentium III/III Xeon");
                      structureSize = _T("0.25µm");
                      break;
 
              case 8: cpuString = _T("Intel Pentium III/III Xeon/Celeron");
                      coreType = _T("Coppermine");
                      structureSize = _T("0.18µm");
                      cacheSize = _T("256 KB");
                      break;

              case 9: cpuString = _T("Intel Pentium-M");
                      structureSize = _T("0.13µm");
                      coreType = _T("Banias");
                      cacheSize = _T("1 MB");
                      break;

              case 10:cpuString = _T("Intel Pentium III/PIII Xeon");
                      structureSize = _T("0.18µm");
                      cacheSize = _T("2 MB");
                      coreType = _T("Coppermine");
                      break;

              case 11:cpuString = _T("Intel Pentium III");
                      structureSize = _T("0.13µm");
                      cacheSize = _T("512 KB");
                      coreType = _T("Tualatin");
                      break;

              case 13:if( steppingid != 8 )
                      {
                        cpuString = _T("Intel Pentium-M");
                        structureSize = _T("0.09µm");
                        cacheSize = _T("2 MB");
                        coreType = _T("Dothan");
                      }
                      else
                      {
                        cpuString = _T("Intel Pentium-M");
                        structureSize = _T("0.09µm");
                        cacheSize = _T("2 MB");
                        coreType = _T("Sonoma");
                      }
                      break;

              case 14:cpuString = _T("Intel Pentium-M");
                      structureSize = _T("65nm");
                      cacheSize = _T("2 MB");
                      coreType = _T("Yonah/Core Duo");
                      break;

              case 15:cpuString = _T("Intel Pentium-M");
                      structureSize = _T("65nm");
                      cacheSize = _T("4 MB");
                      coreType = _T("Core 2");
                      break;

              default:cpuString = _T("Unknown Pentium Pro/II/III/M");
                      noFamily = true;
                      break;
            }
            break;

    case 15:switch( modelid )
            {
              case 0: cpuString = _T("Intel Pentium 4");
                      structureSize = _T("0.18µm");
                      coreType = _T("Willamette");
                      break;

              case 1: cpuString = _T("Intel Pentium 4");
                      structureSize = _T("0.18µm");
                      coreType = _T("Willamette");
                      break;

              case 2: cpuString = _T("Intel Pentium 4");
                      structureSize = _T("0.13µm");
                      cacheSize = _T("512 KB");
                      coreType = _T("Northwood");
                      break;

              case 3: cpuString = _T("Intel Pentium 4");
                      structureSize = _T("0.09µm");
                      cacheSize = _T("1 MB");
                      coreType = _T("Prescott");
                      break;

              case 4: cpuString = _T("Intel Pentium 4 Xeon");
                      structureSize = _T("0.09µm");
                      cacheSize = _T("1 MB");
                      coreType = _T("Prescott");
                      break;

              case 6: cpuString = _T("Intel Pentium 4");
                      structureSize = _T("65nm");
                      cacheSize = _T("2 MB");
                      break;

              default:cpuString = _T("Unknown Pentium 4");
                      noFamily = true;
                      break;
            }
            break;

    default:cpuString = _T("Unknown Intel CPU");
            noFamily = true;
            break;
  }

  _tcscpy_s( cpuStr, cpuString );

  TCHAR* bstr = _T("");

  switch( brandID & 255 )
  {
    case 1: bstr = _T("Intel Celeron");
            structureSize = _T("0.18µm");
            coreType = _T("Coppermine");
            break;

    case 2: bstr = _T("Intel Pentium III");
            structureSize = _T("0.18µm");
            coreType = _T("Coppermine");
            break;

    case 3: bstr = _T("Intel Celeron/Pentium III Xeon");
            structureSize = _T("0.18µm");
            coreType = _T("Coppermine");
            break;

    case 4: bstr = _T("Intel Pentium III");
            structureSize = _T("0.13µm");
            coreType = _T("Tualatin");
            break;

    case 6: bstr = _T("Intel Mobile Pentium III-M");
            structureSize = _T("0.13µm");
            coreType = _T("Tualatin");
            break;

    case 7: bstr = _T("Intel Mobile Celeron");
            structureSize = _T("0.13µm");
            coreType = _T("Tualatin");
            break;

    case 8: if( family == 0xf24 )
            {
              bstr = _T("Intel Mobile Celeron 4");
              structureSize = _T("0.13µm");
              coreType = _T("Northwood");
            }
            else
            {
              bstr = _T("Intel Pentium 4");
              structureSize = _T("0.18µm");
              coreType = _T("Willamette");
            }
            break;

    case 9: bstr = _T("Intel Pentium 4");
            structureSize = _T("0.13µm");
            coreType = _T("Northwood");
            break;

    case 10:bstr = _T("Intel Celeron 4");
            structureSize = _T("0.18µm");
            coreType = _T("Willamette");
            break;

    case 11:if( familyid==15 && modelid==2 )
            {
              bstr = _T("Intel Pentium 4 Xeon");
              structureSize = _T("0.13µm");
              coreType = _T("Northwood");
            }
            else
            {
              bstr = _T("Intel Pentium 4 Xeon");
              structureSize = _T("0.18µm");
              coreType = _T("Willamette");
            }
            break;

    case 12:bstr = _T("Intel Pentium 4 Xeon MP");
            structureSize = _T("0.13µm");
            coreType = _T("Northwood");
            break;

    case 14:bstr = _T("Intel Mobile Pentium 4-M");
            structureSize = _T("0.13µm");
            coreType = _T("Northwood");
            cacheSize = _T("512 KB");
            break;

    case 15:if( family == 0xf27 )
              bstr = _T("Intel Mobile Celeron 4");
            else
              bstr = _T("Intel Mobile Pentium 4-M");
            structureSize = _T("0.13µm");
            coreType = _T("Northwood");
            break;

    case 22:if( familyid==6 && modelid==9 )
            {
              bstr = _T("Intel Pentium-M");
              structureSize = _T("0.13µm");
              cacheSize = _T("1 MB");
              coreType = _T("Banias");
            }
            else
            {
              bstr = _T("Intel Pentium-M");
              structureSize = _T("0.09µm");
              cacheSize = _T("2 MB");
              coreType = _T("Dothan");
            }
            break;
  }

  if( *bstr )
  {
    _tcscpy_s( cpuStr, bstr );
    brandStr = bstr;
  }
  else
    brandStr = cpuString;
}

void CCPUData::identifyAMD()
{
  switch( familyid )
  {
    case 4: switch( modelid )
            {
              case 3: _tcscpy_s( cpuStr, _T("486DX/2") );
                      break;

              case 7: _tcscpy_s( cpuStr, _T("486DX/2-WB") );
                      break;

              case 8: _tcscpy_s( cpuStr, _T("486DX/4") );
                      break;

              case 9: _tcscpy_s( cpuStr, _T("486DX/4-WB") );
                      break;

              case 14:_tcscpy_s( cpuStr, _T("Am5x86-WT") );
                      break;

              case 15:_tcscpy_s( cpuStr, _T("Am5x86-WB") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown 486/5x86 CPU") );
                      break;
            }
            break;

    case 5: switch( modelid )
            {
              case 0: _tcscpy_s( cpuStr, _T("K5/SSA5 (PR75...PR100)") );
                      break;

              case 1: _tcscpy_s( cpuStr, _T("K5 (PR120/PR133)") );
                      break;

              case 2: _tcscpy_s( cpuStr, _T("K5 (PR150/PR166)") );
                      break;

              case 3: _tcscpy_s( cpuStr, _T("K5 (PR200)") );
                      break;

              case 6: _tcscpy_s( cpuStr, _T("K6 (166...233MHz)") );
                      break;

              case 7: _tcscpy_s( cpuStr, _T("K6 (266...300MHz)") );
                      break;

              case 8: _tcscpy_s( cpuStr, _T("K6-2 w/ 3DNow!") );
                      break;

              case 9: _tcscpy_s( cpuStr, _T("K6-3 w/ 3DNow!") );
                      break;

              case 13:_tcscpy_s( cpuStr, _T("K6-2+ / K6-III+ w/ 3DNow!") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown K6 CPU") );
                      break;
            }
            break;

    case 6: switch( modelid )
            {
              case 0:
              case 1: _tcscpy_s( cpuStr, _T("Athlon (25um)") );
                      break;

              case 2: _tcscpy_s( cpuStr, _T("Athlon (18um)") );
                      break;

              case 3: _tcscpy_s( cpuStr, _T("Duron") );
                      break;

              case 4: _tcscpy_s( cpuStr, _T("Athlon (Thunderbird)") );
                      break;

              case 6: _tcscpy_s( cpuStr, _T("Athlon (Palomino)") );
                      break;

              case 7: _tcscpy_s( cpuStr, _T("Duron (Morgan)") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown Athlon") );
                      break;
            }
            break;

    default:_tcscpy_s( cpuStr, _T("Unknown AMD CPU") );
            break;
  }
}

void CCPUData::identifyCyrix()
{
  switch( familyid )
  {
    case 4: switch( modelid )
            {
              case 4: _tcscpy_s( cpuStr, _T("MediaGX") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown Cyrix CPU") );
                      break;
            }
            break;

    case 5: switch( modelid )
            {
              case 2: _tcscpy_s( cpuStr, _T("6x86/6x86L") );
                      break;

              case 4: _tcscpy_s( cpuStr, _T("MediaGX MMX enh.") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown Cyrix CPU") );
                      break;
            }
            break;
     
    case 6: switch( modelid )
            {
              case 0: _tcscpy_s( cpuStr, _T("m II") );
                      break;

              case 5: _tcscpy_s( cpuStr, _T("VIA Cyrix M2 core") );
                      break;

              case 6: _tcscpy_s( cpuStr, _T("WinChip C5A") );
                      break;

              case 7: _tcscpy_s( cpuStr, _T("WinChip C5B") );
                      break;

              default:_tcscpy_s( cpuStr, _T("Unknown Cyrix CPU") );
                      break;
            }
            break;

    default:_tcscpy_s( cpuStr, _T("Unknown Cyrix CPU") );
            break;
  }
}

void CCPUData::init()
{
  features = 0;
  exfeatures = 0;
  typid = 0;
  modelid = 0;
  steppingid = 0;
  *vendorID = '\0';
  vendor = -1;
  maxIDVal = 0;
  exlevel = 0;
  brandID = 0;
  exfeatures2 = 0;
  brandStr = _T("");
  cacheSize = NULL;
  structureSize = NULL;
  coreType = NULL;
  cacheStrNo = 0;
  flagNo = 0;

  f_fpu = f_vme = f_de = f_pse = f_tsc = f_msr = f_pae = f_mce = f_c8x = f_apic = f_sep = 
  f_mtrr = f_pge = f_mca = f_cmov = f_pse36 = f_psn = f_mmx = f_pat = f_fxsr = f_sse = f_sse2 = 
  f_fsc = f_fcmov = f_mmxext = f_3dnowext = f_3dnow = f_cxmmx = f_ss = f_htt = f_tm1 = f_ia64 = 
  f_acpi = f_pbe = f_sse3 = f_mon = f_cpl = f_vmx = f_est = f_tm2 = f_l1id = f_c16b = FALSE;
}

void CCPUData::getCPUFeatures()
{
  DWORD cpuff = 0;
  DWORD cpuffext = 0;
  DWORD bid = 0;
  DWORD fam = 0;
  BYTE vid[16] = "------------";
  DWORD maxl = 0;

  __asm {      
      pushad
      xor eax,eax
      CPU_ID
      mov dword ptr vid,ebx
      mov dword ptr vid[+4],edx
      mov dword ptr vid[+8],ecx
      mov dword ptr maxl,eax

      cmp eax,1
      jl end_cpuff
      xor eax,eax
      inc eax
      CPU_ID
      mov cpuff,edx
      mov cpuffext,ecx
      mov bid,ebx
      mov fam,eax

    end_cpuff:
      popad
  }

#ifdef _UNICODE
  wchar_t pstrw[16];
  char2uc( (char*)pstrw, (char*)vid );
  _tcscpy_s( vendorID, pstrw );
#else
  strcpy_s( vendorID, (char*)vid );
#endif

  family = fam;
  brandID = bid;
  features = cpuff;
  exfeatures = cpuffext;
  maxIDVal = maxl;
}

/////////////////////////////////////////////////////////////////////////////
void CCPUData::Message( LPCTSTR lpszMessage )
{
  if( flagNo < 64 )
    flag[flagNo++] = lpszMessage;

  _tcscat_s( flagText, lpszMessage );
  _tcscat_s( flagText, _T("\r\n") );
}

void CCPUData::readCPUIDLevel( int level, int level2, ULONG* target )
{
  ULONG x[4];

  __asm {      
      pushad
      mov eax,level
      mov ecx,level2
      CPU_ID
      mov dword ptr x,eax
      mov dword ptr x[+4],ebx
      mov dword ptr x[+8],ecx
      mov dword ptr x[+12],edx
      popad
  }

  target[0] = x[0];
  target[1] = x[1];
  target[2] = x[2];
  target[3] = x[3];
}

static CACHEID cids[] = 
{
  { 0x06, _T("L1 instruction cache: 8 KBytes, 4-way set associative, 32 byte line size") },
  { 0x08, _T("L1 instruction cache: 16 KBytes, 4-way set associative, 32 byte line size") },
  { 0x0a, _T("L1 data cache: 8 KBytes, 2-way set associative, 32 byte line size") },
  { 0x0c, _T("L1 data cache: 16 KBytes, 4-way set associative, 32 byte line size") },
  { 0x22, _T("L3 cache: 512 KBytes, 4-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x23, _T("L3 cache: 1 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x25, _T("L3 cache: 2 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x29, _T("L3 cache: 4 MBytes, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x2c, _T("L1 data cache: 32 KBytes, 8-way set associative, 64 byte line size") },
  { 0x30, _T("L1 instruction cache: 32 KBytes, 8-way set associative, 64 byte line size") },
  { 0x41, _T("L2 cache: 128 KBytes, 4-way set associative, 32 byte line size") },
  { 0x42, _T("L2 cache: 256 KBytes, 4-way set associative, 32 byte line size") },
  { 0x43, _T("L2 cache: 512 KBytes, 4-way set associative, 32 byte line size") },
  { 0x44, _T("L2 cache: 1 MByte, 4-way set associative, 32 byte line size") },
  { 0x45, _T("L2 cache: 2 MByte, 4-way set associative, 32 byte line size") },
  { 0x46, _T("L3 cache: 4 MByte, 4-way set associative, 64 byte line size") },
  { 0x47, _T("L3 cache: 8 MByte, 8-way set associative, 64 byte line size") },
  { 0x60, _T("L1 data cache: 16 KByte, 8-way set associative, 64 byte line size") },
  { 0x66, _T("L1 data cache: 8 KByte, 4-way set associative, 64 byte line size") },
  { 0x67, _T("1 data cache: 16 KByte, 4-way set associative, 64 byte line size") },
  { 0x68, _T("L1 data cache: 32 KByte, 4-way set associative, 64 byte line size") },
  { 0x70, _T("Trace cache: 12K-µop, 8-way set associative") },
  { 0x71, _T("Trace cache: 16K-µop, 8-way set associative") },
  { 0x72, _T("Trace cache: 32K-µop, 8-way set associative") },
  { 0x78, _T("L2 cache: 1 MByte, 4-way set associative, 64byte line size") },
  { 0x79, _T("L2 cache: 128 KByte, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x7a, _T("L2 cache: 256 KByte, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x7b, _T("L2 cache: 512 KByte, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x7c, _T("L2 cache: 1 MByte, 8-way set associative, 64 byte line size, 2 lines per sector") },
  { 0x7d, _T("L2 cache: 2 MByte, 8-way set associative, 64byte line size") },
  { 0x7f, _T("L2 cache: 512 KByte, 2-way set associative, 64-byte line size") },
  { 0x82, _T("L2 cache: 256 KByte, 8-way set associative, 32 byte line size") },
  { 0x83, _T("L2 cache: 512 KByte, 8-way set associative, 32 byte line size") },
  { 0x84, _T("L2 cache: 1 MByte, 8-way set associative, 32 byte line size") },
  { 0x85, _T("L2 cache: 2 MByte, 8-way set associative, 32 byte line size") },
  { 0x86, _T("L2 cache: 512 KByte, 4-way set associative, 64 byte line size") },
  { 0x87, _T("L2 cache: 1 MByte, 8-way set associative, 64 byte line size") },
  { 0x00, NULL }
};

void CCPUData::readCacheInfo()
{
  ULONG cid[4];
  
  readCPUIDLevel( 2, 0, cid );

  for( int i=0; i<4; i++ )
  {
    // MSB darf nicht gesetzt sein (EAX, EBX, ECX, EDX)
    if( !(cid[i] & (1<<31)) )
    {
      // LSByte von EAX (AL) ignorieren
      if( i != 0 )
        printCacheInfo( cid[i] & 0xff );

      printCacheInfo( (cid[i]>>8) & 0xff );
      printCacheInfo( (cid[i]>>16) & 0xff );
      printCacheInfo( (cid[i]>>24) & 0xff );
    }
  }
}

void CCPUData::printCacheInfo( int value )
{
  int i=0;

  if( value <= 0 )
    return;

  for( ;; )
  {
    if( cids[i].id == 0 )
      break;
    else if( cids[i].id == value )
    {
      if( cacheStrNo < 31 )
        cacheStr[cacheStrNo++] = (CString)cids[i].text;

      break;
    }
    
    i++;
  }
}
