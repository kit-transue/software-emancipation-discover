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
#include <cLibraryFunctions.h>
#include <machdep.h>
#include "gString.h"
#include "scanner.h"
#include <tcl.h>


Scanner::Scanner()
{
	for (int i=0; i<4; i++) {
		prefix_[i] = NULL;
		cmd_[i] = NULL;
	}
	lineBuffer = NULL;
	lineBufferSize = 0;
	lineBufferIndex = 0;
}

Scanner::~Scanner()
{
	for (int i=0; i<4; i++) {
		if (prefix_[i]) delete [] prefix_[i];
		if (cmd_[i]) delete [] cmd_[i];
	}

	if (lineBuffer) OSapi_free (lineBuffer);
}

void Scanner::ScanPrefix (const vchar* prefix, const vchar* cmd)
{
    for (int i=0; i<4; i++) {
		if (!prefix_[i]) {
			int len = vcharLength (prefix);
			prefix_[i] = new vchar[len+1];
			vcharCopy (prefix, prefix_[i]);

			len = vcharLength (cmd);
			cmd_[i] = new vchar[len+1];
			vcharCopy (cmd, cmd_[i]);

			return;
		}
	}
}

void Scanner::ScanString(const vchar* str, Tcl_Interp* interp)
{
	if (!lineBuffer) 
		lineBuffer = (vchar*)OSapi_malloc (10000);

	while (*str) {
		vchar* nl = vcharSearchChar (str, '\n');
		if (nl == str) {
		        lineNo++;
			str++;
			continue;
		}
		if (nl) {
			int len = nl - str;
			if (len + lineBufferIndex > lineBufferSize) {
			}
			vcharCopyBounded (str, &lineBuffer[lineBufferIndex], len);
			lineBufferIndex += len;
			lineBuffer[lineBufferIndex] = 0;
			str = nl+1;
			const vchar* substr;
			for (int i=0; i<4; i++) {
				if (interp && prefix_[i] &&  cmd_[i] && 
					(substr=vcharSearch (lineBuffer, prefix_[i]))) {
					char line[100];
					sprintf (line, " %d ", lineNo);
					gString tclcmd;
					tclcmd = cmd_[i];
					tclcmd += (vchar*)line;
					tclcmd += (vchar*)"{";
					tclcmd += lineBuffer;
					tclcmd += (vchar*)"}";
					Tcl_Eval (interp, (char*)(vchar*) tclcmd);
				}
			}
			lineNo++;
			lineBufferIndex = 0;
		} else {
			int len = vcharLength(str);
			vcharCopyBounded (str, &lineBuffer[lineBufferIndex], len);
			lineBufferIndex += len;
			lineBuffer[lineBufferIndex] = 0;
			str = (vchar*)"";
		}
	}
}

void Scanner::Reset ()
{
	lineNo = 0;
}

#if 0
main()
{
	Scanner s;

	s.ScanPrefix ((vchar*)"Progress ");

	s.ScanString ((vchar*)"    +++Progress 0 4 1.1.3");
	s.ScanString ((vchar*)"it\n Now");
	s.ScanString ((vchar*)"\n");

	return 0;
}
#endif
