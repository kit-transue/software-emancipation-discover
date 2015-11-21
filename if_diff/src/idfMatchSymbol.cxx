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
#include <idfMatchSymbol.h>
#include <idfSymbol.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <idfMain.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <iostream.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
#include <iostream>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <idfModel.h>
#include <idfLocation.h>

//============================================================================
static boolean is_kind_match(const char *k1, const char *k2)
{
  const int l1 = strlen(k1);
  if (strcmp(k1, k2) == 0) {
    return true;
  }

  idfStringSet *kind_matches = idf_configuration.lookup("SYM kinds match().");
  if (kind_matches != NULL) {
    const int max = 100;
    static char buf[max + 1];

    const int l2 = strlen(k2);
    if (l1 + l2 + 1 <= max) {
      strcpy(buf, k1);
      buf[l1] = '<';
      strcpy(buf + l1 + 1, k2);
      buf[l1 + 1 + l2] = '\0';
      if (kind_matches->lookup(buf)) {
	return true;
      }

      strcpy(buf, k2);
      buf[l2] = '<';
      strcpy(buf + l2 + 1, k1);
      buf[l2 + 1 + l1] = '\0';
      if (kind_matches->lookup(buf)) {
	return true;
      }
    }
  }
  return false;
}

//============================================================================
// unique_relation(idfRelationshipDictionary &rels, int &sym)
//----------------------------------------------------------------------------
// Return true if the given relationship dictionary contains only one
// entry. If so, set sym to that entry.
//============================================================================
static boolean unique_relation(idfRelationshipDictionary &rels, int &sym)
{
  idfIterator iter = rels.first();
  if (rels.done(iter)) {
    return false;
  }
  sym = rels.index(iter);
  return !rels.next(iter);
}

//============================================================================
// Return true if the given attribute dictionaries are 100% equivalent.
//============================================================================
static boolean equivalent_attributes(idfAttributeDictionary *ad1,
				     idfAttributeDictionary *ad2)
{
  if (ad1 == ad2) {
    return true;
  }
  else if (ad1 == NULL || ad2 == NULL) {
    return false;
  }
  else {
    // !!! For now, assume that if they're equivalent then the
    // associations will show up in the same order.
    bool equiv = true;
    idfIterator it1 = ad1->first();
    idfIterator it2 = ad2->first();
    for (; !ad1->done(it1) && !ad2->done(it2); ad1->next(it1), ad2->next(it2)) {
      if (strcmp(ad1->index(it1), ad2->index(it2)) != 0) {
        equiv = false;
      }
      const char *val1 = ad1->lookup(it1);
      const char *val2 = ad2->lookup(it2);
      if (!(val1 == val2 || (val1 != NULL && val2 != NULL && strcmp(val1, val2) == 0))) {
        equiv = false;
      }
    }
    return equiv;
  }
}

//============================================================================
// match_relations(idfSymbol *, idfSymbol*, const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Match the relations of two newly-associated symbols.
// If so, then attempt to match the relations of sym1 and sym2 as well.
//============================================================================
static void match_relations(idfSymbol *sym1, idfSymbol *sym2,
      	      	      	    const idfModel &model1, const idfModel &model2)
{
  idfRelationDictionary &rdict1 = sym1->getRelations();
  idfRelationDictionary &rdict2 = sym2->getRelations();

  for (idfIterator iter = rdict1.first(); !rdict1.done(iter); rdict1.next(iter)) {
    idfRelationshipDictionary *rels1 = rdict1.lookup(iter);
    const char *relation_name = rdict1.index(iter);
    idfRelationshipDictionary *rels2 = rdict2.lookup(relation_name);
#if 0
    cout << "Matching relation " << relation_name << " of [" << sym1->getIdentNum() << "] and [" << sym2->getIdentNum()
         << "]" << (rels2 != NULL ? " good" : " bad") << "." << endl;
#endif
    if (rels2 != NULL) {
#if 0
      for (idfIterator it = rels1->first(); !rels1->done(it); rels1->next(it)) {
      	cout << "   [" << sym1->getIdentNum() << "] " << relation_name << " [" << rels1->index(it) << "]" << endl;
      }
      for (idfIterator ix = rels2->first(); !rels2->done(ix); rels2->next(ix)) {
      	cout << "   [" << sym1->getIdentNum() << "] " << relation_name << " [" << rels2->index(ix) << "]" << endl;
      }

#endif
      // Both symbols have a relation by the given name.
      int sym1, sym2;
      if (unique_relation(*rels1, sym1) && unique_relation(*rels2, sym2)) {
      	// Both symbols are related to a unique symbol through the relation.
      	idf_try_symbol_match(sym1, sym2, model1, model2);
      }
      else {
	for (idfIterator it = rels1->first(); !rels1->done(it); rels1->next(it)) {
	  int eq_count = 0;
    	  int sym2;
	  for (idfIterator ix = rels2->first(); !rels2->done(ix); rels2->next(ix)) {
	    if (equivalent_attributes(rels1->lookup(it), rels2->lookup(ix))) {
    	      eq_count += 1;
    	      sym2 = rels2->index(ix);
	    }
	  }
	  if (eq_count == 1) {
    	    idf_try_symbol_match(rels1->index(it), sym2, model1, model2);
	  }
	}
      }
    }
  }
}

