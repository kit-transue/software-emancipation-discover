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
#include "machdep.h"
#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#include <stdlib.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
namespace std {};
using namespace std;
#include <cstdlib>
#include <strstream>
#endif /* ISO_CPP_HEADERS */
#include <rtlFilter.h>
#include <gtRTL_cfg.h>
#include <filterButtonTable.h>
#include <genString.h>

genString renew_sort_spec (const char *spec);
void get_show_and_cli_filts (const char *str, genString& show, genString& cli);

static const char *translate_from_bad_names (const char *name) {

    return abb_to_title(name);
}


static const char *translate_to_bad_names (const char *name) {

     return title_to_abb(name);
}


void rtlFilter::genFilterInfo (char **argv, 
                               rtlFilterSortInfo **s_info_, 
                               rtlFilterFormatInfo **f_info_, 
			       char **formatExpression,
                               char **filter, 
                               char **hide,
			       char **CLIExpression) {

    int i, j;

    // scan through the list and see how many sort items we have, and how many format items we have.
    // Allocate the appropriate s_info and f_info arrays based on this.

    for (i=0, j=0; *argv[i]; i+=2, j++); 
    rtlFilterSortInfo *s_info = new rtlFilterSortInfo[j+1];
    *s_info_ = s_info;

    i+=2;

    for (j=0; *argv[i]; i+=2, j++);
    rtlFilterFormatInfo *f_info = new rtlFilterFormatInfo[j+1];
    *f_info_ = f_info;

    // Now go through the arguments and fill out the s_info and f_info arrays.

    for (i=0, j=0; *argv[i]; i+=2, j++) {
	s_info[j].code = title_to_abb(argv[i]);
	s_info[j].direction = strcmp (argv[i+1], "Ascending") == 0 ? 'f' : 'r';
    }

    s_info[j].code = "";
    i+=2;

    for (j=0; *argv[i]; i+=2, j++) {
	f_info[j].code = title_to_abb(argv[i]);
	f_info[j].width = atoi (argv[i+1]);
    }

    f_info[j].code = "";
    i+=2;

    *formatExpression = argv[i++];
    *filter = argv[i++];
    *hide   = argv[i++];
    *CLIExpression = argv[i];

}

int rtlFilter::getFilterCount () {
  return def_table.spec.size();
}

char *rtlFilter::getFilterName (int index) {
  return ((gtRTL_spec*)(def_table.spec[index]))->name;
}

void rtlFilter::Create (char *newName, 
                        char *sort, 
                        char *format, 
                        char *filter, 
                        char *hide) {

    gtRTL_spec* new_spec = new gtRTL_spec (newName, sort, format, filter, hide);
    def_table.add_entry(new_spec);

}

char* rtlFilter::compute_sort_titles (struct rtlFilterSortInfo *s_info) {

    ostrstream s2;
    for (int i = 0; s_info[i].code.length(); ++i) {
	if (s_info[i].code[0]) {
	    if (i != 0) s2 << ' ';
	    s2 << '{';
            s2 << abb_to_title (s_info[i].code.str());
	    s2 << "} {";
            s2 << (s_info[i].direction == 'r' ? "Descending" : "Ascending");
	    s2 << '}';
   	}
    }
    if (i != 0) s2 << ' ';
    s2 << "{} {}";
    s2 << ends;
    return s2.str();
}

char* rtlFilter::compute_format_titles (struct rtlFilterFormatInfo *f_info) {

    ostrstream s2;
    for (int i = 0; f_info[i].code.length(); ++i) {
	if (f_info[i].code[0]) {
	    if (i != 0) s2 << ' ';
	    s2 << '{';
            s2 << abb_to_title (f_info[i].code.str());
	    s2 << "} {";
            s2 << f_info[i].width;
	    s2 << '}';
   	}
    }
    if (i != 0) s2 << ' ';
    s2 << "{} {}";
    s2 << ends;
    return s2.str();
}

char* rtlFilter::compute_sort_spec(struct rtlFilterSortInfo *s_info) {

    ostrstream s2;
    s2 << '#';
    for (int i = 0; s_info[i].code.length(); ++i) {
	if (s_info[i].code[0]) {
            s2 << '%' << s_info[i].direction;
            s2 << s_info[i].code.str();
   	}
    }

    s2 << ends;
    return s2.str();
}

