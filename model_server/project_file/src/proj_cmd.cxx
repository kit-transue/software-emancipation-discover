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
#include <cmd.h>
/*
#include <stdio.h>
#include <proj.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#include <path.h>
#include <fileEntry.h>
#include <proj_save.h>
*/
void proj_restore_validate_info  (const char *pname, int mods, int prjs, int lnum)
{
    VALIDATE {
	genString cur;
	cur.printf("%s_%s", pname, "TotalModules");
	cmd_validate ((const char *)cur, mods);

	cur.printf("%s_%s", pname, "TotalProjects");
	cmd_validate ((const char *)cur, prjs);

	cur.printf("%s_%s", pname, "Finalline");
	cmd_validate ((const char *)cur, lnum);
    }
}

void proj_restore_validate_result (const char *pname, int rslt, const char *err_msg)
{
    VALIDATE {
	genString cur;
	cur.printf("%s_%s", pname, "RestoreResult");
	cmd_validate ((const char *)cur, rslt);
	if (rslt != 1 && err_msg && err_msg[0]) {
	    cur.printf("%s_%s", pname, "RestoreError");
	    cmd_validate ((const char *)cur, err_msg);
	}
    }
}
