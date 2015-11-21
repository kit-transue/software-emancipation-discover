/*
 * S Q L _ R E S O L V E . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * Include for routines that resolve symbols that may not have been
 * known when they were first encountered.  These include forward
 * references (columns and package members) and overloaded symbols.
 */

#ifndef SQL_RESOLVE_H
#define SQL_RESOLVE_H

EXTERN_C void ast_resolve_columns (ast_tree * at_table, ast_tree * at_column);

EXTERN_C void ast_resolve_package (const char * package_name, ast_tree * at);

EXTERN_C void ast_resolve_function (ast_tree * at_func, ast_tree * at_args);

EXTERN_C void sym_inactive (SQL_SYM * sym);

EXTERN_C void sym_inactive_clear (void);

#endif /* SQL_RESOLVE_H */
