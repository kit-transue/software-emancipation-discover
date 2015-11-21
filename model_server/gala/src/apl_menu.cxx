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
//-----------------------------------------------------------------------------
// apl_menu.C
//
// Menu manipulation commands.
//-----------------------------------------------------------------------------

#include <vport.h>
#ifndef vliststrINCLUDED
   #include vcharHEADER
#endif

#ifndef vliststrINCLUDED
   #include vstdioHEADER
#endif

#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif

#ifndef vmenuINCLUDED
   #include vmenuHEADER
#endif

#ifndef vmenubarINCLUDED
   #include vmenubarHEADER
#endif

#include <gviewerMenuItem.h>
#include <stdlib.h>

#include "ggeneric.h"
#include "gString.h"
#include "gviewerMenu.h"
#include "gapl_menu.h"
#include "gviewer.h"
#include "gviewerPopupMenuList.h"

#ifndef _TCL
   #include <tcl.h>
#endif

//-----------------------------------------------------------------------------

class ViewerMenuItemList;
class ViewerMenuItem;
class ViewerMenuBar;
class ViewerPopupMenuList;
class ViewerMenu;
class Viewer;
class MyDialog;
class StateGroup;

//-----------------------------------------------------------------------------

static ViewerMenuBar* currentMenuBar = NULL;
static ViewerPopupMenuList * currentPopupMenuList = NULL;

//-----------------------------------------------------------------------------

//
// returns: 0: error ( the path does not exist ), !=0: the path exists
//
vfsPath *
aplPathAccess(const vchar * file) {
    vfsPath * ret_val = (vfsPath *)0;

    if (file) {
	vfsPath * vfs_path = (vfsPath *)0;

	if (vfs_path = vfsPath::Build(vfsPATH_TYPE_NAME, file, NULL)) {
	    vbool acc_val = vFALSE;
	    vbool exc_val = vFALSE; // -- debugging purposes (unexpected exceptions)

	    vexWITH_HANDLING {
		acc_val = vfs_path->Access(vfsFILE_EXISTS);
	    } vexON_EXCEPTION {
		acc_val = vFALSE;
		if (!vfsGetAccessException())
		    exc_val = vTRUE;
	    } vexEND_HANDLING;
	    if (acc_val == vTRUE)
		ret_val = vfs_path;
	    else {
		delete vfs_path;
	    }
	}
    }

    return ret_val;
}

//
// returns: 0: cannot find file to load, !=0: the file is located
//
vfsPath *
aplGetLoadPath(const vchar * file) {
    vfsPath * ret_val = (vfsPath *)0;

    if (file) {
	if (!(ret_val=aplPathAccess(file))) {
	    vstr * app_dir = vfsPath::GetAppDir()->CloneString();
	    if (app_dir) {
		ret_val = aplPathAccess(app_dir);
		vstrDestroy(app_dir);
	    }
	}
    }

    return ret_val;
}

vbool aplMenuLoad (ViewerMenuBar* bar, const char* file)
{
    gInit (aplMenuLoad);

    currentMenuBar = bar;

    // -- load the menus
    vbool ret_val = vFALSE;

    vfsPath * vfs_file = aplGetLoadPath((const vchar *)file);
    if (vfs_file) {
	if (Tcl_EvalFile (g_global_interp, (char *)(vfs_file->CloneString())) == TCL_OK)
	    ret_val = vTRUE;
	delete vfs_file;
    }

    return ret_val;
}

//-----------------------------------------------------------------------------

vbool aplMenuLoad (ViewerPopupMenuList* popupList, const char* file) {

    gInit (aplMenuLoad);

    currentPopupMenuList = popupList;
    ViewerMenuBar * saveMenuBar = currentMenuBar;

    currentMenuBar = NULL;

    // -- load the menus
    vbool ret_val = vFALSE;

    vfsPath * vfs_file = aplGetLoadPath((const vchar *)file);
    if (vfs_file) {
	if (Tcl_EvalFile (g_global_interp, (char *)(vfs_file->CloneString())) == TCL_OK)
	    ret_val = vTRUE;
	delete vfs_file;
    }

    currentMenuBar = saveMenuBar;

    return ret_val;
}

//-----------------------------------------------------------------------------

