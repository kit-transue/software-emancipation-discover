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
/*
 * Entity.h.C
 *
 * Definitions for high-level Entity object
 *
 */
#include <cLibraryFunctions.h>
#include <Entity.h> 
#include <autosubsys.h>
#include <proj.h>

#include <autosubsys-weights.h>

#include <linkType_selector.h>
#include <XrefTable.h>

/* Class statics */

objSet Entity::projects_to_search;
allentity* Entity::allentities;

static Entity no_class;	// used as dummy target for basetype and parent_class
			// if is a builtin type, not a member of the projects
			// being analyzed, or whatever

/* simple constructor */

Entity::Entity():
	isundead(false),
	preve(NULL),
	nexte(NULL),
	parent_class(NULL),
	basetype(NULL)
{ }

/* copy constructor */

Entity::Entity(const Entity& o):
	Obj(o),
	isundead(o.isundead),
	preve(o.preve),
	nexte(o.nexte),
	parent_class(o.parent_class),
	basetype(o.basetype)
{ }

//------------------------------------------
// Debugging print() function
//------------------------------------------
  
void  Entity::print(ostream& ostr, int) const
{
   ostr << get_name();
}

//
// Add Entity to allentities' xref mapping; alreadyused refers to whether the
// entity should consider itself already in a subsystem or not.
// add() and add1() are long gone.

int Entity::add2(const symbolPtr& sym, int) {
  Initialize(Entity::add2);
  allentities->add(this, sym);
  my_sym = sym;
  return 1;
}

//
// Given a particular relation and a symbolPtr, we find the symbol's
// entity and then create the reference.  (For historical reasons, the
// rels are "backwards" -- e.g., if entity A calls entity B, this
// routine will be called with A as "this", B as "sym", and
// AUS_FCALL_INV as the rel; the proper way to read this is
// "sym rel this", or "B is called by this".  This made more sense
// with the "relation" implementation; now that we use refs_and_weights,
// we use the inverse rel with "this" and the passed rel with "sym.")

void Entity::relate_to_symbol(size_t rel, const symbolPtr& sym) {
  Initialize(Entity::relate_to_symbol);

  Entity *ffar = allentities->item(sym);
  if (ffar && this!=ffar) {
     size_t inv_rel;
     bool outgoing = false;
     if (rel >= NWEIGHTS / 2) {
	inv_rel = rel - NWEIGHTS / 2;
	outgoing = true;
     }
     else inv_rel = rel + NWEIGHTS / 2;
     rw.add_ref(ffar->index(), inv_rel);
     ffar->rw.add_ref(index(), rel);
     if (outgoing) {
	// Take care of model aliases -- if this calls foo(), for instance,
	// and there is more than one foo() in the model, we can't tell which
	// foo() is involved, so we'd better add relations with all of them.
	// (Aliased Entities were linked together in decomposer::additems().)
	EntityPtr e;
	for (e = ffar->get_preve(); e; e = e->get_preve()) {
	   rw.add_ref(e->index(), inv_rel);
	   e->rw.add_ref(index(), rel);
	}
	for (e = ffar->get_nexte(); e; e = e->get_nexte()) {
	   rw.add_ref(e->index(), inv_rel);
	   e->rw.add_ref(index(), rel);
	}
	if (rel == AUS_FCALL_INV || rel == AUS_DATAREF_INV) {
	   // apply usage of member so that class is used, too
	   if (!ffar->parent_class && strstr(ffar->my_sym.get_name(), "::")) {
	      symbolPtr parent_sym = ffar->my_sym.get_parent_of(projects_to_search);
	      if (parent_sym.xrisnotnull()) {
		 ffar->parent_class = allentities->item(parent_sym);
	      }
	   }
	   if (ffar->parent_class) {
	      rw.add_ref(ffar->parent_class->index(), AUS_DATAREF);
	      ffar->parent_class->rw.add_ref(index(), AUS_DATAREF_INV);
	   }
	   if (ffar->my_sym.get_kind() == DD_TYPEDEF) {
	      if (!ffar->basetype) {
		 ffar->set_basetype();
	      }
	      if (ffar->basetype != &no_class) {
		 rw.add_ref(ffar->basetype->index(), inv_rel);
		 ffar->basetype->rw.add_ref(index(), rel);
	      }
	   }
	}
     }
  }
}

