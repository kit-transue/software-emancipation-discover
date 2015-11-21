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
//////////////  ddArgInserter.h

//
// description: Class to support propagations of adding an argument to a
// function
//

// History:     10/28/91   S.Spivakovksy   Initial coding.

#ifndef _ddArgInserter_h
#define _ddArgInserter_h

#include "genString.h"
#include "objRelation.h"
#include "objOper.h"
#include "ddict.h"
#include "smt.h"
#ifndef GCC2
#include "astTree.h"
#else
class astDecl;
#endif
#include <objOperate.h>
class ddArgInserter : public objModifier {
public:
  char *new_param;
  char *new_arg;
  astDecl *param_image;
  int param_len;
  int arg_len;
  int index;
  char *pn;
  char *kr_par;
  char *default_val;
  ddElement* dd;
  static int save_val_then_finish_prop;
  static genString saved_val;

  ddArgInserter( int , int , astDecl* , int , char* ,
		int , char* ) {}
  ddArgInserter( int idx,
		int is_last,
		astDecl *pa,
		int lpar,
		char *ppar,
		int larg,
		char *parg, char *pname, ddElement* de);
      
  ~ddArgInserter();
  virtual int apply(RelationalPtr );
  virtual objPropagator * accept(RelationalPtr curr_obj, int);
  static void reset_save_flag();
};

#endif






















