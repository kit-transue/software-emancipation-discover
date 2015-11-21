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
#include <objOperate.h>
// #include <xref.h>

#include <cLibraryFunctions.h>
#ifndef _WIN32
/* #include <sys/time.h> SET_SIMPLIFY REMOVED */
#endif
/* #include <sys/types.h> SET_SIMPLIFY REMOVED */
/* #include <stdlib.h> SET_SIMPLIFY REMOVED */
#ifdef _WIN32
#include <stdio.h>
#endif
#include <machdep.h>
#include <ddict.h>
#include <dd_or_xref_node.h>

ddElementPtr new_dd_find_definition (ddKind kind, char const *name);
symbolPtr get_smt_decl_from_init(symbolPtr sym, genString &sym_name);
defrel_one_to_one(ddElement,ifl_src,ddElement,ifl_trg);
init_rel_or_ptr(ddElement,ifl_src,NULL,ddElement,ifl_trg,NULL);


unsigned long secs_in_years(int no_years)
{
  int add_days = no_years / 4;  // number of leap years
  return (no_years * 365 + add_days) * 24 * 60 * 60;
}

// deci-secs from 1/1/92 
unsigned long dd_timestamp()
{
  static unsigned long last_stamp = 0;
  static unsigned long delta = secs_in_years(1992 - 1970);
  //struct timeval tv;
  //struct timezone tz;
  time_t secs;

// time in dsecs from 1/1/70 GMT
  
  OSapi_time(&secs);

// dsecs from 1/1/92

  unsigned long tm = (secs - delta) * 10;

  if(tm <= last_stamp)
        tm = last_stamp + 1;

  last_stamp = tm;
  return tm;
}

// returns temp string (string persists till next call)
char const *dd_genname(char const *prefix = 0)
{

  static int pid = OSapi_getpid();
  static long hid = OSapi_gethostid();

  static char buf[100];

  if(! prefix) prefix = "symbol";
  unsigned long val = dd_timestamp();

  OSapi_sprintf(buf, "%s__%u_%u_%u", prefix, hid, pid, val);

  return buf;
}

// real stuff


// get other: ref if ifl_src, ifl_src if ifl_trg, assuming only one ifl_trg

ddElement * ifl_dd_get_other_dd(ddElement *dd1)
{
 Initialize(ifl_dd_get_other_dd);

   ddKind kd = dd1->get_kind();

   ddElement * dd2 = 
       (kd==DD_IFL_SRC) ? ifl_src_get_ifl_trg(dd1) : ifl_trg_get_ifl_src(dd1);

   if(dd2) {  // already linked
     return dd2;
   }

   char const *nm = dd1->get_ddname();

   ddKind other_kd = (kd==DD_IFL_SRC) ? DD_IFL_TRG : DD_IFL_SRC;

   dd2 = new_dd_find_definition(other_kd, nm);

   if(dd2){
       if(kd==DD_IFL_SRC)
           ifl_src_put_ifl_trg(dd1, dd2);
       else
           ifl_trg_put_ifl_src(dd1, dd2);
   }

   return dd2;
}

static ddElement *create_dd_for_ifl(appTree *ob2, appTree *ob1)
{
  Initialize(create_dd_for_ifl);
  
  genString sym_name;
  app * ah = ob2->get_main_header();
  ddRoot *dr = dd_sh_get_dr(ah);
  symbolPtr sym = get_smt_decl_from_init(ob1, sym_name);
  genString name = sym.get_name();
  return dr->add_def(sym->get_kind(), strlen(name.str()), name, 0, 0, 0, "");
}