//=============================================
// collect_classes_from_type: takes the type ("has_type" link) of the
// supplied symbol and filters the result for a class or enum that is
// used in the type.  It handles indirecting through typedefs.  If the
// result array is non-empty, it returns "true."
//=============================================

bool collect_classes_from_type(const symbolPtr& type, symbolArr& classes,
			       const objSet& projects_to_search) {
   Initialize(collect_classes_from_type);

   symbolArr targs;
   if (type->get_link(is_using, targs, projects_to_search, type.get_xref()->get_lxref())) {
      symbolPtr targ;
      ForEachS(targ, targs) {
	 ddKind k = targ.get_kind();
	 if (k == DD_CLASS || k == DD_ENUM) {
	    classes.insert_last(targ);
	 }
	 else if (k == DD_TYPEDEF) {
	    symbolArr targs2;
	    if (targ.get_link(have_arg_type, targs2,
			       projects_to_search)) {
	       symbolPtr targ2;
	       ForEachS(targ2, targs2) {
		  ddKind k2 = targ2.get_kind();
		  if (k2 == DD_CLASS || k2 == DD_ENUM) {
		     classes.insert_last(targ2);
		  }
	       }
	    }
	 }
      }
   }
   return (classes.size() > 0);
}

// relate_to_type caches the result of collect_classes_from_type, which is
// horrendously expensive, in the allentities xref mapper, as well as
// establishing the relationship between this entity and the class used in
// its type/return type.

symbolPtr Entity::relate_to_type(const symbolPtr& type, size_t rel) {
   Initialize(Entity::relate_to_type);

   Entity* type_class = allentities->item(type);
   if (!type_class) {
      symbolArr classes;
      if (collect_classes_from_type(type, classes, projects_to_search)) {
	 symbolPtr cl;
	 ForEachS(cl, classes) {
	    if (type_class = allentities->item(cl)) {
	       break;
	    }
	 }
      }
      if (!type_class) {
	 type_class = &no_class;
      }
      allentities->add_sym_alias(type_class, type);
   }
   if (type_class != &no_class) {
      relate_to_symbol(rel, type_class->my_sym);
      return type_class->my_sym;
   }
   return NULL_symbolPtr;
}


//
// newrelate sets up the relations between entities. All of them. It replaces
// an enormous hodge-podge of ad-hockery which ultimately collapsed under its
// own weight (and the lack of support underneath from a changed data 
// dictionary...)

