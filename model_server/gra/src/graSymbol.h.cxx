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
// graSymbol.h.C
//------------------------------------------
// synopsis:
// Implementation of parametric symbols and connectors
// graAttachableSymbol, graParametricSymbol, graConnectionSymbol
//------------------------------------------
// Restrictions:
// Regeneration of a connector can only handle one input symbol currently.
//------------------------------------------

// INCLUDE FILES

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <strstream.h>
#include <fstream.h>
#include <iomanip.h>
#include <string.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <strstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include <Object.h>
#include <machdep.h>
#include <psetmem.h>

#include <general.h>
#include <generic-popup-menu.h>

// x11_intrinsic.h must be included before InterViews files.
#include <x11_intrinsic.h>

#include <InterViews/enter-scope.h>
#include <InterViews/textbuffer.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/instance.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/ellipses.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/splines.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>

#include <rubberGraphic.h>

#include <viewNode.h>
#include <objArr.h>
#include <ldrTypes.h>
#include <driver.h>
#include <graResources.h>
#include <graSymbol.h>
#include <graWorld.h>
#include <graWindow.h>
#include <graTools.h>
#include <viewERDClassNode.h>
#include <viewGraHeader.h>
#include <viewRouter.h>
#include <smt.h>
#include <gra_ivInterface.h>


// TYPE DEFINITIONS

enum arrow_type
{
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_LEFT_LONG,
    ARROW_UP
};


struct _hcoords		// Structure used to sort line segment information
{			// for creating connector symbols.
    float x, y;
    viewSymbolNodePtr viewnode;
};


// VARIABLE DEFINITIONS

// Text cursor
static iv2(Coord) cursor_x[] = { -2,  2,  1,  1,  2,  -2, -1, -1, -2 };
static iv2(Coord) cursor_y[] = { -2, -2,  0, 10, 12,  12, 10,  0, -2 };

// Default font to use if X resource manager comes up NULL.
static const char DFLT_FONT[] = "8x13";

// Default colors to use if X resource manager comes up NULL.
static const char DFLT_BACK[] = "white";
static const char DFLT_FORE[] = "black";

// Call init_PBrush() before referencing the following variables.
static PBrush* pbrush0;
static PBrush* pbrush1;
static PBrush* pbrush3;
static PBrush* pbrush5;

// Call init_PFont() before referencing the following variables.
static PFont* base_font;
static PFont* collapse_font;
static PFont* flow_label_text_font;
static PFont* hilite_font;
static PFont* hilite_text_font;
static PFont* select_font;

// Call init_PColor() before referencing the following variables.
static PColor* base_background;
static PColor* base_foreground;
static PColor* collapse_background;
static PColor* collapse_foreground;
static PColor* connection_background;
static PColor* connection_foreground;
static PColor* cursor_background;
static PColor* cursor_foreground;
static PColor* hilite_background;
static PColor* hilite_foreground;
static PColor* hilite_text_background;
static PColor* hilite_text_foreground;
static PColor* hilite_connection_foreground;
static PColor* select_background;
static PColor* select_foreground;


// FUNCTION DEFINITIONS

const char* GET_VALUE(const char* name, const char* dflt)
{
    const char* value = ui_get_value(DRAWING_WINDOW_CLASS, name);
    return (value && *value) ? value : dflt;
}


double GET_VALUE(const char* name, double dflt)
{
    const char* value = ui_get_value(DRAWING_WINDOW_CLASS, name);
    return (value && *value) ? atof(value) : dflt;
}


static void init_PFont()
{
    Initialize(init_PFont);

    if (base_font == NULL)
    {
        base_font =
	    new PFont(GET_VALUE(BASE_FONT, DFLT_FONT));
        collapse_font =
	    new PFont(GET_VALUE(COLLAPSE_FONT, DFLT_FONT));
        flow_label_text_font =
	    new PFont(GET_VALUE(FLOW_LABEL_TEXT, DFLT_FONT));
        hilite_font =
	    new PFont(GET_VALUE(HILITE_FONT, DFLT_FONT));
        hilite_text_font =
	    new PFont(GET_VALUE(HILITE_TEXT_FONT, DFLT_FONT));
        select_font =
	    new PFont(GET_VALUE(SELECT_FONT, DFLT_FONT));
    }
}


static void init_PColor()
{
    Initialize(init_PColor);

    if (base_background == NULL)
    {
        base_background =
	    new PColor(GET_VALUE(BASE_BACKGROUND, DFLT_BACK));
        base_foreground =
	    new PColor(GET_VALUE(BASE_FOREGROUND, DFLT_FORE));
        collapse_background =
	    new PColor(GET_VALUE(COLLAPSE_BACKGROUND, DFLT_BACK));
        collapse_foreground =
	    new PColor(GET_VALUE(COLLAPSE_FOREGROUND, DFLT_FORE));
        connection_background =
	    new PColor(GET_VALUE(CONNECTION_BACKGROUND, DFLT_BACK));
        connection_foreground =
	    new PColor(GET_VALUE(CONNECTION_FOREGROUND, DFLT_FORE));
        cursor_background =
	    new PColor(GET_VALUE(CURSOR_BACKGROUND, DFLT_BACK));
        cursor_foreground =
	    new PColor(GET_VALUE(CURSOR_FOREGROUND, DFLT_FORE));
        hilite_background =
	    new PColor(GET_VALUE(HILITE_BACKGROUND, DFLT_BACK));
        hilite_foreground =
	    new PColor(GET_VALUE(HILITE_FOREGROUND, DFLT_FORE));
        hilite_text_background =
	    new PColor(GET_VALUE(HILITE_TEXT_BACKGROUND, DFLT_BACK));
        hilite_text_foreground =
	    new PColor(GET_VALUE(HILITE_TEXT_FOREGROUND, DFLT_FORE));
        hilite_connection_foreground =
	    new PColor(GET_VALUE(HILITE_CONNECTION_FOREGROUND, DFLT_FORE));
        select_background =
	    new PColor(GET_VALUE(SELECT_BACKGROUND, DFLT_BACK));
        select_foreground =
	    new PColor(GET_VALUE(SELECT_FOREGROUND, DFLT_FORE));
    }
}


static struct name2pcolor
{
    const char* name;
    PColor* pcolor;
}
*named_pcolors;

static int named_pcolors_size;
static const int INIT_NAMED_PCOLORS_SIZE = 32; // Size to allocate initially;

PColor* lookup_PColor(const char* name)
{
    if(named_pcolors)
    {
	name2pcolor* n2p = named_pcolors;
	for(int i = named_pcolors_size; i > 0; ++n2p, --i)
	    if(!strcmp(n2p->name, name))
		return n2p->pcolor;

	named_pcolors_size += 1;
	named_pcolors = (name2pcolor*)psetrealloc((char *)named_pcolors,
            named_pcolors_size * sizeof(name2pcolor));
    }
    else
    {
	named_pcolors = (name2pcolor*)psetmalloc(
	    INIT_NAMED_PCOLORS_SIZE * sizeof(name2pcolor));
	named_pcolors_size = 1;
    }
    name2pcolor* new_n2p = named_pcolors + named_pcolors_size - 1;
    new_n2p->name = strdup(name);
    new_n2p->pcolor = new PColor(name);

    return new_n2p->pcolor;
}


//------------------------------------------
// init_PBrush
//
// Initialize frequently used PBrush instances.
//------------------------------------------

static void init_PBrush()
{
    if(pbrush0 == NULL)
    {
	pbrush0 = new PBrush(-1);
	pbrush1 = new PBrush(-1, 1);
	pbrush3 = new PBrush(-1, 3);
	pbrush5 = new PBrush(-1, 5);
    }
}

//------------------------------------------
// delete_picture_contents
//
// Remove and delete all immediate children of a Picture.
//------------------------------------------

static void delete_picture_contents(Picture* pic)
{
    if(pic)
    {
	Graphic* g;
	while(g = pic->First())
	{
	    pic->Remove(g);
	    delete g;
	}
    }
}


//------------------------------------------
// Constructor graAttachableSymbol
//------------------------------------------

graAttachableSymbol::graAttachableSymbol()
{
    clear_attachment_points();
}

//------------------------------------------
// Destructor graAttachableSymbol
//------------------------------------------

graAttachableSymbol::~graAttachableSymbol()
{}


//------------------------------------------
// graAttachableSymbol::get_attachment_point()
//
// Returns the coordinates at which the specified attachment should be
// made.
//------------------------------------------

void graAttachableSymbol::get_attachment_point(
    graAttachmentPoint pt, int& outx, int& outy)
{
    Initialize(graAttachableSymbol::get_attachment_point);

    outx = x[pt];
    outy = y[pt];
}

//------------------------------------------
// graAttachableSymbol::clear_attachment_points()
//------------------------------------------

void graAttachableSymbol::clear_attachment_points()
{
    Initialize(graAttachableSymbol::clear_attachment_points);

    for(int i = 0; i < graNumAttachmentPoints; i++)
        x[i] = y[i] = 0;
}

//------------------------------------------
// Constructor graParametricSymbol
//
// This constructor creates a new parametric symbol. These symbols
// are inserted into a graWorld to be visualized and reacted with.
//------------------------------------------

graParametricSymbol::graParametricSymbol(int symbol, int layers)
: viewnode(NULL), bottom_line(NULL), false_bottom1(NULL),
  false_bottom2(NULL), text_focus(0)
{
    int i;

    this->symbol = symbol;
    this->layers = (graSymbolModes) layers;
    this->popup_menu = NULL;

    for (i = 0; i < GRA_MAX_MODES; i++)
	param_overlay[i] = NULL;

    text_mode = graTextModeRight;
    text_width = text_height = 0;

    char_marker = 0;

    editbuffer = NULL;
    buffer = NULL;

    set_layers (layers);

    modified_token = NULL;
    modification_start = modification_end = -1;
}

//------------------------------------------
// get_pictel
//
// Return the static picture for a layer of this symbol.
//------------------------------------------

inline graAttachableSymbol* graParametricSymbol::get_pictel(int layer)
{
    return driver_instance->graphics_world->get_pictel(symbol, layer);
}

//------------------------------------------
// Constructor graParametricSymbol
//
// This constructor creates a new parametric symbol. These symbols
// are inserted into a graWorld to be visualized and reacted with.
// This constructor takes one text parameter.
// Symbolss text mode controls the way the label is set.
//----------------------------------------------------

graParametricSymbol::graParametricSymbol(
    int _symbol, int _layers, char* text, graTextPositions position)
: viewnode(NULL), bottom_line(NULL), false_bottom1(NULL),
  false_bottom2(NULL)
{
    Graphic* label;
    Picture* pp;
    PFont   *my_font;
    int     i;
    int     offset = 0;
    float   cx, cy, w, h;
    float   lab_llx, lab_lly, lab_urx, lab_ury;
    char*   vis_text = 0;

    init_PFont();
    init_PColor();

    this->symbol = _symbol;
    this->layers = (graSymbolModes) _layers;

    text_focus = 0;
    char_marker = 0;

    // Initialize text buffer
    buffer = NULL;
    editbuffer = NULL;

    // Initialize modification housekeeping
    modified_token = NULL;
    modification_start = modification_end = -1;

    popup_menu = NULL;
    for (i = 0; i < GRA_MAX_MODES; i++)
	param_overlay[i] = NULL;

    //  If no actual text given, do not reserve any space for it.

    if ( (text_mode = position) == graTextModeUnknown)
	text_mode = (graTextPositions)
	    driver_instance->graphics_world->get_text_mode(_symbol);
    if (text_mode == graTextModeSuppressed) {
	text_width = text_height = 0;
	set_layers(_layers);
	return;
    }

    if (_symbol >= 0)
	driver_instance->graphics_world->get_symbol_size (_symbol,
							  cx, cy, w, h);
    if ( text_mode != graTextModeInside && text_mode !=
	 graTextModeInsideNoBox &&
	 (strcmp(text, "") == 0 || strcmp(text, " ") == 0)){
	text_mode = graTextModeRight;
	text_width = text_height = 0;
	set_layers (_layers);
	return;
    }

    //  Insert the text parameter according to the symbols rules
    //  For graTextModePositioned mode cut the text to fixed size.

    param_overlay[graLayerNoNormal] = new Picture;

    if (_symbol == ldrFLOW_LABEL)
	my_font = flow_label_text_font;
    else
	my_font = base_font;

    if (text_mode == graTextModePositioned)
    {
	if (_symbol == ldrIF_FLOW)
	    offset = 5;

	int real_len = strlen(text); // in chars
	int pix_len = my_font->Width(text); // in pixels
	// in chars :
	int visible_len = int((w - 12) * real_len / pix_len);
	if (real_len <= visible_len)
	    label = new iv3(Label) (text);

	// cut text and show .. if text does not fit in the slot
	else
	{
	    vis_text = new char [visible_len];
	    strncpy( vis_text, text, visible_len - 4);
	    vis_text[visible_len-4] = '.';
	    vis_text[visible_len-3] = '.';
	    vis_text[visible_len-2] = '.';
	    vis_text[visible_len-1] = '\0';
	    label = new iv3(Label) (vis_text);
	}
    }
    else if (text_mode == graTextModeBottom)
    {
	// truncate text string to width of symbol or visible len
	int visible_len = 10;
	int real_len = strlen (text);
	int char_offset = my_font->Index (text, (int)w, true);

	char_offset = visible_len > char_offset ? visible_len : char_offset;
	if (char_offset < real_len-3)
	{
	    char* vis_text1 = new char [char_offset + 4];
	    strncpy (vis_text1, text, char_offset);
	    vis_text1[char_offset] =
		vis_text1[char_offset+1] =
		    vis_text1[char_offset+2] = '.';
	    vis_text1[char_offset+3] = '\0';
	    label = new iv3(Label) (vis_text1);
	    delete [] vis_text1;
	}
	else
	    label = new iv3(Label) (text);
    }
    else
	label = new iv3(Label) (text);

    label->SetFont (my_font);
    label->FillBg (FILLBG_YES);
    label->SetColors (base_foreground, base_background);
    param_overlay[graLayerNoNormal]->Append (label);

    // There is no symbol for negative values
    if (_symbol >= 0)
    {
	label->GetBounds (lab_llx, lab_lly, lab_urx, lab_ury);
	text_width = int(lab_urx - lab_llx + 0.5);
	text_height = int(lab_ury - lab_lly + 0.5);

	switch (text_mode)
	{
	  case graTextModeLeft:
	    label->Translate (-text_width - 3.0, 0.0);
	    break;

	  default:
	  case graTextModeRight:
	    label->Translate (w + 3.0, h/2.0 - text_height/2.0);
	    break;

	  case graTextModeTop:
	    label->Translate (w/2.0 - text_width/2.0, h + 3.0);
	    break;

	  case graTextModeBottom:
	    label->Translate (w/2.0 - text_width/2.0, -text_height - 3.0);
	    break;

	  case graTextModeInside:
	  case graTextModeInsideNoBox:
	    pp = get_pictel(0);
	    if (pp)
	    {
		Graphic* left_half = pp->First();
		if (left_half)
		    left_half->GetBounds(cx, cy, w, h);
	    }
	    label->Translate (w, ((text_height / 13.0) * h - text_height) / 2.0);
	    create_label_box(1);
	    break;

	  case graTextModePositioned:
	    // Do not put text into narrow corner of a diamond symbol.
	    if (text_width >= w)
		label->Translate (offset + 8,
				  (h - text_height)/2.0);
	    else
		label->Translate (offset + (w - text_width)/2.,
				  (h - text_height)/2.0);
	    break;
	}
    }
    else
    {
	text_mode = graTextModeRight;
	text_width = text_height = 0;
    }

    // Hilite layer
    param_overlay[graLayerNoHilited] = new Picture;
    label = new iv3(Label) (text, label);
    label->SetFont (hilite_font);
    label->SetColors (hilite_foreground, hilite_background);
    param_overlay[graLayerNoHilited]->Append (label);
    if (text_mode == graTextModeInside || text_mode == graTextModeInsideNoBox)
	create_label_box(graLayerNoHilited);

    // Selected Layer
    param_overlay[graLayerNoSelected] = new Picture;

    if (text_mode == graTextModePositioned && vis_text){
//      text has been cut to fit the symbol
        label = new iv3(Label) (vis_text, label);
	delete [] vis_text;
    }
    else
        label = new iv3(Label) (text, label);

    label->SetFont (select_font);
    label->SetColors (select_foreground, select_background);
    param_overlay[graLayerNoSelected]->Append (label);
    if (text_mode == graTextModeInside || text_mode == graTextModeInsideNoBox)
	create_label_box(graLayerNoSelected);

    // setup cursor layer using selected layer
    param_overlay[graLayerNoCursor] = new Picture;
    cursor_y[3] = cursor_y[6] = text_height;
    cursor_y[4] = cursor_y[5] = text_height + 2;

    FillPolygon* cursor = new FillPolygon (cursor_x, cursor_y, 9, label);
    cursor->SetColors (cursor_foreground, cursor_background);
    param_overlay[graLayerNoCursor]->Append(cursor);

    set_layers (_layers);
}

