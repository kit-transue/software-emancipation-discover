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
#include <idfLocation.h>
#include <idfAst.h>
#include <idfSmt.h>
#include <idfSymbol.h>
#include <idfDict.h>
#include <locationList.h>
#include <elsMapper.h>
#include <idfMain.h>
#include <idfModel.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <idfReport.h>

//============================================================================
// Manage the variety of location representations.
//
// The IF parser's locationStruct reflects the syntax that has been parsed.
// The locationStruct contains
//   start_line                                        sl
//   start_column                                          sc
//   end_line                                                  el
//   end_column                                                    ec
//                                                    
//   bytestart                               s         0   s   0   0
//   bytemap ::= bytestart bytelen           s l       0   s   0   l
//   linecolstart                            s/s       s   s   0   0
//   linecolmap ::= linecolstart bytelen     s/s l     s   s   0   l
//   linecolmap ::= linecolstart linecolend  s/s e/e   s   s   e   e
//
// These are distinguishable, except that a byte length of 0 is ambiguous
// with an absent byte length.
//============================================================================

//============================================================================
extern void idf_set_file(locationStruct *loc, idfModel &model)
{
  if (loc != NULL) {
    const char *fn = loc->filename;
    if (fn != NULL) {
      model.getFiles().add(fn, NULL);
    }
  }
}

//============================================================================
static void filter_locations(locationList *loc_list, idfModel &model,
                             void (*filt)(locationStruct *, idfModel &))
{
  if (loc_list != NULL) {
    for (int i = loc_list->size(); i > 0;) {
      i -= 1;
      filt((*loc_list)[i], model);
    }
  }
}

//============================================================================
static void filter_locations(const idfAstSiblings &ast, idfModel &model,
                             void (*filt)(locationStruct *, idfModel &))
{
  int n = ast.getNumber();
  for (int i = 0; i < n; i += 1) {
    idfAst &node = *ast.getSibling(i);
    filter_locations(node.getLocation(), model, filt);
    filter_locations(node.getChildren(), model, filt);
  }
}

//============================================================================
static void filter_locations(const idfAstDictionary &ast, idfModel &model,
                             void (*filt)(locationStruct *, idfModel &))
{
  for (idfIterator iter = ast.first(); !ast.done(iter); ast.next(iter)) {
    filter_locations(*ast.lookup(iter), model, filt);
  }
}

//============================================================================
// idf_filter_locations(idfModel &)
//----------------------------------------------------------------------------
// Set up the model's file dictionary.
//============================================================================
void idf_filter_locations(idfModel &model, void (*filt)(locationStruct *, idfModel &))
{
  for (idfSmt *smt = model.getSmt(); smt != NULL; smt = smt->getNext()) {
    filter_locations(smt->getLocation(), model, filt);
  }

  const idfSymbolDictionary &dict = model.getDictionary();
  for (idfIterator iter = dict.first(); !dict.done(iter); dict.next(iter)) {
    filt((location *)dict.lookup(iter)->getLocation(), model);
  }

  if (model.getAst() != NULL) {
    filter_locations(*model.getAst(), model, filt);
  }
}

//============================================================================
void idf_try_file_match(locationList *loc_list1, locationList *loc_list2,
                        const idfModel &model1, const idfModel &model2)
{
  if (loc_list1 != NULL && loc_list2 != NULL) {
    int len1 = loc_list1->size();
    int len2 = loc_list2->size();
    for (int i = 0; i < len1 && i < len2; i += 1) {
      idf_try_file_match((*loc_list1)[i], (*loc_list2)[i], model1, model2);
    }
  }
}

//============================================================================
static void enter_match(const char *fn, const char*match,
                        const idfModel &model, const idfModel &model2)
{
  idfStringStringDictionary &dict = model.getFiles();
  idfStringStringDictionary &dict2 = model2.getFiles();
  if (dict.contains(fn) && dict.lookup(fn) == NULL && dict2.contains(match)) {
    dict.reassign(fn, match);
  }
}

//============================================================================
void idf_try_file_match(const locationStruct *loc1, const locationStruct *loc2,
                        const idfModel &model1, const idfModel &model2)
{
  if (loc1 != NULL && loc2 != NULL) {
    if (idf_filename_match(loc1->filename, loc2->filename)) {
      // We consider this a valid match!
      // Enter it into the models.
      enter_match(loc1->filename, loc2->filename, model1, model2);
      enter_match(loc2->filename, loc1->filename, model2, model1);
    }
  }
}

