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
#include <idfMatchAst.h>
#include <idfMain.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <idfSymbol.h>
#include <idfMatchSymbol.h>
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#endif /* ISO_CPP_HEADERS */
#include <idfReport.h>
#include <genString.h>
#include <idfModel.h>
#include <idfLocation.h>

static boolean match_ast(idfAst *ast1, idfAst *ast2, 
	                 const idfModel &model1, const idfModel &model2,
      	      	      	 boolean *matched_among_children = NULL);
static boolean get_location(idfAst *ast, locationStruct &loc);

//============================================================================
static boolean SS_no_match(idfAst *ast1, idfAst *ast2)
{
  if (ast1 == NULL || ast2 == NULL) {
    return true;
  }
  const char *kind1 = ast1->getKind();
  const char *kind2 = ast2->getKind();
  boolean no_match = false;
  idfStringSet *ssp = idf_configuration.lookup("AST kinds don't match().");
  if (ssp != NULL && kind1 != NULL && kind2 != NULL)
  {
    genString s;
    s.printf("%s<%s", kind1, kind2);
    genString sr;
    sr.printf("%s<%s", kind2, kind1);
    no_match = ssp->lookup(s) || ssp->lookup(sr);
  }
  return no_match;
}

//============================================================================
static idfStringSet *init_string_set(const char *directive)
{
  idfStringSet *ss = idf_configuration.lookup(directive);
  if (ss == NULL) {
    ss = new idfStringSet;
    idf_configuration.add(directive, ss);
  }
  return ss;
}

#if 0
//============================================================================
// Set of matches between model AST names and EDG.
// Example:    "expr_stmt<expression_statement"
//============================================================================
static idfStringSet *kind_matches = NULL;
static void init_kind_matches()
{
  kind_matches = init_string_set("AST kinds match().");
}
#endif

//============================================================================
static boolean is_kind_match(const char *k1, const char *k2, boolean &loc_ok)
{
  static idfStringSet *kind_matches = init_string_set("AST kinds match().");

  const char *lok = ",loc_ok";
  const int lok_len = 7;

  loc_ok = false;
  const int max = 100;
  static char buf[max + 1];
  const int l1 = strlen(k1);
  if (strcmp(k1, k2) == 0) {
    if (l1 + l1 + lok_len + 1 <= max) {
      strcpy(buf, k1);
      buf[l1] = '<';
      strcpy(buf + l1 + 1, k2);
      strcpy(buf + l1 + 1 + l1, lok);
      buf[l1 + 1 + l1 + lok_len] = '\0';
      loc_ok = kind_matches->lookup(buf);
    }
    return true;
  }

  const int l2 = strlen(k2);
  if (l1 + l2 + lok_len + 1 <= max) {
    strcpy(buf, k1);
    buf[l1] = '<';
    strcpy(buf + l1 + 1, k2);
    buf[l1 + 1 + l2] = '\0';
    if (kind_matches->lookup(buf)) {
      return true;
    }
    strcpy(buf + l1 + 1 + l2, lok);
    buf[l1 + 1 + l2 + lok_len] = '\0';
    if (kind_matches->lookup(buf)) {
      return loc_ok = true, true;
    }

    strcpy(buf, k2);
    buf[l2] = '<';
    strcpy(buf + l2 + 1, k1);
    buf[l2 + 1 + l1] = '\0';
    if (kind_matches->lookup(buf)) {
      return true;
    }
    strcpy(buf + l2 + 1 + l1, lok);
    buf[l2 + 1 + l1 + lok_len] = '\0';
    if (kind_matches->lookup(buf)) {
      return loc_ok = true, true;
    }
  }
  return false;
}

//============================================================================
static boolean is_kind_within(const char *k1, const char *k2)
{
  static idfStringSet *kind_within = init_string_set("AST kind is within().");

  const int max = 100;
  static char buf[max + 1];
  const int l1 = strlen(k1);
  const int l2 = strlen(k2);
  if (l1 + l2 + 1 <= max) {
    strcpy(buf, k1);
    buf[l1] = '<';
    strcpy(buf + l1 + 1, k2);
    buf[l1 + 1 + l2] = '\0';
    if (kind_within->lookup(buf)) {
      return true;
    }
  }
  return false;
}

//============================================================================
static void id_string(idfAst *ast, genString &str)
{
  if (ast == NULL) {
    str = "<NULL>";
  }
  else {
    const char *kind = ast->getKind();
    if (kind == 0) {
      kind = "<kind missing>";
    }
    boolean loc_present = false;
    genString loc;
    if (ast->getLocation() != NULL) {
      idf_location_list_str(ast->getLocation(), loc);
      loc_present = true;
    }
    else {
      locationStruct child_based_loc;
      if (get_location(ast, child_based_loc)) {
      	genString child_based_loc_str;
      	idf_location_str(&child_based_loc, child_based_loc_str);
      	loc.printf("(children at %s)", child_based_loc_str.str());
      	loc_present = true;
      }
    }
    str = kind;
    if (loc_present) {
      str += ' ';
      str += loc;
    }
  }
}

//============================================================================
static locationStruct *comparable(idfAst *ast)
{
  return (ast == NULL) ? NULL : ast->getComparableLocation();
}

//============================================================================
static boolean endless(const locationStruct &loc)
{
  return idf_endless(loc);
}

