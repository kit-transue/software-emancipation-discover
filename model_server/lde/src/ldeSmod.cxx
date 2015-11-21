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
///////////////////////////  ldeSmod.h.C /////////////////////
//
// Smod Short Extractor
//

#include <lde_smt_text.h>
#include <ldeSmod.h>
#include <steHeader.h>
#include <ldrSmodNode.h>
#include <smt.h>
#include <ste_smod.h>

ldeSmod::ldeSmod( smtTreePtr root ) :
    smt_root(root),
    cur_region(0),
    status(LdeExtractStart)
{ }

steDisplayNodePtr ldeSmod::extract_region( smtTreePtr root ){
 Initialize(ldeSmod::extract_region);
    steDisplayNodePtr ans = NULL;
    switch (status){
           case LdeExtractStart :
	     {
                ans = db_new(steDisplayNode,(steSTR)); 
                ans->set_appTree( root );
                root->smod_vision = SmodLocal;
                steDisplayNodePtr par = db_new(steDisplayNode,(stePAR)); 
                ans->put_first( par );
                cur_region = db_new(ldrSmodNode,(root,root));
                par->put_first(cur_region);
	      }
                break;

           case LdeWait :
                status = LdeRegionStart;                
                ans = cur_region = db_new(ldrSmodNode,(root,root));
                cur_region->set_node_type( stePAR );
                break;

           case LdeRegionStart :
                status = LdeRegionAppend;
           case LdeRegionAppend :
                cur_region->end_smt(root->get_last_leaf());
                ans = cur_region;
                break;

       }       
   return ans;
}

steDisplayNodePtr ldeSmod::extract( smtTreePtr root, steNodeType type){
 Initialize(ldeSmod::extract);
    if ( root->smod_vision == SmodFull ) {
       status = LdeWait;
       return lde_extract_smt_text( root, type );
    }else if ( root->temp_vis == 0 )
       return extract_region( root );
    else
       return extract_node( root, type );
}

// Normal Visible extraction    
//
//
steDisplayNodePtr ldeSmod::extract_node( smtTreePtr root, steNodeType type ){
 Initialize(ldeSmod::extract_node);

    steDisplayNode * nl;

    nl = db_new(steDisplayNode,());      // new created ldr node
    nl->smod_vision = root->smod_vision;
       
    if( root->type == SMT_token ) {
      status = LdeWait;
      nl->set_appTree(root);
      cur_region = 0;
      return nl;
    }

    if ( status == LdeExtractStart || root->smod_vision )
       status = LdeWait;                       // Start new Region

    int itab;
    symtab my_tab = lde_smt_get_type( root, type, itab );
    nl->set_node_type(type);

    // Create subtree
    smtTree * t;
    int regular = (type == steREG || type == stePAR || type == steHEAD ||
                  type == steFOOT || (itab >= 0 && my_tab.s_plain));

    steDisplayNode * pl = 0;    // current previous node;
    for( t = root->get_first() ; t ; t = t->get_next()) {
      steNodeType ntype;
      if(regular)
        ntype = steREG;
      else if(itab >= 0 && my_tab.s_header && pl == 0)
        ntype = steHEAD;
      else if(itab >= 0 && my_tab.s_footer && t->get_next() == 0)
        ntype = steFOOT;
      else
        ntype = steSTR;

      steDisplayNode * l = extract(t, ntype);
      if ( l == 0 || l == pl )
         continue;             // visibility zero;

      if ( status == LdeRegionAppend && (pl == 0 && l == cur_region) )
         continue;

      if(pl)
        pl->put_after(l);
      else
        nl->put_first(l);

      if ( l->smod_vision && l->smod_vision != SmodRegion )
         status = LdeWait;

      pl = l;
      steDisplayNodePtr the_last = checked_cast(steDisplayNode,l->get_last());
      if ( l->smod_vision == SmodZero && the_last ){
         pl = the_last;
         l->splice();
         obj_delete(l);
      }
    }

    nl->set_appTree( root );

  return nl;
}

/*
   START-LOG-------------------------------------------

   $Log: ldeSmod.cxx  $
   Revision 1.2 1996/03/24 12:22:32EST rajan 
   isolate system dependencies - I
// Revision 1.4  1994/02/16  15:20:35  boris
// Bug track: #6248
// Move ste_finalize() before real_stmt execution
//
// Revision 1.3  1993/06/04  13:52:51  bakshi
// bracket var decl in case for c++3.0.1 port
//
// Revision 1.2  1993/04/04  23:29:06  boris
// Used ndm smtTree flag for "Reparse" category
//
// Revision 1.1  1993/03/26  04:33:35  boris
// Initial revision
//

   END-LOG---------------------------------------------
*/
