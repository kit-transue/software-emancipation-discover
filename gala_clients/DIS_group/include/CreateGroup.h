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
// CreateGroup.h - GROUPswApp CreateGroup class definitions.
//    This class handles Group creation requests from the gala UI.
//
//----------


#ifndef _CreateGroup_h
#define _CreateGroup_h



#ifndef vportINCLUDED
   #include <vport.h>
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif

// Discover includes.
// Need to adjust some Galaxy defs.
#include "galaxy_undefs.h"
#include <groupHdr.h>
#include <gRTListServer.h>


class GROUPswApp;


class CreateGroup {
public:
    typedef enum {
        COPY_NONE=0,
        COPY_ALL_TO_FLAT,
        COPY_ALL_TO_ALL
    } GROUP_COPY_TYPE_ENUM;

    CreateGroup(int groupsId);
    ~CreateGroup();
    vchar* getNextGroupName();
    vbool setNextGroupName(const vchar*);
    groupHdrPtr createNextGroup(const int gType, const symbolArr mId,
                                const int mCopy);

    vbool findImportGroups(int Id);
    vbool importRTLGroups(int Id);
    vbool importPackageSession(const vchar*);

private:
    vbool updateNextGroupName();   // Increment name counter and update Next Name.
    vbool findRTLGroups(symbolArr&);
    vbool importRTLGroup(char* fileName);

 
    GROUPswApp* MyParent;    // Parent GROUP/sw application.
    vstr* NextGroupNameBase; // Base name, e.g. 'GROUP-'
    int   NextGroupCounter;  // Count or index of next group.
    vstr* NextGroupName;     // Base with Count tacked onto the end.
};

#endif    // _CreateGroup_h





//----------     end of CreateGroup.h     ----------//
