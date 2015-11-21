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
//////////////////////   FILE SoftId.h  ///////////////////////
// 
// -- Contains SoftId related stuff
//     
#ifndef _SoftId_h
#define _SoftId_h

#include "objRelation.h"
#include "objOper.h"

enum SoftAssocType
{
     SA_BASE = 1, SA_HS
};

//**************
// SoftId CLASS
//**************

class SoftId  : public appTree {
// This class is used to group a list of parameter that together define 
// the soft assoc type.
// All SoftAssocs in the system must have relation to it.

 private:
     char     id_name[MAX_NAME_LEN];  // user's name.
     RelType  *rel_type;              // Type of relation between object.
     SoftAssocType soft_type;         // 
     char     *desc;                  // free text to describe the association.

 public:

    define_relational(SoftId,appTree);
   
    SoftId (const char *, RelType *, SoftAssocType, const char *);
    SoftId (SoftId const &);

    ~SoftId ();

    copy_member(SoftId);

    virtual char* get_name() const   { return (char *)id_name; }
    virtual void  put_name(char* nnn){ strcpy(id_name, nnn); }

    RelType* get_rel_type() const    { return rel_type; }
    void  put_rel_type(RelType *rrr) { rel_type = rrr; } 

    SoftAssocType  get_type () const  { return soft_type; }
    void put_type (SoftAssocType ttt) { soft_type = ttt; } 

    char* get_desc()                 { return desc; }
    void  put_desc(const char* ddd);

    static char          *default_name();
    static RelType       *default_reltype();
    static SoftAssocType default_softtype();

    boolean its_me (SoftId *soft_id );
    virtual boolean is_correct () {return 1;}

    virtual int includes(const Obj *softassoc) const;

};

 generate_descriptor(SoftId,appTree);

// -------- DEFINE RELATION -------------
// This relation are defined just to have a fake name when softassoc obj
// Doesn't have real RelType.
defrel_many_to_one (SoftId,softid,Relational,dummyrel);


// -------- EXTERN FUNCTION --------------
extern SoftId *new_softid 
              (const char *, RelType *, SoftAssocType, const char *);
extern SoftId *find_softid (const char *, RelType *, SoftAssocType);

#endif

/*
   START-LOG-------------------------------------------

   $Log: SoftId.h  $
   Revision 1.2 1995/07/27 20:24:05EDT rajan 
   Port
 * Revision 1.2.1.3  1993/06/25  14:00:36  bakshi
 * char * cast in get_name to satisfy c++3.0.1 compiler
 *
 * Revision 1.2.1.2  1992/10/09  18:58:08  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
