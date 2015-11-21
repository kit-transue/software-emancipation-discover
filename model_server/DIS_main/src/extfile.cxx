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
#include <cLibraryFunctions.h>
#include <machdep.h>
#include <externApp.h>
#include <externAttribute.h>
#include <proj.h>
#include <genString.h>


int extfile(char *cmd, char *type, char *name, symbolArr& roots)
{
  externInfo::load();

  externInfo* info = NULL;
  if(type) {
    info = (externInfo*) externInfo::find(type);
    if(!info){
      printf("wrong type\n");
      return -1;
    }
  }

  if(OSapi_strcmp(cmd, "list")==0) {
    roots = info->modules();
  } else if(OSapi_strcmp(cmd, "new")==0 && name) {
    genString physname;
    info->new_fname(name, physname);
    externFile *mod = info->new_extfile(name, physname);
    if(!mod){
       return -1;
    } else {
       roots.insert_last(mod);
    }
  } else {
    return -1;
  }
  return 0;
}


void getAllFilters(genString& filterlist)
{
  symbolArr results;
  int ret = extfile("list", "filter", NULL, results);
  if(ret == -1)   return;

  filterlist = "Default";
  symbolPtr sym;
  ForEachS (sym, results) {
     if (sym.isnotnull()) {
        char * nm = sym.get_name();
        if (nm) {
           char name[1024];
           OSapi_strcpy(name, nm);
           char *start = strrchr(name, '/');
           if(!start) return;
           start++;
           char *p;
           if((p = strrchr(start, '.')) != NULL) {
              char *profix = p+1;
              if(OSapi_strcmp(profix, "flt")==0) {
                 *p = '\0';
                 filterlist += " ";
                 filterlist += start;
              }
           }
        }
     }
  }
}


int getFilterPhyname(char *filter, char *type, genString& physicalnm)
{
  symbolArr results;
  int ret = extfile("list", "filter", NULL, results);
  if(ret == -1)   return -1;

  char fullname[1024];
  OSapi_strcpy(fullname, filter);
  OSapi_strcat(fullname, ".flt");

  symbolPtr sym;
  ForEachS (sym, results) {
     if (sym.isnotnull()) {
        char * nm = sym.get_name();
        if (nm) {
           char *start = strrchr(nm, '/');
           if(!start) 
              start = nm;
           else
              start++;
           if(OSapi_strcmp(start, fullname) == 0) {
              projNode *proj = sym.get_projNode();
              if(proj) {
                 proj->ln_to_fn(nm, physicalnm);        
                 return 0;
              }
           }
        }
     }
  }

  if(OSapi_strcmp(type, "save")==0) {
     results.removeAll();    
     ret = extfile("new", "filter", filter, results);
     if(ret == -1)
        return -1;

     ForEachS (sym, results) {
        if (sym.isnotnull()) {
           char * nm = sym.get_name();
           if (nm) {
              projNode *proj = sym.get_projNode();
              if(proj) {
                 proj->ln_to_fn(nm, physicalnm);
                 return 0;
              }
           }
        }
     }
  } 
  return -1;
}
  
