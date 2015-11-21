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
#include "ast_report.h"
#include "cLibraryFunctions.h"
#include <msg.h>
#include "genError.h"
#if defined(GCC2) || defined(_WIN32)
#include "ast_hook.h"
#endif


// Reports addition of new function decl in the system.
void ast_report_add_fndecl (astFunction_decl * fndecl)
{

   Initialize (ast_report_add_fndecl);

   if (fndecl) {

   }

   Return;

}
#ifdef XXX_ast_get_file_globals  // dormant
#if !defined(GCC2) && !defined(_WIN32)
extern objArr * ast_get_file_globals();
#else
extern treeArr * ast_get_file_globals();
#endif
#endif

// Reports deletion of function decl
void ast_report_delete_fndecl (astFunction_decl * fndecl)
{
   Initialize (ast_report_delete_fndecl);

   if (fndecl) {

   }

   Return;
}

// Reports renaming of a function decl.
void ast_report_rename_fndecl (astFunction_decl * fndecl)
{
   Initialize (ast_report_rename_fndecl);

   if (fndecl) {

   }

   Return;
}

void ast_report_add_global_variable (astTree *g_var)
{
   Initialize (ast_report_add_global_variable);

   Return;
}

void ast_report_delete_global_variable (astTree *g_var)
{
   Initialize (ast_report_delete_global_variable);

   Return;
}

void ast_report_rename_global_variable (astTree *old_var, astTree *new_var)
{
   Initialize (ast_report_rename_global_variable);

   Return;
}

/*
   START-LOG-------------------------------------------

   $Log: ast_report.cxx  $
   Revision 1.6 2000/11/19 16:49:33EST ktrans 
   Remove subsystems
Revision 1.2.1.3  1992/11/21  23:33:28  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:20:52  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