// Destructor
graParametricSymbol::~graParametricSymbol()
{
    Initialize(graParametricSymbol::~graParametricSymbol);

    if(viewnode)
    {
	// Remove reference to picture to prevent second delete.
        viewnode->picture = NULL;

	viewGraHeader* header =
	    checked_cast(viewGraHeader,viewnode->get_header());
	if(header)
	    header->get_gra_interface()->RemoveSymbol(this);
    }
    if (editbuffer)
    {
	delete editbuffer;
	delete buffer;
    }
}

//------------------------------------------
// get_rubband
//
// Return the Rubberband* used to drag this symbol.
//------------------------------------------

iv3(Rubberband)* graParametricSymbol::get_rubband()
{
    Initialize (graParametricSymbol::get_rubband);

    return driver_instance->graphics_world->get_rubband(symbol);
}

//------------------------------------------
// add_layers
//
// Turn on layers in the symbol
//------------------------------------------

void graParametricSymbol::add_layers( int layers )
{
    Initialize (graParametricSymbol::add_layers);

    set_layers (this->layers | layers);
}

//------------------------------------------
// rem_layers
//
// Turn off layers in the symbol
//------------------------------------------

void graParametricSymbol::rem_layers( int layers )
{
    Initialize (graParametricSymbol::rem_layers);

    set_layers (this->layers & ~layers);
}

//------------------------------------------
// get_layers
//
// Report the current layers
//------------------------------------------

int graParametricSymbol::get_layers()
{
    Initialize(graParametricSymbol::get_layers);

    return layers;
}

//------------------------------------------
// set_layers
//
// Clear the old symbol and turn on the reqested layers
//------------------------------------------

void graParametricSymbol::set_layers( int layers )
{
    Initialize (graParametricSymbol::set_layers);

    // Symbols are built assuming a 12-pixel-high font; if we are using
    // a different font size, we must scale the graphics accordingly (the
    // factor of "13.0" is because we use text_height, which came from
    // GetBounds on the label, which adds a pixel to the true height.

    float scale = (text_height == 0) ? 1.0 : float(text_height) / 13.0;

    this->layers = (graSymbolModes) layers;

    // Remove all current layer graphics
    for(Graphic* graphic = First(), *next = NULL; graphic; graphic = next)
    {
	next = Next();
	if (graphic != false_bottom1 && graphic != false_bottom2)
	{
	    Remove(graphic);
	    delete graphic;
	}
    }
    clear_attachment_points();

    // !!!KLUGE!!! Set bottom_line pointer to 1, so that we know it should be
    // re-created later
    if (bottom_line)
	bottom_line = (Graphic*)1;

    // Turn on requested layers

    int i;
    for(i = 0; i< GRA_MAX_MODES; i++ )
    {
	if ( layers & (1 << i) )
	{
            graAttachableSymbol* pp = get_pictel(i);
	    if (pp)
	    {
                if (text_mode == graTextModeInside ||
		    text_mode == graTextModeInsideNoBox)
		{
                    Graphic* left_half = pp->First();
                    Graphic* right_half = pp->Last();
                    if (!left_half || !right_half || left_half == right_half)
		    {
                        Graphic* g = new Instance(pp);
			g->Scale(1.0, scale);
			if (i == graLayerNoDecorationLC ||
			    i == graLayerNoDecorationUC)
			    g->Translate(text_width / 2.0, 0);
			else if (i == graLayerNoDecorationLR ||
				 i == graLayerNoDecorationCR ||
				 i == graLayerNoDecorationUR)
			    g->Translate(text_width, 0);
                        Append(g);
                    }
                    else
		    {
                        left_half = new Instance(left_half);
			left_half->Scale(1.0, scale);
                        right_half = new Instance(right_half);
			right_half->Scale(1.0, scale);
			right_half->Translate(text_width, 0.0);
			Append(left_half);
			Append(right_half);
                    }
                }
		else {
		   Graphic* g = new Instance(pp);
		   g->Scale(1.0, scale);
		   Append(g);
		}

                for (int j = 0; j < graNumAttachmentPoints; j++)
		{
                    int layer_x, layer_y;
                    pp->get_attachment_point(
			graAttachmentPoint(j), layer_x, layer_y);

                    if (layer_x != 0 || layer_y != 0)
		    {
                        if (text_mode == graTextModeInside ||
			    text_mode == graTextModeInsideNoBox)
			{
                            if (j == graAttachLC || j == graAttachUC)
                                layer_x += text_width / 2;
                            else if (j == graAttachLR ||
				     j == graAttachCR ||
				     j == graAttachUR)
                                layer_x += text_width;
                        }
                        x[j] = layer_x;
                        y[j] = int(layer_y * scale + 0.5);
                    }
                }
            }
        }
    }

    // Turn on requested parameters

    for (i = 0; i < GRA_MAX_MODES; i++ )
        if (layers & (1 << i))
            if (param_overlay[i])
   	        Append (new Instance(param_overlay[i]));

    if (bottom_line)
	add_bottom_line();
}

//------------------------------------------
// get_symbol_size
//
// This function gets the center of the symbol and its width and height
//------------------------------------------

void graParametricSymbol::get_symbol_size(
    float &centerx, float &centery, float &width, float &height,
    bool include_text)
{
    Initialize (graParametricSymbol::get_symbol_size);

    float llx, lly, urx, ury;

    if (this->symbol != -1)
    {
	get_pictel(0)->GetBounds (llx, lly, urx, ury);
	width = urx - llx;
	height = ury - lly;
	get_pictel(0)->GetCenter (centerx, centery);
	if (text_mode == graTextModeInside ||
	    text_mode == graTextModeInsideNoBox ||
	    include_text)
	{
	    width += text_width;
	    centerx += text_width / 2;
	}
	// ignore text width for Positioned mode

	if (text_height != 0) {
	   // Account for fonts larger or smaller than 12 pixels
	   float scale = text_height / 13.0;
	   height *= scale;
	   centery *= scale;
	}
    }
    else
    {
	Graphic* label = NULL;
	centerx = centery = width = height = 0.0;

	for (int i = 0; i < GRA_MAX_MODES; i++)
	    if (param_overlay[i])
	    {
		label = param_overlay[i]->First();
		break;
	    }

	// get the height of the label
	if (label)
	{
	    label->GetBounds (llx, lly, urx, ury);
	    height = ury - lly;
	}
    }
}

//------------------------------------------
// graConnectionSymbol::graConnectionSymbol()
//------------------------------------------

/* The default constructor is protected so it can be used only from
 * derived classes; the only difference from the public constructor is
 * that it does not do a regenerate() automatically.
 */

graConnectionSymbol::graConnectionSymbol()
: graParametricSymbol(-1, 0)
{
    num_segments = 0;
    segments = NULL;
}

//------------------------------------------
// Constructor graConnectionSymbol
//------------------------------------------

graConnectionSymbol::graConnectionSymbol( viewConnectionNode& VN )
: graParametricSymbol (-1,0)
{
    num_segments = 0;
    segments = NULL;
    regenerate (VN);
}

//------------------------------------------
// Destructor graConnectionSymbol
//------------------------------------------

graConnectionSymbol::~graConnectionSymbol()
{
    delete [] segments;
}

//------------------------------------------
// Various no-op virtual functions.
//------------------------------------------

void graConnectionSymbol::set_layers(int) {}
void graConnectionSymbol::add_layers(int) {}
void graConnectionSymbol::rem_layers(int) {}

//------------------------------------------
// gra_arrow_head(...)
//------------------------------------------

static FillPolygon* gra_arrow_head(
    arrow_type type, float x_coord, float y_coord, float len, float w)
{
    Initialize (gra_arrow_head);

    iv2(Coord) x_arr[3];
    iv2(Coord) y_arr[3];

    switch (type)
    {
      case ARROW_DOWN :
	x_arr[0] = (int)  x_coord;
	x_arr[1] = (int) (x_coord - w);
	x_arr[2] = (int) (x_coord + w);
	y_arr[0] = (int)  y_coord;
	y_arr[1] = (int) (y_coord + len);
	y_arr[2] = (int) (y_coord + len);
	break;

      case ARROW_RIGHT :	// --->
	x_arr[0] = (int)  x_coord;
	x_arr[1] = (int) (x_coord - len);
	x_arr[2] = (int) (x_coord - len);
	y_arr[0] = (int)  y_coord;
	y_arr[1] = (int) (y_coord - w);
	y_arr[2] = (int) (y_coord + w);
	break;

      case ARROW_LEFT_LONG:	// <----
	x_arr[0] = (int) (x_coord - len);
	x_arr[1] = (int)  x_coord ;
	x_arr[2] = (int)  x_coord ;
	y_arr[0] = (int)  y_coord;
	y_arr[1] = (int) (y_coord - w);
	y_arr[2] = (int) (y_coord + w);
	break;

      case ARROW_LEFT:		// <--
	x_arr[0] = (int)  x_coord;
	x_arr[1] = (int) (x_coord + len);
	x_arr[2] = (int) (x_coord + len);
	y_arr[0] = (int)  y_coord;
	y_arr[1] = (int) (y_coord - w);
	y_arr[2] = (int) (y_coord + w);
	break;

      case ARROW_UP:
	x_arr[0] = (int)  x_coord;
	x_arr[1] = (int) (x_coord - w);
	x_arr[2] = (int) (x_coord + w);
	y_arr[0] = (int)  y_coord;
	y_arr[1] = (int) (y_coord - len);
	y_arr[2] = (int) (y_coord - len);
	break;

      default:			// unKnown type
	Error(ERR_INPUT);
    }
   
    FillPolygon* pp = new FillPolygon(x_arr, y_arr, 3);
    pp->SetColors ( connection_foreground, connection_background);
    return pp;
}

//------------------------------------------
// cmp_coords_descending
//
// Function to compare y coordinates used by qsort to order symbols
// top to bottom
//------------------------------------------

static int cmp_coords_descending( const void* c1,  const void* c2 )
{
    return ((_hcoords*)c2)->y - ((_hcoords*)c1)->y;
}

//------------------------------------------
// cmp_coords_ascending
//
// Function to compare y coordinates used by qsort to order symbols
// bottom to top
//------------------------------------------

static int cmp_coords_ascending( const void* c1,  const void* c2 )
{
    return ((_hcoords*)c1)->y - ((_hcoords*)c2)->y;
}

//------------------------------------------
// cmp_x_coords_ascending
//
// Function to compare x coordinates used by qsort to order symbols
// left to right
//------------------------------------------

static int cmp_x_coords_ascending( const void* c1,  const void* c2 )
{
    return ((_hcoords*)c1)->x - ((_hcoords*)c2)->x;
}


//------------------------------------------
// regenerate
//
// This function regenerates the line segment list of a connector symbol.
// This is done by examining all current input and output symbols.
//------------------------------------------