//============================================================================
static boolean idchar(char c)
{
  return isalnum(c) || c == '_' || c == '-' || c == '~';
}

//============================================================================
// Scan a null-terminated string for an idchar. Return whether one was found.
// Update p to point to the first idchar.
//============================================================================
static boolean scan_id(const char *&p)
{
  char c;
  for (;;) {
    c = *p;
    if (idchar(c) || c == '\0' || c == '@') {
      break;
    }
    p += 1;
  }
  return idchar(c);
}

//============================================================================
// Find the end of a portion of a type name that has begun with a 
// '(' or with the ',' of an apparent argument list.
//============================================================================
static char *find_end(char *p)
{
  int paren_count = 0;
  for (;;) {
    char c = *p;
    if (c == '\0') {
      return p;
    }
    else if (c == '(') {
      paren_count += 1;
    }
    else if (c == ')') {
      if (paren_count <= 0) {
      	return p;
      }
      paren_count -= 1;
    }
    else if (c == ',') {
      if (paren_count <= 0) {
      	return p;
      }
    }
    p += 1;
  }
}

//============================================================================
// Regard the names as matchable if both are empty of identifiers, or if
// they contain one identifier token in common.
//============================================================================
static boolean matchable_names(const char *nm1, const char *nm2)
{
  int ident_count1 = 0;
  const char *p = nm1;
  for(;;) {
    if (!scan_id(p)) {
      break;
    }
    // p points to a name. Does nm2 contain the name?
    const char *q = nm2;
    for (;;) {
      if (!scan_id(q)) {
      	break;
      }
      // p and q point to names. Do they match?
      const char *p2 = p;
      while (*p2 == *q && idchar(*p2)) {
      	p2 += 1;
      	q += 1;
      }
      if (!idchar(*p2) && !idchar(*q)) {
      	// The 2 are the same length.
      	return true;
      }
      while (idchar(*q)) {
      	q += 1;
      }
    }
    // Skip past the identifier.
    ident_count1 += 1;
    while (idchar(*p)) {
      p += 1;
    }
  }
  if (ident_count1 == 0) {
    const char *q = nm2;
    if (!scan_id(q)) {
      // Neither string contains an identifier.
      return true;
    }
  }
  return false;
}

//============================================================================
// Be more strict in matching type names, because we were getting matches
// between 'int' and 'int (int)'.
//============================================================================
static boolean matchable_type_names(const char *nm1, const char *nm2)
{
  char *n1 = strdup(nm1);
  char *n2 = strdup(nm2);
  char *lp1 = strchr(n1, '(');
  char *lp2 = strchr(n2, '(');
#if 0
  cout << "matching type names " << nm1 << " with " << nm2 << endl;
#endif
  if ((lp1 != NULL) != (lp2 != NULL)) {
    return false;
  }
  boolean result = true;
  if (lp1 == NULL) {
    result = matchable_names(n1, n2);
#if 0
    if (result == false) {
      cout << "names did not match" << endl;
    }
#endif
  }
  else {
    *lp1 = '\0';
    *lp2 = '\0';
    if (!matchable_names(n1, n2)) {
#if 0
      cout << "initial names did not match" << endl;
#endif
      result = false;
    }
    char *start1 = lp1 + 1;
    char *start2 = lp2 + 1;
    for (;;) {
      char *end1 = find_end(start1);
      char *end2 = find_end(start2);
      if (*end1 != *end2) {
      	result = false;
#if 0
      	cout << "ends were different" << endl;
#endif
      	break;
      }
      char ender = *end1;
      *end1 = '\0';
      *end2 = '\0';
      if (!matchable_type_names(start1, start2)) {
#if 0
      	cout << "argument type names " << start1 << " and " << start2 << " did not match" << endl;
#endif
      	result = false;
      }
      if (ender != ',') {
      	break;
      }
      start1 = end1 + 1;
      start2 = end2 + 1;
    }
  } 
  free(n1);
  free(n2);
  return result;
}

//============================================================================
// idf_matchable_symbols(idfSymbol *, idfSymbol *)
//----------------------------------------------------------------------------
// Return whether properties of the 2 symbols would indicate that it would
// be screwy to match them up.
//============================================================================
boolean idf_matchable_symbols(idfSymbol *sym1, idfSymbol *sym2)
{
  if (sym1 == NULL || sym2 == NULL) {
    return false;
  }

  const char *kind1 = sym1->getKind();
  const char *kind2 = sym2->getKind();
  if ((kind1 == NULL) != (kind2 == NULL)) {
#if 0
    cout << "not matchable due to null kind" << endl;
#endif
    return false;
  }
  if (kind1 != NULL && !is_kind_match(kind1, kind2)) {
#if 0
    cout << "not matchable due to disparate kinds: " << name1
      	 << " vs " << name2 << endl;
#endif
    return false;
  }
  boolean is_type = kind1 != NULL && strcmp(kind1, "type") == 0;

  const char *name1 = sym1->getName();
  const char *name2 = sym2->getName();
  if ((name1 == NULL) != (name2 == NULL)) {
    return false;
  }
  if (name1 != NULL && (!matchable_names(name1, name2)
      	      	        || (is_type && !matchable_type_names(name1, name2)))) {
    return false;
  }

  const locationStruct *loc1 = sym1->getLocation();
  const locationStruct *loc2 = sym2->getLocation();
#if 0
  int loc_vs_loc;
  if (idf_compare_locations(loc1, loc2, loc_vs_loc) && loc_vs_loc != 0) {
    cout << "not matchable due to different locations" << endl;
    return false;
  }
#endif
#if 0
  // In a SYM record, the position primarily is there to indicate definition
  // status.  It's not always clear where in the definition the position should
  // point, so for now we ignore that.
  if ((loc1 != NULL) != (loc2 != NULL)) {
    cout << "not matchable due to definition status difference: " << name1 << " vs " 
      	 << name2 << endl;
    return false;
  }
#endif
  // But for now, we find better results if we don't even quibble about 
  // definition status.

#if 0
  cout << "matchable symbols" << endl;
#endif
  return true;

  // is_generated not compared.
}

//============================================================================
// try_symbol_match(idfSymbol *, idfSymbol *,
//    	      	    const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Call this only for symbols that are in corresponding relationships to
// elements that are considered to match.
// Return whether sym1 from dict1 and sym2 from dict2 match one another.
// If so, then attempt to match the relations of sym1 and sym2 as well.
//============================================================================
static boolean try_symbol_match(idfSymbol *sym1, idfSymbol *sym2,
      	      	      	        const idfModel &model1, const idfModel &model2)
{
  if (sym1 == NULL || sym2 == NULL) {
    return false;
  }
  if (sym1->getMatch() == sym2 || sym2->getMatch() == sym1) {
    return true;
  }
#if 0
  cout << "Attempting to match [" << sym1->getIdentNum() << "] with [" << sym2->getIdentNum() << "]." << endl;
#endif
  if (!idf_matchable_symbols(sym1, sym2)) {
    return false;
  }
  boolean new_match = false;
  if (sym1->getMatch() == NULL) {
    sym1->setMatch(sym2);
    new_match = true;
  }
  if (sym2->getMatch() == NULL) {
    sym2->setMatch(sym1);
    new_match = true;
  }
  if (new_match) {
    match_relations(sym1, sym2, model1, model2);
    // ?? match_relations(sym2, sym1, model2, model1);

    idf_try_file_match(sym1->getLocation(), sym2->getLocation(), model1, model2);
  }
  return new_match;
}