//============================================================================
// Get the end line and column from the location structure.
// If start_line is 0, then this returns a 1-based "column" number.
//============================================================================
static boolean normalize_end(const locationStruct &loc, int &end_line, int &end_column)
{
  if (loc.end_line == 0) {
    if (loc.end_column <= 0) {
      return false;
    }
    // have a byte length
    end_line = loc.start_line;
    end_column = loc.start_column + loc.end_column - 1;
  }
  else {
    end_line = loc.end_line;
    end_column = loc.end_column;
  }
  return true;
}

//============================================================================
static boolean locations_overlap(const locationStruct &loc1, const locationStruct &loc2)
{
  int end_column1;
  int end_line1;
  boolean has_size1 = normalize_end(loc1, end_line1, end_column1);
  int end_column2;
  int end_line2;
  boolean has_size2 = normalize_end(loc2, end_line2, end_column2);

  if ((loc1.start_line == 0) != (loc2.start_line == 0)) {
    // incomparable
    return false;
  }

  // Treat an unspecifed-size location like a zero-size location.
  boolean zero_size1 = !has_size1 || (   loc1.start_line == end_line1
      	      	      	      	      && loc1.start_column - 1 == end_column1);
  boolean zero_size2 = !has_size2 || (   loc2.start_line == end_line2
      	      	      	      	      && loc2.start_column - 1 == end_column2);

  // whether loc1 is entirely before loc2
  boolean loc1_precedes
    = zero_size1 ?    loc1.start_line < loc2.start_line
      	      	   || (loc1.start_line == loc2.start_line && loc1.start_column < loc2.start_column)
                 :    end_line1 < loc2.start_line
      	      	   || (end_line1 == loc2.start_line && end_column1 < loc2.start_column);
  // whether loc2 is entirely before loc1
  boolean loc2_precedes
    = zero_size2 ?    loc2.start_line < loc1.start_line
      	      	   || (loc2.start_line == loc1.start_line && loc2.start_column < loc1.start_column)
                 :    end_line2 < loc1.start_line
      	      	   || (end_line2 == loc1.start_line && end_column2 < loc1.start_column);

  return !loc1_precedes && !loc2_precedes;
}

//============================================================================
static boolean location_contains(const locationStruct &loc1, const locationStruct &loc2)
{
  int end_column1;
  int end_line1;
  boolean has_size1 = normalize_end(loc1, end_line1, end_column1);
  int end_column2;
  int end_line2;
  boolean has_size2 = normalize_end(loc2, end_line2, end_column2);

  if ((loc1.start_line == 0) != (loc2.start_line == 0)) {
    // incomparable
    return false;
  }

  // Treat an unspecifed-size location like a zero-size location.
  boolean zero_size1 = !has_size1 || (   loc1.start_line == end_line1
      	      	      	      	      && loc1.start_column - 1 == end_column1);
  boolean zero_size2 = !has_size2 || (   loc2.start_line == end_line2
      	      	      	      	      && loc2.start_column - 1 == end_column2);

  // whether loc2 starts with or after loc1
  boolean starts_ok
    =    loc1.start_line < loc2.start_line
      || (loc1.start_line == loc2.start_line && loc1.start_column <= loc2.start_column);
  // whether loc2 ends with or before loc1
  if (!has_size1) {
    end_line1 = loc1.start_line;
    end_column1 = loc1.start_column - 1;
  }
  if (!has_size2) {
    end_line2 = loc2.start_line;
    end_column2 = loc2.start_column;
  }
  boolean ends_ok =    end_line2 < end_line1
      	      	    || (end_line2 == end_line1 && end_column2 <= end_column1);
  return starts_ok && ends_ok;
}

//============================================================================
static boolean ast_loc_sync(idfAst *ast1, idfAst *ast2)
{
  if (ast1 == NULL || ast2 == NULL) {
    return false;
  }
  locationStruct *comp1 = comparable(ast1);
  locationStruct *comp2 = comparable(ast2);
  int ast_vs_ast;
  boolean loc_sync = idf_compare_locations(comp1, comp2, true, ast_vs_ast);
  boolean loc_bad = loc_sync && ast_vs_ast != 0;
  if (loc_bad) {
    loc_sync = false;
  }
  return loc_sync;
}

//============================================================================
// Ensure that a complete location matched to a location with no end
// is not at a higher level in the syntax tree.
//
// The nodes ast1 and ast2 are from different files. If ast1 overlaps any
// siblings of ast2 then they cannot match, because ast1 properly matches
// at a higher level in the AST containing ast2.
//============================================================================
static boolean too_high(idfAst &ast1, idfAst &ast2)
{
  idfAst *sib = &ast2;
  for (;;) {
    sib = sib->getNextSib();
    if (sib == NULL) {
      break;
    }
    if (sib->getLocation() != NULL) {
      break;
    }
  }
  if (sib == NULL) {
    return false;
  }
  locationStruct *comp1 = comparable(&ast1);
  locationStruct *comp2 = comparable(sib);
  return comp1 != NULL && comp2 != NULL && locations_overlap(*comp1, *comp2);
}      	    

//============================================================================
// Return whether the given location overlaps the region of what follows
// given AST node.
//============================================================================
static boolean overlaps_followers(locationStruct *comp, idfAst &ast)
{
  if (comp == NULL) {
    return false;
  }
  // Find a later sibling or aunt/uncle.
  locationStruct next_comp;
  boolean got_next_comp = false;
  for (idfAst *p = &ast; p != NULL && !got_next_comp; p = p->getParent()) {
    for (idfAst *sib = p->getNextSib(); sib != NULL; sib = sib->getNextSib()) {
      if (got_next_comp = sib->getSubtreeLocation(next_comp)) {
      	break;
      }
    }
  }
  if (got_next_comp) {
    int order;
#if defined(IDF_DEBUG_TRY)
    genString next_str;
    idf_location_str(&next_comp, next_str);
    genString comp_str;
    idf_location_str(comp, comp_str);
    cout << "checking whether " << comp_str << " precedes " << next_str << ".";
#endif
    if (idf_location_precedes(comp, &next_comp, order) && order > 0) {
#if defined(IDF_DEBUG_TRY)
      cout << " fails to precede" << endl;
#endif
      return true;
    }
#if defined(IDF_DEBUG_TRY)
    cout << " precedes" << endl;
#endif
  }
  return false;
}

