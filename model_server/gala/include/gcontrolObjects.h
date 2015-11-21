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
#ifndef _gcontrolObjects_h
#define _gcontrolObjects_h

/**********
 *
 * gcontrolObject.h
 *
 **********/


#ifndef vcomboINCLUDED
   #include vcomboHEADER
#endif

#ifndef vnotebookINCLUDED
   #include vnotebookHEADER
#endif

#ifndef vdragINCLUDED
   #include vdragHEADER
#endif

#ifndef vspringdomINCLUDED
   #include vspringdomHEADER
#endif

#ifndef vlistINCLUDED
    #include vlistHEADER
#endif
#ifndef vspinnerINCLUDED
    #include vspinnerHEADER
#endif

#ifndef vrulerINCLUDED
    #include vrulerHEADER
#endif

#include <ggenDialog.h>
#include <gbar.h>
#include <gString.h>
#include <gview.h>
#include <gArray.h>

static const int LOCAL_STRING_SIZE = 1024;

class gdTextItem;
class TextFile;
class Scanner;

class gdWidget
{
  public:
    gdWidget() {dialog_ = NULL; pane_ = NULL; execCommand_ = NULL;}
    void SetTclCmd(vchar*);
    void InitgdWidget(vchar*,vdialogItem*);
    static char* UpdateVariable(ClientData, Tcl_Interp*, char*, char*, int);
    virtual void UpdateDisplay()  {;}
    virtual void InitObject(vchar*) {;}
    virtual void ChangeHasOccurred() {;}
    Tcl_Interp *interp();
    void SetInterpVariable (const vchar *, const vchar *);
    vchar *GetInterpVariable (vchar *);
    
  private:
    vchar* execCommand_;

  protected:
    GDialog* dialog_;
    Pane *pane_;
    int insideUpdate_;
    vdialogItem* item_;
};


class gdButton : public vbutton, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdButton);
    vloadableINLINE_CONSTRUCTORS(gdButton, vbutton);

    virtual void Open();
    virtual int  HandleButtonUp(vevent*);
    virtual int  HandleKeyDown(vevent *);
    virtual void ObserveDialogItem(vevent *event);

};

class gdSash: public vdialogLineItem, public gdWidget
{
  public:
    vspringdomObject *thisObj;
    gdSash *_prev;
    gdSash *_next;
    Pane *_left;
    Pane *_right;
    static int _minWidth;
    static int _minHeight;
    static vspringdomConnection *_target;
    virtual void Activate(void);
    virtual void Deactivate(void);


  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdSash);
    vloadableINLINE_CONSTRUCTORS(gdSash, vdialogLineItem);

    virtual void Open();
    virtual int  HandleButtonDown(vevent*);
    virtual void Draw();

    void SetNext (gdSash *);
    void SetPrev (gdSash *);
    void SetLeft (vdialogItem *);
    void SetRight (vdialogItem *);

    void DumpIt (vspringdom *);

    void MoveToX (int);
    void MoveToY (int);
    void MoveLeft (vcontainer *, vspringdom *, int, int);
    void MoveRight (vcontainer *, vspringdom *, int, int);
	  void MoveUp (vcontainer *, vspringdom *, int, int);
    void MoveDown (vcontainer *, vspringdom *, int, int);
  protected:
    int m_nOldPos;
	vcursor* cursor;
};

class gdSashDrag : public vdrag
{
  public: 
    // vkindTyped
    vkindTYPED_FULL_DECLARATION(gdSashDrag);
    
    // the standard versions of the necessary constructor/destructor
    vkindTYPED_INLINE_DEFAULT_CONSTRUCTOR(gdSashDrag, vdrag);
    vkindTYPED_INLINE_SUPPRESS_CONSTRUCTOR(gdSashDrag, vdrag);
    vkindTYPED_INLINE_DESTRUCTOR(gdSashDrag, vdrag);

    void SetSash (gdSash* wid, vevent*);

    virtual int HandleDetail (vwindow* from, vwindow* to, vevent* event);
    virtual void Draw(vrect *rect);
    virtual void ObserveDrag(vwindow *from, vwindow *to, vevent *event);

  protected:
    gdSash* wid;   // a link to the sash being moved
    int horizontal;
    int x_, y_;
    vwindow* w_;
    int w_ok_;
};

