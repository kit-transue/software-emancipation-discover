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
 * GROUPswApp.C - GROUP/sw Application class def.
 *
 **********/


// Galaxy Includes.
#include <vport.h>

// Client/Server communication includes.
#include <../../../clients/DIS_ui/interface.h>
extern Application* DISui_app;

// Local GROUP includes.
#include "../include/GROUPswApp.h"
#include "../include/CreateGroup.h"

// Discover includes.
#include <galaxy_undefs.h>  // Clean up galaxy stuff.
#include <genError.h>
#include <xrefSymbol.h>
#include <db.h>
#include <projList.h>
#include <Question.h>
#include <RTL_externs.h>
#include <messages.h>
#include <iQuestion.h>

#include <autosubsys.h>

#include "../include/GroupApp.h"



//--------------     class GROUPswApp static members   ---------------------

// Private status flag
vbool GROUPswApp::GROUPswIsRunning = vFALSE;

//Private Ask List info.
GROUPswApp::ASK_STRUCT GROUPswApp::AskList[] = {
	{ "Show Clients",		&GROUPswApp::showClientGroups},
	{ "Show Servers",		&GROUPswApp::showServerGroups},		
        { "Show Peers",                 &GROUPswApp::showPeerGroups},
        { "Show Unrelated",             &GROUPswApp::showUnrelatedGroups},
	{ "Show Local Members",		&GROUPswApp::showLocalMembers},
//	{ "Show Transitive Members",	&GROUPswApp::showTransitiveMembers},
	{ "Show Public Members",	&GROUPswApp::showPublicMembers},
	{ "Show Private Members",	&GROUPswApp::showPrivateMembers},
};
const int GROUPswApp::AskListSize = sizeof(AskList) / sizeof(GROUPswApp::ASK_STRUCT);

vstr* GROUPswApp::AskListString = NULL;

// Returns vTRUE iff GROUP/sw application already running.
vbool GROUPswApp::isRunning()
{
    Initialize(GROUPswApp::isRunning);

    return (GROUPswIsRunning);
}



//----------     class GROUPswApp Public Members     ----------


// class constructor.
GROUPswApp::GROUPswApp()
{
    Initialize(GROUPswApp::GROUPswApp);

    Create = NULL;
}



// class destructor.
GROUPswApp::~GROUPswApp()
{
    Initialize(GROUPswApp::~GROUPswApp);

    if (Create) {
        delete Create;
    }
}



// Start up the GROUP/sw application services.
vbool GROUPswApp::Startup()
{
    Initialize(GROUPswApp::Startup);

    GROUPswIsRunning = vTRUE;
 
    // Get number of groups for Create class init.
    symbolArr allGroups;
    findAllGroups(allGroups);
    int numGroups = allGroups.size();
    
    // CreateGroups sub-class.
    if (Create) delete Create;
    Create = new CreateGroup(numGroups);

    return(vTRUE);
}



// Shutdown the GROUP/sw application services.
vbool GROUPswApp::Shutdown()
{
    Initialize(GROUPswApp::Shutdown);
    return (vTRUE);
}




//----------     Public Group methods.     ----------//



// Init the Groups RTL
vbool GROUPswApp::initGroupsList(int groupsId)
{
    Initialize(GROUPswApp::initGroupsList);

    if (groupsId > 0) {
        RTListServer* groupsRTL = RTListServer::find(groupsId);
		symbolArr sel(0);
		groupsRTL->getSelArr(sel);
        groupsRTL->clear();
        symbolArr allGroups;
        findAllGroups(allGroups);
        groupsRTL->insert(allGroups);    // Get groups from model.
		groupsRTL->select(sel);
    }

    

    return (vTRUE);
}

// Fill in the specified RTL w/groups:

vbool GROUPswApp::GrabSubsysSel(int argc, char **argv)
{
  Initialize(GROUPswApp::GrabSubsysSel);

  vbool ret = vFALSE;

  if (argc >= 2) {

    symbolArr arr(0);
    RTListServer *res_rtl = NULL;

    for (int i = 0; i < argc; i++) {
      int id = OSapi_atoi(argv[i]);
      if (id > 0) {
        RTListServer *rtl = RTListServer::find(id);
	if (i == 0) {
	  if (rtl)	res_rtl = rtl;
	  else		break;
	}
	else {
	  if (rtl) {
            symbolArr sel;
	    symbolPtr sym;

	    rtl->getSelArr(sel);

	    ForEachS (sym, sel) {
              if (sym.get_kind() == DD_SUBSYSTEM)
	        arr.insert_last(sym);
            }
          }
        }
      }
    }

    if (res_rtl) {
      arr.remove_dup_syms();
      res_rtl->clear();
      res_rtl->insert(arr);
      ret = vTRUE;
    }

  }

  return ret;
}

// Move the selected items from one rtl to another:

vbool GROUPswApp::ShiftSel(int src_id, int res_id)
{
  Initialize(GROUPswApp::ShiftSel);

  vbool ret = vFALSE;

  if ((src_id > 0) && (res_id > 0)) {
      RTListServer *src_rtl = RTListServer::find(src_id);
      RTListServer *res_rtl = RTListServer::find(res_id);
      if (src_rtl && res_rtl) {
	symbolArr src(0);
	symbolArr sel(0);
	symbolArr usel(0);

	src = src_rtl->getArr();
	src_rtl->getSelArr(sel);

	if (sel.size() > 0) {
	  symbolPtr j;
	  ForEachS (j, src) {
	    symbolPtr k;
	    bool unselected = true;
            ForEachS (k, sel) {
              if (j == k)
	        unselected = false;
	    }
	    if (unselected)
	      usel.insert_last(j);
	  }

	  src_rtl->clear();
	  src_rtl->insert(usel);
	  res_rtl->insert(sel);
        }

        ret = vTRUE;
      }
  }

  return ret;
}

// Init the Ask list.
vstr* GROUPswApp::getAskListString()
{
    Initialize(GROUPswApp::getAskList);

    if (!GROUPswApp::AskListString) {
        GROUPswApp::AskListString = vstrCloneScribed(vcharScribeLiteral("") );
        for (int i=0; i<GROUPswApp::AskListSize; i++) {
            GROUPswApp::AskListString = vstrAppendScribed(GROUPswApp::AskListString,
                                            vcharScribeLiteral(" {") );
            GROUPswApp::AskListString = vstrAppend(GROUPswApp::AskListString,
                                            (const vchar*) GROUPswApp::AskList[i].name);
            GROUPswApp::AskListString = vstrAppendScribed(GROUPswApp::AskListString,
                                            vcharScribeLiteral("}") );
        }
    }
    return (GROUPswApp::AskListString);
}



// Ask the requested question.
vbool GROUPswApp::Ask(const unsigned int queryId, const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::Ask);
    vbool status = vFALSE;
    ASK_FUNCTION_TYPE method = NULL;

    if (queryId < GROUPswApp::AskListSize) method = AskList[queryId].cmd;

    if (method) status = method(sources, results);

    return (status);
}


// Delete selected entries in the Groups list.
vbool GROUPswApp::deleteGroups(const symbolArr groups)
{
    Initialize(GROUPswApp::deleteGroups);

    bool confirm = true;
    bool yes_to_all = false;

    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Delete the group.
             yes_to_all = group->delete_group(confirm, yes_to_all);
        } else {
            symbolPtr xrefSym = groupSym.get_xrefSymbol();
            symbolPtr fileSym;
            if (xrefSym.xrisnotnull() ) fileSym = xrefSym->get_def_file();
            Xref* xref = 0;
            if (fileSym.xrisnotnull() ) xref = fileSym->get_Xref();
            if (xref) xref->remove_module(fileSym.get_name() );
        }
    }
    return(vTRUE);
}    // end of deleteGroups.



// Save selected entries in the Groups list.
vbool GROUPswApp::saveGroups(const symbolArr groups)
{
    Initialize(GROUPswApp::saveGroups);

    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
		if (group->is_scratch() )
			group->make_nonscratch();
		else
			group->set_modified();
            // Save the group.
            db_save(group, NULL);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }
    return(vTRUE);
}    // end of saveGroups.



