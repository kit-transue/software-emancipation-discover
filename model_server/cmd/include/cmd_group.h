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
#include <genEnum.h>

genENUM(CMD_VALIDATION_GROUP)
{
   Etok(CMD_EVERYTHING),// total validation
   Etok(CMD_SMT),       // smt-tree: hierarchical,  incremental editiong
   Etok(CMD_SMOD),
   Etok(CMD_DD),
   Etok(CMD_PMOD),
   Etok(CMD_MERGE),
   Etok(CMD_STE),
   Etok(CMD_CONFIG),
   Etok(CMD_DEBUG),
   Etok(CMD_OODT),
   Etok(CMD_ASSOC),
   Etok(CMD_FLOWCHART),
   Etok(CMD_PROJ),
   Etok(CMD_IMPACT),
   Etok(CMD_SKETCH),
   Etok(CMD_VIEWER),
   Etok(CMD_MDMS),
   Etok(CMD_EDITOR),
   Etok(CMD_NOTHING)      // no validation: for demos, logging
};

genENUM_prt(CMD_VALIDATION_GROUP);


