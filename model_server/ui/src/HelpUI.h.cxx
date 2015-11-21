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
// HelpUI.h.C
//------------------------------------------
// synopsis:
// UI for help buttons
//
//------------------------------------------
// Restrictions:
//------------------------------------------

#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#include <strstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "cLibraryFunctions.h"
#include "messages.h"
#include "msg.h"
#include "gtDlgTemplate.h"
#include "gtPushButton.h"
#include "gtForm.h"
#include "gtBase.h"
#include "ste_commonTree_functions.h"
#include "genString.h"
#include "viewerShell.h"
#include "view_creation.h"
#include "top_widgets.h"
#include "histview.h"
#include "HelpUI.h"
#include "Notifier.h"
#include "objOper.h"
#include "db.h"
#include "gtBaseXm.h"
#include "proj.h"

#ifndef _WIN32
#include "WinHelp.h"
#else
#undef WinHelp
#endif

static char HELPFILE[1024];


extern Display* UxDisplay;

struct {
    int id;
    const char* name;
  } msg_table[] = {
#include "dis_help.inc"
    -1, 0
  };

extern "C" WinHelp (Display*, char*, unsigned short, unsigned long);

extern "C" void ste_finalize( viewPtr v = NULL );


static int find_id (const char* name)
{
  for (int i=0; 1; i++) {
    if (msg_table[i].id < 0)
      break;
    if (strcmp (name, msg_table[i].name) == 0)
      return msg_table[i].id;
  }
  return 15750000;
}

static commonTreePtr find_index(const char* path, commonTreePtr ste_tree);
// defined in this file.

HelpUI* HelpUI::the_instance;

HelpUI::HelpUI()
: widget_index(0), already_tried_it(0), show_pathnames(0), use_helper_shell(TRUE)
{
  HELPFILE[0]=0;
  char* psetHome = getenv("PSETHOME");
  if(psetHome!=NULL)  strcat(HELPFILE,psetHome);
  strcat(HELPFILE,"/webhelp/model_browser/ciee_unix_webhelp.htm");

}

HelpUI::~HelpUI()
{}


HelpUI* HelpUI::instance()
{
    Initialize(HelpUI::instance);

    if(the_instance == NULL)
	the_instance = new HelpUI;

    return the_instance;
}


boolean HelpUI::LoadWidgetIndex()
{
    Initialize(HelpUI::LoadWidgetIndex);
    push_busy_cursor();

    already_tried_it = 1;

    widget_index = checked_cast(steDocument,
				projHeader::load_module(HELP_INDEX_NAME));
	
    if(widget_index == NULL)
	msg("Error: Could not restore the help widgets index file.") << eom;

    pop_cursor();
    return widget_index != NULL;
}


boolean HelpUI::WidgetIndexAvailable(boolean repeat_message)
//
// Tries to load the widget index file if it is not already loaded
//
{
    Initialize(HelpUI::WidgetIndexAvailable);

    if(!(widget_index  ||  already_tried_it))
    {
	already_tried_it = 1;
	repeat_message = 1;

	LoadWidgetIndex();
    }
    if(!widget_index  &&  repeat_message)
	msg("ERROR: The help system is unavailable.") << eom;

    return widget_index != 0;
}


commonTreePtr HelpUI::IndexRoot(boolean repeat_message)
{
    Initialize(HelpUI::IndexRoot);

    if (!WidgetIndexAvailable(repeat_message))
	return NULL;
   
    commonTreePtr ste_root = NULL;
    if(widget_index)
	ste_root = checked_cast(commonTree, widget_index->get_root());

    if(!ste_root && repeat_message)
    {
	msg("ERROR: The help system is unavailable.") << eom;
	msg("Error: could not find root of help widget index document.") << eom;
    }

    return ste_root;
}


void HelpUI::dump_widget_index()
{
    Initialize(HelpUI::dump_widget_index);

    HelpUI* hui = HelpUI::instance();
    hui->widget_index = NULL;
    hui->already_tried_it = 0;
}


void HelpUI::edit_widget_index()
{
    Initialize(HelpUI::edit_widget_index);
#ifndef NEW_UI
    HelpUI* hui = HelpUI::instance();

    if(hui->WidgetIndexAvailable(TRUE))
    {
	view_create(hui->widget_index, Rep_TextText);
	view_create_flush();
    }
    else
	msg("ERROR: The help system is unavailable.") << eom;
#endif
}