// Print selected entries in the Known Groups list.
vbool GROUPswApp::printGroups(const symbolArr groups)
{
    Initialize(GROUPswApp::printGroups);

    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        dis_message(0, MSG_INFORM, "Processing '%s'\n", groupSym.get_name() );
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Print the group.
//          rcall_dis_DISui( blah, blah, blah);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }
    return(vTRUE);
}    // end of printGroups.



// Rename selected entries in the Known Groups list.
vbool GROUPswApp::renameGroups(const symbolArr groups)
{
    Initialize(GROUPswApp::renameGroups);

    dis_message(0, MSG_INFORM, "Should start up Impact Analysis next.\n");
// CHECK FOR SCRATCH AND RENAME.
// CALL IMPACT ANALYSIS WITH THE RTL OR ARRAY.
    return(vTRUE);
}    // end of renameGroups.

extern GroupApp groupApp;

// Convert subsystems to new-style groups:

vbool GROUPswApp::ConvertToGroup(const symbolArr& groups)
{
  Initialize(GROUPswApp::ConvertToGroup);

  vbool ret = vFALSE;

  symbolPtr sym;
  ForEachS(sym, groups) {
    symbolArr mem(0);
    groupHdrPtr group = groupHdr::sym_to_groupHdr(sym);
    if (group) {
      group->get_members(mem, FALSE);
      symbolArr empty(0);
      groupApp.PerformUtil(GroupApp::EXTGROUP_GRABSCR, empty, mem);
    }
  }
  ret = vTRUE;

  return ret;
}

// Convert selected entries in the Known Groups list to or from Subsystems.
vbool GROUPswApp::convertGroups(const symbolArr groups)
{
    Initialize(GROUPswApp::convertGroups);

    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        // Add the group to the delete list.
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            if (group->is_subsystem()) {
		// Convert subsystem into a saved/model group.
                group->convert_to_group();
            } else if (group->is_scratch() ) {
			// Convert a scratch group into a saved/model group.
			group->make_nonscratch();
            } else {
			// Convert a saved/model group into a subsystem.
                group->convert_to_subsystem();
            }
        }
    }
    return(vTRUE);
}    // end of convertGroups.



// Union selected entries in the Known Groups list.
vbool GROUPswApp::unionGroups(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::unionGroups);
    int status = vFALSE;    // Very pessimistic today.

    if (groups.size() == 2) {
        groupHdrPtr group1 = NULL;
        groupHdrPtr group2 = NULL;
        symbolPtr groupSym;
        ForEachS(groupSym, groups) {
            if (group1 == NULL) 
                group1 = groupHdr::sym_to_groupHdr(groupSym);
            else if (group2 == NULL) 
                group2 = groupHdr::sym_to_groupHdr(groupSym);
        }
        if (group1 && group2) {
            status = unionGroupMembers(group1, group2, members);
        }
    } else {
        dis_message(0, MSG_WARN, "Must have 2 groups selected.\n");
    }
    return(status);
}    // end of unionGroups.



// Intersect selected entries in the Known Groups list.
vbool GROUPswApp::intersectGroups(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::intersectGroups);
    int status = vFALSE;    // Very pessimistic today.

    if (groups.size() == 2) {
        groupHdrPtr group1 = NULL;
        groupHdrPtr group2 = NULL;
        symbolPtr groupSym;
        ForEachS(groupSym, groups) {
            if (group1 == NULL) 
                group1 = groupHdr::sym_to_groupHdr(groupSym);
            else if (group2 == NULL) 
                group2 = groupHdr::sym_to_groupHdr(groupSym);
        }
        if (group1 && group2) {
            status = intersectGroupMembers(group1, group2, members);
        }
    } else {
        dis_message(0, MSG_WARN, "Must have 2 groups selected.\n");
    }
    return(status);
}    // end of intersectGroups.



