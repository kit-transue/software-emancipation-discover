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
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#ifndef ISO_CPP_HEADERS
#include <errno.h>  /* Required for Sun5, not for Sun4 */
#else /* ISO_CPP_HEADERS */
#include <cerrno>
#endif /* ISO_CPP_HEADERS */

#include "XrefQuery.h"
#include "_Xref.h"
#include "api_internal.h"
#include "autosubsys.h"
#include "cmd.h"
#include "format.h"
#include "genArr.h"
#include "genMask.h"
#include "genString.h"
#include "genTry.h"
#include "genWild.h"
#include "scopeClone.h"
#include "scopeRoot.h"
#include "symbolScope.h"
#include "symbolSet.h"
#include "xref_queries.h"

int tree_get_parent(const symbolPtr&, symbolPtr&);
int tree_get_next(const symbolPtr&, symbolPtr&);
int tree_get_previous(const symbolPtr&, symbolPtr&);
int tree_get_first(const symbolPtr&, symbolPtr&);

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 
extern void scope_get_modules(scopeNode *sc, objArr& modules, int depth); 

typedef int * intp;
typedef char * charp;


#define RgenArr(TYPE)							      \
class genArrOf(TYPE)							      \
{									      \
  protected:								      \
    unsigned int capacity;						      \
    unsigned int length;						      \
    TYPE *   ptr;TYPE nullval;							      \
									      \
  public:								      \
    genArrOf(TYPE)() : ptr(0), length(0), capacity(0) {}		      \
    genArrOf(TYPE)(int sz) : ptr(0), length(0), capacity(0) {		      \
	provide(sz);							      \
    }									      \
    ~genArrOf(TYPE)() { if (ptr) psetfree(ptr); }				      \
									      \
    void reset() { length = 0; }					      \
    unsigned size() const { return length; }				      \
    TYPE * grow(int ii = 1) {						      \
	int ol = length;						      \
	length += ii;							      \
	provide(length);						      \
	return ptr + ol;						      \
    }									      \
    TYPE * append(TYPE * el) {						      \
	TYPE * pp = grow(1);						      \
	OS_dependent::bzero((char*)pp, sizeof(TYPE));					      \
	*pp = *el;							      \
	return pp;							      \
    }									      \
    void remove (int ind) { shrink(ind, 1); }				      \
    void shrink(int ind,  int len) {					      \
	int new_length = length - len;					      \
	int ii, jj;							      \
	for(ii=ind, jj=ind+len; ii<new_length; ++ii, ++jj)		      \
	    ptr[ii] = ptr[jj];						      \
	length = new_length;						      \
    }									      \
    TYPE& operator[](int ii) {					      \
	return (ii < 0 || ii >= length) ? nullval :ptr[ii]; }		      \
    void provide(int len) {						      \
	if(len > capacity) {						      \
	    if(capacity == 0)						      \
		capacity = 1;						      \
	    while(capacity < len)					      \
		capacity *= 2;						      \
	    ptr = (TYPE *) (ptr ? psetrealloc((char*)ptr, capacity * sizeof(TYPE)) :     \
			    psetmalloc(capacity * sizeof(TYPE)));		      \
	}								      \
    }									      \
}

int global_error;

void reperr(void)
{
    Initialize(reperr);
    global_error=-1;
    /*   Error(ERR_FAIL);*/
}

class clientMemMan
{
  public:
    RgenArr(charp);
    genArrOf(charp) base;


    int number_of_clients;


    int deleteClient(int client);
    int deleteEntity(int client,int pos);
    int newEntityFor(int client);
    int insertNewClient();
    clientMemMan();
    symbolArr *getval(int client,int pos);

    
};

clientMemMan::clientMemMan(){
  for(int i=0;i<base.size();i++)(base[i])=NULL;
  number_of_clients=0;
}

    int clientMemMan::insertNewClient(){
	for(int i=0;i<number_of_clients;i++)if(!base[i])
	{
	    base[i]=(char *)new charpArr(100);
	    int j;for(j=0;j<100;j++)(*((charpArr *)base[i]))[j]=NULL;
	    number_of_clients ++;
	    return(i);
	}

	int v1=base.size();
	int v2=number_of_clients;

	if(v2>=v1)
	{
	    base.grow(base.size()+1);

	}
	base[number_of_clients]=(char *)new charpArr(100);
	int j;for(j=0;j<100;j++)(*((charpArr *)base[number_of_clients]))[j]=NULL;
	number_of_clients ++;
	return(number_of_clients-1);
    }

    int clientMemMan::newEntityFor(int client){
	if(client >= number_of_clients ||  base[client]==NULL)
	{
	    reperr();
	    return(-1);
	}
	
	charpArr * this_array;
	this_array=(charpArr *)base[client];
	int pos=-1;

	for(int i=0;i<this_array->size();i++)
	    if((*this_array)[i]==NULL)
	    {
		pos=i;
		break;
	    }

	if(pos<0)
	{
	    pos=this_array->size();
	    this_array->grow(this_array->size()+1);
	    for(int i=pos;i<this_array->size();i++)(*this_array)[i]=NULL;
	}

	(*this_array)[pos]=(char *)new symbolArr;
	return(pos);
    }

	
    int clientMemMan::deleteEntity(int client,int pos){
	if(client>=base.size()){reperr();return -1;}
	if(base[client]==0){reperr();return -1;}

	symbolArr * sadel;
	if(pos>=(*((charpArr *)base[client])).size()){reperr();return(-1);}
        sadel=(symbolArr *)(*((charpArr *)base[client]))[pos];
	if(sadel==NULL){reperr();return(-1);}

	(*((charpArr *)base[client]))[pos]=NULL;
	delete(sadel);
	
	return 0;
    }

    int clientMemMan::deleteClient(int client){
	if(client>=base.size()){reperr();return(-1);}
	if(base[client]==NULL){reperr();return(-1);}
	
	for(int i=0;i<base.size();i++)
	{
	    if((*((charpArr *)base[client]))[i])deleteEntity(client,i);
	}
	
	delete((charpArr *)base[client]);
	base[client]=NULL;
	number_of_clients--;
	return(0);
    }


    symbolArr * clientMemMan::getval(int client,int pos){
	if(client>=base.size()){return NULL;}
	if(base[client]==NULL){return NULL;}
	if((*((charpArr *)base[client])).size()<=pos){return NULL;}
	symbolArr * sa;
	sa=(symbolArr *)(*((charpArr *)base[client]))[pos];
	if(sa==NULL){return NULL;}
	
	return sa;
    }




