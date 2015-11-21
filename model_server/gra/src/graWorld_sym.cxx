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
// include files

#include "Object.h"

#include <fcntl.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#endif /* ISO_CPP_HEADERS */
#ifndef hp10
#include <sysent.h>
#endif
#include <general.h>
#include <msg.h>

#include <InterViews/enter-scope.h>
#include <InterViews/world.h>
#include <InterViews/event.h>
#include <InterViews/transformer.h>
#include <InterViews/rubcurve.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/ppaint.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>

#include <genError.h>
#include <genString.h>
#include <systemMessages.h>
#include <customize.h>
#include <top_widgets.h>
#include <gtBase.h>

#include <graError.h>
#include <graEnums.h>
#include <graWorld.h>
#include <graSymbol.h>
#include <graTools.h>

#include <cmd.h>

#include <machdep.h>
#include <driver.h>

#ifdef sun4
extern "C" {
#include <sys/ioccom.h>
#include <sun/audioio.h>
}
#endif

//-----------------------------------------------------------------------------

PColor* lookup_PColor(const char*);	// From graSymbol.h.C

static const int GRA_SYMBOL_ARRAY_SIZE = 163;

struct graSymbolData
{
    graAttachableSymbol* pictel[GRA_MAX_MODES];
    iv3(Rubberband)* rubband;
    short text_mode;
};

//------------------------------------------
// load_coords
//
// Read in an array of coordinates.
//------------------------------------------

static void load_coords(ifstream& infile, int size,
			int& num, iv2(Coord) *x, iv2(Coord) *y)
{
    Initialize(load_coords);

    char buffer[200];
    int load_x = 1;

    num = 0;
    while(infile >> buffer, strcmp(buffer,"$"))
    {
	if(num < size)
	{
	    if(load_x)
		*x++ = atoi(buffer);
	    else
	    {
		*y++ = atoi(buffer);
		num++;
	    }
	    load_x = !load_x;
	}
    }
}

//------------------------------------------
// load_symbols
//
// This member function is invoked from the graWorld constructor. Its purpose 
// is to temporarily read a set of symbols from an ascii file and to provide 
// these parametric symbols for system use. This function therefore was a 
// quick hack to quickly get these symbols in and is intended to be replaced 
// by true parametric symbols which will be able to be edited by a graphical 
// symbol editor.
//
// There is no guarantee what will happen if the symbol file is not formatted
// correctly
//------------------------------------------

static const int COORD_SIZE = 50;
static iv3(Painter)* drag_shape_painter;