//============================================================================
// Return whether the given location overlaps the region outside of the
// given AST node. Returns 1 if comp overlaps what precedes ast,
// 2 if comp overlaps what follows ast, and 3 if comp overlaps both.
//============================================================================
static int overlaps_complement(locationStruct *comp, idfAst &ast)
{
  if (comp == NULL) {
    return 0;
  }
  boolean overlaps_prev = false;

  // Find a previous sibling or aunt/uncle.
  idfAst *p = &ast;
  locationStruct prev_comp;
  boolean got_prev_comp = false;
  while (p != NULL && !got_prev_comp) {
    for (idfAst *sib = p->getPrevSib(); sib != NULL; sib = sib->getPrevSib()) {
      if (got_prev_comp = sib->getSubtreeLocation(prev_comp)) {
      	break;
      }
    }
    p = p->getParent();
  }
  if (got_prev_comp) {
    int order;
#if defined(IDF_DEBUG_TRY)
    genString prev_str;
    idf_location_str(&prev_comp, prev_str);
    genString comp_str;
    idf_location_str(comp, comp_str);
    cout << "checking whether " << prev_str << " precedes " << comp_str << ".";
#endif
    if (idf_location_precedes(&prev_comp, comp, order) && order > 0) {
#if defined(IDF_DEBUG_TRY)
      cout << " fails to precede" << endl;
#endif
      overlaps_prev = true;
    }
#if defined(IDF_DEBUG_TRY)
    if (!overlaps_prev) {
      cout << " precedes" << endl;
    }
#endif
  }
  return (overlaps_prev ? 1 : 0) | (overlaps_followers(comp, ast) ? 2 : 0);
}

//============================================================================
static int overlaps_complement_badly(idfAst &ast1, locationStruct *comp1,
      	          	      	     idfAst &ast2)
{
  int bad = overlaps_complement(comp1, ast2);
  if (bad != 0) {
    // The overlap may be excused by a directive.
    idfStringSet *ssp = idf_configuration.lookup("AST () source location matches out of sequence.");
    const char *kind = ast1.getKind();
    const char *pkind = ast1.getParent() != NULL ? ast1.getParent()->getKind() : NULL;
    if (ssp != NULL && kind != NULL && pkind != NULL) {
      genString kind_in_parent;
      kind_in_parent.printf("%s in %s", kind, pkind);
      if (ssp->contains(kind_in_parent)) {
      	bad = 0;
      }
    }
  }
  return bad;
}

#if 0
//============================================================================
static boolean within_interesting(idfAst *ast)
{
  if (idf_interesting_filename == NULL) {
    return true;
  }
  while (ast != NULL) {
    locationStruct *comp = comparable(ast);
    if (comp != NULL) {
      return idf_filename_match(comp->filename, idf_interesting_filename);
    }
    ast = ast->getParent();
  }
  return false;
}
#endif

