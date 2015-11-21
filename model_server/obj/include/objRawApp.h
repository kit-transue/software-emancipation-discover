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
#ifndef _objRawApp_h
#define _objRawApp_h

//------------------------------------------
// objRawApp.h
//------------------------------------------
//
// class objRawApp
//
// Description: app-header for arbitrary raw UNIX ASCII files
//------------------------------------------

#ifndef _objOper_h
#include <objOper.h>
#endif

#define BIT_PLACE(name,var)\
unsigned int name() const { return var; }\
unsigned int name(unsigned int x) { return var = (x != 0); }

class objRawApp : public app
{
  public:
    objRawApp(char* full_name, unsigned int write_flag, 
	      unsigned int rel_flag, unsigned int new_flag = 0);
    ~objRawApp();

    define_relational(objRawApp,app);
    copy_member(objRawApp);

    virtual void insert_obj(objInserter*, objInserter*);
    virtual void remove_obj(objRemover*, objRemover*);

    virtual void print(ostream&, int) const;

    BIT_PLACE(is_writable,	_is_writable);
    BIT_PLACE(has_rel,		_has_rel);
    BIT_PLACE(is_new,		_is_new);

    int  view_kind ();
    void view_kind (int);

  private:
    unsigned int _is_writable   : 1;
    unsigned int _has_rel       : 1;
    unsigned int _is_new        : 1;

    int v_kind;
};

generate_descriptor(objRawApp,app);


/*
   START-LOG-------------------------------------------

   $Log: objRawApp.h  $
   Revision 1.1 1993/11/06 10:52:11EST builder 
   made from unix file
// Revision 1.4  1993/11/06  00:29:35  boris
// Bug track: Debugger views
// Fixed Debugger viewes performance
//
// Revision 1.3  1993/04/16  17:46:35  glenn
// Remove full_name.
//
// Revision 1.2  1993/04/16  17:05:35  glenn
// Remove redundant _name member.
// Redefine full_name in terms of app::get_filename.
// Remove bad copy ctor.  Add dtor.
//
// Revision 1.1  1992/12/11  22:10:17  boris
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif 
