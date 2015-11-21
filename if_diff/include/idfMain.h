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
#ifndef _idfMain_h
#define _idfMain_h

#include <idf.h>
#include <locationStruct.h>
#include <locationList.h>
#include <general.h>
#include "idfDict.h"
class genString;
class idfAstDictionary;

extern idfModel *idf_current_model;
extern idfSymbolDictionary *idf_current_symbols; // 1 dictionary per compared IF file
extern const char *idf_current_filename;
extern const char *idf_interesting_filename;
extern idfSmt *idf_prev_smt; // most recent smt constructed
extern idfAstDictionary *idf_current_ast_roots;
extern int idf_current_ast_root_count;
extern idfStringSetDictionary idf_configuration;
extern idfStringSet idf_no_values;

extern boolean idf_strmatch(const char *, const char *);
extern char *idf_copy_string(const char *);

//============================================================================
// idfAstDictionary
//----------------------------------------------------------------------------
// dictionary class, indexed by int and yielding an AstSiblings structure
//============================================================================
class idfAstDictionary : private idfIntDictionary {
public:
  boolean         add(int idx, idfAstSiblings *value)
   { return idfIntDictionary::add(idx, (void *)value); }
  boolean         reassign(int idx, idfAstSiblings *value)
   { return idfIntDictionary::reassign(idx, (void *)value); }
  idfAstSiblings *lookup(int idx) const
   { return (idfAstSiblings *)idfIntDictionary::lookup(idx); }
  idfAstSiblings *lookup(const idfIterator &iter) const
   { return (idfAstSiblings *)idfIntDictionary::lookup(iter); }
                  idfIntDictionary::index;
                  idfIntDictionary::first;
                  idfIntDictionary::next;
                  idfIntDictionary::done;
                  idfIntDictionary::contains;
  int		  count() {
		      int n = 0;
		      for (idfIterator iter = first(); !done(iter); next(iter)) {
			n += 1;
		      }
		      return n;
		  }
};

#endif