//============================================================================
// If ast1 contains ast2 but also contains more, preventing a good location
// match, sets higher to 1.  In the inverse situation to -1.  Otherwise 0.
//============================================================================
static void try_match(idfAst *ast1, idfAst *ast2, 
	              const idfModel &model1, const idfModel &model2,
      	      	      int &goodness, int &badness, int &locness, int &higher,
      	      	      boolean use_fancy_loc = false)
{
#if defined(IDF_DEBUG_TRY)
  {
    genString node1str;
    id_string(ast1, node1str);
    genString node2str;
    id_string(ast2, node2str);
    cout << "try_match of " << node1str << " to " << node2str << "." << endl;
  }
#endif
  if (ast1 == NULL || ast2 == NULL || SS_no_match (ast1, ast2)) {
    goodness = 0;
    locness = 0;
    badness = 20;
    higher = 0;
    return;
  }
  locationStruct *comp1 = comparable(ast1);
  locationStruct *comp2 = comparable(ast2);
  int ast_vs_ast = 0;
  boolean loc_sync = idf_compare_locations(comp1, comp2, true, ast_vs_ast);
#if defined(IDF_DEBUG_TRY)
  {
    genString node1str;
    id_string(ast1, node1str);
    genString node2str;
    id_string(ast2, node2str);
    cout << "try_match of " << node1str << " to " << node2str;
    cout << " finds loc_sync " << loc_sync << " and ast_vs_ast " << ast_vs_ast << endl;
  }
#endif
  boolean loc_bad = loc_sync && ast_vs_ast != 0;
#if defined(IDF_DEBUG_TRY)
  if (loc_bad) {
    cout << "loc_bad because ast nodes had different locations." << endl;
  }
#endif
  if (!loc_bad && comp1 != NULL && comp2 != NULL) {
    if (!idf_filename_match(comp1->filename, comp2->filename)) {
      loc_bad = true;
#if defined(IDF_DEBUG_TRY)
      cout << "loc_bad because ast nodes were from different files." << endl;
#endif
    }
  }

  boolean comp1_interesting = (comp1 != NULL
            && idf_filename_match(comp1->filename, idf_interesting_filename));
  boolean comp2_interesting = (comp2 != NULL
            && idf_filename_match(comp2->filename, idf_interesting_filename));
  boolean loc_very_bad = (loc_bad && comp1_interesting != comp2_interesting);
#if 0
  // But it's not very bad if the reason why !compx_interesting is that
  // the IF omits a location for this node while it's within an interesting
  // file.
  if (loc_very_bad) {
    if (!comp1_interesting && comp1 == NULL && within_interesting(ast1)) {
      loc_very_bad = false;
    }
    if (!comp2_interesting && comp2 == NULL && within_interesting(ast2)) {
      loc_very_bad = false;
    }
  }
#endif

  higher = 0;
  if (loc_sync && (endless(*comp1) || endless(*comp2))) {
    if (endless(*comp2) && !endless(*comp1) && too_high(*ast1, *ast2)) {
      higher = 1;
    }
    else if (endless(*comp1) && !endless(*comp2) && too_high(*ast2, *ast1)) {
      higher = -1;
    }
#if defined(IDF_DEBUG_TRY)
    if (higher != 0) {
      cout << "loc_bad because higher is " << higher << endl;
    }
#endif      
    loc_bad = loc_bad || higher != 0;
  }
  else if (!loc_sync && comp1 != NULL && comp2 != NULL) {
    if (location_contains(*comp1, *comp2)) {
      higher = 1;
    }
    else if (location_contains(*comp2, *comp1)) {
      higher = -1;
    }
  }
  int overlaps_complement_1 = overlaps_complement_badly(*ast1, comp1, *ast2);
  int overlaps_complement_2 = overlaps_complement_badly(*ast2, comp2, *ast1);
  boolean bad_overlap = overlaps_complement_1 != 0 || overlaps_complement_2 != 0;
  boolean bad_overlap_matches  =    (overlaps_complement_1 == overlaps_complement_2)
                                 || (overlaps_complement_1 && comp2 == NULL)
                                 || (overlaps_complement_2 && comp1 == NULL);
#if defined(IDF_DEBUG_TRY)
  cout << (bad_overlap ? "" : "no ") << "bad overlap == ("
       << overlaps_complement_1 << ", " << overlaps_complement_2 << ");"
       << (use_fancy_loc ? "" : " not") << " using fancy." << endl;
#endif
  if (loc_bad) {
    loc_sync = false;
    if (use_fancy_loc) {
      // Here, we don't want little variations in start and end location
      // to prevent a match. Call it bad only if one of the locations
      // overlaps a part of the other tree that's outside of the match.
      loc_bad = bad_overlap || loc_very_bad;
    }
  }
  boolean pseudo_loc_sync = false;

  const char *kind1 = ast1->getKind();
  const char *kind2 = ast2->getKind();
  boolean kind_bad = (kind1 == NULL) != (kind2 == NULL);
  boolean suppress_loc_bad = false;
  boolean kind_sync = (kind1 != NULL && is_kind_match(kind1, kind2, suppress_loc_bad));
  if (suppress_loc_bad && !loc_very_bad) {
    loc_bad = false;
  }

  idfSymbol *sym1 = model1.getDictionary().lookup(ast1->getIdentifier());
  idfSymbol *sym2 = model2.getDictionary().lookup(ast2->getIdentifier());
  boolean sym_sync =    sym1 != NULL && sym2 != NULL
      	      	     && (sym1->getMatch() == sym2 || sym2->getMatch() == sym1);
  boolean sym_bad = sym1 != NULL && sym2 != NULL && !idf_matchable_symbols(sym1, sym2);

  const char *value1 = ast1->getValue();
  const char *value2 = ast2->getValue();
  boolean value_bad = (value1 == NULL) != (value2 == NULL);
  boolean value_sync = !value_bad && (value1 != NULL && strcmp(value1, value2) != 0);

  boolean gen_sync = ast1->isGenerated() && ast2->isGenerated();

  boolean sib_sync = false;
  if (ast1->getNextSib() != NULL && ast2->getNextSib() != NULL) {
    idfAst *n1 = ast1->getNextSib();
    idfAst *n2 = ast2->getNextSib();
    sib_sync = n1->getMatch() == n2 || n2->getMatch() == n1;
  }
  else if (   ast1->getNextSib() == NULL && ast2->getNextSib() == NULL
      	   && ast1->getPrevSib() == NULL && ast2->getPrevSib() == NULL) {
    sib_sync = true;
  }
  boolean other_sib_sync = false;
  if (ast1->getPrevSib() != NULL && ast2->getPrevSib() != NULL) {
    idfAst *p1 = ast1->getPrevSib();
    idfAst *p2 = ast2->getPrevSib();
    other_sib_sync = p1->getMatch() == p2 || p2->getMatch() == p1;
  }
  if (!loc_sync && comp1 == NULL && comp2 == NULL) {
    // If neighbors match then it's almost as good as a loc_sync.
    if (   ast_loc_sync(ast1->getNextSib(), ast2->getNextSib())
        || ast_loc_sync(ast1->getPrevSib(), ast2->getPrevSib())
      	|| ((   (ast1->getNextSib() == NULL && ast2->getNextSib() == NULL)
      	     || (ast1->getPrevSib() == NULL && ast2->getPrevSib() == NULL))
      	    && ast_loc_sync(ast1->getParent(), ast2->getParent()))) {
      pseudo_loc_sync = true;
    }
  }

  int nchildren_sync = 0;
  if(ast1->getNumChildren() == ast2->getNumChildren()) {
    nchildren_sync = ast1->getNumChildren();
    if (nchildren_sync == 0) {
      nchildren_sync = 1;
    }
  }
  
#if 0
  cout << "matching with good: " << (loc_sync ? "L" : " ")
    << (pseudo_loc_sync ? "P" : " ")
      << (kind_sync ? "K" : " ")
      << (sym_sync ? "S" : " ")
      << (value_sync ? "V" : " ")
      << (gen_sync ? "G" : " ")
      << (sib_sync ? "S" : " ")
      << nchildren_sync;
#endif

  goodness =   5 * int(loc_sync) + 3 * int(pseudo_loc_sync) + int(!bad_overlap)
      	     + 10 * int(kind_sync) + 5 * int(sym_sync)
             + 4 * int(value_sync) + int(gen_sync) + 2 * int(sib_sync)
             + nchildren_sync;
  badness = 20 * int(loc_bad) + 10 * int(kind_bad) + 5 * int(sym_bad) + 5 * int(value_bad)
             + 10 * int(bad_overlap && !loc_bad && !(suppress_loc_bad && bad_overlap_matches));
  locness = int(loc_sync) - int(loc_bad);
#if 0
  cout << "  badness: " << badness << "  locness: " << locness << endl;
#endif
}

