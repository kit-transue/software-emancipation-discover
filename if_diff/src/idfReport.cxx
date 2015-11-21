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
#include <idfReport.h>
#include <general.h>
#include <locationList.h>
#include <idfDict.h>
#include <idfAst.h>
#include <idfSmt.h>
#include <idfSymbol.h>
#include <idfMain.h>
#include <genString.h>
#include <idfModel.h>
#include <idfLocation.h>
#include <idfMatchSmt.h>
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */

static ostream *ostrm = NULL;
static int const output_width = 78;

static long report_counts[last_report_kind];

//============================================================================
void set_report_stream(ostream &strm)
{
  ostrm = &strm;
}

//============================================================================
// find_line returns how many characters of the message to output on the next
// line.  Take care to get 'max' characters or less, but be darn sure to get
// at least 1 character to avoid looping.
//
// Assume for formatting purposes that the string begins with non-whitespace.
//============================================================================
static boolean find_line(int max, const char *s, int &line_size)
{
  if (s == NULL) {
    return false;
  }
  const char *const q = s + max;
  const char *end = s;
  char const *p;
  for (p = s; p < q && *p != '\0'; p += 1) {
    const char this_char = p[0];
    const char next_char = p[1];
    if (!isspace(this_char) && (   isspace(next_char)
                                || next_char == '\0'
                                || this_char == ',')) {
      end = p + 1;
    }
  }
  if (end > s) {
    line_size = end - s;
    return true;
  }
  // Found no nice place to end the printing.
  // Print as much as we've got, up to max.
  line_size = p - s;
  return p > s;
}

//============================================================================
static void skip_whitespace(const char *&s)
{
  while (isspace(*s)) {
    s += 1;
  }
}

//============================================================================
void idf_report_spacer()
{
  static int report_count = 0;
  if (ostrm != NULL) {
    if (report_count > 0) {
      // Output a spacer.
      (*ostrm) << endl;
    }
    report_count += 1;
  }
}

//============================================================================
void idf_report(const char *s, report_kind kind)
{
  if (ostrm != NULL) {
    // (*ostrm) << s << endl;
    idf_report_spacer();
    int line_size;
    while (find_line(output_width, s, line_size)) {
      ostrm->write(s, line_size) << endl;
      s += line_size;
      skip_whitespace(s);
    }
  }
  report_counts[kind] += 1;
}

//============================================================================
long idf_report_count(report_kind kind)
{
  return report_counts[kind];
}

//============================================================================
void idf_location_str(const locationStruct *loc, genString &result)
{
  result = "";
  if (loc != NULL) {
    genString fn;
    if (loc->filename != NULL) {
      const char *sep = strrchr(loc->filename, '/');
      fn = (sep != NULL) ? sep + 1 : loc->filename;
    }

    genString start;
    if (loc->start_line > 0) {
      // line/column
      start.printf("%d/%d", loc->start_line, loc->start_column);
    }
    else {
      // byte position
      start.printf("%d", loc->start_column);
    }
    genString end;
    if (loc->end_line > 0 || loc->end_column > 0) {
      // end present
      if (loc->end_line > 0) {
	// line/column
	end.printf(" %d/%d", loc->end_line, loc->end_column);
      }
      else {
	// byte length
	end.printf(" %d", loc->end_column);
      }
    }
    if (loc->filename != NULL) {
      result += fn;
      result += " ";
    }
    result += start;
    result += end;
  }
}

//============================================================================
static const char *nonnull(const char *str, const char *substitute = "<NULL>")
{
  return (str != NULL) ? str : substitute;
}

//============================================================================
void idf_location_list_str(locationList *list, genString &result)
{
  result = "";
  if (list != NULL) {
    int length = list->size();
    for (int i = 0; i < length; i += 1) {
      locationStruct *loc = (*list)[i];
      genString loc_str;
      idf_location_str(loc, loc_str);
      if (i > 0) {
      	result += ", ";
      }
      result += loc_str;
    }
  }
}

//============================================================================
static int SS_abs(int i)
{
  return i>0 ? i : -i;
}

//============================================================================
static boolean SS_compare_zero_end_line(const locationStruct &l1,
					const locationStruct &l2, 
                                        int &order, const genString &reason)
{
  if (l1.end_line == 0 && l2.end_line != 0 && l1.start_line != 0 && l1.end_column > 0) {
    order = 0;
    if (l1.start_line != l2.end_line) {
      order = (l1.start_line < l2.end_line ? -1 : 1);
    }
    else if (reason.compare("start_column") == 0) {
      if (SS_abs(l1.start_column - l2.start_column) > 3)
      {
        order = (l1.start_column < l2.start_column ? -1 : 1);
      }
      return true;
    }
    else if (reason.compare("end_column") == 0) {
      int end1_column = l1.start_column + l1.end_column -1;
      if (SS_abs(end1_column - l2.end_column) > 3)
      {
        order = (end1_column < l2.end_column ? -1 : 1);
      }
      return true;
    }
    else if (reason.compare("parentheses") == 0) {
      int end1_column = l1.start_column + l1.end_column -1;
      if (   ((l1.start_column - l2.start_column) != (l2.end_column - end1_column))
          || (SS_abs(l1.start_column-l2.start_column) != 1)) {
        order = (l1.start_column < l2.start_column ? -1 : 1);
      }
      return true;
    }
    else {
      return idf_compare_zero_end_line (l1, l2, order);
    }
  }
  else {
    return false;
  }
}

//============================================================================
static boolean SS_match_locations(const locationStruct *loc1,
      	      	      	          const locationStruct *loc2,
      	      	      	          const idfModel &model1, const idfModel &model2,
                                  const genString &reason)
{
  int order;
  if (   SS_compare_zero_end_line(*loc1, *loc2, order, reason)
      || SS_compare_zero_end_line(*loc2, *loc1, order, reason)) {
    return order == 0;
  }
  if (reason.compare("start_column") == 0) {
    return SS_abs(loc1->start_column - loc2->start_column)<3;
  }
  else if (reason.compare("end_column") == 0) {
    return SS_abs(loc1->end_column - loc2->end_column)<3;
  }
  else if (reason.compare("start_line") == 0) {
    return SS_abs(loc1->start_line - loc2->start_line)<3;
  }
  else if (reason.compare("end_line") == 0) {
    return SS_abs(loc1->end_line - loc2->end_line)<3;
  }
  else if (reason.compare("parentheses") == 0) {
    return (   (   (loc1->end_column - loc2->end_column)
                == (loc2->start_column - loc1->start_column))
            && (SS_abs(loc1->end_column - loc2->end_column) == 1));
  }
  return false;
}

//============================================================================
static boolean equal_locations(const locationStruct *loc1,
      	      	      	       const locationStruct *loc2,
      	      	      	       const idfModel &model1, const idfModel &model2,
      	      	      	       boolean ignore_omitted_length,
      	      	      	       boolean ignore_length_value,
     	      	      	       genString *reason = 0)
{
  if (loc1 == NULL || loc2 == NULL) {
    if (reason != 0) {
      *reason = "presence";
    }
    return loc1 == loc2;
  }
  boolean fudge = idf_configuration.contains("File name completeness differs.");
  const char *fn1 = loc1->filename;
  const char *fn2 = loc2->filename;
  idfStringStringDictionary &dict1 = model1.getFiles();
  idfStringStringDictionary &dict2 = model2.getFiles();
  const char *match1 = dict1.lookup(fn1);
  const char *match2 = dict2.lookup(fn2);
  if (fudge ? !idf_filename_match(fn1, fn2)
      	    : !(idf_strmatch(fn1, fn2) || idf_strmatch(match2, fn1) || idf_strmatch(match1, fn2))) {
    if (reason != 0) {
      *reason = "filename";
    }
    return false;
  }
  if (loc1->start_line != loc2->start_line) {
    if (reason != 0) {
      *reason = "start_line";
    }
    return false;
  }
  if (loc1->start_column != loc2->start_column) {
    if (reason != 0) {
      *reason = "start_column";
    }
    return false;
  }
  boolean loc1_no_end = loc1->end_line == 0 && loc1->end_column == 0;
  boolean loc2_no_end = loc2->end_line == 0 && loc2->end_column == 0;
  if (   (   idf_configuration.contains("Source locations differ in presence of length specification.")
          || ignore_omitted_length)
      && (loc1_no_end || loc2_no_end)) {
    return true;
  }
  if (ignore_length_value && !loc1_no_end && !loc2_no_end) {
    return true;
  }
  int order;
  if (   idf_compare_zero_end_line(*loc1, *loc2, order)
      || idf_compare_zero_end_line(*loc2, *loc1, order)) {
    if (order == 0) {
      return true;
    }
    if (reason != 0) {
      *reason = "end_column";
    }
    return false;
  }
  if (loc1->end_line != loc2->end_line) {
    if (reason != 0) {
      *reason = "end_line";
    }
    return false;
  }
  if (loc1->end_column != loc2->end_column) {
    if (reason != 0) {
      *reason = "end_column";
    }
    return false;
  }
  return true;
}

//============================================================================
static boolean SS_match_location_lists(locationList *loc1, locationList *loc2,
                                       const idfModel &model1, const idfModel &model2,
                                       const genString &reason)
{
  for (int i = 0; i < loc1->size(); i += 1) {
    if (!SS_match_locations((*loc1)[i], (*loc2)[i], model1, model2, reason)) {
      return false;
    }
  }
  return true;
}

//============================================================================
static boolean equal_location_lists(locationList *loc1, locationList *loc2,
      	      	      	      	    const idfModel &model1, const idfModel &model2,
      	      	      	      	    boolean ignore_omitted_length,
      	      	      	      	    boolean ignore_length_value,
      	      	      	      	    genString &reason)
{
  if (loc1 == NULL || loc2 == NULL) {
    reason = "presence";
    return loc1 == loc2;
  }
  if (loc1->size() != loc2->size()) {
    reason = "list length";
    return false;
  }
  for (int i = 0; i < loc1->size(); i += 1) {
    if (!equal_locations((*loc1)[i], (*loc2)[i], model1, model2,
      	      	      	 ignore_omitted_length, ignore_length_value, &reason)) {
      return false;
    }
  }
  return true;
}

