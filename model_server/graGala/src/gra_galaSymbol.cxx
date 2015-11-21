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
#include <vport.h>
#include vstdioHEADER
#include vstdlibHEADER
#include vdrawHEADER
#include vdomainHEADER
#include vdomainitemHEADER
#include vimageHEADER
#include <gra_galaWorld.h>
#include <gra_galaSymbol.h>
#define  xxxPrintf printf
#undef   printf

#include <machdep.h>
#include <driver.h>
#include <viewNode.h>
#include <viewGraHeader.h>
#include <viewRouter.h>
#include <viewERDHeader.h>
#include <viewERDClassNode.h>

#define printf xxxPrintf

int connection_colorR = 0;
int connection_colorG = 0;
int connection_colorB = 50000;

vimage      *gra_get_image(int index);
int         gra_lookup_color(char *name, int& r, int& g, int& b);


struct _hcoords		// Structure used to sort line segment information
{			// for creating connector symbols.
    float             x, y;
    viewSymbolNodePtr viewnode;
};

enum arrow_type
{
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_LEFT_LONG,
    ARROW_UP
};


/*--------------------------------------------------------------------------*/

void add_arrow_head(graDrawingCommands *segment, arrow_type type, float x_coord, float y_coord, float len, float w)
{
    switch (type){
      case ARROW_DOWN :
	segment->AddMoveto((int)x_coord, (int)y_coord);
	segment->AddLineto((int)(x_coord - w), (int)(y_coord + len));
	segment->AddLineto((int)(x_coord + w), (int)(y_coord + len));
	break;

      case ARROW_RIGHT :	// --->
	segment->AddMoveto((int)x_coord, (int)y_coord);
	segment->AddLineto((int)(x_coord - len), (int)(y_coord - w));
	segment->AddLineto((int)(x_coord - len), (int)(y_coord + w));
	break;

      case ARROW_LEFT_LONG:	// <----
	segment->AddMoveto((int)(x_coord - len), (int)y_coord);
	segment->AddLineto((int)x_coord, (int)(y_coord - w));
	segment->AddLineto((int)x_coord, (int)(y_coord + w));
	break;

      case ARROW_LEFT:		// <--
	segment->AddMoveto((int)x_coord, (int)y_coord);
	segment->AddLineto((int)(x_coord + len), (int)(y_coord - w));
	segment->AddLineto((int)(x_coord + len), (int)(y_coord + w));
	break;

      case ARROW_UP:
	segment->AddMoveto((int)x_coord, (int)y_coord);
	segment->AddLineto((int)(x_coord - w), (int)(y_coord - len));
	segment->AddLineto((int)(x_coord + w), (int)(y_coord - len));
	break;
      default:			// unKnown type
	break;
    }
    segment->AddFill();
}

/*--------------------------------------------------------------------------*/

graDrawingCommands::graDrawingCommands()
{
    scalex = (float)1;
    scaley = (float)1;
    drawing_commands    = new int[50];
    cmd_size            = 50;
    last_cmd_pos        = 0;
    drawing_commands[0] = gra_end;
    bounds_rect_w       = 0;
    bounds_rect_h       = 0;
    bounds_rect_x       = 0;
    bounds_rect_y       = 0;
    highlited           = 0;
}

void graDrawingCommands::SetDrawingCmds(int *cmds)
{
    for(int sz = 0; cmds[sz] != gra_end; sz++) ;
    grow(sz);
    for(int i = 0; i < sz; i++)
	drawing_commands[i] = cmds[i];
    drawing_commands[sz] = gra_end;
}

void graDrawingCommands::SetHighlited(int flag)
{
    highlited = flag;
}

void graDrawingCommands::UpdateBounds(int x, int y, int w, int h)
{
    if(bounds_rect_x == 655500)
	bounds_rect_x = x;
    if(bounds_rect_y == 655500)
	bounds_rect_y = y;
    if(y < bounds_rect_y){
	bounds_rect_h += bounds_rect_y - y;
	bounds_rect_y  = y;
    }
    if(x < bounds_rect_x){
	bounds_rect_w += bounds_rect_x - x;
	bounds_rect_x  = x;
    }
    if(w + x > bounds_rect_w + bounds_rect_x)
	bounds_rect_w = w + x - bounds_rect_x;
    if(h + y > bounds_rect_h + bounds_rect_y)
	bounds_rect_h = h + y - bounds_rect_y;
}

void graDrawingCommands::do_moveto(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    vdraw::FMoveTo((double)x, (double)y);
}

void graDrawingCommands::do_lineto(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    vdraw::FLineTo((double)x, (double)y);
}

void graDrawingCommands::do_ellipse(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    int w = drawing_commands[index++] * scalex;
    int h = drawing_commands[index++] * scaley;
    vdraw::RectEllipseStroke(x, y, w, h);
    UpdateBounds(x, y, w, h);
}

void graDrawingCommands::do_stroke(int &index)
{
    const vrectFloat *rects;
    int   count;
    vdraw::FPathBounds(&rects, &count);
    while(count){
	UpdateBounds((int)rects->x, (int)rects->y, (int)rects->w, (int)rects->h);
	count--;
	rects++;
    }
    index++;
    vdraw::Stroke();
}

void graDrawingCommands::do_fill(int &index)
{
    const vrectFloat *rects;
    int   count;
    vdraw::FPathBounds(&rects, &count);
    while(count){
	UpdateBounds((int)rects->x, (int)rects->y, (int)rects->w, (int)rects->h);
	count--;
	rects++;
    }
    index++;
    vdraw::Fill();
}

void graDrawingCommands::do_circle(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    int r = drawing_commands[index++] * scalex;
    vdraw::FArc((double)x, (double)y, (double)r, 0, 360);
}

void graDrawingCommands::do_color(int &index)
{
    vcolorSpec spec;
    vcolor     *color;
    int        r, g, b;

    index++;
    r = drawing_commands[index];
    g = drawing_commands[index + 1];
    b = drawing_commands[index + 2];
    if(highlited){
	int coeff = r / 6500;
	r        += coeff * 2000;
	coeff     = g / 6500;
	g        += coeff * 2000;
	coeff     = b / 6500;
	b        += coeff * 2000;
    }
    spec.SetRGB(r, g, b);
    color = vcolor::Intern(&spec);
    vdraw::SetColor(color);
    index += 3;
}

void graDrawingCommands::do_image(int &index)
{
    int    x, y;
    vrect  src;
    vrect  dest;
    int    image_index;
    vimage *image;
    
    index++;
    vdraw::GetPoint(&x, &y);
    image_index = drawing_commands[index++];
    image       = gra_get_image(image_index);
    src.x       = 0;
    src.y       = 0;
    src.w       = image->GetWidth();
    src.h       = image->GetHeight();
    dest.x      = x;
    dest.y      = y;
    dest.w      = (int)((float)src.w * scalex);
    dest.h      = (int)((float)src.h * scaley);
    vdraw::RectImage(image, &src, &dest);
    UpdateBounds(dest.x, dest.y, dest.w, dest.h);
}

void graDrawingCommands::do_movetoxy(int &index)
{
    index++;
    x1 = drawing_commands[index++] * scalex + xoffset;
    y1 = drawing_commands[index++] * scaley + yoffset;
}

void graDrawingCommands::do_linetoxy(int &index)
{
    index++;
    x2 = drawing_commands[index++] * scalex + xoffset;
    y2 = drawing_commands[index++] * scaley + yoffset;
}

void graDrawingCommands::do_ellipsexy(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    int w = drawing_commands[index++] * scalex;
    int h = drawing_commands[index++] * scaley;
}

void graDrawingCommands::do_strokexy(int &index)
{
    index++;
}

void graDrawingCommands::do_fillxy(int &index)
{
    index++;
}

void graDrawingCommands::do_circlexy(int &index)
{
    index++;
    int x = drawing_commands[index++] * scalex + xoffset;
    int y = drawing_commands[index++] * scaley + yoffset;
    int r = drawing_commands[index++] * scalex + yoffset;
}

void graDrawingCommands::do_colorxy(int &index)
{
    index += 4;
}

void graDrawingCommands::do_imagexy(int &index)
{
    index++;
    index++;
}

void graDrawingCommands::Execute(int x, int y, float sx, float sy)
{
    int index = 0;

    xoffset = x;
    yoffset = y;
    scalex  = sx;
    scaley  = sy;
    vdraw::MoveTo(0, 0);
    vdraw::NewPath();
//    if(drawing_commands[index] == gra_end)
//	printf("warning: empty drawing object!\n");
    while(drawing_commands[index] != gra_end){
	switch(drawing_commands[index]){
	  case gra_moveto:
	    do_moveto(index);
	    break;
	  case gra_lineto:
	    do_lineto(index);
	    break;
	  case gra_circle:
	    do_circle(index);
	    break;
	  case gra_ellipse:
	    do_ellipse(index);
	    break;
	  case gra_stroke:
	    do_stroke(index);
	    break;
	  case gra_fill:
	    do_fill(index);
	    break;
	  case gra_color:
	    do_color(index);
	    break;
	  case gra_image:
	    do_image(index);
	    break;
	  case gra_end:
	    break;
	  default:
	    printf("Unrecognized drawing command.\n");
	    break;
	}
    }
}

