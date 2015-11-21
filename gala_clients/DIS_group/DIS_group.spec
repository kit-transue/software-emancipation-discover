service DISgroup


!#ifdef SERVER
!#include "include/GROUPswApp.h"
!#include "include/CreateGroup.h"
!#include "SelectionIterator.h"
!#include "messages.h"
!static GROUPswApp* App = NULL;
!
!// Needed for DISui calls.
!#include "../../clients/communications/include/Application.h"
!#include "../../clients/DIS_ui/interface.h"
!extern Application* DISui_app;
!
!extern propManager* prop_manager;
!extern "C" void galaxyStartTimer (void);
!extern "C" void galaxyStopTimer (void);
!
!// Class kludge to improve performance.
!class galaxyBoost {
!    public:
!       galaxyBoost (void) { galaxyStopTimer (); }
!       ~galaxyBoost (void) { galaxyStartTimer (); }
!};
!
!// GROUP function to force consistent initialization.
!vbool GroupInit()
!{
!    galaxyBoost boost;
!    if (!App)
!	App = new GROUPswApp;
!    if (!GROUPswApp::isRunning() )
!        App->Startup();
!    return(vTRUE);
!};
!
!#include "include/GroupApp.h"
!GroupApp groupApp;
!
!#endif



@@ dis_group_init
{    // Args: KnownGroupsList 
    if (GroupInit() ) {
        if (strcmp(argv[1], "-groupsList") == 0) {
            if (argc == 3) {
                int rtlId = atoi(argv[2]);
                App->initGroupsList(rtlId);
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_init -groupsList:%d args found instead of 3!!!", argc);
            }

        } else if (strcmp(argv[1], "-askList") == 0) {
            if (argc == 2) {
                char* askList = (char*) App->getAskListString();
                Tcl_SetResult(interp, askList, TCL_VOLATILE);
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_init -askList:%d args found instead of 2!!!", argc);
            }
        }
    }
    return TCL_OK;
}



@@ dis_group_shutdown
{
    galaxyBoost boost;
    if (GROUPswApp::isRunning() ) {
	App->Shutdown();
	delete App;
	App = NULL;
    }
    return TCL_OK;
}





@@ dis_group_create
{
    if (GroupInit() ) {
        if (strcmp(argv[1], "-setName") == 0) {
            App->getCreatePtr()->setNextGroupName((vchar*) argv[2]);

        } else if (strcmp(argv[1], "-getName") == 0) {
            char* nextName = (char*) App->getCreatePtr()->getNextGroupName();
            Tcl_SetResult(interp, (char*) nextName, TCL_VOLATILE);

        } else if (strcmp(argv[1], "-create") == 0) {
            RTListServer* groupsRTL = NULL;
            int groupsId = atoi(argv[2]);
            if (groupsId > 0) {
                groupsRTL = RTListServer::find(groupsId);
            }
            int groupType = atoi(argv[3]);
            symbolArr members(0);
            RTListServer* membersRTL = NULL;
            int membersId = atoi(argv[4]);
            if (membersId > 0) {
                membersRTL = RTListServer::find(membersId);
                if (membersRTL) membersRTL->getSelArr(members);
            }
            int memberCopyType = atoi(argv[5]);
            groupHdrPtr newGroup = App->getCreatePtr()->createNextGroup(groupType,
                                             members, memberCopyType);
            if (groupsRTL && newGroup) {
                groupsRTL->insert(newGroup);
            }
        }
    }
    return TCL_OK;
}





@@ dis_group_ask
{
    if (GroupInit() ) {
        if (argc == 4) {
            SelectionIterator sel((vchar*)argv[1]);

            symbolArr groups(0);
            int sourceId = atoi(argv[2]);
            if (sourceId > 0) {
                RTListServer* sourceRTL = RTListServer::find(sourceId);
                if (sourceRTL) sourceRTL->getSelArr(groups);
            }

            int resultsId = atoi(argv[3]);
            RTListServer* resultsRTL = NULL;
            if (resultsId > 0) {
                resultsRTL = RTListServer::find(resultsId);
                if (resultsRTL) resultsRTL->clear();
            }
            symbolArr results(0);
            while (sel.Next() )
                App->Ask(sel.GetNext(), groups, results);
            if (resultsRTL) resultsRTL->insert(results);
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_query:%d args found instead of 4!!!", argc);
        }
    }
    return TCL_OK;
}