clientMemMan cmm;

class symman: public clientMemMan
{
  public:
    int create(int client,int &object);
    int destroy(int client,int &object);
    int append(int client,int objecttarg, int objectsrc);
    int copy(int client,int objecttarg, int objectsrc);
    char *get_name(int client,int object);
    int get_kind(int client,int object);
    int look_up(int client,int object,int kind, char * name,int def_file);
    int size(int client,int object);
    int set(int client,int objecttarg,int objectsrc,int position);
    int get(int client,int objectfrom,int objectto,int position);
    int set_add(int client, int objecttarg,int objectsrc);
    int set_subtract(int client,int objecttarg,int objectsrc);
    int set_intersect(int client,int objecttarg,int objectsrc);

};
#define clref(x) (*getval(client,x)) 

symman smm;

static int current_client=newClient(); // The client being serviced now ...

//-----------------------------------------------------------------

void UpdateDISetProc(Tcl_Obj *objPtr)
{
    objPtr->bytes = Tcl_Alloc(10);
    sprintf(objPtr->bytes, "_DI_%d", (int)objPtr->internalRep.longValue);
    objPtr->length = strlen(objPtr->bytes);

}

void DupDISetProc(Tcl_Obj * /* srcPtr */, Tcl_Obj * /* dupPtr */)
{
    printf("DupDISetProc not implemented\n");
}


void FreeDISetProc(Tcl_Obj *objPtr)
{
//    printf("Freeing object %d\n", (int)objPtr->internalRep.longValue);
    int obj_id = (int)objPtr->internalRep.longValue;
    smm.destroy(current_client, obj_id);
}


int CreateDISetFromAnyProc(Tcl_Interp *interp, Tcl_Obj *objPtr)
{
    int len;
    return TCL_ERROR;

}

Tcl_ObjType set_type = {
	(char *)"di_set",
	FreeDISetProc,
	DupDISetProc,
	UpdateDISetProc,
	CreateDISetFromAnyProc
};

//-----------------------------------------------------------------



 
int symman::set_add(int client, int objecttarg,int objectsrc)
{
    symbolSet temp1,temp2;
    if(!getval(client,objecttarg)){reperr();return -1;}
    if(!getval(client,objectsrc)){reperr();return -1;}
    temp1.insert(clref(objectsrc));
    temp2.insert(clref(objecttarg));
    symbolSet unions;
    symbolArr result;
    int i,j;
    j=clref(objecttarg).size();
    for(i=0;i<j;i++)
    {
	if(unions.includes(clref(objecttarg)[i]))continue;
	result.insert_last(clref(objecttarg)[i]);
	unions.insert_last(clref(objecttarg)[i]);
    }
    j=clref(objectsrc).size();
    for(i=0;i<j;i++)
    {
	if(unions.includes(clref(objectsrc)[i]))continue;
	result.insert_last(clref(objectsrc)[i]);
	unions.insert_last(clref(objectsrc)[i]);
    }
    clref(objecttarg).removeAll();
    clref(objecttarg).insert_last(result);
    return 0;
}


int symman::set_intersect(int client,int objecttarg,int objectsrc)
{
    symbolSet temp1,temp2;
    if(!getval(client,objecttarg)){reperr();return -1;}
    if(!getval(client,objectsrc)){reperr();return -1;}
    temp1.insert(clref(objectsrc));
    temp2.insert(clref(objecttarg));
    symbolSet unions;
    symbolSet ob2;
    ob2.insert(clref(objectsrc));
    symbolArr result;
    int i,j;
    j=clref(objecttarg).size();
    for(i=0;i<j;i++)
    {
	if(unions.includes(clref(objecttarg)[i]))continue;
	if( ! ob2.includes(clref(objecttarg)[i]))continue;
	result.insert_last(clref(objecttarg)[i]);
	unions.insert_last(clref(objecttarg)[i]);
    }

    clref(objecttarg).removeAll();
    clref(objecttarg).insert_last(result);
    return 0;
}


int symman::set_subtract(int client,int objecttarg,int objectsrc)
{
    symbolSet temp1,temp2;
    if(!getval(client,objecttarg)){reperr();return -1;}
    if(!getval(client,objectsrc)){reperr();return -1;}
    temp1.insert(clref(objectsrc));
    temp2.insert(clref(objecttarg));
    symbolSet unions;
    symbolSet ob2;
    ob2.insert(clref(objectsrc));
    symbolArr result;
    int i,j;
    j=clref(objecttarg).size();
    for(i=0;i<j;i++)
    {
	if(unions.includes(clref(objecttarg)[i]))continue;
	if(ob2.includes(clref(objecttarg)[i]))continue;
	result.insert_last(clref(objecttarg)[i]);
	unions.insert_last(clref(objecttarg)[i]);
    }

    clref(objecttarg).removeAll();
    clref(objecttarg).insert_last(result);
    return 0;
}




