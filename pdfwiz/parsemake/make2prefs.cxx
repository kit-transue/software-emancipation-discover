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
// make2prefs.cxx
// implementation of writing preferences file
// including generation of pdf

#pragma warning(disable:4786)

#include <direct.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <cassert>

#include "message.h"
#include "setech_utils.h"
#include "filename.h"

#include "make2prefs.h"
#include "make_toplevel.h"
#include "pathmgr.h"

// construct/copy/destroy
//prefsroot::prefsroot(makefile const &mf, string const *target /* =0 */) : pdf(0)
prefsroot::prefsroot() : pdf(0)
{
	//addMakefile(mf, target);

}

prefsroot::~prefsroot()
{
	if (pdf)
		delete pdf;
}


// access:
string
prefsroot::sharedModelRoot() const
{
	return adminDir() + "/model";
}

string
prefsroot::sharedSrcRoot() const
{
	return pdf->srcroot();
}

string
prefsroot::flagsDB() const
{
	return adminDir() + "/flags/flags.db";
}

string
prefsroot::pdfFileBuild() const
{
	return adminDir() + "/pdf/build.pdf";
}

string
prefsroot::pdfFileUser() const
{
	return adminDir() + "/pdf/user.pdf";
}

string
prefsroot::prefsFile() const
{
	return adminDir() + "/prefs/build.prefs";
}

string
prefsroot::projectList() const
{
	return pdf->projectList();
}

void
prefsroot::create_prefs()
{
	prefs.set_name(prefsFile());

	// open prototype:
	string prototype(setech_utils::psethome());
	prototype += "/prebuilt/template/prefs/template.prefs";
	prefs.read(prototype);

	prefs.insert("pdfFileBuild", pdfFileBuild());
	prefs.insert("pdfFileUser", pdfFileUser());
	prefs.insert("defaultPDF", pdfFileUser());

	//prefs.insert("cDefines"
	//prefs.insert("cPlusDefines"
	//prefs.insert("cIncludes"
	//prefs.insert("cPlusIncludes"
	if (!prefs.at("cCompiler"))
		prefs.insert("cCompiler", "cl.exe");
	if (!prefs.at("cPlusCompiler"))
		prefs.insert("cPlusCompiler", "cl.exe");

	prefs.insert("projectList", projectList());
	prefs.insert("projectListHeaders", projectList());
	prefs.insert("ADMINDIR", string("\"") + adminDir() + string("\""));
	prefs.insert("projectHome", string("/") + pdf->name());
	prefs.insert("sharedModelRoot", string("\"") + sharedModelRoot() + string("\""));
	prefs.insert("sharedSrcRoot", string("\"") + pdf->srcroot() + string("\""));
	prefs.insert("privateModelRoot", string("\"") + adminDir() + "/model/%USER%" + string("\""));
	prefs.insert("privateSrcRoot", string("\"") + pdf->srcroot() + string("\""));
}

string const &
prefsroot::admindir()
{
	if (_admindir.empty()) {
		_admindir = setech_utils::psethome();
		_admindir += "/models/";
		_admindir += projectName();
	}
	return _admindir;
}

// actions:
void
prefsroot::addMakefile (makefile const &mf, string const *target /* = 0 */)
{
	mflist.push_back(&mf);
#if 0	// pdf management is handled later, after scoping calculated.
	// create pdf if necessary:
	if (!pdf) {
		pdf = new project();
		setProjectName(mf.name());
	}

	add_mftarget2proj(*pdf, mf, *target);
#endif
}

void
prefsroot::addSubProject(subproj * p)
{
	assert(p);
	// create pdf if necessary:
	if (!pdf) {
		pdf = new project();
		setProjectName(p->project_name());
	}

	pdf->add_subproj(p);
}