@@ dis_group_file
{
    if (GroupInit() ) {
        if (argc == 3) {
            vbool status = vFALSE;
            symbolArr groups(0);
            RTListServer* sourceRTL = 0;
            int sourceId = atoi(argv[2]);
            if (sourceId > 0) {
                sourceRTL = RTListServer::find(sourceId);
                if (sourceRTL) sourceRTL->getSelArr(groups);
            }

            if (strcmp(argv[1], "-delete") == 0) {
                // Delete each group, with prompting.
                status = App->deleteGroups(groups);
                // Update the Groups list.
                App->initGroupsList(sourceId);
            } else if (strcmp(argv[1], "-save") == 0) {
                App->saveGroups(groups);
            } else if (strcmp(argv[1], "-print") == 0) {
                App->printGroups(groups);
            } else if (strcmp(argv[1], "-rename") == 0) {
                status = App->renameGroups(groups);
            } else if (strcmp(argv[1], "-convert") == 0) {
                status = App->convertGroups(groups);
            }
            if (status && sourceRTL) {
//                sourceRTL->select(groups);
                sourceRTL->_propagate();
            }
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_file:%d args found instead of 3!!!", argc);
        }
    }
    return TCL_OK;
}





@@ dis_group_ops
{
    if (GroupInit() ) {
        if (argc == 4) {
            vbool status = vFALSE;
            symbolArr groups(0);
            RTListServer* groupsRTL = NULL;
            int groupsId = atoi(argv[2]);
            if (groupsId > 0) {
                groupsRTL = RTListServer::find(groupsId);
                if (groupsRTL) groupsRTL->getSelArr(groups);
            }
            symbolArr members(0);
            RTListServer* membersRTL = NULL;
            int membersId = atoi(argv[3]);
            if (membersId > 0) {
                membersRTL = RTListServer::find(membersId);
                if (membersRTL) membersRTL->clear();
            }
            if (strcmp(argv[1], "-union") == 0) {
                status = App->unionGroups(groups, members);
            } else if (strcmp(argv[1], "-intersect") == 0) {
                status = App->intersectGroups(groups, members);
            } else if (strcmp(argv[1], "-subtractAB") == 0) {
                status = App->subtractABGroups(groups, members);
            } else if (strcmp(argv[1], "-subtractBA") == 0) {
                status = App->subtractBAGroups(groups, members);
            }
            if (status && membersRTL) {
                groupHdrPtr newGroup = App->getCreatePtr()->createNextGroup(0, members, 0);
                if (groupsRTL) groupsRTL->insert(newGroup);
            }
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_file:%d args found instead of 4!!!", argc);
        }
    }
    return TCL_OK;
}


@@ dis_group_edit
{
    if (GroupInit() ) {
        if (argc == 4) {
            vbool status = vFALSE;
            symbolArr groups(0);
            int groupsId = atoi(argv[2]);
            RTListServer* groupsRTL = NULL;
            if (groupsId > 0)
                groupsRTL = RTListServer::find(groupsId);
                if (groupsRTL) groupsRTL->getSelArr(groups);
            symbolArr members(0);
            int membersId = atoi(argv[3]);
            RTListServer* membersRTL = NULL;
            if (membersId > 0) {
                membersRTL = RTListServer::find(membersId);
                if (membersRTL) membersRTL->getSelArr(members);
            }
            if (strcmp(argv[1], "-cut") == 0) {
                status = App->cutMembers(groups, members);
            } else if (strcmp(argv[1], "-copy") == 0) {
                status = App->copyMembers(groups, members);
            } else if (strcmp(argv[1], "-paste") == 0) {
                status = App->pasteMembers(groups, members);
            } else if (strcmp(argv[1], "-assign") == 0) {
                status = App->assignMembers(groups, members);
            }
            if (status && groupsRTL) groupsRTL->select(groups);
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_file:%d args found instead of 4!!!", argc);
        }
    }
    return TCL_OK;
}


@@ dis_group_setGroupRelations
{
    if (GroupInit() ) {
        if (argc == 4) {
            vbool status = vFALSE;
            symbolArr groups(0);
            int groupsId = atoi(argv[2]);
            RTListServer* groupsRTL = NULL;
            if (groupsId > 0) {
                groupsRTL = RTListServer::find(groupsId);
                //if (groupsRTL) groupsRTL->getSelArr(groups);
		if (groupsRTL) groups = groupsRTL->getArr();
            }
            symbolArr targets(0);
            int targetsId = atoi(argv[3]);
            RTListServer* targetsRTL = NULL;
            if (targetsId > 0) {
                targetsRTL = RTListServer::find(targetsId);
                //if (targetsRTL) targetsRTL->getSelArr(targets);
		if (targetsRTL) targets = targetsRTL->getArr();
            }
            if (strcmp(argv[1], "-clients") == 0) {
                status = App->setClientGroups(groups, targets);
            }else if (strcmp(argv[1], "-servers") == 0) {
                status = App->setServerGroups(groups, targets);
            }else if (strcmp(argv[1], "-peers") == 0) {
                status = App->setPeerGroups(groups, targets);
            }else if (strcmp(argv[1], "-unrelated") == 0) {
                status = App->setUnrelatedGroups(groups, targets);
            }
            //if (status && groupsRTL) groupsRTL->select(groups);
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_file:%d args found instead of 4!!!", argc);
        }
    }
    return TCL_OK;
}