class gdOptionMenu : public vcontrolOptionMenu, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdOptionMenu);
    vloadableINLINE_CONSTRUCTORS(gdOptionMenu, vcontrolOptionMenu);

    virtual void Open();
    virtual void SetValue(int);
    virtual void UpdateDisplay();
};

class gdListItem : public vlistitem, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdListItem);
    vloadableINLINE_CONSTRUCTORS(gdListItem, vlistitem);

    virtual void Open();
    void Init(void);
    void LoadInit (vresource);
    void SetRect(const vrect *);
    vchar* GetCellString(int row);
    virtual void DrawCellContents (int row, int, int, vrect *drawrect);
    void ObserveSelect(int);
	int HandleKeyDown(vevent*);

    //    The following are used to turn off selection handling during multiple
    // selections.
    vbool allowSelection_;
    vbool AllowSelection();
    void AllowSelection(vbool handle);

    void InitWidth() { colwidth_ = 0; }

  protected:
    int                   colwidth_;
};

class gdIconListItem : public gdListItem
{
	//this class draws an icon from the gdIconListIcons dictionary to the left of the normal list text.
	// other wise it should behave normally
	//to use it, you have to set the icon list--
	/*eg  set myIconList {
							{return [llength $NameList]}
							{lindex $NameList $Item}
							{ //selection callback}
							{//doubleclick callback}
							{lindex $IconList $Item}
						}
	*/
	//it always makes space for the icon--if it doesn't find your icon (misspelling) it draws the default icon

public:
	vkindTYPED_FULL_DECLARATION(gdIconListItem);
    vloadableINLINE_CONSTRUCTORS(gdIconListItem, gdListItem);
	
	//initialization methods...
	void Init(void);
	void LoadInit(vresource res);

	//overrides of member functions
	virtual void DetermineCellSize(int row, int column, int * width, int * height);
	virtual void DrawCellContents (int row, int, int, vrect *drawrect);
	virtual void SetRowHeight(int row, int height);
	virtual void Destroy();

protected:
	//member functions
	virtual void doInit();
	vchar * GetCellIconName(int row);
	//class data
	static const int icon_x;  //width of icon
	static const int icon_y;  // height of icon
	static const int icon_space; //spacing between icon and text
	static int nRefCount;        //ref count of objects (used to free memory used to to hold icon images)
	static vdict * iconDict;     //format: vname *, vimage *   vname* is the tagname,

	//class functions
	static int LoadIcons(vdict *); //loads the icons
	static vimage * GetCellIcon(vchar *);  //retieves the specified icon (or the default one if it fails)
};



// Use it only with gdListView control.
// To use this class with other types of controls, pls re-define their Draw()
// function and insert gdListBox Draw() function after parent control drawing.
class gdListView;
class gdListCheck  {
  public :
	gdListCheck(gdListView* p);
	~gdListCheck();
    virtual void  Draw(void);
    virtual int   HandleButtonDown(vevent *event);
    virtual int   HandleButtonUp(vevent *event);
    virtual int   HandleKeyDown(vevent *event);
    virtual int   HandleKeyUp(vevent *event);

    // set checkbox main title :
	// 1.if set to NULL, no string will be displayed at the right of indicator
	// 2.if this title set to some string and secondary title is set to NULL
	//   (default situation) the checkbox will display this title not depending
	//   on it's status
	// 3.If secondary title is set by SetAlternateTitle function, this checkbox
	//   will display primary title when on and secondary when off.
	void  SetTitle(const vchar* s);
	void  SetAlternateTitle(const vchar* s); 
	// returns the internal image rectangle
	vrect*        GetIndicator(void);

	// Returns the checkbox value
    int          GetValue();
    // Sets the checkbox value without updating it.
	void         SetValue(int v);

    // Sets the checkbox value and immidiatly redraw it.
	void         SetValueAndRedraw(int v);

