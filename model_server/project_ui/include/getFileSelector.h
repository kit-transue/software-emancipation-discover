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
#ifndef _getFileSelector_h
#define _getFileSelector_h

#include <proj.h>

enum getType
{
    GET_BY_VERSION = 1,
    GET_BY_DATE
};

class getFileSelector
{
  public:
    getFileSelector(genString &date, projModule *mod, getType type);
    ~getFileSelector();
    gtDialogTemplate* get_dlg() { return dlg; }

  private:
    gtForm* file_select_form;
    gtList* select_file_list;
    gtDialogTemplate* dlg;

    char **ver_list;
    int  ver_list_len;
    projModule *module;

    static void cancel_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    static void ok_CB(gtPushButton*, gtEventPtr, void* data, gtReason);
    void build_interface(getType type);
};
#endif

/*
$Log: getFileSelector.h  $
Revision 1.1 1994/02/16 10:45:09EST builder 
made from unix file
 * Revision 1.1  1994/02/16  13:38:08  so
 * Initial revision
 *
*/
