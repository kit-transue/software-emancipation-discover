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
// stack of names of functions, which were parsed; functions to handle them. 
//
//-----------------------------------------------------------------------------------
// synopsis : stack of names of functions which were parsed; functions to handle them.
// 
// Description.
// This stack contains list of names of functions which are in AST, after parsing.
// 
// Functions to handle by names of the functions  which where imported or added 
// during the session (functions in AST):
//
// - char* ast_get_fname (); - function, for which others exist; gets names of 
//   functions in stack, one name per call, in order they were loaded.
//
// - void ast_reset_get_fnames (); - prepares stack so that "ast_get_fname" 
//   will return the name of the first loaded function.
//
// - void ast_init_last_loaded_fnames (); - to delete names of functions of the last
//   loaded unit.
//
// - void ast_init_before_load (); - is executed before loading compile unit,
//   to allow "ast_init_loaded_fnames" to perform its function.
// 
// - int ast_fname_is_here (char*); - returns 1, if NAME already have been in stack;
//   otherwise, returns 0.
//
// - int ast_push_name (char *); - push function name into stack; returns 0 if OK.
//
// - void ast_reset_last_loaded_fnames (); - "ast_get_fname" will return the name of
//   the fist function of the last loaded compile unit.
// 
// - extern "C" void ast_push_fname (char*); - push function name into stack; same as
//   ast_push_name, but with C linkage.
//
// - extern "C" void ast_change_fname_in_names (char* oldname, char* newname); - is used
//   after changing name of the function in AST.
//
// - extern "C" void ast_delete_fname_in_names (char* name); - is used
//   after deleting of the function from AST.
//
//
//    
//-----------------------------------------------------------------------------
// Restrictions:
//
//
//----------------------------------------------------------
//

// include files 
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "ast_shared_fnames.h"

// ============== stack for function names ================//
// Stack contains names of all functions which where imported or added 
// during the session.
// 
const int max_number = 10000;
const overflow = 1;
const noErr = 0;

static int last_loaded_num = 0;
static int before_last_load_num = 0;

static int  get_num;

/*static char * names [max_number];    // Stack of the names of the functions*/
static char** names;

extern "C" int get_max_number_fnames ()
{
    return max_number;
}

void Init () {get_num = 0;}
extern "C" void ast_reset_get_fnames () {get_num = 0;}

void ast_init_before_load ()
{
	before_last_load_num = last_loaded_num;
}

int ast_fname_is_here (char* name)
{
	int i;
	for (i = 0; i < last_loaded_num; ++i)
		if (!strcmp (names [i], name)) break;
	if (i < last_loaded_num) return 1;
	else return 0;
}

int ast_push_name (char *name) 
{
	if (ast_fname_is_here (name)) ;
	else
		if (last_loaded_num >= max_number) return overflow; 
		else 	{  names[last_loaded_num++] = name; }
	return noErr;
}
void ast_reset_last_loaded_fnames ()  { get_num = before_last_load_num; }

extern "C" void ast_push_fname (char* name) { int rc = ast_push_name (name); }

extern "C" char* ast_get_fname ()
{
	if (get_num < last_loaded_num) 
		return names [get_num++];
	else return 0;
}

extern "C" void ast_remove_old_fname (char* oldname)
{
  int i;
  int j;
  if (ast_fname_is_here (oldname))
    for (i = 0; i < last_loaded_num; ++i)
      if (! strcmp (names[i], oldname))
	{
	  for (j=i; j < last_loaded_num - 1; ++j)
	    names [j] = names [j+1];
	  if (j > i) 
	    {
	      --last_loaded_num;
	      names [j] = names [j+1];
	    }
	  break;
	}
}

extern "C" void ast_change_fname_in_names (char* oldname, char* newname)
{
  int i;
  if (ast_fname_is_here (oldname))
    if (ast_fname_is_here (newname))
      ast_remove_old_fname (oldname);
    else
      for (i = 0; i < last_loaded_num; ++i)
	if (! strcmp (names[i], oldname))
	  {
	    names[i] = newname;
	    break;
	  }
}

extern "C" void ast_delete_fname_in_names (char* name)
{
	int i;
	int j;
	if (ast_fname_is_here (name))
		for (i = 0; i < last_loaded_num; ++i)
		if (! strcmp (names[i], name))
			{
			for (j = i; j < last_loaded_num - 1; ++j)
			names[j] = names [j+1];	
			break;
			last_loaded_num--;
			}
}

extern "C" void
assign_ast_shared_fnames (struct ast_shared_fnames* p)
{
  last_loaded_num = p->last_loaded_num;
  names = p->names;
}

extern "C" void
save_ast_shared_fnames (struct ast_shared_fnames* p)
{
  p->last_loaded_num = last_loaded_num;
}

extern "C" void reset_fnames ()
{
  last_loaded_num = 0;
}
/*
// $Log: ast_fnames.C  $
// Revision 1.2 2000/07/10 23:00:45EDT ktrans 
// mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.3  1993/04/05  20:19:57  aharlap
 * part of bug #3092
 *
 * Revision 1.2  1993/04/04  01:41:29  kol
 * added fnames
 *
 * Revision 1.1  1992/11/19  23:21:18  kol
 * Initial revision
 *
Revision 1.2.1.2  92/10/09  17:43:01  kol
moved &Log

//Revision 1.13  92/06/16  18:50:35  kol
//added reset_fnames (for reset_parser)
//
//Revision 1.12  92/05/11  22:19:41  kol
//
//
//Revision 1.11  92/05/11  19:28:22  kol
//fixed bug in delete_fname
//
//Revision 1.10  92/05/10  09:18:53  kol
//removede obsolete "static int num" 
//
//Revision 1.9  92/04/28  18:02:25  aharlap
//
//
//Revision 1.8  92/04/28  16:47:38  aharlap
//
//
//Revision 1.7  92/04/28  16:45:06  aharlap
//
//
//Revision 1.6  92/04/28  13:30:08  aharlap
//
//
//Revision 1.5  92/04/21  20:39:39  kol
//
//
//Revision 1.4  91/09/26  18:02:11  kol
//Fixed bug (in renaming function when there is another function with the same
//name).
//
//Revision 1.3  91/09/24  17:42:20  kol
//Fixed bug 172 (rename and overload)
//
//Revision 1.2  91/09/09  18:12:51  kol
//bug - strcmp !
//
//Revision 1.1  91/08/12  10:26:18  kol
//Initial revision
//
//----------------------------------------------------------
*/
