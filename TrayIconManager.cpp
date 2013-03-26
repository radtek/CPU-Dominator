/*
   SpeedswitchXP V1.5
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
#include "TOptions.h"
#include "AtlBase.h"
#include "TrayIconManager.h"
#include <winuser.h>

static unsigned char digits[25][8] = 
{
  // Ziffern 0-9 in 8x8:
  { 0x3c,0x42,0x42,0x42,0x42,0x42,0x42,0x3c },
  { 0x10,0x30,0x70,0x10,0x10,0x10,0x10,0x7c },
  { 0x3c,0x42,0x02,0x04,0x08,0x10,0x20,0x7e },
  { 0x3c,0x42,0x02,0x0c,0x02,0x02,0x42,0x3c },
  { 0x08,0x18,0x28,0x48,0xfe,0x08,0x08,0x3e },
  { 0x7e,0x40,0x40,0x7c,0x02,0x02,0x42,0x3c },
  { 0x1c,0x20,0x40,0x7c,0x42,0x42,0x42,0x3c },
  { 0x7c,0x44,0x08,0x08,0x10,0x10,0x20,0x20 },
  { 0x3c,0x42,0x42,0x3c,0x42,0x42,0x42,0x3c },
  { 0x3c,0x42,0x42,0x42,0x3e,0x02,0x04,0x38 },

  // Ziffern 0-9 in 5x8:
  { 0x60,0x90,0x90,0x90,0x90,0x90,0x90,0x60 },
  { 0x20,0x20,0x60,0x20,0x20,0x20,0x20,0x70 },
  { 0x60,0x90,0x10,0x20,0x20,0x40,0x40,0xf0 },
  { 0x60,0x90,0x10,0x20,0x10,0x10,0x90,0x60 },
  { 0x20,0x60,0x60,0xa0,0xf0,0x20,0x20,0x70 },
  { 0xf0,0x80,0x80,0xe0,0x10,0x10,0x90,0x60 },
  { 0x70,0x80,0x80,0xe0,0x90,0x90,0x90,0x60 },
  { 0xf0,0x90,0x10,0x10,0x20,0x20,0x20,0x20 },
  { 0x60,0x90,0x90,0x60,0x90,0x90,0x90,0x60 },
  { 0x60,0x90,0x90,0x90,0x70,0x10,0x20,0x60 },

  // 1 zusätzliches Zeichen für '-' in 8x8:
  { 0x00,0x00,0x00,0x7e,0x7e,0x00,0x00,0x00 },

  // 4 zusätzliche Zeichen für 'Sl' und 'Hi' in 8x8:
  { 0x3e,0x40,0x40,0x40,0x3c,0x02,0x02,0x7c },
  { 0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x3c },
  { 0x42,0x42,0x42,0x42,0x7e,0x42,0x42,0x42 },
  { 0x00,0x00,0x10,0x00,0x10,0x10,0x10,0x38 }
};

static int colorVal[16][3] = 
{
  {0, 0, 0},			// Black
  {128, 0, 0},		// Dark Red
  {0, 128, 0},		// Dark Green
  {128, 128, 0},	// Dark Yellow
  {0, 0, 128},		// Dark Blue
  {128, 0, 128},	// Dark Magenta
  {0, 128, 128},	// Dark Cyan	
  {192, 192, 192},// Light Grey
  {128, 128, 128},// Medium Grey
  {255, 0, 0},		// Red
  {0, 255, 0},		// Green
  {255, 255, 0},	// Yellow
  {0, 0, 255},  	// Blue
  {255, 0, 255},	// Magenta
  {0, 255, 255},	// Cyan
  {255, 255, 255}	// White
};


//---------------------------------------------------
// füllt die Bitmap für ein Icon
//   upperStatus: true, wenn der obere Iconteil gefüllt werden soll
//   upperValue: der im oberen Iconbereich anzuzeigende Wert
//   lowerStatus: true, wenn der untere Iconteil gefüllt werden soll
//   lowerValue: der im unteren Iconbereich anzuzeigende Wert
//   iconType: 0=1.Systemicon, 1=2.Systemicon, 2=Lüftericon
//---------------------------------------------------
HICON TTrayIconManager::createIcon( bool upperStatus, int upperValue, bool lowerStatus, int lowerValue )
{
  DWORD dwWidth, dwHeight;
  BITMAPINFO* bi;
  HBITMAP hBitmap;
  void *lpBits;
  HICON hIcon = NULL;

  dwWidth  = 16;
  dwHeight = 16;

  bi = (BITMAPINFO*)new char[ sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2]; 

  ZeroMemory( bi, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 2 );

  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi->bmiHeader.biWidth = dwWidth;
  bi->bmiHeader.biHeight = dwHeight;
  bi->bmiHeader.biPlanes = 1;
  bi->bmiHeader.biBitCount = 1;
  bi->bmiHeader.biCompression = BI_RGB;

  bi->bmiColors[0].rgbRed = colorVal[options.iconColor1][0];
  bi->bmiColors[0].rgbGreen = colorVal[options.iconColor1][1];
  bi->bmiColors[0].rgbBlue = colorVal[options.iconColor1][2];
  bi->bmiColors[1].rgbRed = colorVal[options.iconColor2][0];
  bi->bmiColors[1].rgbGreen = colorVal[options.iconColor2][1];
  bi->bmiColors[1].rgbBlue = colorVal[options.iconColor2][2];
  bi->bmiColors[0].rgbReserved = 0;
  bi->bmiColors[1].rgbReserved = 0;

  HDC hdc = ::GetDC( NULL );

  hBitmap = CreateDIBSection( hdc, 
                              bi, 
                              DIB_RGB_COLORS,
                              (void**)&lpBits, 
                              NULL, 
                              (DWORD)0 );

  BYTE* bitmap = (BYTE*)lpBits;

  createTempIcon( upperStatus?upperValue:lowerValue, (upperStatus&&lowerStatus)?1:0, bitmap );

  if( upperStatus && lowerStatus )
    createTempIcon( lowerValue, 2, bitmap );

  // Create mask bitmap
  BYTE andMask[128];

  for( int i=0; i<128; i++ )
    andMask[i] = 0x00;

  if( options.iconTransparent )
  {
    int k = 15;

    for( int i=0; i<16; i++ )
    {
      for( int j=0; j<2; j++ )
        andMask[k*2+j] = bitmap[i*4+j];
      k--;
    }
  }

  HBITMAP hMonoBitmap = CreateBitmap( dwWidth, dwHeight, 1, 1, andMask );

  ICONINFO ii;
  ii.fIcon = TRUE;
  ii.xHotspot = 0;
  ii.yHotspot = 0;
  ii.hbmMask = hMonoBitmap;
  ii.hbmColor = hBitmap;

  hIcon = CreateIconIndirect( &ii );

  ::ReleaseDC( glb_hWnd, hdc );
  ::DeleteObject( hBitmap );
  ::DeleteObject( hMonoBitmap );

  delete bi;

  return hIcon;
}


//---------------------------------------------------------------
// Iconbitmap mit bestimmtem Wert füllen
//
// temp: 0-200 = normale Temperatur oder Last (0-100)
//       -3    = '--' anzeigen
//       -4    = 'Sl' anzeigen
//       -5    = 'Hi' anzeigen
//---------------------------------------------------------------
void TTrayIconManager::createTempIcon( int temp, int where, BYTE* and )
{
  int num0 = (temp/100)%10;   // erste Dezimalziffer
  int num1 = (temp/10)%10;    // zweite Dezimalziffer
  int num2 = temp%10;         // dritte Dezimalziffer
  int j;
  int lstart, lend, yscaling;

  if( temp == -3 )      // Spezialfall für 'inaktiv' ("--")
  {
    num0 = 0;
    num1 = 20;
    num2 = 20;
  }
  else if( temp == -4 )   // Zeichen für 'Sl' auswählen
  {
    num0 = 0;
    num1 = 21;
    num2 = 22;
  }
  else if( temp == -5 )   // Zeichen für 'Hi' auswählen
  {
    num0 = 0;
    num1 = 23;
    num2 = 24;
  }

  if( where == 0 )        // nur ein Wert im Icon
  {
    lstart = 0;     // komplette Iconbitmap damit füllen
    lend = 15;
    yscaling = 2;   // jede Ziffer auf 16 Zeilen erweitern
  }
  else if( where == 1 )   // obere Hälfte des Icons füllen
  {
    lstart = 8;     // nur obere Hälfte der Bitmap füllen
    lend = 15;
    yscaling = 1;   // keine Skalierung (8 Zeilen)
  }
  else if( where == 2 )   // untere Hälfte des Icons füllen
  {
    lstart = 0;     // nur untere Hälfte der Bitmap füllen
    lend = 7;
    yscaling = 1;   // keine Skalierung (8 Zeilen)
  }

  if( num0 != 0 )   // zur Anzeige von Fahrenheitwerten
  {
    j=7;
    for( int i=lstart; i<=lend; i+=yscaling )
    {
      for( int m=0; m<yscaling; m++ )   // eine Iconzeile pro Durchlauf
      {
        and[(i+m)*4] = 0xff ^ (digits[num0+10][j] | digits[num1+10][j]>>5);
        and[(i+m)*4+1] = 0xff ^ (digits[num1+10][j]<<3 | digits[num2+10][j]>>2);
      }
 
      j--;   // weiter im digits2-Array
    }
  }
  else              // Standardwerte (Celsius)
  {
    j=7;

    for( int i=lstart; i<=lend; i+=yscaling )    // Iconregion füllen
    {
      for( int m=0; m<yscaling; m++ )   // eine Iconzeile pro Durchlauf
      {
        and[(i+m)*4] = 0xff ^ digits[num1][j];
        and[(i+m)*4+1] = 0xff ^ digits[num2][j];
      }
 
      j--;   // weiter im digits-Array
    }
  }
}