int Entity::newrelate() {
   Initialize(Entity::newrelate);

   symbolPtr used;
   symbolPtr user;
   symbolPtr type;
   symbolArr targs;
   symbolPtr targ;
   linkType_selector sel;
   int i;

   if (my_sym.xrisnotnull()) {
      switch(my_sym.get_kind()) {

      case DD_FUNC_DECL: {
	 symbolPtr class_of_func = NULL_symbolPtr;
	 size_t qual_len = 0;
	 sel.add_link_type(is_using);
	 sel.add_link_type(has_type);
	 sel.add_link_type(have_arg_type);
	 const char* my_name = my_sym.get_name();
	 const char* q = strstr(my_name, "::");
	 if (q) {
	    qual_len = q - my_name + 2;
	 }
	 my_sym.get_links(sel, projects_to_search, false);

// Because "is_using" is very general, subsuming many kinds of relationships,
// we process more-specific link types first and remove entities related in
// those ways from the "useds" array.  This means that certain relationships
// will be ignored -- e.g., if a function takes class A as an argument and
// also uses it in the body of the function, only the former relationship
// will be honored.  It's not perfect, but it's the best we can do with the
// kind of information maintained in the PMOD.

// The first step is to find classes used in the function declaration, i.e.,
// as arguments or the return type.  These are lumped together in the PMOD
// under the "have_arg_type" link.  We can then find the return type using
// the "has_type" link and process its target separately, playing the same
// game as for "useds" with the combined list of types.

	 if (sel[has_type].size()) {
	    targ = relate_to_type(sel[has_type][0], AUS_RETTYPE_INV);
	    if (targ.isnotnull()) {
	       sel[have_arg_type].remove(targ);
	       sel[is_using].remove(targ);
	    }
	 }

	 ForEachS(type, sel[have_arg_type]) {
	    ddKind k = type.get_kind();
	    if (k == DD_CLASS || k == DD_ENUM) {
	       relate_to_symbol(AUS_ARGTYPE_INV, type);
	       sel[is_using].remove(type);
	    }
	 }

// Now go through remaining items used by this func and add appropriate
// relationships.

         ForEachS(used, sel[is_using]) {
	    switch (used.get_kind()) {
	    case DD_FUNC_DECL:
	       relate_to_symbol(AUS_FCALL_INV, used);
	       break;

	    case DD_VAR_DECL:
	    case DD_CLASS:
	    case DD_ENUM:
	    case DD_TYPEDEF:
	       relate_to_symbol(AUS_DATAREF_INV, used);
	       break;

            case DD_MACRO:
	       relate_to_symbol(AUS_FCALL_INV, used);
	       break;

	    case DD_FIELD: {
	       if (!(qual_len && memcmp(my_name, used.get_name(), qual_len) == 0)) {
		  /* not a reference to another member of the same class */
		  targ = used.get_parent_of(projects_to_search);
		  if (targ.xrisnotnull()) {
		     relate_to_symbol(AUS_DATAREF_INV, targ);
		  }
	       }
	       if (used.get_link(has_type, targs, projects_to_search)) {
		  relate_to_type(targs[0], AUS_DATAREF_INV);
	       }
	       break;
	    }

            case DD_ENUM_VAL:
            {
                targ = used.get_parent_of(projects_to_search);
                if (targ.xrisnotnull())
                    relate_to_symbol(AUS_DATAREF_INV, targ);
                break;
            }

	    default:
	       break;
	    }
	 }

	 break;
      }
 
/***************************************************************************
 * Many of the same considerations apply to DD_CLASS symbols; of course,
 * the inverse relationships with DD_FUNC_DECLs are the same (used_by
 * instead of is_using), but there are also is_using, which may indicate
 * use of a class as the type of a member or, possibly, friendship.
 *
 * Finding the functions in whose arguments this class participates is more
 * involved, however; we have to find all the functions using the class and
 * scan through their argument lists to find references to the class.
 */

      case DD_CLASS:
	 sel.add_link_type(is_using);
	 sel.add_link_type(have_friends);
	 sel.add_link_type(has_superclass);
	 my_sym.get_links(sel, projects_to_search, false);

// Handle friend relationships

	 for (i = 0; i < sel[have_friends].size(); i++) {
	    targ = sel[have_friends][i];
	    relate_to_symbol(AUS_FRIEND_INV, targ);
	    sel[is_using].remove(targ);
	 }

// Handle superclass/subclass relationships

	 for (i = 0; i < sel[has_superclass].size(); i++) {
	    targ = sel[has_superclass][i];
	    relate_to_symbol(AUS_SUBCLASS_INV, targ);
	    sel[is_using].remove(targ);
	 }

// Handle remaining cases of useds: members, nested classes, and macros

	 {
	    ForEachS(used, sel[is_using]) {
	       switch (used.get_kind()) {
	       case DD_FUNC_DECL:
	       case DD_VAR_DECL:
		  relate_to_symbol(AUS_MEMBER, used);
		  break;

	       case DD_TYPEDEF:
	       case DD_ENUM:
	       case DD_CLASS: {
		  // check for nested typedef: name must begin with
		  // "my_name::"
		  const char* my_name = my_sym.get_name();
		  size_t my_name_len = strlen(my_name);
		  const char* used_name = used.get_name();
		  size_t used_name_len = strlen(used_name);
		  const char* qual = used_name + my_name_len;
		  if (used_name_len > my_name_len &&
		      qual[0] == ':' && qual[1] == ':' &&
		      memcmp(my_name, used_name, my_name_len) == 0) {
		     relate_to_symbol(AUS_ELEMENT_INV, used);
		  }
		  break;
	       }

               case DD_MACRO:
                  relate_to_symbol(AUS_FCALL_INV, used);
                  break;

	       default:
		  break;
	       }
	    }
	 }
         break;

/***************************************************************************
 * DD_VAR_DECLs only participate in a few relationships:
 * used_by indicates a reference to the variable from a function or a
 * membership relationship in a class.  "is_using" reflects either the
 * type of the variable or an initializer.
 */
      case DD_VAR_DECL: {
	 symbolPtr class_of_var = NULL_symbolPtr;
	 size_t qual_len = 0;
	 sel.add_link_type(is_using);
	 sel.add_link_type(has_type);
	 const char* my_name = my_sym.get_name();
	 const char* q = strstr(my_name, "::");
	 if (q) {
	    qual_len = q - my_name;
	 }
	 my_sym.get_links(sel, projects_to_search, false);
	 if (sel[has_type].size()) {
	    targ = relate_to_type(sel[has_type][0], AUS_INSTANCE_INV);
	    if (targ.isnotnull()) {
	       sel[is_using].remove(targ);
	    }
	 }

	 ForEachS(used, sel[is_using]) {
	    switch(used.get_kind()) {
	    case DD_VAR_DECL:
	    case DD_CLASS:
	    case DD_ENUM:
	    case DD_TYPEDEF:
	       relate_to_symbol(AUS_DATAREF_INV, used);
	       break;

	    case DD_FUNC_DECL:
            case DD_MACRO:
	       relate_to_symbol(AUS_FCALL_INV, used);
	       break;

	    case DD_FIELD:
	       if (used.get_link(has_type, targs, projects_to_search)) {
		  relate_to_type(targs[0], AUS_DATAREF_INV);
	       }
	       /* FALLTHROUGH */
	    case DD_ENUM_VAL:
	       targ = used.get_parent_of(projects_to_search);
	       if (targ.xrisnotnull()) {
		  relate_to_symbol(AUS_DATAREF_INV, targ);
	       }
               break;
	    default:
	       break;
	    }
	 }

         break;
      }
      case DD_MACRO:
      {
          symbolArr useds;
          my_sym.get_link_chase_typedefs(is_using, useds, projects_to_search);
          ForEachS(used, useds)
          {
              switch(used.get_kind()) 
              {
              case DD_FUNC_DECL:
              case DD_MACRO:
                  relate_to_symbol(AUS_FCALL_INV, used);
                  break;
              case DD_VAR_DECL:
              case DD_CLASS:
              case DD_ENUM:
              case DD_TYPEDEF:
                  relate_to_symbol(AUS_DATAREF_INV, used);
                  break;
              case DD_FIELD:
		  if (used.get_link(has_type, targs, projects_to_search)) {
		     relate_to_type(targs[0], AUS_DATAREF_INV);
		  }
		  /* FALLTHROUGH */
              case DD_ENUM_VAL:
                  targ = used.get_parent_of(projects_to_search);
                  if (targ.xrisnotnull())
                      relate_to_symbol(AUS_DATAREF_INV, targ);
                  break;
              default:
                  break;
              }
          }
          break;
      }
      case DD_TYPEDEF:
      {
          symbolArr useds;
          my_sym.get_link_chase_typedefs(have_arg_type, useds, projects_to_search);
          ForEachS(used, useds)
          {
              switch(used.get_kind())
              {
              case DD_CLASS:
              case DD_ENUM:
              case DD_TYPEDEF:
                  relate_to_symbol(AUS_DATAREF_INV, used);
                  break;
              case DD_MACRO:
                  relate_to_symbol(AUS_FCALL_INV, used);
                  break;
              default:
                  break;
              }
          }
          break;
      }
      case DD_ENUM:
      {
          symbolArr members;
          my_sym.get_link(is_using, members, projects_to_search);
          symbolPtr member;
          ForEachS(member, members)
          {
              symbolArr targs;
              symbolPtr targ;
              member->get_link(is_using, targs);
              ForEachS(targ, targs)
              {
                  switch(targ.get_kind())
                  {
                  case DD_VAR_DECL:
                      relate_to_symbol(AUS_DATAREF_INV, used);
                      break;
                  case DD_MACRO:
                      relate_to_symbol(AUS_FCALL_INV, used);
                      break;
                  case DD_ENUM_VAL:
                  {
                      symbolPtr targ_enum = targ.get_parent_of(projects_to_search);
                      if (targ_enum.xrisnotnull())
                          relate_to_symbol(AUS_DATAREF_INV, targ_enum);
                      break;
                  }
                  default:
                      break;
                  }
              }
          }
          break;
      }
      default:
          break;
      }
   }

   return 1;
}

