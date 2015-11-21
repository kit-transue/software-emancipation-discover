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
/* This parser is based on a seat-of-the-pants understanding of 
   Microsoft Visual Studio opt files.  These files provide an adjunct to
   the dsw files which define a "workspace", and record the options which
   the user has selected within the workspace.

   Dozens of the files were examined to determine how they store
   the option which supplies the CFG environment when a DSP file is
   built.  The DSP file appears to set the CFG value, however this is
   overridden by the workspace and is recorded in its corresponding .opt file.

   The file is structured in 512-byte multiples.  The most relevant
   blocks contain the CFG values starting at an offset of 8 from the
   512-byte boundary.  In all of the .opt files examined, these blocks
   begin with either the 2-byte number 60000 or 50009.  It's tricky
   because there's usually a final block that begins the same way but
   which contains something else at offset 8.  The order of the CFG
   values matches the order of projects in the corresponding .dsw file.

   Also, at offset 1536 (3*512) there's often a block which pairs up project
   names with full pathnames of the corresponding .dsp file.  This list
   has always been present in the files which hold multiple projects,
   but it is not necessarily in the same order as the CFG blocks mentioned
   above.  It is preceded by a number telling the count of projects.
*/

// I was thinking of using the final 60000 block as a way
// to index into the preceding 60000 blocks, because it seemed to
// track the order of entries, whereas the 384 block did not.
// However, the d:\demo\test\test.* gave the lie to this theory,
// because the last 60000 block listed projects that were not in
// the DSW nor in the preceding 60000 blocks.

#pragma warning (disable:4786)

#include <string>
#include <iostream>
#include <ios>
#include <fstream>
#include <list>
#include <vector>
#include <sstream>

#include "circlebuf.h"
#include "nametracker.h"

namespace std {}
using namespace std;

// ==========================================================================
// state used by track384
//
struct T384 {
    int counting;
    NameTracker project;
    NameTracker file;
    int projsCount;
    bool need_proj_count;
};

// ==========================================================================
// state used by track60000
//
struct T60000 {
    NameTracker name;
    bool examine;
    bool had_name;
    bool had_CProject;
    bool had_last;
    int count60000;
};

// ==========================================================================
// An OptParser handles parsing a Visual Studio workspace's .opt file.
//
struct OptParser {
    OptParser();
    bool bufAt60000();
    void track60000(char ch);
    void track384(char ch);
    void track(char ch);
    void noteError(const string& message);
    void noteWarning(const string& message);
    int parse(const char *fn, vector<string> &, string &message);

    bool had_error;
    bool had_warning;
    string error_message;
    int nProjs;
    list<unsigned short> magic;
    CircleBuf circle;
    T384 t384;
    T60000 t60000;
    vector<string> *cfgsp;
};

// ==========================================================================
OptParser::OptParser()
  : had_error(false),
    had_warning(false),
    nProjs(-1),
    circle(512),
    cfgsp(NULL)
{
    magic.push_back(60000U);
    magic.push_back(50009U);

    t384.counting = -1;
    t384.projsCount = 0;
    t384.need_proj_count = false;

    t60000.examine = false;
    t60000.had_name = false;
    t60000.had_CProject = false;
    t60000.had_last = false;
    t60000.count60000 = 0;
}

// ==========================================================================
// Whether the buffer is currently positioned after the data
// which signals the beginning of a block or series of blocks
// that provides configuration option information on a project.
bool OptParser::bufAt60000()
{
    CircleBuf &buf = circle;
    unsigned short k = (unsigned short)getBufShort(buf, -6);
    short l = getBufShort(buf, -4);
    short m = getBufShort(buf, -2);
    short n = getBufShort(buf, 0);
    if (l == 0 && m == 1 && n == 0) {
	list<unsigned short>::iterator it;
	for (it = magic.begin(); it != magic.end(); ++it) {
	    const unsigned short &mgc = *it;
	    if (k == mgc) {
		return true;
	    }
	}
    }
    return false;
}

// ==========================================================================
// Track a block of code beginning with the special value 60000 decimal.
// (Actually, 50009 appears in the same role in older .opt files.)
// Recognize the CFG setting which occurs near the beginning of the block.
//
void OptParser::track60000(char ch)
{
    unsigned long count_preceding_ch = circle.trackCount();
    int offset = (int)(count_preceding_ch % 128UL);
    if (offset == 8) {
	if ((t60000.had_name || t60000.had_last) != t60000.had_CProject) {
	    if (t60000.had_name) {
		// Found name in 60000 block, but no CProject string.
                // The CProject string is not demanded by this application,
		// so it's totally unknown what the significance might be
                // if it's ever missed.
	    }
	    if (t60000.had_CProject) {
		// Had CProject string in the file, which usually occurs in 
                // a recognized 60000 block, but not always, particularly
                // when the block mentions CClsFldSlob.
	    }
	}
	t60000.had_name = t60000.had_CProject = t60000.had_last = false;
    }
    if (offset == 8
	&& bufAt60000()) {
	t60000.examine = true;
        t60000.count60000 += 1;
    }
    if (t60000.examine && offset == 10) {
	short n = getBufShort(circle, 0);
        int est = (nProjs > 0) ? nProjs : 1;
        int est2 = (t60000.count60000 > 0) ? t60000.count60000 + 1 : 1;
	if (n == 0) {
	    // Odd last 60000 block.
	    t60000.had_last = true;
	}
	else if (0 < n && (n <= est || n <= est2)) {
	    // Looks like a last 60000 block.
	    t60000.had_last = true;
	}
	else if (circle[-1] != '\0') {
	    t60000.name.start(circle[-2]);
	    t60000.name.track(circle[-1]);
	    t60000.had_name = true;
	}
        else {
	    // Unrecognized 60000 block, sometimes occurs when the last
            // such block has more entries than estimated and caught above.
	    // reportBuf(circle, 10);
	}
        t60000.examine = false;
    }
    if (t60000.name.atEnd()) {
	// Recognized a configuration string.
	cfgsp->push_back(t60000.name.name());
    }
    if (bufAt(circle, "CProject", 8)) {
	t60000.had_CProject = true;
    }

    t60000.name.track(ch);
}

