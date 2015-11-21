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
//////////////////////   FILE obj_print.C  /////////////////////////////////
//
// -- Contains Printers.
//

#include "transaction.h"
#include "objTree.h"
#include "obj_print.h"

#include "symbolPtr.h"
#include <cmd.h>

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <sstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _objArr_h
#include <objArr.h>
#endif

static const unsigned arr_size = 40;
static Relational * obj_arr[arr_size];
static const Obj *cur_obj=0;
static int  cur_ind=0;
void _tree_prt(objTree *, ostream&, char const *);
void ptr_prt(ostream &, const void * const);
static void tree_link_prt(objTree *, ostream&);
static void relations_prt(Relational *, ostream&);
static void one_obj_prt(Relational *, ostream&, int, int = 0);
static void outindent(int, ostream&);
void one_node_prt(const Obj *, ostream&);
void one_coll_prt(objArr *, ostream&);
int null_prt(const Obj *, ostream&);
extern "C" int node_prx (const Obj*, ostream&);
static int test_id(const Obj *, ostream&);

void obj_prt_obj(void* obj, ostream&os)
{
  if(!obj)
     os << "<0x0>";
  else
     os << '<' << obj_type(obj) << ' ' << obj << '>';
}
extern "C" void obj_prt_obj(void* obj) { obj_prt_obj(obj, cout);}
extern "C" char const * obj_type(void* oo)
{
 Obj * obj = (Obj*) oo;
 char const *tp = 0;

 if(!obj)
   tp = 0;
 else if(symbolPtr_is_symbol(obj))
   tp = "xrefSymbol";
 else if (!obj->relationalp())
   switch (obj->collectionp()){
   case 0:  tp = "Obj";       break;
   case 1:  tp = "objArr";    break;
   case 2:  tp = "objSet";    break;
   default: tp = "Collection";break;
   }
 else
   tp = RelationalPtr(obj)->descr() ? 
               RelationalPtr(obj)->descr()->name : "Relational";

 return tp;
}

void node_prt(const Obj* obj, ostream& os)
  {
  os << "\n====== node_prt:\n";

  if (node_prx (obj, os))     // print xrefSymbol*
      return;

  if(null_prt(obj, os))
    return;

  cur_obj = obj;
  cur_ind = 0;

  if(obj->collectionp()){
    one_coll_prt((objArr *)obj, os);
  }  else {
    one_node_prt(obj, os);

    if(obj->relationalp()){
      ostringstream ostr;
      RelationalPtr(obj)->print(ostr);
      cmd_validate("node_prt", ostr.str().c_str());
    }
  }
  os << endl;
}
void node_prt(const Obj* obj) {node_prt(obj, cout);}

void tree_prt(objTree *  const obj, ostream&os)
{
  os << "\n====== tree_prt: ";
  obj_prt_obj( obj, os);

  if(null_prt(obj, os))
    return;
  if(is_objTree(obj)) {
    _tree_prt(obj, os, 0);
     VALIDATE{
       os << endl;
       cmd_validate_tree("tree_prt", objTreePtr(obj));
     }
  }  else  {
    os << "Warning: it isn't a tree node: node_prt called\n";
    node_prt(obj, os);
  }
  os << endl;
}
void tree_prt(objTree *  const obj) {tree_prt(obj, cout);}
int null_prt(const Obj *x, ostream&os)
{
  if(x == 0) {
    os << "<0>" << "\n";
    return 1;
  }
  return test_id(x, os);
}

static int test_id(const Obj *x, ostream&os)
  {
  if(x == 0)
    return 1;

  int id = x->get_id();
  int r = 0;
  if(id < 1) {
    os << "Warning !!!!!!! This object ";
    switch(id)
      {
      case 0: 
	os << "is going to be deleted immediately";
        r = 0;
	break;

      case -1:
	os << "deleted: after destructor of class \"Relational\"";
        r = 1;
	break;

      case -2:
	os << "deleted: after destructor of class \"Obj\"";
        r = 1;
	break;

      case -3:
	os << "is disposed: pending to be deleted at the end of transaction";
        r = 0;
	break;

      case -4:
	os << "is going to be deleted (inside \"obj_real_delete\", pend array";
        r = 0;
	break;

      case -5:
	os << "is going to be deleted (inside \"obj_real_delete\", proc array)";
        r = 0;
	break;

      default:
	os << "deleted or spoiled: obj_id has wrong value";
        r = 1;
	break;
      }
    os << "\n Id = " << id << "\n";
    }
  if(r)
    os << "Stop printing this object" << endl;
  return r;
  }

