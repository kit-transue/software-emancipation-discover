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
////////////////////////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////////////////////////

//++X11
#include <X11/Xlib.h>
//--X11

//++MOTIF
#include <Xm/Xm.h>
//--MOTIF

//++XBAE
#include <Xbae/Matrix.h>
//--XBAE

//++GT
#include <gt.h>
#include <gtMatrix.h>
#include <gtMatrixXm.h>
#include <gtPixmap.h>
#include <gtPixmapXm.h>
//--GT

void help_callback(gtMatrixCB callback, void *client_data)
{
}

int gtMatrixXm::num_rows(void)
{
  return XbaeMatrixNumRows(r->widget());
}

int gtMatrixXm::num_columns(void)
{
  return XbaeMatrixNumColumns(r->widget());
}


int gtMatrixXm::add_columns(int position, int num_cols, short *widths, char **cols /*=NULL*/, char **labels /*=NULL*/)
{
  int retval = -1;

  if ( position >= 0 && position <= num_columns() && widths )
    {
      XbaeMatrixAddColumns(r->widget(), position, cols, labels, widths, 0, 0, 0, 0, num_cols);
      retval = 0;
    }

  return retval;
}

int gtMatrixXm::add_rows(int position, int n_rows, char **rows /*=NULL*/, char **labels /*=NULL*/)
{
  int retval = -1;

  if ( position >= 0 && position <= num_rows() )
    {
      XbaeMatrixAddRows(r->widget(), position, rows, 0, 0, n_rows);
      retval = 0;
    }

  return retval;
}

int gtMatrixXm::delete_columns(int position, int n_columns)
{
  int retval = -1;

  if ( position >= 0 && (position + n_columns) <= num_columns() )
    {
      XbaeMatrixDeleteColumns(r->widget(), position, n_columns);
      retval = 0;
    }

  return retval;
}

int gtMatrixXm::delete_rows(int position, int n_rows)
{
  int retval = -1;

  if ( position >= 0 && (position + n_rows) <= num_rows() )
    {
      XbaeMatrixDeleteRows(r->widget(), position, n_rows);
      retval = 0;
    }

  return retval;
}


//
// Selection
//

int gtMatrixXm::select_all(void)
{
  XbaeMatrixSelectAll(r->widget());
  return 0;
}

int gtMatrixXm::deselect_all(void)
{
  XbaeMatrixDeselectAll(r->widget());
  return 0;
}

