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
// File cmd.h - Command Journal
//
// History:   08/08/91   M.Furman        Initial coding.
//            08/12/91   M.Furman        Class objPlacement added
//            08/19/91   M.Furman        Method cmdElement::send_string,
//                                       declarations for argument converting
//                                       functions added.
#ifndef _cmd_h
#define _cmd_h

#include <dis_iosfwd.h>

  class Relational;
  class Obj;
  class objTree;

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
class app;

#include <cmd_group.h>
#include <objDictionary.h>

class objValidationString : public namedString
{ 
  public:
    validation_function valfun;
    define_relational(objValidationString, namedString);
    objValidationString(    const char * tag, const char* val)    
       : namedString(tag, val), valfun(0) {}
    objValidationString(const char * tag, const char* val,    validation_function fun)
       : namedString(tag, val), valfun(fun) {}
    void print(ostream& os, int level=0) const; 
};
generate_descriptor(objValidationString,namedString);

class objSignature : public objValidationString
{ 
  public:
    const char * type_name;
    
    define_relational(objSignature,objValidationString);
    objSignature(const char * tag, const Obj*);
    void set_value(const Obj*);
 void print(ostream& os, int level=0) const; 
};
generate_descriptor(objSignature,objValidationString);

void cmd_group_print(ostream& ostr);
extern "C" int cmd_do_validation(CMD_VALIDATION_GROUP group);
extern "C" int cmd_set_validation_group(char* name);

#define CMD_GROUP(x) if((!cmd_logging_on)&&cmd_do_validation(paste(CMD_,x)))  
#define CMD_MAX_NAME_LEN          64

void cmd_validate_dd(app *app);

extern "C" void cmd_journal_start(char *, char *, char *, char *, char *);
extern "C" void cmd_journal_save(char *);
extern "C" void cmd_journal_execute(char *);
extern "C" void cmd_journal_exec_within(char *);
extern "C" void cmd_journal_insert_line(char *);
int cmd_break();

extern "C" app * dd_map(Relational* node);

 void cmd_init_hooks();

void cmd_validate_tree(const char * labl, objTree* tree);

int cmd_level();
int cmd_begin();
void cmd_end();
void cmd_gen(const char *fmt, ...);
void cmd_gen_write_force(const char *fmt, ...);

void cmd_error(char *);
void cmd_reset();

extern int cmd_logging_on;
extern char *cmd_current_journal_name;
extern char *cmd_execute_journal_name;
extern char *cmd_xterm_input_file;

#define VALIDATING ((!cmd_logging_on) && \
   (cmd_current_journal_name || cmd_execute_journal_name))

#define VALIDATE if(VALIDATING)


void cmd_execute(char *);
int cmd_exec_next_stmt();
int cmd_exec_continue();
int cmd_exec_pause();
int cmd_exec_break();
int cmd_exec_resume();

void cmd_dbg_prompt(char *);
void cmd_dbg_execute(char *);

class viewer;
class view;

extern "C" viewer* cmd_get_cur_viewer();
void cmd_synchronize(const char *);
void cmd_synchronize_parser(const char *);

extern "C" void cmd_find_file (int len, int cur_buf, int cur_scr, int other_fl, char *nm);
extern "C" void cmd_switch_to_buffer (int the_buf);
extern "C" void cmd_epoch_window_command (int sub_cm, int buf_id, int scr_id, int bbb);
extern "C" void cmd_epoch_window_report (int sub_cm, int buf_id, int scr_id, int bbb);


int cmd_get_ind(objTree * t);
view * cmd_get_cur_view();
objTree * cmd_get_child(objTree *t, int ind);
void cmd_record_view(view *);

void cmd_validation_error   (const char *);
void cmd_validation_warning (const char *);
void cmd_validation_message (const char *);
int cmd_validate_string_value(const char * new_val, const char * old_val);
void cmd_validate (const char *labl, const char *val);
void cmd_validate (const char *labl, const char *val, const char *alias);
void cmd_validate (const char *labl, int val);
void cmd_validate (const char *labl, const Obj*);

int cmd_validate_flush ();

void cmd_validate (const char *labl, class symbolPtr&);
void cmd_validate (const char *labl, class symbolArr&);
void cmd_validate (const char *labl, class symbolArr&, int force_sort);
void cmd_validate (const char *labl, class symbolSet&);

void cmd_validate_lineno (const char *nm, int lnum, const char *alias = NULL);

extern "C" void cmd_set_validation_level (int);
extern "C" void cmd_set_script_version (int);
extern "C" void unlink_xterm_file();

int cmd_available_p (int functionality);
int cmd_validate_p (int level = -1);
int get_cmdVersion ();

void cmd_record_viewer_validate (viewer *vwr);
void cmd_abort ();

int cmd_is_epoch_recording ();
int cmd_set_epoch_recording (int);

void cmd_flush();

/*

   START-LOG-------------------------------------------

   $Log: cmd.h  $
   Revision 1.10 2001/07/25 20:41:01EDT Sudha Kallem (sudha) 
   Changes to support model_server.
 * Revision 1.2.1.30  1994/07/25  14:56:55  mg
 * Bug track: NA
 * cfront bug workaround:  enum {app=..} in ostream conflicts with pset class app.
 *
 * Revision 1.2.1.29  1994/07/21  18:30:44  builder
 * include a line for ostream.
 *
 * Revision 1.2.1.28  1994/07/20  20:11:11  mg
 * Bug track: NA
 * restructured for logging
 *
 * Revision 1.2.1.27  1994/05/17  19:09:29  boris
 * Bug track: Scripting
 * Fixed a number of scripting problems
 *
 * Revision 1.2.1.26  1994/04/04  18:38:04  boris
 * Added epoch macro scripting functionality
 *

   END-LOG---------------------------------------------
*/

#endif
