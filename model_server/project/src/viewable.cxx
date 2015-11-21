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

#include <ddict.h>
#include <xref.h>
#include <xref_priv.h>

#include <db.h>
#include <path.h>

bool file_is_not_viewable(symbolPtr &file)
{
    symbolPtr xfile = file.get_xrefSymbol();
    if (xfile.xrisnotnull() && xfile->get_attribute(VIRT_ATT, 1)) {
	return true;
    }
    return false;
}

extern bool is_els_non_ascii_suffix (const char *full_path);

/* boris 12.16.98 Here is the algorithm:
   Module is not viewable (returns true) if either:
   - module = NULL
   - module's xref symbol has attribute "VIRT_ATT"
   - module's language==FILE_LANGUAGE_ELS and filename's suffix matches
     preference '*psetPrefs.ELS.nonAscii.Suffix: .class .zip .jar'
*/
bool module_is_not_viewable(projModule *mod)
{
    bool res = true;
    if (mod == NULL) return res;

    symbolPtr file = mod->get_def_file_from_proj();
    if (file.isnotnull())
	res = file_is_not_viewable(file);
    else {
	int lang = mod->language();
	if (lang == FILE_LANGUAGE_ELS) {
	    const char *phy = mod->get_phys_filename();
	    res = is_els_non_ascii_suffix(phy) ? true : false;
	} else
	    res = false;
     }
    return res;
}
