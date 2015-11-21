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
// See descriptive comment in parser.C.

#include "parseline.h"
#include "ctype.h"
using namespace std;

// ===========================================================================
bool ParseLine::parseLine(Line &line)
{
    tracker isOk(*this);
    nameLine(line) || poundLine(line)
		   || bangLine(line)
		   || emptyLine(line)
		   || miscLine(line)
		   || isOk.fail();
    char ch;
    // Ensure no leftover characters.
    if (isOk && peek(ch)) {
	isOk.fail();
    }
    return isOk;
}

// ===========================================================================
static bool isOfLine(char ch) {
    return (ch != '\n' && ch != '\r');
}

// ===========================================================================
bool ParseLine::nameLine(Line &line)
{
    tracker isOk(*this);
    string n;
    word(n) || isOk.fail();
    if (isOk) {
	parseChar('=') || isOk.fail();
    }
    string val;
    if (isOk) {
	parseSatisfying(val, &isOfLine);
	line.setKind(Line::Definition);
	line.setKey(n);
	line.setString(val);
    }
    return isOk;
}

// ===========================================================================
static bool parseToken_helper(ParseLine &par, Token *result) {
    return par.parseToken(*result);
}

// ===========================================================================
bool ParseLine::bangLine(Line &line)
{
    tracker isOk(*this);
    parseChar('!') || isOk.fail();
    string directive;
    if (isOk) {
	word(directive) || isOk.fail();
    }
    if (isOk) {
	whitespace() || isOk.fail();
    }
    if (isOk) {
	if (directive == "MESSAGE") {
	    string str;
	    parseSatisfying(str, &isOfLine);
	    line.setKind(Line::Message);
	    line.setString(str);
	}
	else if (directive == "IF") {
	    list<Token> tks;
	    many(&parseToken_helper, tks);
	    line.setKind(Line::If);
	    line.setList(tks);
	}
	else if (directive == "ELSEIF") {
	    list<Token> tks;
	    many(&parseToken_helper, tks);
	    line.setKind(Line::Elseif);
	    line.setList(tks);
	}
	else if (directive == "ELSE") {
	    line.setKind(Line::Else);
	}
	else if (directive == "ENDIF") {
	    line.setKind(Line::Endif);
	}
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::emptyLine(Line &line)
{
    tracker isOk(*this);
    char ch;
    if (peek(ch) && isOfLine(ch)) {
	isOk.fail();
    }
    if (isOk) {
	line.setKind(Line::Empty);
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::miscLine(Line &line)
{
    tracker isOk(*this);
    list<Token> tks;
    many(&parseToken_helper, tks);
    line.setKind(Line::Misc);
    line.setList(tks);
    return isOk;
}

// ===========================================================================
bool ParseLine::poundLine(Line &line)
{
    tracker isOk(*this);
    (parseChar('#') && parseChar(' ')) || isOk.fail();
    if (isOk) {
	poundName(line) || doNot(line) || isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::poundName(Line &line)
{
    tracker isOk(*this);
    string key;
    word(key) || isOk.fail();

    if (isOk) {
	whitespace(false);
	if (key == "Begin") {
	    Entity ent;
	    line.setKind(Line::Begin);
	    entity(ent) || isOk.fail();
	    if (isOk) {
		line.setEntity(ent);
	    }
	}
	else if (key == "End") {
	    Entity ent;
	    line.setKind(Line::End);
	    entity(ent) || isOk.fail();
	    if (isOk) {
		line.setEntity(ent);
	    }
	}
	else if (key == "Name") {
	    line.setKind(Line::Name);
	    string nm;
	    quotedString(nm) || isOk.fail();
	    line.setString(nm);
	}
	else if (key == "ADD") {
	    poundDef(Line::Add, line) || isOk.fail();
	}
	else if (key == "SUBTRACT") {
	    poundDef(Line::Subtract, line) || isOk.fail();
	}
	else if (key == "PROP") {
	    poundDef(Line::Prop, line) || isOk.fail();
	}
	else if (key == "TARGTYPE") {
	    string target;
	    quotedString(target) || isOk.fail();
	    if (isOk) {
		list<Token> toklist;
		if (many(&parseToken_helper, toklist)) {
		    line.setList(toklist);
		    line.setKind(Line::Targtype);
		    line.setString(target);
		}
		else {
		    isOk.fail();
		}
	    }
	}
	else if (key == "Microsoft") {
	    ms1(line) || ms2(line) || isOk.fail();
	}
	else {
	    isOk.fail();
	}
    }
    return isOk;
}

// ===========================================================================
ParseLine::ParseLine(const char *input) 
  : Parser(input)
{
}

// ===========================================================================
template<typename result_t>
bool ParseLine::many(bool (*par)(ParseLine &, result_t *), std::list<result_t> &ls)
{
    ls.clear();
    for (;;) {
	result_t result;
	if (!par(*this, &result)) {
	    break;
	}
	ls.push_back(result);
    }
    return true;
}

// ===========================================================================
bool ParseLine::parseToken(Token &token)
{
    tracker isOk(*this);
    string str;
    if (whitespace()) {
	token.setKind(Token::Whitespace);
    }
    else if (word(str)) {
	token.setKind(Token::Word);
	token.setString(str);
    }
    else if (quotedString(str)) {
	token.setKind(Token::StringLiteral);
	token.setString(str);
    }
    else if (parseDelimiter(str)) {
	token.setKind(Token::Delimiter);
	token.setString(str);
    }
    else {
	isOk.fail();
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::doNot(Line &line)
{
    bool ok = matchChars("** DO NOT EDIT **");
    if (ok) {
	line.setKind(Line::DoNot);
    }
    return ok;
}

// ===========================================================================
bool ParseLine::entity(Entity &entity)
{
    tracker isOk(*this);
    entitySource(entity) || entityTarget(entity) || entityProject(entity)
		         || entityGroup(entity)  || entitySpecial(entity)
			 || isOk.fail();
    return isOk;
}

// ===========================================================================
bool ParseLine::entitySource(Entity &entity)
{
    tracker isOk(*this);
    matchWord("Source") || isOk.fail();
    if (isOk) {
	whitespace();
    }
    if (isOk) {
	matchWord("File") || isOk.fail();
    }
    if (isOk) {
	entity.setKind(Entity::SourceFile);
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::entityTarget(Entity &entity)
{
    tracker isOk(*this);
    matchWord("Target") || isOk.fail();
    if (isOk) {
	entity.setKind(Entity::Target);
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::entityProject(Entity &entity)
{
    tracker isOk(*this);
    matchWord("Project") || isOk.fail();
    if (isOk) {
	entity.setKind(Entity::Project);
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::entityGroup(Entity &entity)
{
    tracker isOk(*this);
    matchWord("Group") || isOk.fail();
    if (isOk) {
	whitespace(false);
    }
    string str;
    bool have_name = quotedString(str);
    if (have_name) {
	entity.setKind(Entity::GroupName);
	entity.setString(str);
    }
    else {
	entity.setKind(Entity::Group);
    }
    return isOk;
}

// ===========================================================================
// Begin/End Special Build Tool
//
bool ParseLine::entitySpecial(Entity &entity)
{
    tracker isOk(*this);
    matchWord("Special") || isOk.fail();
    if (isOk) {
	whitespace(false);
	matchWord("Build") || isOk.fail();
    }
    if (isOk) {
	whitespace(false);
	matchWord("Tool") || isOk.fail();
    }
    if (isOk) {
	entity.setKind(Entity::SpecialBuildTool);
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::poundDef(Line::Kind kind, Line &line)
{
    tracker isOk(*this);
    string n1;
    string defName;
    string definition;
    word(n1) || isOk.fail();
    bool is_base = isOk && n1 == "BASE";
    if (isOk) {
	if (is_base) {
	    whitespace() || isOk.fail();
	    if (isOk) {
		word(defName) || isOk.fail();
	    }
	}
	else {
	    defName = n1;
	}
    }
    if (isOk) {
	whitespace() || isOk.fail();
    }
    if (isOk) {
	parseSatisfying(definition, &isOfLine);
	line.setKind(kind);
	line.setKey(defName);
	line.setBool(is_base);
	line.setString(definition);
    }
    return isOk;
}

// ===========================================================================
static bool isNonEq(char ch) {
    return (ch != '=' && ch != '\n' && ch != '\r');
}

// ===========================================================================
bool ParseLine::param(Line &line)
{
    tracker isOk(*this);
    whitespace(false);
    (parseChar('-') && parseChar(' ')) || isOk.fail();
    string defName;
    if (isOk) {
	parseSatisfying(defName, &isNonEq) || isOk.fail();
    }
    if (isOk) {
	parseChar('=') || isOk.fail();
    }
    string defVal;
    if (isOk) {
	quotedString(defVal) || bracketedString(defVal) || isOk.fail();
    }
    if (isOk) {
	if (defName == "Name") {
	    line.setName(defVal);
	}
	else if (defName == "Package Owner") {
	    line.setOwner(defVal);
	}
    }
    return isOk;
}

// ===========================================================================
bool ParseLine::ms1(Line &line)
{
    tracker isOk(*this);
    matchChars("Developer Studio Project File") || isOk.fail();
    if (isOk) {
	whitespace() || isOk.fail();
    }
    if (isOk) {
	line.setKind(Line::Project);
	for (;;) {
	    if (!param(line)) {
		break;
	    }
	}
    }
    return isOk;
}

// ===========================================================================
static bool isNonSpace (char ch) {
    return (!isspace(ch));
}

// ===========================================================================
bool ParseLine::ms2(Line &line)
{
    tracker isOk(*this);
    matchChars("Developer Studio Generated Build File, Format Version") || isOk.fail();
    if (isOk) {
	whitespace() || isOk.fail();
    }
    if (isOk) {
	string n;
	parseSatisfying(n, &isNonSpace);
	line.setKind(Line::Version);
	line.setString(n);
    }
    return isOk;
}
