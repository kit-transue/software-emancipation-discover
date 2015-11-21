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
#ifndef _Relational_h
#define _Relational_h


#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "ddKind.h"
#include "general.h"
#include "objDb.h"
#include "relArr.h"

class app;
class objArr;
class objSet;

#define RelClass(N)  \
 class N; typedef N * paste(N,Ptr); int paste(is_,N)(Relational*)

RelClass(Relational);
class symbolPtr;

extern Relational* DB_NULL;
bool db_restore_complete (const Relational*);


//---------- RelType Class (abstract) --------------------------------------
class RelType;
// Removed "class app" declaration from typedef to eliminate compiler warning.
class app;
typedef Relational* (*restore_fun)(void*, app*, char const *);
struct class_descriptor
{
    unsigned char const type_id;
    unsigned char  depth;
    short const length;
    char const * const name;
    RelType *  rel;
    class_descriptor * base;

    int unused;

    class_descriptor(int l, char const *n, class_descriptor *bs);
};

typedef const class_descriptor * descriptor;

enum AssocType { OTHER_ASSOC, HARD_ASSOC, SOFT_ASSOC , IFL_ASSOC};

class objPropagator;

typedef  void   (*objRelHook)(int,Relational*,Relational*,RelType*);

/*
* function to call on put_relation and rem_relation actions.
* function must be declared as 
*    void foo(int oper, Relational * src, Relational * targ, RelType * rel)
* on put oper = +1, on rem oper = -1.
*/

class RelType : public Obj
{
  public:
    static RelType* find(char const *);

    virtual objPropagator* accept(objPropagator*, int );

    virtual int propagate(RelationalPtr src, int level,
			  RelationalPtr target, int a_type = 0);

    RelType(char const *nnn, int cc = 1, int ff = 0, RelType* invr = 0,
	    int assoc = OTHER_ASSOC);

    RelType(char const *nnn, int cc, int ff, RelType* invr,
                   	    int assoc , int  off, descriptor desc);

 private:
    RelType(const RelType& oo);// : Obj (oo)
//      {
//	inverse_type = oo.inverse_type;
//	for (int i = 0; i < MAX_NAME_LEN; i++)
//	  name[i] = oo.name[i];
//	cardinality = oo.cardinality;
//	flags = oo.flags;
//	assoc_type = oo.assoc_type;
//	next = oo.next;
//	hook = oo.hook;
//        offset =  oo.offset;
//      }
 public:

    int   get_cardinality()          { return cardinality; }
    int   get_flags()                { return flags; }

    RelType* get_inv_type()          { return inverse_type; }
    void  put_inv_type(RelType* inv) { inverse_type = inv; }

    char const *get_name()                 { return name; }
    void  put_name(char const *nnn);

    AssocType get_assoc_type()         { return assoc_type; }
    void  put_assoc_type(AssocType aa) { assoc_type = aa; }

    objRelHook get_hook()               { return hook; }
    void  put_hook(objRelHook new_hook) { hook = new_hook; }

    static RelType* first;	       // head of list
  private:

    RelType*       inverse_type;
    char           name[MAX_NAME_LEN]; //name
    int            cardinality;	       //max # of object to relate
    int            flags;	       //combination of mod, copy, walk, del
    AssocType      assoc_type;	       // SOFT, HARD, or OTHER/_ASSOC
  public:
    RelType*       next;	       // Pointer to next relation type in list
    objRelHook     hook;
    unsigned       offset;             // offset of field containing relation,
                                       // ==0 for (old) dynamic (run-time) rels
    unsigned       forw : 1;           // forward  or reversed 
    unsigned       save : 1;           // is savable
    unsigned       ifl  : 1;           // InterFileLink
    unsigned       hcs  : 1;           // Hook called on the whole set
    unsigned filler :     4;
    unsigned       id   : 8;
    unsigned       xref_id : 8;

 int get_rel_id() { return id;}
 void put_rel_id(int rel_id) {id = rel_id;}
};

