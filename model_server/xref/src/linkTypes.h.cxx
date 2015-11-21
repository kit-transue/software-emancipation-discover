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
#include <stdarg.h>
#include <linkTypes.h>

char linkTypes::local_search[linkTypes::MAX_LINKS];
char linkTypes::search_from[linkTypes::MAX_LINKS];

linkTypes::linkTypes()
{
  for (int i = 0; i < MAX_LINKS; i++)
    links[i] = find[i] = 0;
}


void linkTypes::reset(linkTypes & la)
{
  for (int i = 0; i < MAX_LINKS; i++) {
    links[i] = la.get_link(i);
    find[i] = 0;
  }
}

linkTypes::operator linkType()
{
  linkType lt = lt_next;
  for (int i = 0; i < MAX_LINKS; i++) {
    if (links[i]) {
      lt = (linkType) i;
      break;
    }
  }
  return lt;
}
int linkTypes::contains(linkType l1, ...)
{
  int ret = 0;
  va_list ap;
  va_start(ap, l1);
  linkType lt = l1;
  while (lt) {
    if (contain(lt)) {
      ret = 1;
      break;
    }
#if 0
// warning: 'linkType' is promoted to 'int' when passed through '...'
// warning: (so you should pass 'int' not 'linkType' to 'va_arg')
// note: if this code is reached, the program will abort
    lt = va_arg(ap, linkType);
#else
    lt = (linkType)va_arg(ap, int);
#endif
  }
  va_end(ap);
  return ret;
}

int linkTypes::init_local_search()
{
  int i;
  for (i = 0; i < MAX_LINKS; i++)
    local_search[i] = 0;

  local_search[is_using] = 1;  
  local_search[is_defining ] = 1;  
  local_search[is_defined_in ] = 1;  
  local_search[is_including ] = 1;  
  local_search[member_of ] = 1;  
  local_search[lt_lmd ] = 1;  
  local_search[lt_lmdhigh ] = 1;  
  local_search[lt_attribute ] = 1;  
  local_search[file_ref ] = 1;  
  local_search[has_type ] = 1;  
  local_search[has_superclass ] = 1;  
  local_search[have_friends ] = 1;  
  local_search[have_arg_type ] = 1;  
  local_search[instances_of_assocType] = 1;
    local_search[has_vpub_superclass] = 1;
    local_search[has_vpri_superclass] = 1;
    local_search[has_pub_superclass] = 1;
    local_search[has_pri_superclass] = 1;
    local_search[has_vpro_superclass] = 1;
    local_search[has_pro_superclass] = 1;
  local_search[grp_has_peer] = 1;
  local_search[grp_is_peer_of] = 1;
  local_search[grp_has_server] = 1;
  local_search[grp_has_client] = 1;
  local_search[grp_has_pub_mbr] = 1;
  local_search[grp_has_pri_mbr] = 1;
  local_search[grp_has_trans_mbr] = 1;
  local_search[sym_of_assoc] = 1;

  
  for (i = 0; i < MAX_LINKS; i++)
    if (local_search[i] == 1)
      search_from[i] = FROM_DEF_SYMBOLS;
    else
      search_from[i] = FROM_ALL_SYMBOLS;

  search_from[has_type] = FROM_ANY_SYMBOLS;
  search_from[has_c_proto] = FROM_ANY_SYMBOLS;
  search_from[have_arg_type] = FROM_ANY_SYMBOLS;
  search_from[has_property] = FROM_ANY_SYMBOLS;

  return 1;
}
int linkTypes::local_only() 
{	
  int ret = 1;
  for (int i = 0; i < MAX_LINKS && ret; ++i) {
    if (links[i]) { 
      if (local_search[i] && !find[i])
	ret = 0;
      else if (!local_search[i])
	ret = 0;
    }
//  else
//	links[i] = 0;
  }
  return ret;
}
