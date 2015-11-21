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
#include <genError.h>
#include <genEnum.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

const char *genEnum_get_name (genEnum_item *arr, int sz, int ind)
{
    Initialize(genEnum_get_name);
    Assert(arr);
    Assert(sz > 0);

    genEnum_item *larr = arr;
    for (int ii = 0 ; ii < sz ; ii++) {
	if (larr->idx == ind)
	    return larr->idx_name;

        larr ++ ;
    }
    return NULL;
}

int genEnum_get_item (genEnum_item *arr, int sz, const char * name)
{
    Initialize(genEnum_get_item);
    Assert(arr);
    Assert(name);
    Assert(sz > 0);

    genEnum_item *larr = arr;
    for (int ii = 0 ; ii < sz ; ii++) {
        if(strcmp(larr->idx_name, name) == 0)
            return larr->idx;

	larr ++ ;
    }
    return -999;
}

int genEnum_print (genEnum_item *arr, int sz, ostream& ostr)
{
    Initialize(genEnum_get_item);
    Assert(arr);
    Assert(sz > 0);
   
    ostream * pstr;
    if(&ostr == NULL)
        pstr = &cout;
    else 
        pstr = &ostr;
    genEnum_item *larr = arr;
    for (int ii = 0 ; ii < sz ; ii++) {
        (*pstr) << larr->idx_name << '\n';
	larr ++ ;
    }
    return sz;
}

/*
    $log: $
*/
