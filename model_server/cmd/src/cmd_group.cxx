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
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "cmd_internal.h"
#define genENUM_SRC
#include "cmd_group.h" 
#include "msg.h"

static CMD_VALIDATION_GROUP cmd_validation_group = CMD_NOTHING;

void cmd_group_print(ostream& ostr)
{
  int sz;
  genEnum_item* arr;
  CMD_VALIDATION_GROUP_name_data(&arr, &sz);
  for(int ii=0; ii<sz; ++ii){
     ostr << arr[ii].idx_name + 4 << '\n';
  }
}

extern "C" int cmd_set_validation_group(char* name)
{
  genString nm;
  nm.printf("CMD_%s", name);
  
  CMD_VALIDATION_GROUP  group = CMD_VALIDATION_GROUP_get_item(nm);
  if(group < 0){  // not found
    genString msg;
    msg.printf("unknown group: %s\n", name);
    cmd_validation_error((char*)msg);
    cmd_group_print(cout);
    cmd_validation_group = CMD_EVERYTHING;
  } else {
    cmd_validation_group = group;
  }
  return group;
}

extern "C" int cmd_do_validation(CMD_VALIDATION_GROUP group)
{
  if(cmd_execute_journal_name)
     return 1;
  if(!cmd_current_journal_name)
     return 0;
  if(cmd_validation_group == CMD_NOTHING)
     return 0;

  if(cmd_validation_group == group) 
     return 1;

  if(cmd_validation_group == CMD_EVERYTHING)
     return 1;
 
  return 0;
}

extern "C" int cmd_group_test()
{
     cmd_current_journal_name = " ";
     CMD_VALIDATION_GROUP_prt();
     int i1 = cmd_set_validation_group("smt");
     int i2 = cmd_set_validation_group("SMT");
     const char * nm = CMD_VALIDATION_GROUP_get_name(i2);
     int ii1 = cmd_do_validation(CMD_DD);
     int ii2 = cmd_do_validation(CMD_SMT);
     int ii3 = 0;
     int ii4 = 0;
     CMD_GROUP(SMT){
       ii3 = 1;
     }
     CMD_GROUP(DD){
       ii4 = 2;
     }
     msg("cmd_group_test: $1 $2 $3 $4$5$6$7\n") << i1 << eoarg << i2 << eoarg << nm << eoarg << ii1 << eoarg << ii2 << eoarg << ii3 << eoarg << ii4 << eom;
    
     return i2;
}

/*
  $Log: cmd_group.C  $
  Revision 1.4 2000/12/01 14:06:37EST ktrans 
  More conversions of old messages to msg()
 * Revision 1.5  1994/07/21  17:46:33  builder
 * proper iostream.h and cmd_internal.h included.
 *
 * Revision 1.4  1994/07/20  20:16:52  mg
 * Bug track: NA
 * restructured for logging
 *
 * Revision 1.3  1994/03/06  16:30:26  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.2  1994/02/23  21:11:34  builder
 * *** empty log message ***
 *
 * Revision 1.1  1994/02/23  14:06:51  mg
 * Initial revision
 *
*/

