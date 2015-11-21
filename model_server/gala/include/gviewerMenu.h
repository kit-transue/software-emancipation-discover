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
 * gviewerMenu.h - ViewerMenu class defs.
 *
 **********/

#ifndef _viewerMenu_h
#define _viewerMenu_h

#include <gviewerMenuBar.h>
#include <gviewerMenuItem.h>
#include <gString.h>

class ViewerMenuItemList;
class ViewerPopupMenuList;
class ViewerMenuItem;
class ViewerMenu;
class ViewerMenuBar;
class Viewer;
class MyDialog;
class StateGroup;


/*  This object represents a single menu, or submenu, in the menubar */

class ViewerMenu : public vmenu
{ 
  public:
    ViewerMenu(const vchar*, Viewer*, ViewerMenuBar*, ViewerMenu*, const vchar*,
               const vchar*);
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION (ViewerMenu);
    vloadableINLINE_CONSTRUCTORS(ViewerMenu,vmenu);

    void Destroy();
    static void DestroyAllMenus();
    virtual void Disable();
  
    int GetPosition();
    void Show();
    void Hide();
    ViewerMenuItemList* FindItem (const vchar*);
    ViewerMenu* FindMenu (const vchar*);
    ViewerMenu* FindMenuByName (const vchar*);
    void SetNext(ViewerMenu*);
    virtual void AppendNewItem(const vchar*, const vchar*, const vchar*, const vchar*);
    virtual void AppendLabelItem (const vchar *, const vchar *);
    virtual void AppendToggleItem(const vchar*, const vchar*, const vchar*,
                                  const vchar*, const vchar*);
    virtual void AppendSepItem(const vchar*);
    virtual ViewerMenu* AppendSubItem(const vchar*, const vchar*, const vchar*);
    virtual int AppendSubGroupItem(const vchar*, const vchar*, const vchar*);
    static ViewerMenu* FindMenu (int);
    int GetId();
    void UpdateColors();
    ViewerMenu* GetGlobalList();
    ViewerMenuItemList *GetMenuItemList();
    ViewerMenu *GetNext();
    void AddNewArgument(const vchar*);
    virtual void Open();
    virtual void Close();
  protected:
      void SetupStates(const vchar*);

  protected:
    Viewer* viewer_;
    ViewerMenu* next_, *prev_;
    ViewerMenuBar* myMenuBar_;
    int hidden_;
    ViewerMenuItemList *myItemList_;
    static int UniqId_;
    static ViewerMenu* sMenuList_;
    ViewerMenu* nextMenu_;
    int id_;
    ViewerMenuItemList* oldListItem_;
    ArgList* argList_;
};


class ViewerPopupMenu : public ViewerMenu
{
  public:
    ViewerPopupMenu (	const vchar*, 
			Viewer*, 
			ViewerPopupMenuList*, 
			ViewerMenu*, 
			const vchar*, 
			const vchar*,
			const vchar*);

    vkindTYPED_FULL_DECLARATION (ViewerPopupMenu);
    vloadableINLINE_CONSTRUCTORS (ViewerPopupMenu, ViewerMenu);

    void ExecutePopupCmd ();

  private:
    gString command_;
};


class ViewerAppMenu : public ViewerMenu
{
  public:
    ViewerAppMenu(const vchar*, Viewer*, ViewerMenuBar*, ViewerMenu*,
                  const vchar*, const vchar*);
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION (ViewerAppMenu);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR (ViewerAppMenu, ViewerMenu);

    void PurgeItemList();

  private:
    vchar tclBuildCmd_[1024];
    int   persistent;
};



class ViewerGroupMenu : public ViewerMenu
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION (ViewerGroupMenu);
    vloadableINLINE_CONSTRUCTORS (ViewerGroupMenu, ViewerMenu);

    ViewerGroupMenu(const vchar*, Viewer*, ViewerMenuBar*, ViewerMenu*,
                    const vchar*, const vchar*);   
    void NewSelection(int);
};


#endif