void graConnectionSymbol::regenerate( viewConnectionNode &VN )
{
    Initialize (graConnectionSymbol::regenerate);

    init_PBrush();
    init_PColor();

    float i_cx = 0.0;		// connection point of input
    float i_cy = 0.0;
    float last_x = 0.0;		// location of last joint in multi-segment
    float last_y = 0.0;

    // connectors
    int x = 0;
    int y = 0;
    float delta_x = 0.0;
    float delta_y = 0.0;

    Line* segment;

    PColor* fg = connection_foreground;
    PColor* bg = connection_background;
    struct _hcoords* hcoords;
    int size_hcoords;
    int i, j;
    FillPolygon* pp;
    viewSymbolNodePtr Output_VU;
    int connector_type = VN.map_symbol();

    // Delete all old line segments

    if(num_segments)
    {
        if(get_viewnode())
	{
	    viewGraHeaderPtr vh =
		checked_cast(viewGraHeader,get_viewnode()->get_header());
	    if(vh)
		vh->get_gra_interface()->DamageArea(this);
	}
	delete_picture_contents(this);
	delete [] segments;
	segments = NULL;
	num_segments = 0;
    }

    // Get the first input symbol and its position. For now there is only
    // one input symbol - this logic will have to change.

    viewSymbolNodePtr First_Input =
	checked_cast(viewSymbolNode,(*VN.inputs)[0]);
    if (First_Input->get_picture () == NULL)
	return;
    First_Input->get_position(i_cx, i_cy);

    switch(connector_type)
    {
      case ldrcHIERARCHY:
      case ldrcTRUE_BRANCH:
      case ldrcENDIF:
      case ldrcFOR_BODY:
      case ldrcSEQUENCE:
        First_Input->get_attachment_point(graAttachLC, x, y);
        break;

      case ldrcLEFT_HORIZONTAL:
      case ldrcCASE_EXIT:
        First_Input->get_attachment_point(graAttachCL, x, y);
        break;

      case ldrcIF_ELSE:       //  if/else or else_if / else connector
      case ldrcTREE:          // horizontal call tree
        First_Input->get_attachment_point(graAttachCR, x, y);
// - vertical Tree First_Input->get_attachment_point(graAttachLC, x, y);
        break;

      case ldrcLOOP_DO:
        First_Input->get_attachment_point(graAttachCL, x, y);
        break;

      case ldrcENDELSE:
        First_Input->get_attachment_point(graAttachLC, x, y);
        break;

      case ldrcSUPERCLASS_LIST:
        First_Input->get_attachment_point(graAttachUL, x, y);
        break;

      case ldrcSUBCLASS_LIST:
        First_Input->get_attachment_point(graAttachLR, x, y);
        break;

      case ldrcMEMBER_LIST:
        First_Input->get_attachment_point(graAttachLL, x, y);
        break;

      case ldrcCONTAINER:
        First_Input->get_attachment_point(graAttachUR, x, y);
        break;

      case ldrcFALSE_BRANCH:
      case ldrcREL_LIST_OUT:
      case ldrcREL_OUT_1_OPT:
      case ldrcREL_OUT_N_OPT:
      case ldrcREL_OUT_1_REQ:
      case ldrcREL_OUT_N_REQ:
        First_Input->get_attachment_point(graAttachCR, x, y);
        break;

      case ldrcREL_LIST_IN:
      case ldrcREL_IN_1_OPT:
      case ldrcREL_IN_N_OPT:
      case ldrcREL_IN_1_REQ:
      case ldrcREL_IN_N_REQ:
      case ldrcUP_TREE:
        First_Input->get_attachment_point(graAttachCL, x, y);
// vertical tree First_Input->get_attachment_point(graAttachUC, x, y);
        break;

      case ldrcINVARIANT:
        First_Input->get_attachment_point(graAttachUC, x, y);
        break;
    }
    i_cx += x;
    i_cy += y;

// Get an array for storing information about all output symbols. This array
// can then be sorted.

    objArr outputs;
    Obj* el;
    ForEach (el, *(VN.outputs)) {
	if (checked_cast(viewSymbolNode,el)->get_picture())
	    outputs.insert_last (el);
    }

    size_hcoords = outputs.size ();
    if (size_hcoords == 0)
	return;

    hcoords = new _hcoords[size_hcoords];
    if ( hcoords == NULL )
        return;

    for ( i=0; i<size_hcoords; i++ ){

	Output_VU = checked_cast(viewSymbolNode,outputs[i]);
	hcoords[i].viewnode = Output_VU;
	Output_VU->get_position(hcoords[i].x, hcoords[i].y);

	switch (connector_type) {

	  case ldrcHIERARCHY:
	  case ldrcREL_LIST_OUT:
	  case ldrcREL_OUT_1_OPT:
	  case ldrcREL_OUT_N_OPT:
	  case ldrcREL_OUT_1_REQ:
	  case ldrcREL_OUT_N_REQ:
	    Output_VU->get_attachment_point(graAttachCL, x, y);
	    break;

	  case ldrcTRUE_BRANCH:
	  case ldrcFALSE_BRANCH:
	  case ldrcENDIF:
	  case ldrcFOR_BODY:
	  case ldrcSEQUENCE:
	    Output_VU->get_attachment_point(graAttachUC, x, y);
	    break;

	  case ldrcLEFT_HORIZONTAL:
	    Output_VU->get_attachment_point(graAttachCR, x, y);
	    break;

	  case ldrcIF_ELSE:
	  case ldrcLOOP_DO:
	  case ldrcCASE_EXIT:
	  case ldrcTREE:
            Output_VU->get_attachment_point(graAttachCL, x, y);
// vertical tree Output_VU->get_attachment_point(graAttachUC, x, y);
            break;

	  case ldrcENDELSE:
            Output_VU->get_attachment_point(graAttachCR, x, y);
            break;

	  case ldrcSUPERCLASS_LIST:
	    Output_VU->get_attachment_point(graAttachLR, x, y);
	    break;

	  case ldrcSUBCLASS_LIST:
	    Output_VU->get_attachment_point(graAttachUL, x, y);
	    break;

	  case ldrcMEMBER_LIST:
	    Output_VU->get_attachment_point(graAttachUR, x, y);
	    break;

	  case ldrcCONTAINER:
	    Output_VU->get_attachment_point(graAttachLL, x, y);
	    break;

	  case ldrcREL_LIST_IN:
	  case ldrcREL_IN_1_OPT:
	  case ldrcREL_IN_N_OPT:
	  case ldrcREL_IN_1_REQ:
	  case ldrcREL_IN_N_REQ:
	  case ldrcUP_TREE:
// vertical tree   Output_VU->get_attachment_point(graAttachLC, x, y);
	    Output_VU->get_attachment_point(graAttachCR, x, y);
	    break;

	  case ldrcINVARIANT:
	    Output_VU->get_attachment_point(graAttachLC, x, y);
	    break;
	}

	hcoords[i].x += x;
	hcoords[i].y += y;
    }

// Sort the array decreasing y values

    if (connector_type == ldrcSUPERCLASS_LIST ||
        connector_type == ldrcUP_TREE  ||
        connector_type == ldrcTREE )
        qsort(hcoords, size_hcoords, sizeof(_hcoords), cmp_coords_ascending);
/* --- vertical tree; sort by x value decreasing
   else if (connector_type == ldrcTREE ||
   connector_type == ldrcUP_TREE )
   qsort(hcoords, size_hcoords, sizeof(_hcoords), cmp_x_coords_ascending);
   */
   else qsort (hcoords, size_hcoords, sizeof(_hcoords),
	       cmp_coords_descending);

// Determine what kind of connector we need and generate the line segments

    switch (connector_type) {

      case ldrcHIERARCHY:
      case ldrcSUPERCLASS_LIST:
      case ldrcSUBCLASS_LIST:
      case ldrcREL_LIST_OUT:
      case ldrcREL_LIST_IN:
      case ldrcREL_OUT_1_OPT:
      case ldrcREL_OUT_N_OPT:
      case ldrcREL_OUT_1_REQ:
      case ldrcREL_OUT_N_REQ:
      case ldrcREL_IN_1_OPT:
      case ldrcREL_IN_N_OPT:
      case ldrcREL_IN_1_REQ:
      case ldrcREL_IN_N_REQ:
      case ldrcMEMBER_LIST:
      case ldrcCONTAINER:
      case ldrcINVARIANT:

// The connector is a hierarchical connector. The line segments must be
// generated in the order...
//
//	|
//	1
//	|
//	--2--
//	|
//	3
//	|
//	--4--
//
// (inverted for superclass lists, of course)
//

        num_segments = (connector_type == ldrcMEMBER_LIST ?
			2 : size_hcoords * 2);
	segments = new _segments[num_segments];
        if ( segments == NULL )
	    return;

// The following code computes delta values for different geometries of
// connections, of which there are three:
//
// 1:		|
// 		+--
// 		|
// 		+--
//
// 2:		--+--
// 		  |
// 		  +--
//
// 3:		\
// 		 \
// 		 |\
// 		 | \
// 		 |  \
// 		  \
// 		   \
// 		    \
//
// (These may be reflected left-to-right and/or top-to-bottom, but the
// signs work out correctly from the symbol node layout chosen.)
//
// The delta_x value determines how far to the right of the input symbols
// connection point subsequent segments will connect; the delta_y value
// determines how far below the input symbols connection point the
// junction point is (and, by extension, how far above the target symbols
// connection point the junction will be made).

        switch(connector_type)
	{
	  case ldrcHIERARCHY:		// case 1
            delta_x = 0.0;
            delta_y = hcoords[0].y - i_cy;
            break;

	  case ldrcSUPERCLASS_LIST:
	  case ldrcSUBCLASS_LIST:		// case 3
	  case ldrcMEMBER_LIST:
	  case ldrcCONTAINER:
	  case ldrcINVARIANT:
            delta_x = (hcoords[0].x > i_cx) ? VIEW_MULTI_SUBCLASS_HSHIFT :
	    -VIEW_MULTI_SUBCLASS_HSHIFT;
            delta_y = (hcoords[0].y > i_cy) ? VIEW_MULTI_SUBCLASS_HSHIFT :
	    -VIEW_MULTI_SUBCLASS_HSHIFT;
            break;

	  case ldrcREL_LIST_OUT:
	  case ldrcREL_LIST_IN:		// case 2
	  case ldrcREL_OUT_1_OPT:
	  case ldrcREL_OUT_N_OPT:
	  case ldrcREL_OUT_1_REQ:
	  case ldrcREL_OUT_N_REQ:
	  case ldrcREL_IN_1_OPT:
	  case ldrcREL_IN_N_OPT:
	  case ldrcREL_IN_1_REQ:
	  case ldrcREL_IN_N_REQ:
            delta_x = (hcoords[0].x > i_cx) ? VIEW_MULTI_SUBCLASS_HSHIFT :
	    -VIEW_MULTI_SUBCLASS_HSHIFT;
            delta_y = 0.0;
            break;
        }

        for ( i=0; i<size_hcoords; i++ )
	{
            if ( i == 0 )
	    {
                last_x = i_cx + delta_x;
                last_y = i_cy + delta_y;
                segment = new Line (
		    (iv2(Coord)) i_cx, (iv2(Coord)) i_cy,
		    (iv2(Coord)) last_x, (iv2(Coord)) last_y);
            } else {
                segment = new Line (
		    (iv2(Coord)) last_x,
		    (iv2(Coord)) last_y,
		    (iv2(Coord)) last_x,
		    (iv2(Coord)) (last_y + (hcoords[i].y - hcoords[i - 1].y)));
                last_y += (hcoords[i].y - hcoords[i - 1].y);
	    }

            segment->SetBrush(pbrush0);
            ((Graphic*)segment)->SetColors(fg, bg);
            Append(segment);

            Graphic* arrow;
            if (i == 0)
	    {
                switch(connector_type)
		{
		  case ldrcREL_IN_N_OPT:
		  case ldrcREL_IN_N_REQ:
                    arrow = gra_arrow_head(ARROW_RIGHT, i_cx - 7, i_cy, 7, 3);
                    arrow->SetColors(fg, bg);
                    Append(arrow);
                    /* FALLTHROUGH */

		  case ldrcREL_IN_1_OPT:
		  case ldrcREL_IN_1_REQ:
                    arrow = gra_arrow_head(ARROW_RIGHT, i_cx, i_cy, 7, 3);
                    arrow->SetColors(fg, bg);
                    Append(arrow);
                    if (connector_type == ldrcREL_IN_1_OPT ||
			connector_type == ldrcREL_IN_N_OPT)
                        Append(new Circle(
			    (iv2(Coord)) i_cx, (iv2(Coord)) i_cy, 4, segment));
                    break;
                }
            }

// Insert line segment into segment array along with corresponding viewnode

            segments[i*2].line = segment;
            segments[i*2].viewnode = hcoords[i].viewnode;

            segment = new Line (
		(iv2(Coord)) last_x, (iv2(Coord)) last_y,
		(iv2(Coord)) hcoords[i].x, (iv2(Coord)) hcoords[i].y );
            segment->SetBrush(pbrush0);
            ((Graphic*) segment)->SetColors (fg,bg);
            Append(segment);

	    switch(connector_type)
	    {
	      case ldrcREL_OUT_N_OPT:
	      case ldrcREL_OUT_N_REQ:
                arrow = gra_arrow_head(ARROW_RIGHT, hcoords[i].x - 7,
				       hcoords[i].y, 7, 3);
                arrow->SetColors(fg, bg);
		Append(arrow);
		/* FALLTHROUGH */

	      case ldrcREL_OUT_1_OPT:
	      case ldrcREL_OUT_1_REQ:
                arrow = gra_arrow_head(ARROW_RIGHT, hcoords[i].x,
				       hcoords[i].y, 7, 3);
                arrow->SetColors(fg, bg);
		Append(arrow);
		if (connector_type == ldrcREL_OUT_1_OPT ||
		    connector_type == ldrcREL_OUT_N_OPT)
		    Append(new Circle((iv2(Coord)) hcoords[i].x,
				      (iv2(Coord)) hcoords[i].y, 4, segment));
		break;
	    }

            segments[i*2+1].line = segment;
            segments[i*2+1].viewnode = hcoords[i].viewnode;

            if (connector_type == ldrcMEMBER_LIST)
		break;	// only first member is visually connected to class
        }
        segments[0].viewnode = First_Input;
        break;


//   arrow head for end/if, if/else or else_if/else

      case ldrcIF_ELSE:
      case ldrcENDIF:
      case ldrcTRUE_BRANCH:
      case ldrcLEFT_HORIZONTAL:
      case ldrcSEQUENCE:

	if (i_cx == hcoords[0].x)       // vertical down arrow
	    pp = gra_arrow_head(ARROW_DOWN, hcoords[0].x,
				hcoords[0].y, 7, 3);

	else if (i_cx < hcoords[0].x)   // horizontal right arrow
	    pp = gra_arrow_head(ARROW_RIGHT, hcoords[0].x,
				hcoords[0].y, 7, 3);

	else                           // horizontal left arrow
	    pp = gra_arrow_head(ARROW_LEFT_LONG, hcoords[0].x,
				hcoords[0].y, 9, 4);   // special longer one
	Append(pp);

      case ldrcFOR_BODY:

// These types of connectors are a straight line

        num_segments = 1;
	segments = new _segments[num_segments];
        if ( segments == NULL )
	    return;

        segment = new Line(
	    (iv2(Coord))i_cx, (iv2(Coord))i_cy,
	    (iv2(Coord))hcoords[0].x, (iv2(Coord))hcoords[0].y );
        segments[0].line = segment;
        segments[0].viewnode = hcoords[0].viewnode;

	// thick line for GOTO

        if (connector_type == ldrcLEFT_HORIZONTAL)
            segment->SetBrush(pbrush3);
        else
            segment->SetBrush(pbrush0);
        ((Graphic*)segment)->SetColors(fg,bg);
        Append (segment);
        break;


      case ldrcENDELSE:

	// This type of connector is a 90 degree angle line
	// with first vertical and the second horizontal parts

	num_segments = 2;
	segments = new _segments[num_segments];
	if ( segments == NULL )
	    return;

	segment = new Line ((iv2(Coord))i_cx, (iv2(Coord))i_cy ,
          	            (iv2(Coord))i_cx, (iv2(Coord))hcoords[0].y );
	segments[0].line = segment;
	segments[0].viewnode = hcoords[0].viewnode;

	segment->SetBrush(pbrush0);
	((Graphic*) segment)->SetColors (fg,bg);
	Append (segment);

	segment = new Line ((iv2(Coord))hcoords[0].x, (iv2(Coord))hcoords[0].y,
			    (iv2(Coord))i_cx, (iv2(Coord))hcoords[0].y );
	segments[1].line = segment;
	segments[1].viewnode = hcoords[0].viewnode;

	segment->SetBrush(pbrush0);
	((Graphic*) segment)->SetColors (fg,bg);
	Append (segment);

	// add an arrow <-

	pp = gra_arrow_head(ARROW_LEFT, hcoords[0].x, hcoords[0].y, 7, 3);
	Append(pp);
	break;

     case ldrcFALSE_BRANCH:

// This type of connector is a 90 degree angle line

        num_segments = 2;
	segments = new _segments[num_segments];
        if ( segments == NULL )
	    return;

        segment = new Line ((iv2(Coord))i_cx, (iv2(Coord))i_cy ,
	    (iv2(Coord))hcoords[0].x, (iv2(Coord))i_cy );
        segments[0].line = segment;
        segments[0].viewnode = hcoords[0].viewnode;

        segment->SetBrush(pbrush0);
        ((Graphic*) segment)->SetColors (fg,bg);
        Append (segment);

        segment = new Line ((iv2(Coord))hcoords[0].x, (iv2(Coord))i_cy,
	    (iv2(Coord))hcoords[0].x, (iv2(Coord))hcoords[0].y );
        segments[1].line = segment;
        segments[1].viewnode = hcoords[0].viewnode;

        segment->SetBrush(pbrush0);
        ((Graphic*) segment)->SetColors (fg,bg);
        Append (segment);
        break;

     case ldrcCASE_EXIT:
     case ldrcLOOP_DO:

	// add arrow heads

	// horizontal left arrow
	pp = gra_arrow_head(ARROW_LEFT, i_cx - VIEW_CONN_SHIFT,  i_cy, 7, 3);

	Append(pp);

	// horizontal right arrow
	pp = gra_arrow_head(ARROW_RIGHT, hcoords[0].x,
			    hcoords[0].y, 7, 3);
	Append(pp);

// This type of connector is a 3 segment lines
//   ----
//   |
//   ----
//  or
//   ----
//      |
//      |----  - can degenerate into a straight line

        num_segments = 3;
	segments = new _segments[num_segments];
        if ( segments == NULL )
	    return;

        delta_x = - VIEW_CONN_SHIFT;

        segment = new Line (
	    (iv2(Coord))i_cx, (iv2(Coord))i_cy ,
	    (iv2(Coord))(i_cx + delta_x), (iv2(Coord))i_cy );
        segments[0].line = segment;
        segments[0].viewnode = hcoords[0].viewnode;

        segment->SetBrush(pbrush0);
        ((Graphic*) segment)->SetColors (fg,bg);
        Append (segment);

        segment = new Line (
	    (iv2(Coord))(i_cx + delta_x), (iv2(Coord))i_cy,
	    (iv2(Coord))(i_cx + delta_x), (iv2(Coord))hcoords[0].y );
        segments[1].line = segment;
        segments[1].viewnode = hcoords[0].viewnode;

        segment->SetBrush(pbrush0);
        ((Graphic*) segment)->SetColors (fg,bg);
        Append (segment);

        segment = new Line (
	    (iv2(Coord))(i_cx + delta_x), (iv2(Coord))hcoords[0].y,
	    (iv2(Coord))hcoords[0].x, (iv2(Coord))hcoords[0].y );
        segments[2].line = segment;
        segments[2].viewnode = hcoords[0].viewnode;

        segment->SetBrush(pbrush0);
        ((Graphic*) segment)->SetColors (fg,bg);
        Append (segment);

        segments[0].viewnode = First_Input;
        break;

//
// This type of connector is a n segment lines
//
//             . input        --- !! now it is HORIZONTAL and both directional
//             |                     from the root
//             |
//   ---------------------- --  tree_offset
//   |     |      |       |
//   .     .      .       .  output points

      case ldrcTREE:
      case ldrcUP_TREE:

//----------------------------  LEFT TO RIGHT GRAPH

#ifdef HORIZ_TREE

// --- horizontal tree code

	float tmp_y, aux_y;

	// same y-offset for CL and CR attachment
	(hcoords[0].viewnode)->get_attachment_point(graAttachCL, x, y);

	if (size_hcoords == 1)
	{
	    // one-to-one connector
	    num_segments = size_hcoords;
	    segments = new _segments[num_segments];
	    if ( segments == NULL )
		return;

	    // try horizontal line;  x, y - attachment point offsets
	    if ( i_cy < (hcoords[0].y + y) &&
		 i_cy > (hcoords[0].y - y) )
		tmp_y = i_cy;
	    else
		tmp_y = hcoords[0].y;

	    segment = new Line((iv2(Coord))i_cx, (iv2(Coord))i_cy,
			       (iv2(Coord))hcoords[0].x, (iv2(Coord))tmp_y );

	    segments[0].line = segment;
	    segments[0].viewnode = First_Input;
	    segment->SetBrush(pbrush0);
	    ((Graphic*) segment)->SetColors (fg,bg);
	    Append (segment);
	}
	else		//  multiple outputs : one-to-many connector
	{
	    float tree_offset = hcoords[0].x - 5; // 5 = node_y_space/2
	    if (i_cx > hcoords[0].x)
		tree_offset += 10; // positive or neagtive offset
	    num_segments = 2 * size_hcoords + 1;
	    segments = new _segments[num_segments];
	    if ( segments == NULL )
		return;

	    // line segment from input
	    if ( hcoords[0].y < i_cy &&	// horizontal
		 hcoords[size_hcoords -1].y > i_cy )
		tmp_y = i_cy;
	    else		// to the middle
		tmp_y = 0.5 * (hcoords[0].y +
			       hcoords[size_hcoords -1].y);
	    segment = new Line ((iv2(Coord))i_cx, (iv2(Coord))i_cy ,
				(iv2(Coord))tree_offset, (iv2(Coord))tmp_y );

	    segments[0].line = segment;
	    segments[0].viewnode = First_Input;

	    segment->SetBrush(pbrush0);
	    ((Graphic*) segment)->SetColors (fg,bg);
	    Append (segment);

	    for (i = 1, j = 0; i < size_hcoords * 2; j++, i += 2 )
	    {
		if (hcoords[j].y >= tmp_y)
		    break;

		segment = new Line(
		    (iv2(Coord))tree_offset, (iv2(Coord))hcoords[j].y,
		    (iv2(Coord))hcoords[j].x, (iv2(Coord))hcoords[j].y );
		segments[i].line = segment;
		segments[i].viewnode = hcoords[j].viewnode;
		segment->SetBrush(pbrush0);
		((Graphic*) segment)->SetColors (fg,bg);
		Append (segment);

		aux_y = hcoords[j + 1].y;
		if (aux_y >= tmp_y)
		    aux_y = tmp_y;

	        segment = new Line (
		    (iv2(Coord))tree_offset, (iv2(Coord))hcoords[j].y,
		    (iv2(Coord))tree_offset, (iv2(Coord))aux_y);
	        segments[i+1].line = segment;
	        segments[i+1].viewnode = First_Input;
	        segment->SetBrush(pbrush0);
	        ((Graphic*) segment)->SetColors (fg,bg);
	        Append (segment);
	    }

	    for (; i < size_hcoords * 2; j++, i += 2)
	    {
		segment = new Line(
                    (iv2(Coord))tree_offset, (iv2(Coord))hcoords[j].y,
                    (iv2(Coord))hcoords[j].x, (iv2(Coord))hcoords[j].y );
                segments[i].line = segment;
                segments[i].viewnode = hcoords[j].viewnode;
                segment->SetBrush(pbrush0);
                ((Graphic*) segment)->SetColors (fg,bg);
                Append (segment);

		segment = new Line (
                    (iv2(Coord))tree_offset, (iv2(Coord))aux_y,
                    (iv2(Coord))tree_offset, (iv2(Coord))hcoords[j].y);
                segments[i+1].line = segment;
                segments[i+1].viewnode = First_Input;
                segment->SetBrush(pbrush0);
                ((Graphic*) segment)->SetColors (fg,bg);
                Append (segment);

		aux_y = hcoords[j].y;
	    }
	}

//-------------- end of horizontal tree

#else

//-------------- vertical tree

//
// --- JJS 2/1/95 ----
// This code (which has been disabled for what looks like quite some
// time) is now seriously out of date.  It is no longer symmetrical
// with the above.
//

	float tmp_x;

//   same x-offset for LC and UC attachment

	(hcoords[0].viewnode)->get_attachment_point(graAttachLC, x, y);
	if (size_hcoords == 1)
	{

//       one-to-one connector
	    num_segments = size_hcoords;
	    segments = new _segments[num_segments];
	    if ( segments == NULL )
		return;

//       try vertical line;  x, y - attachment point offsets
	    if ( i_cx < (hcoords[0].x + x) &&
		 i_cx > (hcoords[0].x - x) )
		tmp_x = i_cx;
	    else
		tmp_x = hcoords[0].x;

	    segment = new Line((iv2(Coord))i_cx, (iv2(Coord))i_cy,
			       (iv2(Coord))tmp_x, (iv2(Coord))hcoords[0].y );

	    segments[0].line = segment;
	    segments[0].viewnode = First_Input;
	    segment->SetBrush(pbrush0);
	    ((Graphic*) segment)->SetColors (fg,bg);
	    Append (segment);
	}
	else			//   multiple outputs : one-to-many connector
	{
	    float tree_offset = hcoords[0].y - 7; /* 7 = node_y_space /2 */
	    /* see view_edge_init  */
	    num_segments = 2 * size_hcoords;
	    segments = new _segments[num_segments];
	    if ( segments == NULL )
		return;

	    // line segment from input
	    if ( hcoords[0].x  < i_cx && // vertical
		 hcoords[size_hcoords -1].x  > i_cx )
		tmp_x = i_cx;
	    else		// to the middle
		tmp_x = 0.5 * (hcoords[0].x +
			       hcoords[size_hcoords -1].x);
	    segment = new Line ((iv2(Coord))i_cx, (iv2(Coord))i_cy ,
				(iv2(Coord))tmp_x, (iv2(Coord))tree_offset );

	    segments[0].line = segment;
	    segments[0].viewnode = First_Input;

	    segment->SetBrush(pbrush0);
	    ((Graphic*) segment)->SetColors (fg,bg);
	    Append (segment);

	    // connect ouputs
	    for ( i=1, j=0; i < size_hcoords * 2; j++, i+=2 )
	    {
		segment = new Line(
		    (iv2(Coord))hcoords[j].x, (iv2(Coord))tree_offset,
		    (iv2(Coord))hcoords[j].x, (iv2(Coord))hcoords[j].y );
		segments[i].line = segment;
		segments[i].viewnode = hcoords[j].viewnode;
		segment->SetBrush(pbrush0);
		((Graphic*) segment)->SetColors (fg,bg);
		Append (segment);

		// horizontal line segment
		if ( j < size_hcoords -1)
		{
		    segment = new Line (
			(iv2(Coord))hcoords[j].x, (iv2(Coord))tree_offset,
			(iv2(Coord))hcoords[j+1].x, (iv2(Coord))tree_offset);
		    segments[i+1].line = segment;
		    segments[i+1].viewnode = First_Input;
		    segment->SetBrush(pbrush0);
		    ((Graphic*)segment)->SetColors (fg,bg);
		    Append(segment);
		}
	    }
	}
#endif
	break;
    }

    delete [] hcoords;		// Deallocate the sorting array
}


