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
#include "xxinterface.h" /* for license checking */

#include <stdlib.h>

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
extern "C" int _lc(void)
{
  return 0;
}

extern "C" int _lhb(void)
{
  return 0;
}

extern "C" int _le(void)
{
  return LIC_SUCCESS;
}

extern "C" int _lo(license_id)
{
  return 0;
}

extern "C" int _li(license_id)
{
  return 0;
}

extern "C" int _lf(license_id)
{
   return LIC_SUCCESS;
}

extern "C" int _lh(license_id)
{
   return LIC_SUCCESS;
}

extern "C" int _lm(license_id)
{
   return LIC_SUCCESS;
}

extern "C" int _los(char const *)
{
   return LIC_SUCCESS;
}

extern "C" int _lis(char const *)
{
   return LIC_SUCCESS;
}

extern "C" int _lfs(char const *)
{
   return LIC_SUCCESS;
}

extern "C" int _lhs(char const *)
{
   return LIC_SUCCESS;
}

extern "C" int _lms(char const *)
{
   return LIC_SUCCESS;
}
