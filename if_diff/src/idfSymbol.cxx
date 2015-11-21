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
#include <idfSymbol.h>
#include <idfDict.h>
#include <idfMain.h>
#include <idfLocation.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#include <cctype>
#include <cstdio>
#endif /* ISO_CPP_HEADERS */

//============================================================================
//============================================================================
idfSymbol::idfSymbol(int i, const idfModel &m)
  : match(NULL)
  , discrepancyReportedFlag(false)
  , name(NULL)
  , kind(NULL)
  , location(NULL)
  , generated(false)
  , ident_num(i)
  , refBeforeDef(true)
  , localName(NULL)
  , definingFunction(NULL)
  , blockId(0)
  , hasBlockId(false)
  , model(&m)
{
  relations = new idfRelationDictionary;
  attributes = new idfAttributeDictionary;
}

//============================================================================
//============================================================================
idfSymbol::idfSymbol(int i, const char *n, const char *k,
                     const struct location *loc, boolean g, boolean map,
      	      	     const idfModel &m)
  : name(NULL), kind(NULL), location(NULL), generated(false)
  , discrepancyReportedFlag(false)
  , match(NULL)
  , ident_num(i)
  , refBeforeDef(false)
  , localName(NULL)
  , definingFunction(NULL)
  , blockId(0)
  , hasBlockId(false)
  , model(&m)
{
  relations = new idfRelationDictionary;
  attributes = new idfAttributeDictionary;
  set(n, k, loc, g, map);
}

//============================================================================
//============================================================================
idfSymbol::~idfSymbol()
{
  delete relations;
  delete attributes;
  delete [] ((char *)name);
  delete [] ((char *)kind);
  delete [] ((char *)localName);
  delete [] ((char *)definingFunction);
  delete location;
}

//============================================================================
// A symbol should only have its identifying characteristics set once,
// so this returns false if they have already been set.
//============================================================================
boolean idfSymbol::set(const char *nm, const char *knd,
                       const struct location *loc, boolean gen,
      	      	       boolean map)
{
  generated = gen;
  if (name != NULL || kind != NULL || location != NULL) {
    return false;
  }
  if (nm != NULL) {
    name = new char[strlen(nm) + 1];
    strcpy(((char *)name), nm);
    setLocalName();
  }
  if (knd != NULL) {
    kind = new char[strlen(knd) + 1];
    strcpy(((char *)kind), knd);
  }
  if (loc != NULL) {
    location = idf_copy_location_struct(loc, map);
  }
  return true;
}

//============================================================================
boolean idfSymbol::getLocalVariableName(const char *&nm,
				        const char *&fn_name,
				        const unsigned int *&block_id) const
{
  nm = localName;
  fn_name = definingFunction;
  block_id = hasBlockId ? &blockId : NULL;
  return (localName != NULL);
}

//============================================================================
// Allocate a null-terminated string that duplicates the character
// array starting at the given location and having the given length.
//============================================================================
static char *char_array_dup(const char *p, int len)
{
  if (p == NULL || len < 0) {
    return NULL;
  }
  char *d = new char[len + 1];
  if (d != NULL) {
    strncpy(d, p, len);
    d[len] = '\0';
  }
  return d;
}

//============================================================================
// Find the extent of white space at the end of a string.
// Given a pointer 1 beyond the end of a character array,
// check for trailing white space characters and return a pointer
// to the trailing white space.
//============================================================================
static char *trailing_white_space(char *begin, char *end)
{
  while (end > begin && isspace(end[-1])) {
    end -= 1;
  }
  return end;
}

//============================================================================
// Find the extent of white space at the start of a null-terminated string.
// Check for initial white space characters and return a pointer
// just after the white space.
//============================================================================
static char *leading_white_space(char *begin)
{
  while (isspace(*begin)) {
    begin += 1;
  }
  return begin;
}

//============================================================================
// Analyze "name" for the syntax used for local variables.
// Set localName to NULL if and only if the '@' syntax was not used.
//============================================================================
void idfSymbol::setLocalName()
{
  localName = NULL;
  if (name != NULL) {
    char *at = strchr(name, '@');
    if (at != NULL) {
      // Allocate and set localName.
      char *end = trailing_white_space((char *)name, at);
      localName = char_array_dup(name, end - name);

      // Find the function name.
      at = leading_white_space(at);
      char *brace = strchr(at, '{');
      end = (brace != NULL) ? trailing_white_space(at, brace)
                     	    : strchr(at, '\0');
      definingFunction = char_array_dup(at, end - at);

      // Find the block id.
      hasBlockId = false;
      blockId = 0;
      if (brace != NULL) {
      	char *rbrace = strrchr(brace, '}');
      	if (rbrace != NULL) {
      	  unsigned int i;
      	  int n = sscanf(brace, "{%u}", &i);
      	  if (n == 1) {
      	    hasBlockId = true;
      	    blockId = i;
      	  }
      	}
      }
    }
  }
}

