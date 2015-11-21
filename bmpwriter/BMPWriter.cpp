/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>

BOOL CreateConsole(void) {
	FreeConsole();        //to be safe :)
	if(AllocConsole()) {
		int hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
		*stdout = *(::_fdopen(hCrt, "w"));
		::setvbuf(stdout, NULL, _IONBF, 0);
		*stderr = *(::_fdopen(hCrt, "w"));
		::setvbuf(stderr, NULL, _IONBF, 0);
		return TRUE;
	}
	return FALSE;
}


inline void debug(int intVal) {
	wprintf(L"%d",intVal);
}

inline void debug(WCHAR* str) {
	wprintf(str);
}

inline void debugln(WCHAR* str) {
	debug(str);
	debug(L"\n");
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap's color format, width, and height. 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) {
        debug(L"GetObject"); 
	}

    // Convert the color format to a count of bits. 
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure 
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD 
    // data structures.) 

     if (cClrBits != 24) 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER) + 
                    sizeof(RGBQUAD) * (1<< cClrBits)); 

     // There is no RGBQUAD array for the 24-bit-per-pixel format. 

     else 
         pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
                    sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure. 

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag. 
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color 
    // indices and store the result in biSizeImage. 
    // For Windows NT/2000, the width must be DWORD aligned unless 
    // the bitmap is RLE compressed. This example shows this. 
    // For Windows 95/98, the width must be WORD aligned unless the 
    // bitmap is RLE compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                  * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the 
    // device colors are important. 
     pbmi->bmiHeader.biClrImportant = 0; 
     return pbmi; 
 } 

void CreateBMPFile(WCHAR* pszFile, PBITMAPINFO pbi, 
                  HBITMAP hBMP, HDC hDC) { 
    HANDLE hf;                 // file handle 
    BITMAPFILEHEADER hdr;       // bitmap file-header 
    PBITMAPINFOHEADER pbih;     // bitmap info-header 
    LPBYTE lpBits;              // memory pointer 
    DWORD dwTotal;              // total count of bytes 
    DWORD cb;                   // incremental count of bytes 
    BYTE *hp;                   // byte pointer 
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) {
         debug(L"GlobalAlloc"); 
	}

    // Retrieve the color table (RGBQUAD array) and the bits 
    // (array of palette indices) from the DIB. 
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
        DIB_RGB_COLORS)) 
    {
        debug(L"GetDIBits"); 
    }

    // Fix this if GetDIBits messed it up....
	pbi->bmiHeader.biClrUsed = (pbi->bmiHeader.biBitCount <= 8) ? 1<<pbi->bmiHeader.biBitCount : 0;

	// Create the .BMP file. 
    hf = CreateFileW(pszFile, 
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) 0, 
                    NULL, 
                   CREATE_ALWAYS, 
                   FILE_ATTRIBUTE_NORMAL, 
                   (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        debug(L"CreateFile"); 
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
    // Compute the size of the entire file. 
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 pbih->biSize + pbih->biClrUsed 
                 * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices. 
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    pbih->biSize + pbih->biClrUsed 
                    * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file. 
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
       debug(L"WriteFile BITMAPFILEHEADER"); 
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file. 
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
                  + pbih->biClrUsed * sizeof (RGBQUAD), 
                  (LPDWORD) &dwTmp, ( NULL))) 
        debug(L"WriteFile BITMAPINFOHEADER and RGBQUAD"); 

    // Copy the array of color indices into the .BMP file. 
    dwTotal = cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
           debug(L"WriteFile array of color indices"); 

    // Close the .BMP file. 
     if (!CloseHandle(hf)) 
           debug(L"CloseHandle"); 

    // Free memory. 
    GlobalFree((HGLOBAL)lpBits);
}