//============================================================================
// Where the IF dumped from the model has separate type and class symbols
// for one class, class_of_type and type_of_symbol take you back and forth.
// type_of_symbol can be used for any symbol, not just classes.
//============================================================================
static idfSymbol *class_of_type(idfSymbol &sym)
{
  idfRelationshipDictionary *rdp = sym.getRelations().lookup("type");
  if (rdp != NULL) {
    for (idfIterator iter = rdp->first(); !rdp->done(iter); rdp->next(iter)) {
      int sym_id = rdp->index(iter);
      idfSymbol *sp = sym.getModel().getDictionary().lookup(sym_id);
      if (sp != NULL && idf_strmatch(sp->getKind(), "class")) {
      	// This is the class of a type.
      	return sp;
      }
    }
  }
  return NULL;
}

//============================================================================
static idfSymbol *type_of_symbol(idfSymbol &sym)
{
  idfRelationshipDictionary *rdp = sym.getRelations().lookup("~type");
  if (rdp != NULL) {
    for (idfIterator iter = rdp->first(); !rdp->done(iter); rdp->next(iter)) {
      int sym_id = rdp->index(iter);
      idfSymbol *sp = sym.getModel().getDictionary().lookup(sym_id);
      if (sp != NULL && idf_strmatch(sp->getKind(), "type")
                     && rdp->lookup(iter) == NULL) {
      	// This is the type of a symbol.
      	return sp;
      }
    }
  }
  return NULL;
}

//============================================================================
// Given a symbol, allocate a string and fill it with the name of the symbol 
// prefaced by its type.  Returns NULL if the symbol has no name and returns 
// the symbol name if the symbol has no type or the type has no name.
//============================================================================
static char *name_with_type(idfSymbol &sym)
{
  char *result = NULL;
  char *temp;
  const char *sym_name = sym.getName();
  if (sym_name != NULL) {
    int length = strlen(sym_name) + 1;
    idfSymbol *type = type_of_symbol(sym);
    const char *type_name = (type != NULL) ? type->getName() : NULL;
    if (type_name != NULL) {
      length += (strlen(type->getName()) + 1);
      result = new char[length];
      temp = result;
      while (*type_name != '\0') {
        *temp = *type_name;
        temp++;
        type_name++;
      }
      *temp = ' ';
      temp++;
    }
    else {
      result = new char[length];
      temp = result;
    }
    while (*sym_name != '\0') {
      *temp = *sym_name;
      temp++;
      sym_name++;
    }
    *temp = '\0';
  }
  return result;
}

//============================================================================
// Debbuging Functions for Reporting All That's There
// (as distinct from the usual reporting of just differences)
//============================================================================

//============================================================================
// ast_there()
//----------------------------------------------------------------------------
// Outputs the AST given by 'node' as a sequence of one or more lines.
// Each line is indented using the 'prefix' string.
//============================================================================
static void ast_there(idfAst *node, char *prefix)
{
    (*ostrm) << (node->getMatch() != NULL ? "M" : " ");
    if (prefix != NULL) {
	(*ostrm) << prefix;
    }
    const char *kind = node->getKind();
    if (kind == NULL) {
      kind = "<kind missing>";
    }
    (*ostrm) << kind;

    if (node->getIdentifier() > 0) {
      (*ostrm) << " [" << node->getIdentifier() << "]";
    }

    genString loc;
    idf_location_list_str(node->getLocation(), loc);
    if (loc.length() > 0) {
      (*ostrm) << ' ' << loc.str();
    }

    if (strcmp(kind, "constant") == 0) {
      const char *value = node->getValue();
      if (value == 0) {
      	value = "<value missing>";
      }
      (*ostrm) << " {\"" << value << "\"}";
    }
    else {
      (*ostrm) << " {";
#if defined(IDF_DEBUG)
      idfAst *match = node->getMatch();
      if (match != 0) {
      	(*ostrm) << " matches ";
	const char *kind = match->getKind();
	if (kind == NULL) {
	  kind = "<kind missing>";
	}
	(*ostrm) << kind;

	if (match->getIdentifier() > 0) {
	  (*ostrm) << " [" << match->getIdentifier() << "]";
	}

	genString loc;
	idf_location_list_str(match->getLocation(), loc);
	if (loc.length() > 0) {
	  (*ostrm) << ' ' << loc.str();
	}
      }
#endif      	
      (*ostrm) << endl;

      // Output child nodes, with revised indentation prefix.
      genString pref = prefix;
      pref += "  ";
      int num_children = node->getNumChildren();
      for (int i = 0; i < num_children; i += 1) {
      	idfAst *child = node->getChild(i);
	ast_there(child, pref);
      }
      (*ostrm) << ' ';
      if (prefix != NULL) {
      	(*ostrm) << prefix;
      }
      (*ostrm) << "}";
    }
    (*ostrm) << endl;
}

//============================================================================
static void report_there(const idfModel &model)
{
  // Show SMT.
  for (idfSmt *sp = model.getSmt(); sp != NULL; sp = sp->getNext()) {
    genString s;
    genString loc;
    idf_location_list_str(sp->getLocation(), loc);
    char match = sp->getMatch() != 0 ? 'M' : ' ';
    if (!sp->isKeyword()) {
      s.printf("%c  SMT [%d] %s", match, sp->getId(), loc.str());
    }
    else {
      const char *name = sp->getName();
      if (name == NULL) {
      	name = "<name missing>";
      }
      s.printf("%c  SMT keyword \"%s\" %s", match, name, loc.str());
    }      
    idf_report(s.str(), incidental_note);
  }

  // Show symbols.
  const idfSymbolDictionary &dict = model.getDictionary();
  for (idfIterator iter = dict.first(); !dict.done(iter); dict.next(iter)) {
    idfSymbol *sym = dict.lookup(iter);
    genString str;
    const char *name = sym->getName();
    if (name == NULL) {
      name = "<name missing>";
    }
    char match = sym->getMatch() != NULL ? 'M' : ' ';
    genString loc;
    idf_location_str(sym->getLocation(), loc);
    str.printf("%c  SYM [%d] \"%s\" %s", match, dict.index(iter), name, loc.str());
    idf_report(str, incidental_note);
  }

  // Show AST.
  idfAstDictionary *astd = model.getAst();
  for (idfIterator asti = astd->first(); !astd->done(asti); astd->next(asti)) {
    idfAstSiblings *astRoots = astd->lookup(asti);
    int n_roots = astRoots != NULL ? astRoots->getNumber() : 0;
    (*ostrm) << " AST {" << endl;
    for (int i = 0; i < n_roots; i += 1) {
      idfAst *ap = astRoots->getSibling(i);
      ast_there(ap, "  ");
    }
    (*ostrm) << " }" << endl;
  }
}

//============================================================================
// Reporting Differences in Files
//============================================================================

//============================================================================
static void report_files(const idfModel &model1, const idfModel &model2)
{
  boolean fudge = idf_configuration.contains("File name completeness differs.");

  idfStringStringDictionary &dict = model1.getFiles();
  idfIterator iter;
  for (iter = dict.first(); !dict.done(iter); dict.next(iter)) {
    const char *fn1 = dict.index(iter);
    const char *fn2 = dict.lookup(iter);
    if (fn2 == NULL) {
      // fn1 was never matched with another file.
    }
    else {
      if (fudge ? !idf_filename_match(fn1, fn2)
		: !idf_strmatch(fn1, fn2)) {
	genString msg;
	msg.printf("File names differ. %s uses \"%s\", while %s uses \"%s\".",
		   model1.getFilename(), fn1 != NULL ? fn1 : "<NULL>",
		   model2.getFilename(), fn2 != NULL ? fn2 : "<NULL>");
	idf_report(msg, difference_note);
      }
    }
  }
  idfStringStringDictionary &dict2 = model2.getFiles();
  for (idfIterator iter2 = dict2.first(); !dict2.done(iter); dict2.next(iter)) {
    const char *fn2 = dict2.lookup(iter);
    const char *fn1 = dict2.index(iter);
    if (fudge ? !idf_filename_match(fn1, fn2)
              : !idf_strmatch(fn1, fn2)) {
      // Avoid printing the same mismatch over again.
      if (fn2 == NULL) {
      	// fn1 was never matched with another file.
      }
      else if (idf_strmatch(fn1, dict.lookup(fn2))) {
      	// already reported
      }
      else {
        genString msg;
        msg.printf("File names differ. %s uses \"%s\", while %s uses \"%s\".",
        	   model1.getFilename(), fn2,
                   model2.getFilename(), fn1);
        idf_report(msg, difference_note);
      }
    }
  }
}

//============================================================================
// Reporting Differences in SMTs
//============================================================================

//============================================================================
static void smt_id(idfSmt &sr, const char *filename, genString &id_str)
{
  genString loc;
  idf_location_list_str(sr.getLocation(), loc);
  if (!sr.isKeyword()) {
    id_str.printf("%s: SMT [%d] %s", filename, sr.getId(), loc.str());
  }
  else {
    const char *name = sr.getName();
    if (name == NULL) {
      name = "<name missing>";
    }
    id_str.printf("%s: SMT keyword \"%s\" %s", filename, name, loc.str());
  }
}

