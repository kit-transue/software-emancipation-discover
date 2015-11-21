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
//#include <cLibraryFunctions.h>
#define _cLibraryFunctions_h
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
namespace std {};
using namespace std;
#include <strstream>
#endif /* ISO_CPP_HEADERS */
#include <cmd.h>
#include <xref.h>
#include <cmd_internal.h>
#include <objSet.h>
#include <ddict.h>
#include <symbolSet.h>

#include <genTmpfile.h>
#include <proj.h>
#ifndef _WIN32
#include <unistd.h>
#endif /*!_WIN32*/

#include <shell_calls.h>
#include <options.h>

static char *dbg_label = "GDB BUFFER_0";
static char *dbg_flag = "GDB";

int cmd_rerecording = 0;

extern "C" void cmd_journal_rerecord(char* file)
{
    genString new_file = file;
    new_file += ".new";
    char* pdf_file = get_main_option("-P");
    if (!pdf_file)
	pdf_file = get_main_option("-pdf");
	
    cmd_journal_start((char*)new_file, 0, 0, pdf_file, 0);

    cmd_rerecording = 1;
    cmd_journal_execute(file);
}

extern "C" char *smt_truncate_filename(char *);
int cmd_validate_flush(int);
extern "C" void cmd_journal_dump_pset(app *ah)
{
  static int Journal_DebugLevel = customize::getIntPref("DIS_internal.ModelDump_Enabled");

  if (Journal_DebugLevel > 0 ) {
    if (!ah || !is_smtHeader(ah)) return;
    smtHeader * sh = (smtHeader *) ah;
    char *filen = sh->get_phys_name();
    if (filen && *filen) {
      projModule *mod = appHeader_get_projectModule(ah);
      genString dumped_name = get_paraset_file_name(filen, mod);
      dumped_name += ".dump";
      cmd_journal_start(dumped_name,0,0,0,0);
      cmd_validate_dd(sh);
      cmd_validate_tree(smt_truncate_filename(filen), sh->get_root());
      cmd_validate_flush(1);
      cmd_journal_save(NULL);
    }
  }
}

init_relational(objValidationString,namedString);
init_relational(objSignature,objValidationString);

void objSignature::print(ostream& os, int) const
{
  os << "ui_validate_obj(\"" << get_name() << "\", \"" << type_name << 
      "\", \"" << ((objSignature*)this)->get_value() << "\");";
  os << ends;
}

void objValidationString::print(ostream& os, int) const
{
//   os << get_name() << ' ' << ((objSignature*)this)->get_value();
  os << "ui_validate__namedString(\"" << get_name() <<
      "\", \"" << ((objSignature*)this)->get_value() << "\");";
  os << ends;
}

objSignature::objSignature(const char*tag, const Obj* obj) :
     objValidationString(tag, 0, obj->val_signature())
{
  set_value(obj);
}
void objSignature::set_value(const Obj* obj)

{
  ostrstream ostr;
  obj->put_signature(ostr);
  ostr << ends;
  char * sign = ostr.str();

  objValidationString::set_value(sign);
  delete(sign);

  type_name = obj->objname();
}

static objSet cmdInts;
static objSet cmdStrings;
static objSet cmdObjs;

static objArr records;

extern "C" void cmd_validate_int (const char *labl, int val)
{ cmd_validate (labl, val);}
extern "C" void cmd_validate_str (const char *labl, char* val)
{ cmd_validate (labl, val);}


void cmd_validate (const char *labl, symbolPtr& sym)
{
    if (! VALIDATING)
	return;
    Initialize (cmd_validate);

    if (sym.is_xrefSymbol()) {
       ostrstream ostr;
       sym.put_signature(ostr);
       ostr << ends;
       char * val = ostr.str ();
       cmd_validate(labl, val);
       delete val;
    } else {
       Obj* obj = sym;
       cmd_validate	(labl, obj);
    }

}

void cmd_validate(const char *labl, symbolSet& set)
{
    if (! VALIDATING)
	return;
    
    Initialize (cmd_validate);
    symbolArr sarr;
    sarr = set;
    cmd_validate(labl, sarr, 1); // force to sort
}