int graDrawingCommands::ExecutePointNearCheck(int x, int y, float sx, float sy, int px, int py, int distance)
{
    int cmd_index;
    int index           = 0;
    int distance_square = distance * distance;

    xoffset = x;
    yoffset = y;
    scalex  = sx;
    scaley  = sy;
    x1      = 0;
    y1      = 0;
    while(drawing_commands[index] != gra_end){
	cmd_index = index;
	switch(drawing_commands[index]){
	  case gra_moveto:
	    do_movetoxy(index);
	    break;
	  case gra_lineto:
	    do_linetoxy(index);
	    if(CheckPoint(px, py, distance_square))
		return cmd_index; 
	    break;
	  case gra_circle:
	    do_circlexy(index);
	    break;
	  case gra_ellipse:
	    do_ellipsexy(index);
	    break;
	  case gra_stroke:
	    do_strokexy(index);
	    break;
	  case gra_fill:
	    do_fillxy(index);
	    break;
	  case gra_color:
	    do_colorxy(index);
	    break;
	  case gra_image:
	    do_imagexy(index);
	    break;
	  case gra_end:
	    break;
	  default:
	    printf("Unrecognized drawing command.\n");
	    break;
	}
    }
    return -1;
}

static float dot_product(float x1, float y1, float x2, float y2)
{
    return(x1 * x2 + y1 * y2);
}

int graDrawingCommands::CheckPoint(int px, int py, int distance)
{
    float fx1, fy1;
    float fx2, fy2;
    float ppx, ppy;
    float t, fd;

    fx1 = px - x1;
    fy1 = py - y1;  // X1

    fx2 = x2 - x1;
    fy2 = y2 - y1;  // X2

    t = dot_product(fx1, fy1, fx2, fy2) / dot_product(fx2, fy2, fx2, fy2);
    if(t >= 1.0){
	ppx = fx2;
	ppy = fy2;
    } else if(t <= 0.0){
	ppx = fx1;
	ppy = fy1;
    } else {
	ppx = t * fx2 + x1;
	ppy = t * fy2 + y1;
    }
    
    fd = (ppx - (float)px) * (ppx - (float)px) + (ppy - (float)py) * (ppy - (float)py);
    if(fd <= distance)
	return 1;
    x1 = x2;
    y1 = y2;
    return 0;
}

void graDrawingCommands::GetBounds(vrectLong &rect, float sx, float sy)
{
    vdraw::GSave();
    vdraw::NullDevice();
    bounds_rect_x = 655500;
    bounds_rect_y = 655500;
    bounds_rect_w = 0;
    bounds_rect_h = 0;
    Execute(0, 0, sx, sy);
    rect.x = bounds_rect_x;
    rect.y = bounds_rect_y;
    rect.w = bounds_rect_w;
    rect.h = bounds_rect_h;
    vdraw::GRestore();
}

void graDrawingCommands::grow(int len)
{
    if(len + last_cmd_pos >= cmd_size){
	int delta = 50;
	if(len > 50)
	    delta = ((len / 50) + 1) * 50;
	int new_size  = cmd_size + delta;
	int *new_cmds = new int[new_size];
	for(int i = 0; i <= last_cmd_pos; i++)
	    new_cmds[i] = drawing_commands[i];
	delete [] drawing_commands;
	drawing_commands = new_cmds;
	cmd_size         = new_size;
    }
}

