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
#include <idfAst.h>
#include <idfMain.h>
#include <astnodeList.h>
#include <idfLocation.h>
#include <idfModel.h>

//============================================================================
// Since all astNodeStructs are processed and disposed of during the IF parse
// phase, there's no need for a version of this constructor that doesn't
// map locations back to the original reference.
//============================================================================
idfAst::idfAst(astnodeStruct *ast, idfAst *par, const idfModel &m)
  : kind(idf_copy_string(ast->node))
  , identifier(ast->identifier)
  , value(idf_copy_string(ast->value))
  , isGen(ast->generated)
  , location(idf_copy_location_list(ast->location, true))
  , discrepancyReportedFlag(false)
  , containsDiscrepancyFlag(false)
  , parent(par)
  , children(ast->children, this, m) // Constructs and hooks together the children.
  , nextSib(NULL)
  , prevSib(NULL)
  , match(NULL)
  , hasSubtreeLocation(false)
{
  determineSubtreeLocation();
}

//============================================================================
// Constructs and hooks together children.
//============================================================================
idfAstSiblings::idfAstSiblings(astnodeList *list, idfAst *parnt,
      	      	      	       const idfModel &m)
  : first(NULL)
  , hasSubtreeLocation(false)
  , model(&m)
{
  // Construct the corresponding nodes, and hook them together.
  if (list != NULL) {
    int len = list->size();
    idfAst *prev_child = NULL;
    for (int i = 0; i < len; i += 1) {
      astnodeStruct *child_struct = (*list)[i];
      idfAst *child = child_struct != NULL ? new idfAst(child_struct, parnt, m) : NULL;
      if (prev_child != NULL) {
	prev_child->nextSib = child;
      }
      else {
	first = child;
      }
      if (child != NULL) {
	child->prevSib = prev_child;
      }
      prev_child = child;
    }
  }
  determineSubtreeLocation();
}

//============================================================================
idfAst::~idfAst()
{
  delete [] kind;
  delete [] value;
  delete location;

  // Children destroyed implicitly.
}

//============================================================================
idfAstSiblings::~idfAstSiblings()
{
  // Destroy children.
  int n = getNumber();
  if (n > 0) {
    idfAst *sib = getSibling(0);
    while (sib != NULL) {
      idfAst *next = sib->nextSib;
      delete sib;
      sib = next;
    }
  }
}

//============================================================================
// Determine a location for the siblings.
//============================================================================
void idfAstSiblings::determineSubtreeLocation()
{
  boolean have_start = false;
  int num_children = getNumber();
  int num_inspected = 0;
  while (!have_start && num_inspected < num_children) {
    idfAst *child = getSibling(num_inspected);
    locationStruct child_loc;
    if (child->getSubtreeLocation(child_loc)) {
      have_start = true;
      subtreeLocation.filename = child_loc.filename;
      subtreeLocation.start_line = child_loc.start_line;
      subtreeLocation.start_column = child_loc.start_column;
    }
    num_inspected += 1;
  }
  
  if (!have_start) {
    hasSubtreeLocation = false;
    return;
  }

  // Find end line, if possible.
  num_inspected = 0;
  boolean have_end = false;
  subtreeLocation.end_line = 0;
  subtreeLocation.end_column = 0;
  while (!have_end && num_inspected < num_children) {
    idfAst *child = getSibling(num_children - num_inspected - 1);
    locationStruct child_loc;
    if (child->getSubtreeLocation(child_loc) && !idf_endless(child_loc)) {
      if (!idf_strmatch(subtreeLocation.filename, child_loc.filename)) {
        hasSubtreeLocation = false;
      	return;
      }
      have_end = true;
      if (child_loc.end_line != 0) {
      	subtreeLocation.end_line = child_loc.end_line;
      	subtreeLocation.end_column = child_loc.end_column;
      }
      else if (subtreeLocation.start_line == child_loc.start_line) {
      	// can continue to use a byte length
      	subtreeLocation.end_line = 0;
      	subtreeLocation.end_column = child_loc.end_column + (child_loc.start_column - subtreeLocation.start_column);
      }
      else {
      	// translate end byte length to line/column
      	subtreeLocation.end_line = child_loc.start_line;
      	subtreeLocation.end_column = child_loc.start_column + child_loc.end_column - 1;
      }
    }
    num_inspected += 1;
  }
  hasSubtreeLocation = true;
}

//============================================================================
boolean idfAstSiblings::getSubtreeLocation(locationStruct &loc) const
{
  if (hasSubtreeLocation) {
    loc = subtreeLocation;
  }
  return hasSubtreeLocation;
}

//============================================================================
// Determine a location that considers the children as well as this node.
//============================================================================
void idfAst::determineSubtreeLocation()
{
  locationStruct *comp1 = getComparableLocation();
  locationStruct loc;
  locationStruct *comp2 = children.getSubtreeLocation(loc) ? &loc : NULL;
  hasSubtreeLocation = idf_location_union(comp1, comp2, subtreeLocation);
  if (!hasSubtreeLocation && comp1 != NULL) {
    loc = *comp1;
    hasSubtreeLocation = true;
  }
}

//============================================================================
boolean idfAst::getSubtreeLocation(locationStruct &loc) const
{
  if (hasSubtreeLocation) {
    loc = subtreeLocation;
  }
  return hasSubtreeLocation;
}

//============================================================================
int idfAstSiblings::getNumber() const
{
  int n = 0;
  for (idfAst *sib = first; sib != NULL; sib = sib->nextSib) {
    n += 1;
  }
  return n;
}

//============================================================================
// Return the n-th child of this AST node, 0-indexed.
//============================================================================
idfAst *idfAstSiblings::getSibling(int n) const
{
  if (n < 0) {
    return NULL;
  }
  for (idfAst *sib = first; sib != NULL; sib = sib->nextSib) {
    if (--n < 0) {
      return sib;
    }
  }
  return NULL;
}

//============================================================================
locationStruct *idfAst::getComparableLocation() const
{
  locationList *ll = getLocation();
  if (ll != NULL) {
    return ll->size() > 0 ? (*ll)[0] : NULL;
  }
  return NULL;
}

//============================================================================
const char *idfAst::getKind() const
{
  return kind;
}

//============================================================================
unsigned int idfAst::getIdentifier() const
{
  return identifier;
}

//============================================================================
const char *idfAst::getValue() const
{
  return value;
}

//============================================================================
boolean idfAst::isGenerated() const
{
  return isGen;
}

//============================================================================
idfAst *idfAst::getParent() const
{
  return parent;
}
