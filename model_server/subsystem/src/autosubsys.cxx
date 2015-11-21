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
/*
 * autosubsys.h.C
 * 
 * key functions for handling automatic subsystem extraction
 *
 */

#include <cLibraryFunctions.h>

#ifdef _WIN32
   #include <windows.h>
#endif

#include <msg.h>
#include <machdep.h>
#include <psetmem.h>
#ifndef ISO_CPP_HEADERS
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <cstdarg>
#include <cstdlib>
using namespace std;
#include <cstdio>
#include <ctime>
#endif /* ISO_CPP_HEADERS */
#include <sys/stat.h>
#include <genString.h>
#include <customize.h>
#include <Entity.h>
#include <autosubsys.h>
#include <RTL.h>
#include <RTL_apl_extern.h>
#include <RTL_externs.h>
#include <OODT_apl_entries.h>
#include <xref.h>
#include <systemMessages.h>
#include <projList.h>
#include <path.h>
#include <proj_hash.h>
#include <extract-dialog.h>
#include <xxinterface.h>
#include <db_intern.h>
#include <StatusDialog.h>
#include <top_widgets.h>
#include <ParaCancel.h>
#include <autosubsys-weights.h>
#include <subsys_decomposition.h>
#include <externApp.h>
#include <driver_mode.h>

#ifdef index
#undef index
#endif

#include <groupTree.h>
#include <ldrSubsysMapHierarchy.h>

#include <avl_tree.h>
#include <ModelAliasList.h>

#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
static PROCESS_INFORMATION pi;
static char *shared_memory;
#endif

extern globalConnectedToNewUI;

static void StartClientNotification(const char* tit,const char* msg,int total) {
#ifdef _WIN32
static int msgAmountID;
static int msgProcessedID ;

  // Registrating user-defined messages
  msgAmountID    = RegisterWindowMessage("WM_SETAMOUNT");
  msgProcessedID = RegisterWindowMessage("WM_SETPROCESSED");

  STARTUPINFO si;
  SECURITY_ATTRIBUTES saProcess, saThread;

  ZeroMemory(&si,sizeof(si));
  si.cb=sizeof(si);
  saProcess.nLength = sizeof(saProcess);
  saProcess.lpSecurityDescriptor=NULL;
  saProcess.bInheritHandle=TRUE;

  saThread.nLength=sizeof(saThread);
  saThread.lpSecurityDescriptor=NULL;
  saThread.bInheritHandle = TRUE;

  genString title   = tit;
  genString message = msg;
  genString amount;
  amount.printf("%d",total);
  genString query;
  query.printf("CancelDialog %s -%s -%s",title, message, amount);
  ::CreateProcess(NULL,query,&saProcess,&saThread,TRUE,0,NULL,NULL,&si,&pi);
  shared_memory=(char *) ::VirtualAllocEx(pi.hProcess,NULL,1024,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
#endif
}


static bool SendClientNotification(char* msg,int total, int processed) {
#ifdef _WIN32
static int msgAmountID;
static int msgProcessedID ;

  // Registrating user-defined messages
  msgAmountID    = RegisterWindowMessage("WM_SETAMOUNT");
  msgProcessedID = RegisterWindowMessage("WM_SETPROCESSED");

  DWORD status;
  ::GetExitCodeProcess(pi.hProcess,&status);
  if(status==STILL_ACTIVE) {
      DWORD written;
      ::WriteProcessMemory(pi.hProcess,shared_memory,msg,strlen(msg),&written);
	  ::PostMessage(HWND_BROADCAST,msgAmountID,total,(LPARAM)shared_memory);
	  ::PostMessage(HWND_BROADCAST,msgProcessedID,processed,(LPARAM)shared_memory);
	  return TRUE;
  } else return FALSE;
#else
  return false;
#endif
}

static void StopClientNotification(void) {
#ifdef _WIN32
  // End command 
  ::VirtualFreeEx(pi.hProcess,shared_memory,0,MEM_RELEASE);
  ::TerminateProcess(pi.hProcess,0);
#endif
}

/********************************************************/
/*							*/
/*			Variables		*/
/*							*/
/********************************************************/

static size_t num_entities = 0;
static size_t num_processed = 0;
static size_t last_percent = 0;
static StatusDialog* sd = NULL;
genString stat_msg;

static bool         isNewUI=false;
static symbolArr*  newUIElementsScope=NULL;

static int subsystemno=0;	/* used to generate unique subsystem names */
static int min_subsys_size = 25;	// Will probably eventually be set by user

/********************************************************/
/*							*/
/*		Convenience functions			*/
/*							*/
/********************************************************/

static bool cancelled() {
   if (sd && num_entities) {
      bool result = false;
      if (num_processed == 0)
          result = sd->is_cancelled(1, stat_msg);
      else
          result = sd->is_cancelled(num_processed, stat_msg);

      if (result) {
	 delete sd;
	 sd = NULL;
	 if(globalConnectedToNewUI==0) pop_cursor();
      }
      return result;
   }
   return false;
}

//
// Yes, essentially a duplicate of printf, but easier to find
// references to. Also easier to change to reference a logfile,
// which is what this originally did anyway.

static void Log(const char *format, ...) {
  char buf[256];
  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);
  msg("$1") << buf << eom;
}

