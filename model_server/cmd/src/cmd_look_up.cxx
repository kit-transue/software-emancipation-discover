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
// cmd_look_up
//------------------------------------------
// synopsis:
//
// description:
//-----------------------------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files


#include "genError.h"
#include "cmd.h"
#include "cmd_menu_names.h"


   extern "C"   char *cmd_lookup(char *names[], int n_names, int *flag);

// returns Widget name - a minimum path which is enough to find the Widget
// input : array of pointers to names - the path for look_up table.

   char *cmd_lookup(char *names[], int n_names, int *return_flag)
   {
       char *item_name = NULL;
       int len, i, j, k;
       int curr_level;

       Initialize(cmd_lookup);  

       curr_level = -1;
       k = -1;
       for (i = 0; i < n_names; i++){
           len = strlen(names[i]);               // get the i-th name in path
           if (len > CMD_MAX_NAME_LEN)
               Error(ERR_INPUT);
       
//         search for next name in path from current position

           j = k;
           while( strcmp (CMD_MENU_TABLE[++j].displayed_name, "$END") != 0 ){
                if (curr_level >= CMD_MENU_TABLE[j].level)
                    return NULL;                 // name is not found - error ?
                if (strcmp(names[i], CMD_MENU_TABLE[j].displayed_name) == 0){

//                  not enough - can be many same names ( like "File" !);
//                  check if the parent is O.K

                    if ( i == 0 ||                       // no parents of the top one
                         CMD_MENU_TABLE[j].par_order == 0 ||    // ignore parent check
                         CMD_MENU_TABLE[j].level     == curr_level + 1){

                         k = j;                       // save found position
                         curr_level = CMD_MENU_TABLE[j].level;
                         break;
                    }
                }
           }
           if ( strcmp (CMD_MENU_TABLE[j].displayed_name, "$END") == 0 )
               return NULL;                 // name is not found - error ?
       }
       item_name    = CMD_MENU_TABLE[k].internal_name;
       *return_flag = CMD_MENU_TABLE[k].flag;

       return item_name;
   }



/*

   START-LOG-------------------------------------------

   $Log: cmd_look_up.C  $
   Revision 1.1 1993/07/28 16:28:07EDT builder 
   made from unix file
 * Revision 1.2  1993/01/26  22:57:20  sergey
 * Minor clean up.
 *
 *
 * Revision 1.5  1993/01/05  21:17:26  sergey
 * cmd_lookup() returns now flag showing if dialog is a direct child of aset.
 *
 * Revision 1.4  1993/01/04  21:11:37  sergey
 * Added level to MenuName class to handle same item names and for more input control.
 *
 * Revision 1.3  1992/12/23  15:59:05  sergey
 * Added extern "C" statement.
 *
 * Revision 1.2  1992/12/22  22:35:18  sergey
 * Replaced Error exit by NULL char * return.
 *
 * Revision 1.1  1992/12/21  21:53:02  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/






