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

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */

#include "gtGenArr.h"
#include <psetmem.h>

void GenArr::reset() {
   for (int i = 0; i < length; i++) {
      free(ptr[i]);
      ptr[i] = 0;
   }
   length = 0;
}   

char** GenArr::grow(int ii) {
	provide(length + ii);						      
	for (int i = 0; i < ii; i++) {
	   ptr[length + i] = 0;
	}
	length += ii;							      
	return ptr;     						      
}

char** GenArr::append(char*  el) {
       if (search(el)<0)
	{
 	      	grow(1);       						      
       		ptr[length-1] =(char *) OSapi_strdup(el);
	       	return ptr;    						      
	}
        return NULL;
    }									      
    
void GenArr::shrink(int ind,  int len) {				
       int new_length = length - len;	      
       int ii, jj;							      
	for(ii=ind, jj=ind+len; ii<new_length; ++ii, ++jj) {
	   free(ptr[ii]);
	   ptr[ii] = ptr[jj];	
        }
       for (int i = length; i < new_length; i++) {
	  ptr[i] = 0;
       }
	length = new_length;						      
    }									      

void GenArr::provide(int len) {						      
	if(len > capacity) {						      
	    if(capacity == 0)						      
		capacity = 1;						      
	    while(capacity < len)					      
		capacity *= 2;						      
	    ptr = (char **) (ptr ? psetrealloc((char *)ptr, capacity * sizeof(char*)) : 
  		 psetmalloc(capacity*sizeof(char *)));		
	    for (int i = length; i < capacity; i++) {
	       ptr[i] = 0;
	    }
	}					
			      
    }									      
char** GenArr::del(char *el)
{
    int i = search(el);
    if (i>=0)
	remove(i);
    return ptr;
}

char** GenArr::del(int pos)
{
   if((pos>=0) && pos<length)
	 remove(pos); 	
   return ptr;
}
int GenArr::binsrch(char *el,int low,int high)
{
    if(low>high)
	return -1;
    else{
	int mid = (low+high)/2;
        int i;
        if((i = strcmp(el,ptr[mid]))>0)
	   return binsrch(el,mid+1,high);
        else
	   if(i<0)
               return binsrch(el,mid+1,high);
           else
               return mid;
	}
}

int GenArr::search(char *el)
{

    for(int i=0; i<length;i++)
	if(strcmp(el,ptr[i])==0)
	  return i;
    return -1;
}	 	
void GenArr::print()
{
     for(int i=0; i<length;i++)
	msg("$1") << ptr[i] << eom;
} 	

void GenArr::sort()
{
#ifdef ISO_CPP_HEADERS
   std::qsort(ptr,length,sizeof(char *),compare_strings);
#else
   qsort(ptr,length,sizeof(char *),compare_strings);
#endif
}
 
int GenArr::compare_strings(const void *a,const void *b)
{
    char **tempa= (char **) a;
    char **tempb= (char **) b ;

  return strcmp(tempa[0],tempb[0]);
}

/*main()
{

	GenArr test_array;

	test_array.append("hello");
	test_array.append("dello");
	test_array.append("sello");
	test_array.append("pello");
	test_array.append("mello");
	test_array.print();
	test_array.sort();
	test_array.print();
}
*/




































