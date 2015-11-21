service DIScm


!#ifdef SERVER
!#include "gRTListServer.h"
!#include "messages.h"
!
!#include "galaxy_undefs.h"
!#include "include/ui_cm_oper.h"
!#include "include/ui_cm_get.h"
!#include "include/ui_cm_unget.h"
!#include "include/ui_cm_put.h"
!#include "include/ui_cm_lock.h"
!#include "include/ui_cm_unlock.h"
!
!#endif



@@ dis_cm_operation
{    // Args: operation rtlServerId.

        if (argc >= 3) {
		// Get the arguments.
		const char* cm_oper_name = argv[1];
		const char* cm_rtl_id = argv[2];
		int rtlId = atoi(cm_rtl_id);

		// Get array of file symbols.
		symbolArr file_symbols(0);
		RTListServer* fileRTL = 0;
		if (rtlId > 0) {
			fileRTL = RTListServer::find(rtlId);
			if (fileRTL) fileRTL->getSelArr(file_symbols);
		}

		// See if this operation is during a put.
		bool duringPut = false;
		if ( (argc >= 4) && ui_cm_put::PutAlreadyInProgress ) {
			duringPut = true;
		}

		ui_cm_oper* cm_oper = NULL;

		if (strcmp(cm_oper_name, "-check_out") == 0) {
			cm_oper = new ui_cm_get("Check Out");
		} else if (strcmp(cm_oper_name, "-check_in") == 0) {
			if (ui_cm_put::PutAlreadyInProgress)
				dis_message(NULL, MSG_ERROR,
				"You cannot check in any more files until the current check in is completed.");
			else 
				cm_oper = new ui_cm_put("Check In", fileRTL);
		} else if (strcmp(cm_oper_name, "-undo_check_out") == 0) {
			cm_oper = new ui_cm_unget("Un-Check Out");
		} else if (strcmp(cm_oper_name, "-lock") == 0) {
			cm_oper = new ui_cm_lock("Lock");
		} else if (strcmp(cm_oper_name, "-unlock") == 0) {
			cm_oper = new ui_cm_unlock("Unlock");
		} else {
			dis_message(NULL, MSG_ERROR,
				"'%s' is an unknown CM operation.", cm_oper_name);
		}

		// Perform the operation on the modules.
		if (cm_oper) {
			cm_oper->CheckPreConditions(file_symbols);

			while (cm_oper->AnyModulesToProcess() ) 
				cm_oper->ProcessNextModule();

			cm_oper->CheckPostConditions();

			delete cm_oper;

			if (duringPut) {
				// Alright, this is a triffle kludgy.
				// For PutStatus operations we will assume that
				// all of the selected files were successful.
				symbolArr& putSyms = fileRTL->getArr();
				symbolPtr sym;
				ForEachS(sym, file_symbols) {
					putSyms.remove(sym);
				}
				fileRTL->clear();
				fileRTL->insert(putSyms);
				fileRTL->_propagate();

				// Also need to update the project contents.
				if (ui_cm_put::HomeProjectRTL) {
					int result = 1;
					const char * title = ui_cm_put::HomeProjectRTL->getTitle();
					ui_cm_put::HomeProjectRTL->queryProjects(
						(char*) title, &result);
				}
				

			} else if (fileRTL) {
				// Need to update the project contents.
				int result = 1;
				fileRTL->queryProjects((char*)fileRTL->getTitle(), &result);
			}
		}

        } else {
		dis_message(NULL, MSG_ERROR,
			"dis_cm_oper_name:%d args found instead of 3.", argc);
        }

    return TCL_OK;
}



@@ dis_cm_get
{    // Args: getOperation rtlServerId.

        if (argc == 3) {
		// Get the arguments.
		const char* cm_get_name = argv[1];
		int rtlId = atoi(argv[2]);

		// Get array of file symbols.
		RTListServer* fileRTL = 0;
		if (rtlId > 0) {
			fileRTL = RTListServer::find(rtlId);
			if (fileRTL) {
				if (strcmp(cm_get_name, "-failures") == 0) {
					fileRTL->clear();
					fileRTL->insert(ui_cm_put::FailedModules);
				} else if (strcmp(cm_get_name, "-failureType") == 0) {
					symbolArr selection(0);
					fileRTL->getSelArr(selection);
					genString type("");
					ui_cm_put::GetFailureType(selection, type);
					Tcl_SetResult(interp, type, TCL_VOLATILE);
				} else {
					dis_message(NULL, MSG_ERROR,
						"'%s' is an unknown CM get command.", cm_get_name);
				}
			} else {
				dis_message(NULL, MSG_ERROR,
					"'%d' is an unknown RTL id.", rtlId);
			}
		} else {
			dis_message(NULL, MSG_ERROR,
				"'%s' is an unknown RTL id.", argv[2]);
		}
        } else {
		dis_message(NULL, MSG_ERROR,
			"dis_cm_get:%d args found instead of 3.", argc);
        }

    return TCL_OK;
}




@@ dis_cm_set
{    // Args: setOperation 

        if (argc == 2) {
		// Get the arguments.
		const char* cm_set_name = argv[1];

		if (strcmp(cm_set_name, "-putDone") == 0) {
			// If any symbols are in the failure array,
			// Move them back to the HomeProjectRTL.
			if (ui_cm_put::FailedModules.size() > 0) {
//				HomeProjectRTL->insert(ui_cm_put::FailedModules);
				// Also need to update the project contents.
				if (ui_cm_put::HomeProjectRTL) {
					int result = 1;
					const char * title = ui_cm_put::HomeProjectRTL->getTitle();
					ui_cm_put::HomeProjectRTL->queryProjects(
						(char*) title, &result);
				}
			}
			ui_cm_put::PutAlreadyInProgress = false;
		} else {
			dis_message(NULL, MSG_ERROR,
			"'%s' is an unknown CM set command.", cm_set_name);
		}
        } else {
		dis_message(NULL, MSG_ERROR,
			"dis_cm_set:%d args found instead of 2.", argc);
        }

    return TCL_OK;
}