@@ dis_group_setMemberRelations
{
    if (GroupInit() ) {
        if (argc == 4) {
            vbool status = vFALSE;
            symbolArr groups(0);
            int groupsId = atoi(argv[2]);
            RTListServer* groupsRTL = NULL;
            if (groupsId > 0) {
                groupsRTL = RTListServer::find(groupsId);
                if (groupsRTL) groupsRTL->getSelArr(groups);
            }
            symbolArr members(0);
            int membersId = atoi(argv[3]);
            RTListServer* membersRTL = NULL;
            if (membersId > 0) {
                membersRTL = RTListServer::find(membersId);
                if (membersRTL) membersRTL->getSelArr(members);
            }
            if (strcmp(argv[1], "-public") == 0) {
                status = App->setPublicMembers(groups, members);
            }else if (strcmp(argv[1], "-private") == 0) {
                status = App->setPrivateMembers(groups, members);
            }else if (strcmp(argv[1], "-visible") == 0) {
                status = App->setVisibleMembers(groups, members);
            }else if (strcmp(argv[1], "-invisible") == 0) {
                status = App->setInvisibleMembers(groups, members);
            }
            if (status && groupsRTL) groupsRTL->select(groups);
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_file:%d args found instead of 4!!!", argc);
        }
    }
    return TCL_OK;
}




@@ dis_group_validate
{
    if (GroupInit() ) {
        if (strcmp(argv[1], "-findViolations") == 0) {
            if (argc == 5) {
                vbool status = vFALSE;
				const vchar* outputName   = (vchar*) argv[2];
                const char* fileName = argv[3];
                symbolArr groups(0);
                int groupsId = atoi(argv[4]);
                RTListServer* groupsRTL = NULL;
                if (groupsId > 0) {
                    groupsRTL = RTListServer::find(groupsId);
                    if (groupsRTL) groupsRTL->getSelArr(groups);
                    status = App->findViolations(groups, fileName, outputName);
                }
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_validate -findViolations:%d args found instead of 4!!!", argc);
            }
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_validate:'%s' is an unknown option.", argv[1]);
        }
    }
    return TCL_OK;
}




@@ dis_group_import
{
    if (GroupInit() ) {
        if (strcmp(argv[1], "-find") == 0) {
            if (argc == 2) {
                vbool status = vFALSE;
                genString groupNames;
                status = App->findRTLGroups(groupNames);
                Tcl_SetResult(interp, (char*) groupNames, TCL_VOLATILE);
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_import -find:%d args found instead of 2!!!", argc);
            }

        } else if (strcmp(argv[1], "-RTL") == 0) {
            if (argc == 2) {
                vbool status = vFALSE;
                status = App->importRTLGroups();
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_import -RTL:%d args found instead of 2!!!",
                            argc);
            }

        } else if (strcmp(argv[1], "-package") == 0) {
            if (argc == 3) {
                vbool status = vFALSE;
                const char* packageName = argv[2];
                status = App->importPackageSession(packageName);
            } else {
                dis_message(NULL, MSG_WARN, "dis_group_import -package:%d args found instead of 3!!!",
                            argc);
            }
        } else {
            dis_message(NULL, MSG_WARN, "dis_group_import:'%s' is an unknown option.", argv[1]);
        }
    }
    return TCL_OK;
}

@@ dis_group_GrabSubsysSel
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    App->GrabSubsysSel(argc - 1, argv + 1);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s <rtl> <id_1> [ <id_2> <id_3> ... ]", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

@@ dis_group_shift_sel
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int src_id = atoi(argv[1]);
    int res_id = atoi(argv[2]);
    App->ShiftSel(src_id, res_id);
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s <rtl_1> <rtl_2>", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

@@ dis_group_unselect_rtl
{
  int ret = TCL_ERROR;

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      int id = atoi(argv[i]);
      if (id > 0) {
        RTListServer *rtl = RTListServer::find(id);
	    if (rtl) {
	      symbolArr arr(0);
	      arr = rtl->getArr();
	      if (arr.size() > 0) {
	        rtl->clear();	  
	        rtl->insert(arr);
          }
        }
      }
    }
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s <id_1> [ <id_2> <id_3> ... ]", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

