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
#ifndef _representation_h
#define _representation_h

#include <genEnum.h>

/*
 * representation.h
 */

/* Make sure that this file can be included in C */

genENUM(repType) {
   Etok_val(Rep_UNKNOWN,-1),

   Etok_val(Rep_VertHierarchy,0),
   Etok(Rep_FlowChart),
   Etok(Rep_TextDiagram),
   Etok(Rep_TextText),
   Etok(Rep_TextFullDiagram),
   Etok(Rep_xListText),
   Etok(Rep_xListScrolledlist),
   Etok(Rep_xListScrolledbuttons),
   Etok(Rep_MultiConnection),
   Etok(Rep_SmtText),
   Etok(Rep_Tree),
   Etok(Rep_Grid),
   Etok(Rep_ERD),
   Etok(Rep_xListRowColumn),
   Etok(Rep_DGraph),
   Etok(Rep_OODT_Inheritance),
   Etok(Rep_OODT_Relations),
   Etok(Rep_OODT_Scope),
   Etok(Rep_DataChart),
   Etok(Rep_SubsystemMap),
   Etok(Rep_RawText),
   Etok(Rep_ExternalFile),
   Etok(Rep_ExternalFile_I),
   Etok(Rep_ExternalFile_X),
   Etok(Rep_List),
   Etok(Rep_Menu),
   Etok(Rep_Last)			/* Keep this last please ... */
};
genENUM_prt(repType);

genENUM(appType) {
   Etok(App_AST),   
   Etok(App_STE),
   Etok(App_RTL),
   Etok(App_SMT),
   Etok(App_DD),
   Etok(App_SUBSYS),
   Etok(App_SCRAPBOOK),
   Etok(App_PROJECT),
   Etok(App_UFILE),
   Etok(App_DGRAPH),
   Etok(App_XREF),
   Etok(App_KEYWORD),
   Etok(App_ASSOCTYPE),
   Etok(App_ASSOCINSTANCE),
   Etok(App_RAW),
   Etok(App_UDIR),
   Etok(App_UDIR_TREE),
   Etok(App_STYLE),
   Etok(App_OODT_RELATION),

   Etok(App_EXT),
   Etok(App_EXT_I),
   Etok(App_EXT_X),
   Etok(App_LAST)			/* Implemented for Style Handling */
};
genENUM_prt(appType);

genENUM(fileLanguage) {
    Etok_val(FILE_LANGUAGE_UNKNOWN,-1),

    Etok(FILE_LANGUAGE_C),		/* These next     */
    Etok(FILE_LANGUAGE_CPP),		/* four constants */
    Etok(FILE_LANGUAGE_FORTRAN),	/* correspond to  */
    Etok(FILE_LANGUAGE_COBOL),          /* smtLanguage.   */

    Etok(FILE_LANGUAGE_STE),
    Etok(FILE_LANGUAGE_RAW),
    Etok(FILE_LANGUAGE_MAKEFILE),

    Etok(FILE_LANGUAGE_EXT),
    Etok(FILE_LANGUAGE_EXT_I),
    Etok(FILE_LANGUAGE_EXT_X),

    Etok(FILE_LANGUAGE_ESQL_C),		/* These two constants */
    Etok(FILE_LANGUAGE_ESQL_CPP),	/* also are smtLanguages */
    Etok(FILE_LANGUAGE_ELS),

    Etok(FILE_LANGUAGE_LAST)
};
genENUM_prt(fileLanguage);

/*
   START-LOG-------------------------------------------

   $Log: representation.h  $
   Revision 1.10 2000/04/17 10:43:39EDT ktrans 
   Periodic merge from message branch:
   clean up virtual function hierarchies
   remove dormant code (including hyperlinks/help)
   etc.
   For details for this file, see contributor version on stream_message branch.
 * Revision 1.2.1.16  1994/07/27  22:38:13  bakshi
 * Bug track: n/a
 * epoch list project
 *
 * Revision 1.2.1.15  1994/05/23  14:34:32  azaparov
 * Added new types for debugger
 *
 * Revision 1.2.1.14  1994/01/21  00:31:15  kws
 * New rtl display
 *
 * Revision 1.2.1.13  1993/12/31  19:39:02  boris
 * Bug track: Test Validation
 * Test Validation
 *
 * Revision 1.2.1.12  1993/12/17  18:00:21  himanshu
 * *** empty log message ***
 *
 * Revision 1.2.1.11  1993/12/03  17:12:53  himanshu
 * Bug track: new code
 * added new member Rep_ExternalFile to repType enum type
 *
 * Revision 1.2.1.10  1993/11/03  04:08:09  himanshu
 * Bug track: 5166
 * Added a new type Rep_ExternalFiel to indicate a new 'External
 *
 * Revision 1.2.1.9  1993/01/20  00:15:10  wmm
 * Add App_OODT_RELATION.
 *
 * Revision 1.2.1.8  1993/01/05  20:29:59  boris
 * added App_STYLE app type
 *
 * Revision 1.2.1.7  1992/12/17  23:34:11  sharris
 * Added 2 new App type for query search -- App_UDIR and App_UDIR_TREE
 *
 * Revision 1.2.1.6  1992/12/11  22:10:17  boris
 * y
 *
 * Revision 1.2.1.5  1992/12/08  21:11:32  glenn
 * Add Rep_DataChart, Rep_SubsystemMap
 *
 * Revision 1.2.1.4  1992/12/08  20:47:42  glenn
 * Add Rep_UNKNOWN
 *
 * Revision 1.2.1.3  1992/11/30  22:48:19  kws
 * Added 2 new app types ASSOCTYPE, and ASSOCINSTANCE
 *
 * Revision 1.2.1.2  1992/10/09  18:58:22  boris
 * Fix comment
 *
   END-LOG---------------------------------------------
*/

#endif /* _representation_h */
