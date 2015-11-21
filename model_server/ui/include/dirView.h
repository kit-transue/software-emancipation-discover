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
#ifndef _dirView_h
#define _dirView_h

// dirView.h
//------------------------------------------
// synopsis:
// Visual representation of instance of class directory.
//
//------------------------------------------

#ifndef _genString_h
#include <genString.h>
#endif
#ifndef _genArrInt_h
#include <genArrInt.h>
#endif
#ifndef _directory_h
#include <directory.h>
#endif
#ifndef _gt_h
#include <gt.h>
#endif

class gtList;
class gtPushButton;
class directory;
class configObj;

class dirView
{
  public:
    dirView(gtBase* parent, const char* pathname);
    ~dirView();

    void set_order(dirSortOrder);
    void filter_show(const char*);
    void filter_hide(const char*);
    void show_shadow(int);
    void update();

    gtForm* top() { return _top; }
    gtList* list() { return _list; }
    directory* dir() { return _dir; }
    configObj *build_config_tree();
    const dirEntry* entry(unsigned int);
    directory* get_current_dir();

  private:
    static void sort_by_name(gtPushButton*, gtEventPtr, void*, gtReason);
    static void sort_by_type(gtPushButton*, gtEventPtr, void*, gtReason);
    static void sort_by_modified(gtPushButton*, gtEventPtr, void*, gtReason);
    static void toggle_shadow(gtToggleButton*, gtEventPtr, void*, gtReason);

    directory* _dir;
    directory* _sdir;
    gtForm* _top;
    gtLabel* label;
    gtList* _list;
    genString _filter_show;
    genString _filter_hide;
    dirSortOrder order;
    genArrOf(int) indices;
    unsigned int shadow : 1;
    directory& current_dir();

};

#endif // _dirView_h

/*
    START-LOG-------------------------------

    $Log: dirView.h  $
    Revision 1.1 1993/05/27 16:23:12EDT builder 
    made from unix file
 * Revision 1.2.1.3  1992/10/19  15:52:58  oak
 * Added get_current_dir().
 *
 * Revision 1.2.1.2  92/10/09  19:56:44  kws
 * Fix comments
 * 

    END-LOG---------------------------------
*/

