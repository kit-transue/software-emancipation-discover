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
// INCLUDE FILES

#include <objOperate.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <graEnums.h>
#include <graSymbol.h>

#include <InterViews/enter-scope.h>
#include <InterViews/sensor.h>
#include <InterViews/perspective.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/polygons.h>
#include <InterViews/Graphic/label.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/printer.h>
#include <InterViews/leave-scope.h>
#include <ivfix.h>

#include <Print.h>
#include <graResources.h>
#include <representation.h>
#include <genError.h>
#include <objOper.h>
#include <ddict.h>
#include <dialog.h>
#include <driver.h>
#include <ldrNode.h>
#include <ldrHeader.h>
#include <viewError.h>
#include <viewNode.h>
#include <viewGraHeader.h>
#include <viewFn.h>
#include <viewSelection.h>
#include <viewFlowchartHeader.h>
#include <viewTreeHeader.h>
#include <button.h>
#include <graWorld.h>
#include <graTools.h>
#include <graWindow.h>
#include <ste_category_handler.h>
#include <systemMessages.h>
#include <rebuildControl.h>
#include <graInterface.h>
#include <gra_ivInterface.h>
#define wait ____wait____
#include <top_widgets.h>
#undef wait

#ifdef USE_SPARCWORKS
ivCanvas *Graphic::c ; 
#endif

extern "C" isServer();

extern int is_view_node_visible(viewTree *v);

extern long gala_current_window;;
extern long current_root_window;

void gra_focus_symbol(graWindow *window, graParametricSymbol *symbol);

graInterface::graInterface()
{
}

graInterface::~graInterface()
{
}

graInterface *graInterface::Create(void)
{
    return(new gra_ivInterface);
}

/* -------------------------------------------------------------------------- */

gra_ivInterface::gra_ivInterface()
{
    picture = NULL;
    window  = NULL;
}

graInterface *gra_ivInterface::Copy()
{
    gra_ivInterface *interface = new gra_ivInterface();
    interface->picture         = picture;
    interface->window          = window;
    return interface;
}

void gra_ivInterface::CloseWin()
{
    if(window)
    {
        gra_handle_window_delete(window);
	driver_instance->graphics_world->remove_window(window->get_scene());
	delete window->get_scene();
	window = NULL;
    }
}

void gra_ivInterface::DeletePicture()
{
    gra_handle_window_delete(window);
    delete picture;
}

void gra_ivInterface::CreatePicture()
{
    if ( !picture )
	picture = new Picture;
}

void gra_ivInterface::CreateWin(viewGraHeader *header, long parent_window)
{
    if (!window) {
	graScene* scene = new graScene(header, picture, allEvents);
	if(scene)
	    window = scene->get_window();
	if(window) {
	    if (isServer()) current_root_window = gala_current_window;
	    driver_instance->graphics_world->insert_window(scene);
	    if (isServer()) current_root_window = NULL;
	    window->set_widget(parent_window);
	}
    }
}

void gra_ivInterface::ManipulatePrespective(int op, int i1, int i2, int i3, int i4)
{
    gra_manipulate_perspective (window, op, i1, i2, i3, i4);
}

int gra_ivInterface::GetSliderStyle()
{
    return window->slider_style();
}

void gra_ivInterface::SetSliderStyle(int style)
{
    window->slider_style(style);
}

int gra_ivInterface::WindowExists()
{
    return (window != NULL);
}

graParametricSymbol *gra_ivInterface::GetTextFocus()
{
    return window->get_text_focus();
}

void gra_ivInterface::DamageArea(graParametricSymbol *sym)
{
    if(window)
	window->damage_area(sym);
}

void gra_ivInterface::InvalidatePicture()
{
    if(window && picture)
	window->damage_area(picture);
}

void gra_ivInterface::RepaintView()
{
    if ( window ) {
	// update perspective for correct values.
	window->Update();
	window->repaint_window();
    }
}

void gra_ivInterface::RefreshWindow()
{
    if(window)
	window->refresh();
}

void gra_ivInterface::Unhighlight()
{
    if (window)
	window->unhighlight();
}

void gra_ivInterface::InsertSymbol(graParametricSymbol *sym)
{
    if ( !picture )
	picture = new Picture;
    
    if ( sym )
        picture->Append( sym );
}

void gra_ivInterface::RemoveSymbol(graParametricSymbol *sym)
{
    if (window)
	gra_handle_symbol_delete(window, sym);
    
    if(picture)
    {
	if (window)
	    window->damage_area(sym);
	
	picture->Remove(sym);
    }
}

void gra_ivInterface::RemovePictures(void)
{
    Graphic *graphic;
    if ( picture ) {
	while ( graphic = picture->First() ) {
            window->damage_area( graphic );
            picture->Remove( graphic );
        }
    }
}

int gra_ivInterface::GetWindowSize(int& width, int& height)
{
    return window->size(width, height);
}

void gra_ivInterface::SelectSymbol(graParametricSymbol* symbol)
{
    if (symbol)
    {
        symbol->add_layers(graSymbolModeSelected);
	symbol->hilite_modification_text();
	
	if(window)
	{
	    window->damage_area(symbol);
	    window->repair_damage ();
	}
    }
}

