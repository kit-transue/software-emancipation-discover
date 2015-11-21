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
////////////////////////////////   FILE objSet.h  /////////////////////////////////
//
// -- Contains objSet  Class Definition
#ifndef __objSet
#define __objSet

#ifndef _objDb_h
#include <objDb.h>
#include <objCollection.h>
#endif
#ifndef SET_H      
#include <Set.h>
#endif      

#include <dis_iosfwd.h>

        class objSet : public Obj
        {
             public:
                Set       os_list;                    // NIH Set

             public:
                objSet() : os_list(2) {}
                objSet(int len) : os_list(len) {}
                ~objSet();
                objSet(const objSet &x) { os_list = x.os_list;};
                objSet(Obj *);

                virtual int  collectionp() const;
                virtual void insert(Obj *);
                virtual void insert(objSet *);
                virtual void remove(const Obj *);
                virtual void remove(objSet *);
                virtual void remove_all ();
                virtual int includes(const Obj *) const;
                virtual unsigned size() const;
                virtual int empty() const;
                virtual void purge();

		objSet& operator=(Obj* );
                objSet& operator=( const objSet& x)
		  {os_list = x.os_list; return *this;}
                int operator==(const objSet& x) const  {return (os_list==
                                                              x.os_list);}
                int operator!=(const objSet& x)  {return (os_list!=x.os_list);}
                objSet operator&=(const objSet& x);
                objSet operator|=(const objSet& x);
                objSet operator-=(const objSet& x);
                objSet operator&(const objSet&) const;
                objSet operator|(const objSet&) const;
                objSet operator-(const objSet&) const;
	       
	       virtual void put_signature(ostream& os) const;
               virtual const char * objname() const;
	};


#endif

/*
   START-LOG-------------------------------------------

   $Log: objSet.h  $
   Revision 1.6 2000/07/07 08:11:58EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.4  1994/02/07  17:38:51  mg
 * Bug track: 0
 * cmd_validate
 *
 * Revision 1.2.1.3  1993/07/28  20:23:28  mg
 * uninlined virtuals
 *
 * Revision 1.2.1.2  1992/10/09  18:58:18  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
