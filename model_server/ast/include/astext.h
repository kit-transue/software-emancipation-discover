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
/* Return the (unique) IDENTIFIER_NODE node for a given name.
   The name is supplied as a char *.  */

extern "C" astIdentifier *get_identifier (char*);
/*extern "C" tree lookup_name (tree);*/

/* In a VAR_DECL, nonzero means allocate static storage.
   In a FUNCTION_DECL, currently nonzero if function has been defined.
   In a CONSTRUCTOR, nonzero means allocate static storage.  */
#ifndef TREE_STATIC
#define TREE_STATIC(NODE) ((NODE)->/* !! <common.> !! */static_flag)
#endif

/* Nonzero for a TREE_LIST or TREE_VEC node means that the derivation
   chain is via a `virtual' declaration.  */
#ifndef TREE_VIA_VIRTUAL
#define TREE_VIA_VIRTUAL(NODE) ((NODE)->static_flag)
#endif

/* In a VAR_DECL or FUNCTION_DECL,
   nonzero means name is to be accessible from outside this module.
   In an identifier node, nonzero means a external declaration
   accesible from outside this module was previously seen
   for this name in an inner scope.  */
#ifndef TREE_PUBLIC
#define TREE_PUBLIC(NODE) ((NODE)->public_flag)
#endif

/* Nonzero for TREE_LIST or TREE_VEC node means that the path to the
   base class is via a `public' declaration, which preserves public
   fields from the base class as public.  */
#ifndef TREE_VIA_PUBLIC
#define TREE_VIA_PUBLIC(NODE) ((NODE)->public_flag)
#endif

/* Ditto, for `private' declarations.  */
#ifndef TREE_VIA_PRIVATE
#define TREE_VIA_PRIVATE(NODE) ((NODE)->private_flag)
#endif
 
/* Nonzero for TREE_LIST node means that the path to the
   base class is via a `protected' declaration, which preserves
   protected fields from the base class as protected.
   OVERLOADED.  */
#ifndef TREE_VIA_PROTECTED
#define TREE_VIA_PROTECTED(NODE) ((NODE)->protected_flag)
#endif


/* In a VAR_DECL, PARM_DECL or FIELD_DECL, or any kind of ..._REF node,
   nonzero means it may not be the lhs of an assignment.
   In a ..._TYPE node, means this type is const-qualified
   (but the macro TYPE_READONLY should be used instead of this macro
   when the node is a type).  */
#ifndef TREE_READONLY
#define TREE_READONLY(NODE) ((NODE)->/* !! <common.> !! */readonly_flag)
#endif

/* In INTEGER_TYPE or ENUMERAL_TYPE nodes, means an unsigned type.
   In FIELD_DECL nodes, means an unsigned bit field.
   The same bit is used in functions as DECL_BUILT_IN_NONANSI.  */
#ifndef TREE_UNSIGNED
#define TREE_UNSIGNED(NODE) ((NODE)->/* !! <common.> !! */unsigned_flag)
#endif

/* These are currently used in classes in C++.  */
#ifndef TREE_PRIVATE
#define TREE_PRIVATE(NODE) ((NODE)->private_flag)
#endif

#ifndef TREE_PROTECTED
#define TREE_PROTECTED(NODE) ((NODE)->protected_flag)
#endif