//============================================================================
// Put the end lines and columns into a consistent form for comparison.
//============================================================================
static void canonical_end(const locationStruct &loc, int &end_line, int &end_column)
{
  end_line = loc.end_line == 0 ? loc.start_line : loc.end_line;
  end_column = loc.end_line == 0 ? loc.start_column - 1 + loc.end_column
                                 : loc.end_column;
}

//============================================================================
// Create a location which represents the union of the two locations.
// Fail and return false if both are NULL, or if they come from different
// files (by simple name match).
//
// If the two ranges do not overlap, the a location (range) is returned,
// with the gap filled in.
//============================================================================
boolean idf_location_union(const locationStruct *loc1,
                           const locationStruct *loc2, locationStruct &loc)
{
  if (loc1 == NULL && loc2 == NULL) {
    return false;
  }
  if (loc1 == NULL) {
    loc = *loc2;
    return true;
  }
  else if (loc2 == NULL) {
    loc = *loc1;
    return true;
  }
  if (!idf_strmatch(loc1->filename, loc2->filename)) {
    return false;
  }
  loc.filename = loc1->filename;

  if ((loc1->start_line == 0) != (loc2->start_line == 0)) {
    // Start locations are not comparable since one is byte-based and
    // the other is row/column based.
    return false;
  }
  loc.start_line = loc1->start_line < loc2->start_line ? loc1->start_line
                                                       : loc2->start_line;
  if (loc1->start_line == loc2->start_line) {
    loc.start_column = loc1->start_column < loc2->start_column ? loc1->start_column
                                                               : loc2->start_column;
  }
  else if (loc1->start_line < loc2->start_line) {
    loc.start_column = loc1->start_column;
  }
  else {
    loc.start_column = loc2->start_column;
  }

  // Put the end lines and columns into a consistent form for comparison.
  int end_line1;
  int end_column1;
  int end_line2;
  int end_column2;
  canonical_end(*loc1, end_line1, end_column1);
  canonical_end(*loc2, end_line2, end_column2);
  if ((end_line1 == 0) != (end_line2 == 0)) {
    // End locations are not comparable since one is byte-based and
    // the other is row/column based.
    return false;
  }

  // Find the max end line & column.
  loc.end_line = end_line1 > end_line2 ? end_line1 : end_line2;
  if (end_line1 == end_line2) {
    loc.end_column = end_column1 > end_column2 ? end_column1 : end_column2;
  }
  else if (end_line1 > end_line2) {
    loc.end_column = end_column1;
  }
  else {
    loc.end_column = end_column2;
  }

  // Express the end line and column consistently with the input.
  if (loc.end_line == loc.start_line && (loc1->end_line == 0 || loc2->end_line == 0)) {
    loc.end_line = 0;
    loc.end_column -= (loc.start_column - 1);
  }
  return true;
}

//============================================================================
boolean idf_endless(const locationStruct &loc)
{
  return loc.end_line == 0 && loc.end_column == 0;
}

#if 0
No need to copy file name because the parser delivers only
file names from its permanent cache.  Also, the mapper counts
on the file names coming from the cache, so copying can cause
bugs.

static const char *last_filename = NULL;
//============================================================================
// copy_filename(const char *)
//----------------------------------------------------------------------------
// Avoid having the filename stored a zillion times in memory, by
// reusing the same storage, if the filename occurs successively.
//============================================================================
static const char *copy_filename(const char *fn1)
{
  const char *fn2 = NULL;
  if (fn1 != NULL) {
    if (last_filename != NULL && strcmp(fn1, last_filename) == 0) {
      return last_filename;
    }
    last_filename = fn2 = idf_copy_string(fn1);
  }
  return fn2;
}
#endif

//============================================================================
static void locstr(const locationStruct *l1, genString &str)
{
  genString lns;
  idf_location_str(l1, lns);
  const char *nm = NULL;
  if (l1 != NULL) {
    nm = l1->filename;
  }
  if (nm == NULL) {
    nm = "<no filename>";
  }
  str.printf("%s: %s", nm, lns.str());
}

