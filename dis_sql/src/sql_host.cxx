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
/*
 * S Q L _ H O S T . C
 *
 * 1998 Software Emancipation Technology, Inc.
 *
 * Created Feb. 1998 Mark B. Kaminsky
 *
 * Routines to create and write to the host language output file.
 * I.E. for ESQL/C, a C source file, and for ESQL/C++, a C++ source code file.
 */

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#include <cstdlib>
#include <cstring>
#endif /* ISO_CPP_HEADERS */

#include "sql_all.h"		// EXTERN_C
#include "sql_iff.h"		// IFF error log
#include "sql_host.h"		// host file output
#include "sql_scanner.h"	// scanner line/column number

//
// I/O File pointer to hostfile
//
static FILE* host_fp = 0;
EXTERN_C FILE* outf;		// in esql.c

//
// current host file
//
EXTERN_C const char * hostfilename = 0;


EXTERN_C int open_hostfile (const char * filename)
{
   int ret_val = 0;

   if (filename) {
      hostfilename = filename;
      if ((host_fp = fopen (hostfilename, "w")) == NULL) {
	 ret_val = -1;
	 iff_error ("Error openning host file");
      } else {
	 outf = host_fp;
      }
   } else {
      iff_error ("Missing host file name");
      ret_val = -1;
   }
   return (ret_val);
}

EXTERN_C int close_hostfile (void)
{
   if (host_fp) {
      fclose (host_fp);
      host_fp = 0;
      hostfilename = 0;
      outf = 0;
   }
   return (0);
}

// IFF Mapping records for esql
//
// Maps between text of input source file and host output
//

class iff_map_record {
 private:
   int newline_checked;		// check for beginning of line in map copy
   const char * src_filename;	// input filename (main or include)
   iffmap * copy_head;		// list of copy records
   iffmap * copy_current;	// current copy record
   iffmap * fixed_head;		// list of fixed records
   iffmap * fixed_current;	// current fixed record

   int start_entry (iffmap **headp, iffmap **currentp,
		    int il, int ic, int ol, int oc);
   int end_entry (iffmap **currentp,
		  int il, int ic, int ol, int oc);

   // copy and assignment shouldn't be called for this class
   // make them private and not implemented
   iff_map_record (const iff_map_record& rhs);
   iff_map_record& operator= (const iff_map_record& rhs);

 public:
   iff_map_record (void);
   ~iff_map_record ();
   int clear (void);
   int set_src_filename (const char *);
   const char * get_src_filename (void);
   int copy_start (int il, int ic, int ol, int oc);
   int copy_end (int il, int ic, int ol, int oc);
   int fixed_start (int il, int ic, int ol, int oc);
   int fixed_end (int il, int ic, int ol, int oc);
   iffmap * get_copy_head (void);
   iffmap * get_fixed_head (void);
   int newline_check (void);
};

iff_map_record::iff_map_record (void) :
	newline_checked (1),
	src_filename (0),
	copy_head (0),
	copy_current (0),
	fixed_head (0),
	fixed_current (0)
{
}

iff_map_record::~iff_map_record ()
{
   clear ();
}

// delete lists
int iff_map_record::clear (void)
{
   iffmap * mp;
   iffmap * mp_next;
   for (mp = copy_head; mp; mp = mp_next) {
      mp_next = mp->next;
      delete mp;
   }
   for (mp = fixed_head; mp; mp = mp_next) {
      mp_next = mp->next;
      delete mp;
   }
   src_filename = 0;
   copy_head = 0;
   copy_current = 0;
   fixed_head = 0;
   fixed_current = 0;
   return (0);
}

// input source file name
int iff_map_record::set_src_filename (const char * name)
{
   src_filename = name;
   return (0);
}
	
const char * iff_map_record::get_src_filename (void)
{
   return (src_filename);
}
	
// create new entry and set start values 
int iff_map_record::start_entry (iffmap **headp,
				 iffmap **currentp,
				 int il, int ic, int ol, int oc)
{
   int retval = 0;
   iffmap * mp = new iffmap;
   if (mp) {
      mp->src_lineno_start = il;
      mp->src_column_start = ic;
      mp->dest_lineno_start = ol;
      mp->dest_column_start = oc;
      mp->next = 0;

      if ( ! *headp ) {
	 *headp = mp;
	 *currentp = mp;
      } else if (*currentp) {
	 (*currentp)->next = mp;
	 *currentp = mp;
      }

   } else {
      retval = -1;
   }
   return (retval);
}

