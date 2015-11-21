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
#include <astnodeStruct.h>
#include <locationStruct.h>
#include <relationStruct.h>
#include <symbolStruct.h>
#include <astnodeList.h>
#include <attributeList.h>
#include <locationList.h>
#include <maplineList.h>
#include <stringList.h>
#include <transformationList.h>
#include <uintList.h>
#include <elsMapper.h>
#include <idfSymbol.h>
#include <idfDict.h>
#include <idfSmt.h>
#include <idfMain.h>
#include <idfAst.h>
#include <idfReport.h>
#include <genString.h>
#include <if_parser_cbs.h>

//============================================================================
static idfSymbol &get_symbol(int i) {
  idfSymbol *sym = idf_current_symbols->lookup(i);
  if (sym == NULL) {
    sym = new idfSymbol(i, *idf_current_model);
    idf_current_symbols->add(i, sym);
    
#if defined(REPORT_REF_BEFORE_DEF)
    genString msg;
    const char *fn = idf_current_filename;
    if (fn == NULL) {
      fn == "<unknown file>";
    }
    msg.printf("%s: Reference to symbol [%d] before defined.", fn, i);
    idf_report(msg, source_error);
#endif
  }
  return *sym;
}

//============================================================================
static idfRelationshipDictionary &get_relationships(idfRelationDictionary &r, const char *name) {
  idfRelationshipDictionary *rd = r.lookup(name);
  if (rd == NULL) {
    rd = new idfRelationshipDictionary;
    r.add(name, rd);
  }
  return *rd;
}

//============================================================================
extern "C" int sym_insert_CB(unsigned int i, symbolStruct *s)
{
#if 0
  cout << "Defining symbol [" << i << "] with name " << s->name
       << " and generated property " << s->generated << "." << endl;
#endif
  idfSymbol *sym = idf_current_symbols->lookup(i);
  if (sym != 0) {
    if (!sym->set(s->name, s->kind, s->loc, s->generated, true)) {
      genString msg;
      msg.printf("Duplicate definition of symbol [%d].", i);
      idf_report(msg, internal_error);
    }
  }
  else {
    sym = new idfSymbol(i, s->name, s->kind, s->loc, s->generated, true,
      	      	      	*idf_current_model);
    if (!idf_current_symbols->add(i, sym)) {
      genString msg;
      msg.printf("Error while processing definition of symbol [%d].", i);
      idf_report(msg, internal_error);
    }
  }
#if 0
  cout << "Defined symbol [" << i << "] with name " << sym->getName()
       << " and generated property " << sym->isGenerated() << "." << endl;
#endif
  return 1;
}

//============================================================================
// Add attributes to the given attribute dictionary.
// Addition of an already-present attribute is an error, reported
// with the context given in msg_context.
//============================================================================
static void add_attributes(idfAttributeDictionary &ad,
                           attributeList *attributes,
                           const char *msg_context,
      	      	      	   const char *relation)
{
  if (attributes) {
    idfStringSet *ssp = relation != NULL
                          ? idf_configuration.lookup("Attribute of relation () can have multiple values.")
                          : 0;
    for (int i = 0; i < attributes->size(); i++) {
      const char *n = (*attributes)[i]->name;
      if (n == NULL) {
        n = "";
      }
      const char *v = (*attributes)[i]->value;
      if (v == NULL) {
        v = "";
      }

      // Provide for attributes such as the argument attribute of type, which
      // can have several values for the same pair of symbols.  Treat these
      // as independent attributes.  For example, if 
      //       [1] is type of [2] with argument value 1   and
      //       [1] is type of [2] with argument value 2
      // then it's entered into the attribute dictionary
      //    not as "argument"->"1" and "argument"->"2"
      //    but as "argument(1)->"" and "argument(2)"->""
      genString s;
      genString alt_name;
      if (ssp != NULL && (s.printf("%s of %s", n, relation), ssp->contains(s))) {
      	alt_name.printf("%s(%s)", n, v);
      	n = alt_name.str();
      	v = "";
      }
      	
      boolean already_there = !ad.add(n, strdup(v));
      if (already_there) {
      	const char *old_val = ad.lookup(n);
        genString msg;
      	if (idf_strmatch(old_val, v)) {
      	  if (!idf_configuration.contains("Duplicate attributes are redundant.")) {
            msg.printf("%s: duplicate attribute %s, value %s", msg_context, n, v);
            idf_report(msg, source_error);
      	  }
      	}
      	else {
      	  msg.printf("%s: conflicting values for attribute %s, %s vs. %s",
	    msg_context, n, old_val != NULL ? old_val : "<NULL>", v);
          idf_report(msg, source_error);
      	}
      }
    }
  }
}

extern "C" int atr_modify_CB(unsigned int i, attributeList *l)
{
  idfSymbol &sym = get_symbol(i);
  idfAttributeDictionary &ad = sym.getAttributes();
  genString context;
  context.printf("ATR [%d] %s", i, "<attribute list>");
  add_attributes(ad, l, context.str(), 0);
  return 1;
}

