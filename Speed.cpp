/***************************************************************
* C file:  Speed.c... for cpuinf32 DLL
*
*       This program has been developed by Intel Corporation.  
*       You have Intel's permission to incorporate this code 
*       into your product, royalty free.  Intel has various 
*       intellectual property rights which it may assert under
*       certain circumstances, such as if another manufacturer's
*       processor mis-identifies itself as being "GenuineIntel"
*       when the CPUID instruction is executed.
*
*       Intel specifically disclaims all warranties, express or
*       implied, and all liability, including consequential and
*       other indirect damages, for the use of this code, 
*       including liability for infringement of any proprietary
*       rights, and including the warranties of merchantability
*       and fitness for a particular purpose.  Intel does not 
*       assume any responsibility for any errors which may 
*       appear in this code nor any responsibility to update it.
*
*  * Other brands and names are the property of their respective
*    owners.
*
*  Copyright (c) 1995, Intel Corporation.  All rights reserved.
*
*  Runtime optimizations & modifications by Christian Diefer
***************************************************************/

#define _UNICODE

#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "speed.h"
#include "cpuid.h"

#define ROUND_THRESHOLD 6

static struct FREQ_INFO GetRDTSCCpuSpeed();
static struct FREQ_INFO GetBSFCpuSpeed(ulong cycles);

// Number of cycles needed to execute a single BSF instruction.
//    Note that processors below i386(tm) are not supported.
static ulong processor_cycles[] = {
    00,  00,  00, 115, 47, 43, 
    38,  38,  38, 38,  38, 38, 
};

ushort sproc;
DWORD sfeat;
static int firstrun = 1;

/***************************************************************
* CpuSpeed() -- Return the raw clock rate of the host CPU.
*
* Inputs:
*   clocks:     0: Use default value for number of cycles
*                  per BSF instruction.
*               -1: Use CMos timer to get cpu speed.
*               Positive Integer: Use clocks value for number
*                  of cycles per BSF instruction.
*
* Returns:
*       If error then return all zeroes in FREQ_INFO structure
*       Else return FREQ_INFO structure containing calculated 
*       clock frequency, normalized clock frequency, number of 
*       clock cycles during test sampling, and the number of 
*       microseconds elapsed during the sampling.
***************************************************************/

struct FREQ_INFO cpuspeed( int clocks ) 
{
  ulong cycles=0;              // Clock cycles elapsed during test
  struct FREQ_INFO cpu_speed;  // Return structure for cpuspeed
  ushort processor;            // Family of processor
  DWORD features;              // Features of Processor


  if( firstrun )
  {
    processor = sproc = wincpuid();
    features = sfeat = wincpufeatures();
    firstrun = 0;
  }
  else
  {
    processor = sproc;
    features = sfeat;
  }

  memset( &cpu_speed, 0x00, sizeof(cpu_speed) );

  // Check for manual BSF instruction clock count
  cycles = ITERATIONS * processor_cycles[processor];

  if( features & TSC_SUPPORT )
  {                       
    // On processors supporting the Read Time Stamp opcode, compare elapsed time on
    //   the High-Resolution Counter with elapsed cycles on the Time Stamp Register.
    return GetRDTSCCpuSpeed();
  }
  else if( processor >= 3 ) 
    return GetBSFCpuSpeed( cycles );

  return cpu_speed;
} // cpuspeed()


static struct FREQ_INFO GetBSFCpuSpeed( ulong cycles )
{
  // If processor does not support time stamp reading, but is at least a 386 or above, utilize method of 
  //   timing a loop of BSF instructions which take a known number of cycles to run on i386(tm), i486(tm), 
  //   and Pentium(R) processors.

  LARGE_INTEGER t0,t1;        // Variables for High-Resolution Performance Counter reads
  ulong freq = 0;             // Most current frequ. calculation
  ulong ticks;                // Microseconds elapsed during test
  LARGE_INTEGER count_freq;   // High Resolution Performance Counter frequency
  int i;                      // Temporary Variable
  ulong current = 0;          // Variable to store time elapsed during loop of BSF instructions
  ulong lowest = ULONG_MAX;   // Since algorithm finds the lowest value out of a set of samplings, 
                              //   this variable is set intially to the max unsigned long value). 
                              //   This guarantees that the initialized value is not later used as 
                              //   the least time through the loop.
  struct FREQ_INFO cpu_speed;


