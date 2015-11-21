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
#include <getResource.h>
#include <galaxy_undefs.h>

#include <cLibraryFunctions.h>
#include <machdep.h>

#include "prefs.h"
#include "customize.h"

// Client/Server communication includes.
#include "Application.h"
#include <../../DIS_ui/interface.h>
extern Application* DISui_app; 


void GetFolder(int viewId, vchar* listName)
{
   vstr *Folders;
   vresource res;
   if(!getResource("PrefFolder", &res)) {
       printf("The resource is not found.\n");
       return;
   }

   size_t numEntries = vresourceCountComponents(res);
   size_t i;
   const vname *tmpTag;
   genString englishName;
   bool first = true;

   for(i = 0; i < numEntries; i++) {
      tmpTag = vresourceGetNthTag(res, i);
      englishName = " {";
      englishName += (char*) tmpTag;
      englishName += "}";
    
      if (first) {
        Folders = vstrCloneScribed(vcharScribeLiteral((char*)englishName));
        first = false;
      } else 
        Folders = vstrAppendScribed(Folders, vcharScribeLiteral((char*)englishName));
   }

      // Send updated list to the client.
    vstr* cmdStr = vstrCloneScribed(vcharScribeFormat( (vchar*)"dis_setvar -view %d %s {%s}", viewId, listName, Folders) );

    rcall_dis_DISui_eval_async(DISui_app, cmdStr);
    vstrDestroy(cmdStr);
}


void GetValues(char * foldengname, int viewId, vchar* listName)
{
    const vchar *foldername = (vchar *)ReadFoldername(foldengname);
    vresource res, inerres;

    if(!getResource(foldername, &res)) {
       printf("The resource is not found.\n");
       return;
    }

   size_t numEntries = vresourceCountComponents(res);
   size_t i;
   const vname *tmpTag;
   int valtype;
   bool first = true;
   char *ename, *value;
   char prefName[1024];
   char result[1024];
   vstr *Values;

   for(i = 0; i < numEntries; i++) {
      tmpTag = vresourceGetNthTag(res, i);
      if (isEnglishName(tmpTag, prefName) ) {
         // Get the description.
         if (vresourceGetType(res, tmpTag) == vresourceSTRING) {
            ename = (char *) vresourceGetString(res, tmpTag);
         } else {
            printf("The English name is not found.\n");
            return;
         }

         // Get the preference value.
         value = customize::getStrPref(prefName);
         
         setdisplayvalue(ename, value, result);
      
         if(first){
            Values = vstrCloneScribed(vcharScribeLiteral(result) );
            first = false;
         } else 
            Values = vstrAppendScribed(Values, vcharScribeLiteral(result));
      }
   }

    // Send updated list to the client.
    vstr* cmdStr = vstrCloneScribed(vcharScribeFormat( (vchar*)"dis_setvar -view %d %s {%s}", viewId, listName, Values) );
    rcall_dis_DISui_eval_async(DISui_app, cmdStr);
    vstrDestroy(cmdStr);
}

void GetEname(char *selectedname, char *engname)
{
   while(*selectedname == ' ')
      selectedname++;
   while(*selectedname != ':')
      *engname++ = *selectedname++;
   *engname = '\0';
}


void GetSelectedValue(char *selectedname, char *retvalue)
{
   while(*selectedname != ':')
      selectedname++;
   selectedname++;
   while(*selectedname == ' ')
      selectedname++;
   strcpy(retvalue, selectedname);
}


void GetKeyname(char *foldengname,char *engname, char *keyname)
{
   const vchar *foldername = (vchar *)ReadFoldername(foldengname);
    vresource res, inerres;

    if(!getResource(foldername,&res)) {
       printf("Cannot find preference folder '%s' for '%s'.\n", foldername, foldengname);
       return;
    }

   size_t numEntries = vresourceCountComponents(res);
   size_t i;
   const vname *tmpTag;
   char prefName[1024], *ename;

   for(i = 0; i < numEntries; i++) {
      tmpTag = vresourceGetNthTag(res, i);
      if (isEnglishName(tmpTag, prefName)) {
         if (vresourceGetType(res, tmpTag) == vresourceSTRING) {
            ename = (char *) vresourceGetString(res, tmpTag);
            if(strcmp(ename, engname) == 0) {
               strcpy(keyname, prefName);
               return;
	    }
	 }
      }
   }
}


// This function returns the type of the value which is corresponding to 
// the English name. 
//    0 -- not found
//    1 -- integer
//    2 -- string
//    3 -- multiline string
int GetType(char *foldengname, char *engname)
{
    // With the text formatted preference file, there is only one type, STRING.
    return 2;
}

char *ReadFoldername(char *engname)
{
   vresource res;
   if(!getResource("PrefFolder", &res)) {
       printf("The resource is not found.\n");
       return NULL;
   }

   size_t numEntries = vresourceCountComponents(res);
   size_t i;
   const vname *tmpTag;
   char *foldername;

   for(i = 0; i < numEntries; i++)
   {
      tmpTag = vresourceGetNthTag(res, i);
      if(strcmp((char *)tmpTag, engname) == 0) 
         if (vresourceGetType(res, tmpTag) == vresourceSTRING) {
            foldername = (char *) vresourceGetString(res, tmpTag);
            return foldername;
	 }
   }
   return NULL;
}

vbool isEnglishName(const vchar *name, char *inname)
{
   char *nm = inname;
   if(strchr((char *)name, '!')) {
      while (*name != '!') 
        *inname++ = *name++; 
      *inname = '\0';
      return vTRUE;
   } else 
      return vFALSE;
}

void setdisplayvalue(char *Ename, char *value, char *rst)
{
   strcpy(rst,"{");
   strcat(rst, Ename);
   strcat(rst,":  ");
   strcat(rst, value);
   strcat(rst,"} ");
}

             