void cmd_validate (const char *labl, symbolArr& sarr, int force_sort)
{
    if (! VALIDATING)
	return;
    
    Initialize(cmd_validate);

    if (force_sort)
	sarr.sort();
    
    int sz = sarr.size();
    genString sz_labl = labl;
    sz_labl += ".size";
    cmd_validate((char*)sz_labl, sz);
    for(int ii=0; ii<sz; ++ii){
       genString el_labl;
       el_labl.printf("%s.%d", labl, ii);
       cmd_validate((char*)el_labl, sarr[ii]);
    }
}
void obj_get_head(const char * head, Obj& input, objArr& output)
{
 Obj* el;
 int len = strlen(head);
 ForEach(el, input){
   RelationalPtr obj = RelationalPtr(el);
   const char * name = obj->get_name();
   if(strncmp(name, head, len)==0)
     output.insert_last(obj);
 }
}
void obj_prt_head(const char * head, Obj& input)
{
  objArr output;
  obj_get_head(head, input,  output);

  RelationalPtr el;
  int sz = output.size();
  
  cout << endl << "real validation data" << endl;

  for(int ii = 0; ii < sz-1; ++ii){
     el = RelationalPtr(output[ii]);
     el-> print(cout);
     cout << endl;
  }
  cout << endl;
}
void cmd_obj_error (const char * labl, char *txt)
{
    obj_prt_head(labl, records);
    if(cmd_rerecording)
       cmd_validation_warning ((char *)txt);
    else
       cmd_validation_error ((char *)txt);
}
void cmd_validate (const char *labl, symbolArr& sarr)
{
    if (! VALIDATING)
	return;
    
    Initialize(cmd_validate);
    cmd_validate (labl, sarr, 0); // do not sort by default
}

void cmd_validate (const char *labl, int val)
{
    if (! VALIDATING)
	return;

    Initialize(cmd_validate);
    Relational *ob = obj_search_by_name (labl, cmdInts);
    if (ob && !is_namedInt(ob)) {
	cmdInts.remove(ob);
	ob = NULL;
    }

    if (!ob) {
	ob = new namedInt(labl, val);
	cmdInts.insert(ob);

	// if recording keep validation till the end of transaction
	if (cmd_current_journal_name)
	    records.insert_last(ob);
    } else {
	(checked_cast(namedInt,ob))->set_value (val);

	// if recording, send "double" warning
	if (cmd_current_journal_name) {
	    cout << "cmd_validate: name " << labl << " was used again" << endl;
	}
    }
}


void cmd_validate (const char *pre_lable, const char *val, const char *alias)
{
    Initialize(cmd_validate);
    if (! VALIDATING)
        return;
    if(	!(pre_lable && pre_lable[0]))
	return;

    genString wrk;
    const char *labl;
    if (alias && alias[0]) {
	wrk = pre_lable;
	wrk += "_";
	wrk += alias;
	labl = (const char *)wrk;
    }else
	labl = pre_lable;

    Relational *ob = obj_search_by_name (labl, cmdStrings);
    if (ob && !is_objValidationString(ob)) {
	cmdStrings.remove(ob);
	ob = NULL;
    }

    if (!ob) {
	ob = new objValidationString(labl, val);
	cmdStrings.insert(ob);

	// if recording keep validation till the end of transaction
	if (cmd_current_journal_name)
	    records.insert_last(ob);
	
    } else
	(checked_cast(objValidationString,ob))->set_value (val);
}

void  cmd_record_arr(const char*labl, objArr&oarr)
{
   int sz = oarr.size();
   for(int ii=0; ii<sz; ++ii){
      genString tag;
      tag.printf("%s.%d",labl,ii);
      Obj* el = oarr[ii];
      cmd_validate(tag, el);
   }
}
int cmd_cmp_dds(ddElementPtr o1, ddElementPtr o2)
{
  int i1 = o1->get_kind();
  int i2 = o2->get_kind();
  if(i1 < i2) return -1;
  if(i1 > i2) return 1;

  char * n1 = o1->get_name();
  char * n2 = o2->get_name();

  int diff =  strcmp(n1, n2);
  return diff;
}