  memset( &cpu_speed, 0x00, sizeof(cpu_speed) );

  if( !QueryPerformanceFrequency(&count_freq) )
    return cpu_speed;

  for( i=0; i < SAMPLINGS; i++ )
  {       
    // Sample Ten times. Can be increased or decreased depending on accuracy vs. time requirements

    QueryPerformanceCounter( &t0 );   // Get start time

    _asm {
        mov eax, 80000000h  
        mov bx, ITERATIONS      // Number of consecutive BSF instructions to execute. Set identical to 
                                //   nIterations constant in speed.h
      loop1:  
        bsf ecx,eax
        dec bx
        jnz loop1
    }
                             
    QueryPerformanceCounter( &t1 );   // Get end time
    
    // Number of external ticks is difference between two hi-res counter reads
    current = (ulong) t1.LowPart - (ulong) t0.LowPart;  

    if( current < lowest )        // Take lowest elapsed time to account for some
      lowest = current;           //   samplings being interrupted by other operations 
  }                              
         
  ticks = lowest;

  // Note that some seemingly arbitrary mulitplies and divides are done below. This is to maintain a 
  //   high level of precision without truncating the most significant data. According to what value 
  //   ITERATIIONS is set to, these multiplies and divides might need to be shifted for optimal precision.

  ticks = ticks * 100000;         // Convert ticks to hundred thousandths of a tick
            
  ticks = ticks / (count_freq.LowPart/10);  // Hundred Thousandths of a Ticks / ( 10 ticks/second )
                                            //   = microseconds (us)

  if( ticks%count_freq.LowPart > count_freq.LowPart/2 )  
    ticks++;                      // Round up if necessary
            
  freq = cycles/ticks;            // Cycles / us  = MHz

  cpu_speed.raw_freq  = freq;
  if( cycles%ticks > ticks/2 )
    freq++;                       // Round up if necessary    

  cpu_speed.in_cycles = cycles;   // Return variable structure
  cpu_speed.ex_ticks  = ticks;    //   determined by one of 
  cpu_speed.norm_freq = freq;

  return cpu_speed;
}   

static struct FREQ_INFO GetRDTSCCpuSpeed()
{
  ulong stamp0=0, stamp1=0;         // Time Stamp Variable for beginning and end of test
  struct FREQ_INFO cpu_speed;
  LARGE_INTEGER t0,t1;              // Variables for High-Resolution Performance Counter reads
  ulong freq  =0;                   // Most current frequ. calculation
  ulong freq2 =0;                   // 2nd most current frequ. calc.
  ulong freq3 =0;                   // 3rd most current frequ. calc.
  ulong total;                      // Sum of previous three frequency calculations
  int tries=0;                      // Number of times a calculation has been made on this call to cpuspeed
  ulong total_cycles=0, cycles=0;   // Clock cycles elapsed during test
  ulong total_ticks=0, ticks;       // Microseconds elapsed during test
  LARGE_INTEGER count_freq;         // High Resolution Performance Counter frequency

#ifdef WIN32
  int iPriority;
  HANDLE hThread = GetCurrentThread();
#endif


  memset( &cpu_speed, 0x00, sizeof(cpu_speed) );

  if( !QueryPerformanceFrequency(&count_freq) )
    return cpu_speed;

  // On processors supporting the Read Time Stamp opcode, compare elapsed time on the 
  //   High-Resolution Counter with elapsed cycles on the Time Stamp Register.
    
  // This do loop runs up to 20 times or until the average of the previous three calculated frequencies is 
  //   within 1 MHz of each of the individual calculated frequencies. This resampling increases the 
  //   accuracy of the results since outside factors could affect this calculation
  do 
  {          
    tries++;        // Increment number of times sampled on this call to cpuspeed
            
    freq3 = freq2;  // Shift frequencies back to make room for new frequency measurement
    freq2 = freq;

    QueryPerformanceCounter( &t0 );   // Get high-resolution performance counter time
            
    t1.LowPart = t0.LowPart;          // Set Initial time
    t1.HighPart = t0.HighPart;

#ifdef WIN32
    iPriority = GetThreadPriority( hThread );
    if( iPriority != THREAD_PRIORITY_ERROR_RETURN )
      SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );
#endif // WIN32

