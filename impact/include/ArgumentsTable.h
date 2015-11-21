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
#ifndef __ARGUMENTSTABLE_HEADER_FILE
#define __ARGUMENTSTABLE_HEADER_FILE

#include "Argument.h"

#include "stl.h"

class CArgumentFields {
 public:
  CArgumentFields(Widget type,Widget name) { m_type = type; m_name = name; };
  Widget m_type;
  Widget m_name;
};

typedef vector <CArgumentFields*> TWidgets;
typedef TWidgets::iterator TWidgetsIterator;

class CArgumentsTable {
 public:
  static const char* NAME_FIELD;
  static const char* TYPE_FIELD;

  CArgumentsTable(Widget parent,int width, int height);
  ~CArgumentsTable();
  void Add(CArgument* arg);
  Widget GetWidget(); 
  int GetRowsCount();
  CArgument Get(int idx);

 private:
  static void OnCellChanged(Widget textW,XtPointer client_data, XtPointer call_data);
  
  Widget m_Table;
  Widget m_Scroll;
  Widget m_Form;
  int    m_nWidth;
  TWidgets m_Arguments;
};

#endif //__ARGUMENTSTABLE_HEADER_FILE
