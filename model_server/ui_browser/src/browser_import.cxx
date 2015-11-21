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
// browser_import.C
//------------------------------------------
// synopsis:
// Callback function to import a file.
//
// description: 
// 
//------------------------------------------
// Log: see the bottom 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files

#include <msg.h>
#include <strings.h>
#include "genArr.h"
#include "genString.h"
#include "waiter.h"
#include "messages.h"
#include "smt.h"
#include "RTL.h"

#include "errorBrowser.h"
#include <sys/param.h>
#undef rindex
#include <machdep.h>
extern void logger_disable();
extern void logger_enable();
extern void project_convert_filename(char *path, genString &ret_path);

extern "C"
  {
  int check_smt_asynch_data ();
  void clear_smt_asynch_data ();
  };

// type definitions

typedef genString* genStringPtr;
genArr(genStringPtr);

typedef void* (*voidFuncCharPtr)(char*);
genArr(voidFuncCharPtr);

RelClass(steDocument);

// external functions

extern "C" const char* ui_get_file_suffix(const char*);

// static variables

static const char  begin_list[] = "begin list";
static const char  end_list[]   = "end list";
static int import_canceled = FALSE;

//------------------------------------------
// fileSelectorCmd
//------------------------------------------

class fileSelectorCmd : public commandContext
{
  public:
    ~fileSelectorCmd();

    void store(const char*, voidFuncCharPtr = NULL);
    genString& recall(int, voidFuncCharPtr&);
    int count() { return pathname_array.size(); }

  protected:
    fileSelectorCmd(Widget, const char*, estimTime);

  private:
    genStringPtrArr pathname_array;
    voidFuncCharPtrArr func_array;
};

fileSelectorCmd::fileSelectorCmd(Widget w, const char* n, estimTime t)
: commandContext(w, n, t)
{
    feedback()->label(/*T*/"File");
    feedback()->status(/*T*/"Status");
}

fileSelectorCmd::~fileSelectorCmd()
{
    for(int i = 0; i < count(); ++i)
	delete *pathname_array[i];
}

void fileSelectorCmd::store(const char* pathname, voidFuncCharPtr func)
{
    genStringPtr string = new genString(pathname);

    pathname_array.append(&string);
    func_array.append(&func);
}

genString& fileSelectorCmd::recall(int index, voidFuncCharPtr& func)
{
    func = *func_array[index];
    return **pathname_array[index];
}

//------------------------------------------
// importFiles
//------------------------------------------

class importFiles : public fileSelectorCmd
{
  private:
    static int cancel_result;
  public:
    static void* last_import_result;

    importFiles(Widget w) : fileSelectorCmd(w, /*T*/"Import", ESTIM_MEDIUM){ 
					    last_import_result = NULL; 
					    importFiles::cancel_result = FALSE;}
  protected:
    virtual void execute();
};

void* importFiles::last_import_result;
int importFiles::cancel_result;


/*_______________________________________________________________________
!
! Function:	cancel_import
!
! Synopsis:	This is a very temporary fix to a bad callback problem.
!		This callback should be passed down through importFiles,
!		commandContext, waiter, and windowWaiter, to the Progress
!		dialog constructor where it is called.  
!	
! Returns:	int  False if the popup should be popped down.
!
_______________________________________________________________________*/ 
extern "C" int browser_import_cancel(void*, int) { 
    import_canceled = TRUE;
    return TRUE; 
}

static genString rep_path;
static importFiles *import_one_file = 0;
extern "C" void reparse_finish()
{
    Initialize(reparse_finish);

    msg("Finished importing file: $1.", normal_sev) << (char *)rep_path << eom;

    if (import_one_file)
        delete import_one_file;
    import_one_file = 0;
}

/*_______________________________________________________________________
!
! Function:	importFiles::execute
!
! Synopsis:	The main loop for importing files.
!
_______________________________________________________________________*/ 
void importFiles::execute()
{
    errorBrowserClear();
    for(int i = 0; ((i < count()) && (!import_canceled)); ++i)
    {
	voidFuncCharPtr import;
	genString& pathname = recall(i, import);

	feedback()->label(pathname);
        logger_disable();
	msg("Importing $1 of $2: $3", normal_sev) << i + 1 << eoarg << count() << eoarg << (char *)pathname << eom;
        logger_enable();

	if(import)
	    last_import_result = (*import)(pathname);
	else
	    msg("Cannot import file: $1", error_sev) << (char*)pathname << eom;
        if (!check_smt_asynch_data())
        {
            feedback()->percent_completed((i + 1) * 100 / count());
        }
        else
            rep_path = (char *)pathname;
    }
    import_canceled = FALSE;
    if (!check_smt_asynch_data())
    {
        if (count() == 1)
	    msg("Finished importing 1 file.", normal_sev) << eom;
        else if (count() > 1)
	    msg("Finished importing $1 files.", normal_sev) << count() << eom;
    }  
    if(!errorBrowser_empty())
      errorBrowser_show_errors();
}

//------------------------------------------
// Local file importing functions
//------------------------------------------

static void* import_C(char* fn)
{
    return smt_prog_import(fn, NULL, smt_C);
}

static void* import_Cplusplus(char* fn)
{
    return smt_prog_import(fn, NULL, smt_CPLUSPLUS);
}


//------------------------------------------
// browser_import
//------------------------------------------

extern "C" void browser_import(
    Widget parent_w, const char* pn, const char* file_type)
{
    static importFiles* command = NULL;

    if(!file_type)
	file_type = "plain";

    if(!strcmp(file_type, begin_list)){
	command = new importFiles(parent_w);
    } else if(!strcmp(file_type, end_list)) {
	
	if(command) {

	    command->start();
	    delete command;
	    command = NULL;
	}
    } else {
        genString ret_path;
        project_convert_filename((char *)pn, ret_path);
        if (!ret_path.length()) {
	    msg("File/directory not found: $1", error_sev) << pn << eom;
	    return;
	}
    
        pn = (char *)ret_path.str();

	voidFuncCharPtr import = NULL;

	if(!strcmp(file_type, "c") || !strcmp(file_type, "esql-c"))
	    import = import_C;
	else if(!strcmp(file_type, "cplusplus") || !strcmp(file_type, "esql-cplusplus"))
	    import = import_Cplusplus;
	else if (!strcmp(file_type, "makefile")) {
	    clear_smt_asynch_data (); 
	    return;
	}

	if(command){
	    command->store(pn, import);
	} else {
	    if (import != import_C && import != import_Cplusplus)
		clear_smt_asynch_data ();
	    import_one_file = new importFiles(parent_w);
	    import_one_file->store(pn, import);
	    import_one_file->start();
	    if (!check_smt_asynch_data ()) {
		delete import_one_file;
		import_one_file = 0;
	    }
	}
    }
}

