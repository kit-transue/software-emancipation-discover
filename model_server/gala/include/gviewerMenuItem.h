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
#ifndef _viewerMenuItem_h
#define _viewerMenuItem_h

#ifndef vportINCLUDED
#include "vport.h"
#endif

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#ifndef vscribeINCLUDED
#include vscribeHEADER
#endif

#ifndef vliststrINCLUDED
#include vliststrHEADER
#endif

#ifndef vmenuINCLUDED
#include vmenuHEADER
#endif

static const vchar* HIDE_CODE = (vchar*)"hidden";
static const int LAST_ITEM = -1;
static const int STRING_SIZE = 1024;

class ViewerMenu;
class ViewerMenuBar;
class ViewerGroupMenu;
class Viewer;
class MyDialog;
class ViewerMenuItem;

vchar* ConvertScribeToString (vscribe *s);

class ArgList
{
  public:
    ArgList(const vchar*);
    ~ArgList();
    vchar* GetArg(const vchar*);
    void AddNewArgument(const vchar*);

  private:
    vchar* contents_;
};

class ViewerMenuItemList
{
 public:
    ViewerMenuItemList (ViewerMenu*, ViewerMenuItemList*, vmenuItem*);
    int GetPosition();
    void Show();
    void Hide();
    void Enable();
    void Disable();
    void SetOn(int);
    ViewerMenu* GetSubMenu (void);
    ViewerMenuItemList* FindItem (const vchar*);
    void SetNext(ViewerMenuItemList*);
    void DestroyList(); 
    void ResetStates(vliststr*);
    vmenuItem *GetMenuItem();
    ViewerMenuItemList *GetNext();
    void AddNewArgument(const vchar*);

  private:
    vmenuItem *myItem_;                // A link to the item stored by this cell
    ViewerMenuItemList *next_, *prev_; // links to the listitems before and after in the list
    ViewerMenu* myMenu_;               // A link to the menu this list is associated with
    int hidden_;                       // Whether or not this item is hidden
};


class ViewerMenuItem : public vmenuItem
{ 
  public:
    ViewerMenuItem(const vchar*, const vchar*, Viewer*, const vchar*);
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ViewerMenuItem);
    vloadableINLINE_CONSTRUCTORS(ViewerMenuItem,vmenuItem);
    void Destroy();
    virtual void ObserveMenuItem(vevent*);
    virtual void Hilite (const vrect*, const vrect*);
    virtual void Unhilite (const vrect*, const vrect*);
    vchar * GetTcl_Command();
    ArgList* GetArgList();
    void AddNewArgument(const vchar*);
    void SetUpAccelerator();
	void AddPropagation(ViewerMenu*);

  private:
    vchar* tclCommand_;       // The tcl command to be executed
    Viewer* myViewer_;        // The viewer to send tcl commands to
    ArgList* argList_;
};


class ViewerMenuLabelItem : public vmenuItem
{
  public:
    vkindTYPED_FULL_DECLARATION(ViewerMenuLabelItem);
    vloadableINLINE_CONSTRUCTORS(ViewerMenuLabelItem,vmenuItem);

    ViewerMenuLabelItem (const vchar *);

    void Hilite        (const vrect *bounds, const vrect *content);
    void HiliteDefault (const vrect *bounds, const vrect *content);
    void Unhilite      (const vrect *bounds, const vrect *content);
  private:

};


class ViewerMenuToggleItem : public vmenuToggleItem
{ 
  public:
    ViewerMenuToggleItem(const vchar*, const vchar*, const vchar*,
                         Viewer*, const vchar*, ViewerMenu*);
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ViewerMenuToggleItem);
    vloadableINLINE_CONSTRUCTORS(ViewerMenuToggleItem,vmenuToggleItem);
    void Destroy();
    virtual void ObserveMenuItem(vevent*);
    virtual void Hilite (const vrect*, const vrect*);
    virtual void Unhilite (const vrect*, const vrect*);
    vchar * GetTcl_Command();
    virtual void SetOn(int);
    void AddNewArgument(const vchar*);
    void SetUpAccelerator();

  private:
    vchar* onTclCommand_, *offTclCommand_;       // The tcl command to be executed
    Viewer* myViewer_;        // The viewer to send tcl commands to
    ArgList* argList_;
    ViewerMenu* owner_;
};


#endif
