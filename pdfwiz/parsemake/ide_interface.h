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
// ide_interface.h
// 11.dec.97 Kit Transue
// access to IDE settings

#if !defined(_IDE_INTERFACE_H)
#define _IDE_INTERFACE_H

#pragma warning(disable:4786)

#include <string>
#include <vector>
using namespace std;

class ide_interface
{
public:
	// static types and class memebers:
	typedef vector<string> stringvec;
	static ide_interface * get_platform_interface();

	// construct/copy/destroy
	virtual ~ide_interface();
	// access
	virtual stringvec const & last_projects() = 0;
	virtual string standard_includes() = 0;
	virtual string standard_defines() = 0;
	virtual string standard_flags() = 0;
};

class vc5_ide : public ide_interface
{
public:
	virtual stringvec const & last_projects();
	virtual string standard_includes();
	virtual string standard_defines();
	virtual string standard_flags();
private:
	stringvec lps;
};

class vc4_ide : public ide_interface
{
public:
	virtual stringvec const & last_projects();
	virtual string standard_includes();
	virtual string standard_defines();
	virtual string standard_flags();
private:
	stringvec lps;
};

#endif // defined _IDE_INTERFACE_H