	void         SetRect(vrect* newRect);
	vrect*       GetRect(void);
	// the checkbox will not draw itself if it is completely outside this rectangle.
	// in the case of intersection it will still draw itself.
	void         SetClipRect(const vrect* rect);
	// this function sets the distance between checkbox rectangle and the
	// indicator (image) rectangle. The following will be performed:
	//   1. The yOffset will define the distance between bottom of the checkbox
	//      rectangle and bottom of indicator rectangle, as well as distance
	//      between top of checkbox rectangle and top of indicator rectangle.
	//   2. The xOffset will determide the distance between left side of 
	//      the checkbox rectangle and left side of indicator rectangle as well as
	//      the distance between indicator and checkbox title.
	void         SetIndicatorOffsets(int xOffset,int yOffset);
	// This function permit you to set new image resources to draw
	// images other than default ones for ON and OFF state.
	// You must use the names in the followind form:
    // <tag/tag/.../ImageTag>.
	void         SetImageResources(char* onPath, char* offPath);
    char* GetCallback(void);
	void  SetCallback(char* name);
protected:
	virtual void ParentDraw(void);
  protected:
	gString     title;
	gString     alternate;
	gdListView* parentWindow;
	vrect       windowRect;
	int         value;
	vrect       clipRect;
	int         x_shift;
	int         y_shift;
	vrect       indicator;
	gString     onImagePath;
	gString     offImagePath;
	gString     callback_str;
   
};

class gdListView;

class ListViewTextItem : public vtextitem
{
public:
    vloadableINLINE_CONSTRUCTORS(ListViewTextItem, vtextitem);
    vkindTYPED_FULL_DECLARATION(ListViewTextItem);
    
    static vdict *EdListTextitemKEY_BINDINGS;  // initialize to NULL

    void SetParentView(gdListView *theListView) { parentview = theListView; };
private:
    gdListView   *parentview;
    unsigned int dirty;
    unsigned int allowAny;
};

class gdListView;
class ListViewPopupMenu : public vmenu
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ListViewPopupMenu);
    vloadableINLINE_CONSTRUCTORS(ListViewPopupMenu, vmenu);

    // non-virtual member function
    void       SetListView(gdListView *alist) { list = alist; }
    gdListView *GetListView() const { return list; }
    void       SetRowCol(int r, int c) { row = r; col = c; }
    void       GetRowCol(int &r, int &c) { r = row; c = col; }

private:
    gdListView *list;
    int        row, col;
};

class ListViewPopupMenuItem : public vmenuItem
{
public:
    // The standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(ListViewPopupMenuItem);
    vloadableINLINE_CONSTRUCTORS(ListViewPopupMenuItem, vmenuItem);

    // virtual member function overrides
    virtual void ObserveMenuItem(vevent *event);

    ListViewPopupMenu *DetermineMenu();
};

typedef struct _PopupMenuItemDesc {
    char *title;
    char *callback;
} PopupMenuItemDesc;

typedef PopupMenuItemDesc *PopupMenuItemDescPtr;

gArray (PopupMenuItemDescPtr);

class ListViewCell {
public:
    char             *value;
    char             *data;
    int              read_only;
	gdListCheck*     check_button;
    gArrayOf         (PopupMenuItemDescPtr) popup_items;
};

class ListViewCell;
class ListViewItemList;

//=======================================================================================
// This class designed to speed image loading from resource file.
// We will use LRU strategy.
//=======================================================================================
typedef struct {
   vimage*  m_Image;
   gString* m_Name;
   int      m_refCount;
} BufferEntry;

typedef BufferEntry* BufferEntryPtr;

class imageBuffer {
public:
	// constructs image buffer with 8 places for images.
	imageBuffer(void);
	// constructs image buffer with selected number of image placeholders.
	imageBuffer(int size);
	// removes all previously selected images and image name table.
	~imageBuffer(void);
	// loads spesified image from the resource file. If this image is already in use it will return
	// it's address and increment it's reference cound to perform LRU strategy. If this image is not
	// in the image buffer and there are free places to hold this image the function will load the 
	// image into any empty place. If there are no empty places it will first remove the image 
	// with smallest reference count. If there is no such image in the resource file this function
	// will do nothing and return NULL;
    vimage* GetImageFromResource(gString* imageName);
protected:
	int findEmptyCell(void);      // returns the index of the first free cell in the buffer
	                              // or -1 if there are no free cells
	int findLRUCell(void);        // returns the index of the last recently used cell
	                              //
	int findName(gString* name);  // returns the index of the cell with given name or -1 if no such name. 
private:
	int             m_bufSize;     // buffer size in cells
	BufferEntryPtr* m_Buffer;   // buffer itself
};
//==========================================================================================