void graWorld::load_symbols()
{
    Initialize (graWorld::load_symbols);

    char buffer[200];
    int intarg[100];
    int numargs;
    iv2(Coord) x[COORD_SIZE];
    iv2(Coord) y[COORD_SIZE];
    PColor *bg = pwhite;
    PColor *fg = pblack;
    PBrush *brush = new PBrush (0);
    PFont *font = new PFont("9x15");
    PFont* icon_font = new PFont("aseticons-20");
    int pattern = 0;
    Picture *picture;
    Graphic *graphic;
    int index;
    graLayerNo pictel_idx;
    int translation_seen;
    int scale_seen;
    float tx = 0.0, ty = 0.0;
    float scale = 1.0;
    int i;

#ifndef __GNUG__
    delete [] graSymbolArray;
#else
    delete [GRA_SYMBOL_ARRAY_SIZE] graSymbolArray;
#endif
    graSymbolArray = new graSymbolData[GRA_SYMBOL_ARRAY_SIZE];
    OS_dependent::bzero (graSymbolArray, GRA_SYMBOL_ARRAY_SIZE * sizeof(graSymbolData));
    num_gra_symbol_types = 0;

// Find the symbol file pointed to by the symbol file environment variable

    genString file = customize::getStrPref("DIS_graphics.Symbols_DataFile");
    if (!file.length()){
        msg("ERROR: Can Not Find symbols.dat File.") << eom;
        throw_error(GRAERR_NO_SYMBOL_VAR); 
    }
// Open the symbol file

    ifstream infile;
    infile.open (file, ios::in);
    if ( infile == NULL ){
//	Error (GRAERR_NO_SYMBOL_FILE);
//      Exit with message

        msg("ERROR: Can Not Open symbol.dat File.") << eom;
        throw_error(GRAERR_NO_SYMBOL_FILE);
    }

// Read symbols from the file until there are no more

    for ( index = 0, infile >> buffer; 
	  !infile.eof (); index++, infile >> buffer ) {

// Set default text_mode.

        graSymbolArray[index].text_mode = graTextModeRight;

// Flag to remember if the translate/scale keyword was seen

        translation_seen = 0;
        scale_seen = 0;

// Get the next keyword. 
        for ( infile >> buffer; !infile.eof (); infile >> buffer ) {

// keyword <$>  - means we have reached end of the current symbol definition

	    if ( strcmp (buffer, "$") == 0 )
		break;

// keyword <drag_shape> - a series of connected lines used to represent
// the symbol when it is being dragged around.

	    else if(strcmp(buffer, "drag_shape") == 0) {
		if(drag_shape_painter == NULL)
		{
		    drag_shape_painter = new iv3(Painter);
		    drag_shape_painter->ref();
		}
		load_coords(infile, COORD_SIZE, numargs, x, y);
		graSymbolArray[index].rubband =
		    new iv3(SlidingLineList)(0, 0, x, y, numargs, 0, 0);
		graSymbolArray[index].rubband->ref();
		graSymbolArray[index].rubband->SetPainter(drag_shape_painter);
	    }

// keyword <base_info> - common graphics for all modes of the symbol. Set up 
// a new picture and make it the current picture to fill with graphics.

	    else if ( strcmp (buffer, "base_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoBase] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoBase]->Append (picture);
                pictel_idx = graLayerNoBase;
	    }

// keyword <expanded_info> - common graphics for when symbol has visible 
// children. Set up a new picture and make it the current picture to fill 
// with graphics.

	    else if ( strcmp (buffer, "expanded_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoExpanded] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoExpanded]->
                        Append (picture);
                pictel_idx = graLayerNoExpanded;
	    }

// keyword <collapsed_info> - common graphics for when symbol has invisible 
// children. Set up a new picture and make it the current picture to fill 
// with graphics.

	    else if ( strcmp (buffer, "collapsed_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoCollapsed] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoCollapsed]->
                        Append (picture);
                pictel_idx = graLayerNoCollapsed;
	    }

// keyword <note_info> - common graphics for when symbol has a note attached
// to it. Set up a new picture and make it the current picture to fill 
// with graphics.

	    else if ( strcmp (buffer, "note_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoNote] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoNote]->
                        Append (picture);
                pictel_idx = graLayerNoNote;
	    }

// keyword <hyper_info> - common graphics for when symbol has a hyperlink
// attached to it. Set up a new picture and make it the current picture to
// fill with graphics.

	    else if ( strcmp (buffer, "hyper_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoHyperMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoHyperMarker]->
                        Append (picture);
                pictel_idx = graLayerNoHyperMarker;
	    }

// keyword <normal_info> - common graphics for when symbol is not hilighted. 
// Set up a new picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "normal_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoNormal] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoNormal]->
                        Append (picture);
                pictel_idx = graLayerNoNormal;
	    }

// keyword <hilited_info> - common graphics for when a symbol is hilited. 
// This mode cannot be used at the same time as <normal>. Set up a new picture 
// and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "hilited_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoHilited] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoHilited]->
                        Append (picture);
                pictel_idx = graLayerNoHilited;
	    }

// keyword <selected_info> - common graphics for when a symbol is selected.
// Set up a new picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "selected_info") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoSelected] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoSelected]->
                        Append (picture);
                pictel_idx = graLayerNoSelected;
	    }

// keyword <lower_left_decoration> - common graphics for ornamentation
// applied to the lower left part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "lower_left_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationLL] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationLL]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationLL;
	    }

// keyword <center_left_decoration> - common graphics for ornamentation
// applied to the center left part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "center_left_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationCL] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationCL]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationCL;
	    }

// keyword <upper_left_decoration> - common graphics for ornamentation
// applied to the upper left part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "upper_left_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationUL] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationUL]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationUL;
	    }

// keyword <lower_center_decoration> - common graphics for ornamentation
// applied to the lower center part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "lower_center_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationLC] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationLC]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationLC;
	    }

// keyword <upper_center_decoration> - common graphics for ornamentation
// applied to the lower left part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "upper_center_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationUC] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationUC]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationUC;
	    }

