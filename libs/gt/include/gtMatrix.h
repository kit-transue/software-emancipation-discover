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

#ifndef _gtMatrix_h
#define _gtMatrix_h

// gtMatrix.h
//------------------------------------------
// synopsis:
// generic matrix manager (gtMatrix)
//
//------------------------------------------

#ifndef _general_h
#include <general.h>
#endif

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _gtManager_h
#include <gtManager.h>
#endif

class gtMatrix : public gtManager
{
public:
  enum cell_data_type { STRING, PIXMAP };

public:
  static   gtMatrix *create(gtBase *parent, const char *name);
  static   gtMatrix *create(void *parent, const char *name);
  virtual ~gtMatrix(void);
  
public:

  //
  // Online Help
  //

  virtual void help_callback(gtMatrixCB callback, void *client_data) = 0;

  //
  // Matrix size
  //

  virtual int num_rows(void) = 0;
  virtual int num_columns(void) = 0;

  //
  // Adding/Removing rows/columns
  //

  virtual int  add_columns(int position, int num_columns, short *widths, char **cols = NULL, char **labels = NULL) = 0;
  virtual int  add_rows(int position, int num_rows, char **rows = NULL, char **labels = NULL) = 0;
  virtual int  delete_columns(int position, int num_columns) = 0;
  virtual int  delete_rows(int position, int num_rows) = 0;

  //
  // Selection
  //

  virtual int  select_all(void) = 0;
  virtual int  deselect_all(void) = 0;
  virtual int  select_cell(int row, int column) = 0;
  virtual int  deselect_cell(int row, int column) = 0;
  virtual int  select_column(int column) = 0;
  virtual int  deselect_column(int column) = 0;
  virtual int  select_row(int row) = 0;
  virtual int  deselect_row(int row) = 0;

  virtual bool is_cell_selected(int row, int column) = 0;
  virtual bool is_column_selected(int column) = 0;
  virtual bool is_row_selected(int row) = 0;

  virtual int  first_selected_cell(int &row, int &column) = 0;
  virtual int  first_selected_column(void) = 0;
  virtual int  first_selected_row(void) = 0;
  virtual int  get_num_selected(void) = 0;

  //
  // Display
  //

  virtual int disable_refresh(void) = 0;
  virtual int enable_refresh(void) = 0;
  virtual int refresh(void) = 0;
  virtual int refresh_cell(int row, int column) = 0;

  //
  // Editing
  //

  virtual int edit_cell(int row, int column) = 0;
  virtual int cancel_edit(bool unmap) = 0;
  virtual int get_current_cell(int &row, int &column) = 0;

  //
  // Highlighting
  //

  virtual int highlight_cell(int row, int column) = 0;
  virtual int highlight_column(int column) = 0;
  virtual int highlight_row(int row) = 0;
  virtual int unhighlight_all(void) = 0;
  virtual int unhighlight_cell(int row, int column) = 0;
  virtual int unhighlight_column(int column) = 0;
  virtual int unhighlight_row(int row) = 0;

  //
  // Visibility
  //

  virtual bool is_cell_visible(int row, int column) = 0;
  virtual bool is_column_visible(int column) = 0;
  virtual bool is_row_visible(int row) = 0;
  virtual int  num_visible_columns(void) = 0;
  virtual int  num_visible_rows(void) = 0;
  virtual int  make_cell_visible(int row, int column) = 0;
  virtual int  visible_cells(int &top_row, int &bottom_row, int &left_column, int &right_column) = 0;

  //
  // Getting / Setting cell / label contents
  //

  virtual int get_cell_type(int row, int column, cell_data_type &type) = 0;
  
  virtual int get_cell_data(int row, int column, char const *&data) = 0;
  virtual int get_cell_data(int row, int column, gtPixmap *&data) = 0;
  virtual int get_column_label(int column, char const *&) = 0;
  virtual int get_row_label(int row, char const *&) = 0;
  
  virtual int set_cell_data(int row, int columns, char const *data) = 0;
  virtual int set_cell_data(int row, int column, gtPixmap *data) = 0;
  virtual int set_column_label(int column, char const *value) = 0;
  virtual int set_row_label(int row, char const *value) = 0;

  //
  // Associating user data
  //
  
  virtual int set_cell_user_data(int row, int column, void *data) = 0;
  virtual int set_column_user_data(int column, void *data) = 0;
  virtual int set_row_user_data(int row, void *data) = 0;
  virtual int get_cell_user_data(int row, int column, void *&data) = 0;
  virtual int get_column_user_data(int column, void *&data) = 0;
  virtual int get_row_user_data(int row, void *&data) = 0;
  
  //
  // Colors
  //
  
  virtual int set_cell_color(int row, int column, char const *color) = 0; 
  virtual int set_column_background(int column, char const *color) = 0;
  virtual int set_column_foreground(int column, char const *color) = 0;
  virtual int set_row_background(int row, char const *color) = 0;
  virtual int set_row_foreground(int row, char const *color) = 0;
 
protected:
  gtMatrix(void);

};

#endif
