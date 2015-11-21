/*
 * S Q L _ S Y M _ E N U M . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This include file contains the list of symbol kinds for SQL
 * (Structured Query Language).  Some of these from ANSI SQL,
 * but others are specific to Oracle's PL/SQL (Procedural Language / SQL).
 */

#ifndef SQL_SYM_ENUM_H
#define SQL_SYM_ENUM_H

#include "sql_all.h"

enum symbol_kind_enum {
   sk_none = 0,

   sk_externref,
   sk_record_type,
   sk_ref_type,
   sk_table_type,
   sk_type,
   sk_alias,
   sk_constant,
   sk_column,
   sk_column_alias,
   sk_context,
   sk_cursor,
   sk_dblink,
   sk_exception,
   sk_field,
   sk_file,
   sk_function,
   sk_label,
   sk_package,
   sk_plsql_table,
   sk_procedure,
   sk_record,
   sk_savepoint,
   sk_schema,
   sk_sequence,
   sk_string,
   sk_table,
   sk_trigger,
   sk_variable,
   sk_transaction,
   sk_database,
   sk_constraint,
   sk_index
};

typedef enum symbol_kind_enum sym_kind;

/* get text name of each symbol name (e.g. "variable" for sk_variable)
 * or Discover symbol type name for those symbol kinds that don't
 * correspond to a Discover type.
 */
EXTERN_C const char * sym_kind_lookup (sym_kind sk);

#endif /* SQL_SYM_ENUM_H */
