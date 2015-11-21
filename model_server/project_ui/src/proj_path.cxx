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
#include <msg.h>
#include <cmd.h>
#include <objOper.h>
#include <objSet.h>
#include <proj.h>
#include <systemMessages.h>
#include <machdep.h>
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <objRawApp.h>
#include <top_widgets.h>
#include <namedPath.h>

#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cstdio>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

app *proj_path_find_app (const char *fname, int no_raw)
{
    Initialize(proj_path_find_app);
    if (fname == 0 || fname[0] == 0)
	return 0;

    appPtr ah = NULL;
    namedPath *real_item = NULL, *item = proj_path_get_item(fname);

    if (item) {
	item->get_realpath (real_item);
	if (real_item)
	    ah = checked_cast(app,get_relation(app_of_realpath,real_item));
    }

    if (ah && ah->get_type() == App_RAW && no_raw)
	ah = NULL;

    return ah;
}

static namedPath *proj_path_find_realpath (appPtr ah)
{
    Initialize(proj_path_find_realpath);
    namedPath *real_item = NULL;
    if ( !ah || (real_item = checked_cast(namedPath,get_relation(realpath_of_app,ah))) )
	return real_item;

    char *nm = (ah->get_type() == App_RAW) ? ah->get_filename() : ah->get_phys_name();
    if (nm && nm[0]) {
	namedPath * own_item = proj_path_get_item(nm);
	if (own_item)
	    own_item->get_realpath(real_item);
    }
    return real_item;
}

// for RAW files; returned existing on realpath app *
app *proj_path_convert_app (genString & path, genString & real_path, projNode *prj)
{
    Initialize(proj_path_convert_app);
    if (path.str() == 0 || (path.str())[0] == 0)
	return NULL;

    namedPath *proj_item = NULL;
    if (prj) {
	if ( !(proj_item = proj_path_get_item(path)) ) {
	    proj_item = new namedPath(path, path, namedPathProject);
	    converted_names.insert(proj_item);
	}
    }

    app *ah = proj_path_find_app (path);
    if (ah) {
	namedPath *real_item = checked_cast(namedPath,get_relation(realpath_of_app, ah));
	if (real_item)
	    real_item->get_realpath(real_path);
	else {
	    real_item = resolve_namedPath (path, proj_item);
	    if (real_item)
		real_item->get_realpath(real_path);
	    put_relation(realpath_of_app,ah,real_item);
	}
	return ah;
    }

    namedPath *real_item = resolve_namedPath (path, proj_item);
    if (!real_item)
	return NULL;

    real_item->get_realpath(real_path);
    ah = checked_cast(app,get_relation(app_of_realpath,real_item));
    if (ah) {
	if (proj_item) {
	    if (ah->get_type() != App_RAW)
		msg("WARN: Two project files are opened on the same name $1") << (char *)path << eom;
	    else {
		appTreePtr rt = checked_cast(appTree,ah->get_root());
		obj_insert (ah, CHANGE_NAME, (appTree *)NULL, (appTree *)NULL, (void *)((char *)path));
	    }
	}
    }		

    return ah;
}

// returns 0 on failure
int proj_path_toggle_app (appPtr ah, int& poped)
{
    Initialize(proj_path_toggle_app);
    if (!ah)
	return 0;

    namedPath *real_item = proj_path_find_realpath (ah);
    if (!real_item)
	return 0;

    int tp = ah->get_type();
    app *old = checked_cast(app,get_relation(app_of_realpath,real_item));
    
    if (old == ah)
	return 1;
    else if (!old) {
	put_relation(app_of_realpath,real_item,ah);
	return 1;
    }

    if (old->get_type() == App_RAW && tp != App_RAW) {
	put_relation(app_of_realpath,real_item,ah); //cardinality 1 removes old object
	if (poped == 0)
	    push_busy_cursor();
	
	poped++;
	ste_interface_toggle_app (checked_cast(objRawApp,old), ah);
	return 1;
    }

    msg("WARN: Two files are open on the same realpath $1") << real_item->get_value() << eom;
    return 1;
}