void gra_ivInterface::UnselectSymbol(graParametricSymbol* symbol)
{
    if (!symbol) return;
    if (window){
	window->damage_area(symbol);
	window->remove_text_focus();
    }
    symbol->unhilite_all();
    symbol->rem_layers(graSymbolModeSelected);
    if(window) {
	window->damage_area(symbol);
	window->repair_damage();
    }
}

void gra_ivInterface::HighliteSymbol(graParametricSymbol* symbol)
{
    if(window)
	window->highlight(symbol);
}

void gra_ivInterface::UnhighliteSymbol(graParametricSymbol* symbol)
{
    if(window)
	window->unhighlight(symbol);
}

void gra_ivInterface::FocusSymbol(graParametricSymbol* symbol)
{
    gra_focus_symbol(window, symbol);
}

graWindow *gra_ivInterface::GetWindow()
{
    return window;
}

Picture *gra_ivInterface::GetPicture()
{
    return picture;
}

void gra_ivInterface::ComputePagesToPrint(int& rows, int& cols)
{
    if (!picture)
	return;

    int margin, width, height; // absolute page size
    float page_width;
    float page_height;
 
    printDialog *pd = printDialog::instance();
    pd->page_size (width, height, margin);
    if (pd->landscape())
    {
        page_width = height - 2.0 * margin;
        page_height = width - 2.0 * margin;
    }
    else
    {
        page_width = width - 2.0 * margin;
        page_height = height - 2.0 * margin;
    }
 
    // reset the scale to 1.0
    Graphic *graphic = NULL;
    iv3(Transformer) *ps = NULL;

    if (window)
    {
	graphic = window->GetGraphic();
	ps = new iv3(Transformer) (graphic->GetTransformer());
	graphic->SetTransformer(NULL);
    }

    // get size of picture
    iv2(Coord) left, bottom, right, top;
    picture->GetBox(left, bottom, right, top);
 
    iv2(Coord) pic_width = right - left + 1;
    iv2(Coord) pic_height = top - bottom + 1;

    float scale = pd->scale();

    if (scale != 0)
    {
	pic_width = (int) (pic_width * scale);
	pic_height = (int) (pic_height *scale);
    }

    rows = (int) (pic_height / page_height);
    if (pic_height > (page_height * rows)) rows++;

    cols = (int) (pic_width / page_width);
    if (pic_width > (page_width * cols)) cols++;

    // set up transformer back again
    if (graphic && ps)
        graphic->SetTransformer (ps);
}

float gra_ivInterface::ScaleToFitToPage()
{
    if (!picture)
	return 1.0;

    int margin, width, height; // absolute page size
    float page_width;
    float page_height;

    printDialog *pd = printDialog::instance();
    pd->page_size (width, height, margin);

    if (pd->landscape())
    {
	page_width = height - 2.0 * margin;
        page_height = width - 2.0 * margin;
    }
    else
    {
	page_width = width - 2.0 * margin;
        page_height = height - 2.0 * margin;
    }

    // reset the scale to 1.0
    Graphic *graphic = NULL;
    iv3(Transformer) *ps = NULL;

    if (window)
    {
	graphic = window->GetGraphic();
	ps = new iv3(Transformer) (graphic->GetTransformer());
	graphic->SetTransformer(NULL);
    }

    // get size of picture
    iv2(Coord) left, bottom, right, top;
    picture->GetBox(left, bottom, right, top);

    iv2(Coord) pic_width = right - left + 1;
    iv2(Coord) pic_height = top - bottom + 1;

    float hscale = page_width / pic_width;
    float rscale = page_height / pic_height;

    float scale = (hscale < rscale) ? hscale : rscale;

    // set up transformer back again
    if (graphic && ps)
        graphic->SetTransformer (ps);

    return scale;
}

inline const char* GET_VALUE(const char* X)
{
    return ui_get_value(DRAWING_WINDOW_CLASS, X);
}

// draw a string label in top right corner
static void draw_string_label(
    char* page_str, iv3(Printer)* printer, int landscape)
{
    Initialize(draw_string_label);

    printDialog* pd = printDialog::instance();

    iv3(Label)* str = new iv3(Label)(page_str);

    int page_width, page_height, margin;
    PFont *pf = new PFont (GET_VALUE (BASE_FONT));
    PColor *fg = new PColor (GET_VALUE (BASE_FOREGROUND));
    PColor *bg = new PColor (GET_VALUE(BASE_BACKGROUND));

    float scale = 0.5;

    str->SetFont (pf);
    str->Scale (scale, scale);
    iv2(Coord) left, bottom, right, top;
    str->GetBox(left, bottom, right, top);

    printer->push_transform();

    pd->page_size (page_width, page_height, margin);

    // set up tranform to print row, column at top right corner
    iv3(Transformer) t;
    t.Translate (-right, -top);
    if (landscape)
    {
	t.Rotate (90);
	t.Translate (margin, page_height-margin);
    }
    else
	t.Translate (page_width-margin, page_height-margin);

    str->SetCanvas(printer);
    str->SetColors(fg, bg);

    // draw it
    printer->transform (t);
    str->Draw(printer);
    printer->pop_transform();
    str->SetCanvas(NULL);

    // clean up
    delete str;
    delete pf;
    delete fg;
    delete bg;
}

