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
//
//-----------------------------------------------
//
// Synopsis: Reporting functions for ste subsystem
//
//
// Ste events like addition, deletion, replacement
// of documents are reported here.
//-----------------------------------------------
//
// History:
//
//     S. Kadakia   	Initial Revision.
//
//-----------------------------------------------


#include "genError.h"
#include "ste-report.h"
#include "RTL.h"
#include "RTL_Names.h"
#include "RTL_externs.h"
#include "RTL_apl_extern.h"
#include "steSlot.h"

// All documents
static RTLNodePtr documents_rtl = NULL;

// All styles
static RTLNodePtr globals_styles_rtl = NULL;

// Application styles
static RTLNodePtr ste_styles_rtl = NULL;

// system and user categories
static RTLNodePtr sys_categories_rtl = NULL;
static RTLNodePtr user_categories_rtl = NULL;

RTLNodePtr get_global_styles_rtl() { return globals_styles_rtl; }
RTLNodePtr get_global_sys_categories_rtl() { return sys_categories_rtl; }
RTLNodePtr get_global_user_categories_rtl() { return user_categories_rtl; }
RTLNodePtr get_documents_rtl() { return documents_rtl; }

#define SET_STE_STYLES_RTL \
    if (ste_styles_rtl == NULL) \
	ste_styles_rtl = apl_RTL_get_rtl ("AlphaSet STE Styles")

#define SET_DOCUMENTS_RTL \
    if (documents_rtl == NULL) \
        documents_rtl = apl_RTL_get_rtl (ASET_DOCUMENTS)

#define SET_GLOBAL_STYLES_RTL \
    if (globals_styles_rtl == NULL) \
       globals_styles_rtl = apl_RTL_get_rtl (ASET_STYLES);

#define SET_SYSTEMS_CATEGORIES_RTL \
    if (sys_categories_rtl == NULL) \
	sys_categories_rtl = apl_RTL_get_rtl (ASET_CATEGORIES);

#define SET_USER_CATEGORIES_RTL \
    if (user_categories_rtl == NULL) \
	user_categories_rtl = apl_RTL_get_rtl (ASET_USER_CATEGORIES);


// Reports creation of document
void ste_report_doc_create (steDocument *ste_doc_head)
{
   Initialize (ste_report_doc_create);
   char *nm = ste_doc_head->get_name();
   if ( !nm || (strlen( nm ) == 0) )
      Return;

   SET_DOCUMENTS_RTL;

   if (ste_doc_head)
      rtl_add_obj (documents_rtl, ste_doc_head);

   Return;
}

// Reports deletion of document
void ste_report_doc_delete (steDocument *ste_doc_head)
{
   Initialize (ste_report_doc_delete);

   SET_DOCUMENTS_RTL;

   if (ste_doc_head)
      rtl_delete_obj (documents_rtl, ste_doc_head);

   Return;
}

// Reports replacement of document
void ste_report_doc_replace (steDocument *old_ste_doc_head,
			     steDocument *new_ste_doc_head)
{
   Initialize (ste_report_doc_replace);

   // validate pointers
   if ( (old_ste_doc_head == NULL) ||
        (new_ste_doc_head == NULL) )
      Return;

   SET_DOCUMENTS_RTL;

   // do it here
   rtl_delete_obj (documents_rtl, old_ste_doc_head);

   rtl_add_obj (documents_rtl, new_ste_doc_head);

   Return;
}

// Reports addition of new system category
void ste_report_system_category_add (steSlotPtr category)
{
   Initialize (ste_report_system_category_add);

   // validate pointer
   if (category == NULL)
      Return;

   SET_SYSTEMS_CATEGORIES_RTL;

   rtl_add_obj (sys_categories_rtl, category);

   Return;
}

// Reports addition of new user category
void ste_report_user_category_add (steSlotPtr category)
{
   Initialize (ste_report_user_category_add);

   // validate pointer
   if (category == NULL)
      Return;

   SET_USER_CATEGORIES_RTL;

   rtl_add_obj (user_categories_rtl, category);

   Return;
}

// Reports deletion of system category
void ste_report_system_category_delete (steSlotPtr category)
{
   Initialize (ste_report_system_category_delete);

   // validate pointer
   if (category == NULL)
      Return;

   SET_SYSTEMS_CATEGORIES_RTL;

   rtl_delete_obj (sys_categories_rtl, category);

   Return;
}

// Reports deletion of user category
void ste_report_user_category_delete (steSlotPtr category)
{
   Initialize (ste_report_user_category_delete);

   // validate pointer
   if (category == NULL)
      Return;

   SET_USER_CATEGORIES_RTL;

   rtl_delete_obj (user_categories_rtl, category);

   Return;
}

