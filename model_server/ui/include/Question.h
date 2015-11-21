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
#ifndef _Question_h
#define _Question_h

#include <xref.h>
#include "genString.h"

// Question.h
//------------------------------------------
// synopsis:
// Simple OK/Cancel dialog.
//------------------------------------------

extern "C" int popup_Question(const char* title, const char* question, int defalt);

int popup_Question(const char* title, const char* question,
		   const char* OK_label, const char* Cancel_label);
int popup_Question(const char* title, const char* question,
		   const char* OK_label, const char* OK2_label, const char* Cancel_label);

typedef const char *questionText;
typedef const char *questionButton;
typedef const char *questionTitle;
typedef const char *questionContext;

int dis_question (questionTitle, questionButton, questionButton, questionText ...);
int dis_question3 (questionContext, questionTitle, 
		   questionButton, questionButton, questionButton, questionText ...);
int dis_confirm_list(questionTitle, questionButton, questionButton,
                     const symbolArr&, questionText ...);

// Offer to save files associated with the list of objects.
int dis_confirm_save_objects(const objArr&);

int dis_wait_filename( genString &filename, char *title );

// Regardless of the button label, the confirm-dialogs in the gala clients return
// Left, Middle, or Right strings if there is >1 button in the dialog.
// These strings are sent from the client, so be very carefull about changing them.
#define B_LEFT_SELECTED		"LeftButton"
#define B_MIDDLE_SELECTED	"MiddleButton"
#define B_RIGHT_SELECTED	"RightButton"


#define B_CANCEL		"Cancel"
#define B_CHECKOUT		"Checkout"
#define B_CONTINUE		"Continue"
#define B_COWITHOUTSAVE		"Checkout-Without Save"
#define B_DELETE                "Delete"
#define B_HELP			"Help"
#define B_LOADAUTOSAVE          "Load Autosave"
#define B_LOADSOURCE            "Load Source"
#define B_MERGE                 "Merge"
#define B_NO			"No"
#define B_OK			"OK"
#define B_RENAME                "Rename"
#define B_SAVERENAMECO		"Save-Rename-Checkout"
#define B_STOP			"Stop"
#define B_YES			"Yes"

#define CQ_ALREADYLOADED	"Pset.Help.Get.AlreadyLoaded"
#define CO_GROUPSSWITCH         "Pset.Help.Groups.Switch"
#define CQ_AUTOSAVEEXISTS	"Pset.Help.Load.AutosaveExists"
#define CQ_MERGEPSETQ		"Pset.Help.Errors.Merge.LoadGetPset"
#define CQ_CHANGEARGSISKNR	"Pset.Help.ChangeArgs.IsItKandRC"
#define CQ_CHANGEPROPUNPARSED	"Viewer.Edit.ChangeProp.Unparsed"

#define Q_SWITCHTOSCAN          "Browser must be in Scan mode. Switch to Scan?"
#define Q_OPENINGMANYVIEWS	"You have selected %d items.\nOpening a large number of views may be\nexpensive in both time and system resources.\nAre you sure you wish to continue?"
#define Q_OBJECTALREADYEXISTS	"Object %s already exists.\nWish to create another one with same name?\n"
#define Q_QUITPARASET		"Quit?"
#define Q_ACTIONDESTROYSASSOC	"This action destroys the entire association:\n\n%s\n\nTHIS CANNOT BE UNDONE!\n"
#define Q_ACTIONREMOVESALLASSOC	"This action removes all instances of the association:\n\n%s\n\nfrom the file: \n\n%s\n\nTHIS CANNOT BE UNDONE!\n"
#define Q_CANTRECOGNIZECOMPILE2	"Cannot recognize compiler name for %s"
#define Q_CANTRECOGNIZECOMPILER	"Cannot recognize compiler name for %s :\n%s\n"
#define Q_CHANGEPROCEED		"The file %s has been changed.\nDo you want to proceed with \"%s\" ?"
#define Q_CLASSEXISTS		"%s %s already exists.\nWish to create another one with same name?\n"
#define Q_FILECHANGEPROCEED	"The %s %s has been changed. Do you want to proceed with \"%s\"?"
#define Q_FILEISTHEPRIMARYFILE	"The file:\n\n%s\n\nis the primary file for the association:\n\n%s\n\nRemoving the association from this file\nwill cause the association to be destroyed.\n\nTHIS CANNOT BE UNDONE!\n"
#define Q_FILEWRITEPROTECTED	"File %s is write protected.\nTry to save anyway?\n"
#define Q_LOADAUTOSAVE		"The autosave file for \"%s\" is newer than the source. Do you want to load it?"
#define Q_MODELISLOCKED		"The Information Model \"%s\" is locked for writing\nTry again?"
#define Q_NOTALLSELSYMBSPRESENT	"Not all selected symbols present in the LATEST model!\n\nDo you want to continue move?"
#define Q_PSETOUTOFSYNCMD	"The pset file \"%s\" is out of sync.\nYou can merge it with the source file or delete it."
#define Q_PSETOUTOFSYNCMDR	"The pset file \"%s\" is out of sync.\nYou can merge it with the source file, delete it, or rename it."
#define Q_SAVETHENRENAME	"File %s was modified. Do you want to save it then rename it?\nIt will be unloaded and then checked out and reloaded."
#define Q_UNABLETOOPENMODEL	"Unable to open the Information Model: %s \nERROR: \"%s\"\n%s\nTry Again?"
#define Q_UNLOADTHENREALOAD	"File %s will be unloaded and then checked out and reloaded."
#define Q_SELECTEDINVALIDITEM	"You have selected an invalid item.\nIt will be ignored."
#define Q_FOLLOWINGITEMISINVALD "The following item:\n\n%s\n\nis invalid for the selected link type.\nIt will be ignored."
#define Q_STR			"%s"
#define Q_ABANDONNOWMAYCORRUPT	"Abandoning now may cause source code inconsistencies. \nHit Cancel to either undo or continue propagation."
#define Q_PROTOTYPEFORDECL	"%s\n\nFile: %s Line Number: %d\nPrototype for this declaration?"
#define Q_ISKANDRC		"%s\n\nFile: %s Line Number: %d\nIs this definition K&R C?"

#define T_CHANGEPROP		"Change Propagation"
#define Q_STE_VIEW_REFRESH      "View of %s is out of sync. OK to refresh ?"

#define Q_WANT_TO_QUIT          "Do you want to exit DIScover?"

#define T_COPYMODULE	        "Copy Module"
#define T_CREATEWARNING		"Create Warning"
#define T_GETMODULE		"Get Module"
#define T_GETOLDMODULE		"Get Old Module"
#define T_LOADAUTOSAVE		"Load Autosave Copy"
#define T_MERGEFOR		"Merge for %s"
#define T_MODELMISMATCH		"Model Mismatch"
#define T_NULL			""
#define T_SAVE			"Save"
#define T_UNLOADINGFILE		"Unloading File"
#define T_CHANGEARGUMENT	"Change Argument"
#define T_RENAMEWARN		"Rename Warning"
#define T_REMOVELINK		"Remove Link"
#define T_DELETEGROUP		"Delete Group"
#define T_TOOMANYVIEWS		"Too Many Views"
#define Q_DELETEGROUP		"OK to delete group '%s'?"
#define Q_REMOVELINK		"Remove the link for \"%s\"?"

#define CQ_LINKEXISTS		"Pset.Help.Get.LinkExists"

#define B_CHANGEANYWAY		"Change Anyway"
#define T_CHANGELANG		"Change Language"
#define Q_LOSEASSOCCHANGE	"You will lose any manual associations for %s"

#endif // _Question_h
