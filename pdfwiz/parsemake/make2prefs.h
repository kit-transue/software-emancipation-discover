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
// make2prefs.h
// conversion of makefile to preferences file
// includes generation of pdf
// 17.feb.98 kit transue

#if !defined(_MAKE2PREFS_H_)
#define _MAKE2PREFS_H_

#include <list>
using namespace std;

#include "generate_pdf.h"
#include "make_toplevel.h"
#include "prefsfile.h"

class prefsroot {
public:
	prefsroot();
	virtual ~prefsroot();

	// access
	string sharedModelRoot() const;
	string sharedSrcRoot() const;
	string flagsDB() const;
	string pdfFileBuild() const;
	string pdfFileUser() const;
	string prefsFile() const;
	string projectList() const;
	string const & projectName() const;
	string suggestedProjectName() const;
	// really wish this returned a reference, but mutable's worse:
	string const adminDir() const;

	// actions:
	void addMakefile(makefile const &, string const *target = 0);
	void addSubProject(subproj *);
	void write();
	void setProjectName(string const &);
	void setAdminDir(string const &);


private:
	// support stuff, should be moved elsewhere:
	void mkAdmindirStructure();
	bool createDir(string const &dir);

	// generate members:
	void create_prefs();
	// members:
	prefsfile prefs;
	string const &admindir();
	string _admindir;
	project * pdf;

	typedef list <makefile const *> mflist_type;
	mflist_type mflist;
};
#endif //!defined(_MAKE2PREFS_H_)

