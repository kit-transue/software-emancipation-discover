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
// view_report.C
//------------------------------------------
// synopsis:
// 
// Reports app and view creation
//------------------------------------------

#include <msg.h>
#ifndef _representation_h
#include <representation.h>
#endif

#ifndef _Relational_h
#include <Relational.h>
#endif

#include <objOper.h>
#include <view_creation.h>
#include <symbolPtr.h>
#include <cmd.h>
#include <viewerShell.h>

appPtr get_create_app_by_name (const char *);

//
//// app creation report
//

int view_set_app (char *filename)
{
    Initialize(view_set_app);
    appPtr ap_h = get_create_app_by_name ((const char *)filename);
    if (ap_h)
	return 1;

    return -1;
}

extern "C" int view_app (char *filename)
{
    if (filename == 0)
	return -1;

    return view_set_app (filename);
}

void view_report_app (const char *filename)
{
    Initialize(view_report_app);
    DBG{
	msg("view_report_app: $1\n") << filename << eom;
    }
  
    if(cmd_begin()){
	cmd_gen("view_app(%s);", filename); 
    }
    cmd_end();
}

//
//// view creation report
//

int view_set_view (symbolPtr sym, repType tp, int force)
{
    viewPtr v = view_create (sym, tp, force);
    if (v) {
	view_create_flush();	
	return 1;
    }
    return -1;
}

extern "C" int view_view (appTreePtr nd, int type, int force)
{
    if (nd)
    {
	symbolPtr sym = nd;
	return view_set_view (sym, (repType)type, force);
    }
    
    return -1;
}

void view_report_view (symbolPtr sym, repType type, int force)
{
    Initialize(view_report_view);
    
    if(cmd_begin()){
	Relational *rel_obj = sym;
	if (rel_obj && is_appTree(rel_obj)){
	    appTreePtr nd = checked_cast(appTree,rel_obj);
	    DBG{
		msg("view_report_view: node $1, type $2, force $3\n") << (void *)nd << eoarg << (int)type << eoarg << (int)force << eom;
	    }
	    cmd_gen("view_view(%n, %d, %d);", nd, type, force);
	}
    }
    cmd_end();
}

/*
   START-LOG-------------------------------------------

   $Log: view_report.C  $
   Revision 1.3 2000/07/12 18:16:21EDT ktrans 
   merge from stream_message branch
// Revision 1.9  1993/11/19  03:58:09  boris
// Bug track: Emacs windows
// Emacs windows for ParaSET viewers
//
// Revision 1.8  1993/11/05  17:02:48  builder
// Port
//
// Revision 1.7  1993/11/02  21:23:00  andrea
// Bug track: 5081
// I added a check for helperShell when generating the cmd "view_view"
//
// Revision 1.6  1993/08/25  18:58:41  boris
// Fixed script play back the source view creation
//
// Revision 1.5  1993/06/24  16:41:42  bakshi
// added return fot c++3.0.1 port
//
// Revision 1.4  1993/06/06  19:20:47  aharlap
// do not force loading file if no recordind
//
// Revision 1.3  1993/05/26  16:21:17  sergey
// Put a trap in view_report_view for rel_obj = 0. Fix of bug #3492.
//
// Revision 1.2  1993/05/24  13:56:35  boris
// Added zero checking in view_view() and Ignore if zero appTree node.
//
// Revision 1.1  1993/05/18  18:33:11  boris
// Initial revision
//

   END-LOG---------------------------------------------
*/
