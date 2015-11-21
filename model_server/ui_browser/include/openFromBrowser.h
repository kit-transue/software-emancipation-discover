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
#ifndef _openFromBrowser_h
#define _openFromBrowser_h

// openFromBrowser.h
//------------------------------------------
// synopsis:
//------------------------------------------
// class openFromBrowser
//
// Analyze the selected objects in the browser and determine an appropriate
// manner in which to present their contents to the user.  For many, but
// not all, objects this will be a view of some sort.  Objects without a
// view class will use an X dialog window.
//------------------------------------------

#ifndef _representation_h
#include <representation.h>
#endif

#ifndef _waiter_h
#include <waiter.h>
#endif

class objArr;
class symbolArr;

class openFromBrowser : public commandContext
{
  public:
    openFromBrowser(void*, symbolArr&);
    openFromBrowser(void*, symbolArr&, repType);
    
  private:
    virtual void execute();
    
    void* parent;
    symbolArr& array;
    repType rep_type;
};	

/*
    START-LOG-------------------------------

    $Log: openFromBrowser.h  $
    Revision 1.1 1993/05/27 16:23:22EDT builder 
    made from unix file
 * Revision 1.2.1.4  1992/12/15  20:32:44  aharlap
 * changed for new xref
 *
 * Revision 1.2.1.3  1992/12/12  07:09:57  glenn
 * New view creation interface.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:08  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _openFromBrowser_h