// Subtract (A - B) selected entries in the Known Groups list.
vbool GROUPswApp::subtractABGroups(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::subtractABGroups);
    int status = vFALSE;    // Very pessimistic today.

    if (groups.size() == 2) {
        groupHdrPtr group1 = NULL;
        groupHdrPtr group2 = NULL;
        symbolPtr groupSym;
        ForEachS(groupSym, groups) {
            if (group1 == NULL) 
                group1 = groupHdr::sym_to_groupHdr(groupSym);
            else if (group2 == NULL) 
                group2 = groupHdr::sym_to_groupHdr(groupSym);
        }
        if (group1 && group2) {
            status = subtractGroupMembers(group1, group2, members);
        }
    } else {
        dis_message(0, MSG_WARN, "Must have 2 groups selected.\n");
    }
    return(status);
}    // end of subtractABGroups.



// Subtract (B - A) selected entries in the Known Groups list.
vbool GROUPswApp::subtractBAGroups(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::subtractBAGroups);
    int status = vFALSE;    // Very pessimistic today.

    if (groups.size() == 2) {
        groupHdrPtr group1 = NULL;
        groupHdrPtr group2 = NULL;
        symbolPtr groupSym;
        ForEachS(groupSym, groups) {
            if (group1 == NULL) 
                group1 = groupHdr::sym_to_groupHdr(groupSym);
            else if (group2 == NULL) 
                group2 = groupHdr::sym_to_groupHdr(groupSym);
        }
        if (group1 && group2) {
            status = subtractGroupMembers(group2, group1, members);
        }
    } else {
        dis_message(0, MSG_WARN, "Must have 2 groups selected.\n");
    }
    return(status);
}    // end of subtractBAGroups.



// Copy selected Members from the current group.
vbool GROUPswApp::copyMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::copyMembers);

    // Set the Member clipboard to the members.
    MemberClipboard = members;

    return(vTRUE);
}



// Cut selected Members from the current group.
vbool GROUPswApp::cutMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::cutMembers);

    // Copy selected Members into the Member Clipboard.
    //copyMembers(groups, members);

    // Delete the selected members from each group.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
       if (group) {
           group->remove_members(members);
       }
    }

    return(vTRUE);
}



// paste selected Members into the current group.
vbool GROUPswApp::pasteMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::pasteMembers);

    // Create the array of members to paste.
    // This is needed because the cut will use the Clipboard.
    //symbolArr pasteArray(MemberClipboard);

    // Cut any selected Members.
    //cutMembers(groups, members);


  // See if there is a nested project in the members list.
  vbool hasNestedProject = checkForNestedProjects(members);

  // Decide how to handle adding projects to the subsystem.
  groupHdr::nested_projects_action proj_action = groupHdr::MIRROR_HIERARCHY;
  if (hasNestedProject) {
    if (!getProjectCopyType("Add Members to Subsystem") )
       proj_action = groupHdr::ALL_NESTED_FILES;
  }


    // Loop through the selected groups.
    // Add the selected members to each group.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
       if (group) {
           group->add_members(members, proj_action, groupHdr::DEFAULT_ACC);
       }
    }

    return(vTRUE);
}

// Assign groups to members:
vbool GROUPswApp::assignMembers(const symbolArr groups, symbolArr& members)
{
  Initialize(GROUPswApp::assignMembers);

  vbool ret = vTRUE;

  //symbolArr pasteArray(MemberClipboard);
  //cutMembers(groups, members);


  // See if there is a project in the members list.
  vbool hasNestedProject = checkForNestedProjects(members);

  // Decide how to handle adding projects to the subsystem.
  groupHdr::nested_projects_action proj_action = groupHdr::MIRROR_HIERARCHY;
  if (hasNestedProject) {
    if (!getProjectCopyType("Replace Subsystem Members") )
       proj_action = groupHdr::ALL_NESTED_FILES;
  }

  symbolPtr groupSym;
  ForEachS(groupSym, groups) {
    groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
    if (group) {
      symbolArr old(0);
      group->get_members(old, false);
      group->remove_members(old);
      group->add_members(members, proj_action, groupHdr::DEFAULT_ACC);
    }
  }

  return ret;
}


