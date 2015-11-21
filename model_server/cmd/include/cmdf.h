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
// File cmdf.h - Command Journal
//
#ifndef _CMDF_H_
#define _CMDF_H_

objPlacement * CC_hdr(appPtr);
objPlacement * CC_node(appTreePtr);
objArr       * CC_sel(objArr*);
char         * CC_rel_type(RelType *);
char         * CC_old_val(appTreePtr);

void CC_hdr(appPtr, objPlacement*);
void CC_node(appTreePtr, objPlacement*);
void CC_sel(objArr*, objArr*);

char         * CT_oper_type(objOperType);
char         * CT_hdr(objPlacement *);
char         * CT_node(objPlacement *);
char         * CT_sel(objArr*);
char         * CT_rel_type(char *);

void CT_oper_type(objOperType, ostream&);
void CT_hdr(objPlacement *, ostream&);
void CT_node(objPlacement *, ostream&);
void CT_sel(objArr*, ostream&);
void CT_rel_type(char *, ostream&);

extern "C" objOperType    C_oper_type(char *);
extern "C" appPtr         C_hdr(appType, char *);
extern "C" appTreePtr     C_node(appType, char *, ...);
extern "C" objArr       * C_sel(appTreePtr, ...);
extern "C" RelType      * C_rel_type(char *);

appPtr         CU_hdr(objPlacement *);
appTreePtr     CU_node(objPlacement *);
objArr       * CU_sel(objArr*);
RelType      * CU_rel_type(char *);

#endif