//------------------------------------------
// find_insertion
//
// This function determines the insertion point if a symbol was picked.
//------------------------------------------

void graParametricSymbol::find_insertion(
    graWindow*,
    Graphic** selected, int num_selected,
    viewSelection* selection)
{
    Initialize (graParametricSymbol::find_insertion);

    selection->view_node = this->get_viewnode ();
    selection->connector = NULL;
    selection->sub_type = 0;

    for (int selno = 0; selno < num_selected; selno++)
    {
	Graphic* sel;
        if (selected[selno]->IsA(INSTANCE))
	    ((Instance*) selected[selno])->GetOriginal(sel);
        else
	    sel = selected[selno];
        for (int layer = graLayerNoDecorationLL;
	     layer <= graLayerNoDecorationUR; layer++)
	{
            Graphic* decoration = get_pictel(layer);
            if (decoration && sel == decoration)
	    {
                selection->sub_type = layer;
                return;
            }
        }
    }
}

//------------------------------------------
// find_insertion
//
// This function determines the semantically closest symbol node to the
// line segment selected on a connector.
//------------------------------------------

void graConnectionSymbol::find_insertion(
    graWindow*,
    Graphic** selected, int num_selected,
    viewSelection* selection)
{
    static graLayerNo attach[] =
    {
	graLayerNoDecorationLC,	// ldrcHIERARCHY
	graLayerNoDecorationLC,	// ldrcTRUE_BRANCH
	graLayerNoDecorationCR,	// ldrcFALSE_BRANCH
	graLayerNoDecorationLC,	// ldrcENDIF
	graLayerNoDecorationLC,	// ldrcFOR_BODY
	graLayerNoDecorationLC,	// ldrcSEQUENCE
	graLayerNoDecorationUL,	// ldrcSUPERCLASS_LIST
	graLayerNoDecorationLR,	// ldrcSUBCLASS_LIST
	graLayerNoDecorationLL,	// ldrcMEMBER_LIST
	graLayerNoDecorationUR,	// ldrcCONTAINER
	graLayerNoDecorationCR,	// ldrcREL_LIST_OUT
	graLayerNoDecorationCL,	// ldrcREL_LIST_IN
	graLayerNoDecorationUC,	// ldrcINVARIANT
	graLayerNoDecorationLC,	// ldrcENDELSE
	graLayerNoDecorationCR,	// ldrcIF_ELSE
	graLayerNoDecorationCL,	// ldrcLOOP_DO
	graLayerNoDecorationCL,	// ldrcLEFT_HORIZONTAL
	graLayerNoDecorationCL,	// ldrcCASE_EXIT
	graLayerNoDecorationCR,	// ldrcTREE
	graLayerNoDecorationLC,	// ldrcREL_OUT_1_OPT,
	graLayerNoDecorationLC,	// ldrcREL_OUT_N_OPT,
	graLayerNoDecorationLC,	// ldrcREL_OUT_1_REQ,
	graLayerNoDecorationLC,	// ldrcREL_OUT_N_REQ,
	graLayerNoDecorationLC,	// ldrcREL_IN_1_OPT,
	graLayerNoDecorationLC,	// ldrcREL_IN_N_OPT,
	graLayerNoDecorationLC,	// ldrcREL_IN_1_REQ,
	graLayerNoDecorationLC,	// ldrcREL_IN_N_REQ,
	graLayerNoDecorationLC,	// ldrcERD_INHERITANCE,
	graLayerNoDecorationLC,	// ldrcERD_RELATION,
	graLayerNoDecorationLC,	// ldrcERD_IDENTITY,
	graLayerNoDecorationCL	// ldrcUP_TREE
    };

    int i, j;

    Initialize (graConnectionSymbol::find_insertion);

    selection->view_node  = NULL;
    selection->connector  = this->get_viewnode ();
    selection->sub_type  = 0;

    if ( segments == NULL )
	return;

    // Handle picking on odd line segments

    for ( i = 1; i < num_segments; i += 2 )
    {
        for ( j = 0; j < num_selected; j++ )
	{
            if ( segments[i].line == (Line*)selected[j] )
	    {
	        selection->view_node = segments[i].viewnode;
		return;
	    }
        }
    }

    // Handle picking of even line segments

    for ( i = 0; i < num_segments; i += 2 )
    {
        for ( j=0; j<num_selected; j++ )
	{
            if ( segments[i].line == (Line*)selected[j] )
	    {
		if ( i == 0 )	// First line segment picked
		{
 		    selection->view_node = segments[i].viewnode;
                    selection->connector = NULL;
                    selection->sub_type = attach[
			checked_cast(viewConnectionNode,get_viewnode())->
			map_symbol()];
		    return;
		}
		else		// Last two line segments picked
		{
		    selection->view_node = segments[i - 1].viewnode;
		    return;
	  	}
	    }
        }
    }
}

//------------------------------------------
// set_position
//
// Sets the position of a symbol
//------------------------------------------

void graParametricSymbol::set_position( float cx, float cy )
{
    Initialize (graParametricSymbol::set_position);

    iv3(Transformer)* t = NULL;

    iv3(Transformer)* local = GetTransformer();
    if (local)
    {
	float a1, a2, a3, a4, a5, a6;
	local->GetEntries(a1, a2, a3, a4, a5, a6);

	t = new iv3(Transformer) (a1, 0.0, 0.0, a4, 0.0, 0.0);
	t->Translate(cx, cy);
    }
    else
	t = new iv3(Transformer)(1.0, 0.0, 0.0, 1.0, cx, cy);

    SetTransformer(t);
    Unref(t);
}

//------------------------------------------
// set_scale
//
// Sets the scaling factor for the symbol - always call this after set_position
//------------------------------------------

void graParametricSymbol::set_scale( float sx, float sy )
{
    Initialize (graParametricSymbol::set_scale);

    iv3(Transformer)* t = new iv3(Transformer)(sx, 0.0, 0.0, sy, 0.0, 0.0);

    iv3(Transformer)* local = GetTransformer();
    if(local)
    {
	float a1, a2, a3, a4, a5, a6;
	local->GetEntries(a1, a2, a3, a4, a5, a6);

	t->Translate(a5, a6);
    }

    SetTransformer(t);
    Unref(t);
}

//------------------------------------------
// graParametricSymbol::create_label_box(int)
//
// Create the box to hold a graTextModeInside label
// (filled box and top and bottom lines).
// Should be called only for graTextModeInside -- does not check for validity.
//------------------------------------------

void graParametricSymbol::create_label_box(int mode_no)
{
    Initialize(graParametricSymbol::create_label_box);

    float scale = text_height / 13.0;
    Picture* pp = get_pictel(mode_no);
    if (!pp)
        pp = get_pictel(0);
    Graphic* left_half = pp->First();
    Graphic* right_half = pp->Last();
    if (!left_half || !right_half || left_half == right_half)
        return;
    Graphic* colorpic = ((Picture*) right_half)->First();
    PColor* bg = colorpic->GetBgColor();
    PColor* fg = colorpic->GetFgColor();
    PBrush* brush = colorpic->GetBrush();
    int line_width = brush->Width();
    iv2(Coord) llx, lly, urx, ury;
    left_half->GetBox(llx, lly, urx, ury);
    //
    // Note: coords must be adjusted by one to get real endpoints and edges,
    // since bounding box is just outside them.
    //
    llx = urx - (line_width + 1) / 2;
    urx = llx + text_width;
    lly += (line_width + 1) / 2;
    ury -= (line_width + 1) / 2;
    Graphic* graphic = new FillRect(llx, lly, urx, int(scale * ury + 0.5));
    graphic->SetColors(bg, bg);
    //
    // "prepend" background box so label text will overlay it
    //
    param_overlay[mode_no]->Prepend(graphic);

    //
    // Despite what was said above, add one back to the line length to
    // compensate for an apparent clipping bug in InterViews drawing the
    // lines (they end up one pixel short otherwise).
    //
    if (text_mode == graTextModeInside)
    {
	graphic = new Line(llx, lly, urx + 1, lly);
	graphic->SetColors(fg, fg);
	graphic->SetBrush(brush);
    }
    else
	graphic = new Picture;	// empty placeholder for "invisible" line

    param_overlay[mode_no]->Append(graphic);

    if (text_mode == graTextModeInside)
    {
	graphic = new Line(llx, int(scale * ury + 0.5), urx + 1,
			   int(scale * ury + 0.5));
	graphic->SetColors(fg, fg);
	graphic->SetBrush(brush);
    }
    else
	graphic = new Picture;

    param_overlay[mode_no]->Append(graphic);
}

//------------------------------------------
// graParametricSymbol::get_text_width()
//
// Returns actual text width in the label, not the adjusted width after a
// set_text_width() operation.
//------------------------------------------

int graParametricSymbol::get_text_width()
{
    Initialize(graParametricSymbol::get_text_width);

    int ret_val = -1;
    for (int i = 0; i < graLayerNoCursor; i++)
    {
	if (param_overlay[i])
	{
	    Graphic* label;
	    if (text_mode == graTextModeInside ||
		text_mode == graTextModeInsideNoBox)
	    {
		param_overlay[i]->SetCurrent(param_overlay[i]->First());
		label = param_overlay[i]->Next();
	    }
	    else
		label = param_overlay[i];

	    float llx, lly, urx, ury;
	    label->GetBounds(llx, lly, urx, ury);
	    ret_val = int(urx - llx + 0.5);
	    break;
	}
    }
    return ret_val < 0 ? text_width : ret_val;
}

//------------------------------------------
// graParametricSymbol::set_text_width()
//
// Set the text width to the specified amount and regenerate the label box,
// if any.
//------------------------------------------

