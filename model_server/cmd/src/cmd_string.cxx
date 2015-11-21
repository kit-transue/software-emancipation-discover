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
// cmd_string.C
//
// Functions to convert strings to/from "C" representation
//
// History:  08/22/91    M.Furman   Initial coding for 
//           08/23/91    M.Furman   Code for converting from "C" added

#include "genError.h"
#include "cmd.h"
#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#include <ctype.h>
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
using namespace std;
#include <cctype>
#include <iostream>
#endif /* ISO_CPP_HEADERS */

void cmd_string_toc(ostream &s, char *x)
{
    char *p=x;
    int has_nl = 0;
    for(p = x; *p != 0; p++)
      if(*p == '\n'){
         has_nl = 1;
         break;
      }

    if(has_nl) s << "\\\n";
    for(p = x; *p != 0; p++)
      {
      switch(*p)
        {
        case '\\':
          s << "\\\\";
          break;
        case '\"':
          s << "\\\"";
          break;
        case '\n' :
          s << "\\n\\\n";
          break;
        case '\t' :
          s << "\\t";
          break;
        default:
          s << *p;
          break;
        }
      }
    if(has_nl) s << "\\\n";
}

char *cmd_string_toc(char *x)
    {
    ostrstream s;

    cmd_string_toc(s, x);
    s << ends;
    return(s.str());
    }

void cmd_string_fromc(ostream &s, char *x)
    {
    char *p;
    int c, i, n;

    for(p = x; *p != 0; p++)
      {
      if(*p != '\\')
         s << *p;
      else
          switch(c = *++p)
          {
          case 0:
            --p;
            break;
          case 'n':
            s << '\n';
            break; 
          
          case 'r':
            s << '\r';
            break; 
          
          case '\'':
            s << '\'';
            break; 
          
          case '"':
            s << '"';
            break; 
          
          case '\\':
            s << '\\';
            break; 
          
          case 't':
            s << '\t';
            break; 
          
          case 'b':
            s << '\b';
            break; 

          default:
            if(isdigit(c))
              {
              // Parse and convert number (octal)
              n = c - '0';
              for(i = 0; i < 2 && isdigit(c = p[1]); i++,p++)
                {
                n << 3;
                n |= c - '0';
                }
              s.put((char)n);
              }
            else
              s.put((char)c);
            break;
          }
      }
    }

char *cmd_string_fromc(char *x)
    {
    ostrstream s;

    cmd_string_fromc(s, x);
    s << ends;
    return(s.str());
    }