//============================================================================
static boolean get_location(idfAst *ast, locationStruct &loc)
{
  return ast != NULL && ast->getSubtreeLocation(loc);
}

//============================================================================
static boolean compare_by_location(idfAst *ast1, idfAst *ast2, int &order)
{
  locationStruct loc1;
  locationStruct loc2;
  if (!get_location(ast1, loc1) || !get_location(ast2, loc2)) {
    return false;
  }
  return idf_compare_locations(&loc1, &loc2, true, order);
}

//============================================================================
// Returns whether the AST node is regarded as uninteresting on account
// of the -interesting option (global idf_interesting_filename).
// To be uninteresting, the node must be actively uninteresting, by either
// being NULL, or if idf_interesting_filename is non-null, the node must
// have at least one explicit filename that does not match the interesting
// filename, and no explicit, matching filename.
//============================================================================
static boolean is_uninteresting(idfAst *ast)
{
  if (ast == NULL) {
    return true;
  }
  else {
    locationList *loc = ast->getLocation();
    if (loc == NULL) {
      return false;
    }
    else if (idf_interesting_filename == NULL) {
      return false;
    }
    else {
      int len = loc->size();
      boolean found_non_interesting_file = false;
      for (int i = 0; i < len; i += 1) {
	const char *fn = (*loc)[i]->filename;
	if (fn != NULL) {
	  if (idf_filename_match(fn, idf_interesting_filename)) {
	    return false;
	  }
	  else {
	    found_non_interesting_file = true;
	  }
	}
      }
      return found_non_interesting_file;
    }
  }
}