void graParametricSymbol::set_text_width(int new_width)
{
    Initialize(graParametricSymbol::set_text_width);

    if (new_width != text_width)
    {
	if (text_mode == graTextModeInside ||
	    text_mode == graTextModeInsideNoBox)
	{
	    int diff = (new_width - text_width) / 2;

	    // Translate Cursor since label will be translated
	    if (param_overlay[graLayerNoCursor])
		param_overlay[graLayerNoCursor]->Translate (diff, 0.0);

	    text_width = new_width;
	    for (int i = 0; i < graLayerNoCursor; i++)
	    {
		if (param_overlay[i])
		{
		    Graphic* graphic = param_overlay[i]->First(); // background
		    param_overlay[i]->Remove(graphic);
		    delete graphic;

		    graphic = param_overlay[i]->First(); // label
		    graphic->Translate(diff, 0.0);
		    graphic = param_overlay[i]->Last();	// top line
		    param_overlay[i]->Remove(graphic);
		    delete graphic;

		    graphic = param_overlay[i]->Last();	// bottom line
		    param_overlay[i]->Remove(graphic);
		    delete graphic;

		    create_label_box(i);
		}
	    }
	    set_layers(layers);
	}
	else
	    text_width = new_width;
    }
}

//------------------------------------------
// graParametricSymbol::open_node(graWindow*)
//------------------------------------------

// Opens up this node
void graParametricSymbol::open_node (graWindow*)
{
   Initialize (graParametricSymbol::open_node);

   // Ask view to open itself up
   if (viewnode)
      viewnode->open_node ();
}

//------------------------------------------
// graParametricSymbol::set_char_marker(int)
//------------------------------------------

void graParametricSymbol::set_char_marker (int pos)
{
    Initialize (graParametricSymbol::set_char_marker);

    // validate and set character position
    if (pos > 0)
	char_marker = pos;
}

// Text input handling
enum OPTYPE
{
    OP_UNKNOWN,
    DELETE_PREVIOUS,
    DELETE_NEXT,
    INSERT,
    REPORT,
    FORWARD_CHAR,
    BACKWARD_CHAR,
    PREVIOUS_LINE,
    NEXT_LINE
};

#define DELETE 0x7F
#define BACKSPACE 0x08
#define RETURN '\r'
#define NEWLINE  '\n'

#define CTRL_A  0x01
#define CTRL_B  0x02
#define CTRL_C  0x03
#define CTRL_D  0x04
#define CTRL_E  0x05
#define CTRL_F  0x06
#define CTRL_G  0x07
#define CTRL_H  0x08
#define CTRL_I  0x09
#define CTRL_J  0x0A
#define CTRL_K  0x0B
#define CTRL_L  0x0C
#define CTRL_M  0x0D
#define CTRL_N  0x0E
#define CTRL_O  0x0F
#define CTRL_P  0x10
#define CTRL_Q  0x11
#define CTRL_R  0x12
#define CTRL_S  0x13
#define CTRL_T  0x14
#define CTRL_U  0x15
#define CTRL_V  0x16
#define CTRL_W  0x17
#define CTRL_X  0x18
#define CTRL_Y  0x19
#define CTRL_Z  0x1A

//------------------------------------------
// get_operation(...)
//------------------------------------------

static OPTYPE get_operation (unsigned char* string, int len)
{
    Initialize (get_operation);

    OPTYPE r_val = INSERT;

    if (len == 1) {
	switch (*string)
        {
	  case BACKSPACE:
          case DELETE:
	    r_val = DELETE_PREVIOUS;
	    break;

	  case NEWLINE:
	  case RETURN:
	    r_val = REPORT;
	    break;

	  case CTRL_F:
	    r_val = FORWARD_CHAR;
	    break;

	  case CTRL_B:
	    r_val = BACKWARD_CHAR;
	    break;

	  case CTRL_P:
	    r_val = PREVIOUS_LINE;
	    break;

	  case CTRL_N:
	    r_val = NEXT_LINE;
	    break;

	  case CTRL_D:
	    r_val = DELETE_NEXT;
	    break;
        }
    }
    return r_val;
}

//------------------------------------------
// graParametricSymbol::hilite_modification_text()
//------------------------------------------

// hilites modification text
void graParametricSymbol::hilite_modification_text ()
{
    Initialize (graParametricSymbol::hilite_modification_text);

    if (modified_token)
    {
	if (has_text_focus())
	    hilite_text (modification_start, modification_end);
	else
	{
	    if (get_viewnode())
	    {
		viewGraHeaderPtr vh =
		    checked_cast(viewGraHeader,get_viewnode()->get_header());

		graWindow* gwin = ((gra_ivInterface *)vh->get_gra_interface())->GetWindow();
		if (gwin)
		{
		    gwin->set_text_focus (this);
		    hilite_text (modification_start, modification_end);
		}
	    }
	}
    }
}

//------------------------------------------
// graParametricSymbol::hilite_text(int, int)
//------------------------------------------

// hilites text
void graParametricSymbol::hilite_text (int start_pos, int end_pos)
{
    iv3(Label)* label;

    Initialize (graParametricSymbol::hilite_text);

    init_PFont();
    init_PColor();
    PFont  *my_font = base_font;

    // get text
    ostrstream ostr;
    viewnode->send_string (ostr);
    const int text_len = ostr.pcount();
    ostr << ends;
    const char*const text = ostr.str();
    ostr.rdbuf()->freeze(0);	// Let the ostrstream delete its buffer.

    // validate start and end pos
    if (start_pos >= end_pos ||	start_pos < 0 || end_pos > text_len)
	return;

    // unhilite this text if it is already hilited
    unhilite_text (start_pos + 1);

    // get text to be hilited
    int hilite_text_len;

//  for flowcharts - cut hilite text to the symbol size
    if (text_mode == graTextModePositioned){
        float cx, cy, ww, hh;
	int pix_len = my_font->Width(text); // in pixels
	driver_instance->graphics_world->get_symbol_size (symbol,
				  cx, cy, ww, hh);
	int real_len = strlen(text); // in chars
        hilite_text_len = int((ww - 12) * real_len / pix_len);
    }
    else
        hilite_text_len = end_pos - start_pos;

    char*const hilite_text = new char [hilite_text_len + 1];
    strncpy (hilite_text, text + start_pos, hilite_text_len);
    hilite_text[hilite_text_len] = '\0';

    // create hilite text label from selected param layer and
    // translate it to the appropriate position

    if (text_mode == graTextModeInside || text_mode == graTextModeInsideNoBox)
    {
	param_overlay[graLayerNoSelected]->
	    SetCurrent(param_overlay[graLayerNoSelected]->First());
	label = (iv3(Label)*) param_overlay[graLayerNoSelected]->Next();
    }
    else 
	label = (iv3(Label)*) param_overlay[graLayerNoSelected]->First();

    iv3(Label)* hilite_label = new iv3(Label) (hilite_text, label);
    hilite_label->SetFont (hilite_text_font);
    hilite_label->SetColors (hilite_text_foreground, hilite_text_background);
    if (start_pos > 0) {
	PFont* pf = label->GetFont();
	char* p_text = new char[start_pos + 1];
	strncpy (p_text, text, start_pos);
	p_text[start_pos] = '\0';
	hilite_label->Translate ((float)pf->Width(p_text), 0.0);
	delete [] p_text;
    }
    if (param_overlay [graLayerNoTextHilight] == NULL)
	param_overlay [graLayerNoTextHilight] = new Picture;
    param_overlay [graLayerNoTextHilight]->Append (hilite_label);

    // turn on hilite text layer
    add_layers (graSymbolModeTextHilited);

    // delete text and hilite text
    delete [] hilite_text;
}

//------------------------------------------
// graParametricSymbol::unhilite_all()
//------------------------------------------

void graParametricSymbol::unhilite_all ()
{
    Initialize (graParametricSymbol::unhilite_all);

    int hilited = layers & graSymbolModeTextHilited;

    if (hilited)
	rem_layers (graSymbolModeTextHilited);

    delete_picture_contents(param_overlay[graLayerNoTextHilight]);

}

//------------------------------------------
// graParametricSymbol::unhilite_text(int)
//------------------------------------------

// unhilites hilited text token containing pos
void graParametricSymbol::unhilite_text (int pos)
{
    Initialize (graParametricSymbol::unhilite_text);

    Picture* pic = param_overlay[graLayerNoTextHilight];

    // Something must be hilited here
    if (pic == NULL)
	return;

    // get text for the symbol
    ostrstream ostr;
    viewnode->send_string (ostr);
    const int text_len = ostr.pcount();
    ostr << ends;
    const char*const text = ostr.str();
    ostr.rdbuf()->freeze(0);	// Let the ostrstream delete its buffer.

    // validate position
    if (pos < 0 || pos >= text_len)
	return;

    // get x,y for pos
    int x, y;
    iv3(Label)* label;
    if (text_mode == graTextModeInside || text_mode == graTextModeInsideNoBox)
    {
	param_overlay[graLayerNoSelected]->
	    SetCurrent(param_overlay[graLayerNoSelected]->First());
	label = (iv3(Label)*) param_overlay[graLayerNoSelected]->Next();
    }
    else
	label = (iv3(Label)*) param_overlay[graLayerNoSelected]->First();

    char* p_text = new char [pos + 1];
    strncpy (p_text, text, pos);
    p_text[pos] = '\0';
    x = label->GetFont()->Width(p_text);
    delete [] p_text;

    BoxObj box;
    label->GetBox(box);
    x += box.left;
    y = int((box.bottom + box.top)/2.0);

    // find graphic containing the text
    PointObj pt(x,y);
    for (label = (iv3(Label)*) pic->First();
	 label; label = (iv3(Label)*)pic->Next())
    {
	if (label->Contains(pt))
	    break;
	pic->SetCurrent(label);
    }

    // remove this graphic from picture
    if (label)
    {
	int hilited = layers & graSymbolModeTextHilited;
	if (hilited)
	    rem_layers (graSymbolModeTextHilited);
	pic->Remove(label);
	if (hilited)
	    add_layers (graSymbolModeTextHilited);
    }

    // update screen instance
    add_layers (graSymbolModeTextHilited);
}

//------------------------------------------
// graParametricSymbol::handle_text_input(...)
//------------------------------------------

// Handler for text input.
// returns 1 if text is handled, 0 otherwise.
int graParametricSymbol::handle_text_input (
    unsigned char* string, int len, int, int, int, int)
{
    Initialize (graParametricSymbol::handle_text_input);

    if (text_focus == 0 || modified_token == NULL || len == 0)
	return 0;

    if (editbuffer)
    {
	// unhilite everything
	unhilite_all ();

	// Get operation to perform
	OPTYPE opcode = get_operation (string, len);

	// operate on it
	switch (opcode)
	{
	  case DELETE_PREVIOUS:
	    if (char_marker <= 0)
		break;

	    set_modified_token ();

	    if (char_marker == modification_start && modified_token)
	    {
		if (report_text_update())
		    return 1;

		// Set token to be previous token
		char_marker--;
		set_modified_token();
		char_marker++;
	    }
	    if (editbuffer->Delete(char_marker, -1) == -1)
	    {
		modification_end--;
		shift_cursor_left ();
	    }
	    break;

	  case DELETE_NEXT:
	    if (char_marker >= strlen(buffer))
		break;

	    set_modified_token ();

	    if (modified_token && char_marker == modification_end)
	    {
		if (report_text_update())
		    return 1;

		// Set token to be next token
		char_marker++;
		set_modified_token();
		char_marker--;
	    }

	    if (editbuffer->Delete(char_marker, 1) == 1)
		modification_end--;
	    break;

	  case PREVIOUS_LINE:
	    break;
	  case NEXT_LINE:
	    break;

	  case FORWARD_CHAR:
	    if (char_marker >= strlen(buffer))
		break;

	    if (modified_token && char_marker == modification_end)
	    {
		if (report_text_update())
		    return 1;
	    }
	    shift_cursor_right();
	    set_modified_token();
	    break;

	  case BACKWARD_CHAR:
	    if (char_marker <= 0)
		break;

	    if (modified_token && char_marker == modification_start)
	    {
		if (report_text_update())
		    return 1;
	    }
	    shift_cursor_left();
	    set_modified_token();
	    break;

	  case REPORT:		// Report new text to application
	    report_text_update ();
	    return 1;

	  case INSERT:
	  default:
	    set_modified_token();

	    len = editbuffer->Insert (char_marker, (char*)string, len);

	    while (len--)
	    {
                modification_end++;
		shift_cursor_right();
	    }
	    break;
	}
	// display it fresh
	replace_text ((char*) editbuffer->Text());
    }
    return 1;
}

//------------------------------------------
// graParametricSymbol::replace_text(char*)
//------------------------------------------

void graParametricSymbol::replace_text(char* new_text)
{
    Initialize (graParametricSymbol::replace_text);

    if (text_focus == 0)	// Make sure that node has focus.
	return;

    for(int i = 0; i < GRA_MAX_MODES; ++i)
    {
	Picture* param_pic = param_overlay[i];
	if(param_pic)
	{
	    for(Graphic* cur_graphic = param_pic->First();
		cur_graphic != NULL; cur_graphic = param_pic->Next())
	    {
		if(cur_graphic->IsA(LABEL))
		{
		    Graphic* label_graphics =
			new iv3(Label)(new_text, cur_graphic);

		    param_pic->SetCurrent(cur_graphic);
		    param_pic->InsertBeforeCur(label_graphics);
		    param_pic->RemoveCur();
		    delete cur_graphic;

		    param_pic->SetCurrent(label_graphics);
		}
	    }
        }
    }
    set_layers(int(this->layers));
}

//------------------------------------------
// graParametricSymbol::set_text_focus(int)
//------------------------------------------

void graParametricSymbol::set_text_focus (int p_focus)
{
    Initialize (graParametricSymbol::set_text_focus);


//  return also in flow chart case - no modifications allowed yet
    if (p_focus == text_focus || text_mode == graTextModePositioned)
	return;

    if (p_focus == 1)
    {
	text_focus = 1;

	// Create editbuffer if not created
	if (buffer == NULL)
	{
	    Picture* pic = param_overlay[graLayerNoSelected];
	    Graphic* g = pic ? pic->First() : NULL;
	    while(g  &&  !g->IsA(LABEL))
		g = pic->Next();

	    if (g == NULL)
	    {
		text_focus = 0;
		return;
	    }

	    char* text;
	    ((iv3(Label)*)g)->GetOriginal(text);

	    const int len = strlen(text);
	    const int buf_size = (len + 1024) & ~1023;
	    buffer = new char[buf_size];
	    strcpy(buffer, text);
	    editbuffer = new iv3(TextBuffer)(buffer, len, buf_size);
	}

	// turn on cursor
	// hilite_modification_text();
	add_layers (graSymbolModeCursor);
    }
    else if (buffer)
    {
	// Report change if any
	if (report_text_update())
	    return;

	// turn off cursor
	rem_layers (graSymbolModeCursor);
	text_focus = 0;
    }
}

//------------------------------------------
// graParametricSymbol::shift_cursor_left()
//------------------------------------------

void graParametricSymbol::shift_cursor_left ()
{
    Initialize (graParametricSymbol::shift_cursor_left);

    // Make sure that node has focus
    if (text_focus == 0)
	return;

    // Validate marker position
    if (char_marker <= 0)
	return;

    // Go through all layer graphics looking for label
    int i;
    Picture* pic = NULL;

    for (i = graLayerNoSelected; i >= 0; i--)
    {
	if ( ( (1 << i ) & layers) && param_overlay[i] )
	{
	    pic = param_overlay[i];
	    break;
	}
    }

    if (pic)
    {
	Graphic* g = pic->First();
	while (g)
	{
	    if (g->IsA(LABEL))
	    {
		// Get width of the character to be skipped over
		char temp[2];
		temp[0] = editbuffer->Char(char_marker - 1);
		temp[1] = '\0';
		int width = g->GetFont()->Width(temp);

		// translate marker layer by width
		param_overlay[graLayerNoCursor]->Translate(float(-width), 0.0);
		break;
	    }
	    g = pic->Next();
	}
    }
    char_marker--;
}

//------------------------------------------
// graParametricSymbol::shift_cursor_right()
//------------------------------------------

void graParametricSymbol::shift_cursor_right ()
{
    Initialize (graParametricSymbol::shift_cursor_right);

    if (text_focus == 0)
	return;

    // Validate current char marker
    if (char_marker >= editbuffer->Length())
	return;

    // Go through all layer graphics looking for label
    int i;
    Picture* pic = NULL;

    for (i = graLayerNoSelected; i >= 0; i--)
    {
	if ( ( (1 << i ) & layers) && param_overlay[i] )
	{
	    pic = param_overlay[i];
	    break;
	}
    }

    if (pic)
    {
	Graphic* g = pic->First();
	while (g)
	{
	    if (g->IsA(LABEL))
	    {
		// Get width of the character to be skipped over
		char temp[2];
		temp[0] = editbuffer->Char(char_marker);
		temp[1] = '\0';
		int width = g->GetFont()->Width(temp);

		// translate marker layer by width
		param_overlay[graLayerNoCursor]->Translate(float(width), 0.0);
		break;
	    }
	    g = pic->Next();
	}
    }
    char_marker++;
}

