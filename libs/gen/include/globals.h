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
/*  globals.h
   created 5/20/94

purpose:  to define the class which will hold various 
   global variables needed by code all over the system,
   and provide safe access to those variables

all functions and data are static

   */
#ifndef _globals_h
#define _globals_h


typedef  unsigned int uint;


enum  error_levels
{
#ifdef _WIN32
    __NO_ERROR = 0,
#else
    // if nobody reports any errors, 0 is the default
    NO_ERROR = 0,
#endif
    // minor errors are things that affect part of the model,
    //  such as parse errors, minor diagnostics, ...
    MINOR_ERROR = 1,
    // crash means serious or fatal errors, such as
    //   could not open home pmod, ran out of memory,
    //   ran out of file handles, trap handler
    CRASH_ERROR = 2,
    //  MAX_ERROR_LEVEL is just used in the error_status() function to
    //  make sure no errors occur outside of this range
    MAX_ERROR_LEVEL = CRASH_ERROR
};

class  globals
{
  public:
//  indicate that an error of "level" severity has occurred
//  call with no argument to get the present error level
#ifdef _WIN32
    static error_levels  error_status(error_levels  level=__NO_ERROR);
#else
    static error_levels  error_status(error_levels  level=NO_ERROR);
#endif

    //    static uint files_with_parse_errors();
    //    static uint incr_parse_errors();
  private:
//  err_status - log of the worst error level that has occurred this run
    static error_levels  err_status;

    static uint parse_errors;
    globals();
    globals(const globals&);

};


extern "C" void fast_exit(error_levels exit_status);
extern "C" void real_fast_exit(error_levels exit_status);


/*
   START-LOG-------------------------------------------

  $log:  $
   END-LOG---------------------------------------------

*/
#endif // _globals_h

