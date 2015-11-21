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
// File appMarker.h.C

#include <objRelation.h>
#include <appMarker.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

init_relational(appMarker, Relational);
init_relation(node_of_marker,1,0,marker_of_node,MANY,0);
init_relation(header_of_marker,1,0,marker_of_header,MANY,0);

init_relation(header_of_mark,1,NULL,mark_of_header,1,relationMode::D);    // the only mark of header

void gen_print_indent(ostream& st, int level);

void	appMarker::print(ostream& os, int level) const
  {
  static char * type_string[] = {"rem","down", "up"}; 
  gen_print_indent(os, level);
  os << "type " << type_string[type] << " offset " << offset << endl;
  }

appMarker :: appMarker (int tp, appTree * node, int offset)
  {
  type = tp;
  set(node, offset);
  }

void appMarker :: set(appTree * node, int ofs)
  {
  if(node)
    {
    put_relation(node_of_marker, this, node);
    put_relation(header_of_marker, this, node->get_header());
    }
  offset = ofs;
  }

appTree * appMarker :: get(int & ofs)
{
    Initialize(appMarker :: get);

    objTree * node = checked_cast(objTree,get_relation(node_of_marker, this));
    Relational * hdr = checked_cast(objTree,get_relation(header_of_marker, this));
    if (node && hdr) {
	if((checked_cast(appTree,node))->get_header() != hdr)
	{
//	    cout << "appMarker: node and header don't correspond each other" << endl;
	    clean();
	    node = 0;
	}
    } else if(hdr) {
	if (type == 0) {
//	    cout << "appMarker: node lost" << endl;
	    clean();
	} else {
//	    cout << "appMarker: node lost: moved to first leaf" << endl;
	    node = checked_cast(appTree,(checked_cast(app,hdr))->get_root()->get_first_leaf());
	    put_relation(node_of_marker, this, node);
	    offset = 0;
	}      
    } else if (node) {
//	cout << "appMarker: header lost" << endl;
	clean();
    }  
    ofs = offset; 
    return checked_cast(appTree,node);
}

void appMarker :: clean()
  {
  rem_relation(node_of_marker, this, 0);
  rem_relation(header_of_marker, this, 0);
  offset = 0;
  }

// report removing the node; argument:
// 	0 - remove without subtree;
//      1 - remove with subtree;
//      2 - move to another location
//	    within the same APP.
//      moving to another APP always
//      considered as remove (1).
void appMarker :: report_remove(int rt)
  {
    Initialize(appMarker :: report_remove);

  int ofs;
  objTree * napp = get(ofs);
  objTree * w;
  switch(type)
    {
    default:
    case 0:	// 0: delete marker
      clean();
      break;

    case 1:	// 1: marker for editor
      if(rt == 0 && (w = napp->get_first()))
	set(checked_cast(appTree,w), 0);
      else if(w = napp->get_next_leaf())
        set(checked_cast(appTree,w), 0);
      else
	set(checked_cast(appTree,napp->get_prev_leaf()), -1);
      break;
    }
  }

void appMarker :: report_split(int b, appTree * t1, appTree * t2)
  {
    Initialize(appMarker :: report_split);

  objTree * node = checked_cast(objTree,get_relation(node_of_marker, this));
  if(offset >= b || offset < 0)
    {
    if(t2)
      {
      if(t2 != node)
        put_relation(node_of_marker, this, t2);
      if(offset >= 0)
        offset -= b;
      }
    else
      offset = -1;
    }
  else
    {
    if(t1)
      {
      if(t1 != node)
        put_relation(node_of_marker, this, t1);
      }
    else
      offset = 0;  
    }
  }


void appMarker::get_mark (appPtr hd, appTreePtr &nd, int &off)
{
    Initialize(appMarker::get_mark);
    appMarker *mrk = (hd) ? checked_cast(appMarker,get_relation(mark_of_header,hd)) : NULL;
    if (mrk) {
	nd = mrk->get (off);
    } else {
	nd = NULL;
	off = -13;
    }
}
    
/*
   START-LOG-------------------------------------------

   $Log: appMarker.h.cxx  $
   Revision 1.5 2002/03/04 17:24:16EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.5  1993/07/21  00:25:23  boris
removed redandant message from output

Revision 1.2.1.4  1993/07/09  01:57:55  boris
Fixed bug #3894 with extra  appMarker message

Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:53:05  boris
*** empty log message ***


   END-LOG---------------------------------------------

*/
