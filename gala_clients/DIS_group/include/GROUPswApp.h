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
// GROUPswApp.h - GROUP/sw Application defs.
//
//----------

#ifndef _GROUPswApp_h
#define _GROUPswApp_h

#ifndef vportINCLUDED
   #include <vport.h>
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif



class CreateGroup;

// Discover includes.
// Need to adjust some Galaxy defs.
#include <galaxy_undefs.h>
#ifndef _gRTListServer_h
   #include <gRTListServer.h>
#endif
#undef memcmp    // Discover has its own memcmp.
#ifndef _groupHdr_h
   #include <groupHdr.h>
#endif



class GROUPswApp {
    friend class CreateGroup;
public:
    GROUPswApp();
    ~GROUPswApp();
    static vbool isRunning();
    vbool Startup();
    vbool Shutdown();

    // Public Group Methods.
    vbool initGroupsList(int gId);
    vbool GrabSubsysSel(int, char **);
    vbool ShiftSel(int, int);
    vchar* getAskListString();
    vbool Ask(const unsigned int queryId, const symbolArr sources, symbolArr& results);
    vbool deleteGroups(const symbolArr groups);
    vbool saveGroups(const symbolArr groups);
    vbool printGroups(const symbolArr groups);
    vbool renameGroups(const symbolArr groups);
    vbool convertGroups(const symbolArr groups);
    vbool ConvertToGroup(const symbolArr&);
    vbool unionGroups(const symbolArr groups, symbolArr& members);
    vbool intersectGroups(const symbolArr groups, symbolArr& members);
    vbool subtractABGroups(const symbolArr groups, symbolArr& members);
    vbool subtractBAGroups(const symbolArr groups, symbolArr& members);

    vbool copyMembers(const symbolArr groups, symbolArr& members);
    vbool cutMembers(const symbolArr groups, symbolArr& members);
    vbool pasteMembers(const symbolArr groups, symbolArr& members);
    vbool assignMembers(const symbolArr groups, symbolArr& members);

    CreateGroup* getCreatePtr();

    // Group Relate methods.
    vbool setClientGroups(const symbolArr groups, symbolArr& clients);
    vbool setServerGroups(const symbolArr groups, symbolArr& servers);
    vbool setPeerGroups(const symbolArr groups, symbolArr& peers);
    vbool setUnrelatedGroups(const symbolArr groups, symbolArr& unrelated);

    // Member Relate methods.
    vbool setPublicMembers(const symbolArr groups, symbolArr& members);
    vbool setPrivateMembers(const symbolArr groups, symbolArr& members);
    vbool setVisibleMembers(const symbolArr groups, symbolArr& members);
    vbool setInvisibleMembers(const symbolArr groups, symbolArr& members);

    // Validation methods.
    vbool findViolations(const symbolArr groups, const char* fileName,const vchar* tclBuffer=NULL);
   
    // Semi-private typedefs.
    typedef vbool (*ASK_FUNCTION_TYPE)(const symbolArr, symbolArr&);
    typedef struct {
        char* name;
        ASK_FUNCTION_TYPE cmd;
    } ASK_STRUCT;

    // find RTL groups that could be imported.
    vbool findRTLGroups(genString& groupNames);
    vbool importRTLGroups();
    vbool importPackageSession(const char* filename);


private:
    // Private Discover Group operations.
    static vbool findAllGroups(symbolArr&);
    vbool importRTL(char * filename);
    vbool scanRTLDir(char * filename, genString& groupNames);
    vbool importRTLDir(char * filename);
    vbool extractRTLName(char *filename, genString &name);
    int import_group_from_file(groupHdr * a_group, char * filename, symbolArr & sel);
    void process_entry(groupHdr * a_group, symbolArr & sel, char * line);
    ddKind ddKind_internal_val(char *obj_kind);
    vbool findGroupMembers(const groupHdrPtr group);
    vbool createGroup(const vchar*, const vbool, const vbool);
    vbool fetchGroups(objArr& groups);
    vbool unionGroupMembers(const groupHdrPtr groupA, const groupHdrPtr groupB,
                            symbolArr& members);
    vbool intersectGroupMembers(const groupHdrPtr groupA, const groupHdrPtr groupB,
                            symbolArr& members);
    vbool subtractGroupMembers(const groupHdrPtr groupA, const groupHdrPtr groupB,
                            symbolArr& members);

    // See if there are any nested projects.
    vbool checkForNestedProjects(const symbolArr& members);

    // Ask the user how to handle projects as members.
    vbool getProjectCopyType(const char* title);

    // Group Ask functions.
    static vbool showClientGroups(const symbolArr sources, symbolArr& results);
    static vbool showServerGroups(const symbolArr sources, symbolArr& results);
    static vbool showPeerGroups(const symbolArr sources, symbolArr& results);
    static vbool showUnrelatedGroups(const symbolArr sources, symbolArr& results);

    // Member ask functions.
    static vbool showLocalMembers(const symbolArr sources, symbolArr& results);
    static vbool showTransitiveMembers(const symbolArr sources, symbolArr& results);
    static vbool showPublicMembers(const symbolArr sources, symbolArr& results);
    static vbool showPrivateMembers(const symbolArr sources, symbolArr& results);

    // Private data elements.
    static vbool GROUPswIsRunning;

    static ASK_STRUCT AskList[];
    static const int AskListSize;
    static vstr* AskListString;

    symbolArr MemberClipboard;    // Used for cut/copy/paste operations.

    CreateGroup* Create;    // Group creation class.
};

#endif    // _GROUPswApp_h




//----------     end of GROUPswApp.h     ----------//
