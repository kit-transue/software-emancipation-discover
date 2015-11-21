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
//  checkpmod.C  
//      functions to do self-consistency check of a single pmod



/*  Consistency check utility:       9/13/93
    
Major Goal:
    Self-consistency check for a *single* pmod (  sym & ind, of course)
    User-runnable test that indicates any possible inconsistency that
       we know how to test for.

Operation:
    User invokes it by an optional switch on paraset -batch
    User should not need write permissions to run this
    Output is in the form of error messages.  For some errors, it is acceptable
       that the test gives "Diagnostics" messages.  But where the error is confined
       to the information on a particular module, it is desirable that the module be
       named.
    Output for a good pmod should be minimal, to make it easy to spot errors
    
Tests performed:       
  A. On file pair as a whole:
    * header fields of sym and ind file - test them for consistency with each other,
       and for agreement with actual file sizes
    * incorrect hash value for symbol:  For each symbol pointed to by the ind file,
       make sure the hash agrees with the symbol referenced
    * multiple index entries pointing to same symbol:  make sure that two or more
       index entries do not point to the same or overlapping symbols
    * file memory usage (overlap is an error, unused is a statistic that
       indicates waste, a need to run tidy, or a bug)
  B. On symbol portion of the file pair
    * closed-ness:  Make sure all symbols can be reached by searching links from
       those DD_MODULE symbols pointed to by the index file
    * symbol validity:
        no_links field versus number of actual links
	name_length field versus actual size of null-terminated name
	has_def_file field versus presence of exactly one is_defined_in link
        kind field, compared to the enumeration of valid kinds
	name field, characters compared to valid ascii (what about international?)
	for non-foreign modules, check if logical name is in name space of current pdf
  C. On list of links for each symbol:
    * invalid numeric links:  Each numeric link type has a legal range possible
    * reverse link:  Some types of links must have matching reverse links
    * link type:  link type field must be one of the valid values
    * link duplication:  some link types are limited to only one instance
        per symbol
    * duplicate links with same target and type:  illegal
    * invalid link:  this particular link not valid with this combination of source
        and destination target kinds    (added 5/12/94)
    
    ********* Known limitations of the checkpmod logic ***********


1) Checkpmod never looks outside of the pair of pmod files
          (eg.  sys.dd.pmod.sym
                sys.dd.pmod.ind)
  for its testing. It does not compare against source code,
  against the existence of actual files, against psets, nor
  against other pmods.
  --note:  starting 5/12/94, it checks each file symbol for validity
  against pdf, and for the existence of the actual file

2) Checkpmod does not test whether information in a pmod
   should logically have been included in a different pmod
   instead.  For example, assume we are building a pmod for 
   subproject A, containing file a.c and a.h, and a different
   pmod for file  b.c and b.h.

   Assume that file a.c includes b.h.  Pmod for subproject A
   should contain the name of b.h, so that it can indicate the
   include relationship.  But it should not have any of the
   classes or functions defined in b.h that are not also 
   referenced in file a.c.  Those should go in the pmod for
   subproject B.

   Similarly, subproject B should not indicate the include
   relationship. (but it does at present, see bug7266)
   
   Any particular such test could be added to checkpmod, but
   I don't see a general test that could be reasonably
   added for the existence of extra symbols and links.

3. Checkpmod does not check whether names that correspond
   to specific syntactic elements of C and C++ actually are
   valid strings for that purpose.  For example, C++ does not
   permit the character % inside a function name. Checkpmod
   does not test for that.

4. Checkpmod does not test for whether certain relationships
   described in the pmod are illegal in C or C++.  For example,
   A function may be defined inside another function in Pascal, but
   not in C or C++.  On the other hand, a class may be defined
   inside another class.  Checkpmod does not make such a distinction.
   note:  starting 5/12/94, it checks against a simple matrix, looking
   at link type, and validating that the link type is valid for the
   particular source symbolkind, and destination symbolkind.
   
   
There may be other things that Checkpmod does not test for, but these
are the ones currently known.   3/21/94

Log file support removed as per request from Smit (bug no. 9289). All error
output goes to cerr; all other output goes to cout.

                                                 ljb, 05/15/1995

*/

#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <driver_mode.h>
#include <genTmpfile.h>
#include <genWild.h>
#include <fileCache.h>

#include <xref.h>
#include <xref_priv.h>

#include <RTL_externs.h>
#include <proj.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>
#include <Question.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <checkpmod.h>

#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */
#define  SEEK_END  2

//  Here is where the interactive set-home calls this logic,
//    maybe from  ui/browser/browser_callbacks.C,  function PUSHBUTTONCB(project_home_project)

//  array of flags indicating which linkTypes should be symmetrical
//     These get initialized in the testpmod constructor, and used
//     by the function   testpmod::check_back_link()
#define MAX_NUM_LINKS 128
static  bool link2way[MAX_NUM_LINKS];   