inline void obj_rel_put_hook(RelType * rel, objRelHook hook)
                            {rel->put_hook(hook); rel->hcs = 0;}
inline void obj_rel_put_hook_opt(RelType * rel, objRelHook hook, int opt)
                            {rel->put_hook(hook); rel->hcs = opt;}

//---------------- Relational Class (abstract) -------------------------

class objArr;

struct relStruct {
  Relational * src;
  Relational * trg;
  RelType * rel;
};

class genString;   // Removed from description() to eliminate compiler warning.
class Relational : public Obj
{
  friend Obj* get_loaded_rel (const Relational*, const RelType*);
  
  public:
    Relational();
    Relational(const Relational&);

    virtual ~Relational();

    virtual bool relationalp() const;

    virtual RelationalPtr rel_copy() const;

    virtual void	print(ostream& = cout, int level = 0) const;
    virtual char const *objname() const;
  
    virtual descriptor	descr() const;
    static  class_descriptor _descrpt;

    virtual int		subclass(descriptor d);
    virtual void	remove();
    virtual void	send_string(ostream&) const;
    virtual void 	propagate(int, objArr*oa=0);

    virtual char const *get_name() const;
    virtual void  set_name(char const *);

    virtual objPropagator* accept(objPropagator*, RelType*);
    virtual void notify(int, Relational*, objPropagator*, RelType*);
    virtual int save(void *buffer, int maxlength);
    
    
    virtual void description(genString&);
    
    // returns 1 if filled the symbolPtr structure
    virtual int construct_symbolPtr(symbolPtr&) const; // { return 0; }
    virtual ddKind get_symbolPtr_kind() const; // { return DD_UNKNOWN; }
    virtual int is_symbolPtr_def () const; // { return 0; }   //analog to get_is_def()
    virtual app *get_symbolPtr_def_app (); // { return 0; }

    // Virtual place holders
    virtual void Relational_stab1_virtual ();
    virtual void Relational_stab2_virtual ();
    virtual void Relational_stab3_virtual ();

    virtual void Relational_stab4_virtual ();
    virtual void Relational_stab5_virtual ();
    virtual void Relational_stab6_virtual ();
    virtual void Relational_stab7_virtual ();
    virtual void Relational_stab8_virtual ();
    
    Relational& operator=(const Relational&);

    relArr&	get_arr() { return rel_list;}
 
    Obj * get_ptr(const RelType * rt) const
     {
     return  rt->offset ? *(Obj **)((char *) this + rt->offset) : get_loaded_rel (this, rt);
     }

    Obj * get_raw_ptr(const RelType * rt) const
     {
     return  rt->offset ? *(Obj **)((char *) this + rt->offset) : rel_list[rt];
     }

    void rem_pointers();   // removes all pointers

    Obj*	get_rel(const RelType*) const;
    void	put_rel(Relational&, RelType*);
    void	put_rel(objSet&, RelType*);
    void        put_dbnull (RelType* rt) { priv_add_one_rel (DB_NULL, rt); }
    void        rem_dbnull (RelType* rt) { priv_rem_one_rel (DB_NULL, rt); }
    void 	restore_rel (Relational&, RelType*);
    static void restore_rel(class relStructArr& rel_arr);
    void	rem_rel(Relational&, RelType*);
    void	rem_rel(objSet&, RelType*);
    void	rem_rel(RelType*);
    void	rem_rel_all();
    int		test_rel(Relational&, const RelType*) const;

    void        call_rel_hooks(int flag);
  void        rem_rel_from(objSet &); // rem all rels in objSet
  protected:
    relArr rel_list;		// array of pointers to Relation class objects

  private:
    void       priv_add_one_rel(Relational*, RelType*);
    void       priv_rem_one_rel(Relational*, RelType*);

    void       priv_add_one_ptr(Relational*, RelType*);
    void       priv_rem_one_ptr(Relational*, RelType*);

};

#endif				// _Relational_h