class gdRuler;
class gdListView: public vlistview, public gdWidget, public imageBuffer
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdListView);
    vloadableINLINE_CONSTRUCTORS(gdListView, vlistview);

    virtual void      Open();
    virtual void      Close();
    virtual int       HandleButtonDown(vevent *event);
    virtual int       HandleButtonUp(vevent *event);
	virtual int       HandleMotion(vevent *event);
	virtual int       HandleDrag(vevent *event);
    virtual int       HandleKeyDown(vevent *event);
    virtual int       HandleKeyUp(vevent *event);
    virtual void      Draw();
    virtual void      SetRect(const vrect *rect);
    virtual void      SetColumnWidth(int column, int width);
	virtual void      ObserveScroll(int deltaX, int deltaY);
    
    void              DrawCellContents(int row, int column,   int selected, vrect *drawrect);


    void              ObserveSelect(int);
    void              setColumns(int c);
    int               getColumns ();
    void              HandlePopupItem(ListViewPopupMenuItem *item);
    void              AddPopupItem(char *title, char *callback);
    void              SetChangeCallback(char *cb);
    void              SetDblClickCallback(char *cb);
    void              SetCellMenu(int row, int col, char *callback, int count, char **items);

    gdListCheck*      SetCellCheck(int row, int col,char* callback, int state);
	int               InspectCellCheck(int row, int col);
	void              ChangeCellCheck(int row, int col, int value);
	virtual void      OnCheck(int row, int column, int new_status);

    ListViewPopupMenu *BuildPopupMenu(gArrayOf (PopupMenuItemDescPtr) &popup_items, int r, int c);
    ListViewPopupMenu *CreateCellMenu(int r, int c);
    ListViewCell      *GetCellInfo(int row, int col);
    void              CreateInternalList();
    void              SetDataCell(int row, int col, char *value, int read_only, char *data);
    vchar             *GetCellValue(int row, int col);
    vchar             *GetCellData(int row, int col);

    void              PropagateItemChange(int row, int col, char *txt);
    void              CloseEditor();
    void              CreateEditor();
    void              RemoveAllRow(int row);
	void              Flush(void);

	void              SetCellChangeCallback(char* name);
	char*             GetCellChangeCallback(void);

	gdRuler           *_ruler;

  protected:
    virtual void Init();
    virtual void LoadInit(vresource resource);
    virtual void Destroy();
    void    CommonInit();
    gdListCheck*   CreateCheckButton(int row,int col);

  private:
    int              CurrentRow;
	int              CurrentColumn;
    ListViewTextItem *textitem;
    ListViewItemList *itemlist;
    vlist            *internal_list;
    int              columns;
    int              editing;
    int              edit_row;
    int              edit_col;
    char             *change_callback;
    char             *click_callback;
    gArrayOf (PopupMenuItemDescPtr) popup_items;
	gString          cell_change_callback;
};



class gdNumberSpinner: public vspinner, public gdWidget
{
public:
	vkindTYPED_FULL_DECLARATION(gdNumberSpinner);
    vloadableINLINE_CONSTRUCTORS(gdNumberSpinner, vspinner);
	virtual void Open();
	virtual void ObserveDialogItem(vevent *);
	virtual void UpdateDisplay();
	virtual void SetMax(int);
	virtual void SetMin(int);
	virtual void SetValue(int);

};




//----------------------------------------------------------------------------------------------------------
// This class was designed to represent Text Spinner in C++.
// The main idea is to embagge TCL callback to notify TCL every time selection changed.
//----------------------------------------------------------------------------------------------------------
class gdTextSpinner: public vspinnerText, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdTextSpinner);
    vloadableINLINE_CONSTRUCTORS(gdTextSpinner, vspinnerText);

    // Re-definition of Open. We need this to initialize gdWidget and for future
	// extentions - to perform some operations while opening this window.
    virtual void Open();

	// We need this function to embagge any string wich will be passed to TCL in
	// OnSelectionChanged to notify new spin selection.
    void SetChangeCallback(char *cb);

	// This function returns the TCL callback function string.
	char* GetChangeCallback(void);

	// This function returns current text spinner index
	int GetCurrentIndex(void);

	// This function returns the currently selected text string
	char * GetCurrentText(void);

	// This function cleans all text spin list
	void Clean(void);
  protected:
	// We need this function to catch GALAXY notifications and call OnSelectionChanged
	// function to pass new selection to the TCL.
    virtual void ObserveDialogItem(vevent *event);

	// Re-definition of Init. We need this to set empty string in TCL callback vString
	// and for future extentions - to initialize all data.
    virtual void Init();
	// Re-definition of Init. We need this to set empty string in TCL callback vString
	// and for future extentions - to initialize all data.
    virtual void LoadInit(vresource resource);
	// Re-definition of Destroy. Here all initialized memory blocks must be free.
	// (From my experience this function will never be called, but in future the 
	// situation can change.)
    virtual void Destroy();
    // Here all work common to Init and LoadInit must be done.
    void    CommonInit();
    // Responce table definition:
  protected:
	// Called every time spin selection changed. m_CurrentIndex & m_CurrentText 
	// contain old selections at this moment, will be updated immidiatly after
	// this function exit.This version just call TCL string if it is not empty.
	virtual void OnSelectionChanged(int newIndex, vchar* newText); 
  private:
	int              m_CurrentIndex;   // currently selected index
	gString          m_CurrentText;    // currently selected text
	gString          m_ChangeCallback; // stores TCL command called on SelectionChanged
};
//=======================================================================================