//============================================================================
// Returns true if loc and other_loc match on the basis that the
// length in loc is implicitly equal to the length of name.
//============================================================================
static boolean keyword_loc_match(const char *name, locationList *loc, locationList *other_loc)
{
  if (   name == 0 || loc == 0 || other_loc == 0
      || loc->size() != 1 || other_loc->size() != 1) {
    return false;
  }
  locationStruct &ls = *((*loc)[0]);
  locationStruct &other_ls = *((*other_loc)[0]);
  boolean fudge = idf_configuration.contains("File name completeness differs.");
  if (fudge ? !idf_filename_match(ls.filename, other_ls.filename)
      	    : !idf_strmatch(ls.filename, other_ls.filename)) {
    return false;
  }
  
  if (   ls.start_line == other_ls.start_line
      && ls.start_column == other_ls.start_column
      && ls.end_line == 0
      && ls.end_column == 0
      && (   (   other_ls.end_line == 0
              && other_ls.end_column == strlen(name))
          || (   other_ls.end_line == other_ls.start_line
              && other_ls.end_column - other_ls.start_column + 1 == strlen(name)))) {
    return true;
  }
  return false;
}

//============================================================================
static boolean sym_has_attribute(idfSymbol &symbol, const char *attribute_name)
{
  return symbol.getAttributes().contains(attribute_name);
}

//============================================================================
static const char *get_filename(idfSmt *smt)
{
  if (smt != NULL) {
    locationList *ll = smt->getLocation();
    if (ll != NULL && ll->size() > 0) {
      return (*ll)[0]->filename;
    }
  }
  return NULL;
}

//============================================================================
static const char *get_sym_kind(idfSmt &smt)
{
  if (smt.isId()) {
    idfSymbol *sym = smt.getModel().getDictionary().lookup(smt.getId());
    if (sym != NULL) {
      return sym->getKind();
    }
  }
  return NULL;
}

//============================================================================
static const char *get_sym_name(idfSmt &smt)
{
  if (smt.isId()) {
    idfSymbol *sym = smt.getModel().getDictionary().lookup(smt.getId());
    if (sym != NULL) {
      return sym->getName();
    }
  }
  return NULL;
}

//============================================================================
// Report SMT node mismatches and differences.
//============================================================================
static void report_SMT(idfSmt *smt, const idfModel &model1, const idfModel &model2)
{
  for (idfSmt *sp = smt; sp != NULL; sp = sp->getNext()) {
    if (sp->getDiscrepancyReported()) {
      continue;
    }
    genString id;
    smt_id(*sp, model1.getFilename(), id);
    if (sp->getMatch() == 0) {
      idfSymbol *sym = sp->isId() ? model1.getDictionary().lookup(sp->getId()) : NULL;
      const char *sym_name = sym != NULL ? sym->getName() : NULL;
      const char *sym_kind = sym != NULL ? sym->getKind() : NULL;
      idfStringSet *ssp;
      if (idf_configuration.contains("SMT symbol nodes for operators do not correspond.")
      	  && sym_name != NULL && strstr(sym_name, "operator") != NULL) {
      	// Don't report lack of corresponding SMT node to SMT symbol node of
      	// operator.
      }
      else if ( (ssp = idf_configuration.lookup("SMT records for () symbols"
                                                   " are missing.")) != NULL
               && sym_kind != NULL 
               && ssp->contains( sym_kind )
              ) {
        // Don't report lack of corresponding SMT node to SMT symbol node 
        // which is listed in configuration as missing SMT records.
      }
      else if (idf_configuration.contains("SMT symbol nodes for implicit constructors"
                                          " and destructors do not correspond.")
      	       && sym != NULL
      	       && (   sym_has_attribute(*sym, "constructor")
                   || sym_has_attribute(*sym, "destructor"))
      	       && sp->isImplicit()
              ) {
      	// Don't report lack of corresponding SMT node to SMT symbol node of
      	// implicit constructor or destructor reference.
      }
      else if (   idf_interesting_filename != NULL
               && idf_configuration.contains(
                          "SMT nodes are absent for uninteresting files.")
      	       && !idf_filename_match(idf_interesting_filename, get_filename(sp))) {
      	// Don't report lack of corresponding SMT node if this SMT node is
        // not in the interesting file.      
      }
      else {
	genString s;
	s.printf("%s has no corresponding node in other file.", id.str());
	idf_report(s.str(), difference_note);
	sp->setDiscrepancyReported(true);
      }
    }
    else {
      idfSmt &match = *(sp->getMatch());
      genString mid;
      smt_id(match, model2.getFilename(), mid);
      genString reason;
      if (!equal_location_lists(sp->getLocation(), match.getLocation(),
                                model1, model2, false, false, reason)) {
      	boolean ignore = false;
      	if (idf_configuration.contains("Keywords differ in explicitness of length.")) {
      	  if (   sp->isKeyword()   && keyword_loc_match(sp->getName(),
                                                        sp->getLocation(),
                                                        match.getLocation())
      	      || match.isKeyword() && keyword_loc_match(match.getName(),
                                                        match.getLocation(),
                                                        sp->getLocation())) {
      	    ignore = true;
      	  }
      	}
      	idfStringSet *ssp;
      	if ((ssp = idf_configuration.lookup("SMT records for () symbols"
                                            " differ in explicitness of length.")) != NULL) {
      	  const char *skind = get_sym_kind(*sp);
      	  const char *mkind = get_sym_kind(match);
      	  if (      skind != NULL && ssp->contains(skind)
                 && keyword_loc_match(get_sym_name(*sp),
                                      sp->getLocation(),
                                      match.getLocation())
      	      ||    mkind != NULL && ssp->contains(mkind)
                 && keyword_loc_match(get_sym_name(match),
                                      match.getLocation(),
                                      sp->getLocation())) {
      	    ignore = true;
      	  }
      
      	}
      	if (idf_configuration.contains("SMT nodes of destructors use different"
                                       " portions of the identifier.")) {
      	  if (idf_destructor_location_match(*sp, match, model1, model2)) {
      	    ignore = true;
      	  }
      	}
      	if (!ignore) {
	  genString s;
	  s.printf("%s and %s differ in location (%s).", id.str(), mid.str(), reason.str());
	  idf_report(s.str(), difference_note);
	  sp->setDiscrepancyReported(true);
	  match.setDiscrepancyReported(true);
      	}
      }
      if (!idf_strmatch(sp->getName(), match.getName())) {
      	boolean ignore = false;
      	if (idf_configuration.contains("Indirect keyword names are unchecked.")) {
      	  if (sp->isKeyword() && sp->getName() == NULL
      	      && keyword_loc_match(match.getName(), match.getLocation(), sp->getLocation())) {
      	    ignore = true;
      	  }
      	  if (match.isKeyword() && match.getName() == NULL
      	      && keyword_loc_match(sp->getName(), sp->getLocation(), match.getLocation())) {
      	    ignore = true;
      	  }
      	}
      	if (!ignore) {
	  genString s;
	  s.printf("%s and %s differ in name (%s vs. %s).", id.str(), mid.str(),
      	      	   nonnull(sp->getName()), nonnull(match.getName()));
	  idf_report(s.str(), difference_note);
	  sp->setDiscrepancyReported(true);
	  match.setDiscrepancyReported(true);
      	}
      }
      idfSymbol *sym = sp->isId() ? model1.getDictionary().lookup(sp->getId()) : NULL;
      idfSymbol *msym = match.isId() ? model2.getDictionary().lookup(match.getId()) : NULL;
      if ((sym == NULL || msym == NULL) ? (msym != sym) : (   sym->getMatch() != msym 
                                                           && msym->getMatch() != sym)) {
      	genString s;
      	s.printf("%s and %s differ in symbol.", id.str(), mid.str());
      	idf_report(s.str(), difference_note);
      	sp->setDiscrepancyReported(true);
      	if (msym != 0 && msym->getMatch() == sym) {
      	  match.setDiscrepancyReported(true);
      	}
      }
      else if (sp->isId() != match.isId()) {
      	genString s;
      	s.printf("%s and %s differ in whether they contain an identifier.", id.str(), mid.str());
      	idf_report(s.str(), difference_note);
      	sp->setDiscrepancyReported(true);
   	match.setDiscrepancyReported(true);
      }
      if (sp->isKeyword() != match.isKeyword()) {
      	genString s;
      	s.printf("%s and %s differ in whether they contain a keyword.", id.str(), mid.str());
      	idf_report(s.str(), difference_note);
      	sp->setDiscrepancyReported(true);
   	match.setDiscrepancyReported(true);
      }
      if (sp->isImplicit() != match.isImplicit()) {
      	genString s;
      	s.printf("%s and %s differ in whether they are implicit.", id.str(), mid.str());
      	idf_report(s.str(), difference_note);
      	sp->setDiscrepancyReported(true);
   	match.setDiscrepancyReported(true);
      }
    }
  }
}

//============================================================================
// Report SMT node mismatches and differences.
//============================================================================
static void report_SMT(const idfModel &model1, const idfModel &model2)
{
  report_SMT(model1.getSmt(), model1, model2);
  report_SMT(model2.getSmt(), model2, model1);
}

//============================================================================
// Reporting Differeces in Symbol Records
// (including attributes and relations)
//============================================================================

//============================================================================
static boolean is_type_of_class(idfSymbol &sym, const idfModel &model)
{
  return class_of_type(sym) != NULL;
}

//============================================================================
static boolean name_is_constructor(const char *nm)
{
  boolean answer = false;
  if (nm != NULL) {
    int offset = 0;
    int i = 0;
    boolean done = false;
    while (!done) {
      if (nm[i] == '\0') {
        done = true;
      }
      else if (offset == 0) {
        if (nm[i] == ':' && nm[i+1] == ':') {
          i++;
          offset = i+1;
        }
      }
      else {
        if (nm[i] != nm[i-offset]) {
          done = true;
          if (i == 2*(offset-1) && nm[i] == '(' && nm[i-offset] == ':') {
            answer = true;
          }
        }
      }
      i++;
    }
  }
  return answer;
}