void graDrawingCommands::AddLine(int x1, int y1, int x2, int y2)
{
    grow(7);
    drawing_commands[last_cmd_pos++] = gra_moveto;
    drawing_commands[last_cmd_pos++] = x1;
    drawing_commands[last_cmd_pos++] = y1;
    drawing_commands[last_cmd_pos++] = gra_lineto;
    drawing_commands[last_cmd_pos++] = x2;
    drawing_commands[last_cmd_pos++] = y2;
    drawing_commands[last_cmd_pos++] = gra_stroke;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddEllipse(int x, int y, int w, int h)
{
    grow(5);
    drawing_commands[last_cmd_pos++] = gra_ellipse;
    drawing_commands[last_cmd_pos++] = x;
    drawing_commands[last_cmd_pos++] = y;
    drawing_commands[last_cmd_pos++] = w;
    drawing_commands[last_cmd_pos++] = h;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddCircle(int x, int y, int r)
{
    grow(5);
    drawing_commands[last_cmd_pos++] = gra_circle;
    drawing_commands[last_cmd_pos++] = x;
    drawing_commands[last_cmd_pos++] = y;
    drawing_commands[last_cmd_pos++] = r;
    drawing_commands[last_cmd_pos++] = gra_stroke;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::SetColors(int r, int g, int b)
{
    grow(4);
    drawing_commands[last_cmd_pos++] = gra_color;
    drawing_commands[last_cmd_pos++] = r;
    drawing_commands[last_cmd_pos++] = g;
    drawing_commands[last_cmd_pos++] = b;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddMoveto(int x, int y)
{
    grow(3);
    drawing_commands[last_cmd_pos++] = gra_moveto;
    drawing_commands[last_cmd_pos++] = x;
    drawing_commands[last_cmd_pos++] = y;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddLineto(int x, int y)
{
    grow(3);
    drawing_commands[last_cmd_pos++] = gra_lineto;
    drawing_commands[last_cmd_pos++] = x;
    drawing_commands[last_cmd_pos++] = y;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddLineWidth(int lw)
{
    grow(2);
    drawing_commands[last_cmd_pos++] = gra_linewidth;
    drawing_commands[last_cmd_pos++] = lw;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddImage(int index)
{
    grow(2);
    drawing_commands[last_cmd_pos++] = gra_image;
    drawing_commands[last_cmd_pos++] = (int)index;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddRect(int x, int y, int w, int h)
{
}

void graDrawingCommands::AddStroke(void)
{
    grow(1);
    drawing_commands[last_cmd_pos++] = gra_stroke;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddFill(void)
{
    grow(1);
    drawing_commands[last_cmd_pos++] = gra_fill;
    drawing_commands[last_cmd_pos]   = gra_end;
}

void graDrawingCommands::AddCommands(graDrawingCommands *cmds)
{
    grow(cmds->last_cmd_pos);
    for(int i = 0; i < cmds->last_cmd_pos; i++)
	drawing_commands[last_cmd_pos++] = cmds->drawing_commands[i];
    drawing_commands[last_cmd_pos] = gra_end;
}

void graDrawingCommands::ClearCommands(void)
{
    drawing_commands[0] = gra_end;
    last_cmd_pos        = 0;
}

int graDrawingCommands::GetCurrentCmdIndex(void)
{
    return last_cmd_pos;
}

//------------------------------------------
// class Line
//------------------------------------------

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
// graFlexConn
//------------------------------------------

graFlexConn::graFlexConn(viewConnectionNode *vn) : is_highlighted(false)
{
    num_log_segs = vn->inputs->size() + vn->outputs->size() + 1;
    log_segs     = new logical_segment[num_log_segs];
    int lsegno   = 0;
    for (int i = 0; i < vn->inputs->size(); i++)
	log_segs[lsegno++].vn = (viewSymbolNodePtr)((*vn->inputs)[i]);
    for (i = 0; i < vn->outputs->size(); i++)
	log_segs[lsegno++].vn = (viewSymbolNodePtr)((*vn->outputs)[i]);
}

graFlexConn::~graFlexConn()
{
    delete [] log_segs;
}

graSymbolType graFlexConn::SymbolType()
{
    return graFlexConnType;
}

void graFlexConn::regenerate(viewConnectionNode& vcn)
{
    int r, g, b;

    viewERDConnectionNode& vn = (viewERDConnectionNode&)vcn;
    const char* fg_name       = vn.color_resource();
    gra_lookup_color((char*)fg_name, r, g, b);
    SetColors(r, g, b);
//    const char* fg_value      = (fg_name ? GET_VALUE(fg_name, DFLT_FORE) : NULL);
//    PColor* fg                = (fg_value ? lookup_PColor(fg_value) : connection_foreground);


    // Delete old line segments.
    if(num_segments)
    {
	if(get_viewnode())
	{
	    viewGraHeaderPtr vh = (viewGraHeaderPtr)(get_viewnode()->get_header());
	    if(vh)
		vh->get_gra_interface()->DamageArea(this);
	}
//	delete_picture_contents(this);
	delete [] segments;
	segments     = NULL;
	num_segments = 0;
    }

    // Calculate number of physical segments required
    for (int lsegno = 0; lsegno < num_log_segs; lsegno++)
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
		AddLine(x0, y0, x1, y1);
	    }
	}
    }
    int psegno     = 0;
    bool arrows    = vn.do_arrows();
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
		int i1 = GetCurrentCmdIndex();
 		AddLine(x0, y0, x1, y1);
		int i2 = GetCurrentCmdIndex();
		segments[psegno].path     = new GraphPath(i1, i2);
		segments[psegno].viewnode = NULL;
		psegno++;
	    }
	    add_decoration(log_segs[lsegno], 0, 60000, 0, arrows && lsegno != num_log_segs - 1, lsegno < num_inputs);
	}
    }
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graFlexConn::add_decoration(logical_segment& seg, int r, int g, int b, bool do_arrow, bool is_input)
{
    int x0 = seg.vert_x[0];
    int y0 = seg.vert_y[0];
    int x1 = seg.vert_x[1];
    int y1 = seg.vert_y[1];

    int xsign     = (x0 <= x1) ? 1 : -1;
    int ysign     = (y0 <= y1) ? 1 : -1;
    bool vertical = (x0 == x1);

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
//	double alen = GET_VALUE(ERD_ARROWHEAD_LENGTH, 9.0);
//	double awid = GET_VALUE(ERD_ARROWHEAD_WIDTH, 4.0);
	double alen = 9.0;
	double awid = 4.0;
	add_arrow_head(this, type, pointx, pointy, alen, awid);
    }

    switch (seg.decoration)
    {
      case NOTHING:
	break;

      case SOLID_DOT:
	if (vertical)
	    y0 += 5 * ysign;
	else x0 += 5 * xsign;
	AddCircle(x0, y0, 5);
//	gr = new FillCircle(x0, y0, 5);
//	gr->SetColors(fg, bg);
//	Append(gr);
	break;

      case HOLLOW_DOT:
	if (vertical)
	    y0 += 5 * ysign;
	else x0 += 5 * xsign;
	AddCircle(x0, y0, 5);
	break;

      case DOUBLE_ARROW:
	if (vertical)
	{
	    AddLine(x0, y0 + 6 * ysign, x0 - 5, y0 + 16 * ysign);
	    AddLine(x0, y0 + 6 * ysign, x0 + 5, y0 + 16 * ysign);
	}
	else
	{
	    AddLine(x0 + 6 * xsign, y0, x0 + 16 * xsign, y0 - 5);
	    AddLine(x0 + 6 * xsign, y0, x0 + 16 * xsign, y0 + 5);
	}
	/* FALLTHROUGH */

      case SINGLE_ARROW:
	if (vertical)
	{
	    AddLine(x0, y0, x0 - 5, y0 + 10 * ysign);
	    AddLine(x0, y0, x0 + 5, y0 + 10 * ysign);
	}
	else
	{
	    AddLine(x0, y0, x0 + 10 * xsign, y0 - 5);
	    AddLine(x0, y0, x0 + 10 * xsign, y0 + 5);
	}
	break;

      case DOUBLE_BAR:
	if (vertical)
	    AddLine(x0 - 6, y0 + 12 * ysign, x0 + 6, y0 + 12 * ysign);
	else AddLine(x0 + 12 * xsign, y0 - 6, x0 + 12 * xsign, y0 + 6);
	/* FALLTHROUGH */

      case SINGLE_BAR:
	if (vertical)
	    AddLine(x0 - 6, y0 + 8 * ysign, x0 + 6, y0 + 8 * ysign);
	else AddLine(x0 + 8 * xsign, y0 - 6, x0 + 8 * xsign, y0 + 6);
	break;

      case TRIANGLE:
	{
	    int x[3];
	    int y[3];
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
	    
	    AddMoveto(x[0], y[0]);
	    AddLineto(x[1], y[1]);
	    AddLineto(x[2], y[2]);
	    AddFill();
	}
	break;

      case SEMICIRCLE:
	{
	    int x[5];
	    int y[5];
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
//	    gr = new FillBSpline(x, y, 5);
//	    gr->SetColors(bg, bg);
//	    Append(gr);
//	    gr = new BSpline(x, y, 5);
//	    gr->SetBrush(pbrush0);
//	    gr->SetColors(fg, bg);
//	    Append(gr);
	    AddLine(x[0], y[0], x[4], y[4]);
//	    gr = new Line(x[0], y[0], x[4], y[4]);
//	    gr->SetBrush(pbrush0);
//	    gr->SetColors(fg, bg);
//	    Append(gr);
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
	    AddMoveto(x[0], y[0]);
	    AddLineto(x[1], y[1]);
	    AddLineto(x[2], y[2]);
	    AddLineto(x[3], y[3]);
	    AddFill();
	}
	break;

      case BAR_CHICKENFOOT:
	if (vertical)
	    AddLine(x0 - 4, y0 + 8 * ysign, x0 + 4, y0 + 8 * ysign);
	else AddLine(x0 + 8 * xsign, y0 - 4, x0 + 8 * xsign, y0 + 4);
	/* FALLTHROUGH */

      case CHICKENFOOT:
	if (vertical)
	{
	    AddLine(x0 - 4, y0, x0, y0 + 8 * ysign);
	    AddLine(x0 + 4, y0, x0, y0 + 8 * ysign);
	}
	else
	{
	    AddLine(x0, y0 - 4, x0 + 8 * xsign, y0);
	    AddLine(x0, y0 + 4, x0 + 8 * xsign, y0);
	}
	break;

      case CIRCLE_BAR:
	if (vertical)
	    AddLine(x0 - 6, y0 + 8 * ysign, x0 + 6, y0 + 8 * ysign);
	else AddLine(x0 + 8 * xsign, y0 - 6, x0 + 8 * xsign, y0 + 6);
	if (vertical)
	    y0 += 14 * ysign;
	else x0 += 14 * xsign;
	AddCircle(x0, y0, 6);
	break;

      case CIRCLE_CHICKENFOOT:
	if (vertical)
	{
	    AddLine(x0 - 4, y0, x0, y0 + 8 * ysign);
	    AddLine(x0 + 4, y0, x0, y0 + 8 * ysign);
	    y0 += 14 * ysign;
	}
	else
	{
	    AddLine(x0, y0 - 4, x0 + 8 * xsign, y0);
	    AddLine(x0, y0 + 4, x0 + 8 * xsign, y0);
	    x0 += 14 * xsign;
	}
	AddCircle(x0, y0, 6);
	break;
    }
}


void graFlexConn::add_vertex(int lsegno, int x, int y)
{
    if (lsegno < num_log_segs) {
	log_segs[lsegno].vert_x.insert_last(x);
	log_segs[lsegno].vert_y.insert_last(y);
    }
}
 
void graFlexConn::reinit_seg(int lsegno)
{
    if (lsegno < num_log_segs) {
	log_segs[lsegno].vert_x.removeAll();
	log_segs[lsegno].vert_y.removeAll();
	log_segs[lsegno].decoration = NOTHING;
	delete log_segs[lsegno].label;
	log_segs[lsegno].label = NULL;
    }
}
 
void graFlexConn::set_decoration(int lsegno, deco_type decoration)
{
    if (lsegno < num_log_segs) {
	log_segs[lsegno].decoration = decoration;
    }
}
 
void graFlexConn::set_label(int seg_num, const char*)
{
}
 
void graFlexConn::get_label_size(int seg_num, float& x, float& y)
{
}
 
void graFlexConn::set_label_loc(int seg_num, bool centered, bool above_or_right)
{
}

void graFlexConn::do_highlight(void)
{
    SetHighlited(1);
}
 
void graFlexConn::undo_highlight(void)
{
    SetHighlited(0);
}

void graFlexConn::Draw(int &cx, int &cy, int &w, int &h, bool)
{
    vrectLong rect;

    Execute(x, y, scalex, scaley);
    GetBounds(rect, scalex, scaley);
    cx = rect.x;
    cy = rect.y;
    w  = rect.w;
    h  = rect.h;
}

void graFlexConn::MoveDX(int dx)
{
    x = dx;
    if(domain && domain_object){
	int       xx, yy, w, h;
	vrectLong rect;
	GetPosition(xx, yy, w, h);
	rect.x = xx + x; rect.y = yy + y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graFlexConn::MoveDY(int dy)
{
    y = dy;
    if(domain && domain_object){
	int       xx, yy, w, h;
	vrectLong rect;
	GetPosition(xx, yy, w, h);
	rect.x = xx + x; rect.y = yy + y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graFlexConn::GetPosition(int &x, int &y, int &w, int &h)
{
    vdraw::GSave();
    vdraw::NullDevice();
    Draw(x, y, w, h, TRUE);
    vdraw::GRestore();
}

//------------------------------------------
// graConnectionSymbol
//-----------------------------------------

//------------------------------------------
// graConnectionSymbol::graConnectionSymbol()
//------------------------------------------

/* The default constructor is protected so it can be used only from
 * derived classes; the only difference from the public constructor is
 * that it does not do a regenerate() automatically.
 */

graConnectionSymbol::graConnectionSymbol() : graParametricSymbol(-1, 0)
{
    num_segments = 0;
    segments     = NULL;
    x            = 0;
    y            = 0;
}

graConnectionSymbol::graConnectionSymbol(viewConnectionNode& VN) : graParametricSymbol(-1, 0)
{
    num_segments = 0;
    segments     = NULL;
    x            = 0;
    y            = 0;
    regenerate (VN); //??? WHAT IS THAT FOR???
}

graConnectionSymbol::~graConnectionSymbol()
{
    delete [] segments;
}

graSymbolType graConnectionSymbol::SymbolType()
{
    return graConnectionSymbolType;
}

void graConnectionSymbol::set_layers(int)
{
}

void graConnectionSymbol::add_layers(int)
{
}
    
void graConnectionSymbol::rem_layers(int)
{
}

void graConnectionSymbol::GetPosition(int &x, int &y, int &w, int &h)
{
    vdraw::GSave();
    vdraw::NullDevice();
    Draw(x, y, w, h, TRUE);
    vdraw::GRestore();
}

void graConnectionSymbol::Draw(int &cx, int &cy, int &w, int &h, bool)
{
    vrectLong rect;

    Execute(x, y, scalex, scaley);
    GetBounds(rect, scalex, scaley);
    cx = rect.x;
    cy = rect.y;
    w  = rect.w;
    h  = rect.h;
}

void graConnectionSymbol::SetDomainAndObject(vdomain *vd, vdomainObject *obj)
{
    domain        = vd;
    domain_object = obj;
    domain->SetObjectOpaque(vdomainObjectToSet(obj), vFALSE);
}

//------------------------------------------
// find_insertion
//
// This function determines the semantically closest symbol node to the
// line segment selected on a connector.
//------------------------------------------
void graConnectionSymbol::find_insertion(int gra_cmd_index, viewSelection* selection)
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

    selection->view_node = NULL;
    selection->connector = this->get_viewnode ();
    selection->sub_type  = 0;

    if (segments == NULL)
	return;

    // Handle picking on odd line segments
    for ( i = 1; i < num_segments; i += 2 ){
	GraphPath *p = segments[i].path;
	if(p->cmd_start <= gra_cmd_index && p->cmd_end >= gra_cmd_index){
	    selection->view_node = segments[i].viewnode;
	    return;
	}
    }

    // Handle picking of even line segments
    for ( i = 0; i < num_segments; i += 2 ){
	GraphPath *p = segments[i].path;
	if(p->cmd_start <= gra_cmd_index && p->cmd_end >= gra_cmd_index){
	    if ( i == 0 ){	// First line segment picked
		selection->view_node = segments[i].viewnode;
		selection->connector = NULL;
		selection->sub_type  = attach[
		    ((viewConnectionNode *)get_viewnode())->
		    map_symbol()];
		return;
	    } else {		// Last two line segments picked
		selection->view_node = segments[i - 1].viewnode;
		return;
	    }
	}
    }
}

void graConnectionSymbol::set_position(float cx, float cy)
{
}

void graConnectionSymbol::MoveDX(int dx)
{
    x = dx;
    if(domain && domain_object){
	int       xx, yy, w, h;
	vrectLong rect;
	GetPosition(xx, yy, w, h);
	rect.x = xx + x; rect.y = yy + y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graConnectionSymbol::MoveDY(int dy)
{
    y = dy;
    if(domain && domain_object){
	int       xx, yy, w, h;
	vrectLong rect;
	GetPosition(xx, yy, w, h);
	rect.x = xx + x; rect.y = yy + y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
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
void graConnectionSymbol::regenerate(viewConnectionNode& VN)
{
    int start_index;
    int end_index;

    float i_cx   = 0.0;		// connection point of input
    float i_cy   = 0.0;
    float last_x = 0.0;		// location of last joint in multi-segment
    float last_y = 0.0;

    // connectors
    int   x       = 0;
    int   y       = 0;
    float delta_x = 0.0;
    float delta_y = 0.0;

    graDrawingCommands *segment = new graDrawingCommands();

#if 0
    if(connection_color == NULL){
	vcolorSpec spec;
	spec.SetRGB(0, 0, 50000);
	connection_color = vcolor::Intern(&spec);
    }
    color *fg = connection_color;
#endif
    int fgR = connection_colorR;
    int fgG = connection_colorG;
    int fgB = connection_colorB;

    struct _hcoords* hcoords;
    int    size_hcoords;
    int    i, j;
    viewSymbolNodePtr Output_VU;
    int connector_type = VN.map_symbol();

    // Delete all old line segments
    ClearCommands();
    if(num_segments)
    {
//	delete_picture_contents(this);
	delete [] segments;
	segments     = NULL;
	num_segments = 0;
    }
    // Get the first input symbol and its position. For now there is only
    // one input symbol - this logic will have to change.
    viewSymbolNodePtr First_Input = (viewSymbolNode *)(*VN.inputs)[0];
	if ( First_Input == 0 )
		return;
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

	  default:
		printf("Unknown LDR type found in graConnectionSymbol::regenerate(): %d\n", connector_type);
		x = 0;
		y = 0;
		break;
    }
    i_cx += x;
    i_cy += y;

// Get an array for storing information about all output symbols. This array
// can then be sorted.

    objArr outputs;
    Obj* el;
    ForEach (el, *(VN.outputs)) {
	if (((viewSymbolNode *)el)->get_picture())
	    outputs.insert_last (el);
    }
    
    size_hcoords = outputs.size ();
    if (size_hcoords == 0)
	return;
    
    hcoords = new _hcoords[size_hcoords];
    if ( hcoords == NULL )
        return;
    
    for ( i=0; i<size_hcoords; i++ ){

	Output_VU           = (viewSymbolNode *)outputs[i];
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
    
    segment->SetColors(fgR, fgG, fgB);
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
	    int start_index;
	    int end_index;
            if ( i == 0 )
	    {
                last_x  = i_cx + delta_x;
                last_y  = i_cy + delta_y;
		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)i_cx, (int)i_cy, (int)last_x, (int)last_y);
		end_index = segment->GetCurrentCmdIndex();
            } else {
		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)last_x, (int)last_y, (int)last_x, (int)(last_y + (hcoords[i].y - hcoords[i - 1].y)));
		end_index = segment->GetCurrentCmdIndex();
                last_y += (hcoords[i].y - hcoords[i - 1].y);
	    }
	    segment->SetColors(fgR, fgG, fgB);

            if (i == 0)
	    {
                switch(connector_type)
		{
		  case ldrcREL_IN_N_OPT:
		  case ldrcREL_IN_N_REQ:
		    add_arrow_head(segment, ARROW_RIGHT, i_cx - 7, i_cy, 7, 3);
                    /* FALLTHROUGH */

		  case ldrcREL_IN_1_OPT:
		  case ldrcREL_IN_1_REQ:
		    add_arrow_head(segment, ARROW_RIGHT, i_cx, i_cy, 7, 3); 
                    if (connector_type == ldrcREL_IN_1_OPT ||
			connector_type == ldrcREL_IN_N_OPT)
			segment->AddCircle((int)i_cx, (int)i_cy, 4);
                    break;
                }
            }

// Insert line segment into segment array along with corresponding viewnode
	    segments[i*2].path     = new GraphPath(start_index, end_index);
            segments[i*2].viewnode = hcoords[i].viewnode;
	    
	    start_index = segment->GetCurrentCmdIndex();
	    segment->AddLine((int)last_x, (int)last_y, (int)hcoords[i].x, (int)hcoords[i].y);
	    end_index = segment->GetCurrentCmdIndex();
	    switch(connector_type)
	    {
	      case ldrcREL_OUT_N_OPT:
	      case ldrcREL_OUT_N_REQ:
		add_arrow_head(segment, ARROW_RIGHT, hcoords[i].x - 7,
			       hcoords[i].y, 7, 3);
		/* FALLTHROUGH */

	      case ldrcREL_OUT_1_OPT:
	      case ldrcREL_OUT_1_REQ:
		add_arrow_head(segment, ARROW_RIGHT, hcoords[i].x,
			       hcoords[i].y, 7, 3); 
		if (connector_type == ldrcREL_OUT_1_OPT ||
		    connector_type == ldrcREL_OUT_N_OPT)
		    segment->AddCircle((int)hcoords[i].x, (int)hcoords[i].y, 4);
		break;
	    }

	    segments[i*2+1].path     = new GraphPath(start_index, end_index);
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
	    add_arrow_head(segment,ARROW_DOWN, hcoords[0].x, hcoords[0].y, 7, 3); 
	else if (i_cx < hcoords[0].x)   // horizontal right arrow
	    add_arrow_head(segment, ARROW_RIGHT, hcoords[0].x, hcoords[0].y, 7, 3);
	else                           // horizontal left arrow
	    add_arrow_head(segment, ARROW_LEFT_LONG, hcoords[0].x, hcoords[0].y, 9, 4);

      case ldrcFOR_BODY:

// These types of connectors are a straight line

        num_segments = 1;
	segments     = new _segments[num_segments];
        if ( segments == NULL )
	    return;

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)i_cx, (int)i_cy, (int)hcoords[0].x, (int)hcoords[0].y);
	end_index = segment->GetCurrentCmdIndex();
	segments[0].path     = new GraphPath(start_index, end_index);
        segments[0].viewnode = hcoords[0].viewnode;

	// thick line for GOTO

//      if (connector_type == ldrcLEFT_HORIZONTAL)
//          segment->SetBrush(pbrush3);
//      else
//          segment->SetBrush(pbrush0);
//      ((Graphic*)segment)->SetColors(fg,bg);
//      Append (segment);
        break;


      case ldrcENDELSE:

	// This type of connector is a 90 degree angle line
	// with first vertical and the second horizontal parts

	num_segments = 2;
	segments     = new _segments[num_segments];
	if ( segments == NULL )
	    return;

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)i_cx, (int)i_cy, (int)i_cx, (int)hcoords[0].y);
	end_index            = segment->GetCurrentCmdIndex();
	segments[0].path     = new GraphPath(start_index, end_index);
	segments[0].viewnode = hcoords[0].viewnode;

//	segment->SetBrush(pbrush0);
//	((Graphic*) segment)->SetColors (fg,bg);
//	Append (segment);

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)hcoords[0].x, (int)hcoords[0].y, (int)i_cx, (int)hcoords[0].y);
	end_index            = segment->GetCurrentCmdIndex();
	segments[1].path     = new GraphPath(start_index, end_index);
	segments[1].viewnode = hcoords[0].viewnode;

//	segment->SetBrush(pbrush0);
//	((Graphic*) segment)->SetColors (fg,bg);
//	Append (segment);

	// add an arrow <-

	add_arrow_head(segment, ARROW_LEFT, hcoords[0].x, hcoords[0].y, 7, 3);
	break;

     case ldrcFALSE_BRANCH:

// This type of connector is a 90 degree angle line

        num_segments = 2;
	segments = new _segments[num_segments];
        if ( segments == NULL )
	    return;

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)i_cx, (int)i_cy, (int)hcoords[0].x, (int)i_cy);
	end_index            = segment->GetCurrentCmdIndex();
        segments[0].path     = new GraphPath(start_index, end_index);
        segments[0].viewnode = hcoords[0].viewnode;

