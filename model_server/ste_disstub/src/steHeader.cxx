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
#include <fstream.h> 
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#include <iostream>
#endif /* ISO_CPP_HEADERS */

//
// EXTERNAL "stubs" declarations
//
extern "C" {
    char * EDITOR_message_print(char *);
};

int        ste_memseg_flag = 0;  // set shared memory Flag ;

////////
// Func:    operator <<
// Params:  The stream to flush and an used "trigger" arguement.
// Returns: The same flushed stream.
// Why:     Needed to flush only if a certain flag is set.  In order to 
//          continue to use a "<<" chain, a new "dummy" type had to be
//          introduced.
////////
ostream& operator << (ostream& os, struct ste_end_flush_handler*)
{
    return os;
}

////////
// Func:    steHeaderStream
// Params:  An (optional) new fd to use for communication with the editor.
// Returns: The stream to use communication.
///////
ostream& steHeaderStream(int)
{
    return cout;
}

////////
// Func:    steHeader
// Params:  The command that is being send to editor.
// Returns: The stream to use for any additional data.
///////
ostream& steHeader(int command)
{
    cout << endl;
    EDITOR_message_print("steHeader");
    cout << "\t " << command << " " << flush;
    return steHeaderStream(1);
}

