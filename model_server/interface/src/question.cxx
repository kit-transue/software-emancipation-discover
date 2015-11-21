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
// question.C
#if defined(NEW_UI)
#include <vport.h>
#include vstrHEADER
#include "../../DIS_ui/interface.h"
#include "galaxy_undefs.h"
extern Application* DISui_app;

#else    // Old UI
#include "Question.h"
#include "gtPushButton.h"
#include "gtQuestion.h"
#include "FileSelDlg.h"
#endif   // New UI.

#include "cLibraryFunctions.h"
#include "iQuestion.h"
#include "genString.h"
#include "messages.h"
#include "genError.h"
#include "driver_mode.h"
#include "SocketClient.h"

#if defined(NEW_UI)
#include <db.h>
#endif

extern globalConnectedToNewUI;
static genString vqst;

void vquestion(const char* fmt, va_list ap) 
{
    vqst.vsprintf(fmt, ap);
}

int  dis_question (questionTitle 	title, 
                   questionButton	OK_label, 
                   questionButton	Cancel_label, 
                   questionText 	question ...) 
{
    //if not in gui mode, do not popup question
    if (!is_gui())
      return 0;
    
    va_list ap;
    va_start(ap, question);
    vquestion(question, ap);
    va_end(ap);

#ifndef NEW_UI
    return popup_Question (title, vqst, OK_label, Cancel_label);
#else
    vstr* str = vstrClone ((vchar*)(char*)vqst);

    if ( (OK_label == NULL) || (OK_label[0] == '\0') )
       OK_label = B_OK;
    if ( (Cancel_label == NULL) || (Cancel_label[0] == '\0') )
       Cancel_label = B_CANCEL;

	if(globalConnectedToNewUI==0) {
        genString command;
        command.printf("dis_confirm2 {%s} {%s} {%s} {%s}",
                        title, OK_label, Cancel_label, str);
        vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
        vstrDestroy (str);

         if (strcmp ((char *)results, B_LEFT_SELECTED) == 0) return 1;
	} else {
		genString notification_string;
		if(strlen(title)>0) {
		    notification_string.printf("question\3-title\3%s\3%s\3%s\3%s\3",title,str,OK_label,Cancel_label);
        } else {
		    notification_string.printf("question\3-title\3Discover\3%s\3%s\3%s\3",str,OK_label,Cancel_label);
		}
		char* result = send_client_notification(notification_string);
        if (strcmp (result, OK_label) == 0) return 1;
	}
	return 0;
#endif

}

int  dis_question3 (questionContext	context,
		    questionTitle 	title, 
                    questionButton	Left_label, 
                    questionButton	Middle_label, 
                    questionButton	Right_label, 
                    questionText 	question ...) 
{
    //if not in gui mode, do not popup question
    if (!is_gui())
      return 0;
  
    // Init vqst.
    va_list ap;
    va_start(ap, question);
    vquestion(question, ap);
    va_end(ap);

#ifndef NEW_UI
    if (context) gtPushButton::next_help_context (context);
    return popup_Question (title, vqst, Left_label, Middle_label, Right_label);
#else
    vstr* str = vstrClone ((vchar*)(char*)vqst);

    if ( (Left_label == NULL) || (Left_label[0] == '\0') ) Left_label = B_YES;
    if (Middle_label == NULL) Middle_label = "";
    if ( (Right_label == NULL) || (Right_label[0] == '\0') ) Right_label = B_CANCEL;

     int int_result = 0;
	if(globalConnectedToNewUI==0) {
        genString command;
        command.printf("dis_confirm3 {%s} {%s} {%s} {%s} {%s}",
                        title, Left_label, Middle_label, Right_label, str);
        vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
        vstrDestroy (str);

        if (strcmp ((char *)results, B_LEFT_SELECTED) == 0)
            int_result = 1;
        else if (strcmp((char*) results, B_MIDDLE_SELECTED) == 0)
            int_result = 2;

	} else {
		genString notification_string;
		if(strlen(title)>0) {
		    notification_string.printf("question\3-title\3%s\3%s\3%s\3%s\3",title,str,Left_label,Middle_label,Right_label);
        } else {
		    notification_string.printf("question\3-title\3Discover\3%s\3%s\3%s\3",str,Left_label,Middle_label,Right_label);
		}
		char* result = send_client_notification(notification_string);
        if (strcmp (result, Left_label) == 0)   int_result = 1;
        if (strcmp (result, Middle_label) == 0) int_result = 2;
	}
    return (int_result);
#endif
}

