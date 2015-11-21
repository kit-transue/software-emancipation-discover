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
#include <Dt.h>
/*
 * 
 *	input parameters:
 * 		font_list[]	: array of font names
 * 				eg: 6x4, 8x7, 9x6 ....
 *		size    	: size of the font_list array
 */
XFontStruct **
DaInitAppFonts( dpy, font_list, size)
    Display *dpy ;
    char **font_list ;
    int size ;
{
    int i ;
    XFontStruct **fontArray ;

    fontArray = (XFontStruct **) malloc ( size * sizeof(XFontStruct) ) ;

    for (i=0;i<size;i++) {
        if ( ( fontArray[i] = XLoadQueryFont(dpy, font_list[i])) == NULL) {
            fontArray[i] = (XFontStruct *) DT_NO_FONT ;
        }
    }
    return(fontArray) ;
}

DaFreeAppFonts(fontArray)
    XFontStruct **fontArray ;
{
    free(fontArray) ;
    return(DT_OK) ;
}

/*
 * Format for 3 significant figures
 */

char *
DaGetFormatString( value)
    float value ;
{
    if ( value >= 100.0) return( "%0.0f") ;
    if ( value >= 10.0)  return( "%0.1f") ;
    if ( value >= 1.0)   return( "%0.2f") ;
    if ( value >= .1)    return( "%0.3f") ;
    return( "%0.4f") ;
}

