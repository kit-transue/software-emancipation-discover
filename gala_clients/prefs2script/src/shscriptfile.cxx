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
#include "shscriptfile.h"
#include "cstring.h"


ShScriptFile::ShScriptFile()
{
	mCharData="#!sh -v" +NL;
	mCommentString="### ";
	mpMySepString="/";
	mEchoString="echo ";
	mExportString="export ";
	Comment((const char *) MSG1);

}

ShScriptFile::~ShScriptFile()
{
}




void ShScriptFile::SetEnvironmentVariable(const char * name, const char * value)
{
	cstring myTempString(value);
	myTempString=ConvertSlashes(myTempString);

	mCharData+= NL + name;
	mCharData+= "=";
	mCharData+=myTempString;
	mCharData+= NL + mExportString;
	mCharData+=name+ NL;
	return;
}
void ShScriptFile::SetEnvironmentVariable(cstring &name, cstring &value)

{
	SetEnvironmentVariable((const char *) name, (const char *) value);
	return;
}


void ShScriptFile::Echo(const char * text)
{
	mCharData+=NL+mEchoString;
	mCharData+="\'";
	mCharData+=text;
	mCharData+= "\'" + NL;
}

void ShScriptFile::Echo(cstring & text)
{
	Echo( (const char *) text) ;
	return;
}




