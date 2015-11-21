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
 * S Q L _ S Y M _ E N U M . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contains the lookup function sym_kind_lookup() to 
 * get the IFF (Internal Format File) label for each SQL
 * (Structured Query Language) symbol kind.  There are many
 * SQL symbol kinds, but only a few allowed IFF symbol labels.
 * Thus this function changes the symbol to the Discover symbol
 * type that is most closely related.  Most are converted to
 * variable or function.
 *
 * Note: It is best to keep as many native SQL symbol kinds as possible,
 * and only convert them on IFF output because Discover may one
 * day support configurable symbol types in place of the current
 * hard coded ones.
 */

#include "sql_all.h"			/* EXTERN_C */
#include "sql_sym_enum.h"		/* symbol kinds enum */

EXTERN_C const char * sym_kind_lookup (sym_kind sk)
{
   const char *s;

   switch (sk) {
      case sk_none:		s = ""; break;

      case sk_externref: s = "externref"; break;
      case sk_record_type: s = "struct"; break;
      case sk_ref_type: s = "typedef"; break;
      case sk_table_type: s = "typedef"; break;
      case sk_type: s = "type"; break;
      case sk_alias: s = "variable"; break;
      case sk_constant: s = "constant"; break;
      case sk_column: s = "field"; break;
      case sk_column_alias: s = "field"; break;
      case sk_context: s = "variable"; break;
      case sk_cursor: s = "cursor"; break;
      case sk_dblink: s = "variable"; break;
      case sk_exception: s = "exception"; break;
      case sk_file: s = "file"; break;
      case sk_field: s = "field"; break;
      case sk_function: s = "function"; break;
      case sk_label: s = "label"; break;
      case sk_package: s = "package"; break;
      case sk_plsql_table: s = "typedef"; break;
      case sk_procedure: s = "function"; break;
      case sk_record: s = "struct"; break;
      case sk_savepoint: s = "function"; break;
      case sk_schema: s = "variable"; break;
      case sk_sequence: s = "function"; break;
      case sk_string: s = "string"; break;
      case sk_table: s = "table"; break;
      case sk_trigger: s = "function"; break;
      case sk_variable: s = "variable"; break;
      case sk_transaction: s = "variable"; break;
      case sk_database: s = "variable"; break;
      case sk_constraint: s = "variable"; break;
      case sk_index: s = "variable"; break;
      default:		s = ""; break;
   }

   return (s);
}