BOOL LoadBitmapFromBMPFile( WCHAR* szFileName, HBITMAP *phBitmap,
   HPALETTE *phPalette ) {

   BITMAP  bm;

   *phBitmap = NULL;
   *phPalette = NULL;

   // Use LoadImage() to get the image loaded into a DIBSection
   *phBitmap = (HBITMAP)LoadImageW( NULL, szFileName, IMAGE_BITMAP, 0, 0,
               LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
   if( *phBitmap == NULL )
     return FALSE;

   // Get the color depth of the DIBSection
   GetObject(*phBitmap, sizeof(BITMAP), &bm );
   // If the DIBSection is 256 color or less, it has a color table
   if( ( bm.bmBitsPixel * bm.bmPlanes ) <= 8 )
   {
	   HDC           hMemDC;
	   HBITMAP       hOldBitmap;
	   RGBQUAD       rgb[256];
	   LPLOGPALETTE  pLogPal;
	   WORD          i;

	   // Create a memory DC and select the DIBSection into it
	   hMemDC = CreateCompatibleDC( NULL );
	   hOldBitmap = (HBITMAP)SelectObject( hMemDC, *phBitmap );
	   // Get the DIBSection's color table
	   GetDIBColorTable( hMemDC, 0, 256, rgb );
	   // Create a palette from the color tabl
	   pLogPal = (LOGPALETTE *)malloc( sizeof(LOGPALETTE) + (256*sizeof(PALETTEENTRY)) );
	   pLogPal->palVersion = 0x300;
	   pLogPal->palNumEntries = 256;
	   for(i=0;i<256;i++)
	   {
		 pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
		 pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
		 pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
		 pLogPal->palPalEntry[i].peFlags = 0;
	   }
	   *phPalette = CreatePalette( pLogPal );
	   // Clean up
	   free( pLogPal );
	   SelectObject( hMemDC, hOldBitmap );
	   DeleteDC( hMemDC );
   }
   else   // It has no color table, so use a halftone palette
   {
	   HDC    hRefDC;

	   hRefDC = GetDC( NULL );
	   *phPalette = CreateHalftonePalette( hRefDC );
	   ReleaseDC( NULL, hRefDC );
   }
   return TRUE;
} 

int WINAPI WinMain(	HINSTANCE hInstance,      // handle to current instance
					HINSTANCE hPrevInstance,  // handle to previous instance
					LPSTR lpCmdLine,          // command line
					int nCmdShow) {              // show state   
	int	argc = 0;
	int	textX = 0;
	int	textY = 0;
    WCHAR*	text = NULL;
	WCHAR*	fontName = L"Arial";
	WCHAR*	inputFile = NULL;
	WCHAR*	outputFile = NULL;
	int		fontSize = 10;
	int		fontStyle = 0;
	BOOL	isItalic = false;
	int		fontWeight = 100;
	BOOL	stop = FALSE;

	CreateConsole();

	LPWSTR* argv = CommandLineToArgvW( GetCommandLineW(),&argc);

	if(argc < 3) {
        debug(L"Usage: imagewriter <input image file> <output image file> [-x <x coordinate>] [-y <y coordinate>] [-text <text to write>] [-font <font name>] [-size <font size>]\n");
		return 1;
	}

	inputFile = (WCHAR*)argv[1];
	outputFile = (WCHAR*)argv[2];

	for(int i=3;i<argc;i++) {
		if(!_wcsicmp((WCHAR*)argv[i],L"-x")) {
			WCHAR*	endScan = 0;
			textX = wcstol((WCHAR*)argv[++i],&endScan,10);
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-weight")) {
			WCHAR*	endScan = 0;
			fontWeight = wcstol((WCHAR*)argv[++i],&endScan,10);
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-italic")) {
			isItalic = TRUE;
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-stop")) {
			stop = TRUE;
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-y")) {
			WCHAR*	endScan = 0;
			textY = wcstol((WCHAR*)argv[++i],&endScan,10);
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-text")) {
			text = (WCHAR*)argv[++i];
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-font")) {
			fontName = (WCHAR*)argv[++i];
			continue;
		}
		if(!_wcsicmp((WCHAR*)argv[i],L"-size")) {
			WCHAR*	endScan = 0;
			fontSize = wcstol((WCHAR*)argv[++i],&endScan,10);
			continue;
		}
		debug(L"Unknown: ");
	    debug((WCHAR*)argv[i]);
		debug(L"\n");
	}


	if(text == NULL) {
		debug(L"Nothing to draw\n");
		return 1;
	}

	debugln(L"Converting...");
	debug(L"   Input image file: \"");
	debug(inputFile);
	debugln(L"\"");
	debug(L"   Output image file: \"");
	debug(outputFile);
	debugln(L"\"");
	debug(L"   Text: \"");
	debug(text);
	debugln(L"\"");
	debug(L"   Coordinates: X=");
	debug(textX);
	debug(L" Y=");
	debug(textY);
	debug(L"\n");
	debug(L"   Font: \"");
	debug(fontName);
	debug(L"\"");
	debug(L" ");
	if(isItalic) {
		debug(L"Italic");
		debug(L" ");
	}
	debug(L"   size: ");
	debug(fontSize);
	debug(L"   weight: ");
	debug(fontWeight);
	debug(L"\n");


	HBITMAP       hBitmap;
	HPALETTE      hPalette;
	HDC           hDC, hMemDC;
	BITMAP        bm;

	hDC = GetDC(NULL);

	if(LoadBitmapFromBMPFile( inputFile , &hBitmap, &hPalette ) ){
		GetObject( hBitmap, sizeof(BITMAP), &bm );
		hMemDC = CreateCompatibleDC( hDC );
		SelectObject( hMemDC, hBitmap );
		SelectPalette( hDC, hPalette, FALSE );
		RealizePalette( hDC );

		HFONT hFont = CreateFontW(fontSize,0,0,0,fontWeight,isItalic,FALSE,FALSE,
				DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY,DEFAULT_PITCH,fontName);


		SelectObject( hMemDC, hFont );
		TextOutW(hMemDC,textX,textY,text,wcslen(text));

		CreateBMPFile(outputFile, CreateBitmapInfoStruct(hBitmap),hBitmap,hMemDC);
	
		DeleteObject( hFont );
		DeleteObject( hBitmap );
		DeleteObject( hPalette );
		DeleteDC(hMemDC);
	}
	DeleteDC(hDC);
	GlobalFree(argv);

	if(stop) {
		debug(L"Press any key...\n");
		getch();
	}
	return 0;	
}

