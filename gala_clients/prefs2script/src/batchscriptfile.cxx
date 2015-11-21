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
#include "batchscriptfile.h"
#include "cstring.h"

BatchScriptFile::BatchScriptFile()
{
	mCharData="";
	mCommentString="REM ";
	mpMySepString="\\";
	mEchoString="echo ";
	EchoCommands(0); //defaults to echo off.
	mpMyProtectString="\"";
	Comment((const char *)MSG1);
}
BatchScriptFile::~BatchScriptFile()
{
}

void BatchScriptFile::EchoCommands(int state)
{
	if (state)
		mCharData+= NL+ "echo on"+NL;
	else
		mCharData+=NL+ "echo off"+NL;
}

void BatchScriptFile::Echo(const char * text)
{
	Echo(cstring(text));
	return;
}

void BatchScriptFile::SetEnvironmentVariable(const char * name, const char *value)
{
	SetEnvironmentVariable(cstring(name), cstring(value));
	return;
}



//protected methods..

void BatchScriptFile::SetEnvironmentVariable(cstring &name, cstring &value)
{
	//mCharData+= NL + "set " + name + "=" + value +NL;
	mCharData+= NL + "set ";
	mCharData+= name + "=";
	mCharData+= value + NL;

	return;
}



void BatchScriptFile::Echo(cstring & text)
{
	mCharData+= NL + mEchoString  + text +  NL;
}

