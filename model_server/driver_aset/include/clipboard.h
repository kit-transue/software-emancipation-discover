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
#ifndef _clipboard_h_
#define _clipboard_h_

#include "objOper.h"

RelClass(clipboard);

class objArr;
class objArr_Int;

class clipboard: public appTree {

  protected:

    commonTreePtr shallow_copy (commonTreePtr);
    commonTreePtr deep_copy (commonTreePtr);
    commonTreePtr deep_copy (Obj *);

    void       add_object (commonTreePtr);
    void       remove_object (commonTreePtr);

  public:

    define_relational (clipboard, appTree);

    clipboard();
    clipboard(const clipboard &);
    ~clipboard();

    void          copy (commonTreePtr obj, bool shallow);
    void          copy (Obj *, bool shallow);
    void          reference (commonTreePtr obj);
    void          reference_graphics (objArr&);
    void          fill_objects (appPtr header, objArr &);

    virtual void send_string(ostream& stream) const;

    copy_member (clipboard);

    // creates smtHeader over collection, if do_regions is 1
    // creates SMT_temp over each region that is more than one node
    static appPtr create_deep_copy (objArr&, objArr_Int&, int do_regions = 0);
};

generate_descriptor (clipboard, appTree);

/*
   START-LOG-------------------------------------------

   $Log: clipboard.h  $
   Revision 1.1 1994/05/09 14:01:48EDT builder 
   made from unix file
 * Revision 1.2.1.5  1994/05/05  20:04:18  boris
 * Bug track: 6794
 * Fixed "slow merge" problem, and #6977 asyncronous save
 *
 * Revision 1.2.1.4  1992/12/02  23:04:06  smit
 * Fix cut/paste problem.
 *
 * Revision 1.2.1.3  1992/12/01  19:42:04  boris
 * Changed copy first argument from Relational& to Obj& to fix CUT.
 *
 * Revision 1.2.1.2  1992/10/09  18:52:50  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif
