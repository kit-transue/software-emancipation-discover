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
#ifndef _ALL_H
#define _ALL_H

// to skip setfont call in constructors

#define _ASET 1

/*	This file is included in most of the C++ source files
 *	It provides class definitions for graph/node/edge and related items
 */

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <iostream.h>
#include <string.h>
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <iostream>
#include <strstream>
#include <cstring>
#include <ctime>
#endif /* ISO_CPP_HEADERS */
#include <malloc.h>

#include "point.h"
#include "slist.h"
#include "abstraction.h"
class nlist;
class elist;
class graph;
enum dimension { X, Y, Z };
#include "nodelist.h"
#include "graphlist.h"
#include "edgelist.h"
#include "graph.h"

  inline void Edge_Ignore(...){}

#endif

/*
   START-LOG--------------------------------------

   $Log: all.h  $
   Revision 1.7 2000/07/10 23:02:45EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2  1993/04/17  21:24:48  sergey
 * Added Edge_Ignore(..) to supress the warnings. Part of bug #3420.
 *
 * Revision 1.1  1992/10/10  21:33:54  builder
 * Initial revision
 *

   END-LOG----------------------------------------

*/

