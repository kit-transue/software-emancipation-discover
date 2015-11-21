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
#ifndef _ddHeader_h_
#define _ddHeader_h_

#ifndef _representation_h
#include <representation.h>
#endif
#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef _objOper_h
#include <objOper.h>
#endif
#ifndef _genError_h
#include <genError.h>
#endif


define_relation(xref_header_of_dd_header, dd_header_of_xref_header);

class ddHeader: public app {
  char name[256];
  public:

  ddHeader(){set_type(App_DD);}

  ddHeader(char *pname){set_name(pname);set_type(App_DD);}
  define_relational(ddHeader,app);
  virtual void insert_obj(objInserter *oi, objInserter *ni);
  
  virtual void  remove_obj(objRemover *, objRemover * nr);
  

  char const *get_name() const{return name;}
  void set_name( char *p);
  virtual void assign_obj(objAssigner *, objAssigner *);
};

generate_descriptor(ddHeader,app);

extern ddHeader* get_dd_header();

#endif

/*
   START-LOG-------------------------------------------

   $Log: ddHeader.h  $
   Revision 1.5 2000/07/07 08:07:40EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.4  1993/07/13  07:00:48  builder
 * *** empty log message ***
 *
 * Revision 1.2.1.3  1993/06/25  13:51:42  bakshi
 * typecast for c++3.0.1 port
 *
 * Revision 1.2.1.2  1992/10/09  20:06:47  builder
 * FIx comments
 *

   END-LOG---------------------------------------------
*/