// Check the symbol array for projects which contain other projects.
// Also provided for a small fee:
//	If a DD_SUBSYSTEM is found, warn the user that it will be ignored.
vbool GROUPswApp::checkForNestedProjects(const symbolArr& members)
{
        vbool foundNested = vFALSE;

	symbolPtr member;
	ForEachS(member, members) {
		if (!foundNested && (member.get_kind() == DD_PROJECT) ) {
			// Found a project, now see if it contains a project.
			projNodePtr proj = projNode::find_project(member.get_name());
			if (proj) {
				proj->refresh();
				symbolArr& proj_contents = proj->rtl_contents();
				symbolPtr sym;
				ForEachS(sym, proj_contents) {
					if (sym.relationalp())
						RelationalPtr(sym)->construct_symbolPtr(sym);
					if (sym.isnotnull() ) {
						if (sym.get_kind() == DD_PROJECT)
							foundNested = true;
					}
				}
			}
		} else if (member.get_kind() == DD_SUBSYSTEM) {
			dis_message(NULL, MSG_WARN, 
				"Unable to add subsystem '%s' as a member.",
				member.get_name() );
		}
	}


	return(foundNested);
}


// Ask user how to copy the given project.
// Returns true if the user wants multi-level and false for single level.
vbool GROUPswApp::getProjectCopyType(const char* title)
{
	// This function used to be called for
        vbool copyMulti = vTRUE;

	genString quest;
	quest.printf("Add Projects as Multi-Level or Single Level?");

	int status = dis_question(title, "Add Multi-Level",
				"Add Single Level", quest);

	if (status == 0) { // Copy Single button selected.
		copyMulti = vFALSE;
	}

    return(copyMulti);
}



// Get the pointer to the CreateGroup class.
CreateGroup* GROUPswApp::getCreatePtr()
{
    Initialize(GROUPswApp::getCreatePtr);

    return (Create);
}





//----------     Public Relate Group Methods     ----------//



// Show the Client groups for the selected source groups.
vbool GROUPswApp::showClientGroups(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showClientGroups);

    symbolSet clientSet;
    clientSet.insert(results);
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get the clients for this group.
            symbolArr clientSyms;
            group->get_clients(clientSyms);
            // Add the clients to the target Set, if not already there.
            symbolPtr clientSym;
            ForEachS(clientSym, clientSyms) {
                if (!clientSet.includes(clientSym) )
                    clientSet.insert(clientSym);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    results = clientSet;

    return (vTRUE);
}



// Show the Server groups for the selected source groups.
vbool GROUPswApp::showServerGroups(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showServerGroups);

    // Get the list of Server groups.
    symbolSet serverSet;
    serverSet.insert(results);
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get the servers for this group.
            symbolArr serverSyms;
            group->get_servers(serverSyms);
            // Add the servers to the target Set, if not already there.
            symbolPtr serverSym;
            ForEachS(serverSym, serverSyms) {
                if (!serverSet.includes(serverSym) )
                    serverSet.insert(serverSym);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    results = serverSet;
    return (vTRUE);
}



// Show the Peer groups for the selected source groups.
vbool GROUPswApp::showPeerGroups(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showPeerGroups);

    // Get the list of Peer groups.
    symbolSet peerSet;
    peerSet.insert(results);
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get the peers for this group.
            symbolArr peerSyms;
            group->get_peers(peerSyms);
            // Add the peers to the target Set, if not already there.
            symbolPtr peerSym;
            ForEachS(peerSym, peerSyms) {
                if (!peerSet.includes(peerSym) )
                    peerSet.insert(peerSym);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }
    results = peerSet;
    return (vTRUE);
}



// Show the Unrelated groups for the selected source groups.
vbool GROUPswApp::showUnrelatedGroups(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showUnrelatedGroups);

    // Init unrelated array with all possible groups.
    symbolArr unrelated;
    findAllGroups(unrelated);

    // Loop through source groups, removing related groups.
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Remove the source group.
            // A group cannot be related or unrelated to itself.
            unrelated.remove(groupSym);
            // Remove client groups.
            symbolArr clientSyms;
            group->get_clients(clientSyms);
            symbolPtr client;
            ForEachS(client, clientSyms) {
                unrelated.remove(client);
            }

            // Remove server groups.
            symbolArr serverSyms;
            group->get_servers(serverSyms);
            symbolPtr server;
            ForEachS(server, serverSyms) {
                unrelated.remove(server);
            }

            // Remove peer groups.
            symbolArr peerSyms;
            group->get_peers(peerSyms);
            symbolPtr peer;
            ForEachS(peer, peerSyms) {
                unrelated.remove(peer);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    // insert unrelated groups into the results array.
    //     (Unless they are already there.)
    symbolSet resultSet;
    resultSet.insert(results);
    symbolPtr sym;
    ForEachS(sym, unrelated) {
        if (!resultSet.includes(sym) )
            resultSet.insert(sym);
    }
    results = resultSet;

    return (vTRUE);
}