int symman::get(int client,int objectfrom,int objectto,int position)
{
    if(!getval(client,objectto)){reperr();return(-1);}
    if(!getval(client,objectfrom)){reperr();return -1;}
    if(clref(objectfrom).size()<=position){reperr();return -1;}
    clref(objectto).removeAll();
    clref(objectto).insert_last(clref(objectfrom)[position]);
    return 0;
}

int symman::set(int client,int objecttarg,int objectsrc,int position)
{
    if(!getval(client,objecttarg)){reperr();return -1;}
    if(!getval(client,objectsrc)){reperr();return -1;}
    if(clref(objectsrc).size()!=1){reperr();return -1;}
    if(position>clref(objecttarg).size()){reperr();return -1;}
    if(position==clref(objecttarg).size())
    {
	clref(objecttarg).insert_last(clref(objectsrc)[0]);
	return(0);
    }
    clref(objecttarg)[position]=clref(objectsrc)[0];
    return(0);

}

int symman::get_kind(int client,int object)
{
    if(!getval(client,object)){reperr();return -1;}
    if(clref(object).size()==0){reperr();return -1;}
    return(clref(object)[0].get_kind());

}

int symman::look_up(int client,int object,int kind, char *name, int def_file)
{
    projNode *proj;
    symbolArr res;
    int waste;
    waste=def_file;
    for(int i=0;proj=projList::search_list->get_proj(i);i++)
    {
	
	Xref * thisproj;
	thisproj=proj->get_xref();
	if(!thisproj)continue;
	thisproj->find_symbol(res,ddKind(kind), name,0);
	//Hmm... this doesn't seem to take def file as input. Ask Trung
	//what to do...

	if(res.size())
	{

	    getval(client,object)->removeAll();
	    getval(client,object)->insert_last(res[0]);


	    return(object);
	}
    }
    return(-1);
}

char * symman::get_name(int client,int object)
{
    static genString buffer;

    if(!getval(client,object)){reperr();return NULL;}
    if(clref(object).size()==0){reperr();return NULL;}
    buffer = clref(object)[0]->get_name();
    /* The following is of documentation value only as it is useful
       only if done on the client side ! */
    return((char *)buffer);
}
    

int symman::create(int client,int &object)
{
    object=newEntityFor(client);
    return object;
}

int symman::destroy(int client,int &object)
{
    deleteEntity(client,object);
    return(0);
}



int symman::append(int client,int objecttarg, int objectsrc)
{
    if(getval(client,objecttarg) && getval(client,objectsrc))
    {
	clref(objecttarg).insert_last(clref(objectsrc));
	return(0);
    }
    else
    {
	reperr();
	return(-1);
    }
}
 


int symman::copy(int client,int objecttarg, int objectsrc)
{
    if(getval(client,objecttarg) && getval(client,objectsrc))
    {
	clref(objecttarg).removeAll();
	clref(objecttarg).insert_last(clref(objectsrc));
	return(0);
    }
    else
    {
	reperr();
	return(-1);
    }
}




int symman::size(int client,int object)
{
    if(getval(client,object))
    {
	return(getval(client,object)->size());
    }
    else
	return(-1);
}


//////////////////////////////////
// Project management functions //
//////////////////////////////////

extern int scopeMgr_proj_is_special (projNode *proj);

void  cli_get_roots(symbolArr&roots)
{
  projNode *proj;
  for(int i=1;proj=projList::search_list->get_proj(i);i++) {
    if (scopeMgr_proj_is_special(proj))
      continue;
	
    symbolPtr way = proj;
    
    if(way.isnotnull())
      roots.insert_last(way);
    }
}
int find_root_projects(int client,int object)
{
    Initialize(find_root_projects);
    
    if(smm.getval(client,object)==NULL){reperr();return -1;}

    symbolArr& roots = *smm.getval(client,object);
    roots.removeAll();
    cli_get_roots(roots);
    return(object);
}     

void scope_get_projects(symbolPtr& sym, symbolArr& res)
{
  ddKind kind = sym.get_kind();
  if(kind != DD_SCOPE)
    return;

  scopeNode*sc = scopeNodePtr(RelationalPtr(sym));

  if(is_scopeProj(sc)){
    projNode*proj = clone_get_proj(scopeProjPtr(sc));
    res.insert_last(proj);
    return;
  }

  scopeRoot*rt = 0;

  if(is_scopeClone(sc)){
    rt = clone_get_master(scopeClonePtr(sc));
  } else if(is_scopeRoot(sc)){
    rt = scopeRootPtr(sc);
  }

  if(rt) {
    for(scopeClone*cur=scopeRootPtr(sc)->get_first(); cur; cur = cur->get_next()){
       symbolPtr sym = cur;
       scope_get_projects(sym, res);
     }
  }    
}

void scopes_get_projects(symbolArr& scopes, symbolArr& res)
{
  int sz = scopes.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr& sym = scopes[ii];
    scope_get_projects(sym, res);
  }
}