//============================================================================
static boolean unmap_location(locationStruct &loc)
{
  locationStruct l_temp = loc;
#if 0
  genString ls1x;
  locstr(loc, ls1x);
  cout << "mapping location " << ls1x.str() << endl;
#endif
  locationStruct *l_temp2;
  int goodness = elsMapper::getTrueLoc(&l_temp, &l_temp2);
#if 0
  genString ls1;
  locstr(&l_temp, ls1);
  genString ls2;
  locstr(l_temp2, ls2);
  cout << "mapped location " << ls1.str() << " to " << ls2.str()
       << " with goodness " << goodness << endl;
#endif
#if 0
      	cout << "loc, l_temp, l_temp2: " << (void *)&loc << (void *)&l_temp
             << (void *)l_temp2 << endl;
#endif
  if (goodness > 0) {
    // unmapped successfully
    loc = *l_temp2;
    delete l_temp2;
    return true;
  }
  return false;
}

//============================================================================
locationStruct *idf_copy_location_struct(const locationStruct *l1, boolean map)
{
  locationStruct *l2 = NULL;
  if (l1 != NULL) {
    l2 = new locationStruct;
    if (l2 != NULL) {
      *l2 = *l1;
      // unmap_location(*l2);
      // l2->filename = (char *)copy_filename(l2->filename);
    }
  }
  return l2;
}

//============================================================================
extern void idf_unmap_location(locationStruct *loc, idfModel &model)
{
  if (loc != NULL) {
    unmap_location(*loc);
    // loc->filename = (char *)copy_filename(loc->filename);
  }
}

//============================================================================
locationList *idf_copy_location_list(locationList *ll1, boolean map)
{
  locationList *ll2 = NULL;
  if (ll1 != NULL) {
    ll2 = new locationList;
    if (ll2 != NULL) {
#if 0
      for (int i = ll1->length(); i > 0;) {
      	i -= 1;
      	ll2->InsertFirst(idf_copy_location_struct((*ll1)[i], map));
      }
#else
      for (int i = 0; i < ll1->size(); i += 1) {
        InsertLastIntoLocationList(ll2, idf_copy_location_struct((*ll1)[i], map));
      }
#endif
    }
  }
  return ll2;
}

//============================================================================
boolean idf_filename_match(const char *n1, const char *n2)
{
  if (n1 == n2) {
    return true;
  }
  if (n1 == NULL || n2 == NULL) {
    return false;
  }

  // If the names match at the end, consider it a match.
  // For example, /vol1/pset/src/myfile.C would match ./src/myfile.C.
  int len1 = strlen(n1);
  int len2 = strlen(n2);
  const char *p1 = n1 + len1;
  const char *p2 = n2 + len2;
  for (;;) {
    boolean punct1 = p1 == n1; // The next char is like punctuation if it's not there.
    boolean punct2 = p2 == n2;
    char c1 = punct1 ? '\0' : *--p1;
    char c2 = punct2 ? '\0' : *--p2;
    punct1 = punct1 || c1 == '/' || c1 == '\\';
    punct2 = punct2 || c2 == '/' || c2 == '\\';
    if (punct1 != punct2) {
      // We have a difference where one name is longer than another.
      return false;
    }
    if (!punct1) {
      // Both are part of a name.
      if (c1 != c2) {
      	return false;
      }
    }
    else {
      // Both are punctuation. If either has reached the beginning, done.
      if (   p1 == n1 || p2 == n2
      	  || (p1 == n1 + 1 && *n1 == '.') || (p2 == n2 + 1 && *n2 == '.')) {
      	return true;
      }
    }
  }      	
}

//============================================================================
// If l1 has a end_line == 0 and it can be compared in numeric position
// to l2 which has a non-zero end_line, then return true and set order
// to compare to 0 as the end of l1 compares to the end of l2.
//
// Note that l1.start_line must be nonzero, or else l1 is not line/col oriented.
// Note that l1.end_column must be > 0 to indicate that an end is present.
//============================================================================
boolean idf_compare_zero_end_line(const locationStruct &l1, const locationStruct &l2, int &order)
{
  if (l1.end_line == 0 && l2.end_line != 0 && l1.start_line != 0 && l1.end_column > 0) {
    order = 0;
    if (l1.start_line != l2.end_line) {
      order = (l1.start_line < l2.end_line ? -1 : 1);
    }
    int end1_column = l1.start_column + l1.end_column - 1;
    if (end1_column != l2.end_column) {
      order = (end1_column < l2.end_column ? -1 : 1);
    }
    return true;
  }
  else {
    return false;
  }
}

