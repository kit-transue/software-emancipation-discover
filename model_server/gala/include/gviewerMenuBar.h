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
/**********
 *
 * gviewerMenuBar.h - ViewerMenuBar class defs.
 *
 **********/

#ifndef _viewerMenuBar_h
#define _viewerMenuBar_h

#ifndef vportINCLUDED
#include "vport.h"
#endif

#ifndef vliststrINCLUDED
#include vliststrHEADER
#endif

#ifndef vmenubarINCLUDED
#include vmenubarHEADER
#endif

#include <gstate.h>

#include <tcl.h>

class ViewerMenuItemList;
class ViewerMenuItem;
class ViewerMenu;
class ViewerMenuBar;
class Viewer;
class MyDialog;

static const int MENU_HEIGHT = 30;

class ViewerMenuBar : public vmenubar
{
  public:     // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ViewerMenuBar);
    vloadableINLINE_CONSTRUCTORS(ViewerMenuBar, vmenubar);
    void Destroy();
    void InitViewerMenuBar (Viewer*, MyDialog*);
    void ShowItem (const vchar*);
    void HideItem (const vchar*);
    void EnableItem (const vchar*);
    void DisableItem (const vchar*);
    void ActivateItem (const vchar*);
    void DeactivateItem (const vchar*);
    void ShowMenu (const vchar*);
    void HideMenu (const vchar*);
    void EnableMenu (const vchar*);
    void DisableMenu (const vchar*);
    ViewerMenu* AddNewMenu(const vchar*, const vchar*, const vchar*);
    int AddGroupMenu(const vchar*, const vchar*, const vchar*);
    int AddAppMenu(const vchar*, const vchar*, const vchar*);
    ViewerMenuItemList* FindItem (const vchar*);
    ViewerMenu* FindMenu (const vchar*);
    ViewerMenu* FindTopMenuByName (const vchar*);
    void UpdateColors ();
    ViewerMenu *GetMenuList();
    void PropagateState(const vchar*);
    StateGroup* GetStateGroup() {return stateGroup;}
    Tcl_Interp* interp();

    static void SetMenuDataFile(const vchar*);
    static void SetQHelpDataFile(const vchar*);
    
  private:
    ViewerMenu *myMenuList_;     // A pointer to the list of menus
    ViewerMenu *oldMenu_;        // a pointer to the menu added last
    Viewer* viewer_;
    vliststr* legalStateList_;
    Tcl_Interp* interp_;
    vchar* oldState_;
    StateGroup* stateGroup;

    static vchar* custom_menus;
    static vchar* custom_qhelp;
};


#endif












