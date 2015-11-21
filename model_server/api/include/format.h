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
#ifndef	__format_h
#define __format_h

#include <objArr.h>
#include <attribute.h>
#include <genString.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

class Interpreter;
typedef int (*setTraverseFunction) (symbolArr&arr,int arr_index, int group_index,int group_size,char*,Interpreter*inter,void*data);
int api_sort_apply(symbolArr& arr, const char*exp, bool once, setTraverseFunction f, Interpreter*inter, void* data);

class symbolFormat : public objDictionary {
public:
  symbolFormat(const char* name);
  symbolFormat(const char* name, objArr& attributes);
  symbolFormat(const char* name, objArr& attributes, char* form);
  symbolFormat(const char* name, objArr& attributes, char* form, char* head);

  void setAttributes(objArr& attributes);
  
  void setSortOrder(char* order);
  // The sort order should be a string of 'a's and 'd's specifying if the sort on the 
  //    nth attribute should be acsending or decsending.
  //    Ascending is the default.

  void setHeader(char* head);
  void setFormat(const char* form);
  

  void sort(symbolArr& arr, symbolArr& result);
  void printOut(ostream& outstr, symbolArr& arr, int print_indexes = 0, const char* yes = NULL, const char* no = NULL);
  void printOut(ostream& outstr, const symbolPtr&, const char* yes = NULL, const char* no = NULL);

  virtual void	print(ostream& = cout, int level = 0) const;

  objArr    format_array;
  genString sort_order;
  int       format_size;
  genString header;
  genString format;
  int       do_padding;

  static objArr all_formats;
  static symbolFormat* get_format_by_name(char* name);
};


#endif // __format_h