void cli_find_contents(symbolArr&src, symbolArr&trg)
{
  Initialize(cli_find_contents);

  trg.removeAll();       
    
  int size = src.size();
  for (int i =0; i<size; i++) {
    symbolPtr& sym  = src[i];
    ddKind knd = sym.get_kind();
    Relational * ob;
    switch (knd) {
    case DD_SCOPE:
      {
	ob = sym;
	if(!ob)
	  break;
	scopeNode* sc = checked_cast(scopeNode,ob);
	symbolArr scope_contents;
	sc->get_contents(scope_contents);
	    
	trg.insert_last(scope_contents);
	break;
      }
    case DD_PROJECT:
      {
	ob = sym;
	if(!ob || !is_projNode(ob))
	  break;
	projNode* proj = checked_cast(projNode,ob);
	
	proj->refresh();
	symbolArr& proj_contents = proj->rtl_contents();

	int sz = proj_contents.size();
	for (int ii = 0; ii < sz; ++ii)
	{
	  symbolPtr& el = proj_contents[ii];
	  Relational* obj = el;
	  if(is_projNode(obj)){
             projNode* pr = (projNode*)obj;

	     if( !pr->is_visible_proj() ) continue;
	  }
	  trg.insert_last(el);
	}
	break;
      }

    case DD_AST:
      {
	  symbolPtr ch;
	  for (tree_get_first(sym,ch); ch.isnotnull(); tree_get_next(ch,ch))
	      trg.insert_last(ch);
      }
      break;

    default:
      break;
    }
  }
  if(size > 1)
    trg.usort();

}

void cli_find_child_projects(symbolArr&src_arr, symbolArr& trg_arr)
{
   symbolArr res;
   cli_find_contents(src_arr, res);
   trg_arr.removeAll();

   int sz = res.size();

   symbolArr scopes;
   for(int ii=0; ii < sz; ++ii){
     symbolPtr& sym = res[ii];
     ddKind kind = sym.get_kind();
     if(kind == DD_PROJECT)
       trg_arr.insert_last(sym);
     else if(kind == DD_SCOPE)
       scopes.insert_last(sym);
   }  
   if(scopes.size()){
     res.removeAll();
     scopes_get_projects(scopes, res);
     sz= res.size();
     for(int ii = 0; ii<sz; ++ii)
       trg_arr.insert_last(res[ii]);
   }
}

int find_child_projects(int client, int src, int trg)
{
    Initialize(find_child_projects);
    if(!smm.getval(client, src)) return -1;
    if(!smm.getval(client, trg)) return -1;

    symbolArr & src_arr = * smm.getval(client, src);
    symbolArr & trg_arr = * smm.getval(client, trg);

    cli_find_child_projects(src_arr, trg_arr);

    return(trg);
}

projModule * symbol_get_module( symbolPtr& sym);

void cli_find_parent_project(symbolArr&src, symbolArr& trg)
{
  Initialize(cli_find_parent_project);
    trg.removeAll();

    int sz = src.size();
    for(int ii=0; ii<sz; ++ii){
      symbolPtr& sym = src[ii];
      objTree*parent = NULL;
      symbolPtr parent_sym;
      ddKind kind = sym.get_kind();
      switch (kind) {
      case DD_PROJECT:
	{
	appTree * atp = sym;
	projNode *proj=checked_cast(projNode,atp);
	if(proj)
	  parent = proj->find_parent();
        break;
      }
      case DD_SCOPE:
      {
        scopeNode*sc = scopeNodePtr(RelationalPtr(sym));
	if(is_scopeClone(sc))
	   parent = sc->get_parent();
	break;
      }
      case DD_AST:
	tree_get_parent(sym, parent_sym);
	break;

      case DD_INSTANCE:
        parent_sym = sym.scope();
	break;

      default:
       { 
	projModule* mod = symbol_get_module(sym);        
	if(mod)
	  parent =  mod->get_project_internal();
	break;
       }
      }
      if(parent)
	  trg.insert_last(parent);
      else if (parent_sym.isnotnull())
	  trg.insert_last(parent_sym);
    }
    if(sz > 1)
      trg.usort();

}
int find_parent_project(int client,int objectsrc,int objecttarg)
{
    Initialize(find_parent_project);

    symbolArr& src = *smm.getval(client,objectsrc);
    symbolArr& trg = *smm.getval(client,objecttarg);

    cli_find_parent_project(src, trg);

    return(objecttarg);
}

void cli_defines_or_uses_not_unique(symbolArr&src_arr, symbolArr&result_arr, ddSelector&sel, int def_flag)
{
    objArr projects;
    symbolArr modules;
    symbolArr others;

    int array_size=src_arr.size();

    // we definitely should not show checksums and language 
    // kinds to the user - they are for internal purposes only 
    // and will not give any usefull information to the user
    sel.rem(DD_LANGUAGE);
    sel.rem(DD_CHECKSUM);
    sel.rem(DD_VERSION); // DD_VERSION is an expanded macro
                         // for now we should not show them to the 
                         // user because we do not have queries for them 
                         // (for example "Which macro is it?"). When (and if) 
                         // we'll have such a query we should remove this line.

    for(int i=0; i<array_size; i++) {
      symbolPtr& current_symbol=src_arr[i];
      ddKind kind = current_symbol.get_kind();
 
      if(kind == DD_PROJECT || kind == DD_SCOPE) {
        projects.insert_last((Obj*) current_symbol);
      } else if (kind == DD_MODULE) {
	modules.insert_last(current_symbol);
      } else {
	others.insert_last(current_symbol);
      }
    }
    if(def_flag == 0)
      sel.want_used = 1;

    int ii;

    int no_other = others.size();

    for(ii=0; ii < no_other; ++ii) {
      symbolPtr other = others[ii].get_xrefSymbol();
      if(other.isnotnull())
	other.get_define(result_arr, sel);
    }

    int no_mod = modules.size();

    for(ii=0; ii < no_mod; ++ii) {
      symbolPtr mod = modules[ii].get_xrefSymbol();
      if(mod.isnotnull())
	mod.get_define(result_arr, sel);
    }

    if(projects.size() != 0){
      symbolArr result;
      symbolScope scope;

      scope.set_domain(projects);
      scope.query(result,sel);
      result_arr.insert_last(result);
    }
}