char *rtlFilter::compute_format_spec (struct rtlFilterFormatInfo *f_info, char *formatExpression) {

    ostrstream stream;
    char *s = formatExpression;

    for (int i = 0; f_info[i].code.length(); ++i) {
        if (!*s && i > 0) stream << " ";

	while (*s && *s != '%') stream << *s++;
	if (*s == '%') s++;

        stream << "%";
        if (f_info[i].width) stream << f_info[i].width;
        stream << f_info[i].code.str() << ',';
    }

    while (*s && *s != '%') stream << *s++;

    stream << ends;
    return stream.str();
}

char *rtlFilter::compute_filter_spec (char *filter, char *CLIExpression) {
    ostrstream stream;
    stream << filter;
    if (*CLIExpression) {
        stream << '\t';
        stream << CLIExpression;
    }
    stream << ends;
    return stream.str();
}


enum filterIterator {ITER_COUNTING, ITER_LOADING, ITER_END};

void rtlFilter::RTL_parse_specs (	rtlFilterFormatInfo ** f_info_, 
					rtlFilterSortInfo ** s_info_, 
					const char * format_spec, 
					const char * sort_spec, 
					const char * show_spec,
		     			genString & show_filt, 
		     			genString & cli_filt, 
					genString & fexp) {

    rtlFilterFormatInfo *f_info = NULL;
    rtlFilterSortInfo *s_info = NULL;

//
//  Allocate and Load the f_info array.  Also compute the fexp string.
// 

    int i = 0;
    fexp = "";

    if( !format_spec || !* format_spec) {
	f_info = new rtlFilterFormatInfo[2];
	f_info[0].code = "on";
	f_info[0].width = 0;
	fexp = "%";
	i++;
    }

    else {

	// Go through the loop twice; The first time count the number of format items that we have.
	// Allocate the list accordingly.  The second time, place the items in the list.

        for (	filterIterator iter = ITER_COUNTING; 
		iter != ITER_END; 
		iter = (iter == ITER_COUNTING) ? ITER_LOADING : ITER_END) {

	    i = 0;
	    const char *position = format_spec;

	    while(*position) {
	        if(*position++ == '%') {

		    if(!*position) break;


		    if (iter == ITER_LOADING) fexp += '%';

			if (iter == ITER_LOADING) f_info[i].width = 0;
		    while(isdigit(*position)) {
			if (iter == ITER_LOADING) {
		            f_info[i].width *= 10;
		            f_info[i].width += int(*position) - int('0');
			}
		        position++;
		    }

		    while (*position && (*position != ',') && 
			   !isspace(*position) && (*position != '%'))  {
			if (iter == ITER_LOADING) {
		            f_info[i].code += *position;
			}
			position++;
		    }

		    if (*position == ',') position++;

		    i++;
	        }

	        else if (iter == ITER_LOADING) {
	            fexp += *(position - 1);
		}
	    }

	    if (iter == ITER_COUNTING) {
		f_info = new rtlFilterFormatInfo[i+1];
	    }
	}
    }

    // Add a blank entry to signify the end of the list.

    f_info[i].code = "";
    f_info[i].width = 0;

//
//  Allocate and Load the s_info array.  Also compute the fexp string.
// 

    i = 0;
    if( !sort_spec || !* sort_spec) {
        s_info = new rtlFilterSortInfo[2];
        s_info[0].code = "of";
        s_info[0].direction = 'f';
        i++;
    }

    else {
	genString new_spec = renew_sort_spec(sort_spec);
	sort_spec = (char*)new_spec;

	// Go through the loop twice; The first time count the number of format items that we have.
	// Allocate the list accordingly.  The second time, place the items in the list.

        for (	filterIterator iter = ITER_COUNTING; 
		iter != ITER_END; 
		iter = (iter == ITER_COUNTING) ? ITER_LOADING : ITER_END) {

	    i = 0;
	    const char *position = sort_spec;

	    while(*position) {
	        if(*position++ == '%') {

		    if (iter == ITER_LOADING) s_info[i].direction = *position;
		    position++;

		    while(*position && *position != '%') {
		        if (iter == ITER_LOADING) s_info[i].code += *position;
			position++;
		    }

		    i++;
	        }
	    }

	    if (iter == ITER_COUNTING) {
	 	s_info = new rtlFilterSortInfo [i+1];
	    }
        }
    }

    // Add a blank entry to signify the end of the list.

    s_info[i].code = "";

    *s_info_ = s_info;
    *f_info_ = f_info;

    get_show_and_cli_filts(show_spec, show_filt, cli_filt);

}


void rtlFilter::Rename (int index, const char *newName) {
}

void rtlFilter::Save (int index) {
}

void rtlFilter::Delete (int index) {
}