//============================================================================
// match_children(const idfAstChildren &, const idfAstChildren &,
//                const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Match child nodes between AST trees.
// Call this when ast1 is matched to ast2.
//============================================================================
static boolean match_children(const idfAstSiblings &children1,
      	      	      	      const idfAstSiblings &children2, 
	                      const idfModel &model1, const idfModel &model2)
{
#if defined(IDF_DEBUG)
  {
    idfAst *child1 = children1.getSibling(0);
    idfAst *ast1 = child1 != NULL ? child1->getParent() : NULL;
    idfAst *child2 = children2.getSibling(0);
    idfAst *ast2 = child2 != NULL ? child2->getParent() : NULL;
    genString node1str;
    id_string(ast1, node1str);
    if (ast1 == NULL) {
      node1str = "<unknown>";
    }
    genString node2str;
    id_string(ast2, node2str);
    if (ast2 == NULL) {
      node2str = "<unknown>";
    }
    cout << "matching children of " << node1str << " to children of " << node2str << "." << endl;
  }
#endif
  boolean match_found = false;

  // Suppose for now that locations will compare successfully.
  int nch1 = children1.getNumber();
  int nch2 = children2.getNumber();

  // Advance through the children a little at a time.
  int n1 = 0;
  int n2 = 0;
  while (n1 < nch1 && n2 < nch2) {
    boolean m = false;
    if (match_ast(children1.getSibling(n1), children2.getSibling(n2), model1, model2, &m)) {
#if defined(IDF_DEBUG)
      cout << "matched child " << n1 << " with child " << n2 << endl;
#endif
      match_found = true;
      n1 += 1;
      n2 += 1;
    }
    else if (nch1 == 1 && children2.getSibling(n2)->getMatch() != NULL) {
      // Match seems to have happened between children2 child and a descendant of children1 child 0.
#if defined(IDF_DEBUG)
      cout << "apparent match of descendant of child " << n1 << " with child " << n2 << endl;
#endif
      match_children(children1.getSibling(n1)->getChildren(), children2, model1, model2);
      return true;
    }
    else if (nch2 == 1 && children1.getSibling(n1)->getMatch() != NULL) {
      // Match seems to have happened between children1 child and a descendant of children2 child 0.
#if defined(IDF_DEBUG)
      cout << "apparent match of child " << n1 << " with descendant of child " << n2 << endl;
#endif
      match_children(children1, children2.getSibling(n2)->getChildren(), model1, model2);
      return true;
    }
    else {
      match_found = m;
#if defined(IDF_DEBUG)
      cout << "failed to match child " << n1 << " with child " << n2 << endl; 
#endif
      boolean n1_interesting = !is_uninteresting(children1.getSibling(n1));
      boolean n2_interesting = !is_uninteresting(children2.getSibling(n2));

      // no match; advance one or the other.
      if (n1_interesting && !n2_interesting) {
      	n2 += 1;
#if defined(IDF_DEBUG)
      	cout << "Child from second list is uninteresting." << endl;
#endif
      }
      else if (n2_interesting && !n1_interesting) {
      	n1 += 1;
#if defined(IDF_DEBUG)
      	cout << "Child from first list is uninteresting." << endl;
#endif
      }
      else if (n1 + 1 == nch1) {
      	n2 += 1;
#if defined(IDF_DEBUG)
      	cout << "at end of first child list" << endl;
#endif
      }
      else if (n2 + 1 == nch2) {
      	n1 += 1;
#if defined(IDF_DEBUG)
      	cout << "at end of second child list" << endl;
#endif
      }
      else {
      	int c1_vs_c2;
      	if (compare_by_location(children1.getSibling(n1), children2.getSibling(n2), c1_vs_c2)) {
      	  if (c1_vs_c2 < 0) {
#if defined(IDF_DEBUG)
      	    cout << "first location was less" << endl;
#endif
      	    n1 += 1;
	  }
      	  else if (c1_vs_c2 > 0) {
#if defined(IDF_DEBUG)
      	    cout << "second location was less" << endl;
#endif
      	    n2 += 1;
      	  }
      	  else {
      	    // The children matched in location, but something ghastly
      	    // must have kept them from matching overall. Advance past both.
#if defined(IDF_DEBUG)
      	    cout << "ghastly non-match" << endl;
#endif
      	    n1 += 1;
      	    n2 += 1;
      	  }
      	}
      	else {
      	  idfAst *this1 = children1.getSibling(n1);
      	  idfAst *this2 = children2.getSibling(n2);
      	  idfAst *next1 = children1.getSibling(n1 + 1);
      	  idfAst *next2 = children2.getSibling(n2 + 1);
      	  // Could not compare the current children; try the next.
#if defined(IDF_DEBUG)
      	  cout << "current children did not compare, location-wise" << endl;
#endif
      	  locationStruct *loc1 = comparable(this1);
      	  locationStruct *loc2 = comparable(this2);
      	  locationStruct *next_loc1 = comparable(next1);
      	  locationStruct *next_loc2 = comparable(next2);
#if 0
      	  if (idf_compare_locations(loc1, next_loc2, true, c1_vs_c2) && c1_vs_c2 >= 0) {
#if defined(IDF_DEBUG)
      	    cout << "next in 2nd child list matches" << endl;
#endif
      	    n2 += 1;
      	  }
      	  else if (idf_compare_locations(next_loc1, loc2, true, c1_vs_c2) && c1_vs_c2 <= 0) {
#if defined(IDF_DEBUG)
      	    cout << "next in 1st child list matches" << endl;
#endif
      	    n1 += 1;
	  }
#else
      	  if (loc1 != 0 && next_loc2 != 0 && locations_overlap(*loc1, *next_loc2)) {
#if defined(IDF_DEBUG)
      	    cout << "next in 2nd child list matches" << endl;
#endif
      	    n2 += 1;
      	  }
      	  else if (next_loc1 != 0 && loc2 != 0 && locations_overlap(*next_loc1, *loc2)) {
#if defined(IDF_DEBUG)
      	    cout << "next in 1st child list matches" << endl;
#endif
      	    n1 += 1;
	  }
#endif
      	  else {
      	    // No comparison by location seems possible.
#if defined(IDF_DEBUG)
      	    cout << "no comparison by location seems possible." << endl;
#endif
      	    // Try to match the next 2 AST nodes.
      	    // Also try to match one current AST node to the corresponding next.
	    int goodness, badness, locness, higher;
	    try_match(next1, next2, model1, model2, goodness, badness, locness, higher);
	    int this_good = goodness - badness;
	    int use1 = n1 + 1;
	    int use2 = n2 + 1;
	    const int threshold = 3;
            // Try child down 1 level in case child matches better.
	    const int exceed = 4;
	    try_match(next1, this2, model1, model2, goodness, badness, locness, higher);
	    if (goodness - badness > threshold && goodness - badness > this_good + exceed) {
	      use2 = n2;
	      this_good = goodness - badness;
	    }
	    if (use2 > n2) {
	      try_match(this1, next2, model1, model2, goodness, badness, locness, higher);
	      if (goodness - badness > threshold && goodness - badness > this_good + exceed) {
		use1 = n1;
		this_good = goodness - badness;
	      }
	    }
      	    // If just one of the nodes has location information, avoid moving that one.
      	    // If both have location information, and one does not match the interesting
      	    // file, move the uninteresting one.
      	    if (use1 > n1 && use2 > n2) {
      	      if (loc1 != 0 && loc2 != 0) {
      	      	boolean interesting1 = idf_filename_match(loc1->filename, idf_interesting_filename);
      	      	boolean interesting2 = idf_filename_match(loc2->filename, idf_interesting_filename);
      	      	if (interesting1 != interesting2) {
      	      	  if (interesting1) {
      	      	    use1 = n1;
      	      	  }
      	      	  else {
      	      	    use2 = n2;
      	      	  }
      	      	}
      	      }
      	      else {
		if (loc1 != 0) {
		  use1 = n1;
		}
		if (loc2 != 0) {
		  use2 = n2;
		}
	      }
      	    }
      	    // In any case, we must go forward.
      	    n1 = use1;  
    	    n2 = use2;
	  }
      	}
      }
    }
  }
  return match_found;
}