//
// The original relate function called half a dozen other things.
//

int Entity::relate() {
  Initialize(Entity::relate);

  newrelate();

  return 1;
}


//
// Returns the name of the associated xrefSymbol.

char *Entity::get_name() const {
  Initialize(Entity::get_name);

  return my_sym.get_name();
}

//
// Return dormant/live status:

int Entity::isdead() {
  return !isundead;
}

//
// Goes down from this entity to mark all code that is actually referenced.
// We stop going down and go sideways when we find something already marked 
// as live, both to protect against infinite recursion and to improve speed
// if this is done several times with different top-level functions.

void Entity::find_live_code(struct weight *w, int nw, int depth, FILE* dump_file) {
   Initialize(Entity::find_live_code);

   if (dump_file) {
      OSapi_fprintf(dump_file, "%2d:%.*s ", depth, depth, "==============================");
      dump_refs_and_weights(dump_file, *allentities);
   }
   ddKind my_kind = my_sym.get_kind();
   refs_and_weights_iter it(rw);
   ref_and_weight* ref;
   while (ref = it.next()) {
      if (w[ref->weight].outgoing) {
	 EntityPtr ent = EntityPtr((*allentities)[ref->targ_ref]);
	 symbolPtr ent_sym = ent->my_sym;
	 // The condition following the "||" handles the case where there
	 // is a direct reference to a virtual destructor that was earlier
	 // processed only by virtue of being a member of a base class of
	 // a destroyed object.
	 if (!ent->isundead ||
	     (ref->weight == AUS_FCALL &&
	      strstr(ent_sym.get_name(), "::~") &&
	      ent_sym->get_attribute(VIRT_ATT, 1))) {
	    // The condition following the "||" handles the case of a ptr-to-mbr
	    // variable that is initialized with "ent"; dereferencing this variable
	    // does a virtual dispatch, so the code below must be executed in this
	    // case.
	    if (ref->weight == AUS_FCALL ||
		(my_kind == DD_VAR_DECL && ent_sym.get_kind() == DD_FUNC_DECL)) {
	       symbolArr relateds;
	       ent_sym.get_overrides(relateds, false);
	       ent->add_operator_delete(relateds);
	       ent_sym.get_base_dtors(relateds);
	       ent->find_member_dtors(relateds);
	       if (relateds.size() == 0) {
		  relateds.insert_last(ent_sym);
	       }
	       symbolPtr related;
	       ForEachS(related, relateds) {
		  EntityPtr rel_ent = allentities->item(related);
		  if (rel_ent && !rel_ent->isundead) {
		     rel_ent->isundead = 1;
		     if (dump_file) {
			OSapi_fprintf(dump_file, "+");
		     }
		     rel_ent->find_live_code(w, nw, depth + 1, dump_file);
		  }
	       }
	    }
	    else {
	       ent->isundead = 1;
	       if (dump_file) {
		  OSapi_fprintf(dump_file, " ");
	       }
	       ent->find_live_code(w, nw, depth+1, dump_file);
	    }
	 }
      }
   }
}

