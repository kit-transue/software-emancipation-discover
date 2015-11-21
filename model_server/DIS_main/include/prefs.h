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
//----------
//
// prefs.h - PREFSsw Application defs.
//
//----------
 
#ifndef _prefs_h
#define _prefs_h
 
#ifndef vportINCLUDED
   #include <vport.h>
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif
#ifndef vapplicationINCLUDED
#include vapplicationHEADER 
#endif
 
// Discover includes.
// Need to adjust some Galaxy defs.
#undef printf    // Galaxy has its own printf macro.
 
#define INT    -200
#define STRING -201
 
void GetFolder(int viewId, vchar* listName); 
void GetValues(char *foldengname, int viewId, vchar *listName);
void GetEname(char *selectedname, char *ename);
void GetKeyname(char *foldengname,char *engname, char *keyname);
void GetSelectedValue(char *selectedname, char *retvalue);
int GetType(char *foldengname, char *engname);
char *ReadFoldername(char *engname);
vbool isEnglishName(const vchar *name, char *inname);
void setdisplayvalue(char *Ename, char *value, char *result);


#endif    // _prefs_h
 
 
//----------     end of prefs.h     ----------//




