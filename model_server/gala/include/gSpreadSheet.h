#ifndef gSpreadSheetINCLUDED
#define gSpreadSheetINCLUDED 1


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


enum {
  editingSLOP             = 5
};

// forward references
class SpreadSheetTextItem;
class SpreadSheetItemList;


class gdSpreadSheet : public vlistview, public gdWidget
{
public:
  // the default versions of the necessary constructors/destructor
  vloadableINLINE_CONSTRUCTORS(gdSpreadSheet, vlistview);

  // overrides from vkindTyped
  vkindTYPED_FULL_DECLARATION(gdSpreadSheet);


  // virtual member function overrides
  virtual int  IssueSelector(int selector, vdict *context);
  virtual int  HandleButtonDown(vevent *event);
  virtual int  HandleButtonUp(vevent *event);
  virtual int  HandleKeyDown(vevent *event);
  virtual int  HandleKeyUp(vevent *event);
  virtual int  HandleDrag(vevent *event);
  virtual void Focus();
  virtual void UnFocus();
  virtual void Open();
  virtual void Draw();
  virtual void SetRect(const vrect *rect);
  virtual void ObserveScroll(int h, int v);
  virtual void DrawBackground(int r, int c, int selected, vrect *rect);
  virtual void DrawSelection(int r, int c, vrect *rect);

  virtual void DetermineEditingRect(vrect *theRect);
  virtual SpreadSheetTextItem *CreateTextItem();
  virtual void PutDownValue(const vchar *value);
  virtual const vchar *PickUpValue();
  virtual int  EditCell(int row, int column);
  virtual int  DetermineMinRowH();
  virtual void ForceDrawTextItem();
  virtual int  LoseIfOutsideScrolls(vevent *event);

  // non-virtual member functions
  int  IsEditing();
  int  IsSelectedCell(int row, int column);
  int  GetEditingRow();
  int  GetSelectedCell(int *row, int *column);  
  void TriggerAcceptOnEnter(int onOff, int modifiersrequired);
  void TriggerAcceptOnReturn(int onOff, int modifiersrequired);
  SpreadSheetTextItem *GetTextItem();
  void Edit(int turnOn);
  void StopEdit();
  int  CellIsCompletelyVisible(int row, int col);
  int  TriggerAccept (vevent *event);
  int  EditingHappensInside();
  void InvalFirstCol();
  char * GetEditingValue(int row, int col);  
  void SetEditingCellValue(int row, int col, vstr *val);
  void CleanUp();
  void InsertOneEmptyRow(int row, int flag);
  void ClearEditingText();
  void AcceptEdits();
  void GetStartDragPt(vpoint *pt) { pt->Set(startPt_wd.GetX(), startPt_wd.GetY()); }
  void GetDifffromRoot(int *x, int *y) { *x = startPt.GetX()-startPt_wd.GetX();
                                         *y = startPt.GetY()-startPt_wd.GetY(); }
  int  Hasmodified(); 
  void SetModified(int val) { modified = val; }

protected:
  virtual void Init();
  virtual void LoadInit(vresource resource);
  virtual void CommonInit();
  virtual void Destroy();
  virtual void RejectEdits();

private:
  int CanQueryFocusAsTarget();

  vpoint                startPt;
  vpoint                startPt_wd;
  int                   editRow;
  int                   editCol;
  int                   lastX;
  int                   lastY;
  int                   modified;
  SpreadSheetItemList   *itemlist;
  SpreadSheetTextItem   *textitem;
  unsigned int          editing:1;
  unsigned int          inside:1;
  unsigned int          krtn:1;
  unsigned int          kent:1;
  unsigned int          conf:1;
};


// --- textitem class used by gdSpreadSheet view for its "floating" textitem.

class SpreadSheetTextItem : public vtextitem
{
public:
  // the default versions of the necessary constructors/destructor
  vloadableINLINE_CONSTRUCTORS(SpreadSheetTextItem, vtextitem);

  // overrides from vkindTyped
  vkindTYPED_FULL_DECLARATION(SpreadSheetTextItem);

  virtual void Copy(vloadable *original);
  virtual void CommonInit();
  virtual void Touch(int dirty);