void dd_put_ifl(RelType* rel, Relational* o1, Relational*o2) 
{
   Initialize(dd_put_ifl);
   
   appTree *ob1 = checked_cast(appTree, o1);
   appTree *ob2 = checked_cast(appTree, o2);

   if(! rel->forw){
     appTree * temp = ob1;
     ob1 = ob2;
     ob2 = temp;
     rel = rel->get_inv_type();
   }

   RelType *inv = rel->get_inv_type();

   if(rel->get_cardinality() == 1)
         dd_rem_ifl(rel, o1, NULL);

   if(inv->get_cardinality() == 1)
         dd_rem_ifl(inv, o2, NULL);

   char const *name = dd_genname(rel->get_name());

   int db_new_flag = 1;
   if (is_dd_or_xref_node(ob1)) {
       ddElementPtr node_dd = dd_or_xref_nodePtr(ob1)->get_dd();
       smtTreePtr def_smt = 0;
       if (node_dd) {
	   def_smt = checked_cast(smtTree,
                        get_relation(def_smt_of_dd, node_dd));
	   ob1 = (appTree *)def_smt;
       }
       if (!(def_smt && ob1))
	   db_new_flag = 0;
   }
   
   if (is_dd_or_xref_node(ob2)) {
       ddElementPtr node_dd = dd_or_xref_nodePtr(ob2)->get_dd();
       smtTreePtr def_smt = 0;
       if (node_dd) {
	   def_smt = checked_cast(smtTree,
                        get_relation(def_smt_of_dd, node_dd));
	   ob2 = (appTree *)def_smt;
       }
       if (!(def_smt && ob2))
	   db_new_flag = 0;
   }
   
   if (db_new_flag) {
     ddElement *dd1 = 0;
     ddElement *dd2 = 0;
     if (is_smtTree(ob2)) {
        dd1 = db_new(ddElement, (name, ob1, 1, DD_IFL_SRC));
	ddElement *dd_ob2 = create_dd_for_ifl(ob1, ob2);	
        dd2 = db_new(ddElement, (name, dd_ob2, 1, DD_IFL_TRG));	
	put_relation(rel, ob1, dd1);	
	put_relation(inv, dd_ob2, dd2);		
      }
     else if (is_smtTree(ob1)) {
        dd2 = db_new(ddElement, (name, ob2, 1, DD_IFL_TRG));
	ddElement *dd_ob1 = create_dd_for_ifl(ob2, ob1);
        dd1 = db_new(ddElement, (name, dd_ob1, 1, DD_IFL_SRC));	
	put_relation(rel, dd_ob1, dd1);	
	put_relation(inv, ob2, dd2);	
      }
     else {
        dd1 = db_new(ddElement, (name, ob1, 1, DD_IFL_SRC));       
        dd2 = db_new(ddElement, (name, ob2, 1, DD_IFL_TRG));	
	put_relation(rel, ob1, dd1);
	put_relation(inv, ob2, dd2);
	
     }
       ifl_src_put_ifl_trg(dd1, dd2);
   }
}

void dd_rem_ifl(RelType* rel, Relational* o1, Relational*o2) 
{
 Initialize(dd_rem_ifl);
   Assert(o1);
   if(! o2) {  // remove all
      Obj * os = dd_get_ifl(rel, o1);
      Obj * el;
      ForEach(el, *os){
         o2 = (Relational*) el;
         dd_rem_ifl(rel, o1, o2);
      }
      return;
   }

   appTree *ob1 = checked_cast(appTree, o1);
   appTree *ob2 = checked_cast(appTree, o2);

 ddElement * dd1;
 ddElement * dd2; 

 RelType * inv = rel->get_inv_type();
 Obj * os1 = get_relation(rel, ob1);
 Obj * os2 = get_relation(inv, ob2);
 ddKind kd = rel->forw ? DD_IFL_SRC : DD_IFL_TRG;

 Obj *e1;
 int found = 0;
 ForEach(e1, *os1){
   dd1 = checked_cast(ddElement, e1);
   dd2 = (kd == DD_IFL_SRC) ?
        ifl_src_get_ifl_trg(dd1) : ifl_trg_get_ifl_src(dd1);
   if(os2->includes(dd2)){
      found = 1;
      break;
   }
 }
 if(found) {
  // delete rel rigth away
  rem_relation(rel, ob1, dd1);
  rem_relation(inv, ob2, dd2);

  // delete dd elements
  start_transaction(){
    obj_delete(dd1);
    obj_delete(dd2);
  } end_transaction();
 }
}

static objArr ifl_answer; // persists till next call 
Obj* dd_get_ifl(RelType* rel, const Relational* ob) 
{
 Initialize(dd_get_ifl);

// Assert(ob);
 
 objSet ddset;
 ddset = get_relation(rel, ob);
 if (ob && is_smtTree((Relational *)ob) && ddset.size() == 0) {
   genString dummy;
   symbolPtr sym = get_smt_decl_from_init(checked_cast(appTree, ob), dummy); 
   symbolArr res;
   if (sym.isnotnull())
     sym->get_link(ifl_targ_of_src, res);
   symbolPtr src_sym;
   ForEachS(src_sym, res) {
     ddElement *dd = src_sym->get_def_dd(); 
     if (dd)
       ddset.insert(dd);
   }
 }
 else if(ddset.size() == 0) return NULL;

 int sz = ddset.size();
 if(sz == 0) return NULL;

 if(sz > 1)
    ifl_answer.removeAll();

 Obj * el;
 ForEach(el, ddset){
    ddElement * dd1 = (ddElement*) el;
    ddElement * dd2 = ifl_dd_get_other_dd(dd1);

    Obj * xx = get_relation(rel, dd2 );

    if(sz==1) 
       return xx;
    else
       ifl_answer.insert_last(xx);
 }
 return &ifl_answer;
}