int cmd_cmp_smts(smtTreePtr o1, smtTreePtr o2)
{
  int i1 = o1->tnfirst;
  int i2 = o2->tnfirst;
  if(i1 < i2) return -1;
  if(i1 > i2) return 1;

  i1 = o1->tnmax;
  i2 = o2->tnmax;
  if(i1 < i2) return -1;
  if(i1 > i2) return 1;

  i1 = o1->type;
  i2 = o2->type;
  if(i1 < i2) return -1;
  if(i1 > i2) return 1;

  i1 = o1->extype;
  i2 = o2->extype;
  if(i1 < i2) return -1;
  if(i1 > i2) return 1;

  return 0;
}

int cmd_set_sort(const void *p1, const void *p2)
{
   Obj* o1 = *(Obj**) p1;
   Obj* o2 = *(Obj**) p2;

   const char* n1 = o1->objname();
   const char* n2 = o2->objname();

   int diff = strcmp(n1, n2);
   if(diff) 
        return diff;
   if(!o1->relationalp()) return 0;
   if(!o2->relationalp()) return 0;
   RelationalPtr r1 = RelationalPtr(o1);
   RelationalPtr r2 = RelationalPtr(o2);
   if(is_smtTree(r1))
     diff = cmd_cmp_smts(smtTreePtr(o1), smtTreePtr(o2));
   else if (is_ddElement(r1))
     diff = cmd_cmp_dds(ddElementPtr(o1), ddElementPtr(o2));
   else {
     n1 = r1->get_name();
     n2 = r2->get_name();
     diff = strcmp(n1, n2);
  }
  return diff;
}

void cmd_validate (const char *labl, const Obj *val)
{
  VALIDATE{
    Initialize(cmd_validate(Obj*));
    objSignature *ob = 
        checked_cast(objSignature, obj_search_by_name (labl, cmdObjs));

    if (!ob) {
	ob = new objSignature(labl, val);
	cmdObjs.insert(ob);

	// if recording keep validation till the end of transaction
	if (cmd_current_journal_name)
	    records.insert_last(ob);
	
    } else {
	ob->set_value (val);
    }

    int coll = val->collectionp();
    if(!coll)
        return;

    if(coll==2){
         objArr oarr;
         Obj * el;
         ForEach(el, *val)
             oarr.insert_last(el);
         oarr.sort(cmd_set_sort);
         cmd_record_arr(labl, oarr);
    } else {
         cmd_record_arr(labl, *(objArr*)val);
    }
  }
}

inline int nonnull(const char*ptr) { return ptr && ptr[0];}

cmd_validate_type(const char*p1, const char*p2)
{
  return
   (nonnull(p1)&&nonnull(p2)) ? strcmp(p1, p2) : 0;
}

extern "C" void ui_validate_obj (char *labl, char*type, char* val)
{

    Initialize(ui_validate_obj);

    Assert(nonnull(labl));

    objSignature *  ob = 
         checked_cast(objSignature, obj_search_by_name (labl, cmdObjs));

    if(!ob){
	genString txt;
        txt.printf ("No object with Label \"%s\"", labl);
        cmd_validation_warning ((char *)txt);
	return;
    }

    const char *new_val = ob->get_value ();
    const char *new_type = ob->type_name;

    int diff = cmd_validate_type (new_type, type);
    if (diff) {
	genString txt;
        txt.printf("Wrong obj type. Expected: %s; Real: %s", type, new_type);
        cmd_obj_error (labl, (char *)txt);
        return;
    }

    validation_function fun = ob->valfun;    
    if (fun)
      diff =(*fun)(new_val, val);
    else
      diff = cmd_validate_string_value	(new_val,val);

    if (diff) {
	genString txt;
        txt.printf ("Wrong obj signature. Expected:\n%s; Real:\n%s\n", 
             val, new_val);
        cmd_obj_error ( labl, (char *)txt);
      }
  }

void cmd_validate (const char *labl, const char *val)
{
    Initialize(cmd_validate);
    const char *alias = NULL;
    cmd_validate (labl, val, 0);
}