//
// Creates a unique dormant-code-auto name.

static char *make_subsystem_name() {
  char temp[32], *name;
  symbolArr* existing_group;
  do {
    sprintf(temp, "dormant-code-auto-%d", subsystemno++);
    existing_group = extGroupApp::find(temp);
  } while ( existing_group );
  name = new char[strlen(temp)+1];
  strcpy(name, temp);
  return name;
}


//
// Creates a new group out of an objArr of entities.
// might need to return new name for "VALIDATE"
void
make_subsystem(objArr *x) {
   Initialize(make_subsystem);
   if (!x->empty()) {
      char *name = make_subsystem_name();
      Log("make_subsystem: defining subsystem %s\n", name);
      symbolArr *arr = new symbolArr;
      extGroupApp::create(name, arr);
      Entity *e;
      Obj *el;
      ForEach(el, *x) {
	e = EntityPtr(el);
	arr->insert_last(e->get_xrefSymbol());
      }
      delete [] name;
   //   ReturnValue(s);
   }
 //  ReturnValue(NULL);
}

entity_name_hash_entry::~entity_name_hash_entry()
{
    Initialize(entity_name_hash_entry::~entity_name_hash_entry);

    if (next)
        delete next;
}


/********************************************************/
/*							*/
/*			allentity			*/
/*							*/
/********************************************************/

//
// allentity is a class descended from objArr that keeps
// track of the global list of entities. The allentity
// class was split out of the entlist class when it became
// apparent that they really ought to be separate.
//
// In the long run allentity should probably be turned into
// an RTL - that way we can get painless (or mostly painless) 
// UI display, selection, and whatnot.


// Some of the functions are therefore somewhat obsolete.

void allentity::add(Entity *e, const symbolPtr& sym) {
  insert_last(e);
  add_item(e, sym);
}

// The following adds a symbol alias, i.e., a mapping from the given
// xrefSymbol to an Entity that is already in the array.  It is used
// to map types to the class/enum that is the type's basetype.

void allentity::add_sym_alias(Entity* e, const symbolPtr& sym) {
   Initialize(allentity::add_sym_alias);

   add_item(e, sym);
}

// relate is a startup method which calls the relate methods of all entities.
// This establishes the network of relations between entities based on their
// underlying xref entries.

bool allentity::relate() { 
  Initialize(allentity::relate);

  size_t num_entities = size();
  stat_msg.printf(TXT("Phase 2: find relationships for %d entities"), num_entities);
  num_processed = 0;
  last_percent = 0;
  for (size_t i = 0; i < num_entities; i++) {
	 if(isNewUI==TRUE) {
		 if(num_processed%100==0) {
	         if(SendClientNotification(stat_msg,num_entities,num_processed)==FALSE) {
		         return false;
			 }
		 }
	 } else  {
         if (cancelled()) {
	         return false;
		 }
	 }
     Entity* e = EntityPtr((*this)[i]);
//>>     printf("%.5d: %s\n", i, e->get_name());fflush(stdout);
     e->relate(); 
     num_processed++;
  }
  return true;
}