// ==========================================================================
// Track the block of code which begins at the 1536th byte of a .opt file.
// This contains a list of project name/file name pairs.  All that's really
// used out of this is the count.  1536=384*4.
//
void OptParser::track384(char ch)
{
    bool starting_project = false;
    bool starting_file = false;

    if (t384.counting >= 0) {
        // Counting the 4 bytes between one pair and the next.
	if (t384.counting == 4) {
	    t384.project.start(ch);
            starting_project = true;
	    t384.counting = -1;
	}
	else {
            t384.counting += 1;
	}
    }
    else if (t384.project.atEnd()) {
	// At the end of a project name, start tracking the file name.
	t384.file.start(ch);
        starting_file = true;
    }
    else if (t384.file.atEnd()) {
	// At the end of a file name, if this is not the last project
        // then count 4 bytes to the next pair.
	// cout << "Matchup of project \"" << t384.project.name()
        //      << "\" with file \"" << t384.file.name() << "\"" << endl;
	t384.projsCount += 1;
	if (t384.projsCount < nProjs) {
	    t384.counting = 1;
	}
    }
    else if (circle.trackCount() == 384*4+4) {
	// Note the beginning of the interesting block.
	if (bufAt(circle, "\x00\x00\x00\x00", 4)) {
	}
	else if (bufAt(circle, "\x01\x00\x05\x00", 4)) {
	    t384.need_proj_count = true;
	}
	else {
	    noteWarning("Unexpected data at 384*4");
	}
    }
    else if (t384.need_proj_count && circle.trackCount() == 384*4+8) {
	// Get the count of projects.
	nProjs = getBufShort(circle, -2);
	// cout << "Number of projects is " << nProjs << "." << endl;
        t384.projsCount = 0;
	if (nProjs > 0) {
	    t384.project.start(ch);
            starting_project = true;
	}
    }
    // Track a project name.
    if (!starting_project) {
        t384.project.track(ch);
    }
    // Track a file name.
    if (!starting_file) {
        t384.file.track(ch);
    }
}

// ==========================================================================
// Called to track each byte of the .opt file.
//
void OptParser::track(char ch) {
    track60000(ch);
    track384(ch);
    circle.track(ch);
}

// ==========================================================================
void OptParser::noteError(const string &message)
{
    if (!had_error) {
	error_message = message;
	had_error = true;
    }
}

// ==========================================================================
void OptParser::noteWarning(const string &message)
{
    if (!had_error && !had_warning) {
	error_message = message;
	had_warning = true;
    }
}

// ==========================================================================
// Has same parameters and return value as 'optparse' below.
//
int OptParser::parse(const char *fn, vector<string> &cfgs, string &message)
{
    ifstream infile(fn, ios_base::in | ios_base::binary);
    if (infile.is_open()) {
	cfgs.clear();
	cfgsp = &cfgs;

	// Process the contents of the .opt file, one byte at a time.
	for (;;) {
	    char ch;
	    infile.get(ch);
	    if (infile.eof()) {
		break;
	    }
	    track(ch);
	}

	int expected = nProjs > 0 ? nProjs : 1;
	if (cfgs.size() != expected) {
	    stringstream m;
	    m << "Expected " << expected
              << " configurations in .opt file; found " << cfgs.size()
              << "." << ends;
	    noteWarning(m.str());
	}
    }
    else {
	string f = fn;
        f.append(" failed to open.");
	noteError(f);
    }
    if (had_error) {
	message = error_message;
	return 2;
    }
    else if (had_warning) {
	message = error_message;
	return 1;
    }
    return 0;
}

// ==========================================================================
// Parse the given file, gathering the configuration directives into cfgs.
// If any irregularity is detected, returns nonzero:
//       1 -> warning
//       2 -> error   (Don't use the cfgs.)
// and sets message with a diagnostic that the caller should pass along.
//
int optparse(const char *fn, vector<string> &cfgs, string &message)
{
    OptParser op;
    return op.parse(fn, cfgs, message);
}
