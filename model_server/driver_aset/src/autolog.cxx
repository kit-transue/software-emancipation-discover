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
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <msg.h>
#include "autolog.h"
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <driver_mode.h>
#include <genString.h>
#include <customize.h>
#include <machdep.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>
#else /* ISO_CPP_HEADERS */
#include <ctime>
#endif /* ISO_CPP_HEADERS */
#ifdef _WIN32
#undef _POSIX_
#include <io.h>
#include <windows.h>
#endif

autolog *aset_log;

#ifdef _WIN32
  static const char *dir_sep = "\\";
#else
  static const char *dir_sep = "/";
#endif /*_WIN32*/
autolog::autolog() :
	hostname(customize::host()),
	username(customize::user()),
	last_message(""),
	lines_imported(0),
	current_lines_imported(0),
	files_imported(0),
	current_files_imported(0),
	lines_loaded(0),
	current_lines_loaded(0),
	files_loaded(0),
	current_files_loaded(0),
	open(0),
	filename( OSapi_getenv("PSET_LOGFILE"))
{


  if( filename.is_null() || !filename.length() )
  {
    genString create = customize::getStrPref( "logFileCreate" );
    if( create == (const char *)"yes" )
    {
      filename = customize::getStrPref( "logFileDir" );
      if( filename.length() )
        filename += dir_sep;
      filename += username;
      filename += ".log";
    }
  }
  if (filename.not_null() && filename.length()) {
#ifdef _WIN32
    char* p=strrchr( (char *)filename, '\\' );
    if ( p )
    {
      *p = 0;
      CreateDirectory( (char *)filename, NULL); 
      *p = '\\';
    }
#endif
    logfile.open(filename,ios::app);
    if(! logfile.good()) 
      msg("Unable to open log file: $1") << (char *)filename << eom;
    else {
      open = 1;
      start();
    }
  }
  else
    msg("Unable to get environment variable PSET_LOGFILE") << eom;
}

void autolog::import_lines(int n_lines) {
  current_lines_imported += n_lines;
  lines_imported += n_lines;
  ++files_imported;
  ++current_files_imported;
}

void autolog::load_lines(int n_lines) {
  current_lines_loaded += n_lines;
  lines_loaded += n_lines;
  ++files_loaded;
  ++current_files_loaded;
}

void autolog::import_complete(const char *file) {
  char *buffer = new char[strlen(file) + 50];
  sprintf(buffer,"Import: %s\n  files: %5d (%d)\n",file,
	current_files_loaded,current_files_imported);
  write(buffer);
  sprintf(buffer,"  lines: %5d \n",
	current_lines_loaded);
  write(buffer);
  sprintf(buffer,"Totals: \n  files: %5d (%d)\n",
        files_loaded,files_imported);
  write(buffer);
  sprintf(buffer,"  lines: %5d \n",
        lines_loaded);
  write(buffer);

  delete buffer;
  current_lines_imported = 0;
  current_files_imported = 0;
  current_files_loaded = 0;
  current_lines_loaded = 0;
}

void autolog::write(const char *string) {
  if(open && logfile.good()) {
    while (*string) {
      if(*string == '\n')
	logfile << "\n\t"; // indent log lines.
      else
	logfile << *string;
      string++;
    }
    logfile << flush;
  }
  else open = 0;
}

void autolog::message(const char *string) {
  if (string) {
    int len = strlen(string);
    if (len > 0) {
      write(string);
      if (string [len - 1] != '\n')
	linefeed();
    }
  }
}
void autolog::start() {
  logfile << "\nStart: ";
  timestamp();
  host_name();
  user_name();
#ifdef _WIN32
  if( is_gui() ) // only redirect stdout for gui
  {
    const char* xterm = OSapi_getenv("DISCOVER_XTERM");
    if( !xterm || !OSapi_strlen(xterm) ) //if DISCOVER_XTERM is defined, stdout is already redirected
    {
      int fd = logfile.fd();
      long hFile = _get_osfhandle( fd );
      if( hFile != -1 )
      {
        //set standard handles
        SetStdHandle( STD_OUTPUT_HANDLE, (HANDLE)hFile );
        SetStdHandle( STD_ERROR_HANDLE, (HANDLE)hFile );
        //update stdout, stderr
        _dup2( fd, _fileno(stdout) );
        _dup2( fd, _fileno(stderr) );
        setvbuf( stdout, NULL, _IONBF, 0 );
        setvbuf( stderr, NULL, _IONBF, 0 );
      }
    }
  }
#endif /*_WIN32*/
}

void autolog::print_prefs(char* prefs_file_name) {
//
// Print out the name of the preferences file used.
//  
    write("Preferences file: ");
    write(prefs_file_name);
    linefeed();
}

void autolog::end() {
  logfile << "\nEnd: ";
  timestamp();
  linefeed();
}

void autolog::linefeed() {
  write("\n");
}

void autolog::host_name() {
  write ("Hostname: ");
  write((const char *)hostname);
  linefeed();
}

void autolog::user_name() {
  write("User name: ");
  write((const char *)username);
  linefeed();
}

void autolog::timestamp() {
  time_t tCurTime;
  OSapi_time( &tCurTime);
  write(ctime( &tCurTime ));
}

void autolog::report_error(const char *error) {
  timestamp();
  write(" ");
  write(error);
}

void autolog::load_text(const char *buf,int len) {
  int n_lines = 0;
  for(int i = 0; i < len; ++i)
    if(*buf++ == '\n')
      ++n_lines;
  load_lines(n_lines);
}

int autolog::active() {
  return open;
}

autolog::~autolog() {
  logfile.close();
}

/*

   START-LOG--------------------------------------

   $Log: autolog.cxx  $
   Revision 1.21 2001/07/25 20:41:39EDT Sudha Kallem (sudha) 
   Changes to support model_server.
Revision 1.4  1994/02/08  15:04:24  builder
Port

Revision 1.3  1993/07/14  19:04:47  harry
nothing

Revision 1.2  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.1  1992/10/10  20:55:49  builder
Initial revision


   END-LOG----------------------------------------

*/