void cli_defines_or_uses(symbolArr&src_arr, symbolArr&result_arr, ddSelector&sel, int def_flag)
{
  cli_defines_or_uses_not_unique(src_arr, result_arr, sel, def_flag);
  int array_size=src_arr.size();
  if(array_size >1 || ! def_flag ){
    result_arr.remove_dup_syms();
  }
}

int defines_whatever_selector(int client, int objectsrc, int objecttarg, ddSelector& sel, int def_flag)
{
    Initialize(defines_whatever_selector);


    if(smm.getval(client,objectsrc)==0){reperr();return -1;}
    if(smm.getval(client,objecttarg)==0){reperr(); return -1;}

    symbolArr & src_arr = *smm.getval(client,objectsrc);
    int array_size=src_arr.size();
    
    symbolArr & result_arr = *smm.getval(client,objecttarg);
    result_arr.removeAll();

    if(array_size == 0)
	return objecttarg;
    
    cli_defines_or_uses(src_arr, result_arr, sel, def_flag);

    return objecttarg;
}

void fill_array_with_xref_query(symbolArr&src_arr, const genMask& links, symbolArr&res)
{
  Initialize(fill_array_with_xref_query);
  RTL result(0);

  XrefQuery::DoQueries(src_arr, links, &result);
    
  RTLNode *  rnp = checked_cast(RTLNode,result.get_root());
  symbolArr & arr=rnp->rtl_contents();
    
  res.insert_last(arr);
}

void fill_array_with_xref_query(symbolArr&arr, int link, symbolArr&res)
{
  genMask links;
  links.add((unsigned)link);
  fill_array_with_xref_query(arr, links, res);
}

void defines_whatever_link(const symbolArr& src_arr, symbolArr& result_arr, genMask& links)
{
  int array_size=src_arr.size();
  result_arr.removeAll();

  symbolArr new_src;

  for(int i=0;i<array_size;i++){
    symbolPtr xsym = src_arr[i].get_xrefSymbol();
    if(xsym.isnotnull())
      new_src.insert_last(xsym);
  }
  
  if (new_src.size() > 0)
    fill_array_with_xref_query(new_src, links, result_arr);
}

int defines_whatever_link(int client, int objectsrc, int objecttarg, genMask& links)
{
    Initialize(defines_whatever_link);

    if(smm.getval(client,objectsrc)==0){reperr();return -1;}
    if(smm.getval(client,objecttarg)==0){reperr(); return -1;}

    symbolArr & src_arr = *smm.getval(client,objectsrc);
    symbolArr & result_arr = *smm.getval(client,objecttarg);

    defines_whatever_link(src_arr, result_arr, links);

    return objecttarg;
}

int api_get_link(int client, int objectsrc, int objecttarg, int link)
{
  genMask links;
  links.add((unsigned)link);
  return 
    defines_whatever_link(client, objectsrc, objecttarg, links);
}
 

int defines_whatever(int client, int objectsrc, int objecttarg, ddKind whatever)
{
    Initialize(defines_whatever);

    if(smm.getval(client,objectsrc)==0){reperr();return -1;}
    if(smm.getval(client,objecttarg)==0){reperr(); return -1;}

    symbolArr & src_arr = *smm.getval(client,objectsrc);   
    int array_size = src_arr.size();
    
    if(array_size==0){
      smm.getval(client,objecttarg)->removeAll();
    } else {
      ddSelector sel;
      sel.add(whatever);
      defines_whatever_selector(client, objectsrc, objecttarg, sel, 1);
    }
    return objecttarg;
}

void cli_find_packages(symbolArr&src_arr, symbolArr& pkgs_arr)
{
    // Fill pkgs_arr with all defined packages (SQL).
    ddSelector sel;
    sel.add(DD_PACKAGE);
    pkgs_arr.removeAll();
    cli_defines_or_uses(src_arr, pkgs_arr, sel, 1);

    // For Java, defined packages are those used in "package" statements.
    // Start by finding all packages.
    symbolArr used_pkgs;
    cli_defines_or_uses(src_arr, used_pkgs, sel, 0);

    // Packages that occur in Java "package" statements have a relation of
    // using the file where they occur.  Add any such symbols to the output.
    int sz = used_pkgs.size();
    for (int ii = 0; ii < sz; ii++) {
    	symbolPtr & pkg = used_pkgs[ii];
    	symbolArr def_files;
        symbolArr arr;
    	arr.insert_last(pkg);
    	fill_array_with_xref_query(arr, SHOW_IS_USING_MODULES, def_files);
    	if (def_files.size() > 0) {
    	    pkgs_arr.insert_last(pkg);
    	}
    }
}

int newClient()
{
    return(smm.insertNewClient());
}

void removeClient(int cl)
{
    smm.deleteClient(cl);
}

void setClient(int cl)
{
    current_client = cl;
}