//        segment->SetBrush(pbrush0);
//        ((Graphic*) segment)->SetColors (fg,bg);
//        Append (segment);

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)hcoords[0].x, (int)i_cy, (int)hcoords[0].x, (int)hcoords[0].y);
	end_index            = segment->GetCurrentCmdIndex();
        segments[1].path     = new GraphPath(start_index, end_index);
        segments[1].viewnode = hcoords[0].viewnode;
        break;

     case ldrcCASE_EXIT:
     case ldrcLOOP_DO:

	// add arrow heads

	// horizontal left arrow
	add_arrow_head(segment, ARROW_LEFT, i_cx - VIEW_CONN_SHIFT,  i_cy, 7, 3); 
	// horizontal right arrow
	add_arrow_head(segment, ARROW_RIGHT, hcoords[0].x, hcoords[0].y, 7, 3);

// This type of connector is a 3 segment lines
//   ----
//   |
//   ----
//  or
//   ----
//      |
//      |----  - can degenerate into a straight line

        num_segments = 3;
	segments     = new _segments[num_segments];
        if ( segments == NULL )
	    return;

        delta_x = - VIEW_CONN_SHIFT;

	start_index = segment->GetCurrentCmdIndex();
        segment->AddLine((int)i_cx, (int)i_cy, (int)(i_cx + delta_x), (int)i_cy);
	end_index            = segment->GetCurrentCmdIndex();
        segments[0].path     = new GraphPath(start_index, end_index);
        segments[0].viewnode = hcoords[0].viewnode;

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)(i_cx + delta_x), (int)i_cy, (int)(i_cx + delta_x), (int)hcoords[0].y);
	end_index        = segment->GetCurrentCmdIndex();
        segments[1].path = new GraphPath(start_index, end_index);
        segments[1].viewnode = hcoords[0].viewnode;

	start_index = segment->GetCurrentCmdIndex();
	segment->AddLine((int)(i_cx + delta_x), (int)hcoords[0].y, (int)hcoords[0].x, (int)hcoords[0].y);
	end_index        = segment->GetCurrentCmdIndex();
        segments[2].path = new GraphPath(start_index, end_index);
        segments[2].viewnode = hcoords[0].viewnode;

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
	    segments     = new _segments[num_segments];
	    if ( segments == NULL )
		return;

	    // try horizontal line;  x, y - attachment point offsets
	    if ( i_cy < (hcoords[0].y + y) &&
		 i_cy > (hcoords[0].y - y) )
		tmp_y = i_cy;
	    else
		tmp_y = hcoords[0].y;

	    start_index = segment->GetCurrentCmdIndex();
	    segment->AddLine((int)i_cx, (int)i_cy,(int)hcoords[0].x, (int)tmp_y);
	    end_index            = segment->GetCurrentCmdIndex();
	    segments[0].path     = new GraphPath(start_index, end_index);
	    segments[0].viewnode = First_Input;
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

	    start_index = segment->GetCurrentCmdIndex();
	    segment->AddLine((int)i_cx, (int)i_cy, (int)tree_offset, (int)tmp_y);
	    end_index            = segment->GetCurrentCmdIndex();
	    segments[0].path     = new GraphPath(start_index, end_index);
	    segments[0].viewnode = First_Input;

	    for (i = 1, j = 0; i < size_hcoords * 2; j++, i += 2 )
	    {
		if (hcoords[j].y >= tmp_y)
		    break;

		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)tree_offset, (int)hcoords[j].y, (int)hcoords[j].x, (int)hcoords[j].y );
		end_index = segment->GetCurrentCmdIndex();
		segments[i].path     = new GraphPath(start_index, end_index);
		segments[i].viewnode = hcoords[j].viewnode;

		aux_y = hcoords[j + 1].y;
		if (aux_y >= tmp_y)
		    aux_y = tmp_y;

		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)tree_offset, (int)hcoords[j].y, (int)tree_offset, (int)aux_y);
		end_index = segment->GetCurrentCmdIndex();
		segments[i+1].path     = new GraphPath(start_index, end_index);
	        segments[i+1].viewnode = First_Input;
	    }

	    for (; i < size_hcoords * 2; j++, i += 2)
	    {
		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)tree_offset, (int)hcoords[j].y, (int)hcoords[j].x, (int)hcoords[j].y);
		end_index = segment->GetCurrentCmdIndex();
		segments[i].path     = new GraphPath(start_index, end_index);
                segments[i].viewnode = hcoords[j].viewnode;

		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)tree_offset, (int)aux_y, (int)tree_offset, (int)hcoords[j].y);
		end_index = segment->GetCurrentCmdIndex();
		segments[i+1].path     = new GraphPath(start_index, end_index);
                segments[i+1].viewnode = First_Input;
		aux_y                  = hcoords[j].y;
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

	    start_index = segment->GetCurrentCmdIndex();
	    segment->AddLine((int)i_cx, (int)i_cy, (int)tmp_x, (int)hcoords[0].y);
	    end_index = segment->GetCurrentCmdIndex();
	    segments[0].path     = new GraphPath(start_index, end_index);
	    segments[0].viewnode = First_Input;
	}
	else			//   multiple outputs : one-to-many connector
	{
	    float tree_offset = hcoords[0].y - 7; /* 7 = node_y_space /2 */
	    /* see view_edge_init  */
	    num_segments = 2 * size_hcoords;
	    segments     = new _segments[num_segments];
	    if ( segments == NULL )
		return;

	    // line segment from input
	    if ( hcoords[0].x  < i_cx && // vertical
		 hcoords[size_hcoords -1].x  > i_cx )
		tmp_x = i_cx;
	    else		// to the middle
		tmp_x = 0.5 * (hcoords[0].x +
			       hcoords[size_hcoords -1].x);
	    start_index = segment->GetCurrentCmdIndex();
            segment->AddLine((int)i_cx, (int)i_cy, (int)tmp_x, (int)tree_offset);
	    end_index = segment->GetCurrentCmdIndex();
	    segments[0].path     = new GraphPath(start_index, end_index);
	    segments[0].viewnode = First_Input;

	    // connect ouputs
	    for ( i=1, j=0; i < size_hcoords * 2; j++, i+=2 )
	    {
		start_index = segment->GetCurrentCmdIndex();
		segment->AddLine((int)hcoords[j].x, (int)tree_offset, (int)hcoords[j].x, (int)hcoords[j].y);
		end_index = segment->GetCurrentCmdIndex();
		segments[i].path     = new GraphPath(start_index, end_index);
		segments[i].viewnode = hcoords[j].viewnode;
		// horizontal line segment
		if ( j < size_hcoords -1)
		{
		    start_index = segment->GetCurrentCmdIndex();
		    segment->AddLine((int)hcoords[j].x, (int)tree_offset, (int)hcoords[j+1].x, (int)tree_offset);
		    end_index = segment->GetCurrentCmdIndex();
		    segments[i+1].path     = new GraphPath(start_index, end_index);
		    segments[i+1].viewnode = First_Input;
		}
	    }
	}