//------------------------------------------
// Entity::find_member_dtors: if this Entity is a destructor, examines
// all sibling members for destructors that will be implicitly invoked by
// this destructor and adds any such member destructors to the argument
// symbolArr.  THIS IS A CROCK AND A KLUDGE AND SHOULD BE REMOVED AS SOON
// AS THE MODEL HAS THESE RELATIONSHIPS IN IT.
//------------------------------------------

void Entity::find_member_dtors(symbolArr& dtors) {
   Initialize(Entity::find_member_dtors);

   // Am I a destructor?

   if (strstr(my_sym.get_name(), "::~")) {

      // Find my class and its members (my siblings)

      symbolArr users;
      symbolPtr my_class = my_sym.get_parent_of(projects_to_search);
      if (my_class.xrisnotnull()) {
	 symbolArr siblings;
	 my_class.get_link(is_using, siblings, projects_to_search);

	 // For each data member sibling, check its class for a dtor

	 symbolPtr sibling;
	 ForEachS(sibling, siblings) {
	    if (sibling.get_kind() == DD_FIELD) {
	       symbolArr sib_classes;
	       symbolArr types;
	       if (sibling.get_link(has_type, types, projects_to_search)) {
		  if (collect_classes_from_type(types[0], sib_classes,
						projects_to_search)) {
		     symbolPtr sib_class;
		     ForEachS(sib_class, sib_classes) {
			symbolArr members;
			sib_class.get_link(is_using, members,
					    projects_to_search);
			symbolPtr member;
			ForEachS(member, members) {
			   if (member.get_kind() == DD_FUNC_DECL &&
			       strstr(member.get_name(), "::~")) {
			      member.get_base_dtors(dtors);	// member's and member's bases'
			      break;
			   }
			}
		     }
		  }
	       }
	    }
	 }
      }
   }
}

//------------------------------------------
// Entity::add_operator_delete: another crock to write around missing model
// information.  Every destructor that is (or might be because of virtual
// overriding) should be listed as calling that class's operator delete()
// (if any) so that the operator deletes are not listed as dormant.  This
// should be called immediately after doing a "get_overrides", because it
// assumes the input array is filled (only) with virtual overrides of this
// symbol.
//------------------------------------------

void Entity::add_operator_delete(symbolArr& overrides) {
   Initialize(Entity::add_operator_delete);

   if (strstr(my_sym.get_name(), "::~")) {	// I'm a destructor
      symbolArr op_dels;
      if (overrides.size() == 0) {
	 overrides.insert_last(my_sym);	// at least work on myself
      }
      symbolPtr override;
      ForEachS(override, overrides) {
	 symbolPtr the_class = override.get_parent_of(projects_to_search);
	 if (the_class.xrisnotnull()) {
	    symbolArr members;
	    the_class->get_link(is_using, members);
	    symbolPtr member;
	    ForEachS(member, members) {
	       if (strstr(member.get_name(), "operator delete")) {
		  op_dels.insert_last(member);
		  break;	// only one per class
	       }
	    }
	 }
      }
      overrides.insert_last(op_dels);
   }
}

//------------------------------------------
// Entity::dump_refs_and_weights
//------------------------------------------

void Entity::dump_refs_and_weights(FILE* f, const objArr& entities) {
   Initialize(Entity::dump_refs_and_weights);

   OSapi_fprintf(f, "%s:\n", get_name());
   rw.dump(f, entities);
}

