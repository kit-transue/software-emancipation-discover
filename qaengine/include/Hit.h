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
// Hit.h: interface for the Hit class.
//
//////////////////////////////////////////////////////////////////////
#include "globals.h"
#ifndef _QA_GENERIC_HIT_
	class Hit  
	{
	public:
		string toString(void);
		string* getAstPathPtr();
		void setIndex(int index);
		int getIndex(void);
		Hit();
		Hit(string name,string module,int line,string path);
		virtual ~Hit();
		PropertiesMap* getAttributes(void);
		void setAttribute(string key, string value);
		string getAttribute(string key);
		void setAstPath(string path);
		string getAstPath(void);
		void setModule(string path);
		string getModule(void);
		void setLine(int line);
		int getLine(void);
		void setName(string name);
		string getName(void);
	private:
		int m_Index;
		string        m_Name;
		string        m_Module;
		int           m_Line;
		string        m_AstPath;
		PropertiesMap m_Attributes;
	};
	#define _QA_GENERIC_HIT_
#endif