//============================================================================
// idf_compare_locations(const locationStruct *, const locationStruct *,
//                       boolean ignore_omissions, int &order)
//----------------------------------------------------------------------------
// Compare two locations, determining their trichotomous order.
// Returns false if they are not comparable due to one being line/column
// and the other byte position, etc.
// Sets 'order' to compare with 0 as l1 compares with l2.
// The ordering of the start of the range overrides any considerations of
// the ordering of the end.
// !!! Currently, if the starts are equal then the ordering is decided
// directly by the end positions. But this isn't necessarily what's
// appropriate.
//
// Call this only for purposes of matching nodes.  When it's time to
// report differences in positions, you need to report file name differences
// rather than fudge them.
//
// If ignore_omissions is true, then a location that omits length
// will compare equal to a location that has a length.
//============================================================================
boolean idf_compare_locations(const locationStruct *l1,
      	      	      	      const locationStruct *l2,
      	      	      	      boolean ignore_omissions, int &order)
{
  if (l1 == NULL || l2 == NULL) {
    return false;
  }
  if (!idf_filename_match(l1->filename, l2->filename)) {
    return false;
  }
  // The IF parser's locationStruct reflects the syntax that has been parsed.
  // The locationStruct contains
  //   start_line                                        sl
  //   start_column                                          sc
  //   end_line                                                  el
  //   end_column                                                    ec
  //                                                    
  //   bytestart                               s         0   s   0   0
  //   bytemap ::= bytestart bytelen           s l       0   s   0   l
  //   linecolstart                            s/s       s   s   0   0
  //   linecolmap ::= linecolstart bytelen     s/s l     s   s   0   l
  //   linecolmap ::= linecolstart linecolend  s/s e/e   s   s   e   e
  
  if (l1->start_line < 0 || l2->start_line < 0) {
    return false;
  }
  if ((l1->start_line == 0) != (l2->start_line == 0)) {
    return false;
  }
  if (l1->start_line != l2->start_line) {
    order = l1->start_line < l2->start_line ? -1 : 1;
    return true;
  }
  if (l1->start_column != l2->start_column) {
    order = l1->start_column < l2->start_column ? -1 : 1;
    return true;
  }
  if (ignore_omissions) {
    if (   (l1->end_line == 0 && l1->end_column == 0)
      	|| (l2->end_line == 0 && l2->end_column == 0)) {
      order = 0;
      return true;
    }
  }
  if ((l1->end_line == 0) != (l2->end_line == 0)) {
    if (idf_compare_zero_end_line(*l1, *l2, order)) {
      return true;
    }
    else if (idf_compare_zero_end_line(*l2, *l1, order)) {
      order = -order;
      return true;
    }
    return false;
  }
  if (l1->end_line != l2->end_line) {
    order = l1->end_line < l2->end_line ? -1 : 1;
    return true;
  }
  order = l1->end_column < l2->end_column ? -1 :
	  l1->end_column == l2->end_column ? 0 : 1;
  return true;
}

//============================================================================
// idf_location_precedes(const locationStruct *, const locationStruct *, int &order)
//----------------------------------------------------------------------------
// Compare two locations, determining whether the first precedes the second.
// Returns false if they are not comparable due to one being line/column
// and the other byte position, etc.
// Sets 'order' to compare with 0 as the end of l1 compares with 
// the start of l2. (0 means that they meet but don't overlap.)
//
// An omitted end location is handled as if the length were 0 bytes.
//============================================================================
boolean idf_location_precedes(const locationStruct *l1,
      	      	      	      const locationStruct *l2,
      	      	      	      int &order)
{
  if (l1 == NULL || l2 == NULL) {
    return false;
  }
  if (!idf_filename_match(l1->filename, l2->filename)) {
    return false;
  }
  if (l1->start_line < 0 || l2->start_line < 0) {
    return false;
  }

  // Put the end lines and columns into a consistent form for comparison.
  int end_line1;
  int end_column1;
  canonical_end(*l1, end_line1, end_column1);
  if ((end_line1 == 0) != (l2->start_line == 0)) {
    // Locations are not comparable since one is byte-based and
    // the other is row/column based.
    return false;
  }

  if (end_line1 < l2->start_line) {
    order = -1;
  }
  else if (end_line1 == l2->start_line) {
    int start_column2 = l2->start_column - 1;
    order = end_column1 < start_column2 ? -1 : (end_column1 == start_column2 ? 0 : 1);
  }
  else {
    order = 1;
  }
  return true;
}