// Print one node with relations and tree links
void one_node_prt(const Obj *obj, ostream&os)
{
  Initialize(one_node_prt);

  os << "\nRelational NODE :";
   
  one_obj_prt((Relational *)obj, os, -1);
  
  if(is_objTree((Relational *)obj))
    tree_link_prt(checked_cast(objTree,obj), os);
  relations_prt((Relational *) obj, os);
  os << '\n';
  }

void one_coll_prt(objArr * ta, ostream&os)
{
  os << "Collection <" << (int)ta << 
            "> contains " << ta->size() << " elements" << '\n'; 
  Obj *el;
  ForEach(el, *ta){
    os << '\n';
    outindent(2, os);
    one_obj_prt((Relational *) el, os, 2, 1);
  }
}

void children_prt(objTree * ta, ostream&os)
{
  objTree *cur = ta->get_first();
  if(! cur) return;
  
  os << "\n  =====<Children>:" ;
  while(cur) {
    os << '\n';
    outindent(1, os);
    one_obj_prt(cur, os, 1, 1);

    cur = cur->get_next();
  } 
}

void _tree_prt(objTree *  obj, ostream& st, char const *prefix)
{
  st << endl;
  if(prefix) st << prefix;

  st << obj << " ";
  obj->print(st);

  genString pref = prefix;
  if(prefix &&obj->get_next())
      pref += "| ";
  else
      pref += "  ";
  prefix = pref;

  for(obj = obj->get_first(); obj; obj=obj->get_next())
 {
    _tree_prt(obj, st, prefix);
  }
}

void gen_print_indent(ostream& st, int level)
{
  for(int ii=0; ii< level; ++ii)
    st << "  ";
}

void gen_print_cr(ostream& st, int level)
{
  if(level >= 0){
    st << '\n';
    gen_print_indent(st, level);
  } else {
    st << ' ';
  }
}

extern "C" Relational * follow(ostream&os, int ind) 
{ 
  if(ind >= cur_ind) return NULL;
  Relational * oo = obj_arr[ind]; 
  node_prt(oo, os);
  return oo;
}

extern "C" void node_repeat()
  {
  if (cur_obj) node_prt((Relational *)cur_obj);
  }

extern "C" void tree_repeat(ostream&os)
{ 
  Initialize(tree_repeat);

  if(! cur_obj)
    return;
  tree_prt(checked_cast(objTree,cur_obj), os);
  }

void ptr_prt(ostream & st, const void * const p)
{
  st << ' ' << (void *)p << '(' << (int)p << ')' << ' ';
}


// Print one object
static void one_obj_prt(Relational * obj, ostream&os, int lev, int arr)
  {
  if(obj)
    {
    if(test_id(obj, os))
      return;
    if(arr)
      {
      os << '[' << cur_ind << ']';
      if(cur_ind < arr_size)
        obj_arr[cur_ind] = obj;
      ++cur_ind;
      }
    obj->Relational::print(os, -1);
    os << '\n';
    outindent(lev, os);
    obj->print(os, lev);
    }
  else
    os << "<NONE>";
  os << '\n';
  }

static void outindent(int l, ostream& ostr)
  {
  while(l-- > 0)
    ostr << "  ";
  }

static char const *trn[4] =
  {
  "parent  ",
  "previous",
  "next    ",
  "first   "
  };

static  void oop(Relational * ob, ostream&os, char const *txt){
    if(! ob) return;
    os << '\n';
    outindent(1, os);
    os << "=====<" << txt << "> :\n";
    outindent(1, os);
    one_obj_prt(ob, os, 1, 1);
}

static void tree_link_prt(objTree * t, ostream&os)
  {
  oop(t->get_parent(), os, trn[0]);
  children_prt(t, os);
  oop(t->get_prev(), os, trn[1]);
  oop(t->get_next(), os, trn[2]);
  }

static void prrel(Relational * r, ostream&os, RelType * rt, int ii)
  { 
    static char const *pf[2] = {"relation", "pointer "};

    Obj * ma = r->get_raw_ptr(rt); 
    if(!ma) return;

    char const *rn = rt->get_name();
    os << '\n';
    outindent(1, os);

    os << "=====<" << pf[ii] << "> " << rn << ':';            
    Obj * curr_memb;
    if(ma->collectionp()) 
      os << ' ' << ma->size() << " elements:";
    ForEach(curr_memb, *ma )
      {
      os << '\n';
      outindent(1, os);
      one_obj_prt((Relational * )curr_memb, os, 1, 1);
      }
  }

static void relations_prt(Relational * r, ostream&os)
  {
  relArr& ra =  r->get_arr();
  int sz = ra.size();
 
  RelType * rt;

  for(int i=0; i < sz; ++i)
    {
    Relation * cr = ra[i];
    rt = cr->get_rel_type();
    prrel(r, os, rt, 0);
    }

  for(rt=r->descr()->rel; rt; rt=rt->next)
    prrel(r, os, rt, 1);

  os << endl;
  }
