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
#ifndef __ste_report_h__
#define __ste_report_h__

#include "steDocument.h"
#include "steSlot.h"
#include "RTL.h"
#include "RTL_Names.h"
#include "RTL_externs.h"

// global RTL
extern RTLNodePtr get_global_styles_rtl();
extern RTLNodePtr get_global_sys_categories_rtl();
extern RTLNodePtr get_global_user_categories_rtl();
extern RTLNodePtr get_documents_rtl();

// document reporting functions
extern void ste_report_doc_create (steDocument *ste_doc_head);
extern void ste_report_doc_delete (steDocument *ste_doc_head);
extern void ste_report_doc_replace (steDocument *old_ste_doc_head,
				    steDocument *new_ste_doc_head);

// category reporting functions
extern void ste_report_system_category_add (steSlotPtr category);
extern void ste_report_user_category_add (steSlotPtr category);
extern void ste_report_system_category_delete (steSlotPtr category);
extern void ste_report_user_category_delete (steSlotPtr category);
extern void ste_report_system_category_replace (steSlotPtr old_category,
						steSlotPtr new_category);
extern void ste_report_user_category_replace (steSlotPtr old_category,
						steSlotPtr new_category);

/*
extern void ste_report_style_add (steSlotPtr style);
extern void ste_report_style_delete (steSlotPtr style);
extern void ste_report_style_replace (steSlotPtr old_style, steSlotPtr new_style);
*/
// Styles reporting functions
extern void ste_report_style_add (steSlotPtr style);
extern void ste_report_style_delete (steSlotPtr style);
extern void ste_report_style_replace (steSlotPtr old_style, steSlotPtr new_style);

// Styles for ste application
extern void ste_report_ste_style_add (steSlotPtr style);
extern void ste_report_ste_style_delete (steSlotPtr style);
extern void ste_report_ste_style_replace (steSlotPtr old_style,
					  steSlotPtr new_style);

// Document local styles
extern void ste_report_document_style_create (steDocument *document);
extern void ste_report_document_style_add (steDocument *document,
					   steSlotPtr style);
extern void ste_report_document_style_delete (steDocument *document,
					      steSlotPtr style);
extern void ste_report_document_style_replace
  (steDocument *document, steSlotPtr old_style, steSlotPtr new_style);

#endif