//============================================================================
// match_children_to(idfAst &, idfAst &, const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Match child nodes of ast1 to node ast2.
//============================================================================
static boolean match_children_to(idfAst &ast1, idfAst &ast2, 
	                         const idfModel &model1, const idfModel &model2)
{
#if defined(IDF_DEBUG)
  genString node1str;
  id_string(&ast1, node1str);
  genString node2str;
  id_string(&ast2, node2str);
  cout << "matching children of " << node1str << " to " << node2str << "." << endl;
#endif

  // Suppose for now that locations will compare successfully.
  int nch = ast1.getNumChildren();
  locationStruct ast2_loc;
  boolean ast2_has_loc = ast2.getSubtreeLocation(ast2_loc);

  // Advance through the children.
  int n = 0;
  boolean match_found = false;
  boolean child_match_found = false;
  boolean higher_found = false;
  while (n < nch && !match_found && !higher_found) {
    // Quit if we've gone too far.
    locationStruct *ast1_child_loc = ast1.getChild(n)->getComparableLocation();
    if (overlaps_followers(ast1_child_loc, ast2)) {
      if (   ast1_child_loc != NULL && ast2_has_loc
          && !location_contains(*ast1_child_loc, ast2_loc)) {
        break;
      }
    }

    int goodness, badness, locness, higher;
    try_match(&ast2, ast1.getChild(n), model2, model1, goodness, badness, locness, higher);
    if (higher == 1) {
      // ast2 isn't a proper match with ast1's child because it comprehends
      // not just ast1 but more.
      higher_found = true;
    }
    boolean cm;
    if (match_ast(ast1.getChild(n), &ast2, model1, model2, &cm)) {
      match_found = true;
    }
    if (cm) {
      child_match_found = true;
    }
    n += 1;
  }
  if (higher_found) {
    if (match_children(ast1.getChildren(), ast2.getChildren(), model1, model2)) {
      child_match_found = true;
    }
  }
  return child_match_found;
}

//============================================================================
// match_to_children(idfAst &, idfAst &, const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Match ast1 to child nodes of ast2.
//============================================================================
static boolean match_to_children(idfAst &ast1, idfAst &ast2, 
	                      const idfModel &model1, const idfModel &model2)
{
  return match_children_to(ast2, ast1, model2, model1);
}

//============================================================================
static void set_match(idfAst &ast1, idfAst &ast2,
      	              const idfModel &model1, const idfModel &model2,
                      boolean do_children)
{
  // Something corroborates the match. Make it so.
#if defined(IDF_DEBUG)
  genString node1str;
  id_string(&ast1, node1str);
  genString node2str;
  id_string(&ast2, node2str);
  cout << "matched " << node1str << " to " << node2str << "." << endl;
#endif

  if (ast1.getMatch() == NULL) {
    ast1.setMatch(&ast2);
  }
  if (ast2.getMatch() == NULL) {
    ast2.setMatch(&ast1);
  }
  if (do_children) {
    match_children(ast1.getChildren(), ast2.getChildren(), model1, model2);
  }

  {
    unsigned int id1 = ast1.getIdentifier();
    unsigned int id2 = ast2.getIdentifier();
    if (id1 > 0 && id2 > 0) {
      idf_try_symbol_match(id1, id2, model1, model2);
    }
  }

  idf_try_file_match(ast1.getLocation(), ast2.getLocation(), model1, model2);
}