void
prefsroot::write()
{
	if (!pdf) {
		msgid("Preferences file contains empty PDF--not written") << eom;
		return;
	}
	assert(pdf);
	// ask the PDF to get its SharedSrcRoot
	pdf->set_srcroot();

	// create ADMINDIR structure if necessary
	mkAdmindirStructure();

	// create, then write prefs
	create_prefs();
	prefs.write();

	// write pdf:
	ofstream pdfstream(pdfFileBuild().c_str(), ios::out);
	if (!pdfstream.good()) {
		msgid("Error opening PDF for writing") << pdfFileBuild() << eom;
	}
	else {
		// write body
		pdf->write(&pdfstream);
		// write rules
		pdf->write_rules(&pdfstream, sharedModelRoot(), flagsDB());
		// cleanup
		if (!pdfstream.good()) {
			msgid("Error while writing PDF") << pdfFileBuild() << eom;
		}
		pdfstream.close();
	}

	// write flags database
	ofstream flagsstream(flagsDB().c_str(), ios::out);
	if (!flagsstream.good()) {
		msgid("Error opening flags database for writing") << flagsDB() << eom;
	}
	else {
		assert(prefs.at("cCompiler"));	// set in create_prefs()
		mflist_type::iterator mfi = mflist.begin();
		while (mfi != mflist.end()) {
			// write format version
			flagsstream << (*mfi)->flags_db_version() << endl;
			(*mfi)->write_flags_database(&flagsstream, prefs.at("cCompiler"));
			++mfi;
		}
		if (!flagsstream.good()) {
			msgid("Error while writing flags database") << flagsDB() << eom;
		}
		flagsstream.close();
	}

	// write flists for each project
	if (!pdf->write_flist_to_admindir(admindir())) { 
		msgid("Error while writing flists") << admindir() << eom;
	}
}


bool
prefsroot::createDir(string const &dir)
{
	// should really live somewhere else
	struct _stat buf;
	bool success = true;
	filename d(dir);
	if (_stat(d.name().c_str(), &buf) != 0) {
		assert(filename::is_absolute_path(dir));
		if (filename::is_root(dir))	// can't make root
			success = false;
		else {
			// make sure parent exists:
			success = createDir(d.dirname());
			if (success) {
				success = (_mkdir(d.name().c_str()) == 0);
			}
		}
	}
	else
		success = buf.st_mode & _S_IFDIR;
	return success;
}

void
prefsroot::mkAdmindirStructure()
{
	// confirm that directory exists...
	// and if not, that its parent does...
	bool success = createDir(admindir());
	if (!success) {
		msgid("Error creating directory for admindir") << admindir() << eom;
	}
	else {
		// create directory structure as necessary
		if (success)
			success = createDir(admindir() + "/prefs");
		if (success)
			success = createDir(admindir() + "/pdf");
		if (success)
			success = createDir(admindir() + "/log");
		if (success)
			success = createDir(admindir() + "/flags");
		if (success)
			success = createDir(admindir() + "/tmp");
		if (success)
			success = createDir(admindir() + "/model");
		if (success)
			success = createDir(admindir() + "/bin");
		if (!success) {
			msgid("Error creating admindir subdirectory") << admindir() << eom;
		}
	}
}

void
prefsroot::setAdminDir(string const & s)
{
	directory_vector default_dirs;
	if (!filename::is_absolute_path(s)) {
		_admindir = default_dirs.relative_to_absolute(s);
	}
	else {
		_admindir = s;
	}
}

void
prefsroot::setProjectName(string const &s)
{
	assert(pdf);
	pdf->set_name(s);
}

string const &
prefsroot::projectName() const
{
	assert(pdf);
	return pdf->name();
}

string
prefsroot::suggestedProjectName() const
{
	string ret;
	if (mflist.empty()) {
		ret = "empty";
	}
	else {
		filename fn((*mflist.begin())->name());
		ret = fn.basename();
	}
	return ret;
}


string const
prefsroot::adminDir() const
{
	string ret(_admindir);
	if (_admindir.empty()) {
		ret = setech_utils::psethome();
		ret += "/models/";
		ret += projectName();
	}
	return ret;
}
