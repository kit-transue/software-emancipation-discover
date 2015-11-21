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
// Import / Export of c-files.
//-------------------
// synopsis : Import of "C" program file into AST / Export AST to "C" program file.
// 
//
// description:
// - ast_SPD_Prog_Import_C (char* path_name, char* filename); - import of "C" program file.
//   Calls "g++i -E pathname/filename > newname" (preprocessor only),
//   then calls "cc1plusi -traditional newname" to get "C" program file into AST.
//   
// - ast_SPD_Prog_Export_C (char* filename);  export AST to "C" program file.
//   get list of all function currently existing in AST, and for each of them
//   calls pretty-printer for function.
//
// 
//
//
//    
// Restrictions:
//
//
//----------------------------------------------------------
//

// include files 
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <machdep.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>               /* for "system" command */
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#endif /* ISO_CPP_HEADERS */

#include "astTree.h"

#ifndef _STAND_ALONE 
#include <genError.h>
#else
#define Initialize(X) 
extern "C" int access (char *, int);
#define OSapi_access access
#endif /* _STAND_ALONE */

#ifdef ibm
        extern "C" char* mktemp(char*);
#endif

#define R_OK 4
extern "C" char* ast_get_fname ();
extern "C" void ast_reset_get_fname ();

extern "C" int ast_main_top_C (int, char**, char**);
extern "C" int ast_main_top_CP (int, char**, char**);

extern "C" char * get_exec_path ();                  /* file "exec_path.c" */

void make_comp_parm (int&, void *, char*, int = 0);
void smt_clear_cpp_parm (char **, int);

#ifndef _STAND_ALONE 
extern "C" void * smt_create_argv ();
extern "C" int smt_push_arg (void*, char*);
extern "C" char ** smt_get_argv (void*);
extern "C" void smt_delete_argv (void*);
#endif /* _STAND_ALONE */

inline char* copy(const char * t, int l = 0)
{
    if(l) {
        char* w = strncpy(new char[l + 1], t, l);
        w[l] = 0;
        return w;
    }
    else return strcpy(new char[strlen(t) + 1], t);
}

/*
//-----------------------------------------------------------------------------
// $Log: astImport.cxx  $
// Revision 1.13 2000/10/25 13:34:47EDT ktrans 
// Bug track: 19862.  Remove valid_compiler_options.dat.
 * Revision 1.5  1993/06/03  16:37:02  bakshi
 * Fix fox c++3.0.1 compat.
 *
 * Revision 1.4  1993/04/05  21:45:26  aharlap
 * part of bug #3092
 *
 * Revision 1.3  1993/02/07  01:48:40  aharlap
 * deleted messages
 *
 * Revision 1.2  1992/11/19  23:18:53  kol
 * cleaning
 *
 * Revision 1.1  1992/11/13  18:45:57  kol
 * Initial revision
 *
Revision 1.2.1.4  92/10/15  15:55:34  kol
*** empty log message ***

Revision 1.2.1.2  92/10/09  17:36:00  kol
moved &Log

//Revision 1.22  92/09/24  09:11:43  kol
//removed ifdef GCC2
//
//Revision 1.21  92/07/09  16:27:54  kol
//access instead of fopen
//
//Revision 1.20  92/04/21  20:38:04  kol
//
//
//Revision 1.19  92/01/27  16:02:36  kol
//added -w option to parser
//
//Revision 1.18  92/01/08  15:50:59  kol
//
//
//Revision 1.17  91/12/24  17:33:35  kol
//removed call to g++i(at CPlus)
//
//Revision 1.16  91/12/09  18:53:49  kol
//added import for C++
//
//Revision 1.15  91/11/26  18:27:59  kol
//for IBM.
//
//Revision 1.14  91/11/08  09:42:02  kws
//Handle death of children errors from "system" call. We are now interested in
//these events.
//
//Revision 1.13  91/10/17  12:43:13  kol
//added new option "ansic" into string "cc1plusi -traditional -ansic..."
//
//Revision 1.12  91/09/26  18:00:46  kol
//Fixed bug. Import cannot be executed ("system"), when there is no enough swap space.
//Now I added printing system error message after "system" command fails.
//
//Revision 1.11  91/09/24  11:26:50  kol
//
//
//Revision 1.10  91/09/24  11:16:15  kol
//Fixed bug.
//
//Revision 1.9  91/09/24  09:41:22  kol
//Fixed bug for Import in using path name
//
//Revision 1.8  91/09/23  16:34:53  kol
//Fixed bugs:
//in Import - added path name of the executable to "g++i"; 
//put source after preprocessing into /tmp with unique name
//
//Revision 1.7  91/09/05  16:42:59  kol
//added import ast_brace1.c ... ast_brace2.c
//
//Revision 1.6  91/08/12  11:52:57  kol
//Changed names of #includes
//
//Revision 1.5  91/08/12  10:19:45  kol
//Comments. Changed names. restructured.

//----------------------------------------------------------
*/
