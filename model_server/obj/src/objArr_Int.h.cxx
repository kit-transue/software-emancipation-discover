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
////////////////////////////////   FILE objArr_Int.h.c /////////////////////
//
#include "genError.h"
#include "objCollection.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>

        objArr_Int::objArr_Int(const objArr_Int& x)

//      copy constructor

        {
                unsigned i = x.size();
                put_size(i);
                put_total(i);

                os_list = db_new(int, [i]);
                if (i >0)
		    OS_dependent::bcopy ((char *) x.os_list, (char *) os_list, i*sizeof(int));
        };


        void objArr_Int::operator=(const objArr_Int& x)
//
//      assign operator; delete the old array if it exists

        {
                unsigned i = x.size();
                put_size(i);
                put_total(i);

                if (os_list != NULL)
                     delete os_list;

                os_list = db_new(int, [i]);
                if (i > 0)
		    OS_dependent::bcopy ((char *) x.os_list, (char *) os_list, i*sizeof(int));
        }

        void objArr_Int::insert_first(const int x) 
//
//      increase Array size by 1, shift elements to the right and add new one
//      (actually can increase the size by the entire chunk size)
        {
                Initialize ( objArr_Int::insert_first);

                unsigned i = size();
                unsigned cap = get_total();

//              if no more space left - reallocate array
//
                put_size(i + 1);
                if (cap == i) {
                    cap += ARRINT_CHUNK;
                    int *tmp = db_new (int, [cap]);
                    if (i > 0)
                        while (i--) 
                               *(tmp + i + 1) = os_list[i];
		    if(os_list)
                      delete os_list;
                    os_list = tmp;
                    put_total(cap);
                }
                else{
                    if (i>0)
                        while (i--) 
                              os_list[i+1] = os_list[i];
                }
                os_list[0] = x;
                Return
        }

        void objArr_Int::insert_last(const int x) 
//
//      increase Array size by 1 and add a new element to the end
//      (actually can increase the size by the entire chunk size)
        {
                unsigned sz = size();
                unsigned i = sz;
                unsigned cap = get_total();
//
//              if no more space left - reallocate array
//
                put_size(i + 1);
                if (cap == i){
                    cap += ARRINT_CHUNK;
                    int *tmp = db_new(int,[cap]);
                    if (i>0)
			OS_dependent::bcopy ((char *) os_list, (char *) tmp, i*sizeof(int));

		    if(os_list)
                        delete os_list;

                    os_list = tmp;
                    put_total(cap);
                }
                os_list[sz] = x;
                return;
        }

        void objArr_Int::removeAll(){
           if (sz > 0){
	     if(os_list)
               delete os_list;
	     os_list = 0;
             put_size(0);
	     put_total(0);
           }
        }

        void objArr_Int::remove(const int x)

//      find x, shift array to the left, and  decrease Array size by 1
//                                            (without reallocating)
        {
                Initialize ( objArr_Int::remove);

                unsigned sz = size();
                unsigned i = 0;
                while (i <  sz){
                     if (os_list[i] == x)
                         break;
                     i ++;
                }

                if (i < sz ){                // remove if found
                    while (i++ <  sz - 1) 
                         os_list[i-1] = os_list[i];
                    put_size(--sz);
                }
                Return
        }

        void objArr_Int::insert_all(const objArr_Int& x)

//      add argument x Array to this Array at the right; no 
//      check for possible intersection is performed
        {
                unsigned k = size();
                unsigned j = x.size();
                unsigned i = k + j;

                int *tmp = db_new(int,[i]);
                put_size(i);
                put_total(i);

                if ( j > 0 )              
                     while (j--)
                            *(tmp + (--i)) = (x.os_list)[j];

//      copy old list and delete it

                if (os_list != NULL){ 
                     if (k > 0)
                         while (k--)
                                tmp[k] = os_list[k];
		     if(os_list)                     
                       delete os_list;
                }                   
                os_list = tmp;
        }

        int objArr_Int::includes(const int x) const
//
//      Returns 1 if integer in question belongs to array
//      (does not count # of occurences)

        {
                unsigned i = size(); 
                while(i){
                     if ( os_list[i-1] == x )
                        return 1;
                     i--;
                }
                return 0;
        }
/*
   START-LOG-------------------------------------------

   $Log: objArr_Int.h.cxx  $
   Revision 1.2 2000/07/10 23:07:12EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.5  1993/07/20  20:12:57  bakshi
HP/Solaris Port

Revision 1.2.1.4  1993/02/11  18:28:14  mg
*** empty log message ***

Revision 1.2.1.3  1992/11/20  19:31:28  boris
Removed Initialize() macro

Revision 1.2.1.2  1992/10/09  18:55:15  boris
Fix comments

   Revision 1.2.1.1  92/10/07  20:50:23  smit
   *** empty log message ***

   Revision 1.2  92/10/07  20:50:22  smit
   *** empty log message ***

   Revision 1.1  92/10/07  18:21:16  smit
   Initial revision

   Revision 1.1  92/10/07  17:56:54  smit
   Initial revision

   Revision 1.11  92/08/11  18:41:34  builder
   Fix removeAll problem.
   
   Revision 1.10  92/08/10  19:31:50  smit
   speed up 2 times.
   
   Revision 1.9  92/03/25  09:09:53  glenn
   include genError.h, remove semi-colon after Return macros.
   
   Revision 1.8  92/03/13  09:25:44  mfurman
   Bug (delete already deleted) in remove all fixed
   
   Revision 1.7  92/03/11  16:21:23  sergey
   Added removeAll.
  
   Revision 1.6  92/02/27  15:19:55  sergey
   Minor clean up.
  
   Revision 1.5  92/01/02  11:07:25  mg
   Unhooked NIH.
  
   Revision 1.4  91/10/14  10:19:21  sergey
   Added destructor.
  
  
   -- Contains methods of objArr_Int class - array of Integers
  
   History:   04/17/91     S.Spivakovksy      Initial coding.
              08/12/91	   S.Spivakovsky      Rewrote to switch to NIH.
              09/12/91     S.Spivakovksy      Minor clean up (negative cases).

   END-LOG---------------------------------------------

*/
