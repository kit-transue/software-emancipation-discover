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
/**********
*
* Args.cxx - Galaxy Args class defs.
*
**********/
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "../include/Args.h"

//moved from Application.C
Args* DIS_args;

// Args Class Constructor.
Args::Args(int count, char * pList[]) {
   index = 0;
   argCount = count;
   argList = new char* [argCount];   // Create the array.
   int i = 0;
   while (i<argCount) {
      argList[i] = new char [strlen(pList[i])+1];
      strcpy(argList[i], pList[i]);
      i++;
   }
}

// Args Class Constructor.
Args::~Args() 
{
    for (int i = 0; i < argCount; i++) {
        if (argList[i]) delete[] (argList[i]);
    }
 
    delete[] argList; 
}

// Args method to get the first argument.
char * Args::firstArg() {
   index = 0;
   if (index < argCount)
      return(argList[index]);
   else
      return(NULL);
}

// Args method to get the next argument.
char * Args::nextArg() {
   if (++index < argCount)
      return(argList[index]);
   else
      return(NULL);
}

// Args method to find the given argument.
// Note - Uses firstArg/nextArg.
// Note - Only checks for the length of the given arg.
int Args::findArg(const char * arg) {
   if (arg && *arg ) {
      int length = strlen(arg);
      char * myArg = firstArg();
      while (myArg) {
         if (strncmp(myArg, arg, length) == 0)
            return(1);
         myArg = nextArg();
      }
   }
   return(0);
}

// Args method to get the value following the given arg.
// Note - Uses firstArg/nextArg/findArg.
// Note - Does no checking of what the value is.
int Args::findValue(const char * arg, char * value) {
   if (findArg(arg)) {   // Found the arg.
      char * myValue = nextArg();
      if (myValue) {
         strcpy(value, myValue);
         return(1);
      }
   }
   *value = 0;
   return(0);
}

// Args method to get the number of arguments.
int Args::numArgs() 
{ 
    return(argCount); 
}

