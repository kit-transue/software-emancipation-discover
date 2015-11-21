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
#ifndef _WIN32
#include <sys/param.h>
#endif

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

#include "cmd.h"
#include "machdep.h"
#include "msg.h"
#include "namedPath.h"
#include "objOper.h"
#include "objRawApp.h"
#include "objSet.h"
#include "proj.h"
#include "systemMessages.h"


#define MAXSZ 1024

init_relation(projpaths_of_realpath,MANY,NULL,realpath_of_projpath,1,NULL);
init_relation(paths_of_realpath,MANY,NULL,realpath_of_path,1,NULL);
init_relation(app_of_realpath,1,NULL,realpath_of_app,1,NULL);
init_relation(projnode_of_projpath,1,NULL,projpath_of_projnode,1,NULL);

// static variables
static genString path_from("");
static genString path_to("");
static objArr the_pathes;

// global variable
objSet converted_names;

// function pointer in path_names()
typedef void (*pathfun)(char const *head, char const *tail);

static void append(char const *head, char const *tail, char *buf)
{
   int len ;
   len = head ? strlen(head) : 0;
   if(len>0)
       strcpy(buf, head);
   if(!tail)
      return;
   buf[len] = '/';
   strcpy(buf+len+1,tail);
}

static void register_link (char const *head, char const *tail)
{
    Initialize(register_link);
    char buf[MAXSZ];
    buf[0] = 0;
    append(head, tail, buf);
    namedPath *pth = NULL;
    if (buf[0]) {
	pth = proj_path_get_item(buf);
	if (!pth) {
	    pth = new namedPath(buf, buf);
	    converted_names.insert(pth);
	}
	the_pathes.insert_last(pth);
    }
}

static void path_names(char const *head, char const *tail, pathfun fun, int call)
{
    Initialize(path_names);
    char const *cur_head = head;

    char buf[MAXSZ];
    char bbb[MAXSZ];
    int sz;
    char *nh;

    char const *new_tail;
    char new_head[MAXSZ];
    strcpy(new_head, head);

    nh = strrchr(new_head, '/');

    if(call)
	(*fun)(head, tail);

    if(nh){
	int nhs = nh - new_head;
	int hs = strlen(head);
	*nh  = '\0';
	new_tail = nh + 1;

	if(tail){
	    new_head[hs] = '/';
	    strcpy(new_head+hs+1, tail);
	}

	path_names(new_head, new_tail, fun, 0);
    }
    
    while(cur_head){
	sz = OSapi_readlink(cur_head, buf, MAXSZ);
	if(sz > 0){
	    buf[sz] = '\0';
	    if(buf[0] != '/'){  /* relative name */
		append(new_head, buf, bbb);
		path_names(bbb, tail, fun, 1);
		break;
	    } else {
		cur_head = buf;
		path_names(cur_head, tail, fun, 1);
	    }
	} else {
	    cur_head = 0;
	}
    }

}

static void proj_path_names(char const *path, pathfun fun)
{
    Initialize(proj_path_names);
    if(path[0] == '/')
	path_names(path, 0, fun, 0);
    else {
	char const *head =  getenv("PWD");
	char buf[MAXSZ];
	append(head, path, buf);
	path_names(buf, 0, fun, 0);
    }
}

// handles "///" , "/../" , "/./"
void proj_path_extract_periods (char const *buf, char *new_buf)
{
    if (!buf || !buf[0] || !new_buf)
	return;

    int failed = 0;

    int was_slash     = 0,
	was_slash_dot = 0;

    char const *from = buf;
    char *to = new_buf;
    char c;

    while (*from) {
	c = *from++;
	if (c == '/') {
	    if (was_slash)
		continue;

	    if (was_slash_dot) {
		was_slash_dot = 0;
		to-- ;
		was_slash = 1;
		continue;
	    }

	    was_slash = 1;
	} else if (c == '.') {
	    if (was_slash) {
		was_slash = 0;
		was_slash_dot = 1;
	    } else if (was_slash_dot) {
		was_slash_dot = 0;
		if (to > new_buf + 1) {
		    *(to-2) = 0;
		    to = strrchr(new_buf, '/');
		    if (to == 0)
			to = new_buf;
		} else
		    to = new_buf;

		continue;
	    }
	} else {
	    was_slash = 0;
	    was_slash_dot = 0;
	}

	*to++ = c;
    }

    if (was_slash_dot || was_slash)
	failed = 1;

    *to = 0;

    if (new_buf[0] == 0 || failed) 
	strcpy (new_buf, buf);
}

//creates the item if it does not exist
static namedPath *get_create_item (char const *path)
{
    Initialize(get_create_item);
    namedPath *item = proj_path_get_item(path);
    if (!item) {
	item = new namedPath(path,path);
	converted_names.insert(item);
    }
    the_pathes.insert_last(item);
    return item;
}

