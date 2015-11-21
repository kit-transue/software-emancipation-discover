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
//#include <stdio.h>

#include <cLibraryFunctions.h>
#include <msg.h>
#include <objOper.h>
#include <objArr.h>
#include <view_creation.h>
#include <vpopen.h>
#include <objCollection.h>

void proj_clean()
{
    Initialize(proj_clean);

    objArr list = *(app::get_app_list());

    Obj *el;
    ForEach (el, list)
    {
        appPtr app_head = checked_cast(app,el);
	int tp = app_head->get_type();
	if (tp == App_SMT)
        {
            if (!has_view(app_head)
             && !app_head->needs_to_be_saved())
            {
                DBG
                {
                    msg("REMOVE SMT = $1") << app_head->get_name() << eom;
                }

                obj_unload(app_head);
            }
        }
        else if (tp == App_STE)
        {
            if (!has_view(app_head)
             && !app_head->needs_to_be_saved()
             && strcmp(app_head->get_name(),"ste-clip-board")
             && strcmp(app_head->get_name(),"help_context_links.txt"))
            {
                DBG
                {
                    msg("REMOVE STE = $1") << app_head->get_name() << eom;
                }

                obj_unload(app_head);
            }
        }
    }
}