void HelpUI::set_show_pathnames(boolean new_value)
{
    Initialize(HelpUI::set_show_pathnames);

    HelpUI::instance()->show_pathnames = new_value;
}


boolean HelpUI::get_show_pathnames()
{
    Initialize(HelpUI::get_show_pathnames);

    return HelpUI::instance()->show_pathnames;
}


void HelpUI::set_use_helper_shell(boolean new_value)
{
    Initialize(HelpUI::set_use_helper_shell);

    HelpUI::instance()->use_helper_shell = new_value;
}


boolean HelpUI::get_use_helper_shell()
{
    Initialize(HelpUI::get_use_helper_shell);

    return HelpUI::instance()->use_helper_shell;
}


extern "C" void HelpUI_HelpWidget_CB(Widget w, void*)
{
    Initialize(HelpUI_HelpWidget_CB);
#ifndef NEW_UI
    char *path = widget_help_pathname(w, 1);

    if (::strcmp(path, "Pset.Help.Help") == 0)
	WinHelp (UxDisplay, HELPFILE, HELP_HELPONHELP, 0);
    else if (::strcmp(path, "Pset.Help.Toc") == 0)
	WinHelp (UxDisplay, HELPFILE, HELP_FINDER, 0);
    else {
      int id = find_id (path);

      if (id)
	WinHelp (UxDisplay, HELPFILE, HELP_CONTEXT, id);
      else
	WinHelp (UxDisplay, HELPFILE, HELP_FINDER, 0);
    }
#endif
}


extern void HelpUI_LongHelp(gtBase* base)
{
    Initialize(HelpUI_LongHelp);
#ifndef NEW_UI
    char* path = base->help_pathname(1);

    if (::strcmp(path, "Pset.Help.Help") == 0)
	WinHelp (UxDisplay, HELPFILE, HELP_HELPONHELP, 0);
    else if (::strcmp(path, "Pset.Help.Toc") == 0)
        WinHelp (UxDisplay, HELPFILE, HELP_FINDER, 0);
    else {
      int id = find_id (path);
      
      if (id)
	WinHelp (UxDisplay, HELPFILE, HELP_CONTEXT, id);
      else
	msg("ERROR: Help context not found - $1") << path << eom;
    }
#endif
}


extern void HelpUI_LoadWidgetIndex()
{
    Initialize(HelpUI_LoadWidgetIndex);

    HelpUI::instance()->LoadWidgetIndex();
}


static commonTreePtr find_index(const char* path, commonTreePtr ste_tree)
//
// recursively look for the widget path in the ste document using
// the ste titles.  Returns the ste title node, or null if it could
// not find it.
//
{
    Initialize(find_index);

    commonTreePtr title_node = NULL;
    while (!(title_node = ste_get_title(ste_tree)))
    {
	ste_tree = checked_cast(commonTree,ste_tree->get_first());
	commonTreePtr found_node;
	while (ste_tree)
	{
	    found_node = find_index(path, ste_tree);
	    if (found_node)
		return found_node;
	    ste_tree = checked_cast(commonTree, ste_tree->get_next());
	}
	return(0);
    }
    char* tree_title = NULL;
    ostrstream os;
    if (title_node)
    {
	title_node->send_string(os);
	os << ends;
	tree_title = os.str();
	os.rdbuf()->freeze(0);
    }

    int len = -1;
    for (int i=0; path && path[i]; i++)
	if (path[i] == '.')
	    { len = i; break; }
    
    if ((char*)tree_title && (len>0)
	&& (strlen(tree_title)==len) && !strncmp(tree_title, path, len))
    {
	commonTreePtr found_node;
	while (title_node = checked_cast(commonTree,title_node->get_next()))
	{
	    if (found_node = find_index(path+len+1, title_node))
		return(found_node);
	}	    
    }
    else if ((char*)tree_title && (len == -1) && !strcmp(tree_title, path))
    {
	commonTreePtr child;
	if ((child = checked_cast(commonTree, title_node->get_first()))
                && !(child->get_next())) // if has one and only one child
	    return(checked_cast(commonTree,child->get_first_leaf()));
	else
	    return(title_node);
    }

    return(0);
}