//------------------------------------------
// graParametricSymbol::locate_text_position(int, int)
//------------------------------------------

// Computes index in the text field from the (x,y);
// returns -1, if (x,y) lies outside
int graParametricSymbol::locate_text_position (int x, int y)
{
    Initialize (graParametricSymbol::locate_text_position);

    if (text_focus == 0)
	return -1;

    // Go through all layer graphics looking for label
    int i;
    Picture* pic = NULL;

    for (i = graLayerNoSelected; i >= 0; i--)
    {
	if ( ( (1 << i ) & layers) && param_overlay[i] )
	{
	    pic = param_overlay[i];
	    break;
	}
    }

    if (pic)
    {
	PointObj pt(x, y);

	Graphic** selections;
	int     num_selected = this->GraphicsContaining (pt, selections);
	int     i;
	iv2(Coord)   left, bottom, right, top;
	iv2(Coord)   p_left;
	iv2(Coord)   p_bottom;

	for (i=0; i < num_selected; i++)
	{
	    Graphic* g = selections[i];

	    left = x;
	    bottom = y;

	    // Traverse down the instance hierarchy
	    if (g)
	    {
		while (g->IsA(INSTANCE))
		{
		    g->GetBox(p_left, p_bottom, right, top);
		    left -= p_left;
		    bottom -= p_bottom;
		    ((Instance*)g)->GetOriginal (g);
		}
	    }

	    // Get the element
	    if (g == pic)
	    {
		// set gr to be selected layer
		g = param_overlay[graLayerNoSelected];

		Graphic* l_g = g ? ((Picture*) g)->First() : NULL;

		while (l_g)
		{
		    if (l_g->IsA(LABEL))
		    {
			// check for within
			char* orig_text;
			((iv3(Label)*)l_g)->GetOriginal(orig_text);

			PFont* f = l_g->GetFont();

			int label_width = f->Width(orig_text);

			if (left >= 0 && left <= label_width)
			{
			    g = l_g;
			    break;
			}
		    }
		    ((Picture*)g)->SetCurrent(l_g);
		    l_g = ((Picture*)g)->Next();
		}

		// Compute Position
		if (l_g)
		{
		    // Go through each character checking for position
		    int len = editbuffer->Length();

		    char temp[2];
		    temp[1] = '\0';
		    PFont* f = l_g->GetFont();

		    for (i = 0; i < len; i++)
		    {
			temp[0] = editbuffer->Char(i);

			if (i == 0)
			{
			    // adjust left for better approximation
			    left += f->Width(temp) >> 1;
			}

			if (left  < f->Width(temp))
			    return i;

			left -= f->Width(temp);
		    }
		    return len;
		}
	    }
	}
    }

    // no match reset modified token
    reset_modified_token ();

    return -1;
}

//------------------------------------------
// graParametricSymbol::shift_cursor(int)
//------------------------------------------

// Shifts cursor in the text field to specified position
void graParametricSymbol::shift_cursor (int pos)
{
    Initialize (graParametricSymbol::shift_cursor);

    // verify text focus
    if (text_focus == 0)
	return;

    // Is this a repeat call
    if (pos == char_marker)
	return;

    // Validate cursor position
    if ( (pos >= editbuffer->Length()) || (pos < 0) )
	return;

    // shift right from current position
    while(pos > char_marker)
	shift_cursor_right();

    // shift left from current position
    while (pos < char_marker)
	shift_cursor_left();

    reset_modified_token ();
    set_modified_token();
}

//------------------------------------------
// graParametricSymbol::report_text_update()
//------------------------------------------

// Reports update of text to application
int graParametricSymbol::report_text_update()
{
    Initialize (graParametricSymbol::report_text_update);

    int reported = 0;

    if (text_focus == 0)
	return reported;

    // Get app node
    ldrTreePtr ldrnode = viewnode->get_ldrTree();

    if (ldrnode)
    {
	appTreePtr appnode = ldrnode->get_appTree();

	if (appnode == NULL)
	{
	    ldrnode = checked_cast(ldrTree,ldrnode->get_parent());
	    appnode = ldrnode->get_appTree();
	}

	if (appnode)
	{
	    // Reporting depends on apptype and possibly viewtype
	    app*  apphead = checked_cast(app, appnode->get_header());
	    view* viewhead = checked_cast(view, viewnode->get_header());

	    if (apphead == NULL)
		return reported;

	    switch (apphead->get_type())
	    {
	      case App_SMT:
	      case App_STE:
	      case App_PROJECT:
		if (modified_token)
		{
		    const char*const text = (char*) editbuffer->Text();

		    if (modification_end > modification_start)
		    {
			const int span = modification_end - modification_start;

			ostrstream ostr;
			RelationalPtr(modified_token)->send_string(ostr);
			const int orig_str_len = ostr.pcount();
			ostr << ends;
			const char*const orig_str = ostr.str();
			// Let the ostrstream delete its buffer.
			ostr.rdbuf()->freeze(0);

			if (orig_str_len != span ||
			    strncmp (orig_str, text + modification_start, span))
			{
			    char* new_str = new char[span + 1];
			    strncpy (new_str, text + modification_start, span);
			    new_str[span]='\0';

			    // suspend smt parsing
			    if (apphead->get_type () == App_SMT)
				checked_cast(smtHeader,apphead)->
				    suspend_parsing();

			    obj_insert(
				appnode->get_header(), REPLACE, NULL,
				checked_cast(appTree, modified_token), new_str);

			    reported = 1;

			    delete [] new_str;
			}
		    }
		    else
		    {
			obj_remove (
			    apphead, checked_cast(appTree, modified_token));
			reported = 1;
		    }
		}
		break;

	      case App_RTL:
	      case App_LAST:
		break;
	    }
	}
    }
    return reported;
}

//------------------------------------------
// graParametricSymbol::get_token_to_be_modified()
//------------------------------------------

Obj* graParametricSymbol::get_token_to_be_modified()
{
    Initialize (graParametricSymbol::get_token_to_be_modified);

    Obj* tok = NULL;		// return value of function

    ldrNode* ldrnode = checked_cast(ldrNode,viewnode->get_ldrTree());
    if(ldrnode)
    {
	objArr* array = ldrnode->get_tok_list(1);

	if(array == NULL)
	{
	    ldrnode = checked_cast(ldrNode,ldrnode->get_parent());
	    if(ldrnode)
		array = ldrnode->get_tok_list(1);
	}
	if(array)
	{
	    objArr tokens(*array);
	    delete array;

	    ostrstream ostr;
	    Obj* el;
	    ForEach (el, tokens)
	    {
		RelationalPtr(el)->send_string(ostr);
		if (ostr.pcount() > char_marker)
		{
		    tok = el;
		    break;
		}
	    }
	}
    }
    return tok;
}

//------------------------------------------
// graParametricSymbol::set_modified_token()
//------------------------------------------

void graParametricSymbol::set_modified_token()
{
    Initialize (graParametricSymbol::set_modified_token);

    if (modified_token == NULL)
    {
	Obj* token_to_be_modified = get_token_to_be_modified();

	modified_token = token_to_be_modified;

	set_modification_start_end_pos (modified_token);
    }

    // if modified token is not found, turn cursor off
    if (modified_token == NULL)
	rem_layers (graSymbolModeCursor);
}

//------------------------------------------
// graParametricSymbol::set_modification_start_end_pos(Obj*)
//------------------------------------------
// Sets start position based on position in token

void graParametricSymbol::set_modification_start_end_pos(Obj* modified_token)
{
    Initialize (graParametricSymbol::set_modification_start_end_pos);

    if (modified_token)
    {
	ldrNode* ldrnode = checked_cast(ldrNode,viewnode->get_ldrTree());
	if (ldrnode)
	{
	    objArr* array = ldrnode->get_tok_list(1);
	    if(array)
	    {
		objArr tokens(*array);
		delete array;

		ostrstream ostr;
		int start_pos = 0;
		Obj* el;
		ForEach (el, tokens)
		{
		    // Append token to output string.
		    RelationalPtr(el)->send_string(ostr);

		    // Length of output string up to now.
		    const int end_pos = ostr.pcount();

		    if (el == modified_token)
		    {
			modification_start = start_pos;
			modification_end = end_pos;
			break;
		    }
		    start_pos = end_pos;
		}
	    }
	}
    }
}

//------------------------------------------
// graParametricSymbol::reset_modified_token()
//------------------------------------------

void graParametricSymbol::reset_modified_token()
{
    Initialize (graParametricSymbol::reset_modified_token);

    modified_token = NULL;
    modification_start = modification_end = -1;
}

//------------------------------------------
// graParametricSymbol::generate_popup()
//------------------------------------------

NodePopupInfo *graParametricSymbol::generate_popup()
{
    Initialize (graParametricSymbol::generate_popup);

    return viewnode->generate_popup_menu();
}

//------------------------------------------
// graParametricSymbol::set_popup_item_sensitive(...)
//------------------------------------------

void graParametricSymbol::set_popup_item_sensitive(char* itemname, int val)
{
    Initialize (graParametricSymbol::set_popup_item_sensitive);

    if (this->popup_menu != NULL)
	ui_set_child_sensitive(this->popup_menu, itemname, val);
}

//------------------------------------------
// graParametricSymbol::add_bottom_line()
//------------------------------------------

void graParametricSymbol::add_bottom_line()
{
    Initialize(graParametricSymbol::add_bottom_line);

    if (text_mode != graTextModeInsideNoBox)
	return;

    Picture* pp = get_pictel(0);
    Graphic* left_half = pp->First();
    Graphic* right_half = pp->Last();
    Graphic* colorpic = ((Picture*) right_half)->First();
    if (!colorpic)
	return;

    iv2(Coord) llx, lly, urx, ury;
    left_half->GetBox(llx, lly, urx, ury);
    float len = urx - llx - 2.0;
    right_half->GetBox(llx, lly, urx, ury);
    len += urx - llx - 2.0;
    len += text_width;
    Graphic* line = new Line(0, 0, iv2(Coord)(len), 0);
    line->SetBrush(colorpic->GetBrush());
    line->SetColors(colorpic->GetFgColor(), colorpic->GetBgColor());
    Append(line);

    bottom_line = line;
}

//------------------------------------------
// graParametricSymbol::create_false_bottom()
//------------------------------------------

graParametricSymbol* graParametricSymbol::create_false_bottom()
{
    Initialize(graParametricSymbol::create_false_bottom);

    if (text_mode != graTextModeInside)
	return NULL;

    Picture* pp = get_pictel(0);
    Graphic* left_half = pp->First();
    Graphic* right_half = pp->Last();
    Graphic* colorpic = ((Picture*) right_half)->First();
    if (!colorpic)
	return NULL;

    PColor* fg = colorpic->GetFgColor();
    PColor* bg = colorpic->GetBgColor();
    PBrush* brush = colorpic->GetBrush();
    iv2(Coord) llx, lly, urx, ury;
    left_half->GetBox(llx, lly, urx, ury);
    float len = urx - llx - 2;
    right_half->GetBox(llx, lly, urx, ury);
    len += urx - llx - 2;
    len += text_width;
    iv2(Coord) left_x = 0;

    graParametricSymbol* fb = new graParametricSymbol(-1, 0);
    Graphic* line = new Line(left_x, 0, iv2(Coord)(len), 0);
    line->SetBrush(brush);
    line->SetColors(fg, bg);
    fb->Append(line);

    line = new Line(left_x, FALSE_BOTTOM_HEIGHT, left_x, 0);
    line->SetBrush(brush);
    line->SetColors(fg, bg);
    fb->Append(line);

    line = new Line(iv2(Coord)(len), FALSE_BOTTOM_HEIGHT, iv2(Coord)(len), 0);
    line->SetBrush(brush);
    line->SetColors(fg, bg);
    fb->Append(line);

    Append(fb);
    if (false_bottom1)
	false_bottom2 = fb;
    else
	false_bottom1 = fb;

    return fb;
}

//------------------------------------------
// graParametricSymbol::remove_false_bottom(graParametricSymbol*)
//------------------------------------------

void graParametricSymbol::remove_false_bottom(graParametricSymbol* fb)
{
   Initialize(graParametricSymbol::remove_false_bottom);

   Remove(fb);

   if (false_bottom1 == fb)
   {
      false_bottom1 = false_bottom2;
      false_bottom2 = NULL;
   }
   else if (false_bottom2 == fb)
      false_bottom2 = NULL;
}


//------------------------------------------
// graParametricSymbol::do_highlight(...)
//------------------------------------------

void graParametricSymbol::do_highlight(
    iv3(Canvas)* cnvs, rubberGraphic* xor_box)
{
   Initialize(graParametricSymbol::do_highlight);

   xor_box->SetCanvas(cnvs);
   xor_box->SetGraphic(this);
   xor_box->Redraw();
}


//------------------------------------------
// graParametricSymbol::undo_highlight(iv3(Canvas)*)
//------------------------------------------

void graParametricSymbol::undo_highlight(iv3(Canvas)*) {}


void graParametricSymbol::remove_bottom_line()
{
    Initialize(graParametricSymbol::remove_bottom_line);

    if (bottom_line)
    {
 	Remove(bottom_line);
 	delete bottom_line;
 	bottom_line = NULL;
    }
}


//------------------------------------------
// class logical_segment
//------------------------------------------

struct logical_segment
{
    logical_segment() : vn(NULL), decoration(NOTHING), label(NULL),
			centered(false), above(true) {}
    ~logical_segment() { delete [] label; }

    objArr_Int		vert_x;
    objArr_Int		vert_y;
    viewSymbolNodePtr	vn;
    deco_type		decoration;
    char*		label;
    bool		centered : 1;
    bool		above : 1;
};


//------------------------------------------
// graFlexConn::graFlexConn(viewConnectionNodePtr)
//------------------------------------------

graFlexConn::graFlexConn(viewConnectionNodePtr vn): is_highlighted(false)
{
    Initialize(graFlexConn::graFlexConn);

    num_log_segs = vn->inputs->size() + vn->outputs->size() + 1;
    log_segs = new logical_segment[num_log_segs];
    int lsegno = 0;
    int i;
    for (i = 0; i < vn->inputs->size(); i++)
	log_segs[lsegno++].vn = checked_cast(viewSymbolNode,(*vn->inputs)[i]);
    for (i = 0; i < vn->outputs->size(); i++)
	log_segs[lsegno++].vn = checked_cast(viewSymbolNode,(*vn->outputs)[i]);
}

//------------------------------------------
// graFlexConn::~graFlexConn()
//------------------------------------------

graFlexConn::~graFlexConn()
{
    delete [] log_segs;
}

//------------------------------------------
// graFlexConn::find_insertion(...)
//------------------------------------------

void graFlexConn::find_insertion(
    graWindow*, Graphic** selected, int num_selected, viewSelection* selection)
{
    Initialize(graFlexConn::find_insertion);

    selection->sub_type = 0;

    for (int i = 0; i < num_segments; i++) {
	for (int j = 0; j < num_selected; j++) {
	    if (segments[i].line == selected[j]) {
		if ((selection->view_node = segments[i].viewnode) != 0)
		    selection->connector = NULL;
		else
		    selection->connector = get_viewnode();
		return;
	    }
	}
    }
    selection->view_node = NULL;
    selection->connector = get_viewnode();
}

//------------------------------------------
// graFlexConn::regenerate(viewConnectionNode&)
//------------------------------------------

