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
#ifndef _genError_h
#include <genError.h>
#endif
#ifndef _genString_h
#include <genString.h>
#endif
#include <messages.h>
#include <xrefSymbol.h>
#include <_Xref.h>
#ifndef _proj_h
#include <proj.h>
#endif
#include <projectBrowser.h>  /* Automatically substituted */

#ifndef SUBPROJ_HOME_ALLOWED
#define SUBPROJ_HOME_ALLOWED
#endif

void setup_home_from_selection(symbolArr& projects)
{
    Initialize(setup_home_from_selection);
    
    if(projects.size() > 1)
    {
	msg("ERROR: You must select exactly one project.") << eom;
    }
    else if(projects.size() == 1)
    {
        projNodePtr proj = sym_to_proj(projects[0]);
#ifdef  SUBPROJ_HOME_ALLOWED
        //  if this project does not have a pmod,
        //  find the nearest parent project for which there exists a pmod
        if (proj)
        {
            Xref *xr = proj->get_xref();
            projNode *temppr;
            while (!xr && (temppr=proj->find_parent()))
            {
                proj = temppr;
                xr = proj->get_xref();
            }
        }
#else  
        proj = proj->root_project();
#endif
        if(proj)
        {
            projNodePtr old_proj = projNode::get_home_proj();
            if (!proj->is_writable())
            {
		msg("ERROR: Error, project $1 is read only") << proj->get_name() << eom;
            }
            else if (Xref::test_subproj_pmod(proj)==-1)
            {
		msg("ERROR: Error, project $1 has subprojects\ncontaining .pmod files") << proj->get_name() << eom;
            }
            else
            {
                projNode::set_home_proj(proj);
                // here we check if it's writable, and restore the old_proj
                Xref *tmp_xref = proj->get_xref();
                bool wrt_flag = 0;
                if ( tmp_xref )
                     wrt_flag = tmp_xref->is_writable();
                if (wrt_flag == 0)
                {
                    projNode::set_home_proj(old_proj);
		    msg("ERROR: Error, project $1 is read only") << proj->get_name() << eom;
                }
            }
        }
        else
	    msg("ERROR: Error getting the selected project.") << eom;
    }
    else
    {
	msg("ERROR: No project selected to be home.") << eom;
    }
}