int dis_MenuCreate (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuCreate);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 4)
	TCL_EXECERR ("wrong number of arguments")

    gString tagname;
    gString label;
    gString arguments;
    gString command;
    
    tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    label.put_value_scribed (vcharScribeSystem (argv[2]));
    if (argc >= 4) {
	arguments.put_value_scribed (vcharScribeSystem (argv[3]));
    }

    if (argc >= 5) {
	command.put_value_scribed (vcharScribeSystem (argv[4]));
    }
    
    if (currentMenuBar) currentMenuBar->AddNewMenu (	tagname, 
							label, 
							arguments);

    else if (currentPopupMenuList) currentPopupMenuList->AddNewMenu (	tagname, 
									label, 
									arguments, 
									command);
    
    Tcl_SetResult (interp, argv[1], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuCreateGroup (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuCreateGroup);

    if (currentMenuBar == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 3)
	TCL_EXECERR ("wrong number of arguments")

    gString tagname;
    gString label;
    gString arguments;

    tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    label.put_value_scribed (vcharScribeSystem (argv[2]));
    if (argc >= 4)
	arguments.put_value_scribed (vcharScribeSystem (argv[3]));

    currentMenuBar->AddGroupMenu (tagname, label, arguments);

    Tcl_SetResult (interp, argv[1], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuCreateDynamic (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuCreateDynamic);

    if (currentMenuBar == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 4)
	TCL_EXECERR ("wrong number of arguments")

    gString tagname;
    gString label;
    gString tclCmd;

    tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    label.put_value_scribed (vcharScribeSystem (argv[2]));
    tclCmd.put_value_scribed (vcharScribeSystem (argv[3]));

    currentMenuBar->AddAppMenu (tagname, label, tclCmd);

    Tcl_SetResult (interp, argv[1], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuAppendItem (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendItem);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 5)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    gString tagname;
    gString label;
    gString command;
    gString arguments;

    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    tagname.put_value_scribed (vcharScribeSystem (argv[2]));
    label.put_value_scribed (vcharScribeSystem (argv[3]));
    command.put_value_scribed (vcharScribeSystem (argv[4]));
    if (argc >= 6)
	arguments.put_value_scribed (vcharScribeSystem (argv[5]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);
    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])
		      
    menu->AppendNewItem (label, tagname, command, arguments);
    
    Tcl_SetResult (interp, argv[2], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuAppendLabelItem (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendLabelItem);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc != 4)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    gString tagname;
    gString label;

    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    tagname.put_value_scribed (vcharScribeSystem (argv[2]));
    label.put_value_scribed (vcharScribeSystem (argv[3]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);

    if (menu == NULL) TCL_EXECERR1 ("cannot find menu", argv[1])
		      
    menu->AppendLabelItem (label, tagname);
    
    Tcl_SetResult (interp, argv[2], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuSetToggleItem (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    gInit (dis_MenuSetToggleItem);
    Viewer* vr;
    View *v;
    Viewer::GetExecutingView (vr, v);
    if (vr) {
    	ViewerMenuItemList* menuItem;
    	if (vr->GetMenuBar()) menuItem = vr->GetMenuBar()->FindItem ((const unsigned char *)argv[1]);
    	if (!menuItem && vr->GetPopupList()) menuItem = vr->GetPopupList()->FindItem ((const unsigned char *)argv[1]);

	if (menuItem) {
	    ViewerMenuToggleItem *toggle = ViewerMenuToggleItem::CastDown(menuItem->GetMenuItem());
	    if (toggle) {
		toggle->SetOn (atoi(argv[2]) ? vTRUE : vFALSE);
 	    }
	}
    }

    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuAppendToggleItem (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendToggleItem);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 6)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    gString tagname;
    gString label;
    gString onCommand, offCommand;
    gString arguments;

    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    tagname.put_value_scribed (vcharScribeSystem (argv[2]));
    label.put_value_scribed (vcharScribeSystem (argv[3]));
    onCommand.put_value_scribed (vcharScribeSystem (argv[4]));
    offCommand.put_value_scribed (vcharScribeSystem (argv[5]));
    if (argc >= 7)
	arguments.put_value_scribed (vcharScribeSystem (argv[6]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);

    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])

    menu->AppendToggleItem (label, tagname, onCommand, offCommand, arguments);

    Tcl_SetResult (interp, argv[2], TCL_VOLATILE);
    return TCL_OK;
}


//-----------------------------------------------------------------------------
int dis_MenuItemEnable (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
register i;

    gInit (dis_MenuAppendToggleItem);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 2 ) TCL_EXECERR ("wrong number of arguments")

    
    ViewerMenu* menu;
    if (currentMenuBar)            menu = currentMenuBar->FindMenu((const unsigned char *)argv[1]);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu ((const unsigned char *)argv[1]);

	if(menu==NULL) {
       TCL_EXECERR1 ("cannot find menu", argv[1])
	   return TCL_ERROR;
	}

	// going down menu tree according to the argument sequence.
    ViewerMenuItemList* item;
	for(i=0;i<argc-3;i++) {
	   // looking for the menu with this tagname in the menu bar or current popup menu.
		if(i==0)
	       item = menu->FindItem((const unsigned char *)argv[i+2]);
		else
	       item = item->FindItem((const unsigned char *)argv[i+2]);
	   // error if we can't find.
       if (item == NULL) {
		   TCL_EXECERR1 ("cannot find sub-menu", argv[i+2])
		   return TCL_ERROR;
	   }
    }
    item = item->FindItem ((const unsigned char *)argv[i+2]);
	if(item!=NULL) {
		item->Enable();
        return TCL_OK;
	} else {
       TCL_EXECERR1 ("cannot find menu item", argv[i+2])
	   return TCL_ERROR;
	}
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
int dis_MenuItemDisable (ClientData, Tcl_Interp* interp, int argc, char* argv[]) {
register i;

    gInit (dis_MenuAppendToggleItem);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 2 ) TCL_EXECERR ("wrong number of arguments")

    
    ViewerMenu* menu;
    if (currentMenuBar)            menu = currentMenuBar->FindMenu((const unsigned char *)argv[1]);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu ((const unsigned char *)argv[1]);

	if(menu==NULL) {
       TCL_EXECERR1 ("cannot find menu", argv[1])
	   return TCL_ERROR;
	}

	// going down menu tree according to the argument sequence.
    ViewerMenuItemList* item;
	for(i=0;i<argc-3;i++) {
	   // looking for the menu with this tagname in the menu bar or current popup menu.
		if(i==0)
	       item = menu->FindItem((const unsigned char *)argv[i+2]);
		else
	       item = item->FindItem((const unsigned char *)argv[i+2]);
	   // error if we can't find.
       if (item == NULL) {
		   TCL_EXECERR1 ("cannot find sub-menu", argv[i+2])
		   return TCL_ERROR;
	   }
    }
    item = item->FindItem ((const unsigned char *)argv[i+2]);
	if(item!=NULL) {
		item->Disable();
        return TCL_OK;
	} else {
       TCL_EXECERR1 ("cannot find menu item", argv[i+2])
	   return TCL_ERROR;
	}
}
//-----------------------------------------------------------------------------