// Set the Client groups for the selected source groups.
vbool GROUPswApp::setClientGroups(const symbolArr groups, symbolArr& clients)
{
    Initialize(GROUPswApp::setClientGroups);

    // Loop through the source groups, adding client relations.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Make each group in the client array a client to this group.
            symbolPtr clientSym;
            ForEachS(clientSym, clients) {
                // Make sure that the client isn't the same group as the source.
                if (strcmp(clientSym.get_name(), groupSym.get_name()) != 0) {
                    groupHdrPtr client = groupHdr::sym_to_groupHdr(clientSym);
                    if (client) {
                        group->make_client(client);
                    }
                }
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set the Server groups for the selected source groups.
vbool GROUPswApp::setServerGroups(const symbolArr groups, symbolArr& servers)
{
    Initialize(GROUPswApp::setServerGroups);

    // Loop through the source groups, adding server relations.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Make each group in the server array a server to this group.
            symbolPtr serverSym;
            ForEachS(serverSym, servers) {
                // Make sure that the two groups are not the same.
                if (strcmp(serverSym.get_name(), groupSym.get_name()) != 0) {
                    groupHdrPtr server = groupHdr::sym_to_groupHdr(serverSym);
                    if (server) {
                        group->make_server(server);
                    }
                }
            }
            // Set the servers for this group.
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set the Peer groups for the selected source groups.
vbool GROUPswApp::setPeerGroups(const symbolArr groups, symbolArr& peers)
{
    Initialize(GROUPswApp::setPeerGroups);

    // Loop through the selected groups, adding peer relations.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Make each group in the peer array a peer to this group.
            symbolPtr peerSym;
            ForEachS(peerSym, peers) {
                // Make sure that the two groups are not the same.
                if (strcmp(peerSym.get_name(), groupSym.get_name()) != 0) {
                    groupHdrPtr peer = groupHdr::sym_to_groupHdr(peerSym);
                    if (peer) {
                        group->make_peer(peer);
                    }
                }
            }
            // Set the peers for this group.
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set the Unrelated groups for the selected source groups.
vbool GROUPswApp::setUnrelatedGroups(const symbolArr groups, symbolArr& unrelated)
{
    Initialize(GROUPswApp::setUnrelatedGroups);

    // Make the unrelated grous unrelated to each of the source groups.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Make each group in the unrelated array unrelated to this group.
            // Note - we allow unrelating the selected group as a safety net.
            symbolPtr unrelatedSym;
            ForEachS(unrelatedSym, unrelated) {
                    groupHdrPtr unrelated = groupHdr::sym_to_groupHdr(unrelatedSym);
                if (unrelated) {
                    group->make_unrelated(unrelated);
                }
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}





//----------     Public Relate Member Methods     ----------//



// Show all Members of selected groups.
vbool GROUPswApp::showLocalMembers(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showLocalMembers);

    // This set will hold the members.
    symbolSet memberSet;
    memberSet.insert(results);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get all members for the group.
            symbolArr members;
            group->get_members(members, FALSE);

            // Add the members to the members list.
            //    (If it isn't already in the list.)
            symbolPtr mem;
            ForEachS(mem, members) {
                if (!memberSet.includes(mem) )
                    memberSet.insert_last(mem);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    results = memberSet;
    return (vTRUE);
}



// Show Transitive Members of selected groups.
vbool GROUPswApp::showTransitiveMembers(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showTransitiveMembers);

    // This set will hold the transitive members.
    symbolSet memberSet;
    memberSet.insert(results);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get local and transitive members for the group.
            symbolArr members;
            group->get_members(members, true);
            // Get a set of only local members for the group.
            symbolArr localMembers;
            group->get_members(localMembers, false);
            symbolSet localSet;
            localSet.insert(localMembers);


            // Add the transitive members to the member set.
            //    (If it isn't in the Local Set and it isn't already in the list.)
            symbolPtr mem;
            ForEachS(mem, members) {
                if (!localSet.includes(mem) && !memberSet.includes(mem) ) {
                    memberSet.insert_last(mem);
                }
             }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }
    results = memberSet;

    return (vTRUE);
}



// Show Public Members of selected groups.
vbool GROUPswApp::showPublicMembers(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showPublicMembers);

    // This set will hold the members.
    symbolSet memberSet;
    memberSet.insert(results);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get public members for the group.
            symbolArr members;
            group->get_public_members(members, FALSE);

            // Add the members to the members list.
            //    (If it isn't already in the list.)
            symbolPtr mem;
            ForEachS(mem, members) {
                if (!memberSet.includes(mem) )
                    memberSet.insert_last(mem);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    results = memberSet;
    return (vTRUE);
}



// Show Private Members of selected groups.
vbool GROUPswApp::showPrivateMembers(const symbolArr sources, symbolArr& results)
{
    Initialize(GROUPswApp::showPrivateMembers);

    // This set will hold the members.
    symbolSet memberSet;

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, sources) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Get private members for the group.
            symbolArr members;
            group->get_private_members(members);

            // Add the members to the members list.
            //    (If it isn't already in the list.)
            symbolPtr mem;
            ForEachS(mem, members) {
                if (!memberSet.includes(mem) )
                    memberSet.insert_last(mem);
            }
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }
    results = memberSet;
    return (vTRUE);
}



// Set Public Members of selected groups.
vbool GROUPswApp::setPublicMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::setPublicMembers);

    // Loop through the selected groups, making the selected members public.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            group->make_members_public(members);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set Private Members of selected groups.