//============================================================================
// Returns 'true' if 'sym' is OK to have no corresponding symbol in the other
// IF file.
//
// If 'for_relation' is true, then return 'true' if not only is it OK
// for the symbol to be absent, but it is also OK for any relations that
// might involve the symbol to be absent.
//============================================================================
static boolean ignore_symbol(idfSymbol &sym, const idfModel &model,
                             boolean for_relation)
{
  const char *nm;
  const char *fn;
  const unsigned *id;
  idfStringSet *ssp;
  const char *kind = sym.getKind();
  if (   idf_strmatch(sym.getKind(), "type")
      && idf_configuration.contains("Unrelated type symbols are absent.")) {
    // Don't report the unmatched type symbol.
    return !for_relation;
  }
  else if (   idf_strmatch(sym.getKind(), "type")
      && idf_configuration.contains("Type symbols of classes are absent.")
      && is_type_of_class(sym, model)) {
    // Don't report the type symbol that is the type of a class.
    return !for_relation;
  }
  else if (   idf_strmatch(kind, "constant")
	   && idf_configuration.contains("Constant symbols are absent.")) {
    // Don't report the unmatched constant symbol.
    return true;
  }
  else if (   idf_strmatch(kind, "namespace")
	   && idf_configuration.contains("Namespace symbols are absent.")) {
    // Don't report the unmatched constant symbol.
    return true;
  }
  else if (   (ssp = idf_configuration.lookup("Symbols of kind () are absent.")) != NULL
           && kind != NULL && ssp->contains(kind)) {
    // Don't report the unmatched symbol of the specified kind.
    return true;
  }
  else if (idf_configuration.contains("Symbols may be absent if they are not defined within the interesting file.")) {
    const locationStruct *loc = sym.getLocation();
    if (loc == NULL) {
      return !for_relation;
    }
    else if (   idf_interesting_filename != NULL
	     && !idf_filename_match(idf_interesting_filename, loc->filename)) {
      return !for_relation;
    }
  }
  else if (   sym.isGenerated()
	   && idf_configuration.contains("Compiler-generated symbols are absent.")) {
    // Don't report the unmatched compiler-generated symbol.
    return true;
  }
  else if (   (nm = sym.getName()) != NULL
           && strstr(nm, "operator=(") != NULL
      	   && sym.getRelations().contains("~context")
      	   && idf_configuration.contains("Symbol records of class assignment operators are absent.")) {
    // Don't report the unmatched assignment operator.
    return true;
  }
  else if (   idf_configuration.contains("Symbol records of constructors are absent.")
           && (nm = sym.getName()) != NULL
           && (   sym.getAttributes().contains("constructor")
               || name_is_constructor(nm))) {
    // Don't report the unmatched constructor.
    return true;
  }
  else if (   sym.getLocalVariableName(nm, fn, id)
           && idf_strmatch(nm, "this")
           && idf_configuration.contains("Member function 'this' symbols are absent.")) {
    // Don't report the unmatched 'this' symbol.
    return true;
  }
  return false;
}

//============================================================================
static void symbol_id(idfSymbol &sym, genString &str)
{
  const char *name = sym.getName();
  if (name == NULL) {
    name = "<name missing>";
  }
  genString loc;
  idf_location_str(sym.getLocation(), loc);
#if 0
  genString match_str;
  if (sym.getMatch() != NULL) {
    match_str.printf(" matching %d", sym.getMatch()->getIdentNum());
  }
  else {
    match_str.printf(" unmatching");
  }
  str.printf("SYM [%d] \"%s\" %s%s", sym.getIdentNum(), name, loc.str(), match_str.str());
#else
  const char *loc_str = loc.str();
  if (strlen(loc_str) > 0) {
    str.printf("SYM [%d] \"%s\" %s", sym.getIdentNum(), name, loc.str());
  }
  else {
    str.printf("SYM [%d] \"%s\"", sym.getIdentNum(), name);
  }
#endif
}

//============================================================================
static void symbol_id(idfSymbol &sym,
      	      	      const char *filename, genString &str)
{
  const char *name = sym.getName();
  if (name == NULL) {
    name = "<name missing>";
  }
  genString loc;
  idf_location_str(sym.getLocation(), loc);
#if 0
  genString match_str;
  if (sym.getMatch() != NULL) {
    match_str.printf(" matching %d", sym.getMatch()->getIdentNum());
  }
  else {
    match_str.printf(" unmatching");
  }
  str.printf("%s: SYM [%d] \"%s\" %s%s", filename, sym.getIdentNum(), name, loc.str(), match_str.str());
#else
  str.printf("%s: SYM [%d] \"%s\" %s", filename, sym.getIdentNum(), name, loc.str());
#endif
}

//============================================================================
// Report mismatches in attributes for the given symbols.
// Return true if any mismatches found.
//============================================================================
static boolean report_attributes(idfAttributeDictionary &attr1,
      	      	      	      	 idfAttributeDictionary &attr2,
      	      	      	         const char *sym_id1, const char *sym_id2,
      	      	      	      	 boolean (*filter)(void *, const char *),
      	      	      	      	 void *filter_object)
{
  boolean discrepancy = false;
  idfStringSet *ssp = idf_configuration.lookup("Attribute differs().");
  for (idfIterator iter = attr1.first(); !attr1.done(iter); attr1.next(iter)) {
    const char *attribute = attr1.index(iter);
    const char *value1 = attr1.lookup(iter);
    const char *value2 = attr2.lookup(attribute);
    if (   !idf_strmatch(value1, value2)
        && !(ssp != NULL && ssp->lookup(attribute))
        && !(filter != NULL && filter(filter_object, attribute))) {
      boolean ignore = false;
      genString s;
      if (!attr2.contains(attribute)) {
        s.printf("%s has \"%s\" attribute, but correspondingly %s does not.",
                 sym_id1, attribute, sym_id2);
        if (   idf_configuration.contains("Watch out for inlinestatic attribute.")
            && (   (   idf_strmatch(attribute, "inlinestatic") 
                    && attr2.contains("inline")) 
                || (   (idf_strmatch(attribute, "inline") || idf_strmatch(attribute, "static"))
                    && attr2.contains("inlinestatic")))) {
          // Either 1 had inlinestatic and 2 had inline, or 1 had inline or static and 2 had
          // inlinestatic;  either way, this is OK.
          ignore = true;
        }
      }
      else {
        s.printf("%s has \"%s\" attribute value \"%s\","
                 " but correspondingly %s has value \"%s\".",
                 sym_id1, attribute, nonnull(value1, "<default value>"),
                 sym_id2, nonnull(value2, "<default value>"));
      }
      if (!ignore) {
        idf_report(s, difference_note);
      }
      discrepancy = true;
    }
  }
  // Now look for attributes missing from attr1.
  for (idfIterator iter2 = attr2.first(); !attr2.done(iter2); attr2.next(iter2)) {
    const char *attribute = attr2.index(iter2);
    const char *value1 = attr1.lookup(attribute);
    if (   value1 == NULL
        && !(ssp != NULL && ssp->lookup(attribute))
        && !(filter != NULL && filter(filter_object, attribute))) {
      boolean ignore = false;
      genString s;
      s.printf("%s has \"%s\" attribute, but correspondingly %s does not.",
               sym_id2, attribute, sym_id1);
      if (   idf_configuration.contains("Watch out for inlinestatic attribute.")
          && (   (   idf_strmatch(attribute, "inlinestatic") 
                  && attr1.contains("inline")) 
              || (   (idf_strmatch(attribute, "inline") || idf_strmatch(attribute, "static"))
                  && attr1.contains("inlinestatic")))) {
        // Either 1 had inlinestatic and 2 had inline, or 1 had inline or static and 2 had
        // inlinestatic;  either way, this is OK.
        ignore = true;
      }
      if (!ignore) {
        idf_report(s, difference_note);
      }
      discrepancy = true;
    }
  }
  return discrepancy;
}

//============================================================================
// Report mismatches in attributes for the given symbols.
// Return true if any mismatches found.
//============================================================================
static boolean report_attributes(idfAttributeDictionary *attr1,
      	      	      	      	 idfAttributeDictionary *attr2,
      	      	      	         const char *sym_id1, const char *sym_id2,
      	      	      	      	 boolean (*filter)(void *, const char *),
      	      	      	      	 void *filter_object)
{
  if (attr1 == NULL || attr2 == NULL) {
    if (attr1 != NULL) {
      idfIterator iter = attr1->first();
      if (!attr1->done(iter)) {
      	const char *attribute = attr1->index(iter);
      	if (!(filter != NULL && filter(filter_object, attribute))) {
	  genString s;
	  s.printf("%s has \"%s\" attribute, but correspondingly %s does not.",
		   sym_id1, attribute, sym_id2);
	  s.printf("%s and %s differ in whether they have attributes", sym_id1, sym_id2);
	  idf_report(s.str(), difference_note);
	  return true;
      	}
      }
    }
    if (attr2 != NULL) {
      idfIterator iter = attr2->first();
      if (!attr2->done(iter)) {
      	const char *attribute = attr2->index(iter);
      	if (!(filter != NULL && filter(filter_object, attribute))) {
	  genString s;
	  s.printf("%s has \"%s\" attribute, but correspondingly %s does not.",
		   sym_id2, attribute, sym_id1);
	  s.printf("%s and %s differ in whether they have attributes", sym_id1, sym_id2);
	  idf_report(s.str(), difference_note);
	  return true;
	}
      }
    }
  }
  else {
    return report_attributes(*attr1, *attr2, sym_id1, sym_id2, filter, filter_object);
  }
  return false;
}

