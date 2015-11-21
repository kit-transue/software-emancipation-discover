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
#include <general.h>
#include <genError.h>
#include <systemMessages.h>
#include <ddKind.h>
#include <ddKind_readable_names.h>
#ifndef ISO_CPP_HEADERS
#include <stddef.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstddef>
using namespace std;
#include <cstring>
#endif /* ISO_CPP_HEADERS */

static const char* readable_name[] = {
   TXT("UNKNOWN"),			// DD_UNKNOWN
   TXT("TYPEDEF"),			// DD_TYPEDEF
   TXT("MACRO"),			// DD_MACRO
   TXT("VARIABLE"),			// DD_VAR_DECL
   TXT("FIELD"),			// DD_FIELD
   TXT("FUNCTION"),			// DD_FUNC_DECL
   TXT("PARAMETER"),			// DD_PARAM_DECL
   TXT("ENUM"),				// DD_ENUM
   TXT("ENUMERATION VALUE"),		// DD_ENUM_VAL
   TXT("CLASS"),			// DD_CLASS
   TXT("MACRO LIST"),			// DD_MACRO_LIST
   TXT("FUNCTION LIST"),		// DD_FUNC_DECL_LIST
   TXT("VARIABLE LIST"),		// DD_VAR_DECL_LIST
   TXT("TYPEDEF LIST"),			// DD_TYPEDEF_LIST
   TXT("UNION"),			// DD_UNION
   TXT("GROUP/SUBSYSTEM"),		// DD_SUBSYSTEM
   TXT("ASSOCIATION LINK"),		// DD_ASSOCLINK
   TXT("MODULE"),			// DD_MODULE
   TXT("PROJECT"),			// DD_PROJECT
   TXT("INTERFILE LINK SOURCE"),	// DD_IFL_SRC
   TXT("LINK"),				// DD_LINK
   TXT("INTERFILE LINK TARGET"),	// DD_IFL_TRG
   TXT("STYLE"),			// DD_STYLE
   TXT("CATEGORY"),			// DD_CATEGORY
   TXT("SEMANTIC TYPE"),		// DD_SEMTYPE
   TXT("SYNTACTIC TYPE"),		// DD_SYNTYPE
   TXT("EXTERN FILE"),			// DD_EXTFILE  26
   TXT("LANGUAGE"),			// DD_LANGUAGE     27
   TXT("ERROR NOTE"),			// DD_ERRORNOTE
   TXT("ROOT"),				// DD_ROOT
   TXT("RELATION"),			// DD_RELATION  30
   TXT("RELATION SOURCE"),		// DD_REL_SRC  31
   TXT("RELATION TARGET"),		// DD_REL_TRG  32
   TXT("TEMPLATE"),			// DD_TEMPLATE  33
   TXT("COMPONENT"),			// DD_COMPONENT  34
   TXT("BUGS"),				// DD_BUGS  35
   TXT("SOFTWARE PROJECT"),		// DD_SW_PROJ  36
   TXT("SUBMISSION"),			// DD_SUBMISSION  37
   TXT("ASSOCIATION CONSTANT"),		// DD_ASSOC_CONST  38
   TXT("CHECKSUM"),			// DD_CHECKSUM     39
   TXT("SOFT ASSOCIATION"),		// DD_SOFT_ASSOC  40
   TXT("SOFT ASSOCIATION TYPE"),	// DD_SOFT_ASSOC_TYPE  41
   TXT("LINK SPECIFICATION"),		// DD_LINKSPEC  42
   TXT("LINK NODE"),			// DD_LINKNODE     43
   TXT("STRING"),                       // DD_STRING       44
   TXT("LOCAL"),                        // DD_LOCAL        45
   TXT("SCOPE"),                        // DD_SCOPE        46
   TXT("INSTANCE"),                     // DD_INSTANCE     47
   TXT("SMT"),                          // DD_SMT          48
   TXT("REGION"),                       // DD_REGION       49
   TXT("EXTGROUP"),                     // DD_EXTGROUP     50
   TXT("EXTGROUP TMP"),                 // DD_EXTGROUP_TMP 51
   TXT("AST"),                          // DD_AST          52
   TXT("DATA"),                         // DD_DATA         53
   TXT("PACKAGE"),                      // DD_PACKAGE      54
   TXT("CURSOR"),                       // DD_CURSOR       55
   TXT("SQL TABLE"),                    // DD_SQL_TABLE    56
   TXT("EXCEPTION"),                    // DD_EXCEPTION    57
   TXT("EXTERN REERENCEF"),             // DD_EXTERN_REF   58
   TXT("LABEL"),                        // DD_LABEL        59
   TXT("VERSION"),                      // DD_VERSION      60
   TXT("NUMBER"),                       // DD_NUMBER       61
   TXT("NAMESPACE"),                    // DD_NAMESPACE    62
   TXT("INTERFACE"),                    // DD_INTERFACE    63
};

static size_t num_names = sizeof(readable_name) / sizeof(const char*);

const char* readable_name_of_ddKind(ddKind k) {
   Initialize(readable_name_of_ddKind);

   if (k < 0 || k > num_names) {
      k = DD_UNKNOWN;
   }
   return readable_name[k];
}

ddKind ddKind_of_readable_name(const char* name) {
   Initialize(ddKind_of_readable_name);

   for (size_t i = 0; i < num_names; i++) {
      if (strcmp(name, readable_name[i]) == 0) {
	 return ddKind(i);
      }
   }
   return DD_UNKNOWN;
}

// Give a short, understandable to user name of the kind of entity
char *ddKind_external_name(ddKind kind)
{
    const char *knd = ddKind_name(kind);
    static char obj_kind[4];
 
    knd += 3;                    //skip first 3 chars (DD_)
    strncpy(obj_kind, knd, 3);
    obj_kind[3] = '\0';
    return(obj_kind);
}
 
