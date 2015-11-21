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
// This is a -*- C++ -*- header file

#ifndef _gtMatrixXm_h
#define _gtMatrixXm_h

#ifndef _Xbae_Matrix_h
#include <Xbae/Matrix.h>
#endif

#ifndef _gtMatrix_h
#include <gtMatrix.h>
#endif

#ifndef _gtCallbackMgrXm_h
#include <gtCallbackMgrXm.h>
#endif

class gtPixmapXm;

struct gtMatrixXm_cell_data
{
  gtMatrix::cell_data_type  type;
  char const               *string;
  gtPixmapXm               *pixmap;
  
  inline gtMatrixXm_cell_data(void);
};

class gtMatrixXm_row_data
{
public:
  inline gtMatrixXm_row_data(void);
  inline ~gtMatrixXm_row_data(void);
  
public:
  gtMatrixXm_cell_data &operator[](int);
  
private:
  gtMatrixXm_cell_data *cells;
  int                   num_cells;
  
private:
  int grow_to(int);
};

class gtMatrixXm_matrix_data
{
public:
  inline gtMatrixXm_matrix_data(void);
  inline ~gtMatrixXm_matrix_data(void);
  
public:
  gtMatrixXm_row_data &operator[](int);
  
private:
  gtMatrixXm_row_data *rows;
  int       num_rows;
  
private:
  int grow_to(int);
};

class gtMatrixXm : public gtMatrix, public gtCallbackMgrXm
{
public:
  ~gtMatrixXm(void);

  //
  // Online Help
  //

  void help_callback(gtMatrixCB callback, void *client_data);

  //
  // Matrix size
  //

  int num_rows(void);
  int num_columns(void);

  //
  // Adding/Removing rows/columns
  //

  int  add_columns(int position, int num_columns, short *widths, char **cols = NULL, char **labels = NULL);
  int  add_rows(int position, int num_rows, char **rows = NULL, char **labels = NULL);
  int  delete_columns(int position, int num_columns);
  int  delete_rows(int position, int num_rows);

  //
  // Selection
  //

  int  select_all(void);
  int  deselect_all(void);
  int  select_cell(int row, int column);
  int  deselect_cell(int row, int column);
  int  select_column(int column);
  int  deselect_column(int column);
  int  select_row(int row);
  int  deselect_row(int row);

  bool is_cell_selected(int row, int column);
  bool is_column_selected(int column);
  bool is_row_selected(int row);

  int  first_selected_cell(int &row, int &column);
  int  first_selected_column(void);
  int  first_selected_row(void);
  int  get_num_selected(void);

  //
  // Display
  //

  int disable_refresh(void);
  int enable_refresh(void);
  int refresh(void);
  int refresh_cell(int row, int column);

  //
  // Editing
  //

  int edit_cell(int row, int column);
  int cancel_edit(bool unmap);
  int get_current_cell(int &row, int &column);

  //
  // Highlighting
  //

  int highlight_cell(int row, int column);
  int highlight_column(int column);
  int highlight_row(int row);
  int unhighlight_all(void);
  int unhighlight_cell(int row, int column);
  int unhighlight_column(int column);
  int unhighlight_row(int row);

  //
  // Visibility
  //

  bool is_cell_visible(int row, int column);
  bool is_column_visible(int column);
  bool is_row_visible(int row);
  int  num_visible_columns(void);
  int  num_visible_rows(void);
  int  make_cell_visible(int row, int column);
  int  visible_cells(int &top_row, int &bottom_row, int &left_column, int &right_column);

  //
  // Getting / Setting cell / label contents
  //

  int get_cell_type(int row, int column, cell_data_type &type);
  
  int get_cell_data(int row, int column, char const *&the_data);
  int get_cell_data(int row, int column, gtPixmap *&the_data);
  int get_column_label(int column, char const *&label);
  int get_row_label(int row, char const *&label);
  
  int set_cell_data(int row, int column, char const *the_data);
  int set_cell_data(int row, int column, gtPixmap *the_data);
  int set_column_label(int column, char const *value);
  int set_row_label(int row, char const *value);

  //
  // Associating user data
  //
  
  int set_cell_user_data(int row, int column, void *the_data);
  int set_column_user_data(int column, void *the_data);
  int set_row_user_data(int row, void *the_data);
  int get_cell_user_data(int row, int column, void *&the_data);
  int get_column_user_data(int column, void *&the_data);
  int get_row_user_data(int row, void *&the_data);
  
  //
  // Colors
  //
  
  int set_cell_color(int row, int column, char const *color); 
  int set_column_background(int column, char const *color);
  int set_column_foreground(int column, char const *color);
  int set_row_background(int row, char const *color);
  int set_row_foreground(int row, char const *color);

protected:
  gtMatrixXm(void);

private:

  gtMatrixXm_matrix_data data;

private:

  static void draw_cell(gtMatrixXm *, gtEventPtr event, void *, gtReason);
  int         draw_cell(XbaeMatrixDrawCellCallbackStruct *);

  static void write_cell(gtMatrixXm *, gtEventPtr, void *, gtReason);
  int         write_cell(XbaeMatrixWriteCellCallbackStruct *);

};

gtMatrixXm_cell_data::gtMatrixXm_cell_data(void)
{
  type   = gtMatrixXm::STRING;
  string = 0;
  pixmap = 0;
}

gtMatrixXm_row_data::gtMatrixXm_row_data(void)
{
  cells = 0;
  num_cells = 0;
}

gtMatrixXm_row_data::~gtMatrixXm_row_data(void)
{
  delete [] cells;
}

gtMatrixXm_matrix_data::gtMatrixXm_matrix_data(void)
{
  rows = 0;
  num_rows = 0;
}

gtMatrixXm_matrix_data::~gtMatrixXm_matrix_data(void)
{
  delete [] rows;
}

#endif

