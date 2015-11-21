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

#ifndef _rtlCommon_h
#define _rtlCommon_h

// Note - These values are checked to see if they could be printed 
//      as an ASCII character.

enum iconKind {
    iconUnknown = 0x80,
    iconFolder,
    iconHomeFolder,
    iconList,
    iconDocument,
    iconCSource,
    iconCppSource,
    iconFunction,
    iconMacro,
    iconClass,
    iconSubsystem,
    iconUnion,
    iconTypedef,
    iconEnum,
    iconVariable,
    iconMember,
    iconGroup,
    iconScratchGroup,
    iconWrite15,
    iconWrite16,
    iconWrite17,
    iconWrite18,
    iconWrite19,
    iconWrite20,
    iconWrite21,
    iconWrite22,
    iconWrite23,
    iconWrite24,
    iconWrite25,
    iconWrite26,
    iconInstance,
    iconString,
    iconLocal,
    iconSQL_C,
    iconSQL_CPlusPlus,
    iconDefect,
    iconJavaSource,
    iconInterface,
    iconPackage,
    iconComponent,
    iconNumKinds
};

#define rtlUnknown	0
#define rtlNormal	1
#define rtlProject	2
#define rtlInstance	3
#define rtlTopProject	4

#endif // _rtlCommon_h