// set end values
int iff_map_record::end_entry (iffmap **currentp,
				 int il, int ic, int ol, int oc)
{
   if (*currentp) {
	 (*currentp)->src_lineno_end = il;
	 (*currentp)->src_column_end = ic;
	 (*currentp)->dest_lineno_end = ol;
	 (*currentp)->dest_column_end = oc;
   }
   return (0);
}

//
// Routines to start and end copy and fixed records
// based on input and output file line and columns.
// 
int iff_map_record::copy_start (int il, int ic, int ol, int oc)
{
   newline_checked = 0;
   return (start_entry (&copy_head, &copy_current, il, ic, ol, oc));
}

int iff_map_record::copy_end (int il, int ic, int ol, int oc)
{
   newline_checked = 1;
   return (end_entry (&copy_current, il, ic, ol, oc));
}

int iff_map_record::fixed_start (int il, int ic, int ol, int oc)
{
   return (start_entry (&fixed_head, &fixed_current, il, ic, ol, oc));
}

int iff_map_record::fixed_end (int il, int ic, int ol, int oc)
{
   return (end_entry (&fixed_current, il, ic, ol, oc));
}

// Get head of linked list
// called by IFF routines to output map entries
//
iffmap * iff_map_record::get_copy_head (void)
{
   return (copy_head);
}

iffmap * iff_map_record::get_fixed_head (void)
{
   return (fixed_head);
}

// iff_map_record::newline_check ---
// check for column beyond end of previous line.
// Only needed for the start of a copy record.
// correct to start of next line.
//
int iff_map_record::newline_check (void)
{
   if (newline_checked == 0) {
      newline_checked = 1;
      if (copy_current) {
	 // test that current map entry is in progress (no end info)
	 if (copy_current->src_lineno_end == 0) {
	    // check input (source) file is previous line and beyond end
	    // if so increment line to current line and first column
	    if ((copy_current->src_lineno_start == (ilineno - 1)) &&
		(copy_current->src_column_start > icolumn_last)) {
	       ++(copy_current->src_lineno_start);
	       copy_current->src_column_start = 1;
	    }
	    // do same for output (destination) file entry
	    if ((copy_current->dest_lineno_start == (olineno - 1)) &&
		(copy_current->dest_column_start > ocolumn_last)) {
	       ++(copy_current->dest_lineno_start);
	       copy_current->dest_column_start = 1;
	    }
	 }
      }
   }
	     
   return (0);
}
  
// 
// C interface
//

static iff_map_record imap;

EXTERN_C int iff_map_clear (void)
{
   return (imap.clear ());
}

EXTERN_C int iff_map_set_src_filename (const char *name)
{
   return (imap.set_src_filename (name));
}

EXTERN_C const char * iff_map_get_src_filename (void)
{
   return (imap.get_src_filename ());
}

EXTERN_C int iff_map_initial (void)
{
   return (imap.clear ());
}

EXTERN_C int iff_map_copy_start_loc (int il, int ic, int ol, int oc)
{
   return (imap.copy_start (il, ic, ol, oc));
}

EXTERN_C int iff_map_copy_end_loc (int il, int ic, int ol, int oc)
{
   return (imap.copy_end (il, ic, ol, oc));
}

EXTERN_C int iff_map_fixed_start_loc (int il, int ic, int ol, int oc)
{
   return (imap.fixed_start (il, ic, ol, oc));
}

EXTERN_C int iff_map_fixed_end_loc (int il, int ic, int ol, int oc)
{
   return (imap.fixed_end (il, ic, ol, oc));
}

EXTERN_C iffmap * iff_map_get_copy_head (void)
{
   return (imap.get_copy_head ());
}

EXTERN_C iffmap * iff_map_get_fixed_head (void)
{
   return (imap.get_fixed_head ());
}

EXTERN_C int iff_map_newline_check (void)
{
   return (imap.newline_check ());
}
