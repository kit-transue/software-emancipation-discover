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
// Implementation of ldrClipboard class
//------------------------------------------------
//
//
//
//
//------------------------------------------------

#include "genError.h"
#include "ldrClipboard.h"
#include "smt.h"
#include "steTextNode.h"
#include "ste_commonTree_functions.h"
#include "ldrTypes.h"
#include "RTL.h"

init_relational (ldrClipboard, ldrSymbolNode);

ldrClipboard::ldrClipboard (appTreePtr node)
{
    Initialize(ldrClipboard::ldrClipboard);

    set_appTree (node);

    // get the symbol type
    appTreePtr first_node;

    if (is_clipboard(node))
	first_node = checked_cast(appTree, node->get_first());
    else
	first_node = node;

    if (first_node) {

	if (is_steTextNode(first_node)) {

	    switch (checked_cast(steTextNode,first_node)->get_node_type()) {

	      case (stePAR) :
		set_symbol_type (ldrTEXT);
		break;

	      case (steSTR):
		if (ste_get_title(first_node)) {
		    // symbol is section
		    set_symbol_type (ldrTEXT);
		} else {
		    // symbol is paragraph
		    set_symbol_type (ldrTEXT);
		}
		break;

	      case (steHEAD):
		// symbol is title
		set_symbol_type (ldrTEXT);
		break;

	      default:
	      case (steREG):
		// symbol is normal text
		set_symbol_type (ldrTEXT);
		break;

	    }
	} else if (is_smtTree (first_node)) {
	    switch (checked_cast(smtTree,first_node)->get_node_type()) {
	      case SMT_for:
	      case SMT_while:
	      case SMT_do:
		set_symbol_type (ldrLOOP);
		break;

	      case SMT_block:
		set_symbol_type (ldrBLOCK);
		break;

	      case SMT_if:
	      case SMT_ifelse:
	      case SMT_else:
		set_symbol_type (ldrIF);
		break;

	      default:
		set_symbol_type (ldrSOMETHING);
		break;
	    }
	} else if (is_RTLNode(first_node)) {
	    set_symbol_type (ldrLIST);
	}
    }
}

ldrClipboard::ldrClipboard()
{
}

ldrClipboard::ldrClipboard(const ldrClipboard &oo)
{
}

ldrClipboard::~ldrClipboard()
{
}

/*
   START-LOG-------------------------------------------

   $Log: ldrClipboard.cxx  $
   Revision 1.1 1993/01/14 11:21:36EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:43:48  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