@@ dis_group_remove_sel
{
  int ret = TCL_ERROR;
  
  if (argc >= 2) {
	for (int i = 1; i < argc; i++) {
	  int id = atoi(argv[i]);
	  if (id > 0) {
	    RTListServer *rtl = RTListServer::find(id);
		if (rtl) {
		  symbolArr sel(0);
		  symbolArr arr(0);
		  symbolArr usel(0);
		  arr = rtl->getArr();
		  rtl->getSelArr(sel);
		  if (sel.size() > 0) {
		    symbolPtr j;
			ForEachS (j, arr) {
			  symbolPtr k;
			  bool unselected = true;
			  ForEachS (k, sel) {
			    if (j == k)
				  unselected = false;
			  }
			  if (unselected)
			    usel.insert_last(j);
			}
			rtl->clear();
			rtl->insert(usel);
		  }
		}
	  }
	}
    ret = TCL_OK;
  }
  else {
    genString usage;
    usage.printf("Usage: %s <id_1> [ <id_2> <id_3> ... ]", argv[0]);
    Tcl_AppendResult(interp, (char *)usage, NULL);
  }

  return ret;
}

@@ dis_group_export
{
  int ret = TCL_ERROR;

  genString usage;
  usage.printf("Usage: %s group <rtl_id>", argv[0]);

  if (argc >= 3) {
    if (!strcmp(argv[1], "group")) {
      int rtl_id = atoi(argv[2]);
      if (rtl_id > 0) {
        RTListServer *rtl = RTListServer::find(rtl_id);
        if (rtl) {
          symbolArr sel;
          rtl->getSelArr(sel);
          App->ConvertToGroup(sel);
        }
      }
    }
    else 
      Tcl_AppendResult(interp, (char *)usage, NULL);
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

@@ dis_group_info
{
  int ret = TCL_ERROR;

  genString usage;
  usage.printf("Usage: %s { names <src_id> } | { queries }", argv[0]);

  if (argc >= 2) {
    if (!strcmp(argv[1], "names")) {
      if (argc == 3) {
        int id = atoi(argv[2]);
	groupApp.Refresh(id);
        ret = TCL_OK;
      }
      else
        Tcl_AppendResult(interp, (char *)usage, NULL);
    }
    else if (!strcmp(argv[1], "queries")) {
      char *queries = (char *)groupApp.GetQueryList();
      Tcl_SetResult(interp, queries, TCL_VOLATILE);
      ret = TCL_OK;
    }
    else
      Tcl_AppendResult(interp, (char *)usage, NULL);
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

@@ dis_group_query
{
  int ret = TCL_ERROR;

  genString usage;
  usage.printf("Usage: %s <selection> <src_id> <res_id>", argv[0]);

  if (argc == 4) {

    symbolArr src(0);
    int src_id = atoi(argv[2]);
    if (src_id > 0) {
      RTListServer *src_rtl = RTListServer::find(src_id);
      if (src_rtl)
        src_rtl->getSelArr(src);
    }

    RTListServer *res_rtl = NULL;
    int res_id = atoi(argv[3]);
    if (res_id > 0)
      res_rtl = RTListServer::find(res_id);

    symbolArr res(0);
    SelectionIterator sel((vchar *)argv[1]);

    while(sel.Next()) {
      if (groupApp.PerformQuery(sel.GetNext(), src, res)) {
	if (res_rtl) {
	  res_rtl->clear();
	  res_rtl->insert(res);
        }
        groupApp.Refresh(src_id);
        ret = TCL_OK;
      }
    }
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

@@ dis_group_mktemp
{
  int ret = TCL_ERROR;

  genString usage;
  usage.printf("Usage: %s { get } | { set { 0 | 1 } }", argv[0]);

  if (argc >= 2) {
    if (!strcmp(argv[1], "get")) {
      genString str;
      int val = groupApp.GetMakeTempFlag();
      str.printf("%d", val);
      Tcl_SetResult(interp, (char *)str, TCL_VOLATILE);
      ret = TCL_OK;
    }
    else if (!strcmp(argv[1], "set")) {
      if (argc >= 3) {
        int val = atoi(argv[2]);
        if ((val == 0) || (val == 1))
          groupApp.SetMakeTempFlag(val);
        else
          Tcl_AppendResult(interp, (char *)usage, NULL);
      }
      else
        Tcl_AppendResult(interp, (char *)usage, NULL);
    }
    else
      Tcl_AppendResult(interp, (char *)usage, NULL);
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

@@ dis_group_name
{
  int ret = TCL_ERROR;

  genString usage;
  usage.printf("Usage: %s { get_current | get_prefix } | { { set_current | set_prefix } <prefix> }", argv[0]);

  if (argc >= 2) {
    if (!strcmp(argv[1], "get_current")) {
      char *name = groupApp.SnatchResultName(true);
      if (!name)
        name = groupApp.GetResultName();
      Tcl_SetResult(interp, name, TCL_VOLATILE);
      ret = TCL_OK;
    }
    else if (!strcmp(argv[1], "get_prefix")) {
      char *name = groupApp.GetResultPrefix();
      Tcl_SetResult(interp, name, TCL_VOLATILE);
      ret = TCL_OK;
    }
    else if (!strcmp(argv[1], "set_current")) {
      if (argc >= 3) {
        groupApp.SetResultName(argv[2]);
      }
      ret = TCL_OK;
    }
    else if (!strcmp(argv[1], "set_prefix")) {
      if (argc >= 3) {
        if (groupApp.SetResultPrefix(argv[2]))
	  Tcl_SetResult(interp, "0", TCL_VOLATILE);  // 0 - Valid
	else
	  Tcl_SetResult(interp, "1", TCL_VOLATILE);  // 1 - Invalid
      }
      else
	Tcl_SetResult(interp, "1", TCL_VOLATILE);    // Null prefix is invalid
      ret = TCL_OK;
    }
    else
      Tcl_AppendResult(interp, (char *)usage, NULL);
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}

@@ dis_group_manage
{
  int ret = TCL_ERROR;

  char *leftj = "       ";
  genString usage;
  usage.printf("Usage: %s %s\n%s%s%s",
               argv[0],
               "{ save | rename | delete | export | print | refresh } <src_id>",
	       leftj,
               argv[0],
               "{ add | remove | assign | grabscr | grabsav | grabsub } <src_id> <res_id>");

  int src_id = 0;
  int res_id = 0;

  symbolArr src(0);
  symbolArr res(0);

  RTListServer *src_rtl;
  RTListServer *res_rtl;

  if (argc >= 3) {
    src_id = atoi(argv[2]);
    if (src_id > 0) {
      src_rtl = RTListServer::find(src_id);
      if (src_rtl)
        src_rtl->getSelArr(src);
    }
  }

  if (argc >= 4) {
    res_id = atoi(argv[3]);
    if (res_id > 0) {
      res_rtl = RTListServer::find(res_id);
      if (res_rtl) {
        res_rtl->getSelArr(res);
      }
    }
  }

  if (argc >= 3) {

    GroupApp::UtilCode i = (GroupApp::UtilCode) -1;

    if (!strcmp(argv[1], "rename"))
      i = GroupApp::EXTGROUP_RENAME;
    else if (!strcmp(argv[1], "delete"))
      i = GroupApp::EXTGROUP_DELETE;
    else if (!strcmp(argv[1], "add"))
      i = GroupApp::EXTGROUP_ADD;
    else if (!strcmp(argv[1], "remove"))
      i = GroupApp::EXTGROUP_REMOVE;
    else if (!strcmp(argv[1], "assign"))
      i = GroupApp::EXTGROUP_ASSIGN;
    else if (!strcmp(argv[1], "grabscr"))
      i = GroupApp::EXTGROUP_GRABSCR;
    else if (!strcmp(argv[1], "grabsav"))
      i = GroupApp::EXTGROUP_GRABSAV;
    else if (!strcmp(argv[1], "grabsub"))
      i = GroupApp::EXTGROUP_GRABSUB;
    else if (!strcmp(argv[1], "grabsubflat"))
      i = GroupApp::EXTGROUP_GRABSUB_FLAT;
    else if (!strcmp(argv[1], "export"))
      i = GroupApp::EXTGROUP_EXPORT;
    else if (!strcmp(argv[1], "save"))
      i = GroupApp::EXTGROUP_SAVE;
    else if (!strcmp(argv[1], "print"))
      i = GroupApp::EXTGROUP_PRINT;
    else if (!strcmp(argv[1], "refresh"))
      {groupApp.Refresh(src_id); ret = TCL_OK;}
    else
      Tcl_AppendResult(interp, (char *)usage, NULL);

    if (i != -1)
      if (groupApp.PerformUtil(i, src, res)) {
	groupApp.Refresh(src_id);
        ret = TCL_OK;
      }
  }
  else
    Tcl_AppendResult(interp, (char *)usage, NULL);

  return ret;
}
