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
#include <cLibraryFunctions.h>
#include <ddict.h>
#include "dd_or_xref_node.h"

#include <gviewerShell.h>
#include <symbolArr.h>
#include <viewTypes.h>
#include <genString.h>
#include <customize.h>
#include <gviewerMenuBar.h>
#include <gviewerPopupMenuList.h>
#include "../../DIS_ui/interface.h"


static void LoadShell() {
 genString menus = customize::getStrPref("DIS_viewer.Menus_DataFile" );
 if( menus.not_null() && menus.length())
     ViewerMenuBar::SetMenuDataFile( (const vchar*) OSPATH(menus.str()) );
 genString qhelp = customize::getStrPref("DIS_viewer.QuickHelp_DataFile");
 if( qhelp.not_null() && qhelp.length())
     ViewerMenuBar::SetQHelpDataFile( (const vchar*) OSPATH(qhelp.str()) );
 genString pmenus = customize::getStrPref("DIS_viewer.PopupMenus_DataFile");
 if( pmenus.not_null() && pmenus.length())
     ViewerPopupMenuList::SetPopupMenuFile( (const vchar*) OSPATH(pmenus.str()) );
}

void open_view (viewtype vt, symbolArr &symbols)
{
        symbolPtr elem;
        ddElementPtr dd;
        int lock_flowchart=0;

    ForEachS(elem, symbols) {
           if (elem.is_xrefSymbol()) {
              if (elem.get_kind() != DD_FUNC_DECL) {
                     lock_flowchart = 1;
          }
           } else {
              if (is_ddElement(RelationalPtr(elem))) {
                          dd = ddElementPtr(RelationalPtr(elem));
                      if (dd->get_kind() == DD_MODULE) {
                            lock_flowchart = 1;
              }
          }
           }
        }
        if(vt == viewFlowchart && lock_flowchart == 1) {
#ifdef _WIN32
          rcall_dis_DISui_eval_async (Application::findApplication("DISui"),
          (vchar*)"dis_confirm1 \"Invalid Selection\" \"OK\" \"Only functions can be displayed in\nflowcharts. Please make sure that the\nselection contains only functions.\"");
#endif
                return;
        }


    gviewerShell *currentViewer = gviewerShell::GetCurrentViewer();
    if (!currentViewer) {
       LoadShell();
       currentViewer = new gviewerShell(vt, symbols); 
    } else {
        currentViewer->createView (vt, symbols);
    }
}

void open_view (view *vp)
{
    gviewerShell *currentViewer = gviewerShell::GetCurrentViewer();
    if (!currentViewer) {
       LoadShell();
       currentViewer = new gviewerShell(vp);
    } else {
        currentViewer->createView (vp);
    }
}
