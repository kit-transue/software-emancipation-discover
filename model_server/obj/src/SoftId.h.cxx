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
//////////////////////   FILE SoftId.h.C  ///////////////////////
// synopsis :
#include "objOper.h"
#include "Relational.h"
#include "SoftId.h"
//#include "HyperText.h"
#include "soft-report.h"
#include "RTL_apl_extern.h"
#include "RTL.h"


init_relational(SoftId,appTree);
init_relation(dummyrel_of_softid, 1, NULL, softid_of_dummyrel, MANY, NULL);



//******************
// Constructor
//*****************
SoftId::SoftId 
(const char *sn, RelType *rt, SoftAssocType st, const char *desc) {

    if (!sn) sn = default_name();
    if (!rt) rt = default_reltype();
    if (!st) st = default_softtype();

    put_rel_type  (rt);
    put_name  ((char *)sn);
    put_type (st);
    put_desc (desc);

}
//******************
// Copy constructor
//*****************
SoftId::SoftId (SoftId const & )
{
}

//******************
// Desstructor
//*****************
SoftId::~SoftId () {

    if (desc)
	delete (desc);

}
//******************
// put_desc
//*****************
void  SoftId::put_desc(const char* desc_s) {

    if (desc_s)
    {
	char *ddd = new char[strlen(desc_s) + 1];
	strcpy (ddd, desc_s);
	desc = ddd;
    }
    else
	desc = NULL;
}

//********
// its_me
//********
boolean SoftId::its_me (SoftId *soft_id )
{

    if (get_rel_type() == soft_id->get_rel_type() &&
	!strcmp (get_name(), soft_id->get_name()) &&
	get_type()     == soft_id->get_type())
	return 1;

    return 0;
}

//************
// includes
//************
int SoftId::includes(const Obj *softassoc) const {

    Obj *softassoc_set = softid_get_softassocs((SoftId *)this);

    if (softassoc_set)
        return (softassoc_set->includes(softassoc));
    else
        return 0;

}

//*******************
// default_name()
//*******************
static char def_idname[]="_noidname";
char *SoftId::default_name()  { 
    return def_idname;
} 

//*******************
// default_reltype()
//*******************
RelType *SoftId::default_reltype() { 
    return dummyrel_of_softid;
} 

//*******************
// default_softtype()
//*******************
SoftAssocType SoftId::default_softtype() { 
    return SA_BASE ;
}