//------------------------------------------
// allentity::item(const symbolPtr&)
//
// A type-safe wrapper for xrefMapper::item
//------------------------------------------

Entity* allentity::item(const symbolPtr& sym) {
   Initialize(allentity::item);
   
   return (Entity*) xrefMapper::item(sym);
}

//
// This function finds all unreferenced code and places it on the
// provided array. We do a tree search down from main (specifically,
// what find_main() returns), mark those as alive, and deem everything
// else dead. 

int allentity::getdead(symbolArr roots, objArr* dead, weight *w, int n) {
  Initialize(allentity::getdead);
  if (roots.size() == 0) {
    Log("Warning! No roots, hence no dormant code analysis performed.\n");
    ReturnValue(0);
  }
  FILE* dump_file = NULL;
  const char* dump_filename = OSapi_getenv("DIS_DORMANT_DUMP_FILE");
  if (dump_filename) {
     dump_file = OSapi_fopen(dump_filename, "w");
  }
  symbolPtr root;
  ForEachS(root, roots) {

     // First, we may need to transform the root (which is based on a
     // browser selection or a member of a batch-specified group) from
     // a reference xrefSymbol to its definition; otherwise, the item()
     // call may fail to find it, since the hashing was based on the
     // definition xrefSymbol.

     if (root.xrisnotnull()) {
	EntityPtr ent = item(root);
	if (ent) {
	   ent->setundead();
	   ent->find_live_code(w, n, 0, dump_file);
	}
     }
  }
  if (dump_file) {
     OSapi_fprintf(dump_file, "\nDORMANT:\n\n");
  }
  Obj *el;
  ForEach(el, *this) {
     if (EntityPtr(el)->isdead()) {
	dead->insert_last(el);
	if (dump_file) {
	   OSapi_fprintf(dump_file, "\t%s\n", EntityPtr(el)->get_name());
	}
     }
  }
  if (dump_file) {
     OSapi_fclose(dump_file);
  }
  ReturnValue(1);
}



void allentity::sort_filenames(int count)
{
    Initialize(allentity::sort_filenames);
}

allentity::allentity(char *n)
{
    Initialize(allentity::allentity);

    name = n;
    filename_prefix_hash = (entity_name_hash_entry **)calloc(sizeof(entity_name_hash_entry *), MAX_PATH_HASH);
    logicname_hash       = (entity_name_hash_entry **)calloc(sizeof(entity_name_hash_entry *), MAX_PATH_HASH);
    fph_count=0;
    lnh_count=0;
    filename_prefix_buf  = 0;
    logicname_buf        = 0;
}

//
// allentity deletes its contents when it's done. This guarantees that
// all items of class Entity will be gotten rid of when the analysis is
// complete.

allentity::~allentity()
{
    Initialize(allentity::~allentity);

    Obj *el;
    ForEach(el, *this) delete el;
//>>  printf("Entity cleanup complete\n");
    for (int i=0;i<MAX_PATH_HASH;i++)
    {
        if (filename_prefix_hash[i])
            delete filename_prefix_hash[i];
        if (logicname_hash[i])
            delete logicname_hash[i];
    }
    OSapi_free(filename_prefix_hash);
    OSapi_free(logicname_hash);
    if (filename_prefix_buf)
        psetfree(filename_prefix_buf);
    if (logicname_buf)
        psetfree(logicname_buf);
}

