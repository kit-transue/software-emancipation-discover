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
#include <msg.h>
#include <projHeader.h>
#include <genString.h>
#include <_Xref.h>
#include <genError.h>
#include <feedback.h>
#include <options.h>
#include <mpb.h>

extern bool proj_is_writable(projNode *proj, genString& fn);

#ifndef NEW_UI
projNode *popup_home_proj_selector();
#endif

// 1. set home project
// 2. display a message about the selected home project (not for put process)
//
// proj: is the selected home projnode

static void define_home_proj(projNode *proj)
{
    Initialize(define_home_proj);

    projNode::set_home_proj (proj);
    msg("Home project: $1", normal_sev) << proj->get_name() << eom;
}              
 

// check if -x is specified and if the project name after -x is valid
// if no -x is specified, popup the dialog box for the home selection

extern "C" int home_proj_selector()
{
    Initialize(home_proj_selector());
    projNode* proj = NULL;

    // get -x option specification

    char* projname = get_main_option("-x");
    if(!projname) projname = get_main_option("-home");

    if (projname) {
        genString s;
        if (*projname != '/') {
            s.printf("/%s",projname);
            projname = s;
        }

        proj = projNode::find_project(projname);
        if (!proj) {
            msg("Error: Selected home project $1 is not a valid project") << projname << eom;
            return 0;
        }

        if (proj->get_xref(1) && !proj->get_xref()->is_unreal_file())
            ;
        else {
            Xref *proj_xref = proj->get_xref();
            /* check for parents pmod */

            if (proj_xref && !proj_xref->is_unreal_file() && !proj->get_xref(1)){
                projNode* par = proj_xref->get_projNode();
                if (par)
                    msg("project $1 already has pmod", error_sev)
			<< par->get_name() << eom;
                msg("parent of project - $1 already has pmod", error_sev)
			<< projname << eom;
                return 0;
            }
        }

        genString fn;
        if ( !proj_is_writable(proj, fn)) {
            msg("Selected home project $1 is not writable", error_sev)
		<< projname << eom;
            if (fn.length())
                msg("   --> $1", normal_sev) << (char*)fn << eom;
            return 0;
        }
    }

    // if the -x option was specified and the project specified is valid

    if (proj) {
        // check whether the project has subprojects with pmod files
	mpb_incr(4);
        if (Xref::test_subproj_pmod(proj)==-1) {
            msg("Selected project $1 is not allowed to have subprojects with pmod files", error_sev) << proj->get_ln() << eom;
            return 0;
        }      
	mpb_incr(5);
    }

    else {
#ifndef NEW_UI
        proj = popup_home_proj_selector();
#endif
        if (!proj) return 0;
    }
                
    define_home_proj(proj);
               
    return 1;  
}