// keyword <lower_right_decoration> - common graphics for ornamentation
// applied to the lower right part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "lower_right_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationLR] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationLR]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationLR;
	    }

// keyword <center_right_decoration> - common graphics for ornamentation
// applied to the center right part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "center_right_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationCR] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationCR]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationCR;
	    }

// keyword <upper_right_decoration> - common graphics for ornamentation
// applied to the upper right part of the symbol.  Set up a new picture and
// make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "upper_right_decoration") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDecorationUR] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDecorationUR]->
                        Append (picture);
                pictel_idx = graLayerNoDecorationUR;
	    }

// keyword <hard_association> - common graphics for ornamentation applied to
// the symbol when it has a hard association applied to it.  Set up a new 
// picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "hard_association") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoHardAssocMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoHardAssocMarker]->
                        Append (picture);
                pictel_idx = graLayerNoHardAssocMarker;
	    }

// keyword <breakpoint> - common graphics for ornamentation applied to
// the symbol when it has a breakpoint attached to it.  Set up a new 
// picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "breakpoint") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoBreakMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoBreakMarker]->
                        Append (picture);
                pictel_idx = graLayerNoBreakMarker;
	    }

// keyword <disabled_breakpoint> - common graphics for ornamentation
// applied to the symbol when it has a breakpoint attached to it which
// is disabled. Set up a new picture and make it the current picture to
// fill with graphics

	    else if ( strcmp (buffer, "disabled_breakpoint") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoDisabledBreakMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoDisabledBreakMarker]->
                        Append (picture);
                pictel_idx = graLayerNoDisabledBreakMarker;
	    }

// keyword <execution_pt> - common graphics for ornamentation applied to
// the symbol when it has an execution point attached to it.  Set up a new 
// picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "execution_pt") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoExecutionMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoExecutionMarker]->
                        Append (picture);
                pictel_idx = graLayerNoExecutionMarker;
	    }

// keyword <error_mark> - common graphics for ornamentation applied to
// the symbol when it is associated with a parser error.  Set up a new 
// picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "error_mark") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoErrorMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoErrorMarker]->
                        Append (picture);
                pictel_idx = graLayerNoErrorMarker;
	    }

// keyword <search_mark> - common graphics for ornamentation applied to
// the symbol when the user searches for an item associated with it.  Set
// up a new picture and make it the current picture to fill with graphics

	    else if ( strcmp (buffer, "search_mark") == 0 ) {
                picture = new Picture;
                graSymbolArray[index].pictel[graLayerNoSearchMarker] =
                        new graAttachableSymbol;
                graSymbolArray[index].pictel[graLayerNoSearchMarker]->
                        Append (picture);
                pictel_idx = graLayerNoSearchMarker;
	    }

// keyword <right_half> -- separates symbol with <inside> text into left
// and right halves, connected by extensible lines.  Set up a new picture,
// make it the current picture to fill with graphics, and append it to
// the current pictel.  Create an empty picture as the new pictures first
// child whose colors and brush are set so the extensible section can be
// filled with the correct color (implication: the symbols.dat file must
// set the fg/bg colors and brush for filling *before* the <right_half>
// keyword).  The convention is that the bg color is used for the filled
// box containing the label and the fg color is used for the extension
// lines on the top and bottom.

            else if (strcmp(buffer, "right_half") == 0) {
                picture = new Picture;
                Picture* pp = new Picture;
                pp->SetColors(fg, bg);
                pp->SetBrush(brush);
                picture->Append(pp);
                graSymbolArray[index].pictel[pictel_idx]->Append(picture);
            }

// keyword <translate> - if this keyword is seen, the origin of the symbol must
// be translated. This makes the <true lower left corner> of the symbol the 
// origin.

	    else if ( strcmp (buffer, "translate") == 0 ) {
	        infile >> buffer; intarg[0] = atoi (buffer);
	        infile >> buffer; intarg[1] = atoi (buffer);
	        tx = intarg[0];
	        ty = intarg[1];
	 	translation_seen = 1;
	    }

// keyword <scale> - if this keyword is seen, the symbol must
// be scaled. 

	    else if ( strcmp (buffer, "scale") == 0 ) {
	        infile >> buffer; scale = atof (buffer);
	 	scale_seen = 1;
	    }

