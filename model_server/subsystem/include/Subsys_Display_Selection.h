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
#ifndef _Subsys_Display_Selection_h
#define _Subsys_Display_Selection_h

// Subsys_Display_Selection.h
//------------------------------------------
// synopsis:
// Subsys_Display_Selection interface in libGT
//
// description:
// ...
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

#ifndef _prototypes_h
#include "prototypes.h"
#endif

#ifndef _gtDltTemplate_h      
#include <gtDlgTemplate.h>
#endif
      
#ifndef _gtToggleBox_h
#include <gtToggleBox.h>
#endif      

BEGIN_C_DECL
void popup_Subsys_Display_Selection PROTO((void *, void *));
END_C_DECL

class Subsys_Display_Selection {
public:
   gtDialogTemplate	*shell;
   Subsys_Display_Selection();
   void init_fields(void *, void *);
   void do_it();
   static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
   static void Apply_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
   static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);

private:
   void	*viewh, *p_node;       
   gtToggleBox* group1;
};

typedef Subsys_Display_Selection* Subsys_Display_Selection_Ptr;

#endif // _Add_Relation_h

/*
    START-LOG-------------------------------

    $Log: Subsys_Display_Selection.h  $
    Revision 1.3 1995/08/26 09:32:51EDT wmm 
    New group/subsystem implementation
 * Revision 1.2.1.2  1992/10/09  19:56:20  kws
 * Fix comments
 *

    END-LOG---------------------------------
*/

