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
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <dfa_db.h>
#include <dfa.h>
#include <smt.h>
#include <ddict.h>
#include <db.h>
#include <metric.h>
#include <xrefSymbol.h>

struct dfa_assign {
    int lev;
    dfa_assign();
    void init();
    dfa_assign(const dfa_assign&);
};

dfa_assign::dfa_assign() : lev(0)
{
}

void dfa_assign::init()
{
    lev = 0;
}

dfa_assign::dfa_assign(const dfa_assign& other) : lev(other.lev)
{
}

struct dfa_call {
    ddElement * fun;
    int question;
    dfa_call();
    dfa_call(const dfa_call&);
};

dfa_call::dfa_call() : fun(0), question(0)
{
}

dfa_call::dfa_call(const dfa_call& other) 
: fun(other.fun), question(other.question)
{
}

struct dfa_state {
    dfa_assign * assign;
    dfa_assign * value;
    dfa_call* call;
    dfa_state();
    dfa_state(const dfa_state&);
};

dfa_state::dfa_state() : assign(NULL), value(NULL), call(NULL)
{
}

dfa_state::dfa_state(const dfa_state& other) :
assign(other.assign), value(other.value), call(other.call)
{
}

static int dfa_find(smtHeader* head, astRoot* ast, astNode* root,dfa_state* st)
{
    Initialize(dfa_find);

    dfa_state cur_state = *st;
    dfa_state null_state;
    dfa_assign local_assign;
    dfa_assign local_value;
    dfa_call local_call;
    
    astNode * ch = NULL;

    switch (root->get_code()){
      case DFA_REF_DD:
      case DFA_REF_DFA:
	if (st->assign) {
	    smtTree * smt = head->cn_na(root->get_start());
 	    if (smt) {
		int index = ast->smts.index(smt);
		dfa_data* dt = NULL;
		if (index >= 0) {
		    dt = ast->data[index];
		} else {
		    ast->smts.insert(smt);
		    dt = ast->data.grow(1);
		}
		uint& old = (uint&)dt->ast_category;
		int level = st->assign->lev & 0xf;
		xrefSymbol::set_attribute(old,1,ASSIGN_ATT,1);
		xrefSymbol::set_attribute(old,level,ASSIGN_LEVEL_ATT,4);
	    }
	}
	if (st->value) {
	    smtTree * smt = head->cn_na(root->get_start());
 	    if (smt) {
		int index = ast->smts.index(smt);
		dfa_data* dt = NULL;
		if (index >= 0) {
		    dt = ast->data[index];
		} else {
		    ast->smts.insert(smt);
		    dt = ast->data.grow(1);
		}
		uint& old = (uint&)dt->ast_category;
		if (st->value->lev < 0)
		    xrefSymbol::set_attribute(old,1,ADDR_ATT,1);
		else {
		    int level = st->value->lev & 0xf;
		    xrefSymbol::set_attribute(old,1,VALUE_ATT,1);
		    xrefSymbol::set_attribute(old,level,VALUE_LEVEL_ATT,4);
		}
	    }
	}
	if (st->call && st->call->fun) {
	    smtTree * smt = head->cn_na(root->get_start());
 	    if (smt) {
		int index = ast->smts.index(smt);
		dfa_data* dt = NULL;
		if (index >= 0) {
		    dt = ast->data[index];
		} else {
		    ast->smts.insert(smt);
		    dt = ast->data.grow(1);
		}
		uint& old = (uint&)dt->ast_category;
		xrefSymbol::set_attribute(old,1,ARG_ATT,1);
		symbolPtr xsym = st->call->fun->get_xrefSymbol();
		if (xsym.xrisnotnull())
		    dt->offset = xsym.get_offset();
	    }
	}
	if (st->call && st->call->question && root->get_code() == DFA_REF_DD)
	    st->call->fun = ((dfa_ref_dd*)root)->get_dd();
	break;

      case MC_LT_EXPR: 
      case MC_LE_EXPR: 
      case MC_GT_EXPR: 
      case MC_GE_EXPR: 
      case MC_EQ_EXPR: 
      case MC_NE_EXPR:
	cur_state.assign = NULL;
	cur_state.call = NULL;
	cur_state.value = &local_value;
	ch = root->get_first();
	if (ch) {
	    dfa_find(head,ast,ch,&cur_state);
	    local_value.init();
	    ch = ch->get_next();
	    if (ch)
		dfa_find(head,ast,ch,&cur_state);
	}
	break;

      case MC_MODIFY_EXPR:
      case INIT_EXPR:
	ch = root->get_first();
	if (ch) {
	    cur_state.assign = &local_assign;
	    dfa_find(head,ast,ch,&cur_state);
	    ch = ch->get_next();
	    if(ch) {
		cur_state.assign = NULL;
		cur_state.value = &local_value;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case MC_PREDECREMENT_EXPR:
      case MC_PREINCREMENT_EXPR:
      case MC_POSTDECREMENT_EXPR:
      case MC_POSTINCREMENT_EXPR:
	ch = root->get_first();
	if (ch) {
	    cur_state.assign = &local_assign;
	    cur_state.value = &local_value;
	    dfa_find(head,ast,ch,&cur_state);
	    ch = ch->get_next();
	    if (ch) {
		cur_state.assign = NULL;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case COMPOUND_EXPR:
	ch = root->get_first();
	if (ch) {
	    dfa_find(head,ast,ch,&null_state);
	    ch = ch->get_next();
	    if (ch)
		dfa_find(head,ast,ch,st);
	}
	break;

      case MC_PLUS_EXPR:
      case MC_MINUS_EXPR:
	ch = root->get_first();
	if (ch) {
	    dfa_find(head,ast,ch,st);
	    ch = ch->get_next();
	    if(ch) {
		cur_state.assign = NULL;
		local_value.init();
		cur_state.value = &local_value;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case MC_ARRAY_REF:
	ch = root->get_first();
	if (ch) {
	    if (cur_state.assign) {
		cur_state.assign = &local_assign;
		local_assign = *st->assign;
//		cur_state.assign->lev++;
	    }
	    if (cur_state.value) {
		cur_state.value = &local_value;
		local_value = *st->value;
//		cur_state.value->lev++;
	    }
	    dfa_find(head,ast,ch,&cur_state);
	    ch = ch->get_next();
	    if(ch) {
		cur_state.assign = NULL;
		local_value.init();
		cur_state.value = &local_value;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case MC_COMPONENT_REF:
      case JAVA_COMPONENT_REF:
	ch = root->get_first();
	if (ch) {
	    cur_state.assign = NULL;
	    cur_state.value = NULL;
	    dfa_find(head,ast,ch,&cur_state);
	    ch = ch->get_next();
	    if (ch) {
		cur_state.assign = st->assign;
		cur_state.value = st->value;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case MC_CALL_EXPR:
	if (st->call)
	    local_call = *st->call;
	cur_state.call = &local_call;
	local_call.question = 1;
	ch = root->get_first();
	if (ch) {
	    dfa_find(head,ast,ch,&cur_state);
	    cur_state.assign = NULL;
	    cur_state.call->question = 0;
	    for (ch = ch->get_next();ch; ch=ch->get_next()) {
		local_value.init();
		cur_state.value = &local_value;
		dfa_find(head,ast,ch,&cur_state);
	    }
	}
	break;

      case MC_INDIRECT_REF:
	if (cur_state.assign) {
	    cur_state.assign = &local_assign;
	    local_assign = *st->assign;
	    cur_state.assign->lev++;
	}
	if (cur_state.value) {
	    cur_state.value = &local_value;
	    local_value = *st->value;
	    cur_state.value->lev++;
	}
	ch = root->get_first();
	if (ch)
	    dfa_find(head,ast,ch,&cur_state);
	break;

      case MC_ADDR_EXPR:
	if (cur_state.assign) {
	    cur_state.assign = &local_assign;
	    local_assign = *st->assign;
	    cur_state.assign->lev--;
	}
	if (cur_state.value) {
	    cur_state.value = &local_value;
	    local_value = *st->value;
	    cur_state.value->lev--;
	}
	ch = root->get_first();
	if (ch)
	    dfa_find(head,ast,ch,&cur_state);
	break;
	
      case MC_SWITCH_STMT:
	if (ch = root->get_first()) {
	    cur_state.assign = NULL;
	    cur_state.value = &local_value;
	    cur_state.call = NULL;
	    dfa_find(head,ast,ch,&cur_state);
	    for(ch=ch->get_next(); ch; ch=ch->get_next())
		dfa_find(head,ast,ch,st);
	}
	break;   

      case MC_RETURN_STMT:
	cur_state.assign = NULL;
	cur_state.value = &local_value;
	cur_state.call = NULL;
	for (ch = root->get_first(); ch; ch = ch->get_next())
	    dfa_find(head,ast,ch,&cur_state);
        break;

      case MC_FIELD_DECL:
      case MC_VAR_DECL:
	ch = root->get_first();
	if (ch) {
	  smtTree * smt = head->cn_na(ch->get_start());
	  ch = ch->get_next();
	  if (smt) {
	    int index = ast->smts.index(smt);
	    dfa_data* dt = NULL;
	    if (index >= 0) {
	      dt = ast->data[index];
	    } else {
	      ast->smts.insert(smt);
	      dt = ast->data.grow(1);
	    }
	    uint& old = (uint&)dt->ast_category;
	      xrefSymbol::set_attribute(old,1,DECL_ATT,1);
	    if (ch)
	      xrefSymbol::set_attribute(old,1,ASSIGN_ATT,1);
	  }
	  if (ch) {
	    cur_state.assign = NULL;
	    cur_state.value = &local_value;
	    cur_state.call = NULL;
	    dfa_find(head,ast,ch,&cur_state);
	  }
	}
	break;
	
      case MC_FUNCTION_DECL:
	ch = root->get_first();
	if (ch) {
	  smtTree * smt = head->cn_na(ch->get_start()+ch->get_length()-1);
	  if (smt) {
	    int index = ast->smts.index(smt);
	    dfa_data* dt = NULL;
	    if (index >= 0) {
	      dt = ast->data[index];
	    } else {
	      ast->smts.insert(smt);
	      dt = ast->data.grow(1);
	    }
	    uint& old = (uint&)dt->ast_category;
	      xrefSymbol::set_attribute(old,1,DECL_ATT,1);
	  }
	}
        for (; ch; ch=ch->get_next())
	  dfa_find(head,ast,ch,st);
	break;
	
      case MC_PARM_DECL:
	ch = root->get_first();
	if (ch && ch->get_code() == DFA_DECLSPEC)
	  ch = ch->get_next();
	if (ch) {
	  smtTree * smt = head->cn_na(ch->get_start());
	  ch = ch->get_next();
	  if (smt) {
	    int index = ast->smts.index(smt);
	    dfa_data* dt = NULL;
	    if (index >= 0) {
	      dt = ast->data[index];
	    } else {
	      ast->smts.insert(smt);
	      dt = ast->data.grow(1);
	    }
	    uint& old = (uint&)dt->ast_category;
	      xrefSymbol::set_attribute(old,1,DECL_ATT,1);
	    if (ch)
	      xrefSymbol::set_attribute(old,1,ASSIGN_ATT,1);
	  }
	  if (ch) {
	    cur_state.assign = NULL;
	    cur_state.value = &local_value;
	    cur_state.call = NULL;
	    dfa_find(head,ast,ch,&cur_state);
	  }
	}
	break;
	
      default:
	for (ch = root->get_first(); ch; ch = ch->get_next())
	    dfa_find(head,ast,ch,st);
        break;
    }
	
    return 0;
}

int dfa_find(smtHeader* head, astRoot* ast, astNode* root)
{
    dfa_state state;
    return dfa_find(head,ast,root,&state);
}


unsigned int objIndSet::hash_element(const Object* obj) const
{
  const Relational* sp = sym(obj);
  return (unsigned int)sp;
}  

bool objIndSet::isEqualObjects(const Object&o1, const Object&o2) const
{
  const Relational*s1 = sym(&o1);
  const Relational*s2 = sym(&o2);
  return s1 == s2;
}

const Relational* objIndSet::lookup(const Relational* s) const
{
 Initialize(objIndSet::lookup);
 if(!s)
   return NULL;

 ((objIndSet*)this)->arr.insert_last(s);
  int ind = arr.size();
  Object* oo = findObjectWithKey(*((Object*)ind));
  ((objIndSet*)this)->arr.remove(ind-1);
  
  return (oo ? sym(oo) : 0);
}
int  objIndSet::index(const Relational*s) const
{
 Initialize(objIndSet::index);
 if(!s)
   return -2;

 ((objIndSet*)this)->arr.insert_last(s);
  int ind = arr.size();
  Object* oo = findObjectWithKey(*((Object*)ind));
  ((objIndSet*)this)->arr.remove(ind-1);
  
  return (int)oo - 1;
}

bool objIndSet::insert(const Relational* sym) 
{
  Initialize(objIndSet::insert);
  bool retval = 1;
  arr.insert_last(sym);
  int ind = arr.size();
  int old_ind = (int) add(*((Object*)ind));
  if(old_ind != ind){
    arr.remove(ind-1);
    retval = 0;
  }
  return retval;
}