//============================================================================
// match_ast
//----------------------------------------------------------------------------
// Match nodes between AST trees.
// Call this when ast1 ought to match ast2 and is expected to do so.
// If minor differences between the 2 nodes exist, they will be matched
// anyway.
//============================================================================
static boolean match_ast(idfAst *ast1, idfAst *ast2, 
	                 const idfModel &model1, const idfModel &model2,
      	      	      	 boolean *matched_among_children)
{
  boolean mac;
  boolean &child_matched = matched_among_children != NULL ? *matched_among_children : mac;
  child_matched = false;
  if (ast1 == NULL || ast2 == NULL) {
#if defined(IDF_DEBUG)
    cout << "match_ast failed due to null operand" << endl;
#endif
    return false;
  }
  if (ast1->getMatch() != NULL && ast2->getMatch() != NULL) {
#if defined(IDF_DEBUG)
    cout << "match_ast decided on basis of previous match" << endl;
#endif
    return child_matched = ast1->getMatch() == ast2 || ast2->getMatch() == ast1;
  }
  int goodness, badness, locness, higher;
#if defined(IDF_DEBUG)
  genString node1str;
  id_string(ast1, node1str);
  genString node2str;
  id_string(ast2, node2str);
  cout << "attempting to match " << node1str << " to " << node2str << "." << endl;
#endif
  try_match(ast1, ast2, model1, model2, goodness, badness, locness, higher);
#if defined(IDF_DEBUG)
  cout << "match_ast found goodness " << goodness
       << " badness " << badness
       << " locness " << locness << "." << endl;
#endif
  int this_good = goodness - badness;
  idfAst *use1 = ast1;
  idfAst *use2 = ast2;
  const int threshold = 4;
  if (locness >= 0) {
    // Try child down 1 level in case child matches better.
    const int exceed = 5;
    if (ast1->getNumChildren() == 1) {
      try_match(ast1->getChild(0), ast2, model1, model2, goodness, badness, locness, higher);
      if (goodness - badness > threshold && goodness - badness > this_good + exceed) {
      	use1 = ast1->getChild(0);
      	this_good = goodness - badness;
      }
    }
    if (use1 == ast1 && ast2->getNumChildren() == 1) {
      try_match(ast1, ast2->getChild(0), model1, model2, goodness, badness, locness, higher);
      if (goodness - badness > threshold && goodness - badness > this_good + exceed) {
      	use2 = ast2->getChild(0);
      	this_good = goodness - badness;
      }
    }
  }
  if (this_good > threshold) {
    set_match(*use1, *use2, model1, model2, true);
    return child_matched = true; // !! assignment intentional
  }
  else {
    // This is not a match. But try for a match among children.
    const char *kind1 = ast1->getKind();
    const char *kind2 = ast2->getKind();
    if (kind1 != NULL && kind2 != NULL && is_kind_within(kind2, kind1)) {
      child_matched = match_children_to(*ast1, *ast2, model1, model2);
    }
    else if (kind1 != NULL && kind2 != NULL && is_kind_within(kind1, kind2)) {
      child_matched = match_to_children(*ast1, *ast2, model1, model2);
    }
    else {
      // Try for a match among children, based on location.
      locationStruct loc1;
      locationStruct loc2;
      if (get_location(ast1, loc1) && get_location(ast2, loc2)) {
	// Locations exist for both nodes.
	// If there is overlap, continue the comparison.
	if (locations_overlap(loc1, loc2)) {
	  if (location_contains(loc1, loc2)) {
	    child_matched = match_children_to(*ast1, *ast2, model1, model2);
	    if (!child_matched) {
	      child_matched = match_to_children(*ast1, *ast2, model1, model2);
	    }
	  }
	  else if (location_contains(loc2, loc1)) {
	    child_matched = match_to_children(*ast1, *ast2, model1, model2);
	    if (!child_matched) {
	      child_matched = match_children_to(*ast1, *ast2, model1, model2);
	    }
	  }
	  else {
	    child_matched = match_children(ast1->getChildren(), ast2->getChildren(), model1, model2);
	  }
	}
      }
    }

#if defined(IDF_DEBUG)
    cout << "match_ast of " << node1str << " to " << node2str << " "
         << (child_matched ? "found" : "did not find") << " a match among children." << endl;
#endif
    idfStringSet *ssp = idf_configuration.lookup("AST nodes () should be revisited.");
    if (   ast1->getMatch() == NULL && ast2->getMatch() == NULL
        && (   child_matched 
            || (   idf_strmatch(ast1->getKind(), ast2->getKind()) 
                && ssp != NULL && ssp->contains(ast1->getKind())
               )
           )
       ) {
      // Revisit the possibility of a match at this level.
      try_match(ast1, ast2, model1, model2, goodness, badness, locness, higher, true);
#if defined(IDF_DEBUG)
      cout << "match_ast found goodness " << goodness
           << " badness " << badness
           << " locness " << locness << " when revisiting." << endl;
#endif
      if (goodness - badness > threshold) {
        set_match(*ast1, *ast2, model1, model2, true);
        return child_matched = true;
      }
    }
  }
  return false;
}

//============================================================================
// idf_match_ast
//----------------------------------------------------------------------------
// Match nodes between AST tree lists.
//============================================================================
void idf_match_ast(const idfModel &model1, const idfModel &model2)
{
  // init_kind_matches();
#if 0
  while (ast1 != NULL && ast2 != NULL) {
    match_ast(ast1, ast2, model1, model2);
    ast1 = ast1->getNextSib();
    ast2 = ast2->getNextSib();
  }
#elif 0
  // Match in reverse.
  while (ast1 != NULL && ast1->getNextSib() != NULL) {
    ast1 = ast1->getNextSib();
  }
  while (ast2 != NULL && ast2->getNextSib() != NULL) {
    ast2 = ast2->getNextSib();
  }
  while (ast1 != NULL && ast2 != NULL) {
    match_ast(ast1, ast2, model1, model2);
    ast1 = ast1->getPrevSib();
    ast2 = ast2->getPrevSib();
  }
#elif 0
  idfAstSiblings *children1 = model1.getAst();
  idfAstSiblings *children2 = model2.getAst();
  if (children1 != NULL && children2 != NULL) {
    match_children(*children1, *children2, model1, model2);
  }
#else
  idfAstDictionary *asts1 = model1.getAst();
  idfAstDictionary *asts2 = model2.getAst();
  int n1 = asts1->count();
  int n2 = asts2->count();
  while (n1 > 0 && n2 > 0) {
    n1 -= 1;
    n2 -= 1;
    idfAstSiblings *children1 = asts1->lookup(n1);
    idfAstSiblings *children2 = asts2->lookup(n2);
    if (children1 != NULL && children2 != NULL) {
      match_children(*children1, *children2, model1, model2);
    }
  }
  if (n1 != n2 && (asts1->count() > 1 || asts2->count() > 1)) {
    if (!idf_configuration.contains(
	  "AST records, other than the final ASTs, do not match.")) {
      genString s;
      if (n1 > n2) {
	  s.printf("%s: Multiple AST records. Ignoring the first %d of %d.", model1.getFilename(), n1, asts1->count());
      }
      if (n2 > n1) {
	  s.printf("%s: Multiple AST records. Ignoring the first %d of %d.", model2.getFilename(), n2, asts2->count());
      }
      idf_report(s, difference_note);
    }
  }
  else if (n1 != n2) {
      genString s;
      if (n1 > n2) {
	  s.printf("%s: AST missing.", model2.getFilename());
      }
      if (n2 > n1) {
	  s.printf("%s: AST missing.", model1.getFilename());
      }
      idf_report(s, difference_note);
  }
#endif
}
