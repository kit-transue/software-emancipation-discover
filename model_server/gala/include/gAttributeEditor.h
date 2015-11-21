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
#ifndef gAttributeEditorINCLUDED
#define gAttributeEditorINCLUDED


#ifndef vstdioINCLUDED
#include <vstdio.h>
#endif

#ifndef vcharINCLUDED
#include vcharHEADER
#endif

#ifndef vstrINCLUDED
#include vstrHEADER
#endif

#ifndef vnameINCLUDED
#include vnameHEADER
#endif

#ifndef vrectINCLUDED
#include vrectHEADER
#endif

#ifndef vdictINCLUDED
#include vdictHEADER
#endif

#ifndef vcolorINCLUDED
#include vcolorHEADER
#endif

#ifndef vkeyINCLUDED
#include vkeyHEADER
#endif

#ifndef vdialogINCLUDED
#include vdialogHEADER
#endif

#ifndef vlistINCLUDED
#include vlistHEADER
#endif

#ifndef vlistviewINCLUDED
#include vlistviewHEADER
#endif

#ifndef vtextitemINCLUDED
#include vtextitemHEADER
#endif

#include <gcontrolObjects.h>
#include "gdialog.h"

class AttributeEditorTextItem;
class AttributeEditorItemList;


enum AttributeType { ATT_STRING, ATT_ENUM, ATT_INT };

class AttributeEnumVal {
public:
    AttributeEnumVal(char *val) { name = val; next = NULL; }

    gString          name;
    AttributeEnumVal *next;
};

class AttributeInfo {
public:
    AttributeInfo(char *nm, AttributeType tp, char *sc) { name = nm; type = tp; script = sc; enum_vals = NULL; next = NULL; };
    ~AttributeInfo();

    gString          name;
    gString          value;
    gString          script;
    AttributeType    type;
    AttributeInfo    *next;
    AttributeEnumVal *enum_vals;
};

class gdAttributeEditor : public vlistview, public gdWidget
{
  public:
    // the default versions of the necessary constructors/destructor
    vloadableINLINE_CONSTRUCTORS(gdAttributeEditor, vlistview);
    
    // overrides from vkindTyped
    vkindTYPED_FULL_DECLARATION(gdAttributeEditor);
    
    virtual void Open();
    virtual int  HandleButtonDown(vevent *event);
    virtual int  HandleButtonUp(vevent *event);
    virtual int  HandleKeyDown(vevent *event);
    virtual int  HandleKeyUp(vevent *event);
    virtual void Draw();
    virtual void SetRect(const vrect *rect);

    void          AddAttribute(char *name, char *type, char *script);
    void          AddEnumAttributeValue(char *name, char *enum_val);
    void          SetAttributeValue(char *name, char *value);
    void          Reset();
    AttributeInfo *GetAttribute(int r);
    AttributeType GetAttributeType(int r);
    void          SetEnumValue(char *txt);
    void          PropagateItemChange(int row, char *txt);
    void          UpdateList(void);
    void          CreateEditor();
    void          CloseEditor();
    void          PopupEnum(int r, vevent *event);

  protected:
    virtual void Init();
    virtual void LoadInit(vresource resource);
    virtual void Destroy();
    virtual void CommonInit();
   
  private:
    virtual AttributeEditorTextItem *CreateTextItem();
    
    AttributeInfo           *attribute_list;
    AttributeEditorTextItem *textitem;
    AttributeEditorItemList *itemlist;
    int                     edit_row, edit_col;
    int                     editing;
};


class AttributeEditorItemList : public vdialogItemList
{
public:
    // overrides from vkindTyped
    vkindTYPED_FULL_DECLARATION(AttributeEditorItemList);
    // standard versions of the constructors/destructor
    vloadableINLINE_CONSTRUCTORS(AttributeEditorItemList, vdialogItemList);
    
    virtual void DetermineBounds(vrect *rect);
    virtual void DetermineColorsFrom(long flags, vcolor **known,
				     int *ids, int count,
				     vcolor **colors);
    virtual void DetermineColorSchemeFrom(vcolorScheme *known,
					  unsigned long flags,
					  vcolorScheme *scheme);
    virtual void DetermineContent(vrect *rect);
    virtual vcursor *DetermineCursor();
    virtual vdialog *DetermineDialog();
    virtual int DetermineEnabled();
    virtual vfont *DetermineFont();
    virtual int DetermineLineWidth();
    virtual int DetermineShown();
    virtual void DrawDevice();
    virtual vdict *GetKeyDispatch();
    virtual vdict *GetKeyMap();
};


class AttributeEditorTextItem : public vtextitem
{
public:
    // the default versions of the necessary constructors/destructor
    vloadableINLINE_CONSTRUCTORS(AttributeEditorTextItem, vtextitem);
    // overrides from vkindTyped
    vkindTYPED_FULL_DECLARATION(AttributeEditorTextItem);
    
    static vdict *EdListTextitemKEY_BINDINGS;  // initialize to NULL

//    virtual void Copy(vloadable *original);
//    virtual void CommonInit();
//    virtual void Touch(int dirty);
    
    // non-virtual member functions
//    int IsDirty();
//    vchar *ToBuffer();
//    void  SelectAll();
//    gdSpreadSheet *GetParentView();
    void SetParentView(gdAttributeEditor *theListView) { parentview = theListView; };
//    int GetInsertFilterOverride();
//    void SetInsertFilterOverride(unsigned int b);
    
protected:
//    virtual void Init();
//    virtual void CopyInit(vloadable *original);
    
private:
    gdAttributeEditor *parentview;
    unsigned int      dirty;
    unsigned int      allowAny;
};

class AttributePopupMenu : public vmenu
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(AttributePopupMenu);
    vloadableINLINE_CONSTRUCTORS(AttributePopupMenu, vmenu);
    
    // non-virtual member function
    void              SetAttributeEditor(gdAttributeEditor *ae) { attribute_editor = ae; };
    gdAttributeEditor *GetAttributeEditor() const { return attribute_editor; };

private:
    gdAttributeEditor *attribute_editor;
};

class AttributePopupMenuItem : public vmenuItem
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(AttributePopupMenuItem);
    vloadableINLINE_CONSTRUCTORS(AttributePopupMenuItem, vmenuItem);

    // virtual member function overrides
    virtual void ObserveMenuItem(vevent *event);

    AttributePopupMenu *DetermineMenu();
};

#endif
