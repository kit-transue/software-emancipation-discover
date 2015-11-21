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
#include <string>

using namespace std;

#include "line.h"

// ===========================================================================
Line::Line() : theKind(NoKind), theName(NULL), theOwner(NULL) {}

// ===========================================================================
Line::Line(const Line &l) : theKind(l.theKind),
			    theName(l.theName ? new string(*l.theName) : NULL),
			    theOwner(l.theOwner ? new string(*l.theOwner) : NULL),
			    theString(l.theString),
			    theTokens(l.theTokens),
			    theKey(l.theKey),
			    theBool(l.theBool),
			    theEntity(l.theEntity)
{
}

// ===========================================================================
Line &Line::operator =(const Line &l)
{
    if (this != &l) {
	theKind = l.theKind;
	delete theName;
	theName = theName ? new string(*l.theName) : NULL;
	delete theOwner;
	theOwner = theOwner ? new string(*l.theOwner) : NULL;
	theString = l.theString;
	theTokens = l.theTokens;
	theKey = l.theKey;
	theBool = l.theBool;
	theEntity = l.theEntity;
    }
    return *this;
}

// ===========================================================================
Line::~Line()
{
    delete theName;
    delete theOwner;
}

// ===========================================================================
void Line::setKind(Kind k) {
    theKind = k;
}

// ===========================================================================
void Line::setString(const std::string &str)
{
    theString = str;
}

// ===========================================================================
void Line::setList(const std::list<Token> &toks)
{
    theTokens = toks;
}

// ===========================================================================
void Line::setEntity(const Entity &entity)
{
    theEntity = entity;
}

// ===========================================================================
void Line::setKey(const std::string &str)
{
    theKey = str;
}

// ===========================================================================
void Line::setBool(bool b)
{
    theBool = b;
}

// ===========================================================================
void Line::setName(const std::string &str)
{
    delete theName;
    theName = new string(str);
}

// ===========================================================================
void Line::setOwner(const string &str)
{
    delete theOwner;
    theOwner = new string(str);
}

// ===========================================================================
enum Line::Kind Line::kind() const
{
    return theKind;
}

// ===========================================================================
const std::string& Line::getString() const
{
    return theString;
}

// ===========================================================================
const std::list<Token>& Line::tokens() const
{
    return theTokens;
}

// ===========================================================================
const Entity& Line::entity() const
{
    return theEntity;
}

// ===========================================================================
const string& Line::key() const
{
    return theKey;
}

// ===========================================================================
bool Line::getBool() const
{
    return theBool;
}

// ===========================================================================
const string *Line::getName() const
{
    return theName;
}

// ===========================================================================
const string *Line::getOwner() const
{
    return theOwner;
}

// ===========================================================================
void Line::show(ostream &output) const
{
    switch(theKind) {
    case NoKind:
	{
	    output << "*** NoKind ***";
	}
        break;
    case Definition:
	{
	    output << "Definition " << theKey << "=" << theString;
	}
        break;
    case Begin:
	{
	    output << "Begin ";
            theEntity.show(output);
	}
        break;
    case End:
	{
	    output << "End ";
            theEntity.show(output);
	}
        break;
    case Name:
	{
	    output << "Name " << theString;
	}
        break;
    case Add:
	{
	    string p;
	    if (theBool) {
		p = "BASE ";
	    }
	    output << "Add " << p << theKey << "=" << theString;
	}
        break;
    case Subtract:
	{
	    string p;
	    if (theBool) {
		p = "BASE ";
	    }
	    output << "Subtract " << p << theKey << "=" << theString;
	}
        break;
    case Prop:
	{
	    string p;
	    if (theBool) {
		p = "BASE ";
	    }
	    output << "Prop " << p << theKey << "=" << theString;
	}
        break;
    case Targtype:
	{
	    output << "Targtype " << theString << " ";
	    list<Token>::const_iterator it;
	    for (it = theTokens.begin(); it != theTokens.end(); ++it) {
		output << " ";
		(*it).show(output);
	    }
	}
        break;
    case Microsoft:
	{
	    output << "Microsoft ?????";
	}
        break;
    case Project:
	{
	    output << "Project";
	    const string *n = getName();
	    if (n != NULL) {
		output << " name: " << *n;
	    }
	    const string *o = getOwner();
	    if (o != NULL) {
		output << " owner: " << *o;
	    }
	}
        break;
    case Misc:
	{
	    output << "Misc";
	    list<Token>::const_iterator it;
	    for (it = theTokens.begin(); it != theTokens.end(); ++it) {
		output << " ";
		(*it).show(output);
	    }
	}
        break;
    case Pound:
	{
	    output << "Pound ?????";
	}
        break;
    case Empty:
	{
	    output << "Empty";
	}
        break;
    case Message:
	{
	    output << "Message " << theString;
	}
        break;
    case Version:
	{
	    output << "Version " << theString;
	}
        break;
    case DoNot:
	{
	    output << "DoNot";
	}
        break;
    case If:
	{
	    output << "If ";
	    list<Token>::const_iterator it;
	    for (it = theTokens.begin(); it != theTokens.end(); ++it) {
		output << " ";
		(*it).show(output);
	    }
	}
        break;
    case Elseif:
	{
	    output << "Elseif ";
	    list<Token>::const_iterator it;
	    for (it = theTokens.begin(); it != theTokens.end(); ++it) {
		output << " ";
		(*it).show(output);
	    }
	}
        break;
    case Endif:
	{
	    output << "Endif ";
	}
        break;
    default:
	{
	    output << "*** ??? ***";
	}
	break;
    }
}