//============================================================================
// Add relation entries for the given relation name, list of source symbols,
// list of target symbols, and attribute list.
//============================================================================
static void rel_create(const char *name, uintList *source,
      	               uintList *target, attributeList *attributes)
{
  for (int i = 0; i < source->size(); i++) {
    unsigned int src = (*source)[i];
    idfSymbol &sym = get_symbol(src);
    idfRelationDictionary &r = sym.getRelations();
    idfRelationshipDictionary &rel = get_relationships(r, name);
    for (int j = 0; j < target->size(); j++) {
      unsigned int tgt = (*target)[j];
      idfAttributeDictionary *ad = NULL;
      if (attributes != NULL) {
      	ad = rel.lookup(tgt);
      	boolean new_ad = (ad == NULL);
      	if (new_ad) {
      	  ad = new idfAttributeDictionary;
      	  new_ad = true;
      	}
      	genString context;
      	context.printf("REL [%d] %s [%d]", src, name, tgt);
      	add_attributes(*ad, attributes, context.str(), name);
      	if (new_ad) {
	  boolean already_there = !rel.add(tgt, ad);
	  if (already_there) {
      	    // The relation was already there, but with no attributes.
      	    rel.reassign(tgt, ad);
	  }
      	}
      }
      else {
	boolean already_there = !rel.add(tgt, ad);
	if (already_there) {
      	  if (!idf_configuration.contains("Duplicate relation records are redundant.")) {
	    genString msg;
	    msg.printf("REL [%d] %s [%d] is already present", src, name, tgt);
	    idf_report(msg, source_error);
      	  }
	}
      }
    }
  }
}

extern "C" int rel_create_CB(relationStruct *s)
{
  const char *n = s->name;
  if (n == NULL) {
    n = "";
  }
  if (s->source != NULL && s->target != NULL) {
    rel_create(n, s->source, s->target, s->attributes);
    // Add inverse relations, using a the relation name with "~" prepended.
    char *inverse_n = new char[strlen(n) + 2];
    inverse_n[0] = '~';
    strcpy(inverse_n + 1, n);
    rel_create(inverse_n, s->target, s->source, s->attributes);
    delete[] inverse_n;
  }

  return 1;
}

extern "C" int smt_reference_CB(unsigned int i, locationList *l)
{
  idfSmt *smt = new idfSmt(i, false, l, true, *idf_current_model);
  smt->setPrev(idf_prev_smt);
  if (idf_prev_smt != NULL) {
    idf_prev_smt->setNext(smt);
  }
  idf_prev_smt = smt;

  return 1;
}

extern "C" int smt_implicit_CB(unsigned int i, locationList *l)
{
  idfSmt *smt = new idfSmt(i, true, l, true, *idf_current_model);
  smt->setPrev(idf_prev_smt);
  if (idf_prev_smt != NULL) {
    idf_prev_smt->setNext(smt);
  }
  idf_prev_smt = smt;

  return 1;
}

extern "C" int smt_keyword_CB(char *s, locationList *l)
{
  idfSmt *smt = new idfSmt(s, l, true, *idf_current_model);
  smt->setPrev(idf_prev_smt);
  if (idf_prev_smt != NULL) {
    idf_prev_smt->setNext(smt);
  }
  idf_prev_smt = smt;

  return 1;
}

extern "C" int cpp_include_order_CB(stringList *l)
{
  return 1;
}

extern "C" int err_message_CB(char *s, locationStruct *l)
{
  return 1;
}

extern "C" int transform_copy_CB(transformationList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->size();
      for ( int i = 0; i < n && retval > 0; i++ )
	if ( elsMapper::addTransformation((*l)[i], 1) < 0 )
	  retval = -1;
    }
  else
    retval = -1;

  return retval;
}

extern "C" int transform_fixed_CB(transformationList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->size();
      for ( int i = 0; i < n && retval > 0; i++ )
          if ( elsMapper::addTransformation((*l)[i], 0) < 0 )
              retval = -1;
    }
  else
    retval = -1;

  return retval;
}

extern "C" int transform_lines_CB(maplineList *l)
{
  int retval = 1;
  
  if ( l )
    {
      int n = l->size();
      for ( int i = 0; i < n && retval > 0; i++ )
	if ( elsMapper::addTransformation((*l)[i]) < 0 )
	  retval = -1;
    }
  else
    retval = -1;

  return retval;
}

extern "C" int ast_report_tree_CB(astnodeList *a)
{
#if 0
  if (idf_current_ast_roots != NULL) {
    if (!idf_configuration.contains(
          "AST records, other than the final ASTs, do not match.")) {
      const char *fn = idf_current_filename;
      if (fn == NULL) {
	fn == "<unknown file>";
      }
      genString s;
      s.printf("%s: Multiple AST records. Ignoring all but the last.", fn);
      idf_report(s, internal_error);
    }

    delete idf_current_ast_roots;
    idf_current_ast_roots = NULL;
  }
  // List has an effective root.
  idf_current_ast_roots = new idfAstSiblings(a, NULL, *idf_current_model);
#endif

  int n = idf_current_ast_roots->count();
  idf_current_ast_roots->add(n, new idfAstSiblings(a, NULL, *idf_current_model));

  return 1;
}

extern "C" int smt_file_CB(char *)
{
  return 1;
}

extern "C" int iff_parser_preparse_CB(void)
{
  return 1;
}

extern "C" int iff_parser_postparse_CB(void)
{
  return 1;
}