// Reports replacement of system category
void ste_report_system_category_replace (steSlotPtr old_category,
                                         steSlotPtr new_category)
{
   Initialize (ste_report_system_category_replace);

   // validate pointer
   if ( (old_category == NULL) || (new_category == NULL) )
      Return;

   SET_SYSTEMS_CATEGORIES_RTL;

   rtl_delete_obj (sys_categories_rtl, old_category);
   rtl_add_obj (sys_categories_rtl, new_category);

   Return;
}

// Reports replacement of user category.
void ste_report_user_category_replace (steSlotPtr old_category,
                                       steSlotPtr new_category)
{
   Initialize (ste_report_user_category_replace);

   // validate pointer
   if ( (old_category == NULL) || (new_category == NULL) )
      Return;

   SET_USER_CATEGORIES_RTL;

   rtl_delete_obj (user_categories_rtl, old_category);
   rtl_add_obj (user_categories_rtl, new_category);

   Return;
}

void ste_report_style_add (steSlotPtr style)
{
   Initialize (ste_report_style_add);

   SET_GLOBAL_STYLES_RTL;

   if (style)
      rtl_add_obj (globals_styles_rtl, style);

   Return;
}

void ste_report_style_delete (steSlotPtr style)
{
   Initialize (ste_report_style_delete);

   SET_GLOBAL_STYLES_RTL;

   if (style)
      rtl_delete_obj (globals_styles_rtl, style);

   Return;
}

void ste_report_style_replace (steSlotPtr old_style, steSlotPtr new_style)
{
   Initialize (ste_report_style_replace);

   SET_GLOBAL_STYLES_RTL;

   if (old_style && new_style) {
      rtl_delete_obj (globals_styles_rtl, old_style);
      rtl_add_obj (globals_styles_rtl, new_style);
   }

   Return;
}

void ste_report_ste_style_add (steSlotPtr style)
{
   Initialize (ste_report_ste_style_add);

   SET_STE_STYLES_RTL;

   if (style)
      rtl_add_obj (ste_styles_rtl, style);

   Return;
}

void ste_report_ste_style_delete (steSlotPtr style)
{
   Initialize (ste_report_ste_style_delete);

   SET_STE_STYLES_RTL;

   if (style)
      rtl_delete_obj (ste_styles_rtl, style);

   Return;
}

void ste_report_ste_style_replace (steSlotPtr old_style, steSlotPtr new_style)
{
   Initialize (ste_report_ste_style_replace);

   SET_STE_STYLES_RTL;

   if (old_style && new_style) {
      rtl_delete_obj (ste_styles_rtl, old_style);
      rtl_add_obj (ste_styles_rtl, new_style);
   }

   Return;
}

void ste_report_document_style_create (steDocument *document)
{
   Initialize (ste_report_document_style_create);

   RTLPtr rtl;

   rtl = db_new (RTL, (""));

   RTLNodePtr root = checked_cast(RTLNode, rtl->get_root ());

   put_relation (styles_rtl_of_document, document, root);

   Return;
}

void ste_report_document_style_add (steDocument *document, steSlotPtr style)
{
   Initialize (ste_report_document_style_add);

   // get document style rtl
   RTLNodePtr rtl = checked_cast(RTLNode, get_relation (styles_rtl_of_document, document));

   // If not created so far create one.
   if (rtl == NULL) {
      ste_report_document_style_create (document);
      rtl = checked_cast(RTLNode, get_relation (styles_rtl_of_document, document));
   }

   rtl_add_obj (rtl, style);

   Return;
}

void ste_report_document_style_delete (steDocument *document, steSlotPtr style)
{
   Initialize (ste_report_document_style_delete);

   // get document style rtl
   RTLNodePtr rtl = checked_cast(RTLNode, get_relation (styles_rtl_of_document, document));

   if (rtl)
      rtl_delete_obj (rtl, style);

   Return;
}

void ste_report_document_style_replace (steDocument *document,
					steSlotPtr old_style,
					steSlotPtr new_style)
{
   Initialize (ste_report_document_style_replace);

   // get document style rtl
   RTLNodePtr rtl = checked_cast(RTLNode, get_relation (styles_rtl_of_document, document));

   if (rtl) {
      rtl_delete_obj (rtl, old_style);
      rtl_add_obj (rtl, new_style);
   }

   Return;
}
/*
   START-LOG-------------------------------------------

   $Log: ste-report.cxx  $
   Revision 1.1 1992/11/21 18:33:41EST builder 
   made from unix file
Revision 1.2.1.3  1992/11/21  23:33:28  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  19:21:01  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
