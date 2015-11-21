/*
 * S Q L _ S Y M B O L . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * This include file contains the definitions for the routines and
 * structures for the symbol table of the SQL parser. 
 */

#ifndef SQL_SYMBOL_H
#define SQL_SYMBOL_H

#include "sql_all.h"		/* EXTERN_C */
#include "sql_sym_enum.h"	/* sym_kind */

/*
 * Database vendor names and bitmasks
 *
 * The name is used to compare (ignoring case) the command
 * line flag specifing vendor.  The bitmask values are used
 * in the keyword table and to test for vendor specific
 * constructs.
 *
 * Just because a vendor is listed here does not mean that
 * it is supported.  See sql_driver.C for the list of
 * implemented vendors.  The rest are for future use.
 */
#define DBVNAME_UNKNOWN		"Unknown"
#define DBVNAME_ANSISQL		"ANSI"
#define DBVNAME_ORACLE		"Oracle"
#define DBVNAME_INFORMIX       	"Informix"
#define DBVNAME_SYBASE		"Sybase"
#define DBVNAME_DB2		"DB2"

#define DBVENDOR_UNKNOWN	0x00
#define DBVENDOR_ANSI		0x01
#define DBVENDOR_ORACLE		0x02
#define DBVENDOR_INFORMIX	0x04
#define DBVENDOR_SYBASE		0x08
#define DBVENDOR_DB2		0x10

/* keyword is supported by all vendors */
#define DBVENDOR_ALL		0xff

/* database vendor name and value set in sql_driver.C */
EXTERN_C const char * dbvendor_name;
EXTERN_C int dbvendor;

/*
 * Bit mask for symbol attributes (set in parser)
 *
 * Used when producing ELS (External Language Support)
 * IFF (Internal Format File) symbol entries.
 */
#define ATTRMASK_NONE           0x00	/* no special attributes */
#define ATTRMASK_HOST           0x01	/* host language variable */
#define ATTRMASK_HOSTIND        0x02	/* host language NULL indicator */
#define ATTRMASK_CURSOR         0x04	/* cursor indicator */
#define ATTRMASK_LOOPIDX        0x08	/* loop index variable */

/*
 * Structure for symbol table entry.
 *
 * sym_text	- symbol text (e.g. the column "last_name")
 * sym_key	- symbol text used for key matching (in hash table) and
 *		  output of IF.  Generally uppercase version of text (so that
 *		  compares are case-insensitive (needed for SQL)), but
 *		  unchanged for strings, quoted identifiers, and filenames.
 * full_sym     - ptr to fully qualified name symbol (in the case of symbols
 *                which can be referenced by a local or partial name, for
 *                example: "last_name" column of table "employee" would be
 *                "employee.lastname" in full_sym->sym_text.
 * alias_sym    - ptr to "real name" symbol.
 * proc_sym     - ptr to procedure or function symbol for current local symbol.
 * overload_sym - ptr to symbol with same name but different type.
 *		  (linked-list: sym->overload_sym->overload_sym etc)
 * ref_count    - count of references
 * proc_block   - sub-block id within proc (0 is primary)
 * sym_token_type - symbol kind (variable, function, etc.)
 * sym_number	- symbol number
 *		  (zero if not found in program - see built-in symbols below)
 *                a number of symbol entries can have the same symbol number
 *                although they would all represent the same symbol but
 *                in different scopes or aliases.
 * attr_mask	- Bit mask of symbol attributes (above) 
 * sym_defined	- flag: is symbol defined anywhere
 *		  (not just referenced or declared)
 * sym_active   - flag: symbol is currently available.
 *                (false when adding a symbol to the current
 *                scope before it can be used.  E.g. column aliases
 *                are defined in the SELECT item list, but can't be
 *                referenced until the WHERE clause.)
 * sym_specfile - flag: symbol is in symbol specification file (listed on
 *		  the command line).
 * sym_replace  - flag: (esql only) symbol refers to a host symbol and
 *		  should be replaced by the host symbol when the IF file
 *		  is processed.
 *
 * TBD: List of overloaded procs.
 * Since datatypes of parameters are not always known, there needs
 * to be a general symbol for overloaded procs as well as
 * one for each instance.  See comment for ast_resolve::resolve_function()
 * in sql_resolve.C for more info.
 */
typedef struct sql_sym_entry SQL_SYM;

