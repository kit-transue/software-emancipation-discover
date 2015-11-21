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
// File appMarker.h
#ifndef appMarker__h
#define appMarker__h

#include "objOper.h"

class appMarker : public Relational
{
  public:
    unsigned char type;		// Type of marker. Defines bihevior when 
                                // the node is getting temoved:
                                // 0 - delete (marker deleted)
                                // 1 - down (marker moved down if ?>??????
                                // first child retained; otherwise
                                // moved up)
                                // 2 - up.

    char res1;			// Reserved for future use

    short offset;		// Offset 

    appMarker(int, appTree * = 0, int = 0);

    appTree * get(int &);
    void set(appTree *, int);
    void clean();		// Clean marker
    void report_split(int, appTree *, appTree *);// Report "split" 
    virtual void report_remove(int);	// report removing the node; argument:
				// 	0 - remove without subtree;
				//      1 - remove with subtree;
			        //      2 - move to another location
				//	    within the same APP.
				//      moving to another APP always
				//      considered as remove (1).

    virtual void	print(ostream& = cout, int level = 0) const;

    define_relational(appMarker, Relational);

    static void set_mark (appTreePtr nd, int off, int tp = 0);
    static void get_mark (appPtr hd, appTreePtr &nd, int &off);
    
//  RelationalPtr rel_copy() const {return 0;};
}; 

define_relation(node_of_marker, marker_of_node);
define_relation(header_of_marker, marker_of_header);

define_relation(header_of_mark, mark_of_header);    // the only mark of header

#endif
/*
   START-LOG-------------------------------------------

   $Log: appMarker.h  $
   Revision 1.2 1997/03/20 17:18:23EST azaparov 
   
 * Revision 1.2.1.3  1993/02/11  18:23:05  mg
 * conditional inclusion
 *
 * Revision 1.2.1.2  1992/10/09  18:58:09  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