vbool GROUPswApp::setPrivateMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::setPrivateMembers);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            group->make_members_private(members);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set Visible Members of selected groups.
vbool GROUPswApp::setVisibleMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::setVisibleMembers);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            group->make_server_members_visible(members);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}



// Set Invisible Members of selected groups.
vbool GROUPswApp::setInvisibleMembers(const symbolArr groups, symbolArr& members)
{
    Initialize(GROUPswApp::setInvisibleMembers);

    // Loop through the selected groups, adding all members to the members list.
    symbolPtr groupSym;
    ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            group->make_server_members_invisible(members);
        } else {
            dis_message(0, MSG_ERROR, "Symbol '%s' isn't a groupHdr!!!\n",
                    groupSym.get_name() );
        }
    }

    return (vTRUE);
}


// Find violations in the selected groups.
vbool GROUPswApp::findViolations(const symbolArr groups,
                                 const char* fileName,
                                 const vchar* tclBuffer /*=NULL*/)
{
    Initialize(GROUPswApp::findViolations);

    symbolPtr groupSym;

    // Open the report file.
    FILE* out = OSapi_fopen(fileName, "w");

    if (out) {
      // Validate each group.
      ForEachS(groupSym, groups) {
        groupHdrPtr group = groupHdr::sym_to_groupHdr(groupSym);
        if (group) {
            // Validate the group.
            group->validate_group_with_project(0, out);
        }
      }
    
      // Close the report file.
      OSapi_fclose(out);
      dis_message(NULL, MSG_INFORM, "Violation report written to file '%s'.",
                    (const char*) fileName);
      if (tclBuffer) {
            vstr* cmdStr = vstrCloneScribed(vcharScribeFormat( (vchar*)
                       "dis_ViewFile {%s} {%s} 0", tclBuffer, fileName) );
            rcall_dis_DISui_eval_async(DISui_app, cmdStr);
            vstrDestroy(cmdStr);
      }
      return(vTRUE);
    } else {
       dis_message(NULL, MSG_ERROR, "Unable to open report file!");
       return(vFALSE);
    }
}    // end of findViolations.


//----------     NO class GROUPswApp Private Members     ----------








/**********     end of GROUPswApp.C     **********/