#endif
	break;
    }
    
    AddCommands(segment);
    delete [] hcoords;		// Deallocate the sorting array
    
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}


//------------------------------------------
// graAttachableSymbol
//------------------------------------------

graAttachableSymbol::graAttachableSymbol() : graDrawingCommands()
{
}

graAttachableSymbol::~graAttachableSymbol()
{
}

graSymbolType graAttachableSymbol::SymbolType()
{
    return graAttachableSymbolType;
}

void graAttachableSymbol::get_attachment_point(graAttachmentPoint pt, int& outx, int& outy)
{
    outx = x[pt];
    outy = y[pt];
}

void graAttachableSymbol::clear_attachment_points()
{
    for(int i = 0; i < graNumAttachmentPoints; i++)
        x[i] = y[i] = 0;
}

//------------------------------------------
// graParametricSymbol
//------------------------------------------

graParametricSymbol::graParametricSymbol(int symbol, int layers) : graAttachableSymbol()
{
    domain        = NULL;
    domain_object = NULL;
    false_bottom1 = NULL;
    false_bottom2 = NULL;
    scalex        = (float)1;
    scaley        = (float)1;
    labelbox_w    = 70;
    labelbox_h    = 20;
    x             = 0;
    y             = 0;
    this->symbol  = symbol;
    // Initialize modification housekeeping
    modified_token     = NULL;
    modification_start = modification_end = -1;
    for (int i = 0; i < GRA_MAX_MODES; i++)
	     param_overlay[i] = NULL;
	text_mode = graTextModeUnknown;
}
 
vfont *ui_get_font(void);