int dis_MenuAppendSubmenu (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendSubmen);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 4)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    gString tagname;
    gString label;
    gString arguments;

    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    tagname.put_value_scribed (vcharScribeSystem (argv[2]));
    label.put_value_scribed (vcharScribeSystem (argv[3]));
    if (argc >= 5)
	arguments.put_value_scribed (vcharScribeSystem (argv[4]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);

    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])

    menu->AppendSubItem (label, tagname, arguments);

    Tcl_SetResult (interp, argv[2], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuAppendGroupSubmenu (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendGroupSubmenu);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 4)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    gString tagname;
    gString label;
    gString arguments;

    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));
    tagname.put_value_scribed (vcharScribeSystem (argv[2]));
    label.put_value_scribed (vcharScribeSystem (argv[3]));
    if (argc >= 5)
	arguments.put_value_scribed (vcharScribeSystem (argv[4]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);

    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])

    menu->AppendSubGroupItem (label, tagname, arguments);

    Tcl_SetResult (interp, argv[2], TCL_VOLATILE);
    return TCL_OK;
}

//-----------------------------------------------------------------------------

int dis_MenuAppendSeparator (ClientData,
			Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAppendSeparator);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 2)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);
    
    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])

    menu->AppendSepItem (NULL);

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_MenuAddMenuArgument (ClientData,
			Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAddMenuArgument);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 2)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));

    ViewerMenu* menu;
    if (currentMenuBar) menu = currentMenuBar->FindMenu (menu_tagname);
    else if (currentPopupMenuList) menu = currentPopupMenuList->FindMenu (menu_tagname);

    if (menu == NULL)
	TCL_EXECERR1 ("cannot find menu", argv[1])

    if (argc >= 3)
	menu->AddNewArgument((vchar*)argv[2]);

    return TCL_OK;
}


