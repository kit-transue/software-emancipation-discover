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
// cmd_save.C
//
// Functions for saving Command Journal into file
//
// History:  08/22/91    M.Furman   Initial coding for cmd_gen()

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>
#include <genError.h>
#include <cmd_internal.h>
#include <cmd_enums.h>
#include <driver_mode.h>

struct cmdJournal {
 genString creator;
 genString date;
 genString pdf_name;
  genString project;
 genString subsystem;
 genString bug_number;
 genString journal_name;
 ofstream  file_stream;
 int echo;

 cmdJournal(char const *fn, char const *cn, char const *pdf, char const *bug, char const *sub, char const *proj, int ech);
 ~cmdJournal();
 };

cmdJournal * cmd_cur_journal;

static struct cmdJournalExit
{  cmdJournalExit(){}
   ~cmdJournalExit() {if(cmd_cur_journal) cmdJournal_finish();}
} cmd_exit_handler;

void cmdJournal_create
   (char const *fn, char const *cn, char const *pdf, char const*bug, char const*sub, char const *proj, int ech)
{
  cmd_cur_journal = new cmdJournal(fn, cn, pdf, bug, sub, proj, ech);
  if(! cmd_cur_journal->file_stream)
    cmdJournal_finish();
}

void cmdJournal_finish()
{
  delete cmd_cur_journal;
  cmd_cur_journal = 0;
}

void cmd_reset();
extern "C" void pset_getdate(const char* fmt, char* buf, int bufsz);

cmdJournal::cmdJournal(char const *fn, char const *cn, char const *pdf, char const*bug, char const*sub, char const *proj, int ech)

     : creator(cn), journal_name(fn), pdf_name(pdf), project(proj), bug_number(bug),
       subsystem(sub), file_stream(fn, ios::out), echo(ech)
{
    
    Initialize(cmdJournal::cmdJournal);
    IF(!file_stream){
      msg("cannot open $1 for output \n") << fn << eom;
      return;
    }

    //insert comments containing important information for running
    // the script


    char buf[20];
    char * bp = buf;
    const char * fmt = "%m/%d/%y";
//
// Record just basename of the .pdf file, not whole path.
//
    char const *pdf_basename = NULL;

    if (pdf) {
	pdf_basename = strrchr(pdf, '/');
        if (pdf_basename == NULL)
	    pdf_basename = pdf;
        else pdf_basename++;
    }

    pset_getdate(fmt, buf, 20);
    if (is_gui()) {
      file_stream << "/* Name of Journal: " << fn << "  */" << "\n";
      
      file_stream << "/* Creation Date: " << bp << "  */" << "\n";

      if (cn && cn[0])
	file_stream << "/* Name of Journal Creator: " << cn << "  */" << "\n";
      if (pdf_basename && pdf_basename[0])
	file_stream << "/* PDF file: " << pdf_basename  << " */" << "\n";
      if (proj && proj[0])
	file_stream << "/* Home Project: " << proj << "  */" << "\n";
      if (bug && bug[0])
	file_stream << "/* Bug Number: " << bug << "  */" << "\n";
      if (sub && sub[0])
	file_stream << "/* Subsystem: " << sub << "  */" << "\n";
    }
    file_stream << "{\n";
    if (is_gui()) {
      int cmd_ver = get_cmdVersion();
      file_stream << "em_set_speed(0);\n";
//  record scripting version (type of scripting)
      file_stream << "cmd_set_script_version (" << cmd_ver << ");\n";
//  set scripting version for paraset too
      cmd_set_script_version(cmd_ver);
      file_stream << "cmd_set_validation_level (" << FullValidation << ");\n";
      file_stream << "cmd_set_validation_group (\"" << sub << "\");\n";
    }
    file_stream << endl;
  }

cmdJournal::~cmdJournal()
{

file_stream << "}\n";
    file_stream << "/* the end */" << endl;
    file_stream.close();
    cmd_reset();
}


void cmd_gen_real_write(const char *fmt, va_list args)
{
    Initialize(cmd_gen_real_write);
    if (!cmd_current_journal_name)
	return;
   
    char *sp, *w;
    const char *f;
    int i;
    objOperType ot;
 
    ostream& s = cmd_cur_journal->file_stream;
    // process format string
    for(f = fmt; *f != 0; f++){
      if(*f != '%'){
            s << *f; 
            continue;
      }
      switch(*++f){
	 case 0:             // % at end of string - ignore 
           f--;
           break;

         case 'd':           // d - decimal number
           i = va_arg(args, int);
           s << i;
           break;

         case 's':           // s - text string 
           w = va_arg(args, char *);
           if(w == NULL)
             s << "0";
           else
             { 
             s << "\"";
             cmd_string_toc(s, w);
             s  << "\"";
             }
           break;

         case 't':           // t - objOperType value
           ot = va_arg(args, objOperType);
           s << "C_oper_type(\""
             << CT_oper_type(ot)
             << "\")";

           break;
 
         case 'h':           // h - header
         {
           objPlacement op;
           CC_hdr(va_arg(args, appPtr), &op);
           CT_hdr(&op, s);
	 }
           break;

         case 'n':           // n - node
	 {
	   objPlacement op;
           CC_node(va_arg(args, appTreePtr), &op);
           CT_node(&op, s);
	 }
           break;
         case 'l':	     // l - selection
	 {
           objArr arr;

           CC_sel(va_arg(args, objArr*), &arr);
           CT_sel(&arr, s);
	 }
           break; 

	 case 'r':           // r - RelType
           sp = CC_rel_type(va_arg(args, RelType *));
           CT_rel_type(sp, s);
           break;
	 }
    }

    s << endl;

      DBG 
      {
        msg(" -- JOURNAL $1 ") << cmd_level() << eom ;
        msg("") << eom;
      }

  }


/*
  $Log: cmd_save.C  $
  Revision 1.15 2001/07/25 20:41:07EDT Sudha Kallem (sudha) 
  Changes to support model_server.
 * Revision 1.13  1994/07/21  18:31:54  builder
 * include iostream.h
 *
 * Revision 1.12  1994/07/20  20:17:39  mg
 * Bug track: NA
 * restructured for logging
 *
*/

