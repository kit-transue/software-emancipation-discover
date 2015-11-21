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
#ifndef _gtPushButtonXm_h
#define _gtPushButtonXm_h

// gtPushButtonXm.h
//------------------------------------------
// synopsis:
// Class description for the gtLabel object in OSF Motif
//------------------------------------------

#include <gtPushButton.h>
#include <gtCallbackMgrXm.h>

class gtBitmapXm;

class gtPushButtonXm : public gtPushButton, public gtCallbackMgrXm
{
  public:
    gtPushButtonXm(gtBase*, const char* name, const char* label,
		   gtPushCB callback, void* client_data);
    gtPushButtonXm(gtBase*, const char* name, gtBitmapXm* bitmap,
		   gtPushCB callback, void* client_data);
    ~gtPushButtonXm();

    gtBase* container();

    void  set_callback(gtPushCB callback, void* client_data);

    void  shadow_thickness(int);
    void  title(const char*);
    char* title();
    static const char* saved_help_context;
};


/*
   START-LOG-------------------------------------------

// $Log: gtPushButtonXm.h  $
// Revision 1.1 1993/11/18 09:48:14EST builder 
// made from unix file
 * Revision 1.2.1.5  1993/11/17  19:48:48  jon
 * Bug track: 0
 * added constructor for push buttons with gtBitmaps
 * added member to return help context
 *
 * Revision 1.2.1.4  1993/04/28  12:26:36  jon
 * Made saved_help_context a static data member of gtPushButtonXm
 *
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.2  1992/10/09  18:43:58  jon
 * RCS History Marker Fixup
 *
 * Revision 1.2.1.1  92/10/07  22:10:55  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  22:10:54  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:27:42  smit
 * Initial revision
 * 
// Revision 1.6  92/09/13  15:16:26  glenn
// Remove unnecessary data members.
// 
// Revision 1.5  92/08/19  11:34:45  builder
// fixed bug #966
// 
// Revision 1.4  92/08/13  12:10:25  rfermier
// added title()
// 
// Revision 1.3  92/07/30  23:55:58  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/20  13:28:07  rfermier
// Added title function
// 
// Revision 1.1  92/07/08  20:41:07  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtPushButtonXm_h
