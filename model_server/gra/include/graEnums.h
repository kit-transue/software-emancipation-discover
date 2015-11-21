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
#ifndef _graEnums_h
#define _graEnums_h

enum graSymbolTypes {
    graSymbolTypeBox = 0L,
    graSymbolTypeCircle,
    graSymbolTypeOval,
    graSymbolTypeFilletBox,
    graSymbolTypeFolder,
    graSymbolTypeShadowBox,
    graSymbolTypeShadowCircle,
    graSymbolTypeShadowOval,
    graSymbolTypeShadowFilletBox,
    graSymbolTypeShadowFolder
};

enum graLayerNo {
    graLayerNoBase,
    graLayerNoNormal,
    graLayerNoExpanded,
    graLayerNoCollapsed,
    graLayerNoDecorationLL,
    graLayerNoDecorationCL,
    graLayerNoDecorationUL,
    graLayerNoDecorationLC,
    graLayerNoDecorationUC,
    graLayerNoDecorationLR,
    graLayerNoDecorationCR,
    graLayerNoDecorationUR,
    graLayerNoHilited,		// hilite, selected should be in this order
    graLayerNoSelected,
    graLayerNoTextHilight,	// Text Hilight layer no
    graLayerNoBreakMarker,
    graLayerNoDisabledBreakMarker,
    graLayerNoHardAssocMarker,
    graLayerNoExecutionMarker,
    graLayerNoErrorMarker,
    graLayerNoSearchMarker,
    graLayerNoNote,
    graLayerNoHyperMarker,
    graLayerNoCursor = 30,      // Must be the second-last one.
    graLayerNoMAX		// Must be the last (greatest) one
};

#define GRA_MAX_MODES 32

#define GRA_APPNODE_LAYERS              0x7F8000
enum graSymbolModes {
    graSymbolModeBase = 		0x000001L,
    graSymbolModeNormal = 		0x000002L,
    graSymbolModeExpanded = 		0x000004L,
    graSymbolModeCollapsed =	 	0x000008L,
    graSymbolModeDecorationLL =		0x000010L,
    graSymbolModeDecorationCL =		0x000020L,
    graSymbolModeDecorationUL =		0x000040L,
    graSymbolModeDecorationLC =		0x000080L,
    graSymbolModeDecorationUC =		0x000100L,
    graSymbolModeDecorationLR =		0x000200L,
    graSymbolModeDecorationCR =		0x000400L,
    graSymbolModeDecorationUR =		0x000800L,
    graSymbolModeHilited = 		0x001000L,
    graSymbolModeSelected = 		0x002000L,
    graSymbolModeTextHilited =          0x004000L,
    graSymbolModeBreakMarker =		0x008000L,
    graSymbolModeDisabledBreakMarker =	0x010000L,
    graSymbolModeHardAssocMarker = 	0x020000L,
    graSymbolModeExecutionMarker = 	0x040000L,
    graSymbolModeErrorMarker = 		0x080000L,
    graSymbolModeSearchMarker = 	0x100000L,
    graSymbolModeNoteMarker =           0x200000L,
    graSymbolModeHyperMarker =		0x400000L,
    graSymbolModeCursor =		0x80000000L // must be the last
};

enum graTextPositions {
    graTextModeUnknown,
    graTextModeLeft,
    graTextModeRight,
    graTextModeTop,
    graTextModeBottom,
    graTextModePositioned,
    graTextModeInside,
    graTextModeSuppressed,
    graTextModeInsideNoBox
};

enum graAttachmentPoint {
    graAttachLL,
    graAttachCL,
    graAttachUL,
    graAttachLC,
    graAttachUC,
    graAttachLR,
    graAttachCR,
    graAttachUR,
    graNumAttachmentPoints
};

#endif // _graEnums_h

/*
// graEnums.h
//------------------------------------------
// synopsis:
// Enumerated types used in gra module.
// This file can be included in places where you need
// the enumerations, but not the entire gra (read
// InterViews) type hierarchy.
//------------------------------------------
// $Log: graEnums.h  $
// Revision 1.3 1997/08/13 15:44:21EDT twight 
// Bug track: 14378
// Reviewed by: pero
// Removed environment variable checks from getIntPref and getStringPref
// Moved preference files from interface/src to machine/prefs
// Added preferences to pmod_server
// Convert environment variables into DIS_internal preferences.
 * Revision 1.2.1.4  1992/12/10  19:14:08  smit
 * Fix value of appnodes
 *
 * Revision 1.2.1.3  1992/12/09  23:42:37  smit
 * *** empty log message ***
 *
 * Revision 1.2.1.2  1992/10/09  18:52:25  builder
 * fixed rcs heaqer
 *
 * Revision 1.2.1.1  92/10/07  20:41:52  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:41:51  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:30  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:56:09  smit
 * Initial revision
 * 
// Revision 1.9  92/09/23  15:23:08  jon
// added hyper marker stuff and a mask for the category layers
// 
// Revision 1.8  92/07/30  11:56:48  ranger
// Added note graphic
// 
// Revision 1.7  92/07/07  14:37:59  glenn
// Add graLayerNoMAX.
// 
// Revision 1.6  92/06/26  16:01:55  jont
// comma!
// 
// Revision 1.5  92/06/26  15:54:42  jont
// added layers for error and search markers
// 
// Revision 1.4  92/05/27  19:06:23  jont
// added execution marker layer
// 
// Revision 1.3  92/05/20  07:29:07  jont
// Add HardAssocMarker layer.
// 
// Revision 1.2  92/05/13  20:28:48  smit
// More of debugger support
// 
// Revision 1.1  92/03/27  11:07:51  glenn
// Initial revision
// 
//------------------------------------------
*/