// return realpath namedPath *
namedPath *resolve_namedPath (char const *path, namedPath *proj_item)
{
    Initialize(resolve_namedPath);
    if (!path || !path[0])
	return NULL;

    namedPath *real_item = NULL;
    the_pathes.removeAll();

    proj_path_names(path, register_link);

    int sz = the_pathes.size();
    if (sz == 0)
	real_item = get_create_item(path);
    else
	real_item = checked_cast(namedPath, the_pathes[sz - 1]);

    if (!real_item)
	return NULL;
    
    // extracting relative path, redundant slashes and "/./"
    char const*buf = real_item->get_name();
    if (!buf || !buf[0])
	return NULL;

    char *new_buf = new char [strlen(buf) + 13];
    proj_path_extract_periods (buf, new_buf);

    if (new_buf[0] && strcmp(buf, new_buf) != 0) 
	real_item = get_create_item(new_buf);

    delete new_buf;
    
    real_item->path_type = real_item->path_type | namedPathReal;

    if (proj_item && real_item && real_item != proj_item) {
	if (proj_item->is_projpath())
	    put_relation(realpath_of_projpath,proj_item,real_item);
	else
	    put_relation(realpath_of_path,proj_item,real_item);
    }
	    

    char const*proj_name = NULL;
    if (proj_item) 
      proj_name = proj_item->get_value();

    for (int ii = 0 ; ii < the_pathes.size(); ii++) {
	namedPath *item = checked_cast(namedPath, the_pathes[ii]);
	if (item != real_item)
	    put_relation(realpath_of_path,item, real_item);

	if (proj_name && item != proj_item)
	    item->set_value(proj_name);
    }

    the_pathes.removeAll();
    return real_item;
}

init_relational(namedPath, namedString);

void namedPath::get_realpath (namedPath *& real_item)
{
    Initialize(namedPath::get_realpath);
    real_item = NULL;
   if (is_realpath())
       real_item = this;
    else if (is_projpath())
	real_item = checked_cast(namedPath,get_relation(realpath_of_projpath,this));
    else
	real_item = checked_cast(namedPath,get_relation(realpath_of_path,this));
}

namedPath *proj_path_get_item (char const*fname)
{
    Initialize(proj_path_get_item);
    namedPath *item = NULL;

    if (fname && fname[0])
	item = checked_cast(namedPath,obj_search_by_name (fname, converted_names));

    return item;
}

// used only in project/pdf/path.h.C
//Returns 0 on failure
int proj_path_convert (char const*path, genString& ret_path)
{
    Initialize(proj_path_convert);
    if (path == 0 || path[0] == 0 || path[0] == '.')
	return 0;
    
    if ( strcmp(path, path_from) == 0 ) {
	//   Previously converted, just return the correct answer 
	ret_path = path_to;
	return 1;
    }

    if (strcmp(path, (char *)path_to) == 0) {
	//   Whoever is asking already got the answer
	ret_path = path;
	return 1;
    }
    
    namedPath *ob = proj_path_get_item (path);
    if (ob) {
	ret_path = ob->get_value();
	if (ret_path.length()) {
	    path_from = path;
	    path_to = ret_path;
	    return 1;
	}
    }

    return 0;
}

// used only in project/pdf/path.h.C
int proj_path_report_conversion(char const*path, genString& proj_name)
{
    Initialize(proj_path_report_conversion);
    if (path == 0 || path[0] == 0 || path[0] == '.' || proj_name.length() == 0)
	return 0;

    // Check hache table
    namedPath *ob = proj_path_get_item (path);
    if (ob)
	ob->set_value(proj_name);
    else {
	ob = new namedPath(path, proj_name);
	converted_names.insert(ob);
    }

    if (strcmp((char const*)proj_name,path) != 0) {
	namedPath *ob = proj_path_get_item (proj_name);
	if (ob)
	    ob->set_value(proj_name);
	else {
	    ob = new namedPath(proj_name, proj_name);
	    converted_names.insert(ob);
	}
    }

    return 1;
}

void proj_path_report_app (app *ah, projNode *prj)
{
    Initialize(proj_path_report_app);

    if (!ah)
	return;

    if (get_relation(realpath_of_app,ah))
	  return;

    int tp = ah->get_type();
    char const *proj_name = (tp == App_RAW) ? ah->get_filename() : ah->get_phys_name();
    namedPath *proj_item = proj_path_get_item (proj_name);
    namedPath *real_item = NULL;
    if (proj_item)
	proj_item->get_realpath(real_item);
    else {
	proj_item = new namedPath(proj_name, proj_name);
	converted_names.insert(proj_item);
    }

    if (prj) {
	put_relation(projnode_of_projpath,proj_item,prj);
	proj_item->path_type = proj_item->path_type | namedPathProject;
	if (real_item && real_item != proj_item)
	    put_relation(projpaths_of_realpath,real_item,proj_item);
    }

    if (!real_item)
	real_item = resolve_namedPath (proj_name, proj_item);

    if (!real_item)
       return;

    app *old_h = checked_cast(app,get_relation(app_of_realpath,real_item));
    if (!old_h) {
	put_relation(app_of_realpath,real_item,ah);
	return;
    }

    if (old_h == ah)
       return;

    if (proj_item && ah->get_type() != App_RAW) {
	if (old_h->get_type() != App_RAW)
	    msg("WARN: Two project files are opened on the same name $1") << (char const*)real_item->get_name() << eom;
	else {
	    ste_interface_toggle_app (checked_cast(objRawApp,old_h),ah);
	    put_relation(app_of_realpath,real_item,ah);
	}
    }
}