class ListViewItemList : public vdialogItemList
{
public:
    vkindTYPED_FULL_DECLARATION(ListViewItemList);
    vloadableINLINE_CONSTRUCTORS(ListViewItemList, vdialogItemList);
    
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

class gdNotebookPage : public vnotebookPage, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdNotebookPage);
    vloadableINLINE_CONSTRUCTORS(gdNotebookPage, vnotebookPage);

    virtual void ObserveDialogItem(vevent *event);
    virtual void Open();
	virtual void Select();
	void Update ();
};

class gdNotebook : public vnotebook, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdNotebook);
    vloadableINLINE_CONSTRUCTORS(gdNotebook, vnotebook);

    virtual void Open();
};

class gdProgressItem: public ProgressItem, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdProgressItem);
    vloadableINLINE_CONSTRUCTORS(gdProgressItem, ProgressItem);

    virtual void Open();
};

class gdTextObserver : public vtextObserver
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdTextObserver);
    vloadableINLINE_CONSTRUCTORS(gdTextObserver,vtextObserver);
    void InitObserver(gdWidget*);
    void ObserveDelete(long, long);
    void ObserveInsert(long, long);

  private:
    gdWidget* owner_;
};

class gdTextItemUpdateTimer : public vtimer {
  public:
    void    SetOwner(gdTextItem *ti) { owner = ti; };
    virtual void ObserveTimer();
  protected:
    gdTextItem *owner;
};

class gdPollTimer : public vtimer {
  public:
    virtual void ObserveTimer();
};

class gdTextEditor : public vtextview, public gdWidget
{
  friend 
	class gdWidget;
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdTextEditor);
    vloadableINLINE_CONSTRUCTORS(gdTextEditor,vtextview);
 
    virtual void Open();
    virtual void Unfocus();
    virtual void ChangeHasOccurred();
    virtual void UpdateDisplay();
    virtual void DrawBackground(vrect *r);
    virtual int  HandleKeyDown(vevent *event);
 
    void OpenFile (vchar* filename);
    void SaveFile ();
    void SaveFileAs (vchar* filename);
    void CloseFile ();
 
  protected:
    // virtual member function overrides
    virtual void Destroy();

  protected:
    int                   item_has_changed;
};

class gdTextItem : public vtextitem, public gdWidget
{
  friend class gdWidget;
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdTextItem);
    vloadableINLINE_CONSTRUCTORS(gdTextItem,vtextitem);

    virtual void Init();
    virtual void LoadInit(vresource);
    virtual void CopyInit(vloadable *);
    virtual void CommonInit();

    virtual void Open();
    virtual void Unfocus();
    virtual void ChangeHasOccurred();
    virtual void UpdateDisplay();
    virtual void UpdateFile(void);
    virtual int  ShowFile(vchar *fname, int keep_updating);
    virtual void StopUpdating(void);
    virtual void TimerNotifyCallback(void);
    virtual void HighliteLine(int line);
    virtual void DrawBackground(vrect *r);
    virtual int  HandleKeyDown(vevent *event);

    void SetUpdateCmd (vchar *updateCommand);
    void AddScanner (const vchar*, const vchar*);
    void OpenFile (vchar* filename);
    void SaveFile ();
    void SaveFileAs (vchar* filename);
    void CloseFile ();
    
  protected:
    // virtual member function overrides
    virtual void Destroy();
  protected:
    int                   item_has_changed;
    vfsFile               *vfile;
    char                  *file_buffer;
    long                  file_buffer_size;
    long                  allocated_buffer_size;
    gdTextItemUpdateTimer *timer;
    vchar * updateCommand_;
	Scanner *scanner;
};