void allentity::reset_filename_hash() {
   Initialize(allentity::reset_filename_hash);

   for (size_t i = 0; i < MAX_PATH_HASH; i++) {
      if (filename_prefix_hash[i]) {
	 delete filename_prefix_hash[i];
      }
      if (logicname_hash[i]) {
	 delete logicname_hash[i];
      };
   }
   memset(filename_prefix_hash, 0, sizeof(entity_name_hash_entry*) * MAX_PATH_HASH);
   memset(logicname_hash, 0, sizeof(entity_name_hash_entry*) * MAX_PATH_HASH);
   fph_count=0;
   lnh_count=0;
   if (filename_prefix_buf) {
      psetfree(filename_prefix_buf);
      filename_prefix_buf = 0;
   }
   if (logicname_buf) {
      psetfree(logicname_buf);
      logicname_buf = 0;
   }
}


/********************************************************/
/*							*/
/*			decomposer			*/
/*							*/
/********************************************************/

//
// decomposer is the main class associated with the analysis stuff.
// 

decomposer::decomposer() :
      allentities("allentities"),
      already_inited(false),
      extraction_domainlist_valid(false) {
  weights = NULL;
  nweights = 0;
  threshold = 0;
  subsys_rtl = NULL;
  subsys_mode = AUSM_LEAVE_EXISTING_ALONE;
  projNodePtr proj;
  for (size_t i = 0; ; i++) {
     proj = projList::domain(i);
     if (proj) {
	sys_domainlist.insert_last(proj);
     }
     else break;
  }
}

//
// additems takes all the appropriate xrefSymbols, makes 
// entity instances for them, and adds the entities to the allentity
// class. 
//

/* The following declarations set up an AVL (balanced) tree for the Entity
 * nodes being created.  This allows access to the nodes, sorted by
 * ddKind, entity name, and name of defining file.  The purpose is to be
 * able to warn the user if there are multiple entities of the same name,
 * since Discover isn't necessarily able to keep references to such entities
 * straight.
 */

class entity_node;

struct entity_key: public avl_key {
   entity_key(entity_node* np, EntityPtr ep): nodep(np), entp(ep) { }
   entity_node* nodep;
   EntityPtr entp;
};

// The following struct exists only because the Sun5 SPARCworks compiler
// incorrectly complains that it can't convert the entity_key temporary
// in the entity_node constructor to a const avl_key&, so this class's
// constructor provides the conversion by using an intermediate pointer
// type.  Yuck.

struct writearound_for_sun5_compiler_bug: public avl_node {
   writearound_for_sun5_compiler_bug(avl_treeptr tp, const entity_key& k):
         avl_node(tp, *(const avl_key*) &k) { }
};

class entity_node: public writearound_for_sun5_compiler_bug {
public:
   entity_node(avl_treeptr tp, EntityPtr ep):
	        writearound_for_sun5_compiler_bug(tp, entity_key(this, ep)),
		entp(ep) { }
   void* operator new(size_t sz);
   void operator delete(void*);

   int compare(const avl_key& other) const;

   EntityPtr get_Entity() { return entp; }

   void set_aliased() const;
   bool is_aliased() { return aliased; }

   static int get_alias_count() { return alias_count; }

private:
   EntityPtr entp;
   /* mutable */ bool aliased;
   static int alias_count;
};

int entity_node::alias_count;

void* entity_node::operator new(size_t sz) {
   Initialize(entity_node::operator new);

   void* p = ::operator new(sz);
   OSapi_bzero(p, sz);
   return p;
}

void entity_node::operator delete(void* p) {
   Initialize(entity_node::operator delete);

   ::operator delete(p);
}

void entity_node::set_aliased() const {
   Initialize(entity_node::set_aliased);

   if (!aliased) {
      alias_count++;
      ((entity_node*) this)->aliased = true;
   }
}