//-----------------------------------------------------------------------------

int dis_MenuAddItemArgument (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    gInit (dis_MenuAddItemArgument);

    if (currentMenuBar == NULL && currentPopupMenuList == NULL)
        TCL_EXECERR ("no menu bar")
	    
    if (argc < 2)
	TCL_EXECERR ("wrong number of arguments")

    gString menu_tagname;
    menu_tagname.put_value_scribed (vcharScribeSystem (argv[1]));

    ViewerMenuItemList* item;
    if (currentMenuBar) item = currentMenuBar->FindItem (menu_tagname);
    else if (currentPopupMenuList) item = currentPopupMenuList->FindItem (menu_tagname);

    if (item == NULL) {
	Tcl_AppendResult (interp, "cannot find item: ", argv[1], NULL);
	return TCL_OK;
    }

    if (argc >= 3)
	item->AddNewArgument((vchar*)argv[2]);

    return TCL_OK;
}

static const char *menu_args = "";
static const char *item_args = "";

 

static ViewerMenu *access_create_menu( const char *pszList )
{
  static int nTag = 0;
  char *pszMenu = NULL;
  ViewerMenu *menu = NULL;
  gString tagname;
  gString label;
  gString arguments = menu_args;
  if( currentMenuBar && strlen(pszList) )
  {
    char *pszTok = strdup( pszList );
    if( pszTok )
    {
      pszMenu = strtok( pszTok, "." );
      int fFirstToken = 1;
      while( ( menu || fFirstToken) && pszMenu )
      {
        if( fFirstToken )
        {
          menu = currentMenuBar->FindTopMenuByName( (vchar *)pszMenu );
          if( !menu ) //create menu if it doesn't exist
          {
            label = (vchar *)pszMenu;
            tagname.sprintf( (vchar *)"menutag%d", nTag++ );
            menu = currentMenuBar->AddNewMenu(tagname, label, arguments);
          }
        }
        else
        {
          vmenuItem *item = NULL;
          ViewerMenu *submenu = NULL;
          int nCount = menu->GetItemCount();
          for( int i=0; i<nCount && !submenu; i++ )
          {
            item = menu->GetItemAt(i);
            if( item && !vcharCompare( (vchar *)pszMenu, item->GetTitle() ) )
              submenu = ViewerMenu::CastDown(item->GetSubMenu()); 
          }
          if( !submenu )
          {
            label = (vchar *)pszMenu;
            tagname.sprintf( (vchar *)"menutag%d", nTag++ );
            submenu = menu->AppendSubItem(label, tagname, arguments);
          }
          menu = submenu;
        }
        fFirstToken = 0;
        pszMenu = strtok( NULL, "." );
      }
      free( pszTok );
    }
  }
  return menu;
}


int dis_AccessAddMenu (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
  int nRet = TCL_ERROR;
  if (argc < 2)
	  TCL_EXECERR ("wrong number of arguments")
  
  if( access_create_menu( argv[1] ) )
    nRet = TCL_OK;
  return nRet;
}

int dis_AccessAddItem (ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
  int nRet = TCL_ERROR;
  static int nTag = 0;
  if (argc < 4)
	  TCL_EXECERR ("wrong number of arguments")

  char *pszList = argv[1];
  char *pszLabel = argv[2];
  gString command = "reval DISmain \"dis_access_menu_eval";
  for( int ii = 3; ii < argc; ii++ )
  {
    command += " ";
    command += "{";
    command += argv[ii];
    command += "}";
  }
  command += "\"";
  ViewerMenu *menu = access_create_menu( pszList );
  if( menu )
  {
    //First see if the item already exists
    vmenuItem *item = NULL;
    int nCount = menu->GetItemCount();
    for( int i=0; i<nCount && !item; i++ )
    {
      item = menu->GetItemAt(i);
      if( !item || vcharCompare( (vchar *)pszLabel, item->GetTitle() ) )
        item = NULL; 
    }
    if( !item )
    {
      nRet = TCL_OK;
      gString tagname;
      gString label = (const vchar *)argv[2];
      gString arguments = item_args;
      tagname.sprintf( (vchar *)"itemtag%d", nTag++ );
      menu->AppendNewItem (label, tagname, command, arguments);
    }
  }
  return nRet;
}