void HelpUI::OpenSection(const char* path)
{
    Initialize(HelpUI::OpenSelection);    
#ifndef NEW_UI
    if (strcmp(path, "Pset.Help.Help") == 0)
	WinHelp (UxDisplay, HELPFILE, HELP_HELPONHELP, 0);
    else if (::strcmp(path, "Pset.Help.Toc") == 0)
        WinHelp (UxDisplay, HELPFILE, HELP_FINDER, 0);
    else {
      int id = find_id (path);
      
      if (id)
	WinHelp (UxDisplay, HELPFILE, HELP_CONTEXT, id);
      else
	WinHelp (UxDisplay, HELPFILE, HELP_FINDER, 0);
    }
#endif
    return;
}


static commonTreePtr find_first_paragraph(commonTreePtr ste_node)
{
    Initialize(find_first_paragraph);

    if(ste_node)
    {
	while(ste_node = checked_cast(commonTree, ste_node->get_next()))
	{
	    commonTreePtr sub_node =
		checked_cast(commonTree, ste_node->get_first());
	    if (sub_node->get_node_type() == stePAR)


		return sub_node;
	}
    }
    return NULL;
}


void HelpUI_instance_edit_widget_index()
{
    HelpUI::instance()->edit_widget_index();
}

void HelpUI_instance_dump_widget_index()
{
    HelpUI::instance()->dump_widget_index();
}

boolean HelpUI_get_set_show_pathnames()
{
      boolean new_value = !HelpUI::get_show_pathnames();
      HelpUI::set_show_pathnames(new_value);
      return new_value;
}

boolean HelpUI_get_set_use_helper_shell()
{
    boolean new_value = !HelpUI::get_use_helper_shell();
    HelpUI::set_use_helper_shell(new_value);
    return new_value;
}


/*
   START-LOG-------------------------------------------

   $Log: HelpUI.h.C  $
   Revision 1.30 2003/04/15 12:26:28EDT Dmitry Ryachovsky (dmitry) 
   
   Revision 1.29 2002/09/25 09:59:05EDT skallem 
   Removed reference to Clearcase /paraset for an include file path 
   Revision 1.28 2002/02/20 16:48:56EST ktrans 
   Rebrand DISCOVER
Revision 1.2.1.19  1994/07/27  00:00:39  jethran
removed quickhelp stuff as part of overall removal of quickhelp

Revision 1.2.1.18  1994/04/20  18:25:06  boris
fixed bug7022 with crash in ste_put_obj

Revision 1.2.1.17  1993/04/23  22:07:03  jon
Added view_set_help_focus() to indicated views that are intended
for the Helper Shell so the target button will be ignored. (bug 3373)

Revision 1.2.1.16  1993/03/25  20:09:58  jon
bug 2926 was caused by the fact that STE title nodes may have a
single child of type zero that actually contains the hyperlink.
I modified find_index to return the only child of a title node
instead of the title node itself.

Revision 1.2.1.15  1993/02/15  15:12:38  jon
Removed read-only parameter to display_appTree()

Revision 1.2.1.13  1993/02/04  20:27:53  jon
Changed load mech. to call projectHeader::load_module

Revision 1.2.1.12  1993/02/03  18:02:46  jon
Mapped logical name before calling db_restore

Revision 1.2.1.11  1993/02/03  16:29:19  jon
Changed index name again

Revision 1.2.1.10  1993/01/26  01:11:07  jon
Changed filename of help_index

Revision 1.2.1.9  1993/01/25  20:58:34  jon
Enabled and expanded QuickHelp, added help context paths.

Revision 1.2.1.8  1992/12/12  07:14:56  glenn
New view creation interface - separate help window.

Revision 1.2.1.7  1992/11/23  23:31:28  jon
Added .txt to help_widget_index filename to make it consistant with
the rest of the online doc filenames

Revision 1.2.1.6  1992/11/23  21:59:34  wmm
typesafe casts.
.`

Revision 1.2.1.5  1992/11/05  18:09:28  jon
Added show get_show_pathnames & set_show_pathnames

Revision 1.2.1.4  92/10/15  20:15:36  jon
Used freeze(0) on an ostrstream to avoid an ostrstream lib error

Revision 1.2.1.3  92/10/11  14:40:17  jon
Fixed purify error with stream.freeze(0)

Revision 1.2.1.2  92/10/09  20:13:33  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/

