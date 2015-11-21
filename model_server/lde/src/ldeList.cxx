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
// synopsis : lde for RTL
//
//----------------------------------------------------------
 
#include <ldrList.h>
#include <ldeList.h>
#include <proj.h>

ldrListNodePtr lde_list_extract (RTLNodePtr rtl)
{
    Initialize (lde_build_list_tree);
 
 
    ldrListNodePtr root = db_new (ldrListNode,());
    
    lde_list_extract (root, rtl);

    return root;
}

void lde_list_extract (ldrListNodePtr root, RTLNodePtr rtl)
{
   
    symbolArr& rtl_contents = rtl->rtl_contents();
 
    root->list.reset();
    root->text.reset();

    for(int i = 0; i < rtl_contents.size(); ++i)
    {
        symbolPtr& obj = rtl_contents[i];
        if(obj.is_ast() || obj.is_dataCell() || obj.is_instance() || obj.get_name())
        {
            root->insert_symbol (obj, i);
        }
    }
}


/*
   START-LOG-------------------------------------------

   $Log: ldeList.cxx  $
   Revision 1.2 1997/10/21 11:28:43EDT mg 
   
 * Revision 1.1  1994/01/21  00:27:58  kws
 * Initial revision
 *

   END-LOG---------------------------------------------

*/
