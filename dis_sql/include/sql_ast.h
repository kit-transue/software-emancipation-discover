/*
 * S Q L _ A S T . H
 *
 * Copyright 1997 Software Emancipation Technology, Inc.
 *
 * Created Nov. 1997 Mark B. Kaminsky
 *
 * Include file for ast (Abstract Syntax Tree) routines.  Defines the
 * structures used as tree nodes.  The tree is built during parsing
 * and transversed to output symbol and syntax information.
 */

#ifndef SQL_AST_H
#define SQL_AST_H

#include "sql_all.h"
#include "sql_symbol.h"
#include "sql_ast_enum.h"

/* tree node typedef */
typedef struct ast_tree_struct ast_tree;

/* linked list of nodes typedef */
typedef struct ast_treelist_struct ast_treelist;

/* Location structure
 *
 * Indicates line and column number of the input file
 * where a syntax element is located.
 */
typedef struct ast_loc_struct {
   unsigned int lineno;
   unsigned int column;
} ast_loc;

/*
 * Kinds of symbol relations.
 *
 * Indicates the relationship between two symbols.
 */
enum relation_kind_enum {
   rk_none = 0,			/* no relationship */
   rk_type,			/* data type */
   rk_context,			/* is a member */
   rk_call,			/* is called */
   rk_arg			/* is an argument */
};

typedef enum relation_kind_enum relation_kind;

/*
 * Bit mask for symbol reference type.
 *
 * Used to output symbol references to IF file.
 */
#define REFMASK_NONE            0x000   /* not a reference */ 
#define REFMASK_DEF             0x001   /* definition */ 
#define REFMASK_SMT             0x002   /* token reference */ 
#define REFMASK_AST             0x004   /* tree reference */ 

/*
 * Bit mask for relation attributes
 *
 * Booleans (true if set), except for size, scale, and arg
 * which have values associated with them if set.
 */
#define RELMASK_NONE		0x000	/* no attributes */
#define RELMASK_PUBLIC		0x001	/* public package member */
#define RELMASK_PRIVATE		0x002	/* private package member */
#define RELMASK_SIZE		0x004	/* datatype size */
#define RELMASK_SCALE		0x008	/* datatype scale */
#define RELMASK_RETURN		0x010	/* return datatype */
#define RELMASK_PCTTYPE		0x020	/* datatype via %TYPE */
#define RELMASK_PCTROWTYPE	0x040	/* datatype via %ROWTYPE */
#define RELMASK_ARG		0x080	/* argument number */
#define RELMASK_INPUT		0x100	/* input parameter */
#define RELMASK_OUTPUT		0x200	/* output parameter */
#define RELMASK_CONST		0x400	/* constant */

/*
 * ast_info 
 *
 * Information about the current tree node.
 *
 * rk		- kind of relation (see enum above).
 * relation_attr_mask - bit mask of relation attributes (see list above).
 * size		- value of size.  E.g. 5 for "VARCHAR (5)".
 * scale	- value of scale.  E.g. 2 for "DECIMAL (10, 2)".
 * arg		- number of this argument. E.g. 3 for z in "func (x,y,z)".
 * defined	- Flag: node is a symbol definition, not just a
 *		  symbol reference.
 * reference    - Mask: symbol reference type.
 * keyword      - Flag: node is a keyword reference.
 * relation	- Flag: relationship info present.
 * decldef	- Flag: This declaration node is also a definition.
 * replace	- Flag: (esql only) This node is marked as a replacement
 *		  for a host langauge node (IF post-processing).
 *
 * The difference between defined and decldef is that the former is
 * used for SYM definitions and the later for AST nodes (":def" label).
 * 
 */
typedef struct ast_info_struct {
   relation_kind rk;
   unsigned int relation_attr_mask;
   int size;
   int scale;
   int arg;
   unsigned char defined;
   unsigned char reference;
   unsigned char keyword;
   unsigned char relation;
   unsigned char decldef;
   unsigned char replace;
} ast_info;

/*
 * ast_tree
 *
 * Structure for each tree node.
 *
 * ank		- ast node kind (see sql_ast_enum.h).  This is the kind
 *		  of syntax tree node (e.g assignment_statement,
 *		  variable_definition, variable reference, etc).
 * sym		- symbol table entry.
 * info		- node info (above).
 * start	- starting location (in input file) of this syntatical element.
 * end		- ending location.
 * text		- text for this node (but only if a literal, otherwise 0).
 * head		- pointer to child node list.
 * tail		- tail pointer to child node list.
 */
struct ast_tree_struct {
   ast_node_kind ank;
   SQL_SYM * sym;
   ast_info info;
   ast_loc start;
   ast_loc end;
   const char * text;
   ast_treelist * head;
   ast_treelist * tail;
};

/*
 * ast_treelist
 *
 * List structure for tree nodes.
 *
 * Note that ast points to a tree node (subtree) which in turn has
 * its own child node list (head and tail above).  Therefore, a
 * tree node can have siblings (next & prev), as well as children.
 * 
 */
struct ast_treelist_struct {
   ast_tree * ast;
   ast_treelist * next;
   ast_treelist * prev;
};

/* create a new tree node and initialize it with passed args */
EXTERN_C ast_tree * ast_new_tree (ast_node_kind ank,
   SQL_SYM *sym,
   ast_loc start,
   ast_loc end,
   const char * text);

/* add a child tree to the parent's tree list */
EXTERN_C void ast_child_tree (ast_tree * at, ast_tree * at_child);

/* Extend starting location if before parent tree */
EXTERN_C void ast_extend_start (ast_tree * at, ast_loc start);

/* Extend endpoint if after parent tree */
EXTERN_C void ast_extend_end (ast_tree * at, ast_loc end);

/* Recurvely walk tree to find the node of a given kind */
EXTERN_C ast_tree * ast_find_node_kind (ast_tree * at_head, ast_node_kind k);

/* AST root */
EXTERN_C ast_tree* ast_root;

#endif /* SQL_AST_H */