//============================================================================
// Report mismatches in relations for the given symbols.
// Return true if any mismatches are found.
//============================================================================
static boolean report_relations(idfRelationDictionary &rel1,
      	      	      	      	idfRelationDictionary &rel2,
      	      	      	        idfSymbol &sym1, idfSymbol &sym2,
      	      	      	        const char *sym_id1, const char *sym_id2,
      	      	      	        const idfModel &model1,
      	      	      	      	const idfModel &model2,
      	      	      	      	boolean details)
{
  boolean discrepancy = false;
  idfStringSet *ssp = idf_configuration.lookup("Relation differs().");
  for (idfIterator iter = rel1.first(); !rel1.done(iter); rel1.next(iter)) {
    const char *relation = rel1.index(iter);
    idfRelationshipDictionary *rd1 = rel1.lookup(iter);
    idfRelationshipDictionary *rd2 = rel2.lookup(relation);
    if (   (relation[0] == '~' && idf_configuration.contains("Inverse relation differences are noise.")) 
        || (relation[0] != '~' && idf_configuration.contains("Direct relation differences are noise."))
       ) {
      // Don't report.
    }
    else if (ssp != 0 && ssp->lookup(relation)) {
      // Don't report.
    }
    else if (rd1 == NULL) {
      // should not happen
    }
#if 0
// Removed this, having noticed that it makes the special handling of the "type"
// relation more difficult, and on the basis that a relation to 0 symbols is equivalent
// to no relation at all.
    else if (rd2 == NULL) {
      genString s;
      s.printf("%s and %s differ in \"%s\" relation.", sym_id1, sym_id2, relation);
      idf_report(s.str(), difference_note);
      discrepancy = true;
    }
#endif
    else {
      for (idfIterator ri = rd1->first(); !rd1->done(ri); rd1->next(ri)) {
      	int rsym1_num = rd1->index(ri);
      	idfAttributeDictionary *rad1 = rd1->lookup(ri);

      	// The symbol that's related to sym1 by rel1:
      	idfSymbol *rsym1 = model1.getDictionary().lookup(rsym1_num);
      	if (rsym1 != NULL && rsym1->getMatch() != 0) {
      	  idfSymbol *rsym2 = rsym1->getMatch();
      	  genString rsym_id1;
      	  symbol_id(*rsym1, rsym_id1);
      	  genString rsym_id2;
      	  symbol_id(*rsym2, rsym_id2);
      	  if (rd2 == NULL || !rd2->contains(rsym2->getIdentNum())) { // Relation didn't match.
      	    boolean skip = false;

            genString contextual_rel = "";
            if (relation != NULL && rsym1 != NULL && rsym1->getKind() != NULL) {
              contextual_rel.printf("%s of %s", relation, rsym1->getKind());
            }
            genString mcontextual_rel = "";
            if (relation != NULL && rsym2 != NULL && rsym2->getKind() != NULL) {
              mcontextual_rel.printf("%s of %s", relation, rsym2->getKind());
            }
            if (ssp != NULL && (ssp->contains(contextual_rel) || ssp->contains(mcontextual_rel))) {
              skip = true;
            }

      	    // Skip the test if we don't care about the IF representing the structure
      	    // of type symbols.
      	    boolean inverse = (relation[0] == '~');
      	    if (strcmp(relation + (inverse ? 1 : 0), "type") == 0) {
              // which symbol to check for type
	      idfSymbol &target_sym = inverse ? sym1 : *rsym1;
              if (idf_configuration.contains("Type symbols differ in structure.")) {
		if (idf_strmatch(target_sym.getKind(), "type")) {
		  // Relation represents type of a type.
		  skip = true;
		}
      	      }
      	      if (idf_configuration.contains("Type symbols of classes are absent.")) {
		idfSymbol &source_sym = inverse ? *rsym1 : sym1;
		if (idf_strmatch(source_sym.getKind(), "class")) {
		  // Relation represents type of object is a class.
      	      	  // Since the files are already said to differ in that one file
      	      	  // uses class symbols instead of type symbols, we're OK.
      	      	  //
      	      	  // This doesn't do _all_, since it doesn't handle the discrepancy
      	      	  // that will be found in the file which has type symbols for classes.
      	      	  skip = true;
		}
      	      }
	    }

      	    if (ignore_symbol(sym1, model1, true) || ignore_symbol(*rsym1, model1, true)) {
      	      skip = true;
      	    }

            if (!skip) {
	      genString s;
	      s.printf("%s has \"%s\" relation to %s, but correspondingly %s has _no_ \"%s\" relation to %s.",
                       sym_id1, relation, rsym_id1.str(), sym_id2, relation, rsym_id2.str());
	      idf_report(s.str(), difference_note);
	      discrepancy = true;
            }
      	  }
      	  else if (details) {
      	    idfAttributeDictionary *rad2 = rd2->lookup(rsym2->getIdentNum());
      	    // Relations match! Check attributes.
      	    genString s1;
      	    s1.printf("%s in relation \"%s\" to %s", sym_id1, relation, rsym_id1.str());
      	    genString s2;
      	    s2.printf("%s in relation \"%s\" to %s", sym_id2, relation, rsym_id2.str());
      	    if (report_attributes(rad1, rad2, s1.str(), s2.str(), NULL, NULL)) {
      	      discrepancy = true;
      	    }
      	  }
      	}
      }
    }
  }
  return discrepancy;
}

//============================================================================
// Report mismatches in relations for the given symbols.
// Return true if any mismatches are found.
//============================================================================
static boolean report_relations(idfRelationDictionary &rel1,
      	      	      	      	idfRelationDictionary &rel2,
      	      	      	        idfSymbol &sym1, idfSymbol &sym2,
      	      	      	        const char *sym_id1, const char *sym_id2,
      	      	      	        const idfModel &model1,
      	      	      	      	const idfModel &model2)
{
  boolean discrepancy = 
       report_relations(rel1, rel2, sym1, sym2, sym_id1, sym_id2, model1, model2, true)
    || report_relations(rel2, rel1, sym2, sym1, sym_id2, sym_id1, model2, model1, false);
  return discrepancy;
}

//============================================================================
boolean is_identchar(char c)
{
  return isalnum(c) || c == '_' ;
}

//============================================================================
void next_significant_char(const char *&p)
{
  if (!isspace(*++p)) {
    return;
  }
  boolean prev_is_ident = is_identchar(p[-1]);
  for (;;) {
    // assert isspace(*p)
    if (prev_is_ident && is_identchar(p[1])) {
      return;
    }
    p++;
    if (!isspace(*p)) {
      return;
    }
  }
}

//============================================================================
// Compare strings for a match, ignoring whitespace except as a separator
// between identifiers.
//============================================================================
boolean name_match(const char *name1, const char *name2)
{
  if (name1 == NULL || name2 == NULL) {
    return name1 == name2;
  }
  for (;;) {
    if (*name1 != *name2 && (!isspace(*name1) || !isspace(*name2))) {
      return false;
    }
    if (*name1 == '\0' || *name2 == '\0') {
      return true;
    }
    next_significant_char(name1);
    next_significant_char(name2);
  }
}

//============================================================================
// Compare two strings with the intent of determining whether one is a more
// explicit version of the other.
//============================================================================
static boolean SS_type_match(const char *name1, const char *name2)
{
  return (strstr(name1, name2) != NULL || strstr(name2, name1) != NULL);
}

//============================================================================
// Return the canonical value of the first character of a given string, 
// correcting the offset if necessary, in accordance with config directives.
//============================================================================
static char SS_value(const char *&name)
{
  char result = '\0';
  if (name != NULL) {
    if (   idf_configuration.contains("Single quote is screwed up.")
        && *name == '\\' && *(name+1) == '0' && *(name+2) == '4' && *(name+3) == '7') {
      result = '\'';
      name += 3;
    }
    else if (   idf_configuration.contains("Single quote may be escaped.")
             && *name == '\\' && *(name+1) == '\'') {
      result = '\'';
      name++;
    }
    else if (   idf_configuration.contains("Symbol names differ in treatment of tabs.")
             && *name == '\\' && *(name+1) == 't') {
      result = '\t';
      name++;
    }
    else if (*name == '\\' && *(name+1) == '"') {
      result = '"';
      name++;
    }
    else {
      result = *name;
    }
  }
  return result;
}

//============================================================================
// Correct internal characters of a string in accordance with config directives.
//============================================================================
static char *normalize_string(const char *name)
{
  char *result = NULL;
  if (name != NULL) {
    int length = strlen(name);
    result = new char[length];
    char *temp = result;
    while (*name != '\0') {
      *temp = SS_value (name);
      name++;
      temp++;
    }
    *temp = '\0';
  }
  return result;
}

//============================================================================
// Remove one set of quotes from a string, correcting internal characters in
// accordance with config directives.
//============================================================================
static char *dequote_string(const char *name)
{
  char *result = NULL;
  if (name != NULL) {

    int length = strlen(name);
    boolean found_quote = false;
    if (name[0] == '"' && name[length-1] == '"') {
      name++;
      found_quote = true;
    }
    result = normalize_string (name);
    char *temp = result;
    if (found_quote) {
      while (*temp != '\0') {
        temp++;
      }
      temp--;
      *temp = '\0';
    }
  }
  return result;
}

//============================================================================
static boolean is_c_language(const locationStruct *loc, boolean dflt)
{
  if (loc == NULL) {
    return dflt;
  }
  const char *fn = loc->filename;
  if (fn == NULL) {
    return dflt;
  }
  if (fn != NULL) {
    int fl = strlen(fn);
    if (fl >= 2) {
      const char *c_suffix = fn + fl - 2;
      if (c_suffix[0] == '.' && c_suffix[1] == 'c') {
      	return true;
      }
    }
  }
  return false;
}

//============================================================================
static boolean is_c_linkage(idfSymbol &sym, const idfModel &model)
{
  if (is_c_language(sym.getLocation(), false)) {
    return true;
  }

  const char *val = sym.getAttributes().lookup("linkage");
  return val != NULL && (idf_strmatch(val, "c") || idf_strmatch(val, "C"));
}

//============================================================================
// Report name mismatch between the two symbols.
//
// This is handled from the point of view that the variable@func{block}
// syntax is a hack.  In other words, the names of the symbols are
// first of all expected to match as strings.  Only if they do not
// match as strings are they scrutinized so that the difference
// might be reported more intelligently by interpreting one or both of
// the strings as variable@func{block}.
//============================================================================
static void report_symbol_name(idfSymbol &sym1, idfSymbol &sym2,
                               const idfModel &model1, const idfModel &model2)
{
  boolean (*strmatch)(const char *, const char *);
  if (idf_configuration.contains("Symbol names differ in whitespace.")) {
    strmatch = &name_match;
  }
  else {
    strmatch = &idf_strmatch;
  }
  if (strmatch(sym1.getName(), sym2.getName())) {
    return;
  }

  boolean paren1 = (strchr(sym1.getName(), '(') != NULL);
  boolean paren2 = (strchr(sym2.getName(), '(') != NULL);
  if (   paren1 != paren2
      && (is_c_linkage(sym1, model1) || is_c_linkage(sym2, model2))
      && idf_configuration.contains(
           "C-linkage symbol names differ in presence of parameter list.")) {

    // Grab the plain name, and the name with parenthesized parameter
    // list so that the latter can be copied without the list.
    const char *plain_name = paren1 ? sym2.getName() : sym1.getName();
    const char *fancy_name = paren1 ? sym1.getName() : sym2.getName();
    const char *paren = strchr(fancy_name, '(');
    int len = int(paren - fancy_name);
    char *fancy_names_base = new char[len + 1];
    strncpy(fancy_names_base, fancy_name, len);
    fancy_names_base[len] = '\0';
    if (isspace(fancy_names_base[len - 1])) {
      fancy_names_base[len - 1] = '\0';
    }

    boolean matched = strmatch(plain_name, fancy_names_base);
    delete [] fancy_names_base;

    // Check whether one of the symbols has a linkage(c) attribute.
    if (matched) {
      return;
    }
  }

  {
    boolean report = true;
    genString s;
    genString sym1_str;
    symbol_id(sym1, model1.getFilename(), sym1_str);
    genString sym2_str;
    symbol_id(sym2, model2.getFilename(), sym2_str);

    const char *local1;
    const char *func1;
    const unsigned int *block1;
    boolean is1 = sym1.getLocalVariableName(local1, func1, block1);
    const char *local2;
    const char *func2;
    const unsigned int *block2;
    boolean is2 = sym2.getLocalVariableName(local2, func2, block2);

    if (is1 && is2) {
      if (!strmatch(local1, local2)) {
      	if (local1 != NULL && local2 != NULL /* avoid crash */) {
       	  s.printf("%s and %s differ in local name (%s vs %s).",
      	    sym1_str.str(), sym2_str.str(), local1, local2);
      	}
      	else {
          s.printf("%s and %s differ in local name (unexpected %p %p).",
            sym1_str.str(), sym2_str.str(), (void *)local1, (void *)local2);
      	}
      }
      else if (!strmatch(func1, func2)) {
      	if (func1 != NULL && func2 != NULL /* avoid crash */) {
      	  s.printf("%s and %s differ in defining function name of locals (%s vs %s).",
      	    sym1_str.str(), sym2_str.str(), func1, func2);
      	}
      	else {
          s.printf("%s and %s differ in defining function name of locals (unexpected %p %p).",
            sym1_str.str(), sym2_str.str(), (void *)func1, (void *)func2);
      	}
      }
      else if ((block1 == NULL) != (block2 == NULL)
          || (block1 != NULL && *block1 != *block2)) {
      	if (block1 != NULL && block2 != NULL) {
      	  s.printf("%s and %s differ in block id of locals (%u vs %u).",
      	    sym1_str.str(), sym2_str.str(), *block1, *block2);
      	}
      	else if (block1 != NULL) {
      	  s.printf("%s has a block id of locals in its name, but %s does not.",
      	    sym1_str.str(), sym2_str.str());
      	}
      	else {
      	  s.printf("%s has a block id of locals in its name, but %s does not.",
      	    sym2_str.str(), sym1_str.str());
      	}
      }
      else {
        // The names didn't match, but they're equivalent from the point
        // of view of local name and scope.
      	report = false;
      }
    }
    else if (is1 && !is2) {
      if (strmatch(local1, sym2.getName())) {
      	// mismatch only in that one symbol appears as local
      	if (idf_configuration.contains("Use of scope in local variable names differs.")) {
      	  // reporting suppressed
      	  report = false;
	}
      	else {
      	  s.printf("%s indicates scope of local variable in its name, but %s does not.",
      	    sym1_str.str(), sym2_str.str());
      	}
      }
      else {
      	s.printf("%s and %s differ in name (%s vs %s).", sym1_str.str(), sym2_str.str(),
      	  local1, sym2.getName());
      }
    }
    else if (is2 && !is1) {
      if (strmatch(local2, sym1.getName())) {
      	// mismatch only in that one symbol appears as local
      	if (idf_configuration.contains("Use of scope in local variable names differs.")) {
      	  // reporting suppressed
      	  report = false;
	}
      	else {
      	  s.printf("%s indicates scope of local variable in its name, but %s does not.",
      	    sym2_str.str(), sym1_str.str());
      	}
      }
      else {
      	s.printf("%s and %s differ in name (%s vs %s).", sym1_str.str(), sym2_str.str(),
      	  sym1.getName(), local2);
      }
    }
    else {
      // Neither uses local variable syntax in its name.
      if (   idf_configuration.contains("Number of quotation marks in string names differs.")) {
        char *nq1 = dequote_string(sym1.getName());
        char *nq2 = dequote_string(sym2.getName());
        char *nm1 = normalize_string(sym1.getName());
        char *nm2 = normalize_string(sym2.getName());
        if (strmatch(nq1, nm2) || strmatch(nm1, nq2)) {
          report = false;
        }
        delete nq1;
        delete nq2;
        delete nm1;
        delete nm2;
      }
      if (   idf_configuration.contains("Type symbols differ in explicitness of name.")
          && idf_strmatch(sym1.getKind(), "type")
          && idf_strmatch(sym2.getKind(), "type")
          && SS_type_match (sym1.getName(), sym2.getName())) {
        report = false;
      }
      if (idf_configuration.contains("Symbol names may have types attached.")) {
        // Want to add type to each name and match it against the other's untyped name.
        char *tn1 = name_with_type(sym1);
        char *tn2 = name_with_type(sym2);
        if (   strmatch(tn1, sym2.getName()) || strmatch(tn2, sym1.getName())) {
          report = false;
        }
        delete tn1;
        delete tn2;
      }
      if (report) {
        s.printf("%s and %s differ in name.", sym1_str.str(), sym2_str.str());
      }
    }
    if (report) {
      idf_report(s.str(), difference_note);
      sym1.setDiscrepancyReported(true);
      sym2.setDiscrepancyReported(true);
    }
  }
}

//============================================================================
struct symbol_pair {
  idfSymbol *sym1, *sym2;
  const idfModel *model1, *model2;
};

//============================================================================
// Passed along so that report_attributes can filter out some relations
// to not report. Returns true to suppress reporting.
//============================================================================
static boolean symbol_attribute_filter(void *obj, const char *attribute)
{
  if (obj == NULL || attribute == NULL) {
    return false;
  }
  struct symbol_pair *syms = (symbol_pair *)obj;
  
  idfStringSet *ssp;
  if (   (   syms->sym1->getAttributes().contains("constructor")
          || syms->sym2->getAttributes().contains("constructor"))
      && (ssp = idf_configuration.lookup(
      	      	  "Constructor symbols differ in attribute ().")) != NULL) {
    if (ssp->contains(attribute)) {
      return true;
    }
  }
  return false;
}

//============================================================================
// Report symbol mismatches and differences from one list of symbols.
//============================================================================
static void report_symbols(const idfModel &model,
      	                   const idfModel &model2,
      	      	      	   boolean lack_of_match, boolean quality_of_match)
{
  const idfSymbolDictionary &symbols = model.getDictionary();
  // Show symbols.
  for (idfIterator iter = symbols.first(); !symbols.done(iter); symbols.next(iter)) {
    idfSymbol *sym = symbols.lookup(iter);
    if (sym->getDiscrepancyReported()) {
      continue;
    }
    genString sym_str;
    symbol_id(*sym, model.getFilename(), sym_str);
    if (sym->getMatch() == NULL) {
      if (lack_of_match) {
      	if (ignore_symbol(*sym, model, false)) {
      	  // A constant, compiler-generated, or such.
      	}
      	else{
	  genString s;
	  s.printf("%s has no corresponding node in other file.", sym_str.str());
	  idf_report(s.str(), difference_note);
	  sym->setDiscrepancyReported(true);
      	}
      }
    }
    else {
      if (quality_of_match) {
	idfSymbol &match = *(sym->getMatch());
	genString m_str;
	symbol_id(match, model2.getFilename(), m_str);

        if (match.getMatch() != sym) {
          genString s;
          if (match.getMatch() != NULL) {
            idfSymbol &match_match = *(match.getMatch());
            genString m_match_str;
            symbol_id(match_match, model.getFilename(), m_match_str);
            s.printf("%s matches %s, which prefers %s.", sym_str.str(), 
                                            m_str.str(), m_match_str.str());
          }
          else { // match.getMatch() == NULL
            s.printf("%s matches %s, which has no match!", sym_str.str(), 
                                                           m_str.str());
          }
          idf_report(s.str(), difference_note);
          sym->setDiscrepancyReported(true);
        }

	genString reason;

	boolean ignore_val = idf_configuration.contains(
	                          "SYM source locations are significant"
                                  " only in their presence or absence.");
	if (!equal_locations(sym->getLocation(), match.getLocation(),
                             model, model2, ignore_val, false, &reason)) {
      	  boolean ignore = false;
	  if ((sym->getLocation() == NULL) == (match.getLocation() == NULL)) {
      	    if (ignore_val) {
	      ignore = ignore_val;
      	    }
	  }
      	  else {
            if (idf_configuration.contains("SYM source locations may be absent.")) {
              ignore = true;
            }
      	    else if (idf_configuration.contains("SYM source locations outside of the interesting file may be absent.")) {
      	      const locationStruct *loc = (sym->getLocation() != NULL ? sym : &match)->getLocation();
      	      if (   idf_interesting_filename != NULL
                  && !idf_filename_match(idf_interesting_filename, loc->filename)) {
      	      	ignore = true;
      	      }
      	    }
      	  }
      	  if (!ignore) {
	    genString s;
	    s.printf("%s and %s differ in location (%s).", sym_str.str(), m_str.str(), reason.str());
	    idf_report(s.str(), difference_note);
	    sym->setDiscrepancyReported(true);
	    match.setDiscrepancyReported(true);
      	  }
	}
	if (sym->isGenerated() != match.isGenerated()) {
      	  idfStringSet *ssp = idf_configuration.lookup(
              "Symbols with () attribute differ in whether they are compiler-generated.");
      	  boolean ignore = false;
      	  if (ssp != NULL) {
      	    for (idfIterator iter = ssp->first(); !ssp->done(iter); ssp->next(iter)) {
      	      const char *attr = ssp->index(iter);
      	      if (   sym->getAttributes().contains(attr)
                  || match.getAttributes().contains(attr)) {
      	      	ignore = true;
      	      }
      	    }
      	  }
      	  if (!ignore) {
	    genString s;
	    s.printf("%s%s and %s%s differ in whether they are compiler-generated.",
		      sym_str.str(), sym->isGenerated() ? ":cg" : "",
		      m_str.str(), match.isGenerated() ? ":cg" : "");
	    idf_report(s.str(), difference_note);
	    sym->setDiscrepancyReported(true);
	    match.setDiscrepancyReported(true);
      	  }
	}
      	report_symbol_name(*sym, match, model, model2);
	if (!idf_strmatch(sym->getKind(), match.getKind())) {
          idfStringSet *ssp = idf_configuration.lookup("SYM kinds differ().");
          boolean ignore = false;
          const char *nk = sym->getKind();
          const char *mk = match.getKind();
          if (ssp != NULL && nk != NULL && mk != NULL) {
            genString s;
            s.printf("%s<%s", nk, mk);
            genString sr;
            sr.printf("%s<%s", mk, nk);
            ignore = ssp->lookup(s) || ssp->lookup(sr);
          }
          if (!ignore) {
	    genString s;
	    s.printf("%s and %s differ in kind (%s vs. %s).", sym_str.str(), m_str.str(),
      	      	   nonnull(sym->getKind(), "<no kind>"), nonnull(match.getKind(), "<no kind>"));
	    idf_report(s.str(), difference_note);
	    sym->setDiscrepancyReported(true);
	    match.setDiscrepancyReported(true);
          }
	}
      	symbol_pair syms;
      	syms.sym1 = sym;
      	syms.sym2 = &match;
      	syms.model1 = &model;
      	syms.model2 = &model2;
	if (report_attributes(sym->getAttributes(), match.getAttributes(),
			      sym_str.str(), m_str.str(),
      	      	      	      &symbol_attribute_filter, &syms)) {
	  sym->setDiscrepancyReported(true);
	  match.setDiscrepancyReported(true);
	}
	if (report_relations(sym->getRelations(), match.getRelations(),
      	      	      	     *sym, match,
			     sym_str.str(), m_str.str(),
			     model, model2)) {
	  sym->setDiscrepancyReported(true);
	  match.setDiscrepancyReported(true);
	}
      }
    }
  }
}

//============================================================================
// Report symbol mismatches and differences.
//============================================================================
static void report_symbols(const idfModel &model1, const idfModel &model2)
{
  report_symbols(model1, model2, true, false);
  report_symbols(model2, model1, true, false);
  report_symbols(model1, model2, false, true);
  report_symbols(model2, model1, false, true);
}

//============================================================================
// Reporting Differences in ASTs
//============================================================================

//============================================================================
static void AST_report(idfAst &node, const char *message, const char *prefix)
{
  if (message != NULL) {
    // Fill lines to column output_width unless that would be too skinny.
    // Allow a minumum width.
    int msg_indent = 5 + strlen(prefix);
    const int min = (output_width - 10) / 2;
    int width = output_width - msg_indent;
    if (width < min) {
      width = min;
    }
    int line_size;
    boolean first = true;
    while (find_line(width, message, line_size)) {
      ostrm->write(message, line_size) << endl;
      message += line_size;
      skip_whitespace(message);
      first = false;
    }
#if 0
    genString s;
    s.printf(" %s****%s", prefix, message);
    (*ostrm) << s.str() << endl;
#endif

    node.setDiscrepancyReported(true);
    idfAst *match = node.getMatch();
    if (match != NULL) {
      match->setDiscrepancyReported(true);
    }
  }
}

//============================================================================
static void note_discrepancy(idfAst &node, const char *, const char *)
{
  idfAst *p = &node;
  do {
    p->setContainsDiscrepancy(true);
    p = p->getParent();
  } while (p != NULL);
}

//============================================================================
static boolean is_mismatched(idfAst &node)
{
  idfAst *match = node.getMatch();
  if (match != NULL) {
    return false;
  }
  idfStringSet *ssp;
  if ((ssp = idf_configuration.lookup("AST () nodes are absent.")) != NULL) {
    const char *kind = node.getKind();
    idfAst *parent = node.getParent();
    genString contextual_kind = "";
    if (parent != NULL && parent->getKind() != NULL) {
      contextual_kind.printf("%s in %s", kind, parent->getKind());
    }
    if (kind != NULL && (ssp->contains(kind) || ssp->contains(contextual_kind))) {
      // Don't report this kind of missing match.
      return false;
    }
  }
  if ((ssp = idf_configuration.lookup("AST nodes for symbols named () are absent.")) != NULL) {
    const unsigned int id = node.getIdentifier();
    const idfModel &model = node.getModel();
    if (id > 0) {
      idfSymbol *sym = model.getDictionary().lookup(id);
      if (sym != NULL && ssp->contains(sym->getName())) {
        // Don't report missing matches for symbols with this name.
        return false;
      }
    }
  }
  if (idf_configuration.contains("AST records outside of the interesting file are absent.")) {
    boolean found_interesting_location = false;

    // Look for a location in this node or one of its ancestors.
    locationList *locs = NULL;
    for (idfAst *n = &node; n != NULL; n = n->getParent()) {
      locs = n->getLocation();
      if (locs != NULL && locs->size() > 0) {
      	break;
      }
    }

    // It's interesting if it matches the interesting file, or if there is no particularly
    // interesting file and the location specifies a file at all.
    if (locs != NULL) {
      for (int i = 0; i < locs->size(); i += 1) {
      	if (idf_interesting_filename == 0 || idf_filename_match(idf_interesting_filename, (*locs)[i]->filename)) {
      	  found_interesting_location = true;
	}
      }
    }

    if (!found_interesting_location) {
      return false;
    }
  }
  return true;
}

//============================================================================
static boolean SS_valmatch(idfAst &node, idfAst &match)
{
  if (node.getValue() == NULL || match.getValue() == NULL) {
    idfStringSet *ssp = idf_configuration.lookup(
                                "AST nodes () differ in presence of value.");
    const char *nk = node.getKind();
    const char *mk = match.getKind();
    if (   ssp != NULL && nk != NULL && mk != NULL 
        && (ssp->lookup(nk) || ssp->lookup(mk))) {
      return true;
    }
  }
  return idf_strmatch(node.getValue(), match.getValue());
}

