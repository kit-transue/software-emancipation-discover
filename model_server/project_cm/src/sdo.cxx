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

#include <stdlib.h>
#include <symbolArr.h>
#include <_Xref.h>
#include <XrefTable.h>
#include <proj.h>
#include <machdep.h>
#include <cLibraryFunctions.h>

extern "C" void connect_to_cockpit();
int get_modified_objects(symbolArr &selected, symbolArr &modified);

extern "C" int send_to_cockpit_ci(char *,char *);
extern "C" int send_to_cockpit_co(char *,char *);
extern "C" int send_to_cockpit_update(char *,char *);

extern "C" void connect_to_cockpit();
static char * str_of_files;
static char * str_of_ents;
int get_modified_objects(symbolArr &selected, symbolArr &modified);
void convert_triggers(symbolArr )
{
  
  static int once_only=-1;
  if(once_only==-1)
    {
      once_only=0;
      connect_to_cockpit();
    }
#if 0
  char * filestr=(char *)malloc(2000);
  char * entstr=(char *)malloc(2000);
  filestr[0]=0;
  entstr[0]=0;

    {
      symbolPtr sym_file;
      ForEachS(sym_file,sym_files)
	{
	  symbolArr mods;
	  symbolArr thr;
	  char number[100];
	  number[0]=0;

	  strcat(filestr,sym_file->get_name());
	  strcat(filestr,"|0|\n");
	  filestr=(char *)realloc(filestr,2000+strlen(filestr));


	  thr.insert_last(sym_file);
	  get_modified_objects(thr,mods);
	  symbolPtr sym;    
	  ForEachS(sym,mods)
	    {
	      SwtEntityStatus MOD_KIND=sym->get_attribute(SWT_Entity_Status,2);
	      symbolPtr def=sym->get_def_file();

	      strcat(entstr,sym->get_name());
	      strcat(entstr,"|");
	      OSapi_sprintf(number,"%d",sym->get_kind());
	      strcat(entstr,number);
	      strcat(entstr,"|");
	      strcat(entstr,def->get_name());
	      strcat(entstr,"|");
	      OSapi_sprintf(number,"%d",MOD_KIND);
	      strcat(entstr,number);
	      strcat(entstr,"|");
	      strcat(entstr,"0|\n");
	      entstr=(char *)realloc(entstr,2000+strlen(entstr));


	    } 
	}
    }
str_of_files=(char *)realloc(filestr,strlen(filestr)+1);
str_of_ents=(char *)realloc(entstr,strlen(entstr)+1);
#endif

}

extern "C" int send_to_cockpit_ci(char *,char *);
extern "C" int send_to_cockpit_co(char *,char *);
extern "C" int send_to_cockpit_update(char *,char *);
extern "C" int send_to_cockpit_unco(char *,char *);
extern "C" int send_to_cockpit_delete(char *,char *);
enum TriggerType {CODE_TRIGGER, FILE_TRIGGER};
char const *sdo_trigger_on = OSapi_getenv("SDO_TRIGGER");

void create_one_trigger(symbolArr & sa, genString & str, TriggerType wh, int lt = 0)
  /*
    0 for file and 1 for code
   */
{
  projNode *hpr = projNode::get_home_proj();
  Xref *Xr = hpr->get_xref();
  if (!Xr) return ;

  XrefTable *xrt = Xr->get_lxref();
  if (!xrt) return ;
  
  if (wh == FILE_TRIGGER) 
    xrt->file_trigger(sa, str);
  else
    xrt->code_trigger(sa, str, lt);
  if (lt)
    xrt->insert_sdo_update_time();

  return;
}

void sdo_delete_trigger(symbolArr &sym_files)
{
  if (!sdo_trigger_on) return;
  genString ent_str = " ";
  genString file_str = " ";
  
  create_one_trigger(sym_files, file_str, FILE_TRIGGER);
  create_one_trigger(sym_files, ent_str, CODE_TRIGGER);  
  convert_triggers(sym_files);
  send_to_cockpit_delete((char *)ent_str.str(), (char *)file_str.str());
}  

void sdo_unco_trigger(symbolArr &sym_files)
{
  if (!sdo_trigger_on) return;
  genString ent_str = " ";
  genString file_str = " ";
  
  create_one_trigger(sym_files, file_str, FILE_TRIGGER);
  create_one_trigger(sym_files, ent_str, CODE_TRIGGER);  
  convert_triggers(sym_files);
  send_to_cockpit_unco((char *)ent_str.str(), (char *)file_str.str());
}  


void sdo_ci_trigger(symbolArr &sym_files)
{
  if (!sdo_trigger_on) return;
  genString ent_str = " ";
  genString file_str = " ";
  
  create_one_trigger(sym_files, file_str, FILE_TRIGGER);
  create_one_trigger(sym_files, ent_str, CODE_TRIGGER);  
  convert_triggers(sym_files);
  send_to_cockpit_ci((char *)ent_str.str(), (char *)file_str.str());
}  
void sdo_co_trigger(symbolArr &sym_files)
{
  if (!sdo_trigger_on) return;  
  genString ent_str = " ";
  genString file_str = " ";
  
  create_one_trigger(sym_files, file_str, FILE_TRIGGER);
  convert_triggers(sym_files);
  send_to_cockpit_co((char *)ent_str.str(),(char *)file_str.str());
} 
