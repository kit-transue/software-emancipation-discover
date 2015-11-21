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
#ifndef _scopeMgr__h
#define _scopeMgr__h

#include <Hash.h>
#include <scopeRoot.h>
#include <objArr.h>

class projModule;

enum projScopeMode {
    PROJ_SCOPE_LESS,
    PROJ_SCOPE_FULL
};

class objNameSet : public nameHash {
public:
  Relational*lookup(const char*name) const;
  void insert(Relational*sym) { add(*(Object*) sym);}
  virtual bool isEqualObjects(const Object&, const Object&) const;


private:
  virtual const char* name(const Object*) const;
};

enum CM_COMMAND {
    CM_UNKNOWN = -1,
    CM_GET,
    CM_COPY,
    CM_PUT,
    CM_UNGET,
    CM_OBSOLETE,
    CM_DELETE,
    CM_NEW_FILE
};

class scopeMgr
{
  public:
    objArr scopes;
    objArr units;
    objNameSet clones;

    scopeUnit *slash_unit;

    projScopeMode mode;
    boolean updated_ok;

    scopeTrash *the_trash;

    void update_units ();
    void update_scopes ();

    scopeMgr();
    ~scopeMgr();

    void update (boolean force = false);
    scopeRoot *create (const char *lname, scopeRootKind knd);
    scopeRoot *find (const char *lname);

/*
    void          set_mode (projScopeMode);
    projScopeMode get_mode ();

    void set_global_scope (const char **scope_names);
    void get_global_scope (objArr&);
*/
    static int get_scopes_of (Relational * sc, objArr & result);
    void report_cm_command (CM_COMMAND, projNode *src = NULL, projNode *dst = NULL, const char *ln = NULL);
    void invalidate_unit (projNode *pn);
    void invalidate_all_units ();

    void report_pdf_reload();
    void cleanup ();
    void delete_units();
    void delete_scopes();

    scopeUnit* unit(symbolPtr& sym);
    scopeUnit* unit(projNode *);

    projModule *find_visible_module  (const char *, scopeUnit * = 0);

    void get_pmod_modules (symbolSet&, symbolArr&, projNode *, scopePmodQuery = PMOD_VISIBLE_SLASH);
    void get_pmod_modules (symbolSet&, symbolArr&, scopeNode *, scopePmodQuery = PMOD_VISIBLE_SLASH);
};

extern scopeMgr scope_mgr;
extern void scope_mgr_report_clone(scopeClone*clone);
void scopeMgr_report_cm_command ();
extern int Scope_DebugLevel;
void xref_get_all_pmod_modules (symbolSet& mod, symbolArr& modules, projNode *pn, 
				scopeNode * = NULL, int = 0);
extern void scopeMgr_get_proj_contents(projNode *, symbolArr&);

extern "C" void scopeMgr_fill_units (symbolArr &sarr);
extern "C" void scopeMgr_fill_scopes (symbolArr &sarr);
extern "C" void scopeMgr_fill_root_scopes (symbolArr &sarr, int);
extern "C" void scopeMgr_new_sll (const char *lname, symbolPtr& sym);
extern "C" void scopeMgr_new_dll (const char *lname, symbolPtr& sym);
extern "C" void scopeMgr_new_exe (const char *lname, symbolPtr& sym);
extern "C" void scopeMgr_find (const char *lname, symbolPtr& sym);
extern "C" void scopeMgr_get_scopes_of (Relational *obj, symbolArr& res_arr);
extern "C" void scopeMgr_get_scopes_in (scopeRoot *obj, symbolArr& res_arr);
extern "C" void scopeMgr_get_imported_in (scopeRoot *obj, symbolArr& res_arr);
extern "C" void scopeMgr_get_exported_in (scopeRoot *obj, symbolArr& res_arr);

#define SCOPE_TRASH -13

#endif
