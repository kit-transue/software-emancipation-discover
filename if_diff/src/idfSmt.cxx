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
#include <idfSmt.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <general.h>
#include <idfMain.h>
#include <idfSymbol.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <idfLocation.h>

//============================================================================
idfSmt::idfSmt(const char *nm, locationList *l, boolean map,
               const idfModel &m)
  : id(0)
  , implicit(false)
  , id_flag(false)
  , keywordName(idf_copy_string(nm))
  , location(idf_copy_location_list(l, map))
  , next(NULL)
  , prev(NULL)
  , match(NULL)
  , discrepancyReportedFlag(false)
  , model(&m)
{
}
  
//============================================================================
idfSmt::idfSmt(unsigned int ident, boolean implct, locationList *l,
               boolean map, const idfModel &m)
  : id(ident)
  , implicit(implct)
  , id_flag(true)
  , keywordName(NULL)
  , location(idf_copy_location_list(l, map))
  , next(NULL)
  , prev(NULL)
  , match(NULL)
  , discrepancyReportedFlag(false)
  , model(&m)
{
}

//============================================================================
idfSmt::~idfSmt()
{
  delete [] ((char *)keywordName);
  delete location;
}

//============================================================================
const char *idfSmt::getName()
{
  if (isId() && idf_current_symbols != NULL) {
    idfSymbol *sym = idf_current_symbols->lookup(getId());
    return sym != NULL ? sym->getName() : NULL;
  }
  else if (keywordName != NULL) {
    return keywordName;
  }
  else {
    // !!! could look up keyword name from the file at this point
    return NULL;
  }
}
