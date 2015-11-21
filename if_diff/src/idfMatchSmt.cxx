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
#include <idfMatchSmt.h>
#include <idfMain.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <idfMatchSymbol.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <idfReport.h>
#include <idfSymbol.h>
#include <idfModel.h>
#include <idfLocation.h>

//============================================================================
static int find_count(idfSmt *smt_p)
{
  int count = 0;
  while (smt_p != NULL) {
    smt_p = smt_p->getNext();
    count += 1;
  }
  return count;
}

//============================================================================
static locationStruct *comparable(idfSmt *smt)
{
  if (smt == NULL) {
    return NULL;
  }
  locationList *ll = smt->getLocation();
  if (ll != NULL) {
    return ll->size() > 0 ? (*ll)[0] : NULL;
  }
  return NULL;
}

//============================================================================
// sort(idfSmt *)
//----------------------------------------------------------------------------
// Sort the smt nodes by breaking the list up into chains that are already
// sorted, then merging the whole thing.  Since the merge portion is not
// pairwise, it counts on some pre-sorted-ness in the list for efficiency.
//============================================================================
static idfSmt *sort(idfSmt *start)
{
  int n = find_count(start);

  idfSmt **chains = new idfSmt *[n];
  int chain_count = 0;

  // Get the chains.
  while (start != NULL) {
    chains[chain_count] = start;
    chain_count += 1;

    idfSmt *this_smt = start;
    locationStruct *last_comparable = comparable(start);
    for (;;) {
      idfSmt *next = this_smt->getNext();
      if (next == NULL) {
      	break;
      }
      locationStruct *comp = comparable(next);
      if (comp != NULL) {
      	if (last_comparable == NULL) {
      	  // End of non-comparable SMTs at the beginning of IF.
      	  break;
      	}
      	int order;
      	if (idf_compare_locations(last_comparable, comp, false, order)) {
      	  if (order > 0) {
      	    // The 2 are reversed from correct sequence.
      	    break;
	  }
      	}
      	last_comparable = comp;
      }
      this_smt = next;
    }
    // This is the last in a chain to be merged.
    idfSmt *next = this_smt->getNext();
    if (next != NULL) {
      // Detach the rest.
      this_smt->setNext(NULL);
      next->setPrev(NULL);
    }
    start = next;
  }
  
  // Merge chains 0...chain_count-1.
  idfSmt *new_start = NULL;
  idfSmt *new_end = NULL;
  for (;;) {
    // Find the minimum.
    int min_chain = -1;
    idfSmt *min = NULL;
    locationStruct *min_comp = NULL;
    for (int i = 0; i < chain_count; i += 1) {
      idfSmt *this_smt = chains[i];
      if (this_smt != NULL) {
      	if (min == NULL) {
      	  min_chain = i;
      	  min = this_smt;
      	  min_comp = comparable(this_smt);
      	}
      	else if (min_comp != NULL) {
      	  locationStruct *comp = comparable(this_smt);
      	  if (comp != NULL) {
      	    int comp_vs_min_comp;
      	    if (idf_compare_locations(comp, min_comp, false, comp_vs_min_comp)) {
      	      if (comp_vs_min_comp < 0) {
      	      	// This one is less.
      	      	min_chain = i;
      	      	min = this_smt;
      	      	min_comp = comp;
	      }
      	    }
      	  }
      	}
      }
    }
    if (min == NULL) {
      break;
    }
    // Remove the minimum from its chain, and append it to the new sorted list.
    idfSmt *mins_next = min->getNext();
    if (mins_next != NULL) {
      min->setNext(NULL);
      mins_next->setPrev(NULL);
    }
    chains[min_chain] = mins_next;
    if (new_start == NULL) {
      new_start = min;
    }
    if (new_end != NULL) {
      new_end->setNext(min);
      min->setPrev(new_end);
    }
    new_end = min;
  } 

  delete [] chains;
  return new_start;
}

