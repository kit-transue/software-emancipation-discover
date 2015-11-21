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
#ifndef _dialogType_h
#define _dialogType_h

// enum dialogType
//------------------------------------------
// synopsis:
// Constants for the various dialog types
//
// description:
// enum constants for the various dialog
// types;  there should be one for each
// subclass of dialog
//------------------------------------------
// Restrictions:
// dialogType_PROJ is not currently used
//------------------------------------------

enum dialogType
{
    dialogType_unknown,
    dialogType_SPD,
    dialogType_STE,
    dialogType_PROJ,
    dialogType_CLASS,
    dialogType_RTL,
    dialogType_INTERP,
    dialogType_SMT,
    dialogType_BROWSER,
    dialogType_CallTree,
    dialogType_SUBSYS,
    dialogType_Search,
    dialogType_ERD,
    dialogType_PROJECT
};

#endif // _dialogType_h

/*
   START-LOG--------------------------------------------------------

   $Log: dialogType.h  $
   Revision 1.2 1995/07/27 20:19:40EDT rajan 
   Port
 * Revision 1.1  1993/02/08  16:42:33  glenn
 * Initial revision
 *

   END-LOG----------------------------------------------------------

*/
