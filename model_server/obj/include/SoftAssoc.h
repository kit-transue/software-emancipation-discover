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

//////////////////////   FILE SoftAssoc.h  ///////////////////////
// 
// -- Contains SoftAssoc related stuff
//     
#ifndef _SoftAssoc_h
#define _SoftAssoc_h

#include "objRelation.h"
#include "objOper.h"
#include "SoftId.h"


//************************
// SoftNotifierInfo CLASS
//************************

class SoftNotifierInfo {
// This class is used to pass all kind of information from the moment
// we recognise a soft-notify need to the moment we actualy notify.
// Object will be deleted imidiatly at end of notification. 

 public:

  appTree* startnode;  // The node that was touched
  objOperType oper;    // Operation type - Insert, rem, split, ...,
  int flags;           // M, D, ...(???)

  SoftNotifierInfo (appTree* n, objOperType t, int f)
                            : startnode(n), oper(t), flags(f) {}

// Used to call the two next private functions.
  friend void notify_softassoc (appTree*, objOperType, int);
  friend void notify_softassoc (appTree*, int);

 private:
// This two functions will be called only by their friend up there or
// by them self.
  void notify_softassoc (appTree*);
  void notify_softassoc (Obj *, appTree*);

};


//*****************
// SoftAssoc CLASS
//*****************

class SoftAssoc : public appTree { 

   private:
     int      flags;                   // flags to notify.
     char     soft_name[MAX_NAME_LEN]; // user's name.

   public:
    define_relational(SoftAssoc,appTree);
     
    copy_member(SoftAssoc);

    SoftAssoc (SoftId *, const char*);
    SoftAssoc (SoftAssoc const &);

    virtual char* get_name() const   { return (char *)soft_name; }
    virtual void  put_name(char* nnn){ strcpy(soft_name, nnn); }

    RelType* get_rel_type() const ;
    SoftAssocType  get_type () const;
    char* get_desc();

    static char  *default_name();

    int   get_flags()                { return flags; }
    void  put_flags(int fff)         { flags = fff;}

    Obj *get_my_objects () const;

    virtual void insert(objArr &);
    virtual void insert(Obj *);
    virtual void insert(Relational *);

    virtual void remove() {};
    virtual void remove(objArr &);
    virtual void remove(Obj *);
    virtual void remove(Relational *);

    virtual void merge(objArr &);
    virtual void merge(Obj *);

    virtual boolean is_correct ();
    virtual unsigned size () const;
    virtual int includes(const Obj *) const;

    virtual void notify_me (appTree *, SoftNotifierInfo *);

    virtual boolean active() const   {return 0;}
   };

generate_descriptor(SoftAssoc,appTree);

// -------- DEFINE RELATION -------------
defrel_many_to_many (SoftAssoc,softassoc,Relational,el);
defrel_many_to_one (SoftAssoc,softassoc,SoftId,softid);


// -------- EXTERN FUNCTION --------------
extern Obj *get_softassoc (appTree *);
extern class SoftAssoc *get_softassoc (Obj* , SoftId *, const char *);
extern class SoftAssoc *get_softassoc (objArr &, SoftId *, const char *);

extern class SoftAssoc *new_softassoc (SoftId *, const char *);
//extern class SoftAssoc *set_soft_assoc (objArr &, SoftId *softid, const char *);
extern void remove_soft_assoc (objArr &, SoftAssoc *);
extern void delete_soft_assoc (SoftAssoc *);


#endif

/*
   START-LOG-------------------------------------------

   $Log: SoftAssoc.h  $
   Revision 1.1 1993/06/25 10:00:10EDT builder 
   made from unix file
 * Revision 1.2.1.3  1993/06/25  14:00:01  bakshi
 * char * cast in get_name to satisfy c++3.0.1 compiler
 *
 * Revision 1.2.1.2  1992/10/09  18:58:07  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
