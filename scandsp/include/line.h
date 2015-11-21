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
#ifndef LINE_H
#define LINE_H

#include <string>
#include <list>
#include <iostream>
#include "token.h"
#include "entity.h"

class Line {
public:
    enum Kind {NoKind, Definition, Begin, End, Name, Add, Subtract, Prop, 
	       Targtype, Microsoft, Project, Misc, Pound, Empty, Message,
	       Version, DoNot, If, Elseif, Else, Endif};

    Line();
    Line(const Line &);
    Line &operator =(const Line &);
    ~Line();
    void setKind(Kind);
    void setString(const std::string &);
    void setList(const std::list<Token> &);
    void setEntity(const Entity &);
    void setKey(const std::string &);
    void setBool(bool);
    void setName(const std::string &);
    void setOwner(const std::string &);

    enum Kind kind() const;
    const std::string& getString() const;
    const std::list<Token>& tokens() const;
    const Entity &entity() const;
    const std::string &key() const;
    bool getBool() const;
    const std::string *getName() const;
    const std::string *getOwner() const;

    void show(std::ostream &) const;

private:
    enum Kind theKind;
    std::string theString;
    std::list<Token> theTokens;
    std::string theKey;
    bool theBool;
    Entity theEntity;
    std::string *theName;
    std::string *theOwner;
};

#endif // LINE_H