struct sql_sym_entry {
   char * 		sym_text;
   char * 		sym_key;
   SQL_SYM *            full_sym;
   SQL_SYM *            alias_sym;
   SQL_SYM *            proc_sym;
   SQL_SYM *            overload_sym;
   long int             ref_count;
   int                  proc_block;
   sym_kind		sym_token_type;
   int			sym_number;
   unsigned int		attr_mask;
   unsigned char	sym_defined;
   unsigned char        sym_active;
   unsigned char        sym_specfile;
   unsigned char        sym_replace;
};

/*
 * Structure for built-in symbols table.
 *
 * This is a list of symbols that come already installed with Oracle,
 * Informix, etc. such as the function UPPER() to convert a string to
 * uppercase,  as well as standard exception names, packages, etc.
 *
 * They are added to the symbol table once on initialization
 * with a sym_number of zero.  If referenced in the
 * SQL [input] program, then it is assigned a real (non-zero)
 * number.
 */
typedef struct sql_symtab {
   char* 	sym_text;
   sym_kind	sym_token_type;
} SQL_TAB;

/*
 * Structure for allocating blocks of symbols
 *
 * The symbol table memory is allocated in blocks.
 * When a new symbol table entry is needed, a pointer
 * to the next one in the current blocked is passed back
 * and the block's counter is incremented.  When the current
 * block is filled, another is allocated and placed on the
 * block list.
 *
 * syms		- allocated array of symbol entries
 * current_no	- symbol counter for this block (array index)
 *		  when this reaches the number of symbols allocated
 *		  in a block, this block is fully assigned.
 * next		- ptr to next block or 0 for current block.
 * prev		- ptr to previous block or 0 for first block.
 */
typedef struct sql_symbol_list_struct {
   SQL_SYM * syms;
   unsigned int current_no;
   struct sql_symbol_list_struct * next;
   struct sql_symbol_list_struct * prev;
} SQL_LIST;

/* is symbol kind a procedure or function */
EXTERN_C int is_proc_symbol_kind (sym_kind sk);

/* C function wrapper for initialize */
EXTERN_C int sql_symbols_initialize (int dbtype);

/* C function wrapper for lookup */
EXTERN_C SQL_SYM* sql_symbol_lookup (const char* text);

/* C function wrapper for global lookup */
EXTERN_C SQL_SYM* sql_symbol_global_lookup (const char* text);

/* C function wrapper for add */
EXTERN_C SQL_SYM* sql_symbol_add (const char* text, sym_kind token_type);

/* C function wrapper for add_previous */
EXTERN_C SQL_SYM* sql_symbol_add_prev (const char* text, sym_kind token_type);

/* C function wrapper for add_symbol_scope */
EXTERN_C SQL_SYM* sql_symbol_add_to_symbol_scope (SQL_SYM * scope_sym,
				      const char* text,
				      sym_kind token_type);

/* C function wrapper for add_local */
EXTERN_C SQL_SYM* sql_symbol_local (SQL_SYM * full_sym, const char* text);

/* C function wrapper for add_previous_local */
EXTERN_C SQL_SYM* sql_symbol_prev_local (SQL_SYM * full_sym,
					 const char* text);

/* C function wrapper for global_add */
EXTERN_C SQL_SYM* sql_symbol_extern (const char* text, sym_kind token_type);

/* start a new deeper scope for symbols */
EXTERN_C void sql_push_scope (const char* name, SQL_SYM * proc_sym);

/* end previous scope for symbols */
EXTERN_C void sql_pop_scope (void);

/* get current scope level (0 == global) */
EXTERN_C unsigned int sql_scope_level (void);

/* get current procedure or function symbol */
EXTERN_C SQL_SYM * sql_scope_proc_symbol (void);

/* get current procedure or function block id */
EXTERN_C unsigned int sql_scope_proc_block (void);

/* set current table name */
EXTERN_C int sql_scope_set_table_name (const char * tabname);

/* clear current table name */
EXTERN_C int sql_scope_clear_table_name (void);

/* get current table name */
EXTERN_C const char *sql_scope_table_name (void);

/* assign a symbol_number */
EXTERN_C void sql_symbol_number_assign (SQL_SYM * sym);

/* loop through all symbols in symbol table */
EXTERN_C void sql_symbol_init_list (void);
EXTERN_C SQL_SYM * sql_symbol_next (void);

/* clear local symbols */
EXTERN_C void sql_symbols_reset (void);

#endif /* SQL_SYMBOL_H */