void graFlexConn::regenerate(viewConnectionNode& vcn)
{
    Initialize(graFlexConn::regenerate);

    init_PBrush();
    init_PColor();

    viewERDConnectionNode& vn = (viewERDConnectionNode&)vcn;
    const char* fg_name = vn.color_resource();
    const char* fg_value = (fg_name ? GET_VALUE(fg_name, DFLT_FORE) : NULL);
    PColor* fg = (fg_value ? lookup_PColor(fg_value) : connection_foreground);
    PColor* bg = connection_background;

    // Delete old line segments.
    if(num_segments)
    {
	if(get_viewnode())
	{
	    viewGraHeaderPtr vh =
		checked_cast(viewGraHeader,get_viewnode()->get_header());
	    if(vh)
		vh->get_gra_interface()->DamageArea(this);
	}
	delete_picture_contents(this);
	delete [] segments;
	segments = NULL;
	num_segments = 0;
    }

    // Calculate number of physical segments required
    int lsegno;
    for (lsegno = 0; lsegno < num_log_segs; lsegno++)
    {
	if (log_segs[lsegno].vert_x.size())
	    num_segments += log_segs[lsegno].vert_x.size() - 1;
    }

    segments = new _segments[num_segments];

// Draw physical segments, decorations, and labels.  For each segment, we
// draw both the line and an "alleyway" of background color, causing "tunnels"
// instead of apparent intersections.  We draw all the alleyways first, then
// the lines, so that there is no danger of overwriting a previously-drawn
// line with an alleyway.

    for (lsegno = 0; lsegno < num_log_segs; lsegno++)
    {
	if (log_segs[lsegno].vert_x.size())
	{
	    for (int i = 0; i < log_segs[lsegno].vert_x.size() - 1; i++)
	    {
		int x0 = log_segs[lsegno].vert_x[i];
		int y0 = log_segs[lsegno].vert_y[i];
		int x1 = log_segs[lsegno].vert_x[i + 1];
		int y1 = log_segs[lsegno].vert_y[i + 1];
		Graphic* g = new Line(x0, y0, x1, y1);
		g->SetBrush(pbrush5);
		g->SetColors(bg, fg);
		Append(g);
	    }
	}
    }
    int psegno = 0;
    bool arrows = vn.do_arrows();
    int num_inputs = vn.get_inputs()->size();
    for (lsegno = 0; lsegno < num_log_segs; lsegno++)
    {
	if (log_segs[lsegno].vert_x.size())
	{
	    for (int i = 0; i < log_segs[lsegno].vert_x.size() - 1; i++)
	    {
		int x0 = log_segs[lsegno].vert_x[i];
		int y0 = log_segs[lsegno].vert_y[i];
		int x1 = log_segs[lsegno].vert_x[i + 1];
		int y1 = log_segs[lsegno].vert_y[i + 1];
		segments[psegno].line = new Line(x0, y0, x1, y1);
		segments[psegno].line->SetBrush(pbrush0);
		segments[psegno].line->SetColors(fg, bg);
		Append(segments[psegno].line);
		segments[psegno].viewnode = NULL;
		psegno++;
	    }
	    add_decoration(log_segs[lsegno], fg, bg, arrows && lsegno !=
			   num_log_segs - 1, lsegno < num_inputs);

	}
    }
}


//------------------------------------------
// graFlexConn::do_highlight(...)
//------------------------------------------

void graFlexConn::do_highlight(iv3(Canvas)* cnvs, rubberGraphic*)
{
    Initialize(graFlexConn::do_highlight);

    init_PBrush();
    init_PColor();

    if (!is_highlighted)
    {
	is_highlighted = true;
	for (int i = 0; i < num_segments; i++)
	{
	    segments[i].line->SetColors(
		hilite_connection_foreground, connection_background);
	    segments[i].line->SetBrush(pbrush3);
	}
	Draw(cnvs);
    }
}


//------------------------------------------
// graFlexConn::undo_highlight(iv3(Canvas)*)
//------------------------------------------

void graFlexConn::undo_highlight(iv3(Canvas)* cnvs)
{
    Initialize(graFlexConn::undo_highlight);

    init_PBrush();
    init_PColor();

    const char* fg_name = NULL;
    if(get_viewnode())
	fg_name = checked_cast(viewERDConnectionNode,get_viewnode())->color_resource();

    const char* fg_value = (fg_name ? GET_VALUE(fg_name, DFLT_FORE) : NULL);
    PColor* fg = (fg_value ? lookup_PColor(fg_value) : connection_foreground);
    PColor* bg = connection_background;

    for (int i = 0; i < num_segments; i++)
    {
	segments[i].line->SetColors(fg, bg);
	segments[i].line->SetBrush(pbrush0);
    }
    Draw(cnvs);
    is_highlighted = false;
}

//------------------------------------------
// graFlexConn::add_vertex(int, int, int)
//------------------------------------------

void graFlexConn::add_vertex(int lsegno, int x, int y)
{
    Initialize(graFlexConn::add_vertex);

    if (lsegno < num_log_segs) {
	log_segs[lsegno].vert_x.insert_last(x);
	log_segs[lsegno].vert_y.insert_last(y);
    }
}

//------------------------------------------
// graFlexConn::set_decoration(int, deco_type)
//------------------------------------------

void graFlexConn::set_decoration(int lsegno, deco_type decoration)
{
    Initialize(graFlexConn::set_decoration);

    if (lsegno < num_log_segs) {
	log_segs[lsegno].decoration = decoration;
    }
}

//------------------------------------------
// graFlexConn::set_label(int, const char*)
//------------------------------------------

void graFlexConn::set_label(int lsegno, const char* text)
{
    Initialize(graFlexConn::set_label);

    if (lsegno < num_log_segs)
    {
	delete log_segs[lsegno].label;
	log_segs[lsegno].label = new char[strlen(text) + 1];
	strcpy(log_segs[lsegno].label, text);
    }
}


//------------------------------------------
// graFlexConn::set_label_loc(int, bool, bool)
//------------------------------------------

void graFlexConn::set_label_loc(int lsegno, bool centered, bool above)
{
    Initialize(graFlexConn::set_label_loc);

    log_segs[lsegno].centered = centered;
    log_segs[lsegno].above = above;
}

//------------------------------------------
// graFlexConn::add_decoration(...)
//------------------------------------------

