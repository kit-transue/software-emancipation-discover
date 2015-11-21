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
/**********
 *
 * getresource.h - Global Fuction defs.
 *
 **********/

#ifndef _getresource_h
#define _getresource_h

#ifndef vportINCLUDED
#include <vport.h>
#endif

#ifndef vapplicationINCLUDED
#include vapplicationHEADER
#endif

#ifndef vresourceINCLUDED
#include vresourceHEADER
#endif


// Resource utility functions.
int gala_GetResource(const vname*, vresource*);
int gala_GetResource(vresource, const vname*, vresource*);
vname* getResourceTag(const vchar* name);
vstr* getResourceFolders(const vchar* name);

// Preference utility functions.
vbool getPreference(const char* key, int& value);
vbool getPreference(const char* key, vstr** value);
vbool putPreference(const char* key, const int value);
vbool putPreference(const char* key, const char* value);

// Utilities to change the default resource and preference files.
vbool setResourceFile(const char* fileName);
vbool setPreferences(const char* fileName);
vbool deletePreferences(void);



//-----------------------------------------------------------------------------
//
// inline getResource functions.
//    All return vTRUE and fill in a resource if name is found.
//    All return vFALSE if name is not found.
//    All use gala_GetResource.
//    If dictionary isn't given, checks Preferences then Resources.

inline int getResource(const char* name, vresource * dest)
{
    return gala_GetResource(vnameInternGlobalScribed(
                            vcharScribeSystem(name)), dest);
}

inline int getResource(const vchar* name, vresource * dest)
{
    return gala_GetResource(vnameInternGlobal(name), dest);
}

inline int getResource(vscribe* name, vresource * dest)
{
    return gala_GetResource(vnameInternGlobalScribed(name), dest);
}

inline int getResource(vresource dres, const char* name,
                       vresource * dest)
{
    return gala_GetResource(dres, vnameInternGlobalScribed(
                            vcharScribeSystem(name)), dest);
}

inline int getResource(vresource dres, const vchar* name,
                       vresource * dest)
{
    return gala_GetResource(dres, vnameInternGlobal(name), dest);
}

inline int getResource(vresource dres, vscribe* name,
                       vresource * dest)
{
    return gala_GetResource(dres, vnameInternGlobalScribed(name),
                            dest);
}


#endif
