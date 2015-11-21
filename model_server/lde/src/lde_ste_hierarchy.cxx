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
////////////////////////   FILE lde_ste_hierarchy.C  ///////////////////////////

#include <objOper.h>
#include <representation.h>

#include <steDisplayHeader.h>
#include <steRefNode.h>
#include <steHeader.h>
#include <ldrSmtSte.h>
#include <objArr.h>
#include <ste_fix.h>

#include "reference.h"
#include "lde_reference.h"

static steDisplayNodePtr extract (steTextNodePtr node);
static int ste_is_wrong_branch( steTextNodePtr rt );

//************************
// lde_extract_ste_text ()
//************************
steDisplayNodePtr lde_extract_ste_text(steTextNodePtr text_root)
{
    Initialize(lde_extract_ste_text);

    if(! text_root)
	return(NULL);

    steDisplayNodePtr  disp_root;

    if ( ste_is_wrong_branch(text_root) )
	disp_root = NULL;
    else
	disp_root = extract (text_root);

    return(disp_root);
}

//**********************
// ste_is_wrong_branch():    answer  1 - yes, 0 - no.  
//**********************
static int ste_is_wrong_branch( steTextNodePtr rt )
{
    Initialize(ste_is_wrong_branch);
    if ( !is_steTextNode(rt) )
	return(1);  

    if ( !rt->get_parent() )
	return(0);  

    steTextNodePtr frst = checked_cast(steTextNode,rt->get_first());
    Obj *str1 = get_relation(text_node_ascii,rt);   
    if ( !frst )
	if (( str1 == NULL ) && (!is_steRefNode(rt)))
	    return(1);  
	else
	    return(0);  
   
    steTextNodePtr cur;
    int correct_flag = 0;
    for ( cur = frst ; cur ; cur = checked_cast(steTextNode,cur->get_next() ))
    {
	int loc_flag = ste_is_wrong_branch( cur );
	if ( loc_flag == 0 )
	{
	    correct_flag = 1;
	    break;
	}
    }

    if ( correct_flag == 0 )
	return(1);
    else
	return(0);
}

//********************
// extract ()
//********************
static steDisplayNodePtr extract (steTextNodePtr text_root)
{
    Initialize (extract);

    steDisplayNodePtr  disp_root;

    disp_root = db_new(steDisplayNode,());
    disp_root->set_node_type((steNodeType)text_root->get_node_type());

    apptree_put_ldrtree(text_root, disp_root);

    if (is_steRefNode(text_root))
    {
	steDisplayNodePtr  ref_tree;
	ref_tree = extract_reference_ldr_with_header( text_root );
	disp_root->put_first( ref_tree );
    }
    else
    {
	steDisplayNodePtr cur_disp, prev_disp = 0;
	steTextNodePtr cur_text;

	for(cur_text = checked_cast(steTextNode,text_root->get_first());
	    cur_text;
	    cur_text = checked_cast(steTextNode,cur_text->get_next()  ))
	{
	    cur_disp = lde_extract_ste_text(cur_text);
	    if ( cur_disp == NULL )
	    {
		if ( ste_fix_is_enabled() )
		    ste_fix_add( cur_text );
		else
		{
		    steHeader(Message)
			<< "extract - wrong document tree structure."
			<< steEnd;
		}
		continue;
	    }
	    if(prev_disp)
		prev_disp->put_after(cur_disp);
	    else
		disp_root->put_first( cur_disp );
	    
	    prev_disp = cur_disp;
	}
    }
    return (disp_root);
}  

/*
   START-LOG-------------------------------------------

   $Log: lde_ste_hierarchy.cxx  $
   Revision 1.1 1994/03/16 11:04:10EST builder 
   made from unix file
Revision 1.2.1.7  1993/01/07  01:36:16  jon
Removed extract_reference and other useless code

Revision 1.2.1.6  1993/01/06  20:12:02  jon
Changed structure of ldr for references

Revision 1.2.1.5  1992/12/17  14:48:58  jon
Moved some reference code into new file lde_reference.
Updated other reference code, made some other routines understand
references.

Revision 1.2.1.4  1992/11/21  22:16:34  builder
typesafe casts.

Revision 1.2.1.3  1992/10/29  20:26:49  boris
Added ste_fix facilities

Revision 1.2.1.2  92/10/09  19:19:13  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