int  dis_confirm_list (questionTitle 	title, 
                       questionButton	OK_label, 
                       questionButton	Cancel_label, 
                       const symbolArr&	confirmData, 
                       questionText 	question ...) 
{
        
    //if not in gui mode, do not popup question
    if (!is_gui())
      return 0;
    
    va_list ap;
    va_start(ap, question);
    vquestion(question, ap);
    va_end(ap);

#ifndef NEW_UI
    return popup_Question (title, vqst, OK_label, Cancel_label);
#else
    vstr* str = vstrClone ((vchar*)(char*)vqst);

    if ( (OK_label == NULL) || (OK_label[0] == '\0') ) OK_label = B_OK;
    if ( (Cancel_label == NULL) || (Cancel_label[0] == '\0') ) Cancel_label = B_CANCEL;

    // Create text list of names.
    genString listOfNames = "{ ";
    symbolPtr item;
    ForEachS(item, confirmData) {
        genString thisName;
        thisName.printf(" { %s }", item.get_name() );
        listOfNames += thisName;
    }
    listOfNames += " }";

    genString command;
    command.printf("dis_confirm_list {%s} {%s} {%s} {%s} {%s}",
                    title, OK_label, Cancel_label, str, (char*)listOfNames);

    vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
    vstrDestroy (str);

    if (results && (strcmp((char *)results, B_LEFT_SELECTED) == 0) )
        return 1;
    else
        return 0;
#endif
}

// Ask user whether or not to save modified files/objects.
int dis_confirm_save_objects(const objArr& objects) {

	Initialize(dis_confirm_save_objects(const objArr&));

	int answer = 0;

#if defined(NEW_UI)
dis_message(NULL, MSG_INFORM, "%d objects to save.", objects.size() );

#if defined(LET_USER_CONFIRM_THE_LIST)
	// Create text list of names.
	genString listOfNames = "{ ";

	ObjPtr object;
	ForEach(object, objects) {
		appPtr app_item = checked_cast(app, object);
		if (app_item) {
			genString thisName;
			thisName.printf(" { %s }", app_item->get_filename() );
			listOfNames += thisName;
		}
	}

	listOfNames += " }";

	genString command;
	command.printf("dis_confirm_list {%s} {%s} {%s} {%s} {%s}",
			"Save DISCOVER Model Info to Disk", "OK", "Cancel",
			"These files need to be saved to make your changes pernament",
			(char*)listOfNames);

	vstr *results = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());

	if (results && (strcmp((char *)results, B_LEFT_SELECTED) == 0) )
	    answer = 1;

#else    // LET_USER_CONFIRM_THE_LIST
	answer = 1;
#endif    // LET_USER_CONFIRM_THE_LIST


// Take each item in the list and save it.

	ObjPtr object;
	ForEach(object, objects) {
		appPtr app_item = checked_cast(app, object);
		if (app_item) {
			if (db_save(app_item, NULL) ) {
				projModule* module = appHeader_get_projectModule(app_item);
				if (module) module->invalidate_cache();
				app_item->clear_modified();
				app_item->clear_imported();
			} else {
				dis_message(NULL, MSG_ERROR,
					"M_ERRORWHILESAVINGFILE",
					app_item->get_filename() );
			}
		}
	}


#endif    // New UI.
	return (answer);
}

int dis_wait_filename( genString &filename, char *title )
{
  int nRet = 0;
#ifndef NEW_UI
  FileSelDlg file(NULL, title);
  FileSelCode status = file.WaitFileName(filename);
  nRet = (status == FILE_SEL_OK );
#else
  Application *appl = Application::findApplication("DISui");
  if( appl && appl->mySession() )
  {
    char *pszResult = rcall_dis_choose_file_simple( appl, title );
    if( pszResult && strlen( pszResult ) )
    {
      filename = pszResult;
      nRet = 1;
    }
  }
#endif /*!NEW_UI*/
  return nRet;
}

#ifndef NEW_UI
extern "C" int popup_Question(const char* title, const char* question, int defalt)
{
    Initialize(popup_Question);

    int answer = defalt;

    gtQuestion q(title, question, defalt);
    answer = q.ask();

    return (answer);
}


int popup_Question(const char* title, const char* question,
		   const char* OK_label, const char* Cancel_label)
{
    Initialize(popup_Question);

    int answer = -1;

    gtQuestion q(title, question, OK_label, Cancel_label);
    answer = q.ask();

    return (answer);
}

int popup_Question(const char* title, const char* question,
		   const char* OK_label, const char* OK2_label, const char* Cancel_label)
{
    Initialize(popup_Question);

    int answer = -1;

    gtQuestion q(title, question, OK_label, OK2_label, Cancel_label);
    answer = q.ask();

    return (answer);
}
#endif


//----------     end of Question.C     ----------//