int entity_node::compare(const avl_key& o) const {
   Initialize(entity_node::compare);

   const entity_key& other = (const entity_key&) o;
   symbolPtr my_sym = entp->get_xrefSymbol();
   symbolPtr other_sym = other.entp->get_xrefSymbol();
   int res;

   ddKind my_kind = my_sym.get_kind();
   ddKind other_kind = other_sym.get_kind();
   res = my_kind - other_kind;
   if (res == 0) {	// same kind
      res = strcmp(other_sym.get_name(), my_sym.get_name());
      if (res == 0) {	// same name
	 if (!my_sym->get_attribute(STAT_ATT, 1) &&
	     !other_sym->get_attribute(STAT_ATT, 1)) {
	    // These are model aliases -- not static, but presumably
	    // in separate files.  We will need to warn about them.
	    set_aliased();
	    other.nodep->set_aliased();
	 }
	 symbolPtr my_dfs = my_sym->get_def_file();
	 symbolPtr other_dfs = other_sym->get_def_file();
	 if (my_dfs.xrisnull()) {
	    if (other_dfs.xrisnull()) {
	       return 0;	// same symbol
	    }
	    return 1;
	 }
	 else if (other_dfs.xrisnull()) {
	    return -1;
	 }
	 res = strcmp(other_dfs.get_name(), my_dfs.get_name());
      }
   }
   return (res > 0) ? 1 : ((res < 0) ? -1 : 0);
}

class entity_iterator: public avl_iterator {
public:
   entity_iterator(const avl_tree* tp): avl_iterator(tp) { }
   entity_node* next() { return (entity_node*) avl_iterator::next(); }
};


// The next magic num is defined by trung and wmm for the size of an
// entity in the pmod
#define ENTITY_ROOM_SIZE 100


