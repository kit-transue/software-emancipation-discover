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
/* ------MAINTENANCE NOTE:
        if you change the license strings, be sure to change
        /paraset/data/license.dat.template accordingly.  It would
        also be nice to tell the person who generates license files
        as well!
 * ------ENDNOTE */


#ifdef WIN32
// Stops warnings about truncation of const int to char 
// (arises from the xor operator)
#pragma warning( disable : 4305 4309 )
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <lm_code.h>
#include <lm_attr.h>
#include <messages.h>
#include <lmclient.h>

#include <xxinterface.h>
#include <msg.h>

#include <xxsn.h>

// Version of DISCOVER checkedout from license file: 
#define VERSION "7.3"

////////////////////////////////////////////////////////////////////////////////
//
// Define some data types to help us track the licenses
//
////////////////////////////////////////////////////////////////////////////////

enum license_type {
  ENFORCED,	// Normal checkout procedure
  COUNTED,	// Number of licenses checked out is counted internally
  WAIT,		// License queues if it cannot be checked out initially
  UNENFORCED	// License is not checked
};
    
typedef struct {
  
  // license identification information
  license_id   id;
  char         code[4];
  char         flexlm_id[32];
  char         print_name[32];

  // for customizing the actions; ENFORCED is the standard type
  license_type ltype;

  // to count the number of each license checked out; used by licenses
  // of type COUNTED
  int num_checked_out;

} license_record;

////////////////////////////////////////////////////////////////////////////////
//
// Data structs for license management
//
////////////////////////////////////////////////////////////////////////////////