symbolArr *get_DI_symarr(int val)
{
    return smm.getval(current_client, val);
}

symbolArr *get_DI_symarr(DI_object obj)
{
    return smm.getval(current_client, (int)obj->internalRep.longValue);
}


// THES ARE NEVER USED
//static char * parameters;
//static int param_pos;
//int create_view_of_def(symbolPtr app_node);


extern "C" {

// THIS IS NOT USED
//void DI_open_view(DI_object it)
//{
//}

int cli_get_link(int objectsrc, int objecttarg, int link)
{
  genMask links;
  links.add((unsigned)link);
  return 
    defines_whatever_link(current_client, objectsrc, objecttarg, links);
}
 
void DI_object_create(DI_object *di_obj)
{
    *di_obj       = Tcl_NewObj();
    DI_object obj = *di_obj;
    obj->bytes    = NULL;
    obj->typePtr  = &set_type;
    int tmp;
    smm.create(current_client, tmp);
    obj->internalRep.longValue = tmp;
    Tcl_IncrRefCount(obj);
//    printf("Created object %d\n", (int)obj->internalRep.longValue);
}

void DI_object_delete(DI_object di_obj)
{
    Tcl_DecrRefCount(di_obj);
}

void DI_object_append(DI_object source, DI_object target)
{
    smm.append(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source));
}

void DI_object_copy(DI_object source,DI_object target)
{
    smm.copy(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source));
}

void DI_object_name(DI_object object, genString& name)
{
    name = smm.get_name(current_client, GET_DI_SET_INDEX(object));
}

void DI_object_lookup(DI_object object, int kind, char * name, DI_object def_file)
{
    smm.look_up(current_client, GET_DI_SET_INDEX(object), kind, name, GET_DI_SET_INDEX(def_file));
}

void DI_object_size(DI_object object,intp i)
{
    *i = smm.size(current_client, GET_DI_SET_INDEX(object));
}

void DI_object_kind(DI_object object,intp i)
{
    *i = smm.get_kind(current_client, GET_DI_SET_INDEX(object));
}

void DI_array_set(DI_object source, int position , DI_object target)
{
    smm.set(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source), position);
}

void DI_array_get(DI_object source, int position , DI_object target)
{
    smm.get(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), position);
}

void DI_array_union(DI_object source, DI_object target)
{
    smm.set_add(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source));
}

void DI_array_subtract(DI_object source, DI_object target)
{
    smm.set_subtract(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source));
}

void DI_array_intersect(DI_object source, DI_object target)
{
    smm.set_intersect(current_client, GET_DI_SET_INDEX(target), GET_DI_SET_INDEX(source));
}

void DI_where_defined(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINED_IN);
}

void DI_where_referenced(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_REF_FILE);
}

void DI_files_where_included(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_INCLUDED_BY);
}

void DI_where_used(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_USED_BY);
}

void DI_uses_functions(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_FUNCTIONS);
}

void DI_uses_variables(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_VARIABLES);
}

void DI_uses_classes(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_CLASSES);
}

void DI_uses_structures(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_STRUCTURES);
}

void DI_uses_unions(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_UNIONS);
}

void DI_uses_enums(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_ENUMS);
}

void DI_uses_enum_val(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_ENUM_VAL);
}

void DI_uses_typedefs(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_TYPEDEFS);
}

void DI_uses_macros(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_USING_MACROS);
}

void DI_show_include_files(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_INCLUDING);
}

void DI_show_member_functions(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_MEMBER_FUNCS);
}

void DI_show_member_data(DI_object source, DI_object target)
{
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_MEMBER_DATA);
}

void DI_show_friends (DI_object source, DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_FRIENDS );
}

void DI_show_super_classes (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_HAS_SUPERCLASS );
}

void DI_show_sub_classes (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_HAS_SUBCLASS );
}

void DI_show_defined_as (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_DEFINED_AS );
}

void DI_show_decl_usage (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_DECL_USAGE );
}

void DI_show_declared_in(DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_DECLARED_IN);
}

void DI_show_declares(DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_DECLARES);
}

/////WARNING CHECK THIS PART WITH JERRY------------------------
void DI_show_associations (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_ASSOC_OF_SYM );
}

void DI_show_assocd_entities (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_SYM_OF_ASSOC );
}
//--------------------------------------------------------------
void DI_show_documents (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_DOCUMENTS );
}

void DI_instance_of_template (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_INSTofTEMPL );
}

void DI_template_of_instance (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_TEMPLofINST );
}

void DI_subsystems (DI_object source,DI_object target)
{  
    api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_SUBSYSTEMS );
}

static int is_xref(DI_object it)
{
    if((*smm.getval(current_client, GET_DI_SET_INDEX(it))).size() == 0)
	return 0;
    return((*smm.getval(current_client,GET_DI_SET_INDEX(it)))[0].is_xrefSymbol());
}

//
// Create a new api object and fill it with the specified symbols:
//
void ArrToDobjName(const symbolArr &arr, genString &name)
{
  DI_object dobj;
  DI_object_create(&dobj);
  symbolArr *symarr = get_DI_symarr(dobj);
  symarr->insert_last(arr);
  name.printf("_DI_%d", dobj);
}

extern "C" int api_arg_to_obj(const char *);

//
// Get the contents of an api object:
//
void DobjNameToArr(const char *name, symbolArr &arr)
{
  int dobj = api_arg_to_obj(name);
  symbolArr *symarr = get_DI_symarr(dobj);
  if (symarr)
    arr.insert_last(*symarr);
}

