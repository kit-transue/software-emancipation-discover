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
#ifndef _oodt_class_member_sorter_h
#define _oodt_class_member_sorter_h

// oodt_class_member_sorter
//------------------------------------------
// synopsis:
//
//    classes for sorting OODT Browser class members
//
// description:
//
//    This file provides a base class and individual derived classes
//    for each of the possible sorting orders that can be imposed on
//    the members of an OODT Browser class.  Objects of these classes
//    are registered with the viewClass object in the desired order
//    and are deleted by the viewClassNode destructor and by the
//    viewClassNode::clear_member_sorters() member function.
//    Registering a sorter object causes it to be inserted in a linked
//    list, with the object representing the primary sorting order
//    placed last.  Sorting is accomplished by invoking the sort()
//    member function of the first object on the list, which will
//    successively invoke each of the sort() functions for all the
//    other sorter objects.
//------------------------------------------

// include files

#ifndef _objDb_h
#include <objDb.h>
#endif
#ifndef ARRAYOB_H
#include <ArrayOb.h>
#endif
#ifndef _objArr_h
#include <objArr.h>
#endif

//=======================================================
// This is the abstract base class of all member sorters:

class oodt_class_member_sorter {
public:
   oodt_class_member_sorter(): fwd_link(NULL) { }
   virtual ~oodt_class_member_sorter() { }
   void set_next(oodt_class_member_sorter*);
   oodt_class_member_sorter* get_next() const;
   virtual void sort(objArr&) = 0;
   virtual bool must_be_first() const;

private:
   oodt_class_member_sorter* fwd_link;
};

inline void oodt_class_member_sorter::set_next(oodt_class_member_sorter* nxt) {
   fwd_link = nxt;
}

inline oodt_class_member_sorter* oodt_class_member_sorter::get_next() const {
   return fwd_link;
}


//==============================================
// This is the sorter for base/derived ordering:

enum inheritance_sort_types { BASE_MEMBERS, DERIVED_MEMBERS };

class oodt_inheritance_sorter: public oodt_class_member_sorter {
public:
   oodt_inheritance_sorter(inheritance_sort_types sort_first,
         inheritance_sort_types sort_last);
   void sort(objArr&);

private:
   inheritance_sort_types order[2];
};


//====================================================================
// This is the sorter for member kind (function, data, type) ordering:

enum member_kind_sort_types { FUNCTION_MEMBERS, DATA_MEMBERS, TYPE_MEMBERS };

class oodt_kind_sorter: public oodt_class_member_sorter {
public:
   oodt_kind_sorter(member_kind_sort_types sort_first,
         member_kind_sort_types sort_middle, member_kind_sort_types sort_last);
   void sort(objArr&);

private:
   member_kind_sort_types order[3];
};


//===================================================================
// This is the sorter for member category (class, instance, virtual):

enum member_category_sort_types {
   CLASS_MEMBERS,	// == static
   INSTANCE_MEMBERS,	// == unqualified
   VIRTUAL_MEMBERS	// == virtual
};

class oodt_category_sorter: public oodt_class_member_sorter {
public:
   oodt_category_sorter(member_category_sort_types sort_first,
         member_category_sort_types sort_middle,
         member_category_sort_types sort_last);
   void sort(objArr&);

private:
   member_category_sort_types order[3];
};


//============================================================
// This is the sorter for access (public, protected, private):

enum member_access_sort_types { PUBLIC_MEMBERS, PROTECTED_MEMBERS,
      PACKAGE_PROT_MEMBERS, PRIVATE_MEMBERS };

class oodt_access_sorter: public oodt_class_member_sorter {
public:
   oodt_access_sorter(member_access_sort_types sort_first,
         member_access_sort_types sort_middle,
         member_access_sort_types sort_penultimate,
         member_access_sort_types sort_last);
   void sort(objArr&);

private:
   member_access_sort_types order[4];
};


//===========================================
// This is the sorter for alphabetical order:

class oodt_alpha_sorter: public oodt_class_member_sorter {
public:
   void sort(objArr&);
   bool must_be_first() const;
};


//==========================================
// This is the sorter for declaration order:

class oodt_decl_order_sorter: public oodt_class_member_sorter {
public:
   void sort(objArr&);
   bool must_be_first() const;
};

//====================================================================
// This is the sorter for subsystem element kind (function, data, type,
// document) ordering:

enum subsys_kind_sort_types { SUBSYS_FUNCTIONS, SUBSYS_TYPES, SUBSYS_DATA,
      SUBSYS_DOCUMENTS };

class oodt_subsys_kind_sorter: public oodt_class_member_sorter {
public:
   oodt_subsys_kind_sorter(subsys_kind_sort_types sort_first,
         subsys_kind_sort_types sort_second, subsys_kind_sort_types sort_third,
	 subsys_kind_sort_types sort_last);
   void sort(objArr&);

private:
   subsys_kind_sort_types order[4];
};


/*
    START-LOG-------------------------------

    $Log: oodt_class_member_sorter.h  $
    Revision 1.4 1998/11/13 11:20:54EST ykulemin 
    Adding Java support to the sorting mechanism
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:05  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif
