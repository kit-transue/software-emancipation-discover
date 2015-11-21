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
// oodt_class_member_sorter
//------------------------------------------
// synopsis:
//
//    implementation of classes for sorting OODT Browser class members
//
// description:
//
//    This file implements the sort functions of the various sorters
//    declared in oodt_class_member_sorter.h.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#include "genError.h"
#include "oodt_class_member_sorter.h"
#include "ldrNode.h"
#include "viewNode.h"

/*********************************************
 * oodt_class_member_sorter::must_be_first() *
 *********************************************/

bool oodt_class_member_sorter::must_be_first() const {
   return false;
}


/******************************************************
 * oodt_inheritance_sorter::oodt_inheritance_sorter() *
 ******************************************************/

oodt_inheritance_sorter::oodt_inheritance_sorter(inheritance_sort_types first,
      inheritance_sort_types last) {
   order[0] = first;
   order[1] = last;
}


/***********************************
 * oodt_inheritance_sorter::sort() *
 ***********************************/

void oodt_inheritance_sorter::sort(objArr& members) {
   Initialize(oodt_inheritance_sorter::sort);

   if (members.size()) {
      int count = members.size();
      if (checked_cast(viewSymbolNode,members[count - 1])->
            viewIsKindOf(viewConnectorType))
	 count--;	// don't include connector -- MUST stay last
      objArr temp;
      int j;

      for (int i = 0; i < 2; i++)
	 switch (order[i]) {
	 case BASE_MEMBERS:
            for (j = 0; j < count; j++)
               if (checked_cast(viewMemberNode,members[j])->get_attributes() &
                     INHERITED_MEMBER)
                  temp.insert_last((Obj*) members[j]);
	    break;

	 case DERIVED_MEMBERS:
	    for (j = 0; j < count; j++)
               if ((checked_cast(viewMemberNode,members[j])->get_attributes() &
                     INHERITED_MEMBER) == 0)
                  temp.insert_last((Obj*) members[j]);
	    break;
	 }

      if (count < members.size())
	 temp.insert_last((Obj*) members[count]);   // copy connector

      if (get_next())
         get_next()->sort(temp);

      members = temp;
   }

   Return
}


/****************************************
 * oodt_kind_sorter::oodt_kind_sorter() *
 ****************************************/

oodt_kind_sorter::oodt_kind_sorter(member_kind_sort_types first,
      member_kind_sort_types middle, member_kind_sort_types last) {
   order[0] = first;
   order[1] = middle;
   order[2] = last;
}


/****************************
 * oodt_kind_sorter::sort() *
 ****************************/