int gtMatrixXm::select_cell(int row, int column)
{
  XbaeMatrixSelectCell(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::deselect_cell(int row, int column)
{
  XbaeMatrixDeselectCell(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::select_column(int column)
{
  XbaeMatrixSelectColumn(r->widget(), column);
  return 0;
}

int gtMatrixXm::deselect_column(int column)
{
  XbaeMatrixDeselectColumn(r->widget(), column);
  return 0;
}

int gtMatrixXm::select_row(int row)
{
  XbaeMatrixSelectRow(r->widget(), row);
  return 0;
}

int gtMatrixXm::deselect_row(int row)
{
  XbaeMatrixDeselectRow(r->widget(), row);
  return 0;
}

bool gtMatrixXm::is_cell_selected(int row, int column)
{
  return XbaeMatrixIsCellSelected(r->widget(), row, column);
}

bool gtMatrixXm::is_column_selected(int column)
{
  return XbaeMatrixIsColumnSelected(r->widget(), column);
}

bool gtMatrixXm::is_row_selected(int row)
{
  return XbaeMatrixIsRowSelected(r->widget(), row);
}


int gtMatrixXm::first_selected_cell(int &row, int &column)
{
  int retval = -1;

  XbaeMatrixFirstSelectedCell(r->widget(), &row, &column);

  if ( row != -1 )
    retval = 0;

  return retval;
}

int gtMatrixXm::first_selected_column(void)
{
  return XbaeMatrixFirstSelectedColumn(r->widget());
}

int gtMatrixXm::first_selected_row(void)
{
  return XbaeMatrixFirstSelectedRow(r->widget());
}

int gtMatrixXm::get_num_selected(void)
{
  return XbaeMatrixGetNumSelected(r->widget());
}


//
// Display
//

int gtMatrixXm::disable_refresh(void)
{
  XbaeMatrixDisableRedisplay(r->widget());
  return 0;
}

int gtMatrixXm::enable_refresh(void)
{
  XbaeMatrixEnableRedisplay(r->widget(), true);
  return 0;
}

int gtMatrixXm::refresh(void)
{
  XbaeMatrixRefresh(r->widget());
  return 0;
}

int gtMatrixXm::refresh_cell(int row, int column)
{
  XbaeMatrixRefreshCell(r->widget(), row, column);
  return 0;
}


//
// Editing
//

int gtMatrixXm::edit_cell(int row, int column)
{
  XbaeMatrixEditCell(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::cancel_edit(bool unmap)
{
  XbaeMatrixCancelEdit(r->widget(), unmap);
  return 0;
}

int gtMatrixXm::get_current_cell(int &row, int &column)
{
  XbaeMatrixGetCurrentCell(r->widget(), &row, &column);
  return 0;
}


//
// Highlighting
//

int gtMatrixXm::highlight_cell(int row, int column)
{
  XbaeMatrixHighlightCell(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::highlight_column(int column)
{
  XbaeMatrixHighlightColumn(r->widget(), column);
  return 0;
}

int gtMatrixXm::highlight_row(int row)
{
  XbaeMatrixHighlightRow(r->widget(), row);
  return 0;
}

int gtMatrixXm::unhighlight_all(void)
{
  XbaeMatrixUnhighlightAll(r->widget());
  return 0;
}

int gtMatrixXm::unhighlight_cell(int row, int column)
{
  XbaeMatrixUnhighlightCell(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::unhighlight_column(int column)
{
  XbaeMatrixUnhighlightColumn(r->widget(), column);
  return 0;
}

int gtMatrixXm::unhighlight_row(int row)
{
  XbaeMatrixUnhighlightRow(r->widget(), row);
  return 0;
}


//
// Visibility
//

bool gtMatrixXm::is_cell_visible(int row, int column)
{
  return XbaeMatrixIsCellVisible(r->widget(), row, column);
}

bool gtMatrixXm::is_column_visible(int column)
{
  return XbaeMatrixIsColumnVisible(r->widget(), column);
}

bool gtMatrixXm::is_row_visible(int row)
{
  return XbaeMatrixIsRowVisible(r->widget(), row);
}

int gtMatrixXm::num_visible_columns(void)
{
  return XbaeMatrixVisibleColumns(r->widget());
}

int gtMatrixXm::num_visible_rows(void)
{
  return XbaeMatrixVisibleRows(r->widget());
}

int gtMatrixXm::make_cell_visible(int row, int column)
{
  XbaeMatrixMakeCellVisible(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::visible_cells(int &top_row, int &bottom_row, int &left_column, int &right_column)
{
  XbaeMatrixVisibleCells(r->widget(), &top_row, &bottom_row, &left_column, &right_column);
  return 0;
}


//
// Getting / Setting cell / label contents
//

int gtMatrixXm::get_cell_type(int row, int column, cell_data_type &type)
{
  gtMatrixXm_cell_data &contents = data[row][column];
  type = contents.type;
  return 0;
}


int gtMatrixXm::get_cell_data(int row, int column, char const *&the_data)
{
  int retval = -1;

  gtMatrixXm_cell_data &contents = data[row][column];
  if ( contents.type == STRING )
    {
      the_data = contents.string;
      retval = 0;
    }
  else
    the_data = 0;

  return retval;
}

int gtMatrixXm::get_cell_data(int row, int column, gtPixmap *&the_data)
{
  int retval = -1;

  gtMatrixXm_cell_data &contents = data[row][column];
  if ( contents.type == PIXMAP )
    {
      the_data = contents.pixmap;
      retval = 0;
    }
  else
    the_data = 0;

  return retval;
}

int gtMatrixXm::get_column_label(int column, char const *&label)
{
  label = XbaeMatrixGetColumnLabel(r->widget(), column);
  return 0;
}

int gtMatrixXm::get_row_label(int row, char const *&label)
{
  label = XbaeMatrixGetRowLabel(r->widget(), row);
  return 0;
}


int gtMatrixXm::set_cell_data(int row, int column, char const *the_data)
{
  gtMatrixXm_cell_data &contents = data[row][column];
  contents.type = STRING;
  contents.string = the_data;
  return 0;
}

int gtMatrixXm::set_cell_data(int row, int column, gtPixmap *the_data)
{
  gtMatrixXm_cell_data &contents = data[row][column];
  contents.type = PIXMAP;
  contents.pixmap = (gtPixmapXm*)the_data;
  return 0;
}

int gtMatrixXm::set_column_label(int column, char const *value)
{
   XbaeMatrixSetColumnLabel(r->widget(), column, (char *)value);
   return 0;
}

int gtMatrixXm::set_row_label(int row, char const *value)
{
  XbaeMatrixSetRowLabel(r->widget(), row, (char *)value);
  return 0;
}

//
// Associating user data
//

int gtMatrixXm::set_cell_user_data(int row, int column, void *the_data)
{
  XbaeMatrixSetCellUserData(r->widget(), row, column, (XtPointer) the_data);
  return 0;
}

int gtMatrixXm::set_column_user_data(int column, void *the_data)
{
  XbaeMatrixSetColumnUserData(r->widget(), column, (XtPointer) the_data);
  return 0;
}

int gtMatrixXm::set_row_user_data(int row, void *the_data)
{
  XbaeMatrixSetRowUserData(r->widget(), row, (XtPointer) the_data);
  return 0;
}

int gtMatrixXm::get_cell_user_data(int row, int column, void *&the_data)
{
  the_data = XbaeMatrixGetCellUserData(r->widget(), row, column);
  return 0;
}

int gtMatrixXm::get_column_user_data(int column, void *&the_data)
{
  the_data = XbaeMatrixGetColumnUserData(r->widget(), column);
  return 0;
}

int gtMatrixXm::get_row_user_data(int row, void *&the_data)
{
  the_data = XbaeMatrixGetRowUserData(r->widget(), row);
  return 0;
}


//
// Colors
//

int gtMatrixXm::set_cell_color(int row, int column, char const *color)
{
}
 
int gtMatrixXm::set_column_background(int column, char const *color)
{
}

int gtMatrixXm::set_column_foreground(int column, char const *color)
{
}

int gtMatrixXm::set_row_background(int row, char const *color)
{
}

int gtMatrixXm::set_row_foreground(int row, char const *color)
{
}

void gtMatrixXm::draw_cell(gtMatrixXm *matrix, gtEventPtr, void *, gtReason)
{
  if ( matrix )
    {
      XbaeMatrixDrawCellCallbackStruct *xcbs = (XbaeMatrixDrawCellCallbackStruct *) matrix->get_xmcallback_data();
      if ( xcbs )
	{
	  matrix->draw_cell(xcbs);
	}
    }
}

int gtMatrixXm::draw_cell(XbaeMatrixDrawCellCallbackStruct *xcbs)
{
  int retval = -1;

  if ( xcbs )
    {
      gtMatrixXm_cell_data &the_data = data[xcbs->row][xcbs->column];
      switch ( the_data.type )
	{
	case STRING:
	  xcbs->type   = XbaeString;
	  xcbs->string = (char *)(the_data.string);
	  break;
	  
	case PIXMAP:
	  xcbs->type   = XbaePixmap;
	  xcbs->pixmap = the_data.pixmap->get_image();
	  xcbs->mask   = the_data.pixmap->get_mask();
	  break;
	}
      
      refresh_cell(xcbs->row, xcbs->column);
      retval = 0;
    }

  return retval;
}

void gtMatrixXm::write_cell(gtMatrixXm *matrix, gtEventPtr, void *, gtReason)
{
  if ( matrix )
    {
      XbaeMatrixWriteCellCallbackStruct *xcbs = (XbaeMatrixWriteCellCallbackStruct *) matrix->get_xmcallback_data();
      if ( xcbs )
	{
	  matrix->write_cell(xcbs);
	}
    }
}


int gtMatrixXm::write_cell(XbaeMatrixWriteCellCallbackStruct *xcbs)
{
  int retval = -1;

  if ( xcbs )
    {
      // get the current value of the cell
      gtMatrixXm_cell_data &the_data = data[xcbs->row][xcbs->column];

      switch ( xcbs->type )
	{
	case XbaeString:
	  the_data.type   = STRING;
	  the_data.string = xcbs->string;
	  retval = 0;
	  break;

	case XbaePixmap:
	  // XbaeMatrix does not support
	  // editing of pixmaps at this point.
	  break;
	}
    }

  return retval;  
}
  
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

gtMatrixXm_cell_data &gtMatrixXm_row_data::operator[](int i)
{
  if ( i < 0 ) 
    i = 0;
  
  if ( i >= num_cells )
    grow_to(i);

  return cells[i];
}

int gtMatrixXm_row_data::grow_to(int i)
{
  int retval = -1;

  if ( i >= num_cells )
    {
      int new_num_cells    = 32*((i%32)+1);  // for efficeincy, we grow 32 cells at a time
      gtMatrixXm_cell_data *new_cells = new gtMatrixXm_cell_data [ new_num_cells ];
      
      if ( new_cells )
	{
	  for ( int i = 0; i < num_cells; i++ )
	    new_cells[i] = cells[i];
	  
	  delete [] cells;
	  cells = new_cells;
	  num_cells = new_num_cells;

	  retval = 0;
	}
    }
  else
    retval = 0;
  
  return retval;
}

gtMatrixXm_row_data &gtMatrixXm_matrix_data::operator[](int i)
{
  if ( i < 0 ) 
    i = 0;

  if ( i >= num_rows )
    grow_to(i);

  return rows[i];
}

int gtMatrixXm_matrix_data::grow_to(int i)
{
  int retval = -1;

  if ( i >= num_rows )
    {
      int new_num_rows    = 32*((i%32)+1);  // for efficeincy, we grow 32 rows at a time
      gtMatrixXm_row_data *new_rows = new gtMatrixXm_row_data [ new_num_rows ];
    
      if ( new_rows )
	{
	  for ( int i = 0; i < num_rows; i++ )
	    new_rows[i] = rows[i];
	  
	  delete [] rows;
	  rows = new_rows;
	  num_rows = new_num_rows;
	  
	  retval = 0;
	}
    }
  else
    retval = 0;

  return retval;
}