    while( (ulong)t1.LowPart-(ulong)t0.LowPart < 50 ) 
    {     
      // Loop until 50 ticks have passed since last read of hi-res counter. 
      //   This accounts for overhead later.
      QueryPerformanceCounter( &t1 );

      RDTSC;                      // Read Time Stamp
      _asm {
          MOV stamp0, EAX
      }
    }   
            
    t0.LowPart = t1.LowPart;        // Reset Initial Time
    t0.HighPart = t1.HighPart;

    while( (ulong)t1.LowPart-(ulong)t0.LowPart < 1000 ) 
    {
      // Loop until 1000 ticks have passed since last read of hi-res counter. 
      //   This allows for elapsed time for sampling.
      QueryPerformanceCounter(&t1);
  
      RDTSC;                      // Read Time Stamp
      __asm {
           MOV stamp1, EAX
      }
    }

#ifdef WIN32
    // Reset priority
    if( iPriority != THREAD_PRIORITY_ERROR_RETURN )
      SetThreadPriority( hThread, iPriority );
 #endif

    // Number of internal clock cycles is difference between two time stamp readings.
    cycles = stamp1 - stamp0;   

    // Number of external ticks is difference between two hi-res counter reads.
    ticks = (ulong) t1.LowPart - (ulong) t0.LowPart;    


    // Note that some seemingly arbitrary mulitplies and divides are done below. This is to maintain a 
    //   high level of precision without truncating the most significant data. According to what value 
    //   ITERATIIONS is set to, these multiplies and divides might need to be shifted for optimal precision.

    // Convert ticks to hundred thousandths of a tick
    ticks = ticks * 100000;

    // Hundred Thousandths of a Ticks / ( 10 ticks/second ) = microseconds (us)
    ticks = ticks / ( count_freq.LowPart/10 );      

    total_ticks += ticks;
    total_cycles += cycles;

    if( ticks%count_freq.LowPart > count_freq.LowPart/2 )
      ticks++;            // Round up if necessary

    freq = cycles/ticks;    // Cycles / us  = MHz

    if( cycles%ticks > ticks/2 )
      freq++;             // Round up if necessary

    total = ( freq + freq2 + freq3 );     // Total last three frequency calculations
  } while( (tries < 3 ) ||        
           (tries < 20)&&
           ((abs(3 * (long)freq -(long)total) > 3*TOLERANCE )||
            (abs(3 * (long)freq2-(long)total) > 3*TOLERANCE )||
            (abs(3 * (long)freq3-(long)total) > 3*TOLERANCE )));
    // ... Compare last three calculations to average of last three calculations.                           

  // Try one more significant digit.
  freq3 = ( total_cycles * 10 ) / total_ticks;
  freq2 = ( total_cycles * 100 ) / total_ticks;

  if( freq2 - (freq3 * 10) >= ROUND_THRESHOLD )
    freq3++;

  cpu_speed.raw_freq = total_cycles / total_ticks;
  cpu_speed.norm_freq = cpu_speed.raw_freq;

  freq = cpu_speed.raw_freq * 10;
  if( (freq3 - freq) >= ROUND_THRESHOLD )
    cpu_speed.norm_freq++;

  cpu_speed.ex_ticks = total_ticks;
  cpu_speed.in_cycles = total_cycles;

  return cpu_speed;
}

//***************************************************************
//
// Function: cpuTimeStamp
//
//      Returns the pentium cpu time stamp in 2 32 bit unsigned longs
//
//  Notes: maintains a flag to make sure the cpu supports the RDTSC instruction.  There is
//      the overhead of checking the cpu the first time afterwhich the time consumed in 
//      checking the flag is very minimal.  You could adjust the count but then you would
//      have to do 64bit math.  ugh.
//
//***************************************************************
unsigned long cpuTimeStamp(unsigned long *hi, unsigned long *low)
{
  unsigned long ulHi = 0L;
  unsigned long ulLow = 0L;

  __asm {
    ;RDTSC
    _emit 0Fh
    _emit 31h
    mov ulLow, eax
    mov ulHi, edx
  }       

  *hi = ulHi;
  *low = ulLow;

  return ulLow;       
}   