extern "C" void ui_validate_namedInt (char *labl, int val)
{
    if(cmd_rerecording) return;
    Initialize(ui_validate_namedInt);
    int res = 0;
    int new_val;

    Relational *ob = NULL;
    if (labl && labl[0]) {
	ob = obj_search_by_name (labl, cmdInts);
	if (ob && !is_namedInt(ob))
	    ob = NULL;

	if (ob) {
	    namedInt *n_ob = checked_cast(namedInt,ob);
	    new_val = n_ob->get_value();
	    if (new_val == val)
		res = 1;
	}
    }

    if (res == 0) {
	genString txt;
	if (!(labl && labl[0])) {
	    txt.printf ("No Label name.");
	    cmd_validation_warning ((char *)txt);
	} else if (ob) {
	    txt.printf ("Wrong integer for label \"%s\". Expected: %d; Real: %d", 
 labl, val, new_val);
	    cmd_validation_error ((char *)txt);
	} else {
	    txt.printf ("No object with Label \"%s\"", labl);
	    cmd_validation_warning ((char *)txt);
	}
    }
}

typedef const char * cochp;

int cmd_skip_dir_of_filename(cochp & pp)
{
 if(*pp == '\0') return 0;
 if(*pp != '/')  return 0;

//  find last '/'
 ++pp;
 int ch;
 for(int off=0; ch=pp[off]; ++off){
    if(ch=='/'){
       pp += off+1;
       off = 1;
       continue;
    } else if(ch <= 32) {  // up to ' '
       break;
    } 
 } 
 return 1;
}

int cmd_skip_white_staff(cochp & pp)
{
 if(*pp == '\0') return 1;
 int res = 0;
 int ch;
 while(ch=*pp){
   if(ch==' ' || ch=='\t' ||ch=='\n' || ch=='r'){
    ++pp;
     res = 1;
}
 else {
     break;
   }
 }
 return res;
}

int cmd_skip_any_int(cochp & pp)
{
 if(*pp == '\0') return 0;
 if(*pp=='%' && pp[1]=='d') {
        pp += 2;
        return 1;
 }
 if(*pp=='-')
     ++pp;
 int ch;
 while((ch=*pp) && ((ch <= '9' && ch >= '0'))){
    ++pp;
 }
 return 1;
}

int cmd_skip_hex_pointer(cochp & pp)
{
 if(*pp == '\0') return 0;
 if(*pp != '0')  return 0;
 if(pp[1] != 'x') return 0;
 int ch;
 pp += 2;
 while((ch=*pp) && 
         ((ch <= '9' && ch >= '0') || (ch <= 'f' && ch >= 'a'))){
    ++pp;
 }
 return 1;
}

int cmd_validate_string_value(const char * new_val, const char * old_val)
{
    int diff = 0;
    while (new_val || old_val)
    {
        int h1 = cmd_skip_hex_pointer(new_val);
        int h2 = cmd_skip_hex_pointer(old_val);

        if (h1 == 1 && h2 == 1)
            break;

        if(h1 != h2)
        {
            diff = 1;
            break;
        }

        h1 = cmd_skip_dir_of_filename(new_val);
        h2 = cmd_skip_dir_of_filename(old_val);

        if(h1 != h2)
        {
            diff = 1;
            break;
        }

        h1 = cmd_skip_white_staff(new_val);
        h2 = cmd_skip_white_staff(old_val);
        if(h1 != h2)
        {
            diff = 1;
            break;
        }
        int skip_white_flag = h1;

        if(*old_val=='%' && old_val[1]=='d')
        {
            // any integer
            old_val += 2;
            h1 = cmd_skip_any_int( new_val);
            if(h1==0)
            {
                diff = 1;
                break;
            }
            else
                continue;
        }

        if ( *new_val != *old_val)
        {
            diff = 1;
            break;
        }

        if (!*new_val)
            break;
        if (!skip_white_flag)
        {
            ++ old_val;
            ++ new_val;
        }
    }
    return diff;
}

static int no_lines_in_string(const char* str)
{
    int ch;
    int no_lines = 0;
    while((ch=*str++))
	if(ch=='\n') 
	    ++no_lines;
    return no_lines;
}
static const char * write_tmpfile(const char * str, genTmpfile &tmpf)
{
    const char * name = tmpf.name();
    int fd = tmpf.open();
#ifndef _WIN32
    write(fd, str, strlen(str));
#else
    OSapi_write(fd, (char *)str, strlen(str));
#endif
    return name;
}

