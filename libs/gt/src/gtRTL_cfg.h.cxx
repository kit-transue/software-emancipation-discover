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
// gtRTL_cfg.h.C
//------------------------------------------
// synopsis:
// 
// Access to ~/.psetfilt file
//------------------------------------------

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <customize.h>
#include <gtRTL_cfg.h>
#include <gtPushButton.h>
#include <sys/types.h>
#include <netinet/in.h>

// VARIABLE DEFINITIONS

gtRTL_cfg *def_table;

static const char RTL_FILTER_FILE[] = "%s/.psetfilt";


// FUNCTION DEFINITIONS

gtRTL_spec::gtRTL_spec(char* new_name, char* new_sort, char* new_format,
  		       char* new_filter, char* new_hide)
: name(new_name), sort(new_sort), format(new_format),
  filter(new_filter), hide(new_hide)
{}


static char* read_string(FILE* def_file)
{
    unsigned long net_len, len; 
    OSapi_fread(&net_len, sizeof(unsigned long), 1, def_file);
    len = ntohl(net_len);
    char* stuff = new char[len];
    OSapi_fread(stuff, sizeof(char), len, def_file);
    return stuff;
}


static void write_string(FILE* def_file, genString &string)
{
    if (string.is_null())
	string = "";
    unsigned long l = string.length() + 1;
    unsigned long net_l = htonl(l);
    OSapi_fwrite((const void *)&net_l,sizeof(unsigned long), 1, def_file);
    OSapi_fwrite(string.str(), sizeof(char), l, def_file);
}


gtRTL_cfg::gtRTL_cfg()
{
    bool success = false;

    genString config_file;
    config_file.printf(RTL_FILTER_FILE, customize::home());

    FILE* def_file = OSapi_fopen(config_file, "r");
    if(def_file)
    {
	// Just make doubly sure that the file is there and not zero length.
	struct stat info;
	if((OSapi_stat(config_file, &info) == 0) && (info.st_size != 0))
	{
	    msg("Default Filter:          $1. Loading...") << config_file.str() << eom;
	    unsigned long num_entries, net_num_entries = 0;
	    OSapi_fread(&net_num_entries, sizeof(unsigned long), 1, def_file);
	    num_entries = ntohl(net_num_entries);
	    for(int i = 0; i < num_entries; ++i)
	    {
		char* name   = read_string(def_file);
		char* sort   = read_string(def_file);
		char* format = read_string(def_file);
		char* filter = read_string(def_file);
		char* hide   = read_string(def_file);

		spec.insert_last(
		    new gtRTL_spec(name, sort, format, filter, hide));

		delete [] name;
		delete [] sort;
		delete [] format;
		delete [] filter;
		delete [] hide;
	    }
	    success = true;
            msg(" done\n") << eom;
	}
	else		// The RTL_filters file may be corrupted.  Delete the file.
	    OSapi_unlink(config_file);

	OSapi_fclose(def_file);
    }
    if (!success)
    {
	spec.insert_last(
	    new gtRTL_spec("Default", "on", "%on", "", ""));
	spec.insert_last(
	    new gtRTL_spec("Generic", "off", "%on", "", ""));
	spec.insert_last(
	    new gtRTL_spec("File View", "oxfoff", "%15of %on", "", ""));
    }
}


void gtRTL_cfg::save_entries()
{
    genString config_file;
    config_file.printf(RTL_FILTER_FILE, customize::home());

    FILE* def_file = OSapi_fopen(config_file, "w");
    if(def_file)
    {
	unsigned long num_entries = spec.size();
	unsigned long net_num_entries = htonl(num_entries);
	OSapi_fwrite((const void *)&net_num_entries, sizeof(unsigned long), 1, def_file);
	for(int i = 0; i < num_entries; ++i)
	{
	    write_string(def_file, ((gtRTL_spec*)spec[i])->name);
	    write_string(def_file, ((gtRTL_spec*)spec[i])->sort);
	    write_string(def_file, ((gtRTL_spec*)spec[i])->format);
	    write_string(def_file, ((gtRTL_spec*)spec[i])->filter);
	    write_string(def_file, ((gtRTL_spec*)spec[i])->hide);
	}
	OSapi_fclose(def_file);
    }
}


gtRTL_spec* gtRTL_cfg::get_entry(gtPushButton* pb)
{
    gtRTL_spec* found = NULL;

    char* nm = pb->title();
    for(int i = 0; i < spec.size(); ++i)
    {
	gtRTL_spec* check = (gtRTL_spec*)spec[i];
	if(check->name == nm)
	{
	    found = check;
	    break;
	}
    }
    gtFree(nm);

    return found;
}


/*
   START-LOG-------------------------------------------

   $Log: gtRTL_cfg.h.C  $
   Revision 1.11 2000/07/12 18:09:37EDT ktrans 
   merge from stream_message branch
// Revision 1.2  1993/06/02  20:25:59  glenn
// Remove Initialize macros.  Remove genError.h.
//
// Revision 1.1  1993/04/30  18:50:40  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/
