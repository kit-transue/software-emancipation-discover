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
 * S Q L _ A S T _ M A P . C
 *
 * 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This file contain the mapping between symbol kinds and Abstract Syntax
 * Tree ast node kinds.
 */

#include "sql_all.h"		// EXTERN_C
#include "sql_ast_enum.h"	// ast node kinds
#include "sql_sym_enum.h"	// symbol kinds

//
// ast_sym_kind_map - Return ast node kind for passed symbol kind
//
// Used to build a tree node for a symbol.
//
EXTERN_C ast_node_kind ast_sym_kind_map (sym_kind sk)
{
   ast_node_kind ank;

   switch (sk) {
      case sk_none: ank = ank_none; break;

      case sk_externref: ank = ank_externref; break;
      case sk_record_type: ank = ank_record_type; break;
      case sk_ref_type: ank = ank_ref_type; break;
      case sk_table_type: ank = ank_table_type; break;
      case sk_type: ank = ank_type; break;
      case sk_alias: ank = ank_alias; break;
      case sk_column: ank = ank_column; break;
      case sk_column_alias: ank = ank_column_alias; break;
      case sk_constant: ank = ank_constant; break;
      case sk_context: ank = ank_context; break;
      case sk_cursor: ank = ank_cursor; break;
      case sk_dblink: ank = ank_dblink; break;
      case sk_exception: ank = ank_exception; break;
      case sk_field: ank = ank_field; break;
      case sk_file: ank = ank_file; break;
      case sk_function: ank = ank_function; break;
      case sk_label: ank = ank_label; break;
      case sk_package: ank = ank_package; break;
      case sk_plsql_table: ank = ank_plsql_table; break;
      case sk_procedure: ank = ank_procedure; break;
      case sk_record: ank = ank_record; break;
      case sk_savepoint: ank = ank_savepoint; break;
      case sk_schema: ank = ank_schema; break;
      case sk_sequence: ank = ank_sequence; break;
      case sk_string: ank = ank_string; break;
      case sk_table: ank = ank_table; break;
      case sk_trigger: ank = ank_trigger; break;
      case sk_variable: ank = ank_variable; break;

      default: ank = ank_none;
   }

   return (ank);
}