//============================================================================
// idf_try_symbol_match(int, int, const idfModel &, const idfModel &)
//----------------------------------------------------------------------------
// Call this only for symbols that are in corresponding relationships to
// elements that are considered to match.
// Return whether sym1 from dict1 and sym2 from dict2 match one another.
// If so, then attempt to match the relations of sym1 and sym2 as well.
//============================================================================
boolean idf_try_symbol_match(int sym1, int sym2,
      	      	      	     const idfModel &model1, const idfModel &model2)
{
  idfSymbol *symp1 = model1.getDictionary().lookup(sym1);
  idfSymbol *symp2 = model2.getDictionary().lookup(sym2);
  return try_symbol_match(symp1, symp2, model1, model2);
}

//============================================================================
// Create a dictionary indexed by name, which leads to a unique symbol
// or to NULL.
//============================================================================
idfSymbolNameDictionary *make_name_dictionary(const idfModel &model)
{
  const idfSymbolDictionary &symbols = model.getDictionary();
  idfSymbolNameDictionary *dict = new idfSymbolNameDictionary;
  for (idfIterator iter = symbols.first(); !symbols.done(iter); symbols.next(iter)) {
    idfSymbol *sym = symbols.lookup(iter);
    const char *name = sym->getName();
#if 0
    cout << "Putting " << name << " into name dictionary for [" << sym->getIdentNum() << "] == [" << symbols.index(iter) << "].";
#endif
    if (name != NULL) {
      if (!dict->contains(name)) {
#if 0
      	cout << ": added." << endl;
#endif
      	dict->add(name, sym);
      }
      else {
#if 0
      	cout << ": removed as duplicate." << endl;
#endif
      	dict->reassign(name, NULL);
      }
    }
  }
  return dict;
}

//============================================================================
void idf_match_symbols(const idfModel &model1, const idfModel &model2)
{
  idfSymbolNameDictionary *dict1 = make_name_dictionary(model1);
  idfSymbolNameDictionary *dict2 = make_name_dictionary(model2);
  for (idfIterator iter = dict1->first(); !dict1->done(iter); dict1->next(iter)) {
    idfSymbol *sym1 = dict1->lookup(iter);
    if (sym1 != NULL) {
#if 0
      cout << "idf_match_symbols processing " << dict1->index(iter) << ": [" << sym1->getIdentNum() << "]." << endl;
#endif
      idfSymbol *sym2 = dict2->lookup(dict1->index(iter));
      if (sym2 != NULL) {
#if 0
        cout << "Attempting to match based on name " << dict1->index(iter)
                                            << ": [" << sym1->getIdentNum()
                                       << "] with [" << sym2->getIdentNum() << "]." << endl;
#endif
      	boolean matched = try_symbol_match(sym1, sym2, model1, model2);
#if 0
        if (matched) {
            cout << "Match succeeded (or already at least a 1-way match)." << endl;
	}
        else {
            cout << "Match failed." << endl;
	}
#endif
      }
      else {
#if 0
        cout << "Attempting to match based on name " << dict1->index(iter)
                                            << ": [" << sym1->getIdentNum()
                                       << "] with failure due to sym2 no ident num." << endl;
#endif
      }
    }
    else {
#if 0
       cout << "idf_match_symbols failing " << dict1->index(iter) << ": no ident num." << endl;
#endif
    }
  }
  /* Try to match symbols which are likely to represent files. */
  idf_try_symbol_match(1, 1, model1, model2);
  idf_try_symbol_match(2, 2, model1, model2);
  /* Here it would make sense to mop up extraneous symbols, because aset_CCcc
   * is prone to create type symbols for no particular reason.
   * Partition the unmatched symbols by name and kind.  For each partitioned
   * set of unmatched symbols, if its cardinality equals the number of
   * unmatched symbols having the same name and kind in the other file,
   * do idf_try_symbol_match on those symbols, sequentially by id number.
   */
}
