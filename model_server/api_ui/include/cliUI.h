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
#ifndef _cliUI
#define _cliUI

#include "Relational.h"
#include "objArr.h"
#include "objDictionary.h"
#include "browserShell.h"
#include "tcl.h"

class Interpreter;

struct cliQueryData : public objDictionary {
  cliQueryData(const char*label) : objDictionary(label) {}
  Interpreter  *interpreter;
  genString    query_command;
  int  selection_opt;
  int  result_opt;
  define_relational(cliQueryData,objDictionary);
  virtual void	print(ostream& = cout, int level = 0) const;
};

struct cliMenu : public objDictionary {
  objArr items;
  int  selection_opt;
  int  result_opt;
  cliMenu(const char*tag) : objDictionary(tag),
            selection_opt(0), result_opt(0) {}
  cliMenu(const char*tag, int so, int ro) : objDictionary(tag), 
            selection_opt(so), result_opt(ro) {}
  define_relational(cliMenu,objDictionary);
  virtual void	print(ostream& = cout, int level = 0) const;
};

defrel_many_to_one(browserQueryMenu,browserQueryMenu,cliMenu,cliMenu);

struct cliMenuArr : public objArr {
  cliMenu* operator[](int ind) { return (cliMenu*) objArr::operator[](ind);}
};

extern cliMenuArr * cli_menus;

typedef enum { 
  MENU_ITEM_UNKNOWN,     // Uninitialized
  MENU_ITEM_ACTION,      // Has some user-defined action (callback) associated with it
  MENU_ITEM_SUBMENU,     // A cascading menu
  MENU_ITEM_TOPMENU,     // Top-level menu
  MENU_ITEM_SEPARATOR    // Visual separator
} cliMenuItemCode;

struct cliMenuItem : public objDictionary {
  
  cliMenuItem(const char *tag) : objDictionary(tag), mtype(MENU_ITEM_UNKNOWN), menu(NULL) {}
  cliMenuItem(const char *tag, cliMenuItemCode mt) : objDictionary(tag), mtype(mt), menu(NULL) {}
  cliMenuItem(const char *tag, cliMenuItemCode mt, cliMenu *m) : objDictionary(tag), mtype(mt), menu(m) {}

  cliMenuItemCode mtype;

  cliMenu *menu;

  define_relational(cliMenuItem, objDictionary);
  virtual void print(ostream& = cout, int level = 0) const;
};

defrel_one_to_one(cliMenuItem, cliMenuItem, cliQueryData, cliQueryData);

class genString;
int cli_instance_hook(symbolPtr&, symbolArr&, symbolArr&, genString&, 
	genString&);
int call_cli_callback(Interpreter *, const char *, const char *, 
	symbolArr *, symbolArr *);

int GetSmtCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtOffsetCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtNativeTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetFirstCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetLastCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetNextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetPrevCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetRootCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtGetParentCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtTypeCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtExTypeCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtReplaceTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtInsertTextAfterCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int SmtInsertTextBeforeCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionCreateCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionNativeTextCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionCopyCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionCutCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionPasteAfter(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);
int RegionPasteBefore(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);

extern bool valid_menu_hierarchy(const char *);
extern void menu_child_name(const char *, genString &);
extern void menu_parent_name(const char *, genString &);

void SetNextRtl(gtRTL*);
gtRTL* GetNextRtl();

void AddCategory(const char* name, const char* command);
void AddQuery(const char* name, const char* command);
void RemoveCategory(const char* name);
void RemoveQuery(const char* name);

void SetResults(const symbolArr& syms);
void SetElements(const symbolArr& syms);
void SetNextRtl(const symbolArr& syms);

int BrowserCmd(ClientData cd, Tcl_Interp *interp, int argc, char *argv[]);

#endif