//============================================================================
const char *get_name(idfSmt *smt, const idfModel &model)
{
  if (smt->isId()) {
    idfSymbol *sym = model.getDictionary().lookup(smt->getId());
    return sym != NULL ? sym->getName() : NULL;
  }
  return smt->getName();
}

//============================================================================
static boolean is_macro(idfSmt *smt, const idfModel &model)
{
  if (smt == NULL || !smt->isId()) {
    return false;
  }
  idfSymbol *sym = model.getDictionary().lookup(smt->getId());
  return sym != NULL && idf_strmatch(sym->getKind(), "macro");
}

//============================================================================
boolean idf_destructor_location_match(idfSmt &smt1, idfSmt &smt2,
      	      	      	              const idfModel &model1,
                                      const idfModel &model2)
{
  // Explicit destructor references can be reported variously.
  locationStruct *comp1 = comparable(&smt1);
  locationStruct *comp2 = comparable(&smt2);
  if (   !smt1.isImplicit() && !smt2.isImplicit()
      && comp1 != NULL && comp2 != NULL) {
#if 0
    cout << "Checking for destructor names." << endl;
#endif
    const char *n1 = get_name(&smt1, model1);
    const char *n2 = get_name(&smt2, model2);
#if 0
    cout <<      "n1 == " << (n1 != NULL ? n1 : "<NULL>")
         << " and n2 == " << (n2 != NULL ? n2 : "<NULL>") << "." << endl;
#endif
    if (   n1 != NULL && strchr(n1, '~') != NULL
	&& n2 != NULL && strchr(n2, '~') != NULL) {
#if 0
      genString s1;
      idf_location_str(comp1, s1);
      genString s2;
      idf_location_str(comp2, s2);
      cout << "trying for destructor match at " << s1.str() << " & " << s2.str() << endl;
#endif
      // a destructor
      if (   (   (   (comp1->end_line == 0 && comp1->end_column == 1)
                  || (comp1->end_line == comp1->start_line
                      && comp1->end_column == comp1->start_column))
	      && comp1->start_line == comp2->start_line
	      && comp1->start_column + 1 == comp2->start_column)
	  || (   (   (comp2->end_line == 0 && comp2->end_column == 1)
                  || (comp2->end_line == comp2->start_line
                      && comp2->end_column == comp2->start_column))
	      && comp2->start_line == comp1->start_line
	      && comp2->start_column + 1 == comp1->start_column)) {
	// One of the destructor references is to the ~, and
	// immediately precedes the other.
#if 0
	cout << "got destructor match" << endl;
#endif
	return true;
      }
    }
  }
  return false;
}

