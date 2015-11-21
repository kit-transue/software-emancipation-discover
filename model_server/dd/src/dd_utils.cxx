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
#include "ddSymbol.h"
#include "ddSelector.h"

/****************************************************************/
/* some utilities to work with ddKinds                          */
/* Feel free to modify these functions if you find any problems */
/****************************************************************/

int dd_can_have_ref_file(ddKind kind)
{
  return !(kind == DD_LANGUAGE ||
	   kind == DD_CHECKSUM);
}

/* return true if a symbol with this ddKind can be selected in the browser */
int dd_can_be_selected(ddKind kind)
{
  return kind == DD_TYPEDEF ||
	 kind == DD_MACRO ||
	 kind == DD_VAR_DECL ||
	 kind == DD_FUNC_DECL ||
	 kind == DD_ENUM ||
	 kind == DD_ENUM_VAL ||
	 kind == DD_CLASS ||
         kind == DD_INTERFACE ||
	 kind == DD_UNION ||
	 kind == DD_SUBSYSTEM ||
	 kind == DD_FIELD ||
	 kind == DD_TEMPLATE ||
      //	 kind == DD_SOFT_ASSOC_TYPE ||
      //	 kind == DD_SOFT_ASSOC ||
	 kind == DD_SEMTYPE ||
	 kind == DD_RELATION ||
	 kind == DD_LINKSPEC ||
	 kind == DD_LOCAL ||
	 kind == DD_STRING ||
	 kind == DD_NUMBER ||
	 kind == DD_MODULE ||
	 kind == DD_PROJECT ||
	 kind == DD_SCOPE ||
	 kind == DD_PACKAGE ||
	 kind == DD_CURSOR ||
	 kind == DD_SQL_TABLE ||
	 kind == DD_EXTERN_REF ||
	 kind == DD_CHECKSUM; 
}

int dd_is_function_declaration(ddKind kind)
{
    return kind == DD_FUNC_DECL;
}

int dd_is_type_declaration(ddKind kind)
{
    return kind == DD_TYPEDEF || 
	   kind == DD_ENUM || 
           kind == DD_ENUM_VAL ||
	   kind == DD_CLASS || 
	   kind == DD_INTERFACE || 
           kind == DD_UNION; 
}

int dd_is_nested_type(ddKind kind)
{
    return kind == DD_TYPEDEF ||
           kind == DD_ENUM ||
	   kind == DD_CLASS ||
	   kind == DD_INTERFACE;
}

int dd_is_var_declaration(ddKind kind)
{
    return kind == DD_VAR_DECL || 
           kind == DD_FIELD;
}

int dd_has_smt_direct(ddKind kind)
{
    return kind == DD_TYPEDEF ||
           kind == DD_MACRO ||
	   kind == DD_VAR_DECL ||
	   kind == DD_UNION ||
	   kind == DD_ENUM ||
	   kind == DD_ENUM_VAL ||
	   kind == DD_FIELD ||
	   kind == DD_TEMPLATE ||
	   kind == DD_FUNC_DECL ||
	   kind == DD_CLASS ||
	   kind == DD_INTERFACE ||
	   kind == DD_PACKAGE ||
	   kind == DD_CURSOR  ||
           kind == DD_SQL_TABLE ||
           kind == DD_EXCEPTION ||
	   kind == DD_LABEL ||
           kind == DD_EXTERN_REF;
}

int dd_has_smt_via_main_header(ddKind kind)
{
    return //kind == DD_SOFT_ASSOC_TYPE ||
	   //kind == DD_SOFT_ASSOC ||
	   kind == DD_LINKSPEC ||
	   kind == DD_LINKNODE;
}

int dd_is_not_internal_object(ddKind kind)
{
    return kind == DD_FUNC_DECL ||
           kind == DD_CLASS ||
           kind == DD_INTERFACE ||
           kind == DD_ENUM ||
           kind == DD_MACRO ||
           kind == DD_VAR_DECL ||
           kind == DD_PARAM_DECL ||
           kind == DD_FIELD ||
           kind == DD_TEMPLATE ||
           kind == DD_TYPEDEF ||
           kind == DD_UNION ||
        //kind == DD_SOFT_ASSOC ||
	   kind == DD_LINKSPEC ||
	   kind == DD_PACKAGE ||
	   kind == DD_CURSOR ||
	   kind == DD_SQL_TABLE ||
	   kind == DD_EXTERN_REF; 
}

int dd_notify_xref_if_modified(ddKind kind)
{
    return kind == DD_CLASS || 
           kind == DD_INTERFACE || 
           kind == DD_FUNC_DECL || 
           kind == DD_VAR_DECL ||
	   kind == DD_FIELD;
}

int dd_is_SWT_entity(ddKind kind)
{
    return kind == DD_VAR_DECL ||
           kind == DD_FIELD ||
           kind == DD_FUNC_DECL ||
           kind == DD_ENUM  ||
           kind == DD_ENUM_VAL ||
           kind == DD_CLASS ||
           kind == DD_INTERFACE ||
           kind == DD_UNION ||
           kind == DD_SEMTYPE ||
           kind == DD_SYNTYPE ||
           kind == DD_TEMPLATE ||
           kind == DD_TYPEDEF   ||
           kind == DD_MACRO ||
           kind == DD_PARAM_DECL ||
	   kind == DD_LABEL ||
	   kind == DD_PACKAGE ||
	   kind == DD_NAMESPACE ||
	   kind == DD_CURSOR ||
           kind == DD_MODULE;
}