void oodt_kind_sorter::sort(objArr& members) {
   Initialize(oodt_kind_sorter::sort);

   if (members.size()) {
      int count = members.size();
      if (checked_cast(viewSymbolNode,members[count - 1])->
            viewIsKindOf(viewConnectorType))
	 count--;	// don't include connector -- MUST stay last
      objArr temp;
      member_attributes filter;

      for (int i = 0; i < 3; i++) {
	 switch (order[i]) {
	 case FUNCTION_MEMBERS:
	    filter = FUNCTION_MEMBER;
	    break;

	 case DATA_MEMBERS:
	    filter = DATA_MEMBER;
	    break;

	 case TYPE_MEMBERS:
	    filter = TYPE_MEMBER;
	    break;
	 }

	 for (int j = 0; j < count; j++)
            if (checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
               temp.insert_last((Obj*) members[j]);
      }

      if (temp.size() < count) {   // missed some
         filter = member_attributes(FUNCTION_MEMBER | DATA_MEMBER |
               TYPE_MEMBER);
         for (int j = 0; j < count; j++) {
            if ((checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
                  == 0)
               temp.insert_last((Obj*) members[j]);
         }
      }

      if (count < members.size())
	 temp.insert_last((Obj*) members[count]);   // copy connector

      if (get_next())
         get_next()->sort(temp);

      members = temp;
   }

   Return
}


/************************************************
 * oodt_category_sorter::oodt_category_sorter() *
 ************************************************/

oodt_category_sorter::oodt_category_sorter(member_category_sort_types first,
      member_category_sort_types middle, member_category_sort_types last) {
   order[0] = first;
   order[1] = middle;
   order[2] = last;
}


/********************************
 * oodt_category_sorter::sort() *
 ********************************/

void oodt_category_sorter::sort(objArr& members) {
   Initialize(oodt_category_sorter::sort);

   if (members.size()) {
      int count = members.size();
      if (checked_cast(viewSymbolNode,members[count - 1])->
            viewIsKindOf(viewConnectorType))
	 count--;	// don't include connector -- MUST stay last
      objArr temp;
      member_attributes filter;

      for (int i = 0; i < 3; i++) {
	 switch (order[i]) {
	 case CLASS_MEMBERS:
	    filter = STATIC_MEMBER;
	    break;

	 case INSTANCE_MEMBERS:
	    filter = ORDINARY_MEMBER;
	    break;

	 case VIRTUAL_MEMBERS:
	    filter = member_attributes(VIRTUAL_MEMBER | PURE_VIRTUAL_MEMBER);
	    break;
	 }

	 for (int j = 0; j < count; j++)
            if (checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
               temp.insert_last((Obj*) members[j]);
      }

      if (temp.size() < count) {   // missed some
         filter = member_attributes(STATIC_MEMBER | ORDINARY_MEMBER |
               VIRTUAL_MEMBER | PURE_VIRTUAL_MEMBER);
         for (int j = 0; j < count; j++) {
            if ((checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
                  == 0)
               temp.insert_last((Obj*) members[j]);
         }
      }

      if (count < members.size())
	 temp.insert_last((Obj*) members[count]);   // copy connector

      if (get_next())
         get_next()->sort(temp);

      members = temp;
   }

   Return
}


/********************************************
 * oodt_access_sorter::oodt_access_sorter() *
 ********************************************/

oodt_access_sorter::oodt_access_sorter(member_access_sort_types first,
      member_access_sort_types middle,
      member_access_sort_types penultimate,
      member_access_sort_types last) {
   order[0] = first;
   order[1] = middle;
   order[2] = penultimate;
   order[3] = last;
}


/******************************
 * oodt_access_sorter::sort() *
 ******************************/

void oodt_access_sorter::sort(objArr& members) {
   Initialize(oodt_access_sorter::sort);

   if (members.size()) {
      int count = members.size();
      if (checked_cast(viewSymbolNode,members[count - 1])->
            viewIsKindOf(viewConnectorType))
	 count--;	// don't include connector -- MUST stay last
      objArr temp;
      member_attributes filter;

      for (int i = 0; i < 4; i++) {
	 switch (order[i]) {
	 case PUBLIC_MEMBERS:
	    filter = PUBLIC_MEMBER;
	    break;

	 case PROTECTED_MEMBERS:
	    filter = PROTECTED_MEMBER;
	    break;

	 case PACKAGE_PROT_MEMBERS:
	    filter = PACKAGE_PROT_MEMBER;
	    break;

	 case PRIVATE_MEMBERS:
	    filter = PRIVATE_MEMBER;
	    break;
	 }

	 for (int j = 0; j < count; j++)
            if (checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
               temp.insert_last((Obj*) members[j]);
      }

      if (temp.size() < count) {   // missed some
         filter = member_attributes(PUBLIC_MEMBER | PROTECTED_MEMBER |
               PACKAGE_PROT_MEMBER | PRIVATE_MEMBER);
         for (int j = 0; j < count; j++) {
            if ((checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
                  == 0)
               temp.insert_last((Obj*) members[j]);
         }
      }

      if (count < members.size())
	 temp.insert_last((Obj*) members[count]);   // copy connector

      if (get_next())
         get_next()->sort(temp);

      members = temp;
   }

   Return
}


//==========================================================================
// Comparison function used by oodt_alpha_sorter::sort(): sorts on the
// basis of member name, using number of levels of inheritance and,
// finally, an ASCII comparison of the fully-qualified name, as tiebreakers.
//==========================================================================

static int greater(const char* str1, const char* str2) {
   const char* mbrname1 = strrchr(str1, ':');
   if (mbrname1)
      mbrname1++;
   else mbrname1 = str1;

   const char* mbrname2 = strrchr(str2, ':');
   if (mbrname2)
      mbrname2++;
   else mbrname2 = str2;

   int result = strcmp(mbrname1, mbrname2);
   if (!result) {
      int colons1 = 0;
      const char *p;
      for (p = str1; *p; p++)
         if (*p == ':')
            colons1++;

      int colons2 = 0;
      for (p = str2; *p; p++)
         if (*p == ':')
            colons2++;

      if (!(result = colons2 - colons1))   // sort base before derived
         result = strcmp(str1, str2);
   }

   return result > 0;
}

/*****************************
 * oodt_alpha_sorter::sort() *
 *****************************/

void oodt_alpha_sorter::sort(objArr& members) {
   Initialize(oodt_alpha_sorter::sort);

   int count = members.size();
   if (count == 0)
      Return;

   if (checked_cast(viewSymbolNode,members[count - 1])->
	 viewIsKindOf(viewConnectorType))
      count--;	// don't include connector -- MUST stay last

   bool swapped = true;

   while (swapped) {
      swapped = false;
      for (int i = 1; i < count; i++) {
         ldrMemberNodePtr ldr_obj1 = checked_cast(ldrMemberNode,get_relation(
               ldrtree_of_viewtree, checked_cast(viewMemberNode,members[i - 1])));
         ldrMemberNodePtr ldr_obj2 = checked_cast(ldrMemberNode,get_relation(
               ldrtree_of_viewtree, checked_cast(viewMemberNode,members[i])));
         if (::greater(ldr_obj1->get_member_name(),
               ldr_obj2->get_member_name())) {
            Obj* temp = members[i - 1];
            members[i - 1] = members[i];
            members[i] = temp;
            swapped = true;
         }
      }
   }

   if (get_next())
      get_next()->sort(members);

   Return
}


/**************************************
 * oodt_alpha_sorter::must_be_first() *
 **************************************/

bool oodt_alpha_sorter::must_be_first() const {
   return true;
}


/**********************************
 * oodt_decl_order_sorter::sort() *
 **********************************/

void oodt_decl_order_sorter::sort(objArr& members) {
   Initialize(oodt_decl_order_sorter::sort);

   if (members.size()) {
      viewSymbolNodePtr view_obj = checked_cast(viewSymbolNode,members[0]);
      if (view_obj && !view_obj->viewIsA(viewConnectorType)) {
	 ldrSymbolNodePtr ldr_obj = checked_cast(ldrMemberNode,
	       get_relation(ldrtree_of_viewtree, view_obj));
	 if (ldr_obj) {
            objArr temp;
	    ldrClassNodePtr class_node =
                  checked_cast(ldrClassNode,ldr_obj->get_parent());
	    for (ldr_obj = checked_cast(ldrSymbolNode,class_node->get_first()); ldr_obj;
		  ldr_obj = checked_cast(ldrSymbolNode,ldr_obj->get_next())) {
	       view_obj = checked_cast(viewSymbolNode,get_relation(viewtree_of_ldrtree,
		     ldr_obj));
	       if (view_obj && !view_obj->viewIsA(viewConnectorType))
		  temp.insert_last(view_obj);
	    }
	    if (checked_cast(viewNode,members[members.size() - 1])->
                  viewIsKindOf(viewConnectorType))
	       temp.insert_last((Obj*) members[members.size() - 1]);

            if (get_next())
               get_next()->sort(temp);

            members = temp;
         }
      }
      else if (get_next())
         get_next()->sort(members);
   }

   Return
}


/*******************************************
 * oodt_decl_order_sorter::must_be_first() *
 *******************************************/

bool oodt_decl_order_sorter::must_be_first() const {
   return true;
}


/******************************************************
 * oodt_subsys_kind_sorter::oodt_subsys_kind_sorter() *
 ******************************************************/

oodt_subsys_kind_sorter::oodt_subsys_kind_sorter(subsys_kind_sort_types first,
      subsys_kind_sort_types second, subsys_kind_sort_types third,
      subsys_kind_sort_types last) {
   order[0] = first;
   order[1] = second;
   order[2] = third;
   order[3] = last;
}


/****************************
 * oodt_subsys_kind_sorter::sort() *
 ****************************/

void oodt_subsys_kind_sorter::sort(objArr& members) {
   Initialize(oodt_subsys_kind_sorter::sort);

   if (members.size()) {
      int count = members.size();
      if (checked_cast(viewSymbolNode,members[count - 1])->
            viewIsKindOf(viewConnectorType))
	 count--;	// don't include connector -- MUST stay last
      objArr temp;
      member_attributes filter;

      for (int i = 0; i < 3; i++) {
	 switch (order[i]) {
	 case SUBSYS_FUNCTIONS:
	    filter = FUNCTION_MEMBER;
	    break;

	 case SUBSYS_DATA:
	    filter = DATA_MEMBER;
	    break;

	 case SUBSYS_TYPES:
	    filter = TYPE_MEMBER;
	    break;

	 case SUBSYS_DOCUMENTS:
	    filter = DOCUMENT_MEMBER;
	    break;
	 }

	 for (int j = 0; j < count; j++)
            if (checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
               temp.insert_last((Obj*) members[j]);
      }

      if (temp.size() < count) {   // missed some
         filter = member_attributes(FUNCTION_MEMBER | DATA_MEMBER |
               TYPE_MEMBER);
         for (int j = 0; j < count; j++) {
            if ((checked_cast(viewMemberNode,members[j])->get_attributes() & filter)
                  == 0)
               temp.insert_last((Obj*) members[j]);
         }
      }

      if (count < members.size())
	 temp.insert_last((Obj*) members[count]);   // copy connector

      if (get_next())
         get_next()->sort(temp);

      members = temp;
   }

   Return
}
/*
   START-LOG-------------------------------------------

   $Log: oodt_class_member_sorter.cxx  $
   Revision 1.3 2000/07/07 08:27:03EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.4  1992/12/09  23:01:19  builder
*** empty log message ***

Revision 1.2.1.3  1992/11/23  20:20:32  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:55:19  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