// keyword <brush> - sets the brush pattern and width for stroking lines

	    else if ( strcmp (buffer, "brush") == 0 ) {
		infile >> buffer; intarg[0] = atoi(buffer);
                infile >> buffer; intarg[1] = atoi(buffer);
		brush = new PBrush (intarg[0], intarg[1]);
	    }

// keyword <fg> - sets the current foreground color

	    else if ( strcmp (buffer, "fg") == 0 ) {
		infile >> buffer;
		fg = lookup_PColor(buffer);
	    }

// keyword <bg> - sets the current background color

	    else if ( strcmp (buffer, "bg") == 0 ) {
		infile >> buffer;
		bg = lookup_PColor(buffer);
	    }
 
// keyword <pattern> - sets the current fill pattern to a short integer which 
// represents a 4x4 patternS

	    else if ( strcmp (buffer, "pattern") == 0 ) {
		infile >> buffer;
		pattern = (int)strtol (buffer, NULL, 16);
	    }


// keyword <text_mode> - sets the current mode for positioning the only 
// parameter currently defined (one line of text)

	    else if ( strcmp (buffer, "text_mode") == 0 ) {
		infile >> buffer;
		if ( strcmp (buffer, "positioned") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModePositioned;
		} else if ( strcmp (buffer, "top") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeTop;
		} else if ( strcmp (buffer, "bottom") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeBottom;
		} else if ( strcmp (buffer, "left") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeLeft;
		} else if ( strcmp (buffer, "right") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeRight;
		} else if ( strcmp (buffer, "inside") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeInside;
		} else if ( strcmp (buffer, "suppressed") == 0 ) {
                    graSymbolArray[index].text_mode = graTextModeSuppressed;
                } else if ( strcmp (buffer, "inside_no_box") == 0 ) {
		    graSymbolArray[index].text_mode = graTextModeInsideNoBox;
                }

// keyword <text> - draws some text into the symbol

	    } else if ( strcmp (buffer, "text") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer;

		graphic = new iv3(Label) (buffer);
	 	graphic->Translate (intarg[0], intarg[1]);
		graphic->SetColors (fg, bg);
		graphic->SetFont (font);
		picture->FillBg (FILLBG_NO);

		picture->Append (graphic);
 	    }

// keyword <icon> - draws an icon into the symbol

	    else if (strcmp(buffer, "icon") == 0) {
	       infile >> buffer; intarg[0] = atoi(buffer);
	       infile >> buffer; intarg[1] = atoi(buffer);
	       infile >> buffer; intarg[2] = atoi(buffer);
	       buffer[0] = (char)intarg[2];
	       buffer[1] = 0;

	       if (icon_font->Valid()) {
		  graphic = new iv3(Label)(buffer);
		  graphic->Translate(intarg[0], intarg[1]);
		  graphic->SetColors(fg, bg);
		  graphic->SetFont(icon_font);
		  picture->FillBg(FILLBG_NO);

		  picture->Append(graphic);
	       }
	    }

// keyword <poly> - draws a polygon outline into the symbol

	    else if ( strcmp (buffer, "poly") == 0 ) {
		load_coords(infile, COORD_SIZE, numargs, x, y);
		graphic = new Polygon (x, y, numargs);
		graphic->SetBrush (brush);
		graphic->SetColors (fg, bg);

		picture->Append (graphic);
	    }

// keyword <multiline> - draws a multiple line figure into the symbol

	    else if ( strcmp (buffer, "multiline") == 0 ) {
		load_coords(infile, COORD_SIZE, numargs, x, y);
		graphic = new MultiLine (x, y, numargs);
		graphic->SetBrush (brush);
		graphic->SetColors (fg, bg);

		picture->Append (graphic);
	    }

// keyword <fillpoly> - draws a filled polygon into the symbol

	    else if ( strcmp (buffer, "fillpoly") == 0 ) {
		load_coords(infile, COORD_SIZE, numargs, x, y);
		graphic = new FillPolygon (x, y, numargs);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);
 		if ( pattern != -1 ) {
		    graphic->SetPattern (new PPattern (pattern));
		    graphic->FillBg (FILLBG_YES);
		    pattern = -1;
		}

		picture->Append (graphic);

// The following code addresses an <off-by-one> bug in filled polygons by
// making sure the boundary line is drawn.

                graphic = new Polygon(x, y, numargs);
                graphic->SetColors(fg, bg);
                graphic->SetBrush(brush);
                picture->Append(graphic);
	    }

