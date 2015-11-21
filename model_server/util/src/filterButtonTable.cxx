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

#include <genString.h>
#include <attribute.h>
#include <scopeMgr.h>
#include <metric.h>
#include <genError.h>
#include <systemMessages.h>

typedef struct {
    const char * abbrev;
    const char * title;
} abb_and_title;

static abb_and_title button_table[] = 
{   
    { "ff", TXT("Formatted String") },
    { "on", TXT("Entity Name") },
    { "of", TXT("Entity Filename") },
    { "ox", TXT("Entity File Extension") },
//    { "ot", TXT("Entity Category") },
    { "ol", TXT("Line Number") },
    { "type", TXT("Type") },
    { "args", TXT("Arguments") },
    { "numargs", TXT("Number of Arguments") },
    { "kind", TXT("Entity Kind") },
    { "astcategory", TXT("Instance Context") },
    { "fname", TXT("File Name") },
    { "language", TXT("Language") },
    { "psetfile", TXT("Model File") },
    { "met_CC", TXT("Cyclomatic Complexity") },
    { "met_MYERS", TXT("Myers's Complexity") },
    { "met_FANIN", TXT("Number of Superclasses") },
    { "met_FANOUT", TXT("Number of Subclasses") },
    { "met_BYTES", TXT("Size in Bytes") },
    { "met_DEPTH", TXT("Depth of Nested Control Structures") },
    { "met_DANG_EI", TXT("Number of Dangling Else-Ifs") },
    { "met_DANG_SW", TXT("Number of Defaultless Switch Statements") },
    { "met_START_LN", TXT("Starting Line Number") },
    { "met_RETURNS", TXT("Number of Return Statements") },
    { "met_LOC", TXT("Number of Lines of Code") },
    { "met_ULOC", TXT("Number of Uncommented Lines") },
    { "met_XLOC", TXT("Number of Executable Lines") },
    { "met_IFS", TXT("Number of If Statements") },
    { "met_LOOPS", TXT("Number of Loops") },
    { "met_SWITCHES", TXT("Number of Switch Statements") },
    { "met_DEFAULTS", TXT("Number of Default Statements") },
};

const char * abb_to_title(const char * abb) {
    Initialize(abb_to_title);

    int size = sizeof(button_table)/sizeof(abb_and_title);
    int i;
    for (i=0; i<size && strcmp(abb, button_table[i].abbrev); i++);
    const char * res;
    if (i==size) {
	symbolAttribute * att = symbolAttribute::get_by_name(abb);
	if (att && is_metricAttribute(att))
	  res = Metric::def_array[((metricAttribute*)att)->get_type()]->name;
	else res = abb;
    }
    else res = button_table[i].title;
    return res;
}

const char * title_to_abb(const char * tit) {
    Initialize(title_to_abb);
    
    int size = sizeof(button_table)/sizeof(abb_and_title);
    int i;
    for (i=0; i<size && strcmp(tit, button_table[i].title); i++);
    const char * res = NULL;
    if (i==size) {
	// Check if this is a metric.
	for (int type = 0; type < NUM_OF_METRICS && !res; type ++) {
	    if (!strcmp(tit, Metric::def_array[type]->name)) {
		genString attr_name = "met_";
		attr_name += Metric::def_array[type]->abbrev;
		symbolAttribute * attr = symbolAttribute::get_by_name((const char *)attr_name);
		res = attr->get_name(); 
	    }
	}
	if (!res) res = tit;
    }
    else res = button_table[i].abbrev;
    return res;
}

objNameSet& get_all_extern_attributes(void);