Obj* dd_get_ifl(RelType* rel, symbolPtr sym) 
{
 Initialize(dd_get_ifl);

 if (sym.isnull())
    return NULL;

 if (sym.relationalp())
    return dd_get_ifl (rel, (Relational*) sym);

 objSet ddset;
 symbolArr res;

 if (sym.xrisnotnull())
   sym->get_link(ifl_targ_of_src, res);
 symbolPtr src_sym;
 ForEachS(src_sym, res) {
   ddElement *dd = src_sym->get_def_dd(); 
   if (dd)
     ddset.insert(dd);
 }

 int sz = ddset.size();
 if(sz == 0) return NULL;

 if(sz > 1)
    ifl_answer.removeAll();

 Obj * el;
 ForEach(el, ddset){
    ddElement * dd1 = (ddElement*) el;
    ddElement * dd2 = ifl_dd_get_other_dd(dd1);

    Obj * xx = get_relation(rel, dd2 );

    if(sz==1) 
       return xx;
    else
       ifl_answer.insert_last(xx);
 }
 return &ifl_answer;
}

Obj* dd_get_ifl_info(RelType* rel, const Relational* ob) 
{
 Initialize(dd_get_ifl_info);

 Assert(ob);

 objSet ddset;
 ddset = get_relation(rel, ob);
 
 if (ob && is_smtTree((Relational *)ob) && ddset.size() == 0) {
   genString dummy;
   symbolPtr sym = get_smt_decl_from_init(checked_cast(appTree, ob), dummy); 
   symbolArr res;
   if (sym.isnotnull())
     sym->get_link(ifl_targ_of_src, res);
   symbolPtr src_sym;
   ForEachS(src_sym, res) {
     ddElement *dd = src_sym->get_def_dd(); 
     if (dd)
       ddset.insert(dd);
   }
 }
 else if (ddset.size() == 0) return NULL;

 int sz = ddset.size();
 if(sz == 0) return NULL;

 if(sz > 1)
    ifl_answer.removeAll();

 Obj * el;
 ForEach(el, ddset){
    ddElement * dd1 = (ddElement*) el;
    ddKind kd = dd1->get_kind();

    ddElement * dd2 = 
	(kd==DD_IFL_SRC) ? ifl_src_get_ifl_trg(dd1) : ifl_trg_get_ifl_src(dd1);

    if(dd2) {  // already linked
	if (sz==1)
	    return dd2;
	else
	    ifl_answer.insert_last(dd2);
    }
    else { // fetch xrefSymbol

	char const *nm = dd1->get_ddname();
	ddKind other_kd = (kd==DD_IFL_SRC) ? DD_IFL_TRG : DD_IFL_SRC;

	fsymbolPtr xref_sym = lookup_file_symbol(other_kd, nm);
	if (sz==1)
	    return (Obj*)xref_sym;
	else
	    ifl_answer.insert_last((Obj*)xref_sym);
    }
 }

 return &ifl_answer;
}

boolean dd_is_ifl_loaded(ddElement *dd1)
{
 Initialize(dd_is_ifl_loaded);

 Assert(dd1);

 ddKind kd = dd1->get_kind();

 ddElement * dd2 = 
     (kd==DD_IFL_SRC) ? ifl_src_get_ifl_trg(dd1) : ifl_trg_get_ifl_src(dd1);

 return (dd2 != 0);
}

/*
   START-LOG-------------------------------------------

   $Log: ddIfl.h.cxx  $
   Revision 1.17 2000/11/21 09:22:13EST sschmidt 
   Fix for bug 20104: remove obsolete categories, queries. Cleanup.
// Revision 1.11  1994/08/05  19:11:35  boris
// Bug track: 8020, 7981
// Fixed Double hyper link closing on the same steTextNode
//
// Revision 1.10  1994/02/16  01:55:14  kws
// Port
//
// Revision 1.9  1994/02/11  15:27:15  builder
// Port
//
// Revision 1.8  1993/12/17  19:07:17  himanshu
// Bug track: Frame
// Fixed Saving problem
//
// Revision 1.7  1993/09/16  01:50:59  pero
// Bug track: 4050
// seg fault closing a hyperlink within an ERD; dd_put_ifl() changes
//
// Revision 1.6  1993/07/20  19:11:16  bakshi
// HP/Solaris port.
//
// Revision 1.5  1993/03/27  01:42:22  davea
// Remove local declaration, change xrefSymbol* to fsymbolPtr
//
// Revision 1.3  1993/01/11  19:31:09  jon
// removed Assert in dd_get_ifl so IFLs would work more like old relations
//
// Revision 1.2  1993/01/11  17:16:34  jon
// Added dd_get_ifl_info and dd_is_ifl_loaded
//
// Revision 1.1  1992/12/25  17:26:18  mg
// Initial revision
//

   END-LOG---------------------------------------------

*/