// keyword <point> - draws a point into the symbol

	    else if ( strcmp (buffer, "point") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);

		graphic = new Point ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1]);
		graphic->SetColors (fg, bg);

		picture->Append (graphic);
	    }

// keyword <line> - draws a line into the symbol

	    else if ( strcmp (buffer, "line") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);
		infile >> buffer; intarg[3] = atoi (buffer);

		graphic = new Line ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1], 
			            (iv2(Coord))intarg[2], (iv2(Coord))intarg[3]);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);

		picture->Append (graphic);
	    }

// keyword <rect> - draws a rectangle outline into the symbol

	    else if ( strcmp (buffer, "rect") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);
		infile >> buffer; intarg[3] = atoi (buffer);

		graphic = new Rect ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1], 
				    (iv2(Coord))intarg[2], (iv2(Coord))intarg[3]);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);

		picture->Append (graphic);
	    }

// keyword <fillrect> - draws a filled rectangle into the symbol

	    else if ( strcmp (buffer, "fillrect") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);
		infile >> buffer; intarg[3] = atoi (buffer);

		graphic = new FillRect ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1], 
					(iv2(Coord))intarg[2], (iv2(Coord))intarg[3]);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);
 		if ( pattern != -1 ) {
		    graphic->SetPattern (new PPattern (pattern));
		    graphic->FillBg (FILLBG_YES);
		    pattern = -1;
		}

		picture->Append (graphic);
	    }

// keyword <circle> - draws a circle outline into the symbol

	    else if ( strcmp (buffer, "circle") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);

		graphic = new Circle ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1], 
				      (iv2(Coord))intarg[2]);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);

		picture->Append (graphic);
	    }

// keyword <fillcircle> - draws a filled circle into the symbol

	    else if ( strcmp (buffer, "fillcircle") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);

		graphic = new FillCircle ((iv2(Coord))intarg[0], (iv2(Coord))intarg[1], 
				          (iv2(Coord))intarg[2]);
		graphic->SetColors (fg, bg);
 		if ( pattern != -1 ) {
		    graphic->SetPattern (new PPattern (pattern));
		    graphic->FillBg (FILLBG_YES);
		    pattern = -1;
		}
		graphic->SetBrush (brush);

		picture->Append (graphic);
	    }

// keyword <ellipse> - draws an ellipse outline into the symbol

	    else if ( strcmp (buffer, "ellipse") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);
		infile >> buffer; intarg[3] = atoi (buffer);

		graphic = new Ellipse ((iv2(Coord))intarg[0],
                              (iv2(Coord))intarg[1], (iv2(Coord))intarg[2],
                              (iv2(Coord))intarg[3]);
		graphic->SetColors (fg, bg);
		graphic->SetBrush (brush);
		picture->Append (graphic);
	    }

// keyword <fillellipse> - draws a filled ellipse into the symbol

	    else if ( strcmp (buffer, "fillellipse") == 0 ) {
		infile >> buffer; intarg[0] = atoi (buffer);
		infile >> buffer; intarg[1] = atoi (buffer);
		infile >> buffer; intarg[2] = atoi (buffer);
		infile >> buffer; intarg[3] = atoi (buffer);

		graphic = new FillEllipse ((iv2(Coord))intarg[0],
                              (iv2(Coord))intarg[1], (iv2(Coord))intarg[2],
                              (iv2(Coord))intarg[3]);     
		graphic->SetColors (fg, bg);
 		if ( pattern != -1 ) {
		    graphic->SetPattern (new PPattern (pattern));
		    graphic->FillBg (FILLBG_YES);
		    pattern = -1;
		}
		graphic->SetBrush (brush);
		picture->Append (graphic);
	    }

// keyword <bspline> -- draws a B-spline into the symbol (added primarily
// to offset the lack of an <arc> shape in the InterViews repertoire)

            else if (strcmp(buffer, "bspline") == 0) {
		load_coords(infile, COORD_SIZE, numargs, x, y);
                graphic = new BSpline(x, y, numargs);
                graphic->SetBrush(brush);
                graphic->SetColors(fg, bg);

                picture->Append(graphic);
            }