//------------------------------------------------------------

void DI_find_child_projects (DI_object source,DI_object target)
{  
    find_child_projects(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target));
}

void DI_find_parent_project (DI_object source,DI_object target)
{  
    find_parent_project(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target));
}

void DI_old_find_files(DI_object source, DI_object target)
{
    if(!is_xref(source))
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_MODULE);
    else 
	reperr();

}

//------------------------------------------------------------


void DI_defines_functions (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client,GET_DI_SET_INDEX(source),GET_DI_SET_INDEX(target),SHOW_IS_DEFINING_FUNCTIONS);
    else
	defines_whatever(current_client,GET_DI_SET_INDEX(source),GET_DI_SET_INDEX(target),DD_FUNC_DECL );
}

void DI_defines_variables (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_VARIABLES);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_VAR_DECL );
}

void DI_defines_classes (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_CLASSES);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_CLASS );
}

void DI_defines_structures (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_STRUCTURES);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_CLASS );
}

void DI_defines_unions (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_UNIONS);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_UNION );
}

void DI_defines_enums (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_ENUMS);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_ENUM );
}

void DI_defines_typedefs (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_TYPEDEFS);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_TYPEDEF );
}

void DI_defines_macros (DI_object source,DI_object target)
{  
    if(is_xref(source))
	api_get_link(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), SHOW_IS_DEFINING_MACROS);
    else
	defines_whatever(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), DD_MACRO );
}

//////////////////////////
//////////////////////////

void DI_uses_query(DI_object source, DI_object target, ddSelector& sel)
{
  defines_whatever_selector(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), sel, 0);
}

void DI_perform_query(DI_object source, DI_object target, ddSelector& sel)
{  
  defines_whatever_selector(current_client, GET_DI_SET_INDEX(source), GET_DI_SET_INDEX(target), sel, 1);
}

void DI_roots (DI_object target)
{
    Initialize(DI_roots);

    find_root_projects(current_client, GET_DI_SET_INDEX(target));
}

void DI_find_contents(DI_object source, DI_object target)
{
    Initialize(DI_find_contents);
    
    if (!smm.getval(current_client,GET_DI_SET_INDEX(target))) return;
    if (!smm.getval(current_client,GET_DI_SET_INDEX(source))) return;
    
    symbolArr & src = *smm.getval(current_client,GET_DI_SET_INDEX(source));
    symbolArr & trg = *smm.getval(current_client,GET_DI_SET_INDEX(target));

    cli_find_contents(src, trg);
}

void DI_deep_find_files(DI_object src, DI_object trg, int depth)
{
    Initialize(DI_deep_find_files);
    
    int source = (int)src->internalRep.longValue;
    int target = (int)trg->internalRep.longValue;
    if (!smm.getval(current_client,target)) return;
    
    smm.getval(current_client,target)->removeAll();       
    if (!smm.getval(current_client,source)) return;
    
    int size = smm.getval(current_client, source)->size();
    for (int i =0; i<size; i++) {
	symbolPtr sym  = (*smm.getval(current_client, source))[i];
	ddKind knd = sym.get_kind();
	objArr children;

	if(knd != DD_PROJECT && knd != DD_SCOPE)
	    continue;

	Relational *ob = sym;
	if (knd == DD_PROJECT) {
	    if(!ob || !is_projNode(ob))
		continue;
	    projNode* proj = checked_cast(projNode,ob);
	    if(!proj)
		continue;
	    proj->refresh();
	
	    proj_get_modules(proj, children, depth);
	} else {
	    if(!ob)
		continue;
	    scopeNode* sc = checked_cast(scopeNode,ob);
	    if(!sc)
		continue;
	    
	    scope_get_modules (sc, children, depth);
	}
	
	int size = children.size();
	for (int ii=0; ii< size; ii++) {
	    symbolPtr insert=children[ii];
	    smm.getval(current_client,target)->insert_last(insert);
	}
    }
}

void DI_find_files(DI_object source, DI_object target)
{
    DI_deep_find_files(source, target, 0);
}

//////////////////////////    


//-----------------------------------------------------------
/* Code template
void DI_ (DI_object source,DI_object target)
{  
    defines_whatever(current_client,source,target, );
}
*/

} // end extern C
#ifdef XXX_check_if_used
int check_if_used (symbolPtr sym)
{
  DI_object it;
  DI_object_create(&it);
  (*smm.getval(current_client,GET_DI_SET_INDEX(it))).insert_last(sym);
  DI_object tar;
  DI_object_create(&tar);
  DI_where_used(it,tar);
  int size=0;
  DI_object_size(tar,&size);
  DI_object_delete(it);
  DI_object_delete(tar);
  return(size);
}
#endif
/*  my testing function ... */
#if 0

