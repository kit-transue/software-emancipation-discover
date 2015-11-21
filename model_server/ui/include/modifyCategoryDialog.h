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
#ifndef _modifyCategoryDialog_h
#define _modifyCategoryDialog_h

//
// modifyCategoryDialog.h
//------------------------------------------
// header for modifyCategoryDialog class
//
// copied and modified from projectScope.h
//

// INCLUDE FILES

#ifndef  _gt_h
#include <gt.h>
#endif
#ifndef  _gtLabel_h
#include <gtLabel.h>
#endif
#ifndef  _gtStringEd_h
#include <gtStringEd.h>
#endif
#ifndef  _gtTextEd_h
#include <gtTextEd.h>
#endif
#ifndef  _gtPushButton_h
#include <gtPushButton.h>
#endif
#ifndef  _gtFrame_h
#include <gtFrame.h>
#endif
#ifndef  _gtForm_h
#include <gtForm.h>
#endif
#ifndef  _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef  _gtAppShell_h
#include <gtAppShell.h>
#endif
#ifndef  _gtList_h
#include <gtList.h>
#endif

#ifndef  _genString_h
#include <genString.h>
#endif

// CLASS DEFINITIONS

class modifyCategoryDialog
//
// Modify Categories dialog box, to assign styles to categories, one at a time
//
// based on gtDialogTemplate
//
// individual member functions are documented in modifyCategoryDialog.h.C
//
{
  public:

    modifyCategoryDialog(gtBase* parent);
    ~modifyCategoryDialog();

  private:

    // callbacks
    static void 	ok_CB     	(gtPushButton*, gtEventPtr, void*, gtReason);
    static void  	apply_CB  	(gtPushButton*, gtEventPtr, void*, gtReason);
    static void 	cancel_CB	(gtPushButton*, gtEventPtr, void*, gtReason);
    //                  help_CB         none; n/a for gtDialogTemplate help buttons 
    static void		select_category_CB
					(gtList*,       gtEventPtr, void*, gtReason);
    // data members
    gtDialogTemplate*	dialog_template;	// "backbone" widget
    gtLabel*		description;	      	// holds 1-line description of dialog box
    gtList*	       	category_list;	    
    gtList*		style_list;  	      
};



#endif // _modifyCategoryDialog_h