// keyword <fillbspline> -- draws a filled B-spline into the symbol

            else if (strcmp(buffer, "fillbspline") == 0) {
		load_coords(infile, COORD_SIZE, numargs, x, y);
                graphic = new FillBSpline(x, y, numargs);
                graphic->SetBrush(brush);
                graphic->SetColors(fg, bg);

                picture->Append(graphic);
            }

// keyword <attach> -- specifies the coordinates at which a given attachment
// should be made

            else if (strcmp(buffer, "attach") == 0) {
                graAttachmentPoint pt = graAttachLL;
                infile >> buffer;
                if (strcmp(buffer, "ll") == 0)
                    pt = graAttachLL;
                else if (strcmp(buffer, "cl") == 0)
                    pt = graAttachCL;
                else if (strcmp(buffer, "ul") == 0)
                    pt = graAttachUL;
                else if (strcmp(buffer, "lc") == 0)
                    pt = graAttachLC;
                else if (strcmp(buffer, "uc") == 0)
                    pt = graAttachUC;
                else if (strcmp(buffer, "lr") == 0)
                    pt = graAttachLR;
                else if (strcmp(buffer, "cr") == 0)
                    pt = graAttachCR;
                else if (strcmp(buffer, "ur") == 0)
                    pt = graAttachUR;
                infile >> buffer;
                graSymbolArray[index].pictel[pictel_idx]->x[pt] = atoi(buffer);
                infile >> buffer;
                graSymbolArray[index].pictel[pictel_idx]->y[pt] = atoi(buffer);
            }

	}

// The entire symbol has been read - if a translate has been seen, do it now

	if ( translation_seen || scale_seen ) {
	    for ( i=0; i<16; i++ ) {
	        if ( graSymbolArray[index].pictel[i] ) {
		    if (translation_seen)
		        graSymbolArray[index].pictel[i]->Translate (tx, ty);
		    if (scale_seen)
		        graSymbolArray[index].pictel[i]->Scale (scale, scale);
                    for (int j = 0; j < graNumAttachmentPoints; j++) {
                        if (graSymbolArray[index].pictel[i]->x[j] ||
                                graSymbolArray[index].pictel[i]->y[j]) {
			    if (translation_seen) {
                              graSymbolArray[index].pictel[i]->x[j] += (int) tx;
                              graSymbolArray[index].pictel[i]->y[j] += (int) ty;
			    }
			    if (scale_seen) {
                              graSymbolArray[index].pictel[i]->x[j] *= (int)scale;
                              graSymbolArray[index].pictel[i]->y[j] *= (int)scale;
			    }
                        }
                    }
                }
	    }
	}
    }

// Remember the number of symbols read

    num_gra_symbol_types = index;
}

//------------------------------------------
// get_rubband
//
// Given a symbol number, return the Rubband object used to drag it.
//------------------------------------------

iv3(Rubberband) *graWorld::get_rubband( int symbol )
{
    if ( symbol < 0 || symbol >= num_gra_symbol_types )
        symbol = 0;

    return graSymbolArray[symbol].rubband;
}

//------------------------------------------
// get_pictel
//
// Given a symbol number and a layer number, retrieve the associated picture
//------------------------------------------

graAttachableSymbol *graWorld::get_pictel( int symbol, int pic )
{
    // if symbol is -1, symbol should not be created.
    if (symbol == -1)
       return NULL;
    
    if ( symbol < 0 || symbol >= num_gra_symbol_types )
        symbol = 0;

    if ( pic < 0 || pic >= GRA_MAX_MODES )
        pic = 0;

    graAttachableSymbol* return_val = graSymbolArray[symbol].pictel[pic];

    return return_val ? return_val : graSymbolArray[symbol].pictel[0];
}

//------------------------------------------
// get_text_mode
//
// Given a symbol number, get the text mode of the text parameter
//------------------------------------------

int graWorld::get_text_mode( int symbol )
{
    if ( symbol < 0 || symbol >= num_gra_symbol_types )
        symbol = 0;
  
    return graSymbolArray[symbol].text_mode;
}

//------------------------------------------
// get_symbol_size
//
// Given the symbol number, get the symbols center and size
//------------------------------------------

void graWorld::get_symbol_size( int symbol, float &cx, float &cy,
    float &w, float &h )
{
    Picture *picture;
    float llx, lly, urx ,ury;

    picture = this->get_pictel (symbol, 0);

    picture->GetBounds (llx, lly, urx, ury);

    cx = cy = 0.0;

    w = urx;
    h = ury;
}