class gdQueryTextItem : public vtextitem, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdQueryTextItem);
    vloadableINLINE_CONSTRUCTORS(gdQueryTextItem,vtextitem);

    virtual void Open();
    static char* RequestText(ClientData, Tcl_Interp*, char*, char*, int);
    virtual void UpdateDisplay();
};


class gdExclusiveGroup : public vcontainerExclusiveGroup, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdExclusiveGroup);
    vloadableINLINE_CONSTRUCTORS(gdExclusiveGroup,vcontainerExclusiveGroup);

    virtual void Open();
    virtual void UpdateDisplay();
    virtual void ObserveDialogItem (vevent *event);
};


class gdSlider: public vslider, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdSlider);
    vloadableINLINE_CONSTRUCTORS(gdSlider,vslider);

    virtual void Open();
    virtual void UpdateDisplay();
    virtual void ObserveDialogItem (vevent *event);
};


class gdToggle : public vcontrolToggle, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdToggle);
    vloadableINLINE_CONSTRUCTORS(gdToggle,vcontrolToggle);

    virtual void Open();
    virtual void ObserveDialogItem(vevent*);
    virtual void UpdateDisplay();
    
  private:
    vchar* variableName_;
    gdExclusiveGroup* parent_;
};


class gdComboBox : public vcombo, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdComboBox);
    vloadableINLINE_CONSTRUCTORS(gdComboBox, vcombo);

    virtual void Open();
    virtual void ObserveDialogItem(vevent*);
    virtual void UpdateDisplay();

};



class gdPopDownComboBox : public vcomboPopdown, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdPopDownComboBox);
    vloadableINLINE_CONSTRUCTORS(gdPopDownComboBox, vcomboPopdown);

    virtual void Open();
    virtual void ObserveDialogItem(vevent*);
    virtual void UpdateDisplay();

    void    SetMatchBehavior(int flag);

    // Compare proc.
    static int CompareProc(const vchar*, const vchar*);

    void Deactivate();

  private:
    static  void gdPopDownComboBoxObserver(vcombobox *combo, vevent *event, int message);
};

class gdRuler : public vruler, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdRuler);
    vloadableINLINE_CONSTRUCTORS(gdRuler, vruler);
    virtual void ObserveRuler (vevent *, vrulerItem *, int message);

    virtual void Init();
    virtual void LoadInit(vresource);
    virtual void CopyInit(vloadable *);

    void setListView (gdListView *);

  private:
    void updateListView ();
    void CommonInit();

    gdListView *lview_;
    int compacting;
};

class gdLabel : public vdialogLabelItem, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdLabel);
    vloadableINLINE_CONSTRUCTORS(gdLabel, vdialogLabelItem);

    virtual void Open();
    virtual void UpdateDisplay();
};

class gdMultiLineLabel : public vtextitem, public gdWidget
{
  public:
    // the standard versions of the necessary constructors/destructor
    vkindTYPED_FULL_DECLARATION(gdMultiLineLabel);
    vloadableINLINE_CONSTRUCTORS(gdMultiLineLabel, vtextitem);

    virtual void Open();
    virtual void UpdateDisplay();
};



// Inline functions for gListView class.

inline void gdListView::setColumns(int c) {columns = c;}
inline int gdListView::getColumns () {return columns;}

// Inline functions for Pane class.

inline Tcl_Interp *gdWidget::interp() { return dialog_ ? dialog_->GetInterpreter() : (pane_ ? (pane_->GetOwner() ? pane_->GetOwner()->interp() : NULL) : NULL);}

enum SPRINGCONTAINERTYPE
{
  CONT_UNDEFINED,
  CONT_HORIZONTAL,
  CONT_VERTICAL
};

class springContainer : public vcontainer
{
protected:
  virtual void LoadInit( vresource );
  SPRINGCONTAINERTYPE m_Type;
  int m_nFixedSize;
  int m_fReentrance;
public:
  vkindTYPED_FULL_DECLARATION(springContainer);
  vloadableINLINE_CONSTRUCTORS(springContainer, vcontainer);
  
  int GetFixedSize(){ return m_nFixedSize; }
  virtual void SetRect( const vrect * );
  virtual void GetMinSize( int *, int * );
  int InitWithType( SPRINGCONTAINERTYPE );
  SPRINGCONTAINERTYPE GetContainerType(){ return m_Type; } 
};

#endif

/**********     end of gcontrolObjects.h     **********/