//============================================================================
static void report_AST_node(idfAst &node, char *prefix,
      	                    const idfModel &model1, const idfModel &model2,
      	      	      	    void (*handler)(idfAst &, const char *msg, const char *pre))
{
  idfAst *match_p = node.getMatch();
  if (is_mismatched(node)) {
    handler(node, NULL, prefix);
    return;
  }
  if (match_p == NULL) {
    return;
  }
  idfAst &match = *match_p;
#if 0
  if (node.getDiscrepancyReported() || match.getDiscrepancyReported()) {
    return;
  }
#else
  if (node.getDiscrepancyReported()) {
    return;
  }
#endif

  // For "statements", EDG puts out only start.
  idfStringSet *pres_stset = idf_configuration.lookup(
    "AST node () source locations differ in presence of length specification.");
  boolean ig_prs_len =    (pres_stset != NULL)
                       && (   pres_stset->contains(node.getKind())
      	      	      	   || pres_stset->contains(match.getKind()));
  idfStringSet *len_stset = idf_configuration.lookup(
    "AST node () source locations differ in length specification.");
  boolean ig_val_len =    (len_stset != NULL)
                       && (   len_stset->contains(node.getKind())
      	      	      	   || len_stset->contains(match.getKind()));
  genString reason;
  if (!equal_location_lists(node.getLocation(), match.getLocation(),
                            model1, model2, ig_prs_len, ig_val_len, reason)) {
    boolean loc_is_absent = (node.getLocation() == NULL || match.getLocation() == NULL);
    idfStringSet *ssp;
    genString paren = "parentheses";
    genString failtype = "";
    failtype.printf("%s : %s", node.getKind(), reason.str());
    genString mfailtype = "";
    mfailtype.printf("%s : %s", match.getKind(), reason.str());
    if (   idf_configuration.contains("AST records differ in presence of source location.")
	&& loc_is_absent) {
      // Don't check for or report a difference.
    }
    else if (   !loc_is_absent 
             && (ssp = idf_configuration.lookup("AST nodes () differs by 1 or 2.")) != NULL
             && (   ssp->contains(failtype)
                 || ssp->contains(mfailtype)
                )
             && SS_match_location_lists(node.getLocation(), match.getLocation(), 
                                        model1, model2, reason)
            ) {
      // Locations are within our tolerances, so don't check for or report a difference.
    }
    else if (   !loc_is_absent
             && (ssp = idf_configuration.lookup(
                       "AST nodes () differ in enclosing parentheses.")) != NULL
             && reason.compare("start_column") == 0
             && SS_match_location_lists(node.getLocation(), match.getLocation(),
                                        model1, model2, paren)
            ) {
      // Locations differ only by one on each end, so don't report a difference.
    }
    else {
      ssp = idf_configuration.lookup("AST nodes () differ in presence of source location.");
      boolean ignore =    (ssp != NULL)
                       && loc_is_absent
                       && (   ssp->contains(node.getKind())
                           || ssp->contains(match.getKind()));
      if (!ignore) {
	genString match_loc;
	idf_location_list_str(match.getLocation(), match_loc);
	if (match_loc.length() <= 0) {
	  match_loc = "<no location>";
	}
	genString str;
	str.printf("Location differs from %s in %s.", match_loc.str(), reason.str());
	handler(node, str, prefix);
      }
    }
  }
  if (!idf_strmatch(node.getKind(), match.getKind())) {
    idfStringSet *ssp = idf_configuration.lookup("AST kinds differ().");
    boolean ignore = false;
    const char *nk = node.getKind();
    const char *mk = match.getKind();
    if (ssp != NULL && nk != NULL && mk != NULL) {
      genString s;
      s.printf("%s<%s", nk, mk);
      genString sr;
      sr.printf("%s<%s", mk, nk);
      ignore = ssp->lookup(s) || ssp->lookup(sr);
    }
    if (!ignore) {
      genString str;
      str.printf("Kind \"%s\" differs from \"%s\" of corresponding node.",
        nonnull(node.getKind(), "<no kind>"), nonnull(match.getKind(), "<no kind>"));
      handler(node, str, prefix);
    }
  }
  
  int nid = node.getIdentifier();
  int mid = match.getIdentifier();
  idfSymbol *sym1 = model1.getDictionary().lookup(nid);
  idfSymbol *sym2 = model2.getDictionary().lookup(mid);
  if ((sym1 == NULL || sym2 == NULL) ? sym1 != sym2
                    : (sym1->getMatch() != sym2 && sym2->getMatch() != sym1)) {
    idfSymbol *present_sym = sym1 != NULL ? sym1 : sym2;
    boolean report_missing = true;
    if (ignore_symbol(*present_sym, present_sym == sym1 ? model1 : model2, true)) {
      report_missing = false;
    }
    if (idf_strmatch(present_sym->getKind(), "type")
        && idf_configuration.contains(
                 "AST nodes differ in presence of type symbol.")) {
      report_missing = false;
    }
    idfStringSet *ssp;
    if ((ssp = idf_configuration.lookup(
                  "AST nodes () differ in presence of symbol.")) != NULL) {
      const char *kind1 = node.getKind();
      const char *kind2 = match.getKind();
      if (   (kind1 != NULL && ssp->contains(kind1))
          || (kind2 != NULL && ssp->contains(kind2))) {
      	report_missing = false;
      }
    }
    if (sym1 == NULL) {
      if (report_missing) {
      	genString str;
      	str.printf("Symbol missing from this node, present (as [%d]) in corresponding node.",
      	      	   match.getIdentifier());
        handler(node, str, prefix);
      }
    }
    else if (sym2 == NULL) {
      if (report_missing) {
        handler(node, "Symbol present in this node, missing from corresponding node.", prefix);
      }
    }
    else {
      genString str;
      str.printf("This node and the corresponding AST node both have symbols"
                 " ([%d] and [%d] respectively) but the two symbols do not correspond"
                 " with one another.",
      	         node.getIdentifier(), match.getIdentifier());
      handler(node, str, prefix);
    }
  }

  if (!SS_valmatch(node, match)) {
    genString str;
    str.printf("Value %s of this node differs from value %s of corresponding node.",
      	       nonnull(node.getValue(), "<no value>"), nonnull(match.getValue(), "<no value>"));
    handler(node, str, prefix);
  }
  if (node.isGenerated() != match.isGenerated()) {
    if (node.isGenerated()) {
      handler(node, "This node is generated, but its corresponding node is not.", prefix);
    }
    else {
      handler(node, "This node is not generated, but its corresponding node is.", prefix);
    }
  }
}

//============================================================================
static boolean check_AST_siblings(const idfAstSiblings &, const idfModel &, const idfModel &);

//============================================================================
static void check_AST(idfAst &node,
      	              const idfModel &model1, const idfModel &model2)
{
    node.setContainsDiscrepancy(false);
    report_AST_node(node, "", model1, model2, note_discrepancy);
    check_AST_siblings(node.getChildren(), model1, model2);
}

//============================================================================
static boolean check_AST_siblings(const idfAstSiblings &children,
      	                        const idfModel &model,
                                const idfModel &model2)
{
  boolean found_discrepancy = false;
  for (idfAst *ap = children.getSibling(0); ap != NULL; ap = ap->getNextSib()) {
      check_AST(*ap, model, model2);
      if (ap->getContainsDiscrepancy()) {
      	found_discrepancy = true;
      }
  }
  return found_discrepancy;
}

//============================================================================
static void report_AST(idfAst *node, char *,
      	               const idfModel &, const idfModel &);

//============================================================================
static void report_AST_siblings(const idfAstSiblings &children, char *prefix,
      	                        const idfModel &model,
                                const idfModel &model2)
{
  boolean last_was_ellipsis = false;
  for (idfAst *ap = children.getSibling(0); ap != NULL; ap = ap->getNextSib()) {
    if (ap->getContainsDiscrepancy()) {
      report_AST(ap, prefix, model, model2);
      last_was_ellipsis = false;
    }
    else if (!last_was_ellipsis) {
      genString s;
      s.printf(" %s...", prefix);
      (*ostrm) << s.str() << endl;
      last_was_ellipsis = true;
    }
  }
}

//============================================================================
// Dump the ASTs, indicating mismatches and differences.
// Outputs the AST given by 'node' as a sequence of one or more lines.
// Each line is indented using the 'prefix' string.
//============================================================================
static void report_AST(idfAst *node, char *prefix,
      	               const idfModel &model1, const idfModel &model2)
{
    idfAst *match = node->getMatch();
    boolean mismatch = is_mismatched(*node);
    if (mismatch) {
	report_counts[difference_note] += 1;
    }
    (*ostrm) << (!mismatch ? " " : "X");
    if (prefix != NULL) {
	(*ostrm) << prefix;
    }
    const char *kind = node->getKind();
    if (kind == NULL) {
      kind = "<kind missing>";
    }
    (*ostrm) << kind;

    if (node->getIdentifier() > 0) {
      (*ostrm) << " [" << node->getIdentifier() << "]";
    }

    genString loc;
    idf_location_list_str(node->getLocation(), loc);
    if (loc.length() > 0) {
      (*ostrm) << ' ' << loc.str();
    }

    if (node->getChildren().getNumber() == 0) {
      const char *value = node->getValue();
      if (strcmp(kind, "constant") == 0 || value != NULL) {
	if (value == NULL) {
	  value = "<value missing>";
	}
	(*ostrm) << " {\"" << value << "\"}" << endl;
      }
      else {
	(*ostrm) << " {}" << endl;
      }
      report_AST_node(*node, prefix, model1, model2, AST_report);
    }
    else {
      (*ostrm) << " {";
      (*ostrm) << endl;
      report_AST_node(*node, prefix, model1, model2, AST_report);

      // Output child nodes, with revised indentation prefix.
      genString pref = prefix;
      pref += "  ";
      report_AST_siblings(node->getChildren(), pref, model1, model2);
      (*ostrm) << ' ';
      if (prefix != NULL) {
      	(*ostrm) << prefix;
      }
      (*ostrm) << "}" << endl;
    }
}

//============================================================================
// Dump the AST of model1, indicating mismatches and differences.
// Outputs the AST as a sequence of one or more lines.
//============================================================================
static void report_AST(const idfModel &model1, const idfModel &model2)
{
  // Examine the ASTs to identify where the discrepancies are.
  idfAstDictionary *asts = model1.getAst();
  for (idfIterator iter = asts->first(); !asts->done(iter); asts->next(iter)) {
    idfAstSiblings *children = asts->lookup(iter);
    if (children != NULL && check_AST_siblings(*children, model1, model2)) {

      // Dump the ASTs, indicating mismatches and differences.
      idf_report_spacer();
      (*ostrm) << model1.getFilename() << ":" << endl << " AST " << " {" << endl;
      report_AST_siblings(*children, "  ", model1, model2);
      (*ostrm) << " }" << endl;
    }
  }
}

//============================================================================
void idf_report_differences(const idfModel &model1, const idfModel &model2)
{
  if (ostrm == NULL) {
    set_report_stream(cout);
  }

#if 0
  // For now, just dump something to show that it's there.
  (*ostrm) << "Here's what we've got for file " << model1.getFilename() << "." << endl << endl;
  report_there(model1);
  (*ostrm) << endl << endl;

  (*ostrm) << "Here's what we've got for file " << model2.getFilename() << "." << endl << endl;
  report_there(model2);
#endif

  // Report file name differences.
  report_files(model1, model2);

  // Report SMT node mismatches and differences.
  report_SMT(model1, model2);

  // Report symbol mismatches and differences.
  report_symbols(model1, model2);

  // Dump the ASTs, indicating mismatches and differences.
  if (!idf_configuration.contains("Ignore discrepancies in AST.")) {
    report_AST(model1, model2);
    report_AST(model2, model1);
  }
}