extern "C" {
void oferstest(int y)
{
    int foo = symbolAttribute::api_init_attrs();


    
	
//    cout << smm.newEntityFor(0) << endl; //0
	
//    cout << find_root_projects(0,0)<<"val\n";
//    for(int i=0;i<smm.getval(0,0)->size();i++)
//	cout << (*smm.getval(0,0))[i]->get_name() <<" *** < "<< endl;


//    cout << smm.newEntityFor(0) << endl; //1
//    cout << smm.get(0,0,1,9) << "get" << endl;

//    cout << smm.newEntityFor(0) << endl; //2
//    cout << find_child_projects(0,1,2)<<"val\n";

//    cout << smm.newEntityFor(0) << endl; //3
//    cout << smm.get(0,2,3,9) << "get" << endl;

//    cout << smm.newEntityFor(0) << endl; //4
//    cout << defines_whatever(0,3,4,DD_MODULE)<<"val\n";


    cout << smm.newEntityFor(0) << endl; //0
    
    groupHdr* group = get_group_by_name("my_group");

    if (group == NULL) return;

    smm.getval(0, 0)->removeAll();
    group->get_members( *smm.getval(0, 0), false); 
    
    int size =  smm.getval(0, 0)->size();

    for (int i=0; i< size; i++) {

    symbolPtr sym = (*smm.getval(0,0))[i];
    
    symbolAttribute* att = symbolAttribute::get_by_name("eleme");
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Name does not work" << endl;
    };

    att = symbolAttribute::get_by_name("knd");
    
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Kind does not work" << endl;
    };

    att = symbolAttribute::get_by_name("project");
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Project does not work" << endl;
    };

    att = symbolAttribute::get_by_name("filename");
    if (att) {
	cout << " file- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "File does not work" << endl;
    };

    att = symbolAttribute::get_by_name("real-filename");
    if (att) {
	cout << " real- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "File does not work" << endl;
    };

    att = symbolAttribute::get_by_name("file");
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "File does not work" << endl;
    };

    att = symbolAttribute::get_by_name("is-head");
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Header does not work" << endl;
    };

    att = symbolAttribute::get_by_name("inline");
    if (att) {
	cout << "  inl- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Inline does not work" << endl;
    };

    att = symbolAttribute::get_by_name("lang");
    if (att) {
	cout << "   --- " <<  att->value(sym)  << " &&&&&&&&&& " << endl;
    }
    else {
	cout << "Language does not work" << endl;
    };

};
}
}

int joebob(symbolArr& a, int index, int g_index, int g_size, char*val, Interpreter*,void*)
{
    cout << a[index].get_name() << "     G-size:" << g_size << "  G-index:" << g_index << "  Val:" << (char*)val << endl;
    return 0;
}

typedef int (*setTraverseFunction) (symbolArr&,int,int,int,char*,Interpreter*,void*);

extern void api_sort_apply(symbolArr& arr, char*exp, bool once, setTraverseFunction f, Interpreter*inter, void* data);


extern "C" {
void formatTest(int y)
{
    int foo = symbolAttribute::api_init_attrs();
    cout << smm.newEntityFor(0) << endl; //0
    groupHdr* group = get_group_by_name("my_group");
    if (group == NULL) return;
    smm.getval(0, 0)->removeAll();
    group->get_members( *smm.getval(0, 0), false); 
    int size =  smm.getval(0, 0)->size();


    symbolFormat* f = new symbolFormat("myForm");
    
    objArr atts;
    atts.insert_last(symbolAttribute::get_by_name("name"));
    atts.insert_last(symbolAttribute::get_by_name("kind"));
    //atts.insert_last(symbolAttribute::get_by_name("lang"));
    atts.insert_last(symbolAttribute::get_by_name("header"));
    atts.insert_last(symbolAttribute::get_by_name("function"));

    f->setHeader("\n");
    f->setFormat("NAME: %s;   KIND: %s;  LANG: %s   HEAD: %s;  FUNC: %s\n");
    f->setAttributes(atts);

    f->printOut(cout, *smm.getval(0,0));
    f->sort(*smm.getval(0,0), *smm.getval(0,0));
    cout << endl << "==============================" << endl;
    f->printOut(cout, *smm.getval(0,0));
    cout << endl << "==============================" << endl;
    cout << endl << "==============================" << endl;



    objArr atts2;
    atts2.insert_last(symbolAttribute::get_by_name("numargs"));
    atts2.insert_last(symbolAttribute::get_by_name("inline"));
    atts2.insert_last(symbolAttribute::get_by_name("name"));
    atts2.insert_last(symbolAttribute::get_by_name("kind"));
    atts2.insert_last(symbolAttribute::get_by_name("fname"));

    f->setHeader("<><><><><><><><><><><><>\n");
    f->setFormat("%10s%10s%10s%10s%10s\n");
    f->setAttributes(atts2);

    f->printOut(cout, *smm.getval(0,0));
    f->sort(*smm.getval(0,0), *smm.getval(0,0));
    cout << endl << "==============================" << endl;
    f->setFormat("%6s%6s%40s%20s%10s%s%s\n");
    f->printOut(cout, *smm.getval(0,0));
    
    f->setSortOrder("dadd");
    f->sort(*smm.getval(0,0), *smm.getval(0,1));
    cout << endl << "==============================" << endl;
    f->setFormat("%6s%6s%40s%20s%10s%s%s\n");
    f->printOut(cout, *smm.getval(0,1));

    cout << endl << endl << endl;
    api_sort_apply(*smm.getval(0,0), "kind", true, joebob, NULL,NULL);
    cout << "|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-" << endl;
    api_sort_apply(*smm.getval(0,0), "kind", false, joebob, NULL,NULL);

    cout << endl << endl << endl;
    api_sort_apply(*smm.getval(0,0), "numargs", true, joebob, NULL,NULL);
    cout << "|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-" << endl;
    api_sort_apply(*smm.getval(0,0), "numargs", false, joebob, NULL,NULL);

    cout << endl << endl << endl;
    api_sort_apply(*smm.getval(0,0), "(fun || proj)", true, joebob, NULL,NULL);
    cout << "|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-|-" << endl;
    api_sort_apply(*smm.getval(0,0), "(fun || proj)", false, joebob, NULL,NULL);
}

}
#endif