graParametricSymbol::graParametricSymbol(int symbol, int layers, char* text,
					 graTextPositions position)
{
    graWorld *gra_world = driver_instance->graphics_world;
    x                   = 0;
    y                   = 0;
    domain              = NULL;
    domain_object       = NULL;
    false_bottom1       = NULL;
    false_bottom2       = NULL;
    scalex              = (float)1;
    scaley              = (float)1;
    this->symbol        = symbol;
    this->layers        = (enum graSymbolModes)layers;
    if(text == NULL || text[0] == 0)
	text = gra_world->get_text(symbol);
    if(text == NULL)
	text = "";
    this->text = strdup(text);
    text_mode  = position;
    text_width = 70;
    labelbox_w = 70;
    labelbox_h = 20;
    SetFont(ui_get_font());

    // Initialize modification housekeeping
    modified_token = NULL;
    modification_start = modification_end = -1;

    for (int i = 0; i < GRA_MAX_MODES; i++)
	param_overlay[i] = NULL;

    if(text_mode == graTextModeUnknown)
	text_mode = (graTextPositions)driver_instance->graphics_world->get_text_mode(symbol);
    
    param_overlay[graLayerNoNormal]       = gra_world->get_pictel(symbol, graLayerNoNormal);
    param_overlay[graLayerNoExpanded]     = gra_world->get_pictel(symbol, graLayerNoExpanded);
    param_overlay[graLayerNoCollapsed]    = gra_world->get_pictel(symbol, graLayerNoCollapsed);
    param_overlay[graLayerNoDecorationCL] = gra_world->get_pictel(symbol, graLayerNoDecorationCL);
    param_overlay[graLayerNoDecorationUL] = gra_world->get_pictel(symbol, graLayerNoDecorationUL);
    param_overlay[graLayerNoDecorationLL] = gra_world->get_pictel(symbol, graLayerNoDecorationLL);
    
    // printf("(sym : %d %d )\n", symbol,  param_overlay[graLayerNoNormal]->GetCurrentCmdIndex());

    OSapi_bcopy(gra_world->get_pictel(symbol, graLayerNoNormal)->x, graAttachableSymbol::x, sizeof(graAttachableSymbol::x));
    OSapi_bcopy(gra_world->get_pictel(symbol, graLayerNoNormal)->y, graAttachableSymbol::y, sizeof(graAttachableSymbol::y));

    // Hilite layer
    param_overlay[graLayerNoHilited] = gra_world->get_pictel(symbol, graLayerNoHilited);
    // Selected Layer
    param_overlay[graLayerNoSelected] = gra_world->get_pictel(symbol, graLayerNoSelected);
    // Right half
    param_overlay[graLayerNoNote]         = gra_world->get_pictel(symbol, graLayerNoNote);
    param_overlay[graLayerNoDecorationCR] = gra_world->get_pictel(symbol, graLayerNoDecorationCR);
    param_overlay[graLayerNoDecorationUR] = gra_world->get_pictel(symbol, graLayerNoDecorationUR);
    param_overlay[graLayerNoDecorationLR] = gra_world->get_pictel(symbol, graLayerNoDecorationLR);

    domain_object = NULL;
    set_layers(layers);
}
 
graParametricSymbol::~graParametricSymbol()
{
}

graSymbolType graParametricSymbol::SymbolType()
{
    return graParametricSymbolType;
}

void graParametricSymbol::SetFont(vfont *f)
{
    vrect bounds;

    font = f;
    font->StringBounds((vchar *)text, &bounds);
    labelbox_w = bounds.w + 10;
//    labelbox_h = bounds.h + 10;
    text_width = labelbox_w;
}

void LongRectShowCenter(char *text, vrectLong& rect, vfont *font)
{
#if 0
    vrect     str_rect;
    vrectLong text_rect;
	
    vdrawGSave();
    font->StringBounds((vchar *)text, &str_rect);
    text_rect.x = str_rect.x;
    text_rect.y = str_rect.y;
    text_rect.h = str_rect.h;
    text_rect.w = str_rect.w;
    
        if(text_rect.w > rect.w)
	    text_rect.w = rect.w;
	if(text_rect.h > rect.h)
	    text_rect.h = rect.h;
	text_rect.x = rect.x + ((rect.w - text_rect.w) >> 1);
	text_rect.y = rect.y + ((rect.h - text_rect.h) >> 1);
	vdrawGRestore();
#endif
    vdrawGSave();
    vdraw::Translate(rect.x, rect.y);
    vrect str_rect;
    str_rect.x = str_rect.y = 0;
    str_rect.w = rect.w;
    str_rect.h = rect.h;
    vdraw::RectShowCenter((vchar *)text, &str_rect);
    vdrawGRestore();
}

void graParametricSymbol::DrawTextBox(int xoffset, int yoffset)
{
    vrectLong  rect;
    vcolorSpec spec;
    vcolor     *color;

    rect.x     = xoffset;
    rect.y     = yoffset;
    rect.w     = (int)((float)labelbox_w * scalex);
    rect.h     = (int)((float)labelbox_h * scaley);
    labelbox_x = xoffset;
    labelbox_y = yoffset;
    if((layers & graSymbolModeSelected) != 0){
	vrectFloat frect;
	frect.x = rect.x;
	frect.y = rect.y;
	frect.w = rect.w;
	frect.h = rect.h;
	vdrawGSave();
	spec.SetRGB(vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75);
	color = vcolor::Intern(&spec);
	vdrawSetColor(color);
	vdraw::FRectsFill(&frect, 1);
	vdrawGRestore();
    }
    vdraw::FMoveTo((double)rect.x, (double)rect.y);
    vdraw::FLineTo((double)(rect.x + rect.w), (double)rect.y);
    vdraw::FMoveTo((double)rect.x, (double)(rect.y + rect.h));
    vdraw::FLineTo((double)(rect.x + rect.w), (double)(rect.y + rect.h));
    vdraw::Stroke();
    vdrawGSave();
    spec.SetRGB(0, 0, 0);
    color = vcolor::Intern(&spec);
    vdrawSetColor(color);
    if(scalex < .99 || scalex > 1.01)
	vdraw::SetFont(font->FScale(font->GetFace(), font->GetSize() * scalex));
//    vdraw::RectShowCenter((vchar *)text, &rect);
    LongRectShowCenter(text, rect, font);
    vdrawGRestore();
}

void graParametricSymbol::GetTextBoxBounds(vrectLong &rect)
{
    rect.x = 0;
    rect.y = 0;
    rect.w = (int)((float)labelbox_w * scalex);
    rect.h = (int)((float)labelbox_h * scaley);
}

int graParametricSymbol::TextBoxContainsPoint(int px, int py)
{
    vrect rect;
    rect.x     = 0;
    rect.y     = 0;
    rect.w     = (short)((float)labelbox_w * scalex);
    rect.h     = (short)((float)labelbox_h * scaley);
    return rect.ContainsPoint(px, py);
}

void graParametricSymbol::DrawDecoration(graLayerNo layer, int x, int y)
{
    graDrawingCommands *drawing = param_overlay[layer];
    if(drawing == NULL){
	printf("Unknown layer specified !\n");
	return;
    }
    drawing->Execute(x, y, scalex, scaley);
}

int graParametricSymbol::DecorationPointTextInside(int px, int py)
{
    vrectLong rect;
    vrectLong bounds;
    int       bounds_merge = 0;

    px -= x;
    py -= y;
    if((layers & graSymbolModeDecorationCL) != 0 && param_overlay[graLayerNoDecorationCL]!=NULL){
		param_overlay[graLayerNoDecorationCL]->GetBounds(rect, scalex, scaley);
		if(rect.ContainsPoint(px, py))
			return graLayerNoDecorationCL;
		bounds       = rect;
		bounds_merge = 1;
    }
    if((layers & graSymbolModeDecorationUL) != 0 && param_overlay[graLayerNoDecorationUL]!=NULL){
		param_overlay[graLayerNoDecorationUL]->GetBounds(rect, scalex, scaley);
		if(rect.ContainsPoint(px, py))
			return graLayerNoDecorationUL;
		if(bounds_merge)
			bounds.Union(&bounds, &rect);
		else
			bounds = rect;
		bounds_merge = 1;
    }
    if((layers & graSymbolModeDecorationLL) != 0 && param_overlay[graLayerNoDecorationLL]!=NULL){
		param_overlay[graLayerNoDecorationLL]->GetBounds(rect, scalex, scaley);
		if(rect.ContainsPoint(px, py))
			return graLayerNoDecorationLL;
		if(bounds_merge)
			bounds.Union(&bounds, &rect);
		else
			bounds = rect;
		bounds_merge = 1;
    }
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    if(rect.ContainsPoint(px, py))
	return graLayerNoNormal;
    if((layers & graSymbolModeDecorationCL) != 0)
	bounds.Union(&bounds, &rect);
    else
	bounds = rect;
    px -= bounds.x + bounds.w;
    if(bounds.h > labelbox_h)
	labelbox_h = bounds.h;
    if(TextBoxContainsPoint(px, py))
	return 0xFFF;
    GetTextBoxBounds(rect);
    bounds.w += rect.w;
    px       -= rect.w;
    if((layers & graSymbolModeDecorationUR) != 0 && param_overlay[graLayerNoDecorationUR]!=NULL){
	param_overlay[graLayerNoDecorationUR]->GetBounds(rect, scalex, scaley);
	if(rect.ContainsPoint(px, py))
	    return graLayerNoDecorationUR;
    }
    if((layers & graSymbolModeDecorationLR) != 0 && param_overlay[graLayerNoDecorationLR]!=NULL){
	param_overlay[graLayerNoDecorationLR]->GetBounds(rect, scalex, scaley);
	if(rect.ContainsPoint(px, py))
	    return graLayerNoDecorationLR;
    }
    if((layers & graSymbolModeDecorationCR) != 0 && param_overlay[graLayerNoDecorationCR]!=NULL){
	param_overlay[graLayerNoDecorationCR]->GetBounds(rect, scalex, scaley);
	if(rect.ContainsPoint(px, py))
	    return graLayerNoDecorationCR;
    }
	if(param_overlay[graLayerNoNote]!=NULL)
		param_overlay[graLayerNoNote]->GetBounds(rect, scalex, scaley);
    if(rect.ContainsPoint(px, py))
	return graLayerNoNote;
    return -1;
}