//============================================================================
// is_match(idfSmt *, idfSmt *, const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Compare two SMT nodes that are suspected to match, to attempt one small
// step in the process of matching the entire SMT node list.
//============================================================================
static boolean is_match(idfSmt *smt1, idfSmt *smt2,
      	      	      	const idfModel &model1, const idfModel &model2)
{
  locationStruct *comp1 = comparable(smt1);
  locationStruct *comp2 = comparable(smt2);
  genString sr1;
  idf_location_str(comp1, sr1);
  genString sr2;
  idf_location_str(comp2, sr2);
#if 0
  cout << "checking is_match at " << sr1.str() << " & " << sr2.str() << endl;
#endif
  if (comp1 == NULL || comp2 == NULL) {
    return false;
  }

  {
    // Sometimes the length is omitted on a keyword which
    // is stated explicitly.
    locationStruct loc1 = *comp1;
    locationStruct loc2 = *comp2;
    int smt1_vs_smt2;
    if (   loc1.end_line == 0 && loc1.end_column == 0
        && smt1->isKeyword() && smt1->getName() != NULL) {
      loc1.end_column = strlen(smt1->getName());
    }
    if (   loc2.end_line == 0 && loc2.end_column == 0
      	&& smt2->isKeyword() && smt2->getName() != NULL) {
      loc2.end_column = strlen(smt2->getName());
    }
    boolean comparable = idf_compare_locations(&loc1, &loc2, false, smt1_vs_smt2);
    if (comparable && smt1_vs_smt2 == 0) {
      return true;
    }
  }

  boolean have_macro = is_macro(smt1, model1) || is_macro(smt2, model2);
  boolean allow_loc_omission = have_macro
    && idf_configuration.contains("SMT of macro differs in omitting end location.");
  int smt1_vs_smt2;
  if (idf_compare_locations(comp1, comp2, allow_loc_omission, smt1_vs_smt2)) {
    if (smt1_vs_smt2 == 0) {
      return true;
    }
#if 0
    cout << "Locations are comparable." << endl;
#endif
    if (idf_destructor_location_match(*smt1, *smt2, model1, model2)) {
      return true;
    }
    return false;
  }

  // Could not get a useful comparison of locations.
  // Assume that we wouldn't be comparing these 2 nodes unless they were
  // reasonable to match up on the basis of content.  Look at content.
  if (smt1->isKeyword() != smt2->isKeyword()) {
    return false;
  }
  if (smt1->isId() != smt2->isId()) {
    return false;
  }
  const char *nm1 = smt1->getName();
  const char *nm2 = smt2->getName();
  if (nm1 != NULL && nm2 != NULL) {
    return strcmp(nm1, nm2) == 0;
  }
  return false;
}

//============================================================================
// make_one_match(idfSmt *, idfSmt *, idfSmt *&, idfSmt *&,
//     	      	  idfSymbolDictionary &, idfSymbolDictionary &)
//----------------------------------------------------------------------------
// Given two SMT lists that "ought to match", match the individual nodes
// as much as possible. Previous nodes in the SMT are already matched;
// only subsequent nodes are operated on.
//============================================================================
static boolean make_one_match(idfSmt *smt1, idfSmt *smt2,
      	      	      	      idfSmt *&match1, idfSmt *&match2,
      	      	      	      const idfModel &model1, const idfModel &model2)
{
  if (smt1 == NULL || smt2 == NULL) {
    return false;
  }

  // Look for a good match in the next few nodes.
  boolean found = false;
  const int how_far = 4;
  int outer_count = 0;

  // Search this pair of nodes and up to 3 nodes beyond,
  // searching nearer first, and giving preference to pairs that
  // are equally far along from this point.
  idfSmt *outer_try1 = smt1;
  idfSmt *outer_try2 = smt2;
  while (outer_count < how_far && !found) {
    int inner_count = 0;
    idfSmt *try1 = smt1;
    idfSmt *try2 = smt2;
    while (inner_count <= outer_count) {
      if (is_match(outer_try1, try2, model1, model2)) {
      	found = true;
      	match1 = outer_try1;
      	match2 = try2;
      }
      if (is_match(try1, outer_try2, model1, model2)) {
	found = true;
	match1 = try1;
	match2 = outer_try2;
      }
      if (try1 != NULL) {
        try1 = try1->getNext();
      }
      if (try2 != NULL) {
        try2 = try2->getNext();
      }
      inner_count += 1;
    }
    if (outer_try1 != NULL) {
      outer_try1 = outer_try1->getNext();
    }
    if (outer_try2 != NULL) {
      outer_try2 = outer_try2->getNext();
    }
    outer_count += 1;
  }

  // If that didn't succeed, try a long-distance strategy.
  
  int extent1 = 0;
  int extent2 = 0;
  while (!found && smt1 != NULL && smt2 != NULL) {
    // Push ahead through the SMT lists.
    locationStruct *loc1 = comparable(smt1);
    locationStruct *loc2 = comparable(smt2);
#if 0
    {
      genString sr1;
      idf_location_str(loc1, sr1);
      genString sr2;
      idf_location_str(loc2, sr2);
      cout << "comparing at " << sr1.str() << " & " << sr2.str() << endl;
    }
#endif
    int starts_smt1_vs_smt2;
    boolean starts_compared = idf_compare_locations(loc1, loc2, true, starts_smt1_vs_smt2);
    int smt1_vs_smt2;
    boolean compared = idf_compare_locations(loc1, loc2, false, smt1_vs_smt2);
    if (   (starts_compared && starts_smt1_vs_smt2 == 0 && is_match(smt1, smt2, model1, model2))
      	|| (compared && smt1_vs_smt2 == 0)) {
      // A match!
      found = true;
      match1 = smt1;
      match2 = smt2;
    }
    else if (compared) {
      if (smt1_vs_smt2 <= 0) {
      	// Advance smt1.
        smt1 = smt1->getNext();
      	extent1 += 1;
      }
      if (smt1_vs_smt2 >= 0) {
      	// Advance smt2.
      	smt2 = smt2->getNext();
      	extent2 += 1;
      }
    }
    else if (loc1 != NULL && idf_filename_match(idf_interesting_filename, loc1->filename)) {
      // Advance smt2.
      smt2 = smt2->getNext();
      extent2 += 1;
    }
    else if (loc2 != NULL && idf_filename_match(idf_interesting_filename, loc2->filename)) {
      // Advance smt1.
      smt1 = smt1->getNext();
      extent1 += 1;
    }
    else {      
      if (extent1 <= extent2) {
      	// Advance smt1.
        smt1 = smt1->getNext();
      	extent1 += 1;
      }
      if (extent1 > extent2) {
      	// Advance smt2.
      	smt2 = smt2->getNext();
      	extent2 += 1;
      }
      // One or both pointers have been advanced.
    }
  }
  if (found) {
    // Got one! Set the match.
    match1->setMatch(match2);
    match2->setMatch(match1);
  }
  return found;
}

