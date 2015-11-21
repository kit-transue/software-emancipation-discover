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
#ifndef PARSELINE_H
#define PARSELINE_H

#include <string>
#include <list>
#include "line.h"
#include "token.h"
#include "entity.h"
#include "parser.h"

// ===========================================================================
// Parser for a single line of a DSP file.
// Users of this module call only the constructor,
// then call parseLine on the object.
// Populates a 'Line' structure with the needed information.
// ===========================================================================

class ParseLine : public Parser {
public:
    ParseLine(const char *input);

    bool parseLine(Line &);
    bool nameLine(Line &);
    bool bangLine(Line &);
    bool emptyLine(Line &);
    bool miscLine(Line &);
    bool poundLine(Line &);
    bool poundName(Line &);
    bool doNot(Line &);
    bool entity(Entity &);
    bool entitySource(Entity &);
    bool entityTarget(Entity &);
    bool entityProject(Entity &);
    bool entityGroup(Entity &);
    bool entitySpecial(Entity &);
    bool poundDef(Line::Kind, Line &);
    bool parseToken(Token &);
    bool ms1(Line &);
    bool ms2(Line &);
    bool param(Line &);

    // The following should use a first parameter of type
    // bool (ParseLine::*)(result_t &), but Microsoft VC++ 6
    // flaked out when attempting to match the argument types.
    template<typename result_t>
    bool many(bool (*par)(ParseLine &, result_t *), std::list<result_t> &ls);
};

#endif // PARSELINE_H
