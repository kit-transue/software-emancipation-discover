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
#ifndef _gt_h
#define _gt_h

// gt.h
//------------------------------------------
// synopsis:
// Generic Toolkit Library enums, typedefs, and forward class decls.
//------------------------------------------

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

typedef union _XEvent* gtEventPtr;
typedef XEvent gtEvent;

enum gtCursor
{
    gtDefaultCursor,
    gtBusyCursor,

    gtNUMCURSORS // This one should be last.
};

enum gtCardinalDir
{
    gtDirUp    = 0,	gtTop    = 0,
    gtDirDown  = 1,	gtBottom = 1,
    gtDirLeft  = 2,	gtLeft   = 2,
    gtDirRight = 3,	gtRight  = 3
};

enum gtReason
{
    gtActivate,
    gtValueChanged,
    gtDrag,
    gtHelp,
    gtNoMatch,
    gtChanged,
    gtIncrement,
    gtDecrement,
    gtPageIncrement,
    gtPageDecrement,
    gtModifyText,
    gtToTop,
    gtToBottom,
    gtExpose,
    gtResize,
    gtDestroy,
    gtSingleSel,
    gtMultipleSel,
    gtBrowseSel,
    gtExtendedSel,
    gtDefaultAction,
    gtFocus,
    gtUnfocus,
    gtMap,
    gtUnmap,
    gtEnter,
    gtLeave
};

enum gtResizeOption
{
    gtResizeNone,
    gtResizeGrow,
    gtResizeAny
};

enum gtFrameStyle
{
    gtETCHED_IN,
    gtETCHED_OUT,
    gtIN,
    gtOUT
};

enum gtAlign
{
    gtBeginning,
    gtCenter,
    gtEnd
}; 

enum gtListStyle
{
    gtSingle,
    gtMultiple,
    gtBrowse,
    gtExtended
};

enum gtMessageType
{
    gtMsgError,
    gtMsgInformation,
    gtMsgPlain,
    gtMsgQuestion,
    gtMsgWarning,
    gtMsgWorking
};

enum gtMenuChoice
{
    gtMenuNull,
    gtMenuStandard,
    gtMenuCheck,
    gtMenuCascade,
    gtMenuSeparator,
    gtMenuLabel,
    gtMenuSetLoc,
    gtMenuRadio
};

enum gtDirection
{
    gtHORZ,
    gtVERT
};

enum gtSeparatorStyle
{
    gtNONE,
    gtSINGLE,
    gtDOUBLE,
    gtSINGLE_DASHED,
    gtDOUBLE_DASHED,
    gtSHADOW_IN,
    gtSHADOW_OUT
};

enum gtPacking
{
    gtNoPacking,
    gtPackTight,
    gtPackColumn
};

class gtApplicationShell;
class gtArrowButton;
class gtBase;
class gtBitmap;
class gtBlank;
class gtCascadeButton;
class gtColormap;
class gtDialogShell;
class gtDialogTemplate;
class gtFileSelector;
class gtForm;
class gtFrame;
class gtHorzBox;
class gtLabel;
class gtLabeledStringEditor;
class gtList;
class gtListDialog;
class gtMainWindow;
class gtManager;
class gtMatrix;
class gtMenuBar;
class gtMessageDialog;
class gtNoParent;
class gtOptionMenu;
class gtPulldownMenu;
class gtPanedWindow;
class gtPixmap;
class gtPopupMenu;
class gtPrimitive;
class gtPushButton;
class gtRep;
class gtRTL;
class gtRadioBox;
class gtScale;
class gtScrolledWindow;
class gtScrollbar;
class gtSelectionBox;
class gtSelectionDialog;
class gtSeparator;
class gtShell;
class gtString;
class gtStringEditor;
class gtTextEditor;
class gtToggleButton;
class gtToggleBox;
class gtTopLevelShell;
class gtTransientShell;
class gtVertBox;
class gtWindow;

typedef void (*gtEventHandler)(void*, caddr_t data, gtEventPtr);

typedef void (*gtArrowCB)(gtArrowButton*, gtEventPtr, void*, gtReason);
typedef void (*gtBlankCB)(gtBlank*, gtEventPtr, void*, gtReason);
typedef void (*gtCascadeCB)(gtCascadeButton*, gtEventPtr, void*, gtReason);
typedef void (*gtFormCB)(gtForm*, gtEventPtr, void*, gtReason);
typedef void (*gtListCB)(gtList*, gtEventPtr, void*, gtReason);
typedef void (*gtMatrixCB)(gtMatrix*, gtEventPtr, void*, gtReason);
typedef void (*gtOptionCB)(gtOptionMenu*, gtEventPtr, void*, gtReason);
typedef void (*gtPulldownCB)(gtPulldownMenu*, gtEventPtr, void*, gtReason);
typedef void (*gtPopupCB)(gtPopupMenu*, gtEventPtr, void*, gtReason);
typedef void (*gtPushCB)(gtPushButton*, gtEventPtr, void*, gtReason);
typedef void (*gtScaleCB)(gtScale*, gtEventPtr, void*, gtReason);
typedef void (*gtScrollCB)(gtScrollbar*, gtEventPtr, void*, gtReason);
typedef void (*gtStringCB)(gtStringEditor*, gtEventPtr, void*, gtReason);
typedef void (*gtTextCB)(gtTextEditor*, gtEventPtr, void*, gtReason);
typedef void (*gtToggleCB)(gtToggleButton*, gtEventPtr, void*, gtReason);
typedef void (*gtTLShellCB)(gtTopLevelShell*, gtEventPtr, void*, gtReason);

#if defined(sun5) || defined(__MC_LINUX__)
#include <libintl.h>
#else
#ifndef _WIN32
extern "C" const char* gettext(const char*);
#else
#define gettext( param ) (param)
#endif /* !_WIN32 */
#endif /* sun5 */
void gtFree(void*);


/*
   START-LOG-------------------------------------------

   $Log: gt.h  $
   Revision 1.8 2000/07/07 08:10:15EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.6  1993/05/24  17:10:14  glenn
 * Add changed_callback().
 *
 * Revision 1.2.1.5  1993/01/02  19:49:59  glenn
 * Add gtFree.
 *
 * Revision 1.2.1.4  1992/12/30  19:08:54  glenn
 * Add gtPacking.
 *
 * Revision 1.2.1.3  1992/10/23  17:39:48  glenn
 * Add gtCursor.
 *
// Revision 1.5  92/09/20  21:03:29  glenn
// Add gtEnter, gtLeave.
// 
// Revision 1.4  92/08/18  12:24:27  rfermier
// added gtColormap, new reasons
// 
// Revision 1.3  92/08/14  17:13:29  glenn
// Add gtFocus, gtUnfocus.
// 
// Revision 1.2  92/08/06  18:22:09  rfermier
// added gtListDialog
// 
// Revision 1.1  92/07/30  20:32:37  glenn
// Initial revision
// 
   END-LOG---------------------------------------------
 */

#endif // _gt_h
