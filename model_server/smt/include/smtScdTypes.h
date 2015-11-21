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
/* Types od SMT statements: */

#ifndef _prototypes_h
#include "prototypes.h"
#endif

extern char * smt_token_name PROTO((int));

#ifndef deftok
#define deftok(x,n)  x=n
#define tok_enum enum
#endif

tok_enum {

/* preprocessor */
deftok(SMT_comment,1),
deftok(SMT_m_include,2),
deftok(SMT_m_define,3),
deftok(SMT_m_if,4),
deftok(SMT_m_else,5),
deftok(SMT_m_endif,6),
deftok(SMT_m_undef,7),
deftok(SMT_m_pragma,8),
deftok(SMT_m_gen,9),

deftok(SMT_file,10),
deftok(SMT_decl,11),  /* declaration */
deftok(SMT_fdecl,13), /* function decl */
deftok(SMT_pdecl,14), /* parameter decl */
deftok(SMT_cdecl,15), /* class decl */
deftok(SMT_edecl,16), /* enumeration decl */
deftok(SMT_cbody,17), /* class body */
deftok(SMT_ebody,18), /* enumeration body */
deftok(SMT_sdecl,19), /* decl with structure */
                      /* in line for type */

deftok(SMT_gen,21),
deftok(SMT_if,22),
deftok(SMT_ifelse,23),
deftok(SMT_else,20),
deftok(SMT_for,24),
deftok(SMT_while,25),
deftok(SMT_do,26),
deftok(SMT_block,27),
deftok(SMT_switch,28),
deftok(SMT_case,29),
deftok(SMT_default,30),
deftok(SMT_break,31),
deftok(SMT_continue,32),
deftok(SMT_return,33),
deftok(SMT_goto,34),
deftok(SMT_label,35),
deftok(SMT_empty,36),
deftok(SMT_expr,37),
deftok(SMT_dowhile,39),
deftok(SMT_stmt,40),
deftok(SMT_nstdloop,41),
deftok(SMT_clause,42),
deftok(SMT_class,43),
deftok(SMT_else_clause,44),
deftok(SMT_else_if_clause,45),
deftok(SMT_then_clause,46),
deftok(SMT_case_clause,47),
deftok(SMT_superclass,48),
deftok(SMT_macrocall,49),
deftok(SMT_macroarg,61),
deftok(SMT_macrostmt,38),
deftok(SMT_title,50),

deftok(SMT_typedef,51),

deftok(SMT_gdecl, 52),
deftok(SMT_ldecl, 53),  /* global decl, and local decl for var */
deftok(SMT_mem_decl, 54),
deftok(SMT_enum_field, 55),
deftok(SMT_list_decl, 56),
deftok(SMT_declspec, 57),
deftok(SMT_list_pdecl, 58),
deftok(SMT_subtitle,59),
deftok(SMT_fdef,60),
/* 61 macroarg */
deftok(SMT_try_catch,62),
deftok(SMT_try_clause,63),
deftok(SMT_catch_clause,64),

/* Special types	*/
deftok(SMT_null,-1), 	/* undefined node */
deftok(SMT_token,101), 	/* single token */
deftok(SMT_group,102), 	/* group of tokens */
deftok(SMT_temp,103),	/* temporary structured node which must be
			   destroyed on any parsing			*/

/* Types of tokens      */
#define SMTT_FIRST_VALUE SMTT_ref

deftok(SMTT_ref,110), /* Reference - special kind of token
					   without text value   */
deftok(SMTT_kwd,111),
deftok(SMTT_ident,112),
deftok(SMTT_op,113),
deftok(SMTT_macro,115),
deftok(SMTT_const,116),
deftok(SMTT_string,117),
deftok(SMTT_begin,118),
deftok(SMTT_end,119),

deftok(SMTT_untok, 122),/* Untokenized text			*/
deftok(SMTT_lb, 123),	/* Line break				*/
deftok(SMTT_el, 124),  	/* Empty line- special kind of comment 	*/
deftok(SMTT_commentl,125),	/* Comment as separate line	*/
deftok(SMTT_comment,126),	/* Comment on the same line	*/
deftok(SMTT_grouping,127),	/* '('   ','   ')' 		*/
deftok(SMT_begcomm,130),        
deftok(SMT_endcomm,131),
deftok(SMTT_ppp,132)
   };

#define SMT_STAT SMT_block, SMT_switch, SMT_if, SMT_stmt, SMT_nstdloop,\
        SMT_goto, SMT_for, SMT_while, SMT_do,\
        SMT_break,SMT_continue, SMT_return, SMT_label, SMT_empty,\
        SMT_dowhile, SMT_comment, SMT_m_include
#define SMT_DECL SMT_list_decl, SMT_cdecl, SMT_edecl
#define SMT_DEFI SMT_typedef, SMT_fdef, SMT_m_define

/*
   START-LOG-------------------------------------------

   $Log: smtScdTypes.h  $
   Revision 1.4 1995/07/27 20:27:24EDT rajan 
   Port
 * Revision 1.2.1.8  1993/06/17  20:24:33  mg
 * deftok(SMT_macroarg,61)
 *
 * Revision 1.2.1.7  1993/06/14  23:45:15  sergey
 * Added some of SMT_micro types to the list of statement types (SMT_STAT). Fixed bug #3699.
 *
 * Revision 1.2.1.6  1993/01/05  22:00:06  mg
 * new db
 *
 * Revision 1.2.1.5  1992/12/13  21:58:10  jon
 * Added define SMT_DEFI
 *
 * Revision 1.2.1.4  1992/11/20  00:19:49  aharlap
 * added SMTT_ppp
 *
 * Revision 1.2.1.3  1992/11/11  00:14:23  aharlap
 * added SMT_begcomm and SMT_endcomm
 *
 * Revision 1.2.1.2  1992/10/09  19:25:32  boris
 * Fix comments
 *


   END-LOG---------------------------------------------

*/