extern "C" void prt_diff(const char*val, const char*new_val)
{
    int old_lines = no_lines_in_string(val);
    int new_lines = no_lines_in_string(new_val);
    if(old_lines > 3 && new_lines > 3){
        cout << endl;
        genTmpfile tmpf1("cmd_");
        genTmpfile tmpf2("cmd_");

	const char * oldf = write_tmpfile(val, tmpf1);
	const char * newf = write_tmpfile(new_val, tmpf2);
	vsysteml("diff", oldf, newf, NULL);
        cout << endl;
    }
}

extern "C" void ui_validate_namedString (char *labl, char *val)
{
    if(cmd_rerecording) return;
    Initialize(ui_validate_namedInt);
    int diff = 0;
    const char *new_val;

    Relational *ob = NULL;
    if (labl && labl[0]) {
	ob = obj_search_by_name (labl, cmdStrings);
	if (ob && !is_objValidationString(ob))
	    ob = NULL;

	if (ob) {
	    objValidationString *n_ob = checked_cast(objValidationString,ob);
	    new_val = n_ob->get_value();
	    diff = cmd_validate_string_value(new_val,val);
	}
    }

    if (diff) {
	genString txt;
	if (!(labl && labl[0])) {
	    txt.printf ("No Label name.");
	    cmd_validation_warning ((char *)txt);
	} else if (ob) {
	    txt.printf ("Wrong string \"%s\". Expected:\n%s; Real:\n%s\n", labl, val, new_val);

	    // Temporary GDB glich to filter out gdb validation
	    if (strstr(labl, dbg_flag)) {
		CMD_GROUP(DEBUG) {
		    if (strcmp(labl, dbg_label) == 0)
			cmd_validation_warning ((char *)txt);
		    else
			cmd_validation_error ((char *)txt);
		}
	    } else
		cmd_validation_error ((char *)txt);
	} else {
	    txt.printf ("No object with Label \"%s\"", labl);
	    cmd_validation_warning ((char *)txt);
	}
	prt_diff(val, new_val);
    }
}

static char *cmdLinenoLabel = "Lineno Label";

void cmd_validate_lineno (const char *nm, int lnum, const char *alias)
{
    Initialize(cmd_validate_lineno);
    int err_flag = 1;
    if (nm && nm[0] && lnum >= 0) {
	char *f_name = strrchr ((char *)nm, '/');
	if (f_name)	    nm = f_name + 1;

	if (nm[0]) {
	    genString valid;
	    if (lnum > 0)
		valid.printf ("%s:%d", nm, lnum);
	    else
		valid = nm;

    cmd_validate (cmdLinenoLabel, valid, alias);
	    err_flag = 0;
	}
    }

    if (err_flag)
	cmd_validation_error("Wrong input in cmd_validate_lineno()");
}

extern "C"void validate_test ()
{
    Initialize(validate_test);
    cmd_current_journal_name = "AAA";
    cmd_validate_str ("Nothing1", "afoo");
    cmd_validate_str ("Nothing2", "bfoo");
    cmd_validate_str ("Nothing3", "doo");
    cmd_validate_str ("Nothing4", "cfoo");
   
    cmd_validate ("strings", &cmdStrings);
    cmd_validate ("objects", &cmdObjs);
}

extern "C" int gettime();  // in millisecs;

static int started_flag = 0;
static int current_cpu_time = 0;

extern "C" void cmd_init_cpu_time()
{
      started_flag = 1;
      current_cpu_time = gettime();
}


extern "C" void cmd_validate_cpu_time(double limit)
{
  if(!started_flag){
      cmd_init_cpu_time();
  } else {
      int prev = current_cpu_time;
      current_cpu_time = gettime();
      double delta = (current_cpu_time - prev) / 1000000.0;
      cout << "\n cpu_time " << delta << endl;
      if (delta > limit) {
         cmd_validation_error("Exeeded cpu limit\n");
      }
  }
}

void cmd_validate_relations (const char *labl, RelationalPtr node, 
                          obj_type_function fun)

{
  relArr& ra =  node->get_arr();
  int sz = ra.size();

  for(int i=0; i < sz; ++i){
    Relation * cr = ra[i];
    RelType * rt = cr->get_rel_type();
    objSet os;
    Obj*el;
    Obj*mb = cr->members;
    ForEach(el, *mb){
      if((!fun) || (*fun)(RelationalPtr(el)))
         os.insert(el);
    }
    int count = os.size();
    if(count==0) continue;

    Obj*mbrs;
    Obj * el1;
    if(count==1)
      ForEach(el1, os)
           mbrs = el1;
    else 
      mbrs = &os;
    
    genString tag;
    tag.printf("%s.%s", labl, rt->get_name());
    cmd_validate((char*)tag, mbrs);
  }
}