void graFlexConn::add_decoration(
    logical_segment& seg, PColor* fg, PColor* bg, bool do_arrow, bool is_input)
{
    Initialize(graFlexConn::add_decoration);

    init_PBrush();

    int x0 = seg.vert_x[0];
    int y0 = seg.vert_y[0];
    int x1 = seg.vert_x[1];
    int y1 = seg.vert_y[1];

    int xsign = (x0 <= x1) ? 1 : -1;
    int ysign = (y0 <= y1) ? 1 : -1;
    bool vertical = (x0 == x1);

    Graphic* gr;
    if (do_arrow)
    {
	float pointx;
	float pointy;
	arrow_type type;
	if (vertical)
	{
	    pointx = x1;
	    if (y0 < y1)	// goes upward
	    {
		if (is_input)
		{
		    pointy = y1 - 6.0;
		    type = ARROW_UP;
		}
		else
		{
		    pointy = y1 - 16.0;
		    type = ARROW_DOWN;
		}
	    }
	    else		// goes downward
	    {
		if (is_input)
		{
		    pointy = y1 + 6.0;
		    type = ARROW_DOWN;
		}
		else
		{
		    pointy = y1 + 16.0;
		    type = ARROW_UP;
		}
	    }
	}
	else			// horizontal
	{
	    pointy = y1;
	    if (x0 < x1)	// goes right
	    {
		if (is_input)
		{
		    pointx = x1 - 6.0;
		    type = ARROW_RIGHT;
		}
		else
		{
		    pointx = x1 - 16.0;
		    type = ARROW_LEFT;
		}
	    }
	    else		// goes left
	    {
		if (is_input)
		{
		    pointx = x1 + 6.0;
		    type = ARROW_LEFT;
		}
		else
		{
		    pointx = x1 + 16.0;
		    type = ARROW_RIGHT;
		}
	    }
	}
	double alen = GET_VALUE(ERD_ARROWHEAD_LENGTH, 9.0);
	double awid = GET_VALUE(ERD_ARROWHEAD_WIDTH, 4.0);
	gr = gra_arrow_head(type, pointx, pointy, alen, awid);
	gr->SetColors(fg, bg);
	Append(gr);
    }

    switch (seg.decoration)
    {
      case NOTHING:
	break;

      case SOLID_DOT:
	if (vertical)
	    y0 += 5 * ysign;
	else x0 += 5 * xsign;
	gr = new FillCircle(x0, y0, 5);
	gr->SetColors(fg, bg);
	Append(gr);
	break;

      case HOLLOW_DOT:
	if (vertical)
	    y0 += 5 * ysign;
	else x0 += 5 * xsign;
	gr = new FillCircle(x0, y0, 5);
	gr->SetColors(bg, bg);
	Append(gr);
	gr = new Circle(x0, y0, 5);
	gr->SetBrush(pbrush1);
	gr->SetColors(fg, bg);
	Append(gr);
	break;

      case DOUBLE_ARROW:
	if (vertical)
	{
	    gr = new Line(x0, y0 + 6 * ysign, x0 - 5, y0 + 16 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0, y0 + 6 * ysign, x0 + 5, y0 + 16 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	else
	{
	    gr = new Line(x0 + 6 * xsign, y0, x0 + 16 * xsign, y0 - 5);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0 + 6 * xsign, y0, x0 + 16 * xsign, y0 + 5);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	/* FALLTHROUGH */

      case SINGLE_ARROW:
	if (vertical)
	{
	    gr = new Line(x0, y0, x0 - 5, y0 + 10 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0, y0, x0 + 5, y0 + 10 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	else
	{
	    gr = new Line(x0, y0, x0 + 10 * xsign, y0 - 5);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0, y0, x0 + 10 * xsign, y0 + 5);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	break;

      case DOUBLE_BAR:
	if (vertical)
	    gr = new Line(x0 - 6, y0 + 12 * ysign, x0 + 6, y0 + 12 * ysign);
	else gr = new Line(x0 + 12 * xsign, y0 - 6, x0 + 12 * xsign, y0 + 6);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	/* FALLTHROUGH */

      case SINGLE_BAR:
	if (vertical)
	    gr = new Line(x0 - 6, y0 + 8 * ysign, x0 + 6, y0 + 8 * ysign);
	else gr = new Line(x0 + 8 * xsign, y0 - 6, x0 + 8 * xsign, y0 + 6);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	break;

      case TRIANGLE:
	{
	    iv2(Coord) x[3];
	    iv2(Coord) y[3];
	    if (vertical)
	    {
		x[0] = x0 - 6;
		x[1] = x0;
		x[2] = x0 + 6;
		y[0] = y[2] = y0;
		y[1] = y0 + 12 * ysign;
	    }
	    else
	    {
		x[0] = x[2] = x0;
		x[1] = x0 + 12 * xsign;
		y[0] = y0 - 6;
		y[1] = y0;
		y[2] = y0 + 6;
	    }
	    gr = new FillPolygon(x, y, 3);
	    gr->SetColors(bg, bg);
	    Append(gr);
	    gr = new Polygon(x, y, 3);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	break;

      case SEMICIRCLE:
	{
	    iv2(Coord) x[5];
	    iv2(Coord) y[5];
	    if (vertical)
	    {
		x[0] = x0 - 6;
		x[1] = x0 - 5;
		x[2] = x0;
		x[3] = x0 + 5;
		x[4] = x0 + 6;
		y[0] = y[4] = y0;
		y[1] = y[3] = y0 + 4 * ysign;
		y[2] = y0 + 7 * ysign;
	    }
	    else
	    {
		x[0] = x[4] = x0;
		x[1] = x[3] = x0 + 4 * xsign;
		x[2] = x0 + 7 * ysign;
		y[0] = y0 - 6;
		y[1] = y0 - 5;
		y[2] = y0;
		y[3] = y0 + 5;
		y[4] = y0 + 6;
	    }
	    gr = new FillBSpline(x, y, 5);
	    gr->SetColors(bg, bg);
	    Append(gr);
	    gr = new BSpline(x, y, 5);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x[0], y[0], x[4], y[4]);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	break;

      case DIAMOND:
	{
	    int x[4];
	    int y[4];
	    if (vertical)
	    {
		x[0] = x[2] = x0;
		x[1] = x0 - 4;
		x[3] = x0 + 4;
		y[0] = y0;
		y[1] = y[3] = y0 + 4 * ysign;
		y[2] = y0 + 8 * ysign;
	    }
	    else
	    {
		x[0] = x0;
		x[1] = x[3] = x0 + 4 * xsign;
		x[2] = x0 + 8 * xsign;
		y[0] = y[2] = y0;
		y[1] = y0 - 4;
		y[3] = y0 + 4;
	    }
	    gr = new FillPolygon(x, y, 3);
	    gr->SetColors(bg, bg);
	    Append(gr);
	    gr = new Polygon(x, y, 3);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	break;

      case BAR_CHICKENFOOT:
	if (vertical)
	    gr = new Line(x0 - 4, y0 + 8 * ysign, x0 + 4, y0 + 8 * ysign);
	else gr = new Line(x0 + 8 * xsign, y0 - 4, x0 + 8 * xsign, y0 + 4);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	/* FALLTHROUGH */

      case CHICKENFOOT:
	if (vertical)
	{
	    gr = new Line(x0 - 4, y0, x0, y0 + 8 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0 + 4, y0, x0, y0 + 8 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	else
	{
	    gr = new Line(x0, y0 - 4, x0 + 8 * xsign, y0);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0, y0 + 4, x0 + 8 * xsign, y0);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	}
	break;

      case CIRCLE_BAR:
	if (vertical)
	    gr = new Line(x0 - 6, y0 + 8 * ysign, x0 + 6, y0 + 8 * ysign);
	else gr = new Line(x0 + 8 * xsign, y0 - 6, x0 + 8 * xsign, y0 + 6);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	if (vertical)
	    y0 += 14 * ysign;
	else x0 += 14 * xsign;
	gr = new FillCircle(x0, y0, 6);
	gr->SetColors(bg, bg);
	Append(gr);
	gr = new Circle(x0, y0, 6);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	break;

      case CIRCLE_CHICKENFOOT:
	if (vertical)
	{
	    gr = new Line(x0 - 4, y0, x0, y0 + 8 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0 + 4, y0, x0, y0 + 8 * ysign);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    y0 += 14 * ysign;
	}
	else
	{
	    gr = new Line(x0, y0 - 4, x0 + 8 * xsign, y0);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    gr = new Line(x0, y0 + 4, x0 + 8 * xsign, y0);
	    gr->SetBrush(pbrush0);
	    gr->SetColors(fg, bg);
	    Append(gr);
	    x0 += 14 * xsign;
	}
	gr = new FillCircle(x0, y0, 6);
	gr->SetColors(bg, bg);
	Append(gr);
	gr = new Circle(x0, y0, 6);
	gr->SetBrush(pbrush0);
	gr->SetColors(fg, bg);
	Append(gr);
	break;
    }
}

//------------------------------------------
// graFlexConn::add_label(logical_segment&)
//------------------------------------------

void graFlexConn::add_label(logical_segment& seg)
{
    Initialize(graFlexConn::add_label);

    if (!seg.label)
	return;

    init_PFont();
    init_PColor();

    PFont* _font = base_font;
    PColor* _fg = base_foreground;
    PColor* _bg = base_background;
    int num_lines = 0;
    float max_len = 0;
    Picture* pic = new Picture;
    iv3(Label)* lab;
    float llx, lly, urx, ury;
    const char* wd = seg.label;
    const char* spc;
    do
    {
	spc = strchr(wd, ' ');
	if (!spc)
	    spc = strchr(wd, 0);
	lab = new iv3(Label)(wd, spc - wd);
	lab->SetFont(_font);
	lab->FillBg(FILLBG_YES);
	lab->SetColors(_fg, _bg);
	pic->Append(lab);
	num_lines++;
	float len = _font->Width(wd, spc - wd);
	max_len = (len > max_len) ? len : max_len;
	for (wd = spc; *wd == ' '; wd++) ;
    }
    while (*wd);

    bool ljust = false;
    bool rjust = false;
    float xdiff, ydiff;
    if (seg.vert_x[0] < seg.vert_x[1])
    {
	if (seg.centered)
	    xdiff = (seg.vert_x[1] - seg.vert_x[0] - max_len) / 2.0;
	else
	{
	    ljust = true;
	    xdiff = 5.0;
	}
	if (seg.above)
	    ydiff = 10.0;
	else
	    ydiff = -10.0 - num_lines * _font->Height();
    }
    else if (seg.vert_x[1] < seg.vert_x[0])
    {
	if (seg.centered)
	    xdiff = -max_len - (seg.vert_x[0] - seg.vert_x[1] - max_len) / 2.0;
	else
	{
	    rjust = true;
	    xdiff = -max_len - 5.0;
	}
	if (seg.above)
	    ydiff = 10.0;
	else
	    ydiff = -10.0 - num_lines * _font->Height();
    }
    else if (seg.vert_y[0] < seg.vert_y[1])
    {
	if (seg.centered)
	    ydiff = (seg.vert_y[1] - seg.vert_y[0] -
		     num_lines * _font->Height()) / 2.0;
	else
	    ydiff = 5.0;
	if (seg.above)
	{
	    ljust = true;
	    xdiff = 10.0;
	}
	else
	{
	    rjust = true;
	    xdiff = -10.0 - max_len;
	}
    }
    else
    {
	if (seg.centered)
	    ydiff = -(num_lines * _font->Height()) -
		(seg.vert_y[0] - seg.vert_y[1] - num_lines * _font->Height()) /
		2.0;
	else
	    ydiff = -5.0 - num_lines * _font->Height();
	if (seg.above)
	{
	    ljust = true;
	    xdiff = 10.0;
	}
	else
	{
	    rjust = true;
	    xdiff = -10.0 - max_len;
	}
    }
    pic->Translate(seg.vert_x[0] + xdiff, seg.vert_y[0] + ydiff);

    int i;
    for (i = 0, lab = (iv3(Label)*) pic->First();
	 lab;
	 i++, lab = (iv3(Label)*) pic->Next())
    {
	if (ljust)
	    lab->Translate(0, _font->Height() * (num_lines - i - 1));
	else
	{
	    lab->GetBounds(llx, lly, urx, ury);
	    float len = urx - llx;
	    if (rjust)
		lab->Translate(max_len - len,
			       _font->Height() * (num_lines - i - 1));
	    else
		lab->Translate((max_len - len) / 2,
			       _font->Height() * (num_lines - i - 1));
	}
    }

    Append(pic);
}


/*
// $Log: graSymbol.h.C  $
// Revision 1.12 2002/04/10 18:59:30EDT andrey 
// Merging from 'Boolean_3_States' branch.
// Revision 1.2.1.16  1994/01/13  01:12:22  kws
// Use psetmalloc
//
// Revision 1.2.1.15  1993/08/27  00:13:34  jon
// bug bug 3642 - Added extra widget as parent to popup menus and deleted the
// widget when the menu is unmapped to avoid a server grab the next time b
// button 3 is pressed.
//
// Revision 1.2.1.14  1993/08/15  23:19:51  sergey
// Modified graParametricSymbol constructor, hilite_text(), set_text_focus(). Fixed bug #4091.
//
// Revision 1.2.1.13  1993/08/10  22:47:50  sergey
// Added SetColors in gra_arrow_head(). Part of bug #3720.
//
// Revision 1.2.1.12  1993/07/09  18:47:09  wmm
// Fix bug 3836.
//
// Revision 1.2.1.11  1993/06/21  20:38:03  boris
// Fixed bug #2313 with unreasonable blue color in diagram
//
// Revision 1.2.1.10  1993/04/26  18:02:32  glenn
// Plug lots of memory leaks, especially when allocating
// InterViews PColors and PBrushes.
// Add dtors for graConnectionSymbol and graAttachableSymbol.
//
// Revision 1.2.1.9  1993/02/26  00:26:38  glenn
// Handle cases where X resource manager database returns NULL.
//
// Revision 1.2.1.8  1993/01/03  16:22:41  kws
// Change UNKNOWN to OP_UNKNOWN to fix multiple defiend enum error
//
// Revision 1.2.1.7  1992/11/23  18:12:55  wmm
// typesafe casts.
//
// Revision 1.2.1.6  1992/11/09  19:30:20  sergey
// Move text_focus = 0 in graParametricSymbol constructor before return stmt.
//
// Revision 1.2.1.5  1992/11/06  22:51:55  so
// Create static variables for PFont and PColor such that
// it creates only once for each object. Before it always
// calls "new PFont (GET_VALUE (BASE_FONT))" to get the base_font.
// It calls "new PFont (GET_VALUE (BASE_FONT))" only once now and save the values
// to be used in the future.
//
// Revision 1.2.1.4  1992/10/28  17:26:28  oak
// Fixed hilighting error such that
// previous selection is not also
// hilighted.
//
// Revision 1.2.1.3  92/10/28  14:35:45  oak
// Fixed read and write error to freed memory.
//
// Revision 1.106  92/10/05  17:42:10  smit
// suspend parsing in hierarchy mode.
//
// Revision 1.105  92/10/02  10:24:59  kws
// Fix crash during update of text of a deleted node
//
// Revision 1.104  92/10/01  13:32:45  jon
// Bug #422
//
// Revision 1.103  92/09/23  15:30:45  jon
// Added member function get_layers
//
// Revision 1.102  92/09/14  17:00:56  sergey
// Corrected offset for call tree connectors. Fixed bug #1280.
//
// Revision 1.101  92/09/11  14:29:37  sergey
// Made Call Tree diagram denser. Fixed bug #1210.
//
// Revision 1.100  92/09/08  09:36:57  oak
// Initialized text_focus in the graParametricSymbol
// constructor to zero (FALSE) to eliminate an
// invalid use of memory error. (Should fix bug 1009)
//
// Revision 1.99  92/08/26  09:26:01  jon
// Added method generate_popup and removed method context_popup
// Bug #639
//
// Revision 1.98  92/07/31  13:46:27  sergey
// Fixed bug in placing ldrcTree connector in multiple output case.
//
// Revision 1.97  92/07/29  23:45:10  jon
// InterViews scope fix
//
// Revision 1.96  92/07/21  18:52:43  smit
// Fix bug#421 and many more potential problems.
//
// Revision 1.95  92/07/15  09:50:09  wmm
// Fix bug introduced in previous submission.
//
// Revision 1.94  92/07/15  09:15:11  wmm
// Fix bug #323.
//
// Revision 1.93  92/07/06  18:48:28  wmm
// Support thick lines for class symbols, per PN suggestion.
//
// Revision 1.92  92/06/26  17:33:50  wmm
// Fix bug which sometimes crashed when a connector was created
// for symbols whose pictures had not yet been created.
//
// Revision 1.91  92/06/25  20:31:55  aharlap
// moved enum decorate_type outside class for GNU
//
// Revision 1.90  92/06/17  15:33:32  glenn
// Use ostr.rdbuf()->freeze(0) to hand responsibility for deleting
// char buffers back to ostrstream.  Use ostr.pcount() to avoid calls
// to strlen().
//
// Revision 1.89  92/06/12  18:44:14  sergey
// Implemented horizontal Call Tree.
//
// Revision 1.88  92/06/11  15:20:35  jont
// added remove_bottom_line
//
// Revision 1.87  92/06/05  16:33:42  sergey
// Added ldrcUpP_TREE connectors to support upper part of Call Tree.
// Added hcoords x-ascending sort for Call Tree Connectors. Added enums to attach[] array fixing a bug.
//
// Revision 1.86  92/06/05  11:09:29  wmm
// Fix crash in switching to modified ERD by making false bottoms part
// of the picture to which they belong (will have to fix suboptimal
// highlighting later).
//
// Revision 1.85  92/06/04  09:29:30  sergey
// Added hcoords x-ascending sort for Call Tree Connectors.
// Added missing ERD.. enums to attach[] array fixing a bug.
//
// Revision 1.84  92/05/28  10:20:33  sergey
// Minor clean up.
//
// Revision 1.83  92/05/27  19:50:28  sergey
// Returned to thin connectors.
//
// Revision 1.82  92/05/27  18:05:50  sergey
// *** empty log message ***
//
// Revision 1.81  92/05/22  10:07:24  smit
// Fix text selection problems.
//
// Revision 1.80  92/05/21  19:09:20  smit
// fix multiple clicking problem in symbols.
//
// Revision 1.79  92/05/20  16:55:25  mfurman
// Fixed bug - call "delete" for graParametricSymbol twice when changing view
//
// Revision 1.78  92/05/18  14:27:08  smit
// Use Michael Furmans fix to get_tok_list.
//
// Revision 1.77  92/05/15  12:45:08  jont
// call generate_popup each time we need to pop up a menu,
// instead of re-using the same one.
//
// Revision 1.76  92/05/08  08:24:20  wmm
// Keep track of whether a graFlexConn is highlighted or not, to allow for
// efficiency when a symbol is repeatedly highlighted by graWorld::flush_repair.
//
// Revision 1.75  92/05/05  18:30:36  kws
// Allow Project/Module names to be editable in graphics
//
// Revision 1.74  92/04/30  17:24:37  sergey
// Changed ldrcTree connector shape; support of MultiNode collapse.
//
// Revision 1.73  92/04/17  11:24:18  wmm
// Add arrows and color coding to ERD connectors.
//
// Revision 1.72  92/04/16  08:23:49  glenn
// Fix problems with include files.
//
// Revision 1.71  92/04/15  10:07:40  wmm
// Fix problem in which graFlexConn highlighting still occasionally resulted in
// use of the rubber band box.
//
// Revision 1.70  92/04/15  08:04:11  wmm
// Implement ability to move graphical symbols; fix minor glitch in painting
// and highlighting graFlexConn connectors.
//
// Revision 1.69  92/04/09  18:52:33  wmm
// Do special highlighting for graFlexConn connectors; do not add labels to
// graFlexConn connectors.
//
// Revision 1.68  92/04/08  13:00:37  smit
// Added ends after send_string is done.
//
// Revision 1.67  92/04/06  18:59:18  sergey
// Fixed the rescaling bug in set_scale.
//
// Revision 1.66  92/03/31  17:24:29  wmm
// Allow connectors to be selected.
//
// Revision 1.65  92/03/31  11:53:28  wmm
// Redo false bottoms and bottom lines to allow for scaling; position labels
// closer to connectors; make "tunnels" for crossing connectors.
//
// Revision 1.64  92/03/27  19:46:52  wmm
// Redo false bottoms for ERDs.
//
// Revision 1.63  92/03/26  17:00:52  wmm
// Support ERDs.
//
// Revision 1.62  92/03/25  14:29:47  glenn
// Change get_operation to ignore shift-keys.
//
// Revision 1.61  92/03/24  17:06:02  smit
// Bottom alignment should adjust length of text.
//
// Revision 1.60  92/03/20  11:38:26  smit
// Added new parameter to constructor.
//
// Revision 1.59  92/03/15  19:02:24  sergey
// New symbols for call tree diagram; merging with wmm.
//
// Revision 1.58  92/03/14  15:28:49  wmm
// Make sure relation arrow heads are the correct color.
// Revision 1.57  92/03/10  08:34:49  wmm
// Add support for OODT relations.
//
// Revision 1.56  92/03/06  12:17:04  sergey
// More tree connectors.
//
// Revision 1.55  92/03/04  13:33:11  sergey
// Added ldrcTREE connector type.
//
// Revision 1.54  92/03/03  18:37:54  glenn
// Call gra_handle_symbol_delete with graWindow*.
//
// Revision 1.53  92/02/28  19:43:48  smit
// Look at connections of existing pictures.
//
// Revision 1.52  92/02/28  17:01:18  smit
// Stop crashing problem when deleting at boundaries.
//
// Revision 1.51  92/02/25  21:01:58  glenn
// Add some of the include files that were removed from graSymbol.h
//
// Revision 1.50  92/02/24  12:51:15  jont
// set popup item sensitive
//
// Revision 1.49  92/02/18  17:45:16  smit
// Dont select text if clicked outside text.
//
// Revision 1.48  92/02/18  15:17:38  wmm
// Fix places where assumption was made that labels are the only contents of
// param_overlay.
//
// Revision 1.47  92/02/11  18:33:26  smit
// Return back after reporting the change.
//
// Revision 1.46  92/02/07  19:06:34  sergey
// More arrow heads and connectors for flowcharts.
//
// Revision 1.45  92/02/07  09:33:00  jont
// implemented context-sensetive popups
//
// Revision 1.44  92/02/06  16:50:09  jont
// *** empty log message ***
//
// Revision 1.43  92/01/30  12:00:26  smit
// Fix text editinbg problem in flow charts.
//
// Revision 1.42  92/01/29  19:39:33  smit
// Fixed couple of bugs.
//
// Revision 1.41  92/01/28  10:21:29  sergey
// GOTO symbol support (arrow heads).
//
//Revision 1.40  92/01/26  16:52:22  sergey
//Added special font for text inside a label.
//
//Revision 1.39  92/01/24  16:56:40  glenn
//Replace GET_PICTEL macro with inline function get_pictel.
//Implement get_rubband.
//
//Revision 1.38  92/01/23  18:17:36  wmm
//Allow labels to shrink after having been expanded.
//
//Revision 1.37  92/01/22  18:59:45  sergey
//Changed .. to ... in displaying long statements.
//
//Revision 1.36  92/01/22  17:45:24  smit
//Report text updates for STE .
//
//Revision 1.35  92/01/22  11:59:22  smit
//Deleting current character should not move cursor ahead.
//
//Revision 1.34  92/01/21  13:39:21  sergey
//Used POSITIOINED text_mode to put text inside symbols.
//
//Revision 1.33  92/01/17  13:02:15  smit
//Fix problems with hiliting.
//
//Revision 1.32  92/01/16  21:53:23  smit
//Add hilited text stuff.
//
//Revision 1.31  92/01/16  10:22:51  smit
//Made cursor colors in graphics windows configurable.
//
//Revision 1.30  92/01/13  19:37:02  smit
//Fix token editing functions.
//
//Revision 1.29  92/01/10  19:12:39  sergey
//Minor clean up.
//
//Revision 1.27  92/01/08  17:10:58  smit
//Token selections.
//
//Revision 1.26  92/01/07  17:18:02  sergey
//Added if/else flowcharting + wrote gra_arrow_head().
//
//Revision 1.25  92/01/07  15:10:20  smit
//Fix bugs.
//
//Revision 1.24  92/01/06  15:47:00  smit
//Handle Control characters forward/backward.
//
//Revision 1.23  92/01/06  13:46:41  smit
//Support token editing.
//
//Revision 1.22  92/01/04  13:52:17  smit
//Allocate buffer only when focused.
//
//Revision 1.21  92/01/02  18:17:50  smit
//Added many methods to support text editing.
//
//Revision 1.20  91/12/26  18:24:03  smit
//Added lots of new things for text input handling.
//
//Revision 1.19  91/12/24  17:23:16  smit
//Implement new methods.
//
//Revision 1.18  91/12/23  13:52:56  wmm
//Redo graConnectionNode::regenerate layout algorithm to do multiway connections
//closer to the source node rather than halfway between source and target;
//fix attachment point selection for class nodes and connectors to class nodes;
//make resizing a label do a whole new set_layers() call.
//
//Revision 1.17  91/12/17  16:14:19  sergey
//Corrected no-text case for containers.
//
//Revision 1.16  91/12/17  15:49:17  smit
//Implement open_node method.
//
//Revision 1.15  91/12/17  15:12:25  wmm
//Add support for multiway collapse/explode via use of sub_type field.
//
//Revision 1.14  91/12/13  13:28:48  wmm
//Add ability to get and set text width for multi-connection view routing.
//
//Revision 1.13  91/12/11  15:15:58  wmm
//Add code to include label text in symbol width.
//
//Revision 1.12  91/12/09  10:31:00  wmm
//Add support for new ldr connection types (regenerate() logic, parametric
//attachment points).
//
//Revision 1.11  91/12/06  13:56:02  sergey
//Corrected set_position.
//
//Revision 1.10  91/12/04  10:14:01  wmm
//Support OODT graphics symbols (separate left and right halves with extensible
//middle for graTextModeInside symbols).
//
//Revision 1.9  91/11/27  12:52:23  smit
//Remove text from layer 0 and move it to normal layer.
//
//Revision 1.8  91/11/13  12:40:09  smit
//Use resources instead of hard coding values.
//
//Revision 1.7  91/11/12  21:08:51  smit
// *** empty log message ***
//
//Revision 1.6  91/11/12  19:19:57  smit
// *** empty log message ***
//
//Revision 1.5  91/11/12  18:56:28  smit
// *** empty log message ***
//
//Revision 1.4  91/10/28  17:15:44  smit
//Added modes for parameters of diagrams.
//
//Revision 1.3  91/10/10  18:11:52  smit
//Changed 10x20 to 9x15 to make it run on Dec and Sun.
//
//Revision 1.2  91/09/19  18:11:08  kws
//Changed InterViews font to be larger
//
//Revision 1.1  91/09/01  19:57:55  kws
//Initial revision
//
*/