//------------------------------------------
// Entity::set_allentities [static]
//------------------------------------------

void Entity::set_allentities(allentity* ae) {
   Initialize(Entity::set_allentities);

   allentities = ae;
}

//------------------------------------------
// Entity:: set_preve
//------------------------------------------

void Entity::set_preve(Entity* e) {
   Initialize(Entity::set_preve);

   preve = e;
}

//------------------------------------------
// Entity::set_nexte
//------------------------------------------

void Entity::set_nexte(Entity* e) {
   Initialize(Entity::set_nexte);

   nexte = e;
}

//------------------------------------------
// Entity::set_basetype()
//------------------------------------------

void Entity::set_basetype() {
   Initialize(Entity::set_basetype);

   if (!basetype) {
      symbolArr targs;
      if (my_sym.get_link(have_arg_type, targs, projects_to_search)) {
	 symbolPtr targ;
	 ForEachS(targ, targs) {
	    ddKind k = targ.get_kind();
	    if (k == DD_CLASS || k == DD_ENUM) {
	       basetype = allentities->item(targ);
	       break;
	    }
	 }
      }
      if (!basetype) {
	 basetype = &no_class;
      }
   }
}

/*
   START-LOG-------------------------------------------

   $Log: Entity.cxx  $
   Revision 1.21 2002/01/23 09:58:06EST ktrans 
   Merge from branch: mainly dormant code removal
Revision 1.2.1.23  1994/03/29  01:23:29  builder
{P``Port

Revision 1.2.1.22  1994/03/22  20:15:42  mg
Bug track: 1
validation for dormant code

Revision 1.2.1.21  1994/02/16  18:11:45  kws
Port

Revision 1.2.1.20  1993/12/08  21:37:38  wmm
Bug track: 5043
Fix bug 5043.

Revision 1.2.1.19  1993/09/14  16:50:09  bella
testing

Revision 1.2.1.18  1993/09/14  16:36:15  bella
testing

Revision 1.2.1.17  1993/08/25  17:02:36  wmm
Fix bug 4531.

Revision 1.2.1.16  1993/08/23  18:32:54  wmm
Fix bug 4151.

Revision 1.2.1.15  1993/05/24  01:00:41  wmm
Fix bugs 3244, 3243, 3252, and 3152.

Revision 1.2.1.14  1993/05/20  11:52:08  wmm
Tweak automatic subsystem extraction algorithm (use only processed objects for intra-subsystem binding calculation).

Revision 1.2.1.13  1993/05/17  17:33:06  wmm
Performance tuning, etc., for subsystem extraction.

Revision 1.2.1.12  1993/05/11  11:09:40  wmm
Performance enhancement for subsystem extraction (forgot to remove Initialize() macro from inlined function, so it wasn't really inlined).

Revision 1.2.1.11  1993/05/10  11:40:36  wmm
Performance improvements for automatic subsystem extraction.

Revision 1.2.1.10  1993/03/27  02:56:51  davea
changes between xrefSymbol* and fsymbolPtr

Revision 1.2.1.9  1993/03/26  22:19:28  wmm
Fix bug 2827 (adjust to new Xref, etc.).

Revision 1.2.1.8  1993/01/28  02:45:15  efreed
patch to account for dd class/member connection mods.

Revision 1.2.1.7  1992/12/28  19:28:11  wmm
Support new subsystem implementation

Revision 1.2.1.6  1992/11/23  19:34:37  wmm
typesafe casts.

Revision 1.2.1.5  1992/11/18  14:37:00  trung
change dd_of_xref to function

Revision 1.2.1.4  1992/11/17  23:11:48  trung
change using to used_of_user

Revision 1.2.1.3  1992/10/20  09:53:17  wmm
Add copy constructor to allow importation into alphaSET.

Revision 1.2.1.2  92/10/09  20:20:58  swu
*** empty log message ***

*/
