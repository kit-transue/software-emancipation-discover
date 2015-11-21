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
//#include <stdio.h>
const char* customize_install_root();

#include <mpb.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include "startprocess.h"
#include <list>
#include <string>

#ifndef ISO_CPP_HEADERS
#include <iomanip.h>
#else /* ISO_CPP_HEADERS */
#include <iomanip>
#endif /* ISO_CPP_HEADERS */

#include <psetmem.h>
#include <vpopen.h>
#include <general.h>
#include <customize.h>
#include <genError.h>
#include <pdf_parser.h>

namespace std {}
using namespace std;
using namespace MBDriver;

extern "C"
{
    void pdf_load_state(struct flex_state_table *pdf_p, FILE *fn);
}
extern void pdf_set_parser(struct flex_state_table *pdf_tbl);


void pdf_table_free(struct flex_state_table *pdf_tbl)
{
#define fr(x) psetfree(pdf_tbl->x), pdf_tbl->paste(x,_len) = 0

     fr(yy_acclist);
     fr(yy_accept);
     fr(yy_ec);
     fr(yy_meta);
     fr(yy_base);
     fr(yy_def);
     fr(yy_nxt);
     fr(yy_chk);

}

static int is_initialized = 0;
void pdf_load_pdf(char *file)
{
  Initialize(pdf_load_pdf);
  FILE * pipe;

  static  struct flex_state_table pdf_table;

  char *root_dir = (char*)customize_install_root();
  
  mpb_incr();

  list<string> args;
#ifdef _WIN32
  args.push_back("dis_flex.exe");
#else
  string flex = root_dir;
  flex += "/bin/dis_flex";
  args.push_back(flex);
#endif
  string skel = "-S";
  skel += root_dir;
  skel += "/lib/flex.skel";
  args.push_back(skel);

  args.push_back("-t");
  args.push_back(file);
  string arg_string = prepare_command_arguments(args);

  mpb_incr();

  pipe = vpopen_sync(arg_string.c_str(), "r");
  
  mpb_incr();

  // Was: Assert(pipe);  Assert inappropriate 
  if (pipe) {
    if(is_initialized)
      pdf_table_free(&pdf_table);
    
    pdf_load_state(&pdf_table, pipe);
    
    mpb_incr();
    
    OSapi_fclose(pipe);

    pdf_set_parser(&pdf_table);

    is_initialized = 1;
  }
  else {
    msg("PDF parse: Failure executing flex", catastrophe_sev) << eom;
    // Indicate this failure somehow?
  }
  
  mpb_incr();

/*  pdf_prt_state(&pdf_table); */
}

/*
$Log: util.cxx  $
Revision 1.25 2002/11/27 17:33:53EST Scott Turner (sturner) 
Invoke flex with consideration for the possibility that
the installation directory contains a space in its name.
Revision 1.24 2001/11/07 14:08:40EST ktrans 
Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
 * Revision 1.2  1994/07/27  21:55:04  aharlap
 * call vpopen_sync()
 *
 * Revision 1.1  1994/05/23  20:21:24  so
 * Initial revision
 *
*/
