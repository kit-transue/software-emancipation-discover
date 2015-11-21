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
#include "token.h"

using namespace std;

// ===========================================================================
Token::Token()
: theKind(NoKind)
{
}

// ===========================================================================
void Token::setKind(Kind k)
{
    theKind = k;
}

// ===========================================================================
void Token::setString(const std::string &str)
{
    theString = str;
}

// ===========================================================================
enum Token::Kind Token::kind() const
{
    return theKind;
}

// ===========================================================================
const std::string& Token::string() const
{
    return theString;
}

// ===========================================================================
void Token::show(ostream &output) const
{
    switch(theKind) {
    case NoKind:
	{
	    output << "*** NoKind ***";
	}
        break;
    case Whitespace:
	{
	    output << "Whitespace";
	}
        break;
    case Word:
	{
	    output << "Word";
	}
        break;
    case Delimiter:
	{
	    output << "Delimiter";
	}
        break;
    case StringLiteral:
	{
	    output << "StringLiteral";
	}
        break;
    default:
	{
	    output << "*** ??? ***";
	}
	break;
    }
    switch(theKind) {
    case Word:
    case Delimiter:
    case StringLiteral:
	{
	    output << "=" << theString;
	}
        break;
    default:
	break;
    }

}

