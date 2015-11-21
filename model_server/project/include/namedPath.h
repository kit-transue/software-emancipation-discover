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

void proj_path_report_app(app *, projNode * = NULL);
app *proj_path_convert_app(genString &, genString &, projNode * = NULL);
app *proj_path_find_app (const char *fname, int no_raw = 0);
void ste_interface_toggle_app (objRawApp *, app*);

define_relation(projpaths_of_realpath,realpath_of_projpath);
define_relation(paths_of_realpath,realpath_of_path);
define_relation(app_of_realpath, realpath_of_app);
define_relation(projnode_of_projpath,projpath_of_projnode);

extern objSet converted_names;

enum namedPathTypes {
    namedPathRegular = 0,
    namedPathProject = 1,
    namedPathReal    = 2
};

class namedPath : public namedString
{
  public:
    int path_type;
    define_relational(namedPath, namedString);
    namedPath(const char *path, const char *proj_path)
    : namedString(path, proj_path), path_type(namedPathRegular) {}

    //    namedPath(const char *path, const char *proj_path, int tp);
        //    : namedString(path, proj_path), path_type(tp) {}

    //    bool is_regular ()  { return (path_type == 0); }
    bool is_projpath () { return ((path_type & namedPathProject) > 0); }
    bool is_realpath()  { return ((path_type & namedPathReal) > 0); }

    //    void get_realpath (genString&);
    void get_realpath (namedPath *&);
    //    void get_projpath (genString&, projNode *prj = NULL);
    //    void get_projpath (namedPath *&, projNode *prj = NULL);
    //    void print(ostream& os, int level=0) const; 
};
generate_descriptor(namedPath, namedString);

namedPath *proj_path_get_item (const char *);
namedPath *resolve_namedPath (char *, namedPath *);