int decomposer::additems(){
  Initialize(decomposer::additems);
  size_t count = 0;	// number hashed entities

  Entity *e;
  if (!already_inited) {
     symbolPtr el;
     symbolArr els;
	 if(isNewUI==FALSE) if(globalConnectedToNewUI==0) push_busy_cursor();
		 // In the old UI we need to get our scope from gala
		 if(isNewUI==false) {
	         els.insert_last(apl_OODT_get_all_of_type(DD_CLASS)->rtl_contents());
	         els.insert_last(apl_OODT_get_all_of_type(DD_FUNC_DECL)->rtl_contents());
	         els.insert_last(apl_OODT_get_all_of_type(DD_VAR_DECL)->rtl_contents());
	         els.insert_last(apl_OODT_get_all_of_type(DD_TYPEDEF)->rtl_contents());
	         els.insert_last(apl_OODT_get_all_of_type(DD_ENUM)->rtl_contents());
	         apl_OODT_get_all_comp_gen_dtors(els);
		 } else {
		 // In our new UI we will use the variable which we will in our call function.
	         els.insert_last(*newUIElementsScope);             
		 }
    if(isNewUI==TRUE) {
	num_entities = els.size();
	stat_msg.printf("Phase 1: collect information for %d entities",num_entities);
	StartClientNotification("Dormant Code Analysis",stat_msg,num_entities);
    } else {
	if(globalConnectedToNewUI==0) pop_cursor();
	if (is_gui()) {
	    sd = new StatusDialog;
	    num_entities = els.size();
	    int popup_ret;
	    stat_msg.printf(TXT("Beginning dormant code analysis for %d entities.  Do you wish to proceed?\n "), num_entities);
	    popup_ret = sd->popup(TXT("Dormant Code Analysis"), stat_msg, num_entities);
	    if(!popup_ret) {
		delete sd;
		sd = NULL;
		return false;
	    }
	    if(globalConnectedToNewUI==0) push_busy_cursor();
	}
    }
    num_processed = 0;
    last_percent = 0;
    int entity_count = 0;
    int name_len = 0;
    Entity::set_allentities(&allentities);
    avl_tree ent_tree;
    ForEachS(el, els) {
        if(isNewUI==TRUE) {
	    if(num_processed%100==0) {
		if(SendClientNotification(stat_msg,num_entities,num_processed)==FALSE) {
		    return false;
		}
	    }
	} else {
	    if (is_gui() && cancelled()) {
		return false;
	    }
	}
        if (el.xrisnotnull()) {
	    Entity *master = item(el);
	    if (!master) {
	        e = new Entity();
	        entity_count++;
	        name_len += strlen(el.get_name());
	        e->add2(el, false);
	        e->index(num_processed++);
	        entity_node* nodep = new entity_node(&ent_tree, e);
	        if (nodep->dup()) {
		    delete nodep;
		}
	    }
        }
    }




    if (entity_node::get_alias_count()) {
	     symbolArr aliases;
	     entity_iterator it(&ent_tree);
	     entity_node* prev = NULL;
	     entity_node* p;
	     while (p = it.next()) {
	         if (p->is_aliased()) {
	             e = p->get_Entity();
	             aliases.insert_last(e->get_xrefSymbol());
	             if (prev) {
		            Entity* preve = prev->get_Entity();
		            symbolPtr e_sym = e->get_xrefSymbol();
		            symbolPtr preve_sym = preve->get_xrefSymbol();
		            if (strcmp(e_sym.get_name(), preve_sym.get_name()) == 0) {
		                preve->set_nexte(e);
		                e->set_preve(preve);
					}
				 }
	             prev = p;
			 } else prev = NULL;
		 }
		 if(isNewUI==FALSE) {
	         if (aliases.size() && is_gui()) {
	             if (!popup_ModelAliasList(aliases)) {
	                if(globalConnectedToNewUI==0) pop_cursor();
	                delete sd;
	                sd = NULL;
	                return false;
				 }
	             sd->reestablish_canceller();			
			 }
		 }
    }
    symbolPtr root;
    Xref *xref = projNode::get_home_proj()->get_xref();
    xref->make_room(entity_count * ENTITY_ROOM_SIZE + name_len + 3000000);
 } else {
     allentities.reset_filename_hash();
     num_processed = 0;
     last_percent = 0;
	 if(isNewUI==TRUE) {
         num_entities = allentities.size();
         stat_msg.printf("Phase 2: Recalculate weighted relationships for %d entities",num_entities);
		 if(SendClientNotification(stat_msg,num_entities,num_processed)==FALSE) {
			 return false;
		 }
	 } else {
         sd = new StatusDialog;
         num_entities = allentities.size();
         stat_msg.printf(TXT("Beginning extraction for %d entities.  Do you wish to proceed?\n "),num_entities);
         if (!sd->popup(TXT("Extract Subsystems"), stat_msg, num_entities)) {
	         delete sd;
	         sd = NULL;
	         return false;
		 }
         if(globalConnectedToNewUI==0) push_busy_cursor();
         stat_msg.printf(TXT("Phase 2: Recalculate weighted relationships for %d entities"),num_entities);
	 }
     Obj* o;
     ForEach(o, allentities) {
		 if(isNewUI==TRUE) {
 		     if(num_processed%100==0) {
			     if(SendClientNotification(stat_msg,num_entities,num_processed)==false) {
				     return false;
				 }
			 }
		 } else {		 
	         if (cancelled()) {
	             return false;
			 }
		 }
	     e = EntityPtr(o);
	     num_processed++;
	 }
 }
 allentities.sort_filenames(count);
 return true;
}

//
// Init builds the entity table, creates the relation network, calculates
// the global binding, and optionally dumps the entity table to 
// $ASET_ENTITY_DUMP_FILE.

int decomposer::init() {
  Initialize(decomposer::init);

  projs_with_pmods.remove_all();
  objArr projs;
  projNodePtr proj;
  size_t i;
  for (i = 0; ; i++) {
     proj = projList::domain(i);
     if (proj) {
	get_pmod_projs_from(projs, proj);
     }
     else break;
  }
  for (i = 0; i < projs.size(); i++) {
     projs_with_pmods.insert(projs[i]);
  }
  Entity::set_projects_to_search(projs_with_pmods);

  if (!additems()) {
     return 0;
  }
  if (!already_inited) {
     objArr projs;
     projNodePtr proj;
     size_t i;
     for (i = 0; ; i++) {
	    proj = projList::domain(i);
	    if (proj) {
	       projs.insert_last(proj);
		}
	    else break;
     }
     proj = projNode::get_home_proj();
     if (proj) {
	     projList::domain_add(proj->get_name());
     }
     if (proj) {	// added home proj, must now remove
	     projList::domain_reset();
	     for (i = 0; i < projs.size(); i++) {
	        proj = projNodePtr(projs[i]);
	        projList::domain_add(proj->get_name());
		 }
     }
  }
  if (allentities.empty()) {
     return 0;
  }
  if (!already_inited) {
     if (!allentities.relate()) {
	     return 0;
     }
     already_inited = true;
  }
  return 1;
}