static license_record license_data[] = 
{
  { LIC_NULL,          { ' ' ^ 0xb4, 0, 0, 0 }, { ' ' ^ 0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { ' ' ^ 0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_ACCESS,        { 'r' ^ 0xb4, 0, 0, 0 }, { 'A' ^ 0xb4, 'c' ^ 0x98, 'c' ^ 0xa2, 'e' ^ 0x66, 's' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'A' ^ 0xb4, 'c' ^ 0x98, 'c' ^ 0xa2, 'e' ^ 0x66, 's' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_ADMIN,         { 'i' ^ 0xb4, 0, 0, 0 }, { 'A' ^ 0xb4, 'd' ^ 0x98, 'm' ^ 0xa2, 'i' ^ 0x66, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'M' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'd' ^ 0xc8, 'm' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_AUTODOC,       { 'd' ^ 0xb4, 0, 0, 0 }, { 'A' ^ 0xb4, 'u' ^ 0x98, 't' ^ 0xa2, 'o' ^ 0x66, 'D' ^ 0xc8, 'o' ^ 0xc8, 'c' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'A' ^ 0xb4, 'u' ^ 0x98, 't' ^ 0xa2, 'o' ^ 0x66, 'D' ^ 0xc8, 'o' ^ 0xc8, 'c' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_CLIENT,        { 'm' ^ 0xb4, 'c' ^ 0x98, 0, 0 }, { 'M' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, 'C' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'M' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, COUNTED,    0},
  { LIC_C_CPP,         { 'x' ^ 0xb4, 'b' ^ 0x98, 0, 0 }, { 'C' ^ 0xb4, '_' ^ 0x98, 'C' ^ 0xa2, 'P' ^ 0x66, 'P' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'C' ^ 0xb4, '/' ^ 0x98, 'C' ^ 0xa2, '+' ^ 0x66, '+' ^ 0xc8, ' ' ^ 0xc8, 'P' ^ 0xc8, 'a' ^ 0xc8, 'r' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, WAIT,       0},
  { LIC_DEBUG,         { 'f' ^ 0xb4, 0, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'b' ^ 0xa2, 'u' ^ 0x66, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'I' ^ 0xb4, 'n' ^ 0x98, 't' ^ 0xa2, 'e' ^ 0x66, 'g' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'd' ^ 0xc8, ' ' ^ 0xc8, 'D' ^ 0xc8, 'e' ^ 0xc8, 'b' ^ 0xc8, 'u' ^ 0xc8, 'g' ^ 0xc8, 'g' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DEFECTLINK_RO, { 'd' ^ 0xb4, 'r' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'f' ^ 0xa2, 'e' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'O' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'e' ^ 0x98, 'f' ^ 0xa2, 'e' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'Q' ^ 0xc8, 'u' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DEFECTLINK_RW, { 'd' ^ 0xb4, 'w' ^ 0x98, 0, 0 } ,{ 'D' ^ 0xb4, 'e' ^ 0x98, 'f' ^ 0xa2, 'e' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'W' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'e' ^ 0x98, 'f' ^ 0xa2, 'e' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'd' ^ 0xc8, 'm' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DELTA,         { 'd' ^ 0xb4, 'a' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'l' ^ 0xa2, 't' ^ 0x66, 'a' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'e' ^ 0x98, 'l' ^ 0xa2, 't' ^ 0x66, 'a' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DESIGN,        { 'e' ^ 0xb4, 0, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 'i' ^ 0x66, 'g' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 'i' ^ 0x66, 'g' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DFA,           { 'l' ^ 0xb4, 'k' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'F' ^ 0x98, 'A' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'a' ^ 0x98, 't' ^ 0xa2, 'a' ^ 0x66, ' ' ^ 0xc8, 'F' ^ 0xc8, 'l' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'n' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 'y' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DISCOVER,      { 'c' ^ 0xb4, 0, 0, 0 }, { 'D' ^ 0xb4, 'I' ^ 0x98, 'S' ^ 0xa2, 'C' ^ 0x66, 'O' ^ 0xc8, 'V' ^ 0xc8, 'E' ^ 0xc8, 'R' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'i' ^ 0x98, 's' ^ 0xa2, 'c' ^ 0x66, 'o' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_DOCLINK_RO,    { 'g' ^ 0xb4, 'r' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'o' ^ 0x98, 'c' ^ 0xa2, 'L' ^ 0x66, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'O' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'o' ^ 0x98, 'c' ^ 0xa2, 'L' ^ 0x66, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'V' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'w' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DOCLINK_RW,    { 'g' ^ 0xb4, 'w' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'o' ^ 0x98, 'c' ^ 0xa2, 'L' ^ 0x66, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'W' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'o' ^ 0x98, 'c' ^ 0xa2, 'L' ^ 0x66, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'M' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'i' ^ 0xc8, 'f' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_DORMANT,       { 'v' ^ 0xb4, 0, 0, 0 }, { 'D' ^ 0xb4, 'o' ^ 0x98, 'r' ^ 0xa2, 'm' ^ 0x66, 'a' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'o' ^ 0x98, 'r' ^ 0xa2, 'm' ^ 0x66, 'a' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'n' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 'y' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_EXTRACT,       { 'h' ^ 0xb4, 0, 0, 0 }, { 'E' ^ 0xb4, 'x' ^ 0x98, 't' ^ 0xa2, 'r' ^ 0x66, 'a' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'E' ^ 0xb4, 'x' ^ 0x98, 't' ^ 0xa2, 'r' ^ 0x66, 'a' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_JAVA,          { 'x' ^ 0xb4, 'a' ^ 0x98, 0, 0 }, { 'J' ^ 0xb4, 'a' ^ 0x98, 'v' ^ 0xa2, 'a' ^ 0x66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'J' ^ 0xb4, 'a' ^ 0x98, 'v' ^ 0xa2, 'a' ^ 0x66, ' ' ^ 0xc8, 'P' ^ 0xc8, 'a' ^ 0xc8, 'r' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, WAIT,       0},
  { LIC_METRICS,       { 'q' ^ 0xb4, 0, 0, 0 }, { 'M' ^ 0xb4, 'e' ^ 0x98, 't' ^ 0xa2, 'r' ^ 0x66, 'i' ^ 0xc8, 'c' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'M' ^ 0xb4, 'e' ^ 0x98, 't' ^ 0xa2, 'r' ^ 0x66, 'i' ^ 0xc8, 'c' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_MODULARITY,    { 't' ^ 0xb4, 0, 0, 0 }, { 'M' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'u' ^ 0x66, 'l' ^ 0xc8, 'a' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'M' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'u' ^ 0x66, 'l' ^ 0xc8, 'a' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_PACKAGE,       { 'l' ^ 0xb4, 0, 0, 0 }, { 'P' ^ 0xb4, 'a' ^ 0x98, 'c' ^ 0xa2, 'k' ^ 0x66, 'a' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'P' ^ 0xb4, 'a' ^ 0x98, 'c' ^ 0xa2, 'k' ^ 0x66, 'a' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_PARTITION,     { 'k' ^ 0xb4, 0, 0, 0 }, { 'P' ^ 0xb4, 'a' ^ 0x98, 'r' ^ 0xa2, 't' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'P' ^ 0xb4, 'a' ^ 0x98, 'r' ^ 0xa2, 't' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_QAC,           { 'V' ^ 0xb4, 'I' ^ 0x98, '1' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, 'C' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'u' ^ 0x98, 'a' ^ 0xa2, 'l' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'o' ^ 0xc8, 'c' ^ 0xc8, 'k' ^ 0xc8, 'p' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_REPORT,    { 'V' ^ 0xb4, 'I' ^ 0x98, '7' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'R' ^ 0x66, 'e' ^ 0xc8, 'p' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'u' ^ 0x98, 'a' ^ 0xa2, 'l' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, ' ' ^ 0xc8, 'R' ^ 0xc8, 'e' ^ 0xc8, 'p' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_RUN,       { 'V' ^ 0xb4, 'I' ^ 0x98, '3' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'R' ^ 0x66, 'u' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'u' ^ 0x98, 'a' ^ 0xa2, 'l' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'n' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 'y' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_BROWSE,    { 'V' ^ 0xb4, 'I' ^ 0x98, '4' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'B' ^ 0x66, 'r' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'A' ^ 0x98, ' ' ^ 0xa2, 'I' ^ 0x66, 'n' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'a' ^ 0xc8, 'n' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'B' ^ 0xc8, 'r' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_QFS_MASTER,    { 'V' ^ 0xb4, 'I' ^ 0x98, '2' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, '_' ^ 0x66, 'M' ^ 0xc8, 'a' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'M' ^ 0xc8, 'a' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_QFS_GLOBAL,    { 'V' ^ 0xb4, 'I' ^ 0x98, 'A' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'G' ^ 0x66, 'l' ^ 0xc8, 'o' ^ 0xc8, 'b' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'G' ^ 0xc8, 'l' ^ 0xc8, 'o' ^ 0xc8, 'b' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'z' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_PORT,      { 'V' ^ 0xb4, 'I' ^ 0x98, '5' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'P' ^ 0x66, 'o' ^ 0xc8, 'r' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'P' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, 't' ^ 0xc8, 'a' ^ 0xc8, 'b' ^ 0xc8, 'i' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_STAND,     { 'V' ^ 0xb4, 'I' ^ 0x98, '6' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'S' ^ 0x66, 't' ^ 0xc8, 'a' ^ 0xc8, 'n' ^ 0xc8, 'd' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'P' ^ 0xc8, 'r' ^ 0xc8, 'o' ^ 0xc8, 'g' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 'm' ^ 0xc8, 'm' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'r' ^ 0xc8, 'u' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_STRUCT,    { 'V' ^ 0xb4, 'I' ^ 0x98, '9' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'S' ^ 0x66, 't' ^ 0xc8, 'r' ^ 0xc8, 'u' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'S' ^ 0xc8, 't' ^ 0xc8, 'r' ^ 0xc8, 'u' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 'u' ^ 0xc8, 'r' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_TESTCOV,   { 'V' ^ 0xb4, 'I' ^ 0x98, 'B' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'T' ^ 0x66, 'e' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'C' ^ 0xc8, 'o' ^ 0xc8, 'v' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'T' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'o' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_QFS_STATS,     { 'V' ^ 0xb4, 'I' ^ 0x98, '8' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'A' ^ 0x98, '_' ^ 0xa2, 'S' ^ 0x66, 't' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Q' ^ 0xb4, 'F' ^ 0x98, 'S' ^ 0xa2, ' ' ^ 0x66, 'S' ^ 0xc8, 't' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 's' ^ 0xc8, ' ' ^ 0xc8, 'R' ^ 0xc8, 'e' ^ 0xc8, 'p' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_REPORTS,       { 'u' ^ 0xb4, 0, 0, 0 }, { 'R' ^ 0xb4, 'e' ^ 0x98, 'p' ^ 0xa2, 'o' ^ 0x66, 'r' ^ 0xc8, 't' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'R' ^ 0xb4, 'e' ^ 0x98, 'p' ^ 0xa2, 'o' ^ 0x66, 'r' ^ 0xc8, 't' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_SIMPLIFY,      { 'n' ^ 0xb4, 0, 0, 0 }, { 'S' ^ 0xb4, 'i' ^ 0x98, 'm' ^ 0xa2, 'p' ^ 0x66, 'l' ^ 0xc8, 'i' ^ 0xc8, 'f' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'S' ^ 0xb4, 'i' ^ 0x98, 'm' ^ 0xa2, 'p' ^ 0x66, 'l' ^ 0xc8, 'i' ^ 0xc8, 'f' ^ 0xc8, 'y' ^ 0xc8, '.' ^ 0xc8, 'h' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_SQL,           { 'x' ^ 0xb4, 0, 0, 0 }, { 'S' ^ 0xb4, 'Q' ^ 0x98, 'L' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'S' ^ 0xb4, 'Q' ^ 0x98, 'L' ^ 0xa2, ' ' ^ 0x66, 'P' ^ 0xc8, 'a' ^ 0xc8, 'r' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, WAIT,       0},
  { LIC_STATISTICS,    { 's' ^ 0xb4, 0, 0, 0 }, { 'S' ^ 0xb4, 't' ^ 0x98, 'a' ^ 0xa2, 't' ^ 0x66, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'S' ^ 0xb4, 't' ^ 0x98, 'a' ^ 0xa2, 't' ^ 0x66, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_SUBCHECK,      { 's' ^ 0xb4, 'c' ^ 0x98, 0, 0 }, { 'S' ^ 0xb4, 'u' ^ 0x98, 'b' ^ 0xa2, 'C' ^ 0x66, 'h' ^ 0xc8, 'e' ^ 0xc8, 'c' ^ 0xc8, 'k' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'S' ^ 0xb4, 'u' ^ 0x98, 'b' ^ 0xa2, 'm' ^ 0x66, 'i' ^ 0xc8, 's' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, ' ' ^ 0xc8, 'C' ^ 0xc8, 'h' ^ 0xc8, 'e' ^ 0xc8, 'c' ^ 0xc8, 'k' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_TASKFLOW,      { 'w' ^ 0xb4, 'a' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'a' ^ 0x98, 's' ^ 0xa2, 'k' ^ 0x66, 'F' ^ 0xc8, 'l' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'a' ^ 0x98, 's' ^ 0xa2, 'k' ^ 0x66, 'F' ^ 0xc8, 'l' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_TESTLINK_RO,   { 's' ^ 0xb4, 'r' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 't' ^ 0x66, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'O' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 't' ^ 0x66, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'Q' ^ 0xc8, 'u' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_TESTLINK_RW,   { 's' ^ 0xb4, 'w' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 't' ^ 0x66, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, 'R' ^ 0xc8, 'W' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'e' ^ 0x98, 's' ^ 0xa2, 't' ^ 0x66, 'L' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'k' ^ 0xc8, ' ' ^ 0xc8, 'A' ^ 0xc8, 'd' ^ 0xc8, 'm' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'i' ^ 0xc8, 's' ^ 0xc8, 't' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_TPM,           { 'w' ^ 0xb4, 'b' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'P' ^ 0x98, 'M' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'r' ^ 0x98, 'e' ^ 0xa2, 'e' ^ 0x66, ' ' ^ 0xc8, 'P' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'n' ^ 0xc8, ' ' ^ 0xc8, 'M' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'c' ^ 0xc8, 'h' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_TPM_REMED,     { 'w' ^ 0xb4, 'c' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'P' ^ 0x98, 'M' ^ 0xa2, '_' ^ 0x66, 'C' ^ 0xc8, 'P' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'P' ^ 0x98, 'M' ^ 0xa2, ' ' ^ 0x66, 'R' ^ 0xc8, 'e' ^ 0xc8, 'm' ^ 0xc8, 'e' ^ 0xc8, 'd' ^ 0xc8, 'i' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_WEBVIEW,       { 'w' ^ 0xb4, 'v' ^ 0x98, 0, 0 }, { 'W' ^ 0xb4, 'e' ^ 0x98, 'b' ^ 0xa2, 'V' ^ 0x66, 'i' ^ 0xc8, 'e' ^ 0xc8, 'w' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'W' ^ 0xb4, 'e' ^ 0x98, 'b' ^ 0xa2, 'V' ^ 0x66, 'i' ^ 0xc8, 'e' ^ 0xc8, 'w' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_Y2K,           { 'w' ^ 0xb4, 0, 0, 0 }, { 'Y' ^ 0xb4, '2' ^ 0x98, 'K' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'Y' ^ 0xb4, '2' ^ 0x98, 'K' ^ 0xa2, ' ' ^ 0x66, 'R' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 'i' ^ 0xc8, 'd' ^ 0xc8, 'u' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, UNENFORCED, 0},
  { LIC_CR_BUILD,      { 'C' ^ 0xb4, 'l' ^ 0x98, 0, 0 }, { 'C' ^ 0xb4, 'R' ^ 0x98, '_' ^ 0xa2, 'B' ^ 0x66, 'u' ^ 0xc8, 'i' ^ 0xc8, 'l' ^ 0xc8, 'd' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'C' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'e' ^ 0x66, 'R' ^ 0xc8, 'o' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'B' ^ 0xc8, 'u' ^ 0xc8, 'i' ^ 0xc8, 'l' ^ 0xc8, 'd' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_CALIPER,    { 'C' ^ 0xb4, 'm' ^ 0x98, 0, 0 }, { 'C' ^ 0xb4, 'a' ^ 0x98, 'l' ^ 0xa2, 'i' ^ 0x66, 'p' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'C' ^ 0xb4, 'a' ^ 0x98, 'l' ^ 0xa2, 'i' ^ 0x66, 'p' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_INTEGRITY,  { 'C' ^ 0xb4, 'n' ^ 0x98, 0, 0 }, { 'I' ^ 0xb4, 'n' ^ 0x98, 't' ^ 0xa2, 'e' ^ 0x66, 'g' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'I' ^ 0xb4, 'n' ^ 0x98, 't' ^ 0xa2, 'e' ^ 0x66, 'g' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_DEVXPRESS,  { 'C' ^ 0xb4, 'a' ^ 0x98, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'v' ^ 0xa2, 'X' ^ 0x66, 'P' ^ 0xc8, 'r' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'D' ^ 0xb4, 'e' ^ 0x98, 'v' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, 'o' ^ 0xc8, 'p' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'X' ^ 0xc8, 'P' ^ 0xc8, 'r' ^ 0xc8, 'e' ^ 0xc8, 's' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_TPM,        { 'C' ^ 0xb4, 'o' ^ 0x98, 0, 0 }, { 'C' ^ 0xb4, 'R' ^ 0x98, '_' ^ 0xa2, 'T' ^ 0x66, 'P' ^ 0xc8, 'M' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'C' ^ 0xb4, 'o' ^ 0x98, 'd' ^ 0xa2, 'e' ^ 0x66, 'R' ^ 0xc8, 'o' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'T' ^ 0xc8, 'P' ^ 0xc8, 'M' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_TRENDS,     { 'C' ^ 0xb4, 'p' ^ 0x98, 0, 0 }, { 'T' ^ 0xb4, 'r' ^ 0x98, 'e' ^ 0xa2, 'n' ^ 0x66, 'd' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'T' ^ 0xb4, 'r' ^ 0x98, 'e' ^ 0xa2, 'n' ^ 0x66, 'd' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
  { LIC_CR_IMPACT,     { 'C' ^ 0xb4, 'q' ^ 0x98, 0, 0 }, { 'I' ^ 0xb4, 'm' ^ 0x98, 'p' ^ 0xa2, 'a' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { 'I' ^ 0xb4, 'm' ^ 0x98, 'p' ^ 0xa2, 'a' ^ 0x66, 'c' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
// Our new license types
  { LIC_BUILD, { 'B' ^ 0xb4, 'u' ^ 0x98, 'I' ^ 0xa2, 0 }, { 'B' ^ 0xb4, 'u' ^ 0x98, 'i' ^ 0xa2, 'l' ^ 0x66, 'd' ^ 0xc8, 'T' ^ 0xc8, 'o' ^ 0xc8, 'o' ^ 0xc8, 'l' ^ 0xc8, 's' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 'B' ^ 0xb4, 'u' ^ 0x98, 'i' ^ 0xa2, 'l' ^ 0x66, 'd' ^ 0xc8, 'T' ^ 0xc8, 'o' ^ 0xc8, 'o' ^ 0xc8, 'l' ^ 0xc8, 's' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED, 0}, 
  { LIC_DEVELOPER, { 'D' ^ 0xb4, 'e' ^ 0x98, 'V' ^ 0xa2, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'v' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, 'o' ^ 0xc8, 'p' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'C' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 'D' ^ 0xb4, 'e' ^ 0x98, 'v' ^ 0xa2, 'e' ^ 0x66, 'l' ^ 0xc8, 'o' ^ 0xc8, 'p' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, COUNTED, 0},
  { LIC_QUALITY, { 'Q' ^ 0xb4, 'u' ^ 0x98, 'A' ^ 0xa2, 0 }, { 'Q' ^ 0xb4, 'u' ^ 0x98, 'a' ^ 0xa2, 'l' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 'C' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 'Q' ^ 0xb4, 'u' ^ 0x98, 'a' ^ 0xa2, 'l' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'y' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, COUNTED, 0},
  { LIC_ARCHITECT, { 'A' ^ 0xb4, 'r' ^ 0x98, 'C' ^ 0xa2, 0 }, { 'A' ^ 0xb4, 'r' ^ 0x98, 'c' ^ 0xa2, 'h' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 'C' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 't' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 'A' ^ 0xb4, 'r' ^ 0x98, 'c' ^ 0xa2, 'h' ^ 0x66, 'i' ^ 0xc8, 't' ^ 0xc8, 'e' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, COUNTED, 0},
  { LIC_SERVER, { 'S' ^ 0xb4, 'r' ^ 0x98, 'V' ^ 0xa2, 0 }, { 'A' ^ 0xb4, 'd' ^ 0x98, 'd' ^ 0xa2, 'M' ^ 0x66, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, 'l' ^ 0xc8, 'S' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 'S' ^ 0xb4, 'e' ^ 0x98, 'r' ^ 0xa2, 'v' ^ 0x66, 'e' ^ 0xc8, 'r' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED, 0},
// And the null license
  { LIC_NUM_LICENSES,  { ' ' ^ 0xb4, 0, 0, 0 }, { ' ' ^ 0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },\
      { ' ' ^ 0xb4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, ENFORCED,   0},
};


static const license_id ArchitectLicense[] = {
  LIC_SIMPLIFY,
  LIC_DORMANT,
  LIC_EXTRACT,
  LIC_PACKAGE,
  LIC_PARTITION,
  LIC_Y2K,
// of course, the Architect license type has to include the overall architect license
  LIC_ARCHITECT,
  LIC_NULL
};

static const license_id QualityLicense[] = {
  LIC_DFA,
  LIC_QAC,     
  LIC_QFS_REPORT, 
  LIC_QFS_RUN,   
  LIC_QFS_BROWSE,  
  LIC_QFS_MASTER, 
  LIC_QFS_GLOBAL,
  LIC_QFS_PORT,   
  LIC_QFS_STAND,   
  LIC_QFS_STRUCT, 
  LIC_QFS_TESTCOV,  
  LIC_QFS_STATS, 
  LIC_STATISTICS,
  LIC_QUALITY,
  LIC_NULL
};


static const license_id BuildLicense[] = {
  LIC_C_CPP,
  LIC_JAVA,
  LIC_SQL,
  LIC_BUILD,
  LIC_NULL
};

static const license_id ServerLicense[] = {
  LIC_SERVER,
  LIC_DISCOVER,
  LIC_NULL
};

static const license_id DeveloperLicense[] = {
  LIC_TPM_REMED,
  LIC_TPM,
  LIC_CR_IMPACT,
  LIC_CLIENT,
  LIC_ACCESS,
  LIC_CR_DEVXPRESS,
  LIC_ADMIN,
  LIC_AUTODOC,
  LIC_DEBUG,
  LIC_DEFECTLINK_RO,
  LIC_DEFECTLINK_RW,
  LIC_DELTA,
  LIC_DESIGN,
  LIC_DOCLINK_RO,
  LIC_DOCLINK_RW,
  LIC_TASKFLOW,
  LIC_TESTLINK_RO,
  LIC_TESTLINK_RW,
  LIC_WEBVIEW,
  LIC_DEVELOPER,
  LIC_NULL
};

// 5 types: Build, Architect, Quality, Developer and Server
// 	- this will have to be incremented when we add a CodeRover type
int const TOTAL_LICENSE_TYPES = 5;
license_id const * const LICENSE_TYPES[] = { BuildLicense, DeveloperLicense, QualityLicense, ArchitectLicense, ServerLicense};
// This might be a sorta shitty way to keep track of the license types
//	(a struct { license_id array[], int typeName } would be more appropriate), but
//	this will do for now.  And this also has to be updated, when we add another license type
int const BUILD_LICENSE_TYPE 		= 0;
int const DEVELOPER_LICENSE_TYPE 	= 1;
int const QUALITY_LICENSE_TYPE 		= 2;
int const ARCHITECT_LICENSE_TYPE 	= 3;
int const SERVER_LICENSE_TYPE 		= 4;

///////////////////////////////////////////////////////////////////

static license_record *_lli(license_id id) {
  license_record *answer = 0;
  for ( int i = 0, done = 0; i < LIC_NUM_LICENSES && ! done; i++ )
    {
      if ( id == license_data[i].id )
        {
          answer = &license_data[i];
          done = 1;
        }
    }
  return answer;
}

////////////////////////////////////////////////////////////////////////////////
//
// Stuff to do with scrambling and unscrambling the strings
//
////////////////////////////////////////////////////////////////////////////////

static void unscramble(char *l)
{
	bool done = (l[0] == 0);
	if (!done) { l[0] ^= 0xb4; done = (l[1] == 0); }
	if (!done) { l[1] ^= 0x98; done = (l[2] == 0); }
	if (!done) { l[2] ^= 0xa2; done = (l[3] == 0); }
	if (!done) { l[3] ^= 0x66; done = (l[4] == 0); }
	for (int i = 4; !done; done = (l[++i] == 0)) {
		 l[i] ^= 0xc8;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Start of license handling
//
////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
  void      driver_exit(int);
}
static void quit(char *);
static void reconn(char *, int, int, int);
static void r_done(char *, int);

////////////////////////////////////////////////////////////////////////////////
//
//  FLEXlm related stuff
//
////////////////////////////////////////////////////////////////////////////////

static int const   LICENSE_RETRYS = 8;
VENDORCODE         vendorcode;
LM_HANDLE         *lm_job;

#ifdef lm_errstring
#undef lm_errstring
#endif
extern "C" char *lm_errstring(int) 
{ 
	if(usingSerialNumber()) {
		return(serialErrorString);
	} else { // FlexLM
		return lc_errstring(lm_job); 
	}
}

////////////////////////////////////////////////////////////////////////////////
//
//    Licensing functions:
//
//    All license functions return LIC_SUCCESS on success
//    and LIC_FAIL on failure.
//   
//    _lc()           Connect to daemon.
//    _lhb()          Send heartbeat.
//    _le()           Print error during initialization message.
//    _lo(license)    Check out license by id. 
//    _li(license)    Check in license by id.
//    _lf(license)    Does feature exist in license file? (by id)
//    _lh(license)    Is license for feature checked out? (by id)
//    _lm(license)    Issue unable to get license for feature. (by id)
//    _los(license)   Check out license by string id.
//    _lis(license)   Check in license by string id.
//    _lfs(license)   Does feature exist in license file? (by string id)
//    _lhs(license)   Is license for feature checked out? (by string id)
//    _lms(license)   Issue unable to get license for feature. (by string id)
//
////////////////////////////////////////////////////////////////////////////////

// returns -1 if the id was not found in any license type, else XXX_LICENSE_TYPE
static license_id searchLicenseType(license_id id) {
	int index=0;
	int indexValue=0;
	bool notFound=true;
	license_id returnValue=LIC_NULL;
	license_id const * idPointer;

	for(index=0; notFound && index < TOTAL_LICENSE_TYPES; index++) {
		int loopIndex=0;
		// we point to one of the license_id arrays
		idPointer = LICENSE_TYPES[index]; 
	
		while(idPointer[loopIndex] != LIC_NULL && idPointer[loopIndex] != id ) {
			loopIndex++;
		}
		if(idPointer[loopIndex] != LIC_NULL) {
			notFound=false;
			indexValue=index; // this is the license type ... XXX_LICENSE_TYPE
		}
	}
	if(notFound) {
		returnValue = LIC_NULL;
	} else {
		switch(indexValue) {
			case BUILD_LICENSE_TYPE :
				returnValue = LIC_BUILD;
				break;
			case DEVELOPER_LICENSE_TYPE :
				returnValue = LIC_DEVELOPER;
				break;
			case QUALITY_LICENSE_TYPE :
				returnValue = LIC_QUALITY;
				break;
			case ARCHITECT_LICENSE_TYPE :
				returnValue = LIC_ARCHITECT;
				break;
			case SERVER_LICENSE_TYPE :
				returnValue = LIC_SERVER;
				break;
			default:
				returnValue = LIC_NULL;
		}
	}	
	
	return(returnValue);
}

// This function returns 1 if the license_id is found and we have such a license type, else 0
static int containsFeature(license_id id) {
	int returnValue = 0;
	license_id newLicenseID = LIC_NULL;


	// identify which type of license this feature is in
	newLicenseID = searchLicenseType(id);

	// do we have this type of license?
	if(newLicenseID != LIC_NULL) {
		if((returnValue = _lf(newLicenseID)) == LIC_SUCCESS) {
			returnValue = 1;
		}
	}	

	return(returnValue);
}

extern "C" int _lc(void)
{
  int rc;
  char daemon[16] = { 'M' ^ 0xb4, 'K' ^ 0x98, 'S' ^ 0xa2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  if(usingSerialNumber()) {
		// lc_new_job returns 0 for success
		rc = 0;
  } else {
	  unscramble(daemon);
	  rc = lc_new_job((LM_HANDLE *)0, 0, &vendorcode, &lm_job);

	  if ( !rc ) 
		{
		  lc_set_attr(lm_job, LM_A_USER_EXITCALL, (LM_A_VAL_TYPE)quit);
		  lc_set_attr(lm_job, LM_A_USER_RECONNECT, (LM_A_VAL_TYPE)reconn);
		  lc_set_attr(lm_job, LM_A_USER_RECONNECT_DONE, (LM_A_VAL_TYPE)r_done);
		  lc_set_attr(lm_job, LM_A_CHECK_INTERVAL, (LM_A_VAL_TYPE)-1);
		  lc_set_attr(lm_job, LM_A_RETRY_COUNT, (LM_A_VAL_TYPE)5);
		  lc_set_attr(lm_job, LM_A_RETRY_INTERVAL, (LM_A_VAL_TYPE)-1);
		  lc_set_attr(lm_job, LM_A_MAX_TIMEDIFF, (LM_A_VAL_TYPE)27);
		}
	}

  return rc;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" int _lhb(void)
{
  static time_t   last_heartbeat = 0;
  static int      failures       = 0;
  
  char downmsg[32]  = { 'L' ^ 0xb4, 'i' ^ 0x98, 'c' ^ 0xa2, 'e' ^ 0x66, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'd' ^ 0xc8, 'o' ^ 0xc8, 'w' ^ 0xc8, 'n' ^ 0xc8, '!' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  char discover[32] = { 'D' ^ 0xb4, 'I' ^ 0x98, 'S' ^ 0xa2, 'C' ^ 0x66, 'O' ^ 0xc8, 'V' ^ 0xc8, 'E' ^ 0xc8, 'R' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  time_t tmp = time(0);

  if(!usingSerialNumber()) {
	  if (tmp - last_heartbeat > 120) 
		{
		  last_heartbeat = tmp;
		  if ( lc_heartbeat(lm_job, 0, 0) ) 
			{
			  ++failures;
			  unscramble(downmsg);
		  msg("$1", error_sev) << downmsg << eom;
			  if (failures >= LICENSE_RETRYS) 
				{
				  unscramble(discover);
				  quit(discover);
				}
			}
		}
	}

  return LIC_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" int _le(void)
{
  char *err_string;

  if(usingSerialNumber()) {
	char errorMessage[64] = { 'F' ^ 0xb4, 'a' ^ 0x98, 'i' ^ 0xa2, 'l' ^ 0x66, 'e' ^ 0xc8, 'd' ^ 0xc8, ' ' ^ 0xc8, 't' ^ 0xc8, 'o' ^ 0xc8, ' ' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'a' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'b' ^ 0xc8, 'a' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'd' ^ 0xc8, ' ' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, ' ' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'i' ^ 0xc8, 'a' ^ 0xc8, 'l' ^ 0xc8, ' ' ^ 0xc8, 'n' ^ 0xc8, 'u' ^ 0xc8, 'm' ^ 0xc8, 'b' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unscramble(errorMessage);
	msg("$1", error_sev) << errorMessage << eom;
  } else {
	err_string = lc_errstring(lm_job);
	char errorMessage[64] = { 'F' ^ 0xb4, 'a' ^ 0x98, 'i' ^ 0xa2, 'l' ^ 0x66, 'e' ^ 0xc8, 'd' ^ 0xc8, ' ' ^ 0xc8, 't' ^ 0xc8, 'o' ^ 0xc8, ' ' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unscramble(errorMessage);
	msg("$1 : $2", error_sev) << errorMessage << eoarg << err_string << eom;
  }

  return LIC_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

// check a license out

extern "C" int _lo(license_id id)
{
  int retval = LIC_FAIL;
  static char name[32], code[32], buffer[128];
  int rc = LIC_FAIL;
  license_id convertedLicenseID=LIC_NULL;
  license_record *lr = 0;

  if((convertedLicenseID = searchLicenseType(id)) != LIC_NULL) {
	lr = _lli(convertedLicenseID);
  }

  if(lr)
    {
      license_type t = lr->ltype;

	  if(usingSerialNumber()) {
		  if(checkSerialNumber(id) == LIC_SUCCESS) {
			retval = LIC_SUCCESS;
		  } else {
			  char message[64] = { 'C' ^ 0xb4, 'o' ^ 0x98, 'u' ^ 0xa2, 'l' ^ 0x66, 'd' ^ 0xc8, ' ' ^ 0xc8, 'n' ^ 0xc8, 'o' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, '(' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, ')' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    		  unscramble(message);

    		  strcpy(name, lr->print_name);
    		  unscramble(name);

    		  strcpy(code, lr->flexlm_id);
    		  unscramble(code);
		
			  sprintf(buffer, message, name, '\n', code);
			  msg("$1", warning_sev) << buffer << eom;
			  retval = LIC_FAIL;
		  }
	  } else {
		  if ( t == UNENFORCED )
			{
			  retval = LIC_SUCCESS;
			}
		  else
			{
			  static char flexlm_id[32]; 
			  strcpy(flexlm_id, lr->flexlm_id);
			  unscramble(flexlm_id);
			  int client_num = 1;
			  if ( t == COUNTED )
				{
				  client_num += lr->num_checked_out;
				}
			  rc = lc_checkout(lm_job, flexlm_id, VERSION, client_num, 
							   LM_CO_NOWAIT, &vendorcode, LM_DUP_NONE);

			  if ( t == WAIT && (rc == LM_MAXUSERS || rc == LM_USERSQUEUED) )
				{
				  char message[64] = { 'W' ^ 0xb4, 'a' ^ 0x98, 'i' ^ 0xa2, 't' ^ 0x66, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, '(' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, ')' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				  unscramble(message);

				  strcpy(name, lr->print_name);
				  unscramble(name);

				  strcpy(code, lr->flexlm_id);
				  unscramble(code);

				  sprintf(buffer, message, name, '\n', code);
			  msg("$1", warning_sev) << buffer << eom;
   
				  rc = lc_checkout(lm_job, flexlm_id, VERSION, client_num, 
								   LM_CO_WAIT, &vendorcode, LM_DUP_NONE);

				  if ( rc && _lh(id) == LIC_SUCCESS )
					rc = 0;
				}

			  if ( ! rc )
				{
				  lr->num_checked_out = client_num;
				  retval = LIC_SUCCESS;

				  LM_VD_FEATURE_INFO fi;
				  fi.feat = lc_auth_data (lm_job, flexlm_id);
				  if ( ! fi.feat )
					{
					  char message[64] = { 'C' ^ 0xb4, 'o' ^ 0x98, 'u' ^ 0xa2, 'l' ^ 0x66, 'd' ^ 0xc8, ' ' ^ 0xc8, 'n' ^ 0xc8, 'o' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'e' ^ 0xc8, 'a' ^ 0xc8, 't' ^ 0xc8, 'u' ^ 0xc8, 'r' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, '(' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, ')' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
					  unscramble(message);

					  strcpy(name, lr->print_name);
					  unscramble(name);

					  strcpy(code, lr->flexlm_id);
					  unscramble(code);

					  sprintf(buffer, message, name, '\n', code);
					  msg("$1", warning_sev) << buffer << eom;

					  fi.feat = lc_get_config (lm_job, flexlm_id);
					}
				  if ( fi.feat )
					rc = lc_get_attr(lm_job, LM_A_VD_FEATURE_INFO, (short *)&fi);
				  if ( fi.feat && !rc )
					{
					  int overdraft = ( fi.num_lic ? 
						 (fi.tot_lic_in_use - (fi.num_lic - fi.overdraft)) : 0);
					  if ( overdraft > 0 )
						{
						  char message[64] = { 'O' ^ 0xb4, 'v' ^ 0x98, 'e' ^ 0xa2, 'r' ^ 0x66, 'd' ^ 0xc8, 'r' ^ 0xc8, 'a' ^ 0xc8, 'f' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'o' ^ 0xc8, 'f' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 'd' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, '(' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, ')' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
						  unscramble(message);

						  strcpy(name, lr->print_name);
						  unscramble(name);

						  strcpy(code, lr->flexlm_id);
						  unscramble(code);

						  sprintf(buffer, message, overdraft, name, '\n', code);
						  msg("$1", warning_sev) << buffer << eom;
						}
					}
				}
			}
		}
    }
  
  return retval;
}

////////////////////////////////////////////////////////////////////////////////

// check a license in

extern "C" int _li(license_id id)
{
  int retval = LIC_FAIL;
  license_id convertedLicenseID=LIC_NULL;
  license_record *lr = 0;

  if((convertedLicenseID = searchLicenseType(id)) != LIC_NULL) {
	lr = _lli(convertedLicenseID);
  }

  if(lr)
    {
    	if(usingSerialNumber()) {
		retval = LIC_SUCCESS;
    	} else {
		  license_type t = lr->ltype;
		  if ( t == UNENFORCED )
			{
			  retval = LIC_SUCCESS;
			}
		  else if ( t != COUNTED || lr->num_checked_out > 0 )
			{
			  static char flexlm_id[32]; 
			  strcpy(flexlm_id, lr->flexlm_id);
			  unscramble(flexlm_id);
			  lc_checkin(lm_job, flexlm_id, 1);
			  lr->num_checked_out--;
			  if ( t == COUNTED && lr->num_checked_out > 0 )
				{
				  lc_checkout(lm_job, flexlm_id, VERSION, lr->num_checked_out, 
							  LM_CO_NOWAIT, &vendorcode, LM_DUP_NONE);
				}
			  retval = LIC_SUCCESS;
			}
		}
    }
  
  return retval;
}

////////////////////////////////////////////////////////////////////////////////

// does feature exist in license file?

extern "C" int _lf(license_id id)
{
  int retval = LIC_FAIL;
  license_id convertedLicenseID=LIC_NULL;
  license_record *lr = 0;

  if((convertedLicenseID = searchLicenseType(id)) != LIC_NULL) {
	lr = _lli(convertedLicenseID);
  }

  if(lr) {
	if(usingSerialNumber()) {
		if(checkSerialNumber(id) == LIC_SUCCESS) {
			retval = LIC_SUCCESS;
		} else {
			retval = LIC_FAIL;
		}
	} else {
		  license_type t = lr->ltype;
		  if ( t == UNENFORCED ) {
			  retval = LIC_SUCCESS;
		  } else {
			  static char flexlm_id[32]; 
			  strcpy(flexlm_id, lr->flexlm_id);
			  unscramble(flexlm_id);
			  if ( ! lc_checkout(lm_job, flexlm_id, VERSION, 1, 
								 LM_CO_LOCALTEST, &vendorcode, LM_DUP_NONE) ) {
				  retval = LIC_SUCCESS;
			  }
		  }
    	}
   }
   return retval;
}

////////////////////////////////////////////////////////////////////////////////

// have we already checked out this license?

extern "C" int _lh(license_id id)
{
  int retval = LIC_FAIL;
  license_id convertedLicenseID=LIC_NULL;
  license_record *lr = 0;

  if((convertedLicenseID = searchLicenseType(id)) != LIC_NULL) {
	lr = _lli(convertedLicenseID);
  }

  if(lr)
    {
      if(usingSerialNumber()) {
		retval = LIC_SUCCESS;
	  } else {
		  license_type t = lr->ltype;
		  if ( t == UNENFORCED ) {
			  retval = LIC_SUCCESS;
		  } else {
			  static char flexlm_id[32]; 
			  strcpy(flexlm_id, lr->flexlm_id);
			  unscramble(flexlm_id);
			  if ( ! lc_status(lm_job, flexlm_id) )
				{
				  retval = LIC_SUCCESS;
				}
		  }
	   }
    }
  
  return retval;
}

////////////////////////////////////////////////////////////////////////////////

extern "C" int _lm(license_id id)
{
  int retval = LIC_FAIL;
  license_id convertedLicenseID=LIC_NULL;
  license_record *lr = 0;

  if((convertedLicenseID = searchLicenseType(id)) != LIC_NULL) {
	lr = _lli(convertedLicenseID);
  }

  if(lr)
    {
      license_type t = lr->ltype;
      if ( t == UNENFORCED )
        {
          retval = LIC_SUCCESS;
        }
      else
        {
          char message[64] = 
          { 'U' ^ 0xb4, 'n' ^ 0x98, 'a' ^ 0xa2, 'b' ^ 0x66, 'l' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 't' ^ 0xc8, 'o' ^ 0xc8, ' ' ^ 0xc8, 'g' ^ 0xc8, 'e' ^ 0xc8, 't' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'f' ^ 0xc8, 'o' ^ 0xc8, 'r' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, '(' ^ 0xc8, 'L' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'd' ^ 0xc8, 'e' ^ 0xc8, ':' ^ 0xc8, ' ' ^ 0xc8, '%' ^ 0xc8, 's' ^ 0xc8, ')' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
          unscramble(message);
    
          char name[32];
          strcpy(name, lr->print_name);
          unscramble(name);
    
          char code[32];
          strcpy(code, lr->flexlm_id);
          unscramble(code);
    
          char buffer[128];
          sprintf(buffer, message, name, '\n', code);
          msg("$1", error_sev) << buffer << eom;
          retval = LIC_SUCCESS;
        }
    }
  
  return retval;
}  

////////////////////////////////////////////////////////////////////////////////
//
// Functions to do the license management by the id strings (mainly for Access)
//
////////////////////////////////////////////////////////////////////////////////

// This will get the license_id for a given string id
static int _gli(const char *s, license_id *id)
{
  int retval = LIC_FAIL;
  if ( s && id )
    {
      for ( int i = 0, done = 0; i < LIC_NUM_LICENSES && ! done; i++ )
        {
          char sid[4];
          strcpy(sid, license_data[i].code);
          unscramble(sid);
          if ( strcmp(sid, s) == 0 )
            {
              *id    = license_data[i].id;
              done   = 1;
              retval = LIC_SUCCESS;
            }
        }
    }

  return retval;
}

extern "C" int _los(const char *s)
{
  int retval = LIC_FAIL;
  license_id id;

  if ( s )
    if ( _gli(s, &id) == LIC_SUCCESS )
      retval = _lo(id);

  return retval;
}

extern "C" int _lis(const char *s) 
{
  int retval = LIC_FAIL;
  license_id id;

  if ( s )
    if ( _gli(s, &id) == LIC_SUCCESS )
      retval = _li(id);

  return retval;
}

extern "C" int _lfs(const char *s) 
{
  int retval = LIC_FAIL;
  license_id id;

  if ( s )
    if ( _gli(s, &id) == LIC_SUCCESS )
      retval = _lf(id);

  return retval;
}

extern "C" int _lhs(const char *s) 
{
  int retval = LIC_FAIL;
  license_id id;

  if ( s )
    if ( _gli(s, &id) == LIC_SUCCESS )
      retval = _lh(id);

  return retval;
}

extern "C" int _lms(const char *s) 
{
  int retval = LIC_FAIL;
  license_id id;

  if ( s )
    if ( _gli(s, &id) == LIC_SUCCESS )
      retval = _lm(id);

  return retval;
}

////////////////////////////////////////////////////////////////////////////////
//
// Callbacks for FLEXlm events back into DISCOVER
//
////////////////////////////////////////////////////////////////////////////////

static void reconn(char *, int, int, int)
{
  /*
   * No need to tell user about each re-connection attempt
   *
   */
}

static void r_done(char *, int)
{
  /*
   * No need to alarm user with a bunch of dialog boxes if the
   * reconnection was successful.  Only tell if unsuccessful.
   *
   */
}

static void quit(char *)
{
    char buffer[64] = 
      { 'L' ^ 0xb4, 'o' ^ 0x98, 's' ^ 0xa2, 't' ^ 0x66, ' ' ^ 0xc8, 'c' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, 'n' ^ 0xc8, 'e' ^ 0xc8, 'c' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'o' ^ 0xc8, 'n' ^ 0xc8, ' ' ^ 0xc8, 't' ^ 0xc8, 'o' ^ 0xc8, ' ' ^ 0xc8, 'l' ^ 0xc8, 'i' ^ 0xc8, 'c' ^ 0xc8, 'e' ^ 0xc8, 'n' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, ' ' ^ 0xc8, 's' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, 'v' ^ 0xc8, 'e' ^ 0xc8, 'r' ^ 0xc8, '!' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, 'E' ^ 0xc8, 'x' ^ 0xc8, 'i' ^ 0xc8, 't' ^ 0xc8, 'i' ^ 0xc8, 'n' ^ 0xc8, 'g' ^ 0xc8, ' ' ^ 0xc8, 'D' ^ 0xc8, 'I' ^ 0xc8, 'S' ^ 0xc8, 'C' ^ 0xc8, 'O' ^ 0xc8, 'V' ^ 0xc8, 'E' ^ 0xc8, 'R' ^ 0xc8, '.' ^ 0xc8, '%' ^ 0xc8, 'c' ^ 0xc8, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unscramble(buffer);
    msg("$1", error_sev) << buffer << eom;
    driver_exit(1);
}