void graParametricSymbol::DrawTextInside(vrectLong& bounds)
{
    vrectLong rect;
    int       bounds_merge = 0;

    int xoffset = x;
    int yoffset = y;
    if((layers & graSymbolModeDecorationUL) != 0){
	DrawDecoration(graLayerNoDecorationUL, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationUL]!=NULL) 
		param_overlay[graLayerNoDecorationUL]->GetBounds(rect, scalex, scaley);
	bounds       = rect;
	bounds_merge = 1;
    }
    if((layers & graSymbolModeDecorationLL) != 0){
	DrawDecoration(graLayerNoDecorationLL, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationLL]!=NULL)
		param_overlay[graLayerNoDecorationLL]->GetBounds(rect, scalex, scaley);
	if(bounds_merge)
	    bounds.Union(&bounds, &rect);
	else
	    bounds = rect;
	bounds_merge = 1;
    }
    if((layers & graSymbolModeDecorationCL) != 0){
	DrawDecoration(graLayerNoDecorationCL, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationCL]!=NULL)
		param_overlay[graLayerNoDecorationCL]->GetBounds(rect, scalex, scaley);
	if(bounds_merge)
	    bounds.Union(&bounds, &rect);
	else
	    bounds = rect;
	bounds_merge = 1;
    }
    DrawDecoration(graLayerNoNormal, xoffset, yoffset);
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    if(bounds_merge)
	bounds.Union(&bounds, &rect);
    else
	bounds = rect;
    xoffset += bounds.x + bounds.w;
    if(bounds.h > labelbox_h)
	labelbox_h = bounds.h;
    DrawTextBox(xoffset, yoffset);
    GetTextBoxBounds(rect);
    bounds.w += rect.w;
    if(bounds.h < rect.h)
	bounds.h = rect.h;
    xoffset += rect.w;
    DrawDecoration(graLayerNoNote, xoffset, yoffset);
	if(param_overlay[graLayerNoNote]!=NULL)
		param_overlay[graLayerNoNote]->GetBounds(rect, scalex, scaley);
    int right_width = rect.x + rect.w;
    if(bounds.h < rect.h)
	bounds.h = rect.h;
    if((layers & graSymbolModeDecorationUR) != 0){
	DrawDecoration(graLayerNoDecorationUR, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationUR]!=NULL)
		param_overlay[graLayerNoDecorationUR]->GetBounds(rect, scalex, scaley);
	if(rect.x + rect.w > right_width)
	    right_width = rect.x + rect.w;
	if(bounds.h < rect.h)
	    bounds.h = rect.h;
    }
    if((layers & graSymbolModeDecorationLR) != 0){
	DrawDecoration(graLayerNoDecorationLR, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationLR]!=NULL)
		param_overlay[graLayerNoDecorationLR]->GetBounds(rect, scalex, scaley);
	if(rect.x + rect.w > right_width)
	    right_width = rect.x + rect.w;
	if(bounds.h < rect.h)
	    bounds.h = rect.h;
    }
    if((layers & graSymbolModeDecorationCR) != 0){
	DrawDecoration(graLayerNoDecorationCR, xoffset, yoffset);
	if(param_overlay[graLayerNoDecorationCR]!=NULL)
		param_overlay[graLayerNoDecorationCR]->GetBounds(rect, scalex, scaley);
	if(rect.x + rect.w > right_width)
	    right_width = rect.x + rect.w;
	if(bounds.h < rect.h)
	    bounds.h = rect.h;
    }
    bounds.w += right_width;
    if(false_bottom1 != NULL){
	int xx, yy, ww, hh;
        false_bottom1->GetPosition(xx, yy, ww, hh);
	false_bottom1->Execute(xx + x, yy + y, scalex, scaley);
	if(bounds.y > yy){
	    bounds.h += bounds.y - yy;
	    bounds.y  = yy;
	}
    }
    if(false_bottom2 != NULL){
	int xx, yy, ww, hh;
	false_bottom2->GetPosition(xx, yy, ww, hh);
	false_bottom2->Execute(xx + x, yy + y, scalex, scaley);
	if(bounds.y > yy){
	    bounds.h += bounds.y - yy;
	    bounds.y  = yy;
	}
    }
}

int graParametricSymbol::DecorationPointTextPositioned(int px, int py)
{
    vrectLong rect;
    
    px -= x;
    py -= y;
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    if(rect.ContainsPoint(px, py))
	return graLayerNoNormal;
    rect.x  = 5;
    rect.y  = 0;
    rect.w -= 10;
    if(rect.ContainsPoint(px, py))
	return 0xFFF;
    return -1;
}

void graParametricSymbol::DrawTextPositioned(vrectLong& bounds)
{
    vrectLong rect;

    int xoffset = x;
    int yoffset = y;
    DrawDecoration(graLayerNoNormal, xoffset, yoffset);
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    bounds = rect;

    rect.x  = xoffset + 5;
    rect.y  = yoffset;
    if(rect.w > 10)
	rect.w -= 10;
    vcolor	  *color;
    vcolorSpec	  spec;
    if((layers & graSymbolModeSelected) != 0){
	vrect      str_rect;
	vrectFloat text_rect;
	font->StringBounds((vchar *)text, &str_rect);
	text_rect.x = str_rect.x;
	text_rect.y = str_rect.y;
	text_rect.h = str_rect.h;
	text_rect.w = str_rect.w + 4;
	if(text_rect.w > rect.w)
	    text_rect.w = rect.w;
	if(text_rect.h > rect.h)
	    text_rect.h = rect.h;
	text_rect.x = rect.x + ((rect.w - (int)text_rect.w) >> 1);
	text_rect.y = rect.y + ((rect.h - (int)text_rect.h) >> 1);
	vdrawGSave();
	spec.SetRGB(vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75);
	color = vcolor::Intern(&spec);
	vdrawSetColor(color);
	vdraw::FRectsFill(&text_rect, 1);
	vdrawGRestore();
    }
    if(text != NULL && text[0] != '\0' && rect.x > 0 && rect.y > 0 && rect.w > 0 && rect.h > 0){
	vdrawGSave();
	spec.SetRGB(0, 0, 0);
	color = vcolor::Intern(&spec);
	vdrawSetColor(color);
	
	if(scalex < .99 || scalex > 1.01)
	    vdraw::SetFont(font->FScale(font->GetFace(), font->GetSize() * scalex));
	vrect r;
	r.x = rect.x;
	r.y = rect.y;
	r.h = rect.h;
	r.w = rect.w;
//	printf("text : %s\nrect: %d %d %d %d\n", text, r.x, r.y, r.w, r.h);
	vdraw::RectShowCenter((vchar *)text, &r);
	//    LongRectShowCenter(text, rect, font);
	vdrawGRestore();
    }
}

int graParametricSymbol::DecorationPointTextRight(int px, int py)
{
    vrectLong rect;
    vrectLong bounds;

    px -= x;
    py -= y;
    if((layers & graSymbolModeExpanded) != 0 && param_overlay[graLayerNoExpanded]!=NULL){
	param_overlay[graLayerNoExpanded]->GetBounds(rect, scalex, scaley);
	rect.w += 2;
	rect.x -= 1;
	rect.y -= 1;
	rect.h += 2;
	if(rect.ContainsPoint(px, py))
	    return graLayerNoExpanded;
	bounds = rect;
    } else if((layers & graSymbolModeCollapsed) != 0 && param_overlay[graLayerNoCollapsed]!=NULL){
	param_overlay[graLayerNoCollapsed]->GetBounds(rect, scalex, scaley);
	rect.w += 2;
	rect.x -= 1;
	rect.y -= 1;
	rect.h += 2;
 	if(rect.ContainsPoint(px, py))
	    return graLayerNoCollapsed;
	bounds = rect;
    }
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    if(rect.ContainsPoint(px, py))
	return graLayerNoNormal;
    if((layers & (graSymbolModeCollapsed | graSymbolModeExpanded)) != 0)
	bounds.Union(&bounds, &rect);
    else
	bounds = rect;
    if(bounds.h < labelbox_h)
	bounds.h = labelbox_h;
    rect.x    = bounds.x + bounds.w + 3;
    bounds.w += 10 + text_width; 
    rect.y    = 0;
    rect.w    = text_width + 5;
    if(rect.ContainsPoint(px, py))
	return 0xFFF;
    return -1;
}

void graParametricSymbol::DrawTextRight(vrectLong& bounds)
{
    vrectLong rect;

    int xoffset = x;
    int yoffset = y;
    if((layers & graSymbolModeExpanded) != 0 && param_overlay[graLayerNoExpanded]!=NULL){
	DrawDecoration(graLayerNoExpanded, xoffset, yoffset);
	param_overlay[graLayerNoExpanded]->GetBounds(rect, scalex, scaley);
	bounds = rect;
    } else if((layers & graSymbolModeCollapsed) != 0 && param_overlay[graLayerNoCollapsed]!=NULL){
	DrawDecoration(graLayerNoCollapsed, xoffset, yoffset);
	param_overlay[graLayerNoCollapsed]->GetBounds(rect, scalex, scaley);
	bounds = rect;
    }
    DrawDecoration(graLayerNoNormal, xoffset, yoffset);
	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    if((layers & (graSymbolModeCollapsed | graSymbolModeExpanded)) != 0)
	bounds.Union(&bounds, &rect);
    else
	bounds = rect;
    if(bounds.h < labelbox_h)
	bounds.h = labelbox_h;
    rect.x    = xoffset + bounds.x + bounds.w + 3;
    bounds.w += 10 + text_width; 
    rect.y    = yoffset;
    rect.w    = text_width + 5;
    vcolor     *color;
    vcolorSpec spec;
    if((layers & graSymbolModeSelected) != 0){
	vrectFloat frect;
	frect.x = rect.x;
	frect.y = rect.y;
	frect.w = rect.w;
	frect.h = rect.h;
	vdrawGSave();
	spec.SetRGB(vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75,
		    vcolorCOMPONENT_MAX*0.75);
	color = vcolor::Intern(&spec);
	vdrawSetColor(color);
	vdraw::FRectsFill(&frect, 1);
	vdrawGRestore();
    }
    vdrawGSave();
    spec.SetRGB(0, 0, 0);
    color = vcolor::Intern(&spec);
    vdrawSetColor(color);
    if(scalex < .99 || scalex > 1.01)
	vdraw::SetFont(font->FScale(font->GetFace(), font->GetSize() * scalex));
//    vdraw::RectShowCenter((vchar *)text, &rect);
    LongRectShowCenter(text, rect, font);
    vdrawGRestore();
}