int gra_ivInterface::SendPostscript(ostream& ostr)
{
    if (!picture)
	return 0;

    char page_str[64];
    int width, height, margin; // absolute sizes
    float page_width;
    float page_height;

    printDialog *pd = printDialog::instance();
    pd->page_size (width, height, margin);
    int landscape = pd ? pd->landscape() : 0;
    float scale = pd->scale();

    iv3(Printer) *printer = NULL;

    int a_page_printed = 0;

    float clipped_page_width = page_width = width - 2.0 * margin;
    float clipped_page_height = page_height = height - 2.0 * margin;

    if (landscape)
    {
	page_width = clipped_page_height;
	page_height = clipped_page_width;
    }

    page_width /= scale;
    page_height /= scale;

    // reset the scale to 1.0
    Graphic *graphic = NULL;
    iv3(Transformer) *ps = NULL;

    if (window)
    {
	graphic = window->GetGraphic();
	ps = new iv3(Transformer) (graphic->GetTransformer());
	graphic->SetTransformer(NULL);
    }

    // get size of picture
    iv2(Coord) left, bottom, right, top;
    picture->GetBox(left, bottom, right, top);
	
    for (int x = 1; left < right; x++, left += (iv2(Coord)) page_width)
    {
	int y = 1;
	for (iv2(Coord) b = bottom; b < top; y++, b += (iv2(Coord)) page_height)
	{
	    // create page only if it contains any visible graphic
	    Graphic** intersection = 0;
	    Graphic** within = 0;
	    BoxObj box(left, b, left+(int)page_width, b+(int) page_height);
	    if (picture->GraphicsIntersecting (box, intersection) ||
		picture->GraphicsWithin(box, within))
	    {
		if (intersection) delete []intersection;
		if (within) delete []within;

		if (a_page_printed == 0) // set up printer first time through
		{
		    a_page_printed = 1;
		    printer = new iv3(Printer)(&ostr);
		    printer->prolog("SourceAnalysis");
		    printer->page (NULL);
		    ostr << "0 setlinewidth" << endl;
		}
		else{			// output previous page
		    printer->page(page_str);
		    ostr << "0 setlinewidth" << endl;
	        }

		picture->SetCanvas(printer);

		// set clipping : Interviews clip_rect does not work,
		// so send clipping instruction directly.
		printer->push_transform();
		ostr << "newpath" << endl;
		ostr << margin << " " << margin << " moveto" << endl;
		ostr << margin << " " << height - margin << " lineto" << endl;
		ostr << width - margin << " " << height - margin
		     << " lineto" << endl;
		ostr << width - margin << " " << margin << " lineto" << endl;
		ostr << "closepath" << endl;
		ostr << "clip" << endl;

		// Set up page transformations
		iv3(Transformer) t;
		
		// Move picture to (0,0)
		t.Translate (-left, -b);
		t.Scale (scale, scale);

		t.Translate (margin, margin);

		// handle landscape mode
		if (landscape)
		{
		    t.Rotate (90.0);
		    t.Translate (width, 0);
		}

		printer->push_transform();
		printer->transform (t);

		picture->DrawClipped(printer, left, b,
				     left + (int) page_width,
				     b + (int) page_height);

		sprintf (page_str, "(%d,%d)", x, y);

		printer->pop_transform();
		printer->pop_transform();

		picture->SetCanvas(NULL);

		// draw page str at the bottom
		draw_string_label (page_str, printer, landscape);
	    }
	}
    }

    // close printer
    if (printer)
    {
	printer->epilog();
	delete printer;
    }

    // set up transformer back again
    if (graphic && ps)
	graphic->SetTransformer (ps);

    return 1;
}


/* ---------------------------------------------------------------------- */

NodePopupInfo::NodePopupInfo(void *cb_data)
{
    item_number   = 0;
    callback_data = cb_data;
}

NodePopupInfo::~NodePopupInfo()
{
    for(int i = 0; i < item_number; i++){
	free(row_text[i]);
    }
}

void *NodePopupInfo::GetCallbackData(void)
{
    return callback_data;
}

void NodePopupInfo::AddRow(int type, const char *text, void *callback)
{
    if(item_number < 50){
	row_text[item_number]     = text ? strdup(text) : NULL;
	row_type[item_number]     = type;
	row_callback[item_number] = callback;
	toggle_state[item_number] = 0;
	item_number++;
    }
}

void NodePopupInfo::SetToggleState(int state)
{
    if(item_number > 0){
	toggle_state[item_number - 1] = state;
    }
}

int NodePopupInfo::GetToggleState(int index)
{
    return toggle_state[index];
}

int NodePopupInfo::GetType(int index)
{
    return row_type[index];
}

char *NodePopupInfo::GetText(int index)
{
    return row_text[index];
}

void *NodePopupInfo::GetCallback(int index)
{
    return row_callback[index];
}