  // non-virtual member functions
  int           IsDirty();
  vchar         *ToBuffer();
  void          SelectAll();
  gdSpreadSheet *GetParentView();
  void          SetParentView(gdSpreadSheet *theListView);
  int           GetInsertFilterOverride();
  void          SetInsertFilterOverride(unsigned int b);

protected:
  virtual void Init();
  virtual void CopyInit(vloadable *original);

private:
  gdSpreadSheet    *parentview;
  unsigned int  dirty;
  unsigned int  allowAny;
};


class SpreadSheetItemList : public vdialogItemList
{
public:
  // overrides from vkindTyped
  vkindTYPED_FULL_DECLARATION(SpreadSheetItemList);
  // standard versions of the constructors/destructor
  vloadableINLINE_CONSTRUCTORS(SpreadSheetItemList, vdialogItemList);

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


class MyDrag : public vdrag
{
public:
    // vkindTyped
    vkindTYPED_FULL_DECLARATION(MyDrag);

    // the standard versions of the necessary constructor/destructor
    vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(MyDrag, vdrag);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(MyDrag, vdrag);
    vkindTYPED_INLINE_DESTRUCTOR(MyDrag, vdrag);

    // virtual member function overrides
    virtual void Init();
    virtual void Draw(vrect *rect);
    virtual void ObserveDrag(vwindow *from, vwindow *to, vevent *event);
    virtual int HandleDetail(vwindow *from, vwindow *to, vevent *event);

    // non-virtual member function
    void SetGoodCursor(vcursor *cursor) { goodCursor = cursor; }
    vcursor *GetGoodCursor() const { return goodCursor; }
    void SetBadCursor(vcursor *cursor) { badCursor = cursor; }
    vcursor *GetBadCursor() const { return badCursor; }
    void SetOriginalCursor(vcursor *cursor) { originalCursor = cursor; }
    vcursor *GetOriginalCursor() const { return originalCursor; }

    void SetSpreadSheet(gdSpreadSheet *alist) { sslist = alist; }
    gdSpreadSheet *GetSpreadSheet() const { return sslist; }

private:
    vcursor *goodCursor;
    vcursor *badCursor;
    vcursor *originalCursor;

    gdSpreadSheet *sslist;
};

class MyPopupMenu : public vmenu
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(MyPopupMenu);
    vloadableINLINE_CONSTRUCTORS(MyPopupMenu, vmenu);

    // non-virtual member function
    void SetSpreadSheet(gdSpreadSheet *alist) { sslist = alist; }
    gdSpreadSheet *GetSpreadSheet() const { return sslist; }

private:
    gdSpreadSheet *sslist;
};



class MyPopupMenuItem : public vmenuItem
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(MyPopupMenuItem);
    vloadableINLINE_CONSTRUCTORS(MyPopupMenuItem, vmenuItem);

    // virtual member function overrides
    virtual void ObserveMenuItem(vevent *event);

    MyPopupMenu *DetermineMenu();
};

/*+------------------------------------------------------------------------+
 * private: inline member functions
 *+------------------------------------------------------------------------+
 */

inline int gdSpreadSheet::IsEditing() { return(this->editing);}

inline int gdSpreadSheet::GetEditingRow() {return(this->editRow);}

inline int gdSpreadSheet::IsSelectedCell(int r, int c)
{ return( this->GetList()->IsCellSelected(this->GetSelection(), (r), (c)));}

inline void gdSpreadSheet::TriggerAcceptOnEnter(int b, int m)
{this->kent = (b) ? TRUE : FALSE;
 this->conf = (m) ? TRUE : FALSE;}

inline void gdSpreadSheet::TriggerAcceptOnReturn(int b, int m)
{this->krtn = (b) ? TRUE : FALSE;
 this->conf = (m) ? TRUE : FALSE;}

inline void gdSpreadSheet::StopEdit()
{this->Edit(FALSE);}

inline SpreadSheetTextItem *gdSpreadSheet::GetTextItem()
{return(this->textitem);}

inline int gdSpreadSheet::EditingHappensInside()
{return(this->inside);}

inline gdSpreadSheet *SpreadSheetTextItem::GetParentView()
{return(this->parentview);}

inline void SpreadSheetTextItem::SetParentView(gdSpreadSheet *v)
{this->parentview = v;}

inline int SpreadSheetTextItem::IsDirty()
{return(this->dirty);}

inline int SpreadSheetTextItem::GetInsertFilterOverride()
{return(this->allowAny);}

inline void SpreadSheetTextItem::SetInsertFilterOverride(unsigned int b)
{this->allowAny = b;}

#endif