void cmd_dump_relations (RelationalPtr node, ostream& ostr,
                          obj_type_function fun)

{
  relArr& ra =  node->get_arr();
  int sz = ra.size();

  for(int i=0; i < sz; ++i){
    Relation * cr = ra[i];
    RelType * rt = cr->get_rel_type();
    objSet os;
    Obj*el;
    Obj*mb = cr->members;
    ForEach(el, *mb){
      if((!fun) || (*fun)(RelationalPtr(el)))
         os.insert(el);
    }
    int count = os.size();
    if(count==0) continue;

    Obj * el1;
    ForEach(el1, os){
      ostr << "  " << rt->get_name() << " --> " << el1 << ' ';
      el1->put_signature(ostr);
      ostr << endl;
    }
  }
}

static int cmd_dd_rels_function(Relational* node)
{
  return
    (is_appTree(node) && (! is_ddRoot(node)));
}

static int dd_rels_function(Relational* node)
{
  return 1;
}

void cmd_validate_dd_node (const char *labl, ddElementPtr dd)
{
  VALIDATE{
    Initialize(cmd_validate_dd_node);
  
    genString tag;
    ddKind kind = dd->get_kind();
    const char * kind_name = ddKind_name(dd->get_kind());
    char *nm = dd->get_name();
    if(kind == DD_PARAM_DECL){
       ddElementPtr fun = checked_cast(ddElement, dd->get_parent());
       char * fun_name = fun->get_name();
       int par_ind = 0;
       for(objTree*cur=dd; cur; cur=cur->get_prev()) ++par_ind;
       tag.printf("%s.%s.%s.%d.%s",
                  labl, kind_name+3, fun_name, par_ind, nm);
    } else {
      tag.printf("%s.%s.%s", labl, kind_name+3, nm);
    }
    cmd_validate_relations((char*)tag, dd, cmd_dd_rels_function);
  }
}

void cmd_dump_dd_node (ddElementPtr dd, ostream& ostr)
{
    Initialize(cmd_dump_dd_node);
    ostr << dd << ' ';
    dd->put_signature(ostr);
    ostr << endl;
    cmd_dump_relations(dd, ostr, dd_rels_function);
}

int obj_tree_get_nodes(objTree*node,  objArr& arr, obj_type_function fun)
{
  int count = 0;
  if( (!fun) || (*fun)(node)){
    if (node->get_id() >=0 ) {
      arr.insert_last(node);
      ++ count;
    }
  }
  for(objTree*cur = node->get_first(); cur; cur=cur->get_next())
     count += obj_tree_get_nodes(cur,  arr, fun);

  return count;
}


void cmd_validate_dd(app *app)
{
  VALIDATE{
  Initialize(cmd_validate_dd);

  ddRoot * ddr = 
      checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, app));
  if(! ddr) return;

  const char * fn = app->get_filename();
  cmd_skip_dir_of_filename(fn);

  objArr dd_arr;
  obj_tree_get_nodes(ddr, dd_arr, is_ddElement);
  genString tag;
  tag.printf("%s.%s", fn, "DD");
  int sz = dd_arr.size();
  cmd_validate((char*)tag, sz);
  for(int ii=0; ii<sz; ++ii)
     cmd_validate_dd_node(fn, ddElementPtr(dd_arr[ii]));

//  dd_map(app);
 }
}

void cmd_dump_app_dd(app *app, ostream& ostr)
{
  Initialize(cmd_dump_app_dd);

  ddRoot * ddr = 
      checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, app));
  if(! ddr) return;

  const char * fn = app->get_filename();
  objArr dd_arr;
  obj_tree_get_nodes(ddr, dd_arr, is_ddElement);
  app->put_signature(ostr);
  ostr << endl;
  int sz = dd_arr.size();
  for(int ii=0; ii<sz; ++ii)
     cmd_dump_dd_node(ddElementPtr(dd_arr[ii]), ostr);
}

