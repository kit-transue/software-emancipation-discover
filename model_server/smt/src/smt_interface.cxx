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
//////////////////////////   FILE smt_interface.C  //////////////////////
//
// -- Contains  interface functions between STE and SMT                  
//
#include <objOper.h>
//#include <ste_smt_interface.h>
//#include <ste_obj_interface.h>
//#include <steDocument.h>
//#include <steView.h>
//#include <ste_format_handler.h>
//#include <steDisplayNode.h>
//#include <ldrSmodNode.h>
#include <smt.h>
//#include <ste_get.h>
//#include <steButton.h>
//#include <steHeader.h>
//#include <ste_actions.h>

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <sstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

//#include <ste_smod.h>

/****
* ste_smt_send_string()   returns number of copied characters
****/
int ste_smt_send_string( smtTreePtr t, ostream& s )
{
    Initialize(ste_smt_send_string);

    smtHeaderPtr h = checked_cast(smtHeader,t->get_header());
    Assert (h);

    int len = t->length ();
    char* buff = h->srcbuf + t->start ();
    for( int i = 0; i < len; i++ )
        s << buff[i] ;
    return len;
}

/****
* ste_smt_put_native()
*****/
int ste_smt_put_native(smtTreePtr t, ostream& os)
{
    return ste_smt_send_string(t, os);
}

/*****
* ste_smt_get_native_string( smtTreePtr nd )
******/
char * ste_smt_get_native_string( smtTreePtr nd ){
 Initialize(ste_smt_get_native_string);
    ostringstream os;
    ste_smt_put_native(nd, os);
    os << ends;
    char *str = new char[os.str().size() + 1];
    return strcpy(str, os.str().c_str());
}

