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
// 	$Id: splash.h 1.1 1996/10/17 09:46:06EDT kws Exp  $	

#ifndef invertINCLUDED
#define invertINCLUDED

// Some convenience values
enum {
   R                               = 0,
   G                               = 1,
   B                               = 2,
   RGB                             = 3,

// for vwindowComplementColors's array
// invertNUM_COMPLEMENTS is the number of complement sets
// invertNUM_COLORS_PER_COMPLEMENT is the number of colors per complement set

   invertNUM_COMPLEMENTS           = 4,
   invertNUM_COLORS_PER_COMPLEMENT = 2,

// The number of commands defined in invertDialog::LoadInit()
   invertNUM_COMMANDS              = 9,

// program status bits reflecting what's currently selected in the
// "Foreground" and "Background" menus
   invertOUTLINE                   = 1,
   invertFILLED                    = 2,
   invertSOLID                     = 4,
   invertSTRIPED                   = 8
};

// array type definitions
typedef vcolor** invert2DColorArray;
typedef vcolor*  invertColorArray;

// class definitions

// invertDialogItem is the interesting bit
class invertDialogItem : public vdialogItem
{
public:
  // the standard versions of the necessary constructors/destructor
  vloadableINLINE_CONSTRUCTORS(invertDialogItem, vdialogItem);
    
  // overrides from vkind
  vkindTYPED_FULL_DECLARATION(invertDialogItem);
    
  // virtual member functions

  // draw a rectangle corresponding to the current rubberband state
  virtual void DrawRubberRect(short options, const vrect *rect);

  // virtual member function overrides
  // vdialogItem overrides
  virtual void Draw();      
  virtual int  HandleButtonDown(vevent *event);
  virtual int  HandleButtonUp(vevent *event);
  virtual int  HandleDrag(vevent *event);
  // vkindTyped overrides
  virtual void LoadInit(vresource resource);
  virtual void Destroy();

  // non-virtual inline member functions

  // returns the private white instance
  vcolor *GetLocalWhite() const                 { return colors[3][0]; }
  // return the private black instance
  vcolor *GetLocalBlack() const                 { return colors[3][1]; }
  // returns the requested color instance
  vcolor *GetLocalColor(short x, short y) const { return colors[x][y]; }
  
  // returns TRUE is vwindow::ComplementColors succeeded, FALSE otherwise
  short HasComplementColors()     { return hasComplementColors; }

  // returns the currently stored drawing function
  int  GetFunction() const { return currentFunction; }
  // sets the current drawing function
  void SetFunction(int x) { currentFunction = x; }

  // returns the currently selected foreground option
  // (invertFILLED or invertOUTLINE)
  short GetForegroundOption() const { return foregroundStatus; }
  // sets the current forground option
  void  SetForegroundOption(int x) { foregroundStatus = x; }

  // returns the currently selected background option
  // (invertSOLID or invertSTRIPED)
  short GetBackgroundOption() const { return backgroundStatus; }
  // sets the current background option
  void  SetBackgroundOption(int x) { backgroundStatus = x; }

  // updates the rectangle being rubberbanded
  void SetRubberRect(int x, int y, int w, int h) {
    rubberRect.Set(x, y, w, h);
  }
  // clears the rectangle being rubberbanded
  void ClearRubberRect() { rubberRect.Set(0, 0, 0, 0); }

  // return the width of the rect being rubberbanded
  short GetRubberRectW() const { return rubberRect.GetWidth(); }
  // set the width of the rect being rubberbanded
  void  SetRubberRectW(int width) { rubberRect.SetWidth(width); }
  // return the height of the rect being rubberbanded
  short GetRubberRectH() const { return rubberRect.GetHeight(); }
  // set the height of the rect being rubberbanded
  void  SetRubberRectH(int height) { rubberRect.SetHeight(height); }
  // return the x-coordinate of the rect being rubberbanded
  short GetRubberRectX() const { return rubberRect.GetX(); }
  // return the y-coordinate of the rect being rubberbanded
  short GetRubberRectY() const { return rubberRect.GetY(); }
  // return the rect being rubberbanded as a vrect structure
  const vrect *GetRubberRect() const { return &rubberRect; }

  // update the cache (used to keep the screen consistent while rubberbanding)
  void UpdateCache(int x, int y, int w, int h) {
    cache.Set(x, y, w, h);
  }
  // return the currently cached rect
  const vrect *GetCache() const { return &cache; }

private: 
  // local methods
  
  // set the flag indicating that vwindow::ComplementColors was successful
  void  EnableComplementColors()  { hasComplementColors = vTRUE; }
  // see the implementation below
  void  DisableComplementColors(const char *message);

  // local variables

  vcolor    ***colors;          // an array of colors to monkey with
  vrect        rubberRect;          // the rect being rubberbanded
  vrect        cache;               // to keep the screen consistent
  int          currentFunction;     // the current drawing function
  short        foregroundStatus;    // status of the foreground
  short        backgroundStatus;    // status of the background
  vbool        hasComplementColors; // does vwindowComplementColors() work?
};

// invertDialog is a handy container for our dialog item
class invertDialog : public vdialog
{
public:
  // the default versions of the necessary constructors/destructor
  vloadableINLINE_CONSTRUCTORS(invertDialog, vdialog);

  // overrides from vkindTyped
  vkindTYPED_FULL_DECLARATION(invertDialog);

  // virtual member function overrides
  // vwindow
  virtual void ObserveWindow(vevent *event);
  // vkindTyped
  virtual void LoadInit(vresource resource);

  // static class instance methods

  // command functions
  static int useFuncXor(vcommandFunction *, vdict *);
  static int useFuncInvert(vcommandFunction *, vdict *);
  static int useFuncHilite(vcommandFunction *, vdict *);
  static int useFuncComplement(vcommandFunction *, vdict *);
  static int fileQuit(vcommandFunction *, vdict *);
  static int backgroundStriped(vcommandFunction *, vdict *);
  static int backgroundSolid(vcommandFunction *, vdict *);
  static int foregroundFilled(vcommandFunction *, vdict *);
  static int foregroundOutline(vcommandFunction *, vdict *);
  // query functions
  static int queryFuncComplement(vcommandFunction *, vdict *);

  // exported instance variables

  // a cached pointer to our dialog item to avoid multiple calls
  // to vdialog::FindItem
  invertDialogItem *content;
};

#endif /* invertINCLUDED */