static void _tree_validate(objTree *  obj, ostream& st, int level)
  {
  st << endl;
  for(int ii=0; ii<level; ++ii)
     st << "  ";
  st << obj->objname() << ' ' << " ";
  obj->put_signature(st);

  for(obj = obj->get_first(); obj; obj=obj->get_next())
    _tree_validate(obj, st, level+1);
  }

void cmd_validate_tree(const char * labl, objTree* tree)
{
  VALIDATE{
       ostrstream ostr;
       _tree_validate(tree, ostr, 0);
       ostr << ends;
       char * str = ostr.str();
       cmd_validate(labl, str, "tree");
       delete str;
  }
}

extern "C" app * dd_map(Relational* node)
{
   app* header = 0;
   if(is_ddElement(node))
      header = ddElementPtr(node)->get_main_header();
   else if(is_appTree(node))
      header = appTreePtr(node)->get_header();
   else if(is_app(node))
      header = appPtr(node);

   if(header)
     cmd_dump_app_dd(header, cout);
   else
     cout << "cmd_dump_dd: wrong input" << endl;

   return header;
}

static int obj_compare_names(const void * p1, const void * p2)
{
   Relational *rp1 = *(Relational**)p1;
   Relational *rp2 = *(Relational**)p2;
   return strcmp(rp1->get_name(), rp2->get_name());
}


int cmd_validate_flush (int do_sort)
{
    Initialize(cmd_validate_flush);
    int sz;
    Obj *obj;
    
    if ( !(sz = records.size()) )
	return 0;
    if (do_sort)
      records.sort(obj_compare_names);
    ForEach(obj, records) {
	RelationalPtr el = (RelationalPtr)obj;
	if (is_namedInt(el)) {
	    namedInt *n_int = (namedInt *)el;
	    char *labl = n_int->get_name ();
	    int val    = n_int->get_value ();
	    cmd_gen_write_force ("ui_validate_namedInt(%s, %d);", labl, val);
	} else if (is_objSignature(el)) {
	    objSignature *ob = (objSignature *)el;
	    char *labl = ob->get_name ();
	    char *sign = (char *)ob->get_value ();
	    cmd_gen_write_force ("ui_validate_obj(%s, %s, %s);", labl, ob->type_name, sign);
	} else if (is_namedString(el)) {
	    namedString *n_str = (namedString *)el;
	    char *labl = n_str->get_name ();
	    char *val  = (char *)n_str->get_value ();
	    cmd_gen_write_force ("ui_validate_namedString(%s, %s);", labl, val);
	}
    }
    
    records.removeAll();
    
    cmdInts.purge();
    cmdStrings.purge();
    cmdObjs.purge();
    
    return sz;
}

int cmd_validate_flush()
{
  return cmd_validate_flush(0);
}

/*
  $Log: cmd_validate.cxx  $
  Revision 1.7 2000/07/10 23:11:11EDT ktrans 
  mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.18  1994/08/04  22:05:51  trung
 * Bug track: 0
 * 7942, 7955, 7972, 7974, 7978
 *
 * Revision 1.17  1994/08/03  18:45:47  mg
 * Bug track: NA
 * print diff for multiline strings
 *
 * Revision 1.16  1994/07/20  20:19:11  mg
 * Bug track: NA
 * restructured for logging
 *
 * Revision 1.15  1994/06/05  20:22:26  davea
 * Bug track: 6174
 * Use operator= for symbolArr, instead of loop
 *
 * Revision 1.14  1994/05/17  19:10:27  boris
 * Bug track: Scripting
 * Fixed a number of scripting problems
 *
 * Revision 1.13  1994/04/17  13:47:33  boris
 * Fixed bug 6770, whith repeating commands in ParaDebugger window
 *
 * Revision 1.12  1994/04/14  14:49:54  boris
 * Moved DBG validation on DBG BUFFER_0 label from cmdError to cmdWarning
 *
 * Revision 1.11  1994/03/22  20:08:28  mg
 * Bug track: 1
 * validation for dormant code
 *
 * Revision 1.10  1994/03/06  16:32:03  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.9  1994/02/23  14:07:16  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.8  1994/02/16  15:19:15  mg
 * Bug track: 0
 * commented out dd_map call
 *
 * Revision 1.7  1994/02/16  14:05:27  mg
 * Bug track: 0
 * more validation
 *
*/



