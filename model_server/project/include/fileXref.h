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
// fileXref
//------------------------------------------
// synopsis:
// Class declarations for creating and mapping filename aliases used by
// rename
//
// description:
//------------------------------------------
/*
    START-LOG-------------------------------

    $Log: fileXref.h  $
    Revision 1.1 1993/05/06 13:35:55EDT builder 
    made from unix file
 * Revision 1.2.1.2  1992/10/09  18:22:53  builder
 * *** empty log message ***
 *

    END-LOG---------------------------------
*/

#ifndef __fileXref
#define __fileXref

// include files

#include "avl_tree.h"

// forward class declarations

class fileXref_ref;
class fileXref_entry;

//------------------------------------------
// Class fileXref extracts the text of the specified steView, parses it
// into fileXrefs, and uses the supplied filter function to determine whether
// each fileXref should be entered into the list.
//------------------------------------------

class fileXref: public avl_tree {
   friend class fileXref_iter;   // to allow conversion of fileXref* 
				  // to avl_tree*
public:
   fileXref();
   ~fileXref();
   void save_xref(const char*);
   void restore_xref(const char*);

   fileXref_ref* find(const fileXref_entry&) const;
   static const char* map_file(const char*);
};                                          

//------------------------------------------
// Class fileXref_entry represents the entry of a given fileXref and 
// functions as a search key for the balanced tree.
//------------------------------------------

class fileXref_entry: public avl_key {
public:
   fileXref_entry(const char* tag, const char* fname);
   ~fileXref_entry();
   fileXref_entry(const fileXref_entry&);

   const char* str() const { return tag; }
   const char* alias() const { return fname; }
   size_t len() const { return tag_len; }

private:
   char* tag;
   int tag_len;

   char* fname;
   int fname_len;
};                                          

//------------------------------------------
// Class fileXref_ref is a reference to a fileXref in a document.  Only the first
// reference to that fileXref will appear in the tree; other references to the
// same fileXref are attached to the first one via a singly-linked list.  The
// compare() function does a caseless comparison.
//------------------------------------------

class fileXref_ref: public avl_node {
public:
   fileXref_ref(fileXref* t, const fileXref_entry& spl);
   ~fileXref_ref();
   int compare(const avl_key&) const;
   fileXref_ref* dup() const { return (fileXref_ref*) avl_node::dup(); }
   fileXref_ref* next() const { return fwd_link; }
   void link_to(fileXref_ref* wd) { fwd_link = wd; }
   const char* str() const { return spelling.str(); }
   const char* alias() const { return spelling.alias(); }
   size_t len() const { return spelling.len(); }
   unsigned int get_flag() { return flag; }
   void set_flag(unsigned int f) { flag = f; }
private:
   fileXref_ref* fwd_link;
   fileXref_entry spelling;
   unsigned int flag;
};
                                            
//------------------------------------------
// Class fileXref_iter provides a conveniently-typed direct interface to the
// balanced-tree iterator, allowing the fileXrefs in the list to be accessed in
// sorted order.
//------------------------------------------

class fileXref_iter: public avl_iterator {
public:
   fileXref_iter(const fileXref* wlp): avl_iterator(wlp) { }
   fileXref_ref* next() { return (fileXref_ref*) avl_iterator::next(); }
};

#endif