//============================================================================
// match_list(const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Given two models with SMT lists that "ought to match", match the individual nodes
// as much as possible. 
//
// Return the number of matches made.
//============================================================================
static int match_list(const idfModel &model1, const idfModel &model2)
{
  idfSmt *smt1 = model1.getSmt();
  idfSmt *smt2 = model2.getSmt();
  idfSmt *match1;
  idfSmt *match2;
  int count = 0;
  while (make_one_match(smt1, smt2, match1, match2, model1, model2)) {
    smt1 = match1->getNext();
    smt2 = match2->getNext();
    count += 1;
  }
  return count;
}

//============================================================================
// match_symbols(const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Go through the matched SMT of model1 and process corresponding symbols.
//============================================================================
static void match_symbols(const idfModel &model1, const idfModel &model2)
{
  idfSmt *smt = model1.getSmt();
  for (; smt != NULL; smt = smt->getNext()) {
    idfSmt *match = smt->getMatch();
    if (match != NULL) {
      if (smt->isId() && match->isId()) {
      	idf_try_symbol_match(smt->getId(), match->getId(), model1, model2);
      }
    }
  }
}

//============================================================================
// match_files(idfModel &, idfModel &)
//----------------------------------------------------------------------------
// Go through the matched SMT of model1 and process corresponding files.
//============================================================================
static void match_files(idfModel &model1, idfModel &model2)
{
  idfSmt *smt = model1.getSmt();
  for (; smt != NULL; smt = smt->getNext()) {
    idfSmt *match = smt->getMatch();
    if (match != NULL) {
      if (smt->isId() && match->isId()) {
      	idf_try_file_match(smt->getLocation(), match->getLocation(), model1, model2);
      }
    }
  }
}

//============================================================================
int idf_match_smt(idfModel &model1, idfModel &model2)
{
  model1.setSmt(sort(model1.getSmt()));
  model2.setSmt(sort(model2.getSmt()));

  int count = match_list(model1, model2);

  if (count > 0) {
    match_symbols(model1, model2);
    match_symbols(model2, model1);

    match_files(model1, model2);
    match_files(model2, model1);
  }
  return count;
}
