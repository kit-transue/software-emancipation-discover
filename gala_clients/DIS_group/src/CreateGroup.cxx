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
// CreateGroup.C - GROUPswApp CreateGroup class definition.
//     This class handles group creation activities.
//
//----------

// Galaxy Includes.
#include <vport.h>

// Client/Server communication includes.
#include "../../../clients/DIS_ui/interface.h"
extern Application* DISui_app;

// GROUP/sw specific includes.
#include "../include/GROUPswApp.h"    // Needed for groupCmd methods.
#include "../include/CreateGroup.h"
 
#include "messages.h"

//----------     CreateGroup class public members     ----------//

// Constructor.
CreateGroup::CreateGroup(int numGroups)
{
    NextGroupNameBase = vstrCloneScribed(vcharScribeLiteral("GROUP-") );
    NextGroupCounter = numGroups + 1; // Start at 001, not 000.
    NextGroupName = vstrCloneScribed(vcharScribeFormat((vchar*)"%s%03d",
                                     NextGroupNameBase, NextGroupCounter) );
}

//Destructor
CreateGroup::~CreateGroup()
{
    if (NextGroupNameBase)
        vstrDestroy(NextGroupNameBase);
    if (NextGroupName)
        vstrDestroy(NextGroupName);
}

// Get the name of the next group to be created.
vchar* CreateGroup::getNextGroupName()
{
    return (NextGroupName);
}

// Set the name of the next group to be created.
vbool CreateGroup::setNextGroupName(const vchar* name)
{
    if (NextGroupName)
        vstrDestroy(NextGroupName);
    NextGroupName = vstrClone(name);
    return (vTRUE);
}

// Create the next group.
groupHdrPtr CreateGroup::createNextGroup(const int groupType,
                                         const symbolArr members,
                                         const int memberCopy)
{
    int isSubsystem = 0;
    int isScratch = 0;

    // Set subsystem and saved flags from groupType.
    if (groupType == 2)
	isSubsystem = 1;
    else if (groupType == 0)
	isScratch = 1;

    // Convert the copy type into a nested_projects_action.
    groupHdr::nested_projects_action action = groupHdr::MIRROR_HIERARCHY;

    groupHdrPtr nextGroup = groupHdr::create((char*)NextGroupName,
                          isSubsystem, isScratch);

    if (nextGroup) {    // Make sure that we got a group.
        // Set the group's members.
        if (members.size() > 0) {
            nextGroup->add_members(members, action, groupHdr::DEFAULT_ACC);
        }

        // set up for the name of the next group to create.
        updateNextGroupName();
    }

    return (nextGroup);
}

// Find the Import Groups.
vbool CreateGroup::findImportGroups(int rtlId)
{
    RTListServer* importsRTL = RTListServer::find(rtlId);
    importsRTL->clear();
    symbolArr importGroups;
    findRTLGroups(importGroups);
    importsRTL->insert(importGroups);    // Display the Import groups.

    return (vTRUE);
}

// Import the selected Import Groups.
vbool CreateGroup::importRTLGroups(int importsId)
{
    Initialize(CreateGroup::selectImportGroupAt);

    RTListServer* importsRTL = RTListServer::find(importsId);

    if (importsRTL) {
        // Get the array of selected groups.
        symbolArr groupSyms;
        importsRTL->getSelArr(groupSyms);
        // Save and clear the Known-Groups while importing.
        symbolArr newGroupSyms(importsRTL->getArr() );
        importsRTL->clear();
        symbolPtr groupSym;
        ForEachS(groupSym, groupSyms) {
	    genString phys_name;
	    projModulePtr pm = checked_cast(projModule, groupSym);
	    if (pm) {
		pm->get_phys_filename(phys_name);
		importRTLGroup((char *)phys_name);
            }
            // Remove the entry from the list.
            newGroupSyms.remove(groupSym);
        }
        // Add the group not imported back into the Imports RTL.
        // Note - This may remove items which we were unable to import.
        importsRTL->insert(newGroupSyms);
    }

    return (vTRUE);
}

// Import the given Package Session.
vbool CreateGroup::importPackageSession(const vchar* sessionName)
{
    if (sessionName && (vcharLength(sessionName) > 0) ) {
        dis_message(NULL, MSG_ERROR, "Cannot Import Session '%s'. Not Implemented Yet!\n",
                    sessionName);
    } else {
        dis_message(NULL, MSG_STDERR, "Null Packe session name given!\n");
    }

    return (vTRUE);
}

//----------     CreateGroup class private members     ----------//

// Increment group counter and update next group name.
vbool CreateGroup::updateNextGroupName()
{
    // Increment the group counter.
    ++NextGroupCounter;

    // Rebuild the NextName string.
    vstrDestroy(NextGroupName);
    NextGroupName = vstrCloneScribed(vcharScribeFormat((vchar*)"%s%03d",
                                     NextGroupNameBase, NextGroupCounter) );

    return(vTRUE);
}

// Find old-style rtl groups that can be imported.
vbool CreateGroup::findRTLGroups(symbolArr& importGroups)
{
    Initialize(CreateGroup::findRTLGroups);

    // First, check for rtl nodes within the current project.
    projList::domain_reset();

    genString pathList = "/local";
    projList::search_list->parse_names_init(pathList);
    projList::search_list->domain_string();

    projNode *pr;
    for(int ii = 0; pr = projList::search_list->get_scan(ii); ++ii) {
        symbolArr modules;
        pr = pr->root_project();
        if (pr) {
	    symbolPtr el;
	    pr->search_modules_by_name(".rtl", modules);
	    ForEachS(el, modules) {
	        projModulePtr pm = checked_cast(projModule, el);
	        if (pm) {
	        }
	    }
        }
    }

    return(vTRUE);
}

// Import old-style rtl group.
vbool CreateGroup::importRTLGroup(char* fileName)
{
    Initialize(CreateGroup::importRTLGroup);


    // Get the RTL name from the file name.
    // Create the group from the RTL name.
    // Import the group from the file.
    // Add the group to the Known Groups RTL.

    return(vTRUE);
}

//----------     end of CreateGroup.C     ----------//
