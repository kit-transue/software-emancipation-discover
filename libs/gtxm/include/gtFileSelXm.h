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
#ifndef _gtFileSelXm_h
#define _gtFileSelXm_h

// gtFileSelXm.h
//------------------------------------------
// synopsis:
// Class description for the gtFileSel object in OSF Motif
//------------------------------------------

#include "x11_intrinsic.h"
#include <Xm/Xm.h>
#include <gtBaseXm.h>
#include <gtFileSel.h>


class gtFileSelectorXm : public gtFileSelector
{
    gtBase 	*parent;
    const char 	*name;

  public:
    gtFileSelectorXm(gtBase *parent_arg, const char *name_arg);
    void construct();
    char *selected();
    gtBase *container();
    gtPushButton *help_button();
    gtPushButton *ok_button();
    gtPushButton *cancel_button();
    gtLabel *select_label();
    gtLabel *list_label();
    gtStringEditor *text();
    gtList *list();
    gtList *dir_list();
    void pattern (const char *);
    void title(const char *);
    char *title();
    void select_label(const char *);
    void dir_mode(int);
};


/*
   START-LOG-------------------------------------------

// $Log: gtFileSelXm.h  $
// Revision 1.1 1993/07/29 10:37:48EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * cleanup
 *
// Revision 1.6  92/08/06  18:12:08  rfermier
// hierarchy change, dir_mode
// 
// Revision 1.5  92/07/30  23:55:48  rfermier
// GT method name changes
// 
// Revision 1.4  92/07/22  13:39:19  rfermier
// Added more child access functions
// 
// Revision 1.3  92/07/17  13:39:25  rfermier
// added serious functionality
// 
// Revision 1.2  92/07/14  13:07:07  rfermier
// Eliminated gtHelp conflict
// 
// Revision 1.1  92/07/08  20:40:54  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtFileSelXm_h
