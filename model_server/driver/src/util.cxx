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
#ifndef _WIN32
#include <sys/param.h>
#else
#include <stdlib.h>
#include <ctype.h>
#endif /*_WIN32*/
#include <msg.h>
#include <machdep.h>
#include <time_log.h>
#include <symbolArr.h>
#include <proj.h>
#include <path.h>
#include <mpb.h>
#include <driver_mode.h>

class Interpreter;
extern Interpreter *InitializeCLIInterpreter(void);
void construct_list_projects();

extern bool suppress_Xref=0;   // while this is one, suppress the Xref constructor for a project
const char* customize_default_project_definition();
const char* customize_install_root();
extern void attach_Xref(projNode* proj, projNode* parent, char const *name);
void project_path_sys_vars_translate(char const *m, genString &ret_path);

static int metering_client = 1;

extern "C" {
    void init_root_path_data();
    void  driver_exit(int);
}

static symbolArr * top_list;

symbolArr * get_top_list()
{
    return top_list;
}

void load_project_rtl (int num_of_pdf_files, genString* pdf_file)
// For each of the pdf files specified on command line (or in environment var),
//   expand and check the projects inside.  Then attach the pmods for top-level.
//
// num_of_pdf_files: is the number of the pdf files saved in the second parameter
//                   pdf_file
//
// pdf_file: is the buffer that contains the pdf file names
{
    Initialize(load_project_rtl);
    if (!num_of_pdf_files) {
	genString list;
	if (is_model_server())  list = customize::getStrPref("pdfFileBuild");
	if (list.length() == 0) list = customize_default_project_definition();
	if (list.length() == 0) {
	    list = customize_install_root();
	    list += "/lib/home.pdf";
	}
  char* tok = strtok(list, ":");

#ifndef _WIN32
  while (tok) {
	    pdf_file[num_of_pdf_files++] = tok;
	    tok = strtok(0, ":");
	}
#else
//because colon is used as a separator for multiple pdf names in preferences,
//we have to apply special logic to allow PC-style names (like c:\directory\file)
//This means that file name consisting of a single alphabetic character is not
 //allowed on NT when multiple PDFs are used  
  int fNameIsPending = 0;
  char pszPendingName[ _MAX_PATH ];
  char* tmp = 0;
  while (tok) 
  {
		if( strlen( tok ) == 1 && isalpha( tok[0] ) ) //if it is an alphabetic character
    {
      if( fNameIsPending )
        pdf_file[num_of_pdf_files++] = pszPendingName;
      
      fNameIsPending = 1;
      strcpy( pszPendingName, tok );
    }
    else
    {
      if( fNameIsPending )
      {
        fNameIsPending = 0;
        strcat( pszPendingName, ":" );
        strcat( pszPendingName, tok );
        create_path_2DIS( pszPendingName, &tmp );
	      pdf_file[num_of_pdf_files++] = tmp;
      }
      else
      {
        create_path_2DIS( tok, &tmp );
	      pdf_file[num_of_pdf_files++] = tmp;
      }
			if ( tmp )
      {
				free( tmp );
        tmp = 0;
      }
    }
	  tok = strtok(0, ":");
	}
#endif /*_WIN32*/
	if (!num_of_pdf_files)
	    pdf_file[num_of_pdf_files++] = NULL;
    }
    suppress_Xref = 1;     // While we are in this loop, do not make Xrefs for the
    init_root_path_data();
    //     projects.  We will do them at end, so that a pmod rule will be honored
    //     regardless of where in the pdf list it may occur.
    char pathname[MAXPATHLEN];
#ifndef _WIN32
    char *wd = (char *)OS_dependent::getwd(pathname);
#endif
    for (int proj_index = 0; proj_index < num_of_pdf_files; proj_index++)
    {
        project_path_sys_vars_translate(pdf_file[proj_index].str(), 
                                        pdf_file[proj_index] );
        char const *pdf_name = pdf_file[proj_index].str();
#ifndef _WIN32
        if (*pdf_name != '/')
        {
            genString new_name;
            new_name.printf("%s/%s", wd, pdf_name);
            pdf_file[proj_index] = new_name.str();
        }
#else
				OSapi_realpath(pdf_name, pathname);
				pdf_file[proj_index] = pathname;
#endif

	msg("Project Definition file: $1", normal_sev)
		<< realOSPATH((char*)pdf_file[proj_index].str()) << eom;
  
        int status = init_psetProjects (pdf_file[proj_index]);
	check_scope_name();

        if (!status)
        {
            msg("Quitting...", catastrophe_sev) << eom;
            driver_exit(1);
        }
    }
    int break_val = 8, val = 1,incr_val = 1, num_incs = 0;
    mpb_incr_values_init(break_val, val, incr_val);
    suppress_Xref = 0;
    // now go through all the projects, making Xrefs for anyone that does
    //    not yet have one.  This does not do the XrefTable, nor map the
    //    pmod in.  But it does nail down the physical file name to be
    //    used.
    //
    // don"t start from 0 which is the home project that might not be set yet
      if (top_list)
	delete top_list;
    top_list = new symbolArr;
    projNode* proj;
    for (int i = 1; (proj=projList::search_list->get_proj(i)) ; ++i) {
      char const *nm = proj->get_name();
	attach_Xref(proj, 0, 0);
      if (nm && nm[0] == '/' && (nm[1] != '_' || nm[2] != '_')
	  && strcmp(nm,"/ParaDOCS"))
	top_list->insert_last(proj);
      
      mpb_step(break_val, incr_val, val, num_incs,i);
    }

    mpb_incr();
}

void set_parse_pdf_is_being_executed(int);
void load_pdfs_complete()
{
    set_parse_pdf_is_being_executed(1);
    construct_list_projects();
    set_parse_pdf_is_being_executed(0);

    if(!is_model_build())
	InitializeCLIInterpreter();
}

void start_metering()
{
    time_log::init();
    call_trace::start_logging();
}

extern "C" void apl_PROJ_start_metering()
{
    if (metering_client)
	start_metering();
}

void stop_metering(char const *root_fcn)
{
    call_trace::stop_logging();
    dump_time_log_statistics(root_fcn);
}

extern "C" void apl_PROJ_stop_metering(const char* root_fcn)
{
    if (metering_client)
	stop_metering(root_fcn);
}

void suspend_metering()
{
    call_trace::stop_logging();
}

extern "C" void apl_PROJ_suspend_metering()
{
    if (metering_client)
	suspend_metering();
}

void resume_metering()
{
    call_trace::start_logging();
}

extern "C" void apl_PROJ_resume_metering()
{
    if (metering_client)
	resume_metering();
}

