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
#ifndef _xref_queries_h
#define _xref_queries_h

/* xref_queries.h
//------------------------------------------
// synopsis:
// contains enum definitions used in the scan mode of the 
// browser and in queries from the viewerShell.
// please note that this file is included by
// plain C files (eg viewerShell.if)
//------------------------------------------*/


#include <prototypes.h>

enum groupItems
{
#if 0
    GROUP_PROJECTS,
#endif
    GROUP_FILES,
    GROUP_SUBSYSTEMS,
    GROUP_RELATIONS,
    GROUP_FUNCTIONS,
    GROUP_VARIABLES,
    GROUP_CLASSES,
    GROUP_STRUCTURES,
    GROUP_UNIONS,
    GROUP_ENUMS,
    GROUP_TYPEDEFS,
    GROUP_MACROS,
    GROUP_TEMPLATES,
    GROUP_ERRORS,
    GROUP_ASSOCTYPES,
    GROUP_ASSOCS,
    GROUP_STRINGS,
    GROUP_LOCALS,

    GROUP_NUMBERED              /* Beginning of numbered groups, must be last. */
};

enum showItems
{
    SHOW_IS_DEFINED_IN,
    SHOW_REF_FILE,
    SHOW_INCLUDED_BY,
    SHOW_USED_BY,

    SHOW_IS_USING,
    SHOW_IS_USING_FUNCTIONS,
    SHOW_IS_USING_DATA_MEMBERS,
    SHOW_IS_USING_VARIABLES,
    SHOW_IS_USING_CLASSES,
    SHOW_IS_USING_STRUCTURES,
    SHOW_IS_USING_UNIONS,
    SHOW_IS_USING_ENUMS,
    SHOW_IS_USING_ENUM_VAL,
    SHOW_IS_USING_TYPEDEFS,
    SHOW_IS_USING_MACROS,
    SHOW_IS_USING_STRINGS,
    SHOW_IS_USING_LOCALS,

    SHOW_IS_DEFINING,
    SHOW_IS_DEFINING_FUNCTIONS,
    SHOW_IS_DEFINING_VARIABLES,
    SHOW_IS_DEFINING_CLASSES,
    SHOW_IS_DEFINING_STRUCTURES,
    SHOW_IS_DEFINING_UNIONS,
    SHOW_IS_DEFINING_ENUMS,
    SHOW_IS_DEFINING_ENUM_VALS,
    SHOW_IS_DEFINING_FIELDS,
    SHOW_IS_DEFINING_TYPEDEFS,
    SHOW_IS_DEFINING_MACROS,

    SHOW_NESTED,
    SHOW_NESTED_CLASSES,
    SHOW_NESTED_UNIONS,
    SHOW_NESTED_ENUMS,
    SHOW_NESTED_TYPEDEFS,
    SHOW_CONTAINING,

    SHOW_IS_INCLUDING,
    SHOW_MEMBER_FUNCS,
    SHOW_MEMBER_DATA,
    SHOW_FRIENDS,
    SHOW_HAS_SUPERCLASS,
    SHOW_HAS_SUBCLASS,
    SHOW_DEFINED_AS,
    SHOW_ENUM_MEMBERS,
    SHOW_TEMPLofINST,
    SHOW_INSTofTEMPL,
    SHOW_DECL_USAGE,

    SHOW_ASSOC_OF_SYM,
    SHOW_SYM_OF_ASSOC,
    SHOW_DOCUMENTS,
    SHOW_INSTANCES,

    SHOW_IS_DEFINING_SUBSYSTEMS,  
    SHOW_GRP_MEMBERS,
    SHOW_GRP_PEERS,
    SHOW_GRP_CLIENTS,
    SHOW_GRP_SERVERS,
    SHOW_GRP_OF_MEMBER,

    SHOW_IS_DEFINING_RELATIONS,   /* New item, used in minibrowser */
    SHOW_IS_DEFINING_ERRORS,      /* New item, used in minibrowser */
    SHOW_MODIFIED_OBJECTS,        /* New item, used in minibrowser */
    SHOW_OVERRIDES,               /* New item, used in minibrowser */
    
    SHOW_DECLARED_IN,
    SHOW_DECLARES, 

    SHOW_EXTENDS_SUPERCLASS,
    SHOW_IMPLEMENTED_BY_SUBCLASS,
    SHOW_IMPLEMENTS_SUPERINTERFACE,
    SHOW_EXTENDED_BY_SUBINTERFACE,
    SHOW_NESTED_INTERFACES,
    SHOW_IS_USING_INTERFACES,
    SHOW_IS_DEFINING_INTERFACES,
    SHOW_NESTED_PACKAGES,
    SHOW_GENERATED,
    SHOW_MACROS,
    SHOW_IS_USING_MODULES,
    SHOW_LAST               /* keep this last for range checking */
};

typedef struct
{
    char*         label;
    int           action;
    unsigned int  needs_cplusplus;
    unsigned int  needs_subsystems;
    unsigned int  kind;
    unsigned int  defines;
}
actionSet;

BEGIN_C_DECL
int epoch_lists_enabled();
END_C_DECL

/*
    START-LOG-------------------------------

    $Log: xref_queries.h  $
    Revision 1.24 1999/03/02 17:40:29EST Scott Turner (sturner) 
    adding SHOW_IS_USING_MODULES
 * Revision 1.1  1994/07/27  22:54:32  bakshi
 * Initial revision
 *

    END-LOG---------------------------------
*/

#endif // _xref_queries_h