//
// Sets the decomposer instance's internal copy of the weights to what
// we've been given.

int decomposer::setweights(weight *w, int n) { 
  Initialize(decomposer::setweights);
  int i; 
  if (!weights || (weights && n != nweights)) {
    if (weights) delete weights;
    weights = new weight[nweights=n];
  }
  for (i=0; i<n; i++) weights[i]=w[i];
  ReturnValue(1);
}

//
// Main dead-code-analysis function.
// We get an array of dead objects from allentities, and then build
// "dead-code-subsystem" around it. If "dead-code-subsystem" exists
// already, the old one is nuked.
//
#include <cmd.h>
int decomposer::builddead() {
  Initialize(decomposer::builddead);
  objArr y;
  allentities.getdead(roots, &y, weights, nweights);
  if (!y.empty()) {
     make_subsystem(&y);
     //s = make_subsystem(&y);
  }
  roots.removeAll();	// don't repeat after undo
#if 0
  VALIDATE{
     objSet os(&y);
     cmd_validate((s) ? s->get_name() : "dormant-code", &os);  
  }
#endif

 return 1;
}

//
// Do dead-code analysis.
//
int decomposer::go_deadcode_only(symbolArr& new_roots, weight *weights_loc,
                                 int nweights_loc, bool newUI, symbolArr* elScope) {
  isNewUI=newUI;
  newUIElementsScope=elScope;
  Initialize(decomposer::go_deadcode_only);
  setweights(weights_loc, nweights_loc);
  roots = new_roots;
  if(roots.empty()) {
    msg("ERROR: No roots selected for dormant code detection.") << eom;
    ReturnValue(0);
  }
  if(!init()) ReturnValue(0);
  builddead();
  if(isNewUI==TRUE) {
     StopClientNotification();
  } else {
     if (is_gui() && sd) {
        delete sd;
        sd = NULL;
        if(globalConnectedToNewUI==0) pop_cursor();
     }
  }
  ReturnValue(1);
}

//------------------------------------------
// decomposer::item(const symbolPtr&)
//------------------------------------------
Entity* decomposer::item(const symbolPtr& sym) {
   Initialize(decomposer::item);
   
   return allentities.item(sym);
}

//------------------------------------------
// decomposer::use_sys_domainlist()
//------------------------------------------

void decomposer::use_sys_domainlist() {
   Initialize(decomposer::use_sys_domainlist);

   projList::domain_reset();
   Obj* o;
   ForEach(o, sys_domainlist) {
      projNodePtr proj = checked_cast(projNode, o);
      projList::domain_add(proj->get_name());
   }
}

//------------------------------------------
// decomposer::set_extraction_domainlist(symbolArr&);
//------------------------------------------

void decomposer::set_extraction_domainlist(symbolArr& selected_projects) {
  Initialize(decomposer::set_extraction_domainlist);
  
  if(selected_projects.size() > 0) {
    extraction_domainlist = selected_projects;
    //extraction_domainlist.removeAll();
    //symbolPtr sp;
    //ForEachS( sp, selected_projects ) {
    //   extraction_domainlist.insert_last((Obj *)sp);
    //}
    extraction_domainlist_valid = true;
  }
}

#if 0
groupHdr *get_group_by_name(const char *name)
{
    Initialize(get_group_by_name);

    ddElementPtr def = dd_get_def(DD_SUBSYSTEM, (char*) name);
    if (def) {
       return groupHdr::sym_to_groupHdr(def->get_xrefSymbol());
    }

    return 0;
}
#endif