void graParametricSymbol::Draw(int& cx, int& cy, int& w, int& h, bool include_text)
{
    vrectLong bounds;

    switch(text_mode) {
      case graTextModeInside:
	DrawTextInside(bounds);
	break;
      case graTextModePositioned:
	DrawTextPositioned(bounds);
	break;
      case graTextModeRight:
	DrawTextRight(bounds);
	break;
      default:
	bounds.x = 0;
	bounds.y = 0;
	bounds.w = 0;
	bounds.h = 0;
	break;
    };
    cx = bounds.x;
    cy = bounds.y;
    w  = bounds.w;
    h  = bounds.h;
}

int graParametricSymbol::DecorationPoint(int px, int py)
{
    switch(text_mode) {
      case graTextModeInside:
	return DecorationPointTextInside(px, py);
      case graTextModePositioned:
	return DecorationPointTextPositioned(px, py);
      case graTextModeRight:
	return DecorationPointTextRight(px, py);
      default:
	return -1;
    };
}

void graParametricSymbol::DrawSelected(int& cx, int& cy, int& w, int& h, bool include_text)
{
}

void graParametricSymbol::GetPosition(int &x, int &y, int &w, int &h)
{
    float cx, cy;
    float fw, fh;
    
    get_symbol_size(cx, cy, fw, fh);
    x = this->x + cx;
    y = this->y + cy;
    w = (int)fw;
    h = (int)fh;
}

void graParametricSymbol::SetDomainAndObject(vdomain *vd, vdomainObject *obj)
{
    domain        = vd;
    domain_object = obj;
}

vdomainObject *graParametricSymbol::GetDomainObject(void)
{
    return domain_object;
}
 
void graParametricSymbol::do_highlight(void)
{
}
 
void graParametricSymbol::undo_highlight(void)
{
}

void graParametricSymbol::get_symbol_size(float& cx, float& cy, float& w, float& h,
					  bool include_text)
{
    int xx, yy, width, height;

    vdraw::GSave();
    vdraw::NullDevice();
    Draw(xx, yy, width, height, include_text);
    vdraw::GRestore();
    cx = (float)xx;
    cy = (float)yy;
    w = (float)width;
    h = (float)height;
}

void graParametricSymbol::set_layers(int layers)
{
    this->layers = (graSymbolModes) layers;
    if(text_mode == graTextModeInside)
	this->layers = (graSymbolModes)((int)(this->layers) | (int)graSymbolModeNoteMarker);
    
    graWorld *gra_world = driver_instance->graphics_world;
    OSapi_bcopy(gra_world->get_pictel(symbol, graLayerNoNormal)->x, graAttachableSymbol::x, sizeof(graAttachableSymbol::x));
    OSapi_bcopy(gra_world->get_pictel(symbol, graLayerNoNormal)->y, graAttachableSymbol::y, sizeof(graAttachableSymbol::y));
    for (int j = 0; j < graNumAttachmentPoints; j++){
	int layer_x, layer_y;
	get_attachment_point(graAttachmentPoint(j), layer_x, layer_y);
	if (layer_x != 0 || layer_y != 0){
	    if (text_mode == graTextModeInside ||
		text_mode == graTextModeInsideNoBox){
		if (j == graAttachLC || j == graAttachUC)
		    layer_x += text_width / 2;
		else if (j == graAttachLR ||
			 j == graAttachCR ||
			 j == graAttachUR)
		    layer_x += text_width;
	    }

	    graAttachableSymbol::x[j] = layer_x;
	    graAttachableSymbol::y[j] = int(layer_y * scaley + 0.5);
	}
    }
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}      
 
void graParametricSymbol::add_layers(int layers)
{
    set_layers (this->layers | layers);
}
 
void graParametricSymbol::rem_layers(int layers)
{
    set_layers (this->layers & ~layers);
}

int graParametricSymbol::get_layers()
{
    return this->layers;
}

void graParametricSymbol::SetObjectBounds(vrectLong *rect)
{
    if(domain && domain_object){
	rect->x -= 1;
	rect->y -= 1;
	rect->w += 2;
	rect->h += 2;
	domain->SetObjectBoundsLong(vdomainObjectToSet(domain_object), rect);
    }
}

void graParametricSymbol::set_position(float cx, float cy)
{
    x = (int)cx;
    y = (int)cy;
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graParametricSymbol::MoveDX(int dx)
{
    x += dx;
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

void graParametricSymbol::MoveDY(int dy)
{
    y += dy;
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
}

int graParametricSymbol::PointNear(int px, int py, int distance)
{
    return ExecutePointNearCheck(x, y, scalex, scaley, px, py, distance);
}

void graParametricSymbol::set_scale(float sx, float sy)
{
    int   x, y, w, h;
    vrect rect;
    GetPosition(x, y, w, h);
    scalex = sx / (float)w;
    scaley = sy / (float)h;
}

int graParametricSymbol::get_text_width()
{
    return text_width;
}
 
void graParametricSymbol::set_text_width(int new_width)
{
    labelbox_w = new_width;
    text_width = new_width;
    set_layers(layers);
}

void graParametricSymbol::set_text_focus(int p_focus)
{
}

void graParametricSymbol::set_char_marker(int pos)
{
}

int graParametricSymbol::handle_text_input(unsigned char *string, int len,
					   int control, int meta, int shift, int shiftlock)
{
    return 0;
}

void graParametricSymbol::replace_text(char*)
{
}

int graParametricSymbol::locate_text_position(int x, int y)
{
    return 0;
}

void graParametricSymbol::shift_cursor(int pos)
{
}

int graParametricSymbol::report_text_update()
{
    return 0;
}

void graParametricSymbol::hilite_text(int start_pos, int end_pos)
{
}

void graParametricSymbol::unhilite_text(int pos)
{
}
 
void graParametricSymbol::unhilite_all()
{
}
 
void graParametricSymbol::hilite_modification_text()
{
}
 
void graParametricSymbol::set_popup_item_sensitive(char*, int)
{
}

void graParametricSymbol::add_bottom_line()
{
}
 
void graParametricSymbol::remove_bottom_line()
{
}

graParametricSymbol* graParametricSymbol::create_false_bottom()
{
    vrectLong rect;
    int       start_x;
    int       len;

	if(param_overlay[graLayerNoNormal]!=NULL)
		param_overlay[graLayerNoNormal]->GetBounds(rect, scalex, scaley);
    start_x = rect.x;
    len     = rect.w;
	if(param_overlay[graLayerNoNote]!=NULL)
		param_overlay[graLayerNoNote]->GetBounds(rect, scalex, scaley);
    len += text_width + rect.w;
    graParametricSymbol* fb = new graParametricSymbol(-1, 0);
    fb->AddLine(start_x, 0, start_x + len, 0);
    fb->AddLine(start_x, FALSE_BOTTOM_HEIGHT, start_x, 0);
    fb->AddLine(start_x + len, FALSE_BOTTOM_HEIGHT, start_x + len, 0);
    if (false_bottom1)
	false_bottom2 = fb;
    else
	false_bottom1 = fb;
    if(domain && domain_object){
	int       x, y, w, h;
	vrectLong rect;
	GetPosition(x, y, w, h);
	rect.x = x; rect.y = y; rect.w = w; rect.h = h;
	SetObjectBounds(&rect);
    }
    return fb;
}
 
void graParametricSymbol::remove_false_bottom(graParametricSymbol *fb)
{
    if (false_bottom1 == fb){
	false_bottom1 = false_bottom2;
	false_bottom2 = NULL;
    } else if (false_bottom2 == fb)
	false_bottom2 = NULL;
}

NodePopupInfo *graParametricSymbol::generate_popup()
{
    return viewnode->generate_popup_menu();
}

void graParametricSymbol::shift_cursor_left ()
{
}
 
void graParametricSymbol::shift_cursor_right ()
{
}

Obj* graParametricSymbol::get_token_to_be_modified()
{
    return NULL;
}

void graParametricSymbol::set_modified_token()
{
}

void graParametricSymbol::reset_modified_token()
{
}

void graParametricSymbol::set_modification_start_end_pos(Obj* modified_token)
{
}


//------------------------------------------
// find_insertion
//
// This function determines the insertion point if a symbol was picked.
//------------------------------------------

void graParametricSymbol::find_insertion(int gra_cmd_index, viewSelection* selection)
{
    selection->view_node = this->get_viewnode ();
    selection->connector = NULL;
    selection->sub_type  = 0;

#if 0
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
#endif
}
