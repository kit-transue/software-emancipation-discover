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
#include <msg.h>
#include <Hash.h>
#include <genString.h>
#include <genError.h>
#include <objArr_Int.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
#include <disbuild_cache.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */

class envItem : public namedObject {
    genString value;
public:

    envItem(const char *name, const char *val) : namedObject(name), value(val) { }
    const char *get_value() { return (const char *)value; }
    virtual int compare(const Object&) const;
    
};

int envItem::compare(const Object& b) const
{
    int a_len = value.length();
    int b_len = ((envItem *)&b)->value.length();
    return a_len - b_len;
}

class namedEnvs : public nameHash {
public:
    virtual const char* name(const Object*)const;
    virtual bool isEqualObjects(const Object&, const Object&) const;
};


bool namedEnvs::isEqualObjects(const Object& o1, const Object& o2) const
{
    envItem *f1 = (envItem *)&o1;
    envItem *f2 = (envItem *)&o2;

    const char *n1 = f1->get_name();
    const char *n2 = f2->get_name();

    bool res = (strcmp(n1,n2) == 0) ? 1 : 0;
    return res;
}

const char *namedEnvs::name (const Object*o)const
{
    envItem *f = (envItem *)o;
    return (const char *)f->get_name();
}

class listItem
{
    Object   *obj;
    listItem *next;
public:
    listItem (Object *o) : obj(o), next(0) { }
    Object  * get_object() { return obj; }
    
    listItem* get_next() { return next; }
    listItem* set_next(listItem *n) { listItem *old = next ; next = n ; return old; }
    listItem* add (listItem *i, listItem ** prev);

    int compare ( listItem *);
};

class sortedList {
    listItem *head;
public:
    sortedList() : head(0) { }
    listItem* add (Object *);
    listItem* get_head () { return head; }
};


listItem *sortedList::add (Object *ob)
{
    listItem *ret = new listItem (ob);
    if (head == 0) {
	head = ret;
    } else {
	listItem *prev = 0;
	listItem *i = head->add (ret, &prev);

	if (i != ret) {
	    delete ret;
	    ret = i;
	} else if (prev == 0)
	    head = ret;
    }
	
    return ret;
}

int listItem::compare (listItem *i)
{
    Object *a = get_object();
    Object *b = i->get_object();

    int res = a->compare (*b);
    return res;
}


listItem * listItem::add (listItem *i, listItem ** prev)
{
    if (obj == i->get_object())
	return this;

    int diff = compare (i);
    if (diff <= 0) {
	i->set_next (this);
	if (*prev) (*prev)->set_next (i);
    } else {
	*prev = this;
	listItem* nxt = get_next();
	if (nxt == 0)
	    set_next (i);
	else
	    i = nxt->add (i, prev);
    }

    return i;
}

static namedEnvs envs;
static sortedList lst;

static const char *get_env (const char *var) 
{
    Initialize(get_env);
    const char *val   = 0;
    Object     *found = 0;
    int x             = 0;
    
    envs.find (var, x, found);
    if (found) {
	envItem *f = (envItem *)found;
	val        = f->get_value ();
    }
    return val;
}

static int dis_no_metachars () 
{
    Initialize(dis_no_metachars);
    static int no_metachars = -1;
    if (no_metachars == -1) {
	no_metachars = 0;
	if (is_disbuild_cache())
	    no_metachars = 1;
	else {
	    char const *env = OSapi_getenv ("DIS_METACHARS_PRJ");
	    if (env && (strcmp(env, "no")    == 0 ||
			strcmp(env, "NO")     == 0 ||
			strcmp(env, "0")      == 0 ||
			strcmp(env, "false")  == 0 ||
			strcmp(env, "FALSE")  == 0) )
		no_metachars = 1;
	}
    }
    return no_metachars;
}

const char* get_env_var_chars();
char get_env_var_char()
{
  const char* env_var_chars = get_env_var_chars();
  char cv ='$';
  if ( env_var_chars && *env_var_chars ) 
  {
    if ( strchr(env_var_chars, '%') )
       cv = '%';
    else
       cv = *env_var_chars;
  }
  return cv;
}

int prj_env_substitute (const char *orig, genString& sub)
{
    Initialize(prj_env_substitute);
    int res = 0;
    listItem *head = lst.get_head();
    if (!orig || !orig[0] || !head) return res;

    char *p = 0;
    create_path_2DIS((char *)orig, &p);
    orig = (const char *)p;

    for ( listItem *cur = head ; cur ; cur = cur->get_next()) {
	Object *ob = cur->get_object();
	envItem *e = (envItem *)ob;
	const char *val = e->get_value();
	int len = strlen(val);

	for ( ; len > 0 && (val[len - 1] == '/' || val[len - 1] == '\\') ; len -- )
	  ;

	if (len && strncmp(orig, val, len) == 0) {
	    const char *tail = orig + len;
      char cv =get_env_var_char();
      if ( cv != '%' )
	      sub.printf("%c%s%s", cv, e->get_name(), tail);
      else
	      sub.printf("%c%s%c%s", cv, e->get_name(), cv, tail);
	    res = 1;
	    break;
	}
    }

    OSapi_free ( p );
    return res;
}

static void verbose_print_one_var (const char *var, const char *val)
{
    Initialize(verbose_print_one_var);

    if (!var || !var[0]) return;

    const char *vvv = (val && val[0]) ? val : "nil";

    msg("PRJ variable: $1 : $2", normal_sev)
	<< var << eoarg
	<< val << eoarg << eom;
}

static void verbose_print_vars ()
{
    Initialize(verbose_print_vars);
    if (is_disbuild_cache()) return;

    listItem *head = lst.get_head();
    if (head == 0) {
	msg("No environment variables in PDFs", normal_sev) << eom;
	return;
    }
    msg("PDFs Environment Variables:", normal_sev) << eom;
    for (listItem *cur = head ; cur ; cur = cur->get_next()) {
	envItem *i = (envItem *)cur->get_object();
	verbose_print_one_var (i->get_name(), i->get_value());
    }
}

static int prj_load_pdfs = 0;
void prj_load_pdfs_start () { prj_load_pdfs = 1; }
void prj_load_pdfs_end   () 
{
    prj_load_pdfs = 0; 

    verbose_print_vars();
}

void prj_report_env_var (const char *var, const char *val)
{
    Initialize(prj_report_env_var);

    if (!prj_load_pdfs || !var || !var[0] || !val || !val[0] || dis_no_metachars()) return;

    const char *old_val = get_env (var);
    if (old_val) return;

    char *p = 0;
    create_path_2DIS((char *)val, &p);
    val = (const char *)p;

    // make sure that the value is not relative path
    if ( val[0] == '/' || (isascii(val[0]) && val[1] == ':' && val[2] == '/') ) {
	envItem *o = new envItem (var, val);
	envs.add(*o);
	lst.add(o);
    }

    OSapi_free ( p );
    return;
}
