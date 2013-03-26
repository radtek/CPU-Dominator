/***************************************************************
* H file: cpuid.h... for cpuinf32 DLL
*
*   This program has been developed by Intel Corporation.  
*		You have Intel's permission to incorporate this code 
*   into your product, royalty free.
*
*  Copyright (c) 1995, Intel Corporation.  All rights reserved.
*
*  Runtime optimizations & modifications for SpeedswitchXP 
*    by C. Diefer
***************************************************************/


#ifndef cpuid_h
#define cpuid_h



// OPCODE DEFINITIONS //////////////////////////////////////////
#define CPU_ID _asm _emit 0x0f _asm _emit 0xa2  
                                        // CPUID instruction

#define RDTSC  _asm _emit 0x0f _asm _emit 0x31  
                                        // RDTSC instruction


// VARIABLE STRUCTURE DEFINITIONS //////////////////////////////
struct TIME_STAMP
{
    DWORD High;                 // Upper 32-bits of Time Stamp
                                //   Register value
    
    DWORD Low;                  // Lower 32-bits of Time Stamp
};                              //   Register value

// extern BYTE vendor_id[16];
// extern DWORD family;
// extern DWORD maxIDVal;
// extern DWORD cpuffext;
// extern DWORD brandid;

// Function Prototypes /////////////////////////////////////////

/***************************************************************
* WORD wincpuidsupport()
* =================================
* Wincpuidsupport() tells the caller whether the host processor
* supports the CPUID opcode or not.
*
* Inputs: none
*
* Returns:
*  1 = CPUID opcode is supported
*  0 = CPUID opcode is not supported
***************************************************************/

WORD wincpuidsupport();


/***************************************************************
* WORD wincpuid()
* ===============
* This routine uses the standard Intel assembly code to 
* determine what type of processor is in the computer, as
* described in application note AP-485 (Intel Order #241618).
* Wincpuid() returns the CPU type as an integer (that is, 
* 2 bytes, a WORD) in the AX register.
*
* Returns:
*  0 = 8086/88
*  2 = 80286
*  3 = 80386
*  4 = 80486
*  5 = Pentium(R) Processor
*  6 = PentiumPro(R) Processor
*  7 or higher = Processor beyond the PentiumPro6(R) Processor
*
*  Note: This function also sets the global variable clone_flag
***************************************************************/
WORD  wincpuid();


/***************************************************************
* DWORD wincpufeatures()
* ======================
* Wincpufeatures() returns the CPU features flags as a DWORD 
*    (that is, 32 bits).
*
* Inputs: none
*
* Returns:
*   0 = Processor which does not execute the CPUID instruction.
*          This includes 8086, 8088, 80286, 80386, and some 
*          older 80486 processors.                       
*
* Else
*   Feature Flags (refer to App Note AP-485 for description).
*      This DWORD was put into EDX by the CPUID instruction.
*
*   Current flag assignment is as follows:
*
*       bit31..10   reserved (=0)
*       bit9=1      CPU contains a local APIC (iPentium-3V)
*       bit8=1      CMPXCHG8B instruction supported
*       bit7=1      machine check exception supported
*       bit6=0      reserved (36bit-addressing & 2MB-paging)
*       bit5=1      iPentium-style MSRs supported
*       bit4=1      time stamp counter TSC supported
*       bit3=1      page size extensions supported
*       bit2=1      I/O breakpoints supported
*       bit1=1      enhanced virtual 8086 mode supported
*       bit0=1      CPU contains a floating-point unit (FPU)
*
*   Note: New bits will be assigned on future processors... see
*         processor data books for updated information
*
*   Note: This function also sets the global variable clone_flag
***************************************************************/
DWORD wincpufeatures();


/***************************************************************
* struct TIME_STAMP winrdtsc()
* ============================
* Winrdtsc() returns the value in the Time Stamp Counter (if one
* exists).
*
* Inputs: none
*
* Returns:
*   0 = CPU does not support the time stamp register
*
* Else
*   Returns a variable of type TIME_STAMP which is composed of 
*      two DWORD variables. The 'High' DWORD contains the upper
*      32-bits of the Time Stamp Register. The 'Low' DWORD 
*      contains the lower 32-bits of the Time Stamp Register.
*
*  Note: This function also sets the global variable clone_flag
***************************************************************/
struct TIME_STAMP winrdtsc();


// Private Function Declarations ///////////////////////////////

/***************************************************************
* static WORD check_clone()
*
* Inputs: none
*
* Returns:
*   1      if processor is clone (limited detection ability)
*   0      otherwise
***************************************************************/
static WORD check_clone();


/***************************************************************
* static WORD check_8086()
*
* Inputs: none
*
* Returns: 
*   0      if processor 8086
*   0xffff otherwise
***************************************************************/
static WORD check_8086();


/***************************************************************
* static WORD check_80286()
*
* Inputs: none
*
* Returns:
*   2      if processor 80286
*   0xffff otherwise
***************************************************************/
static WORD check_80286();


/***************************************************************
* static WORD check_80386()
*
* Inputs: none
*
* Returns:
*   3      if processor 80386
*   0xffff otherwise
***************************************************************/
static WORD check_80386();


/***************************************************************
* static WORD check_IDProc()
* ==========================
* Check_IDProc() uses the CPUID opcode to find the family type
* of the host processor.
*
* Inputs: none
*
* Returns:
*  CPU Family (i.e. 4 if Intel 486, 5 if Pentium(R) Processor)
*
*  Note: This function also sets the global variable clone_flag
***************************************************************/
static WORD check_IDProc();

#endif cpuid_h
