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
// cmd_dialog_names.h
//------------------------------------------
// synopsis:
//    A table of visible dialog names and corresponding internal names.
//
// description:
// ...
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files


// Table to match visible dialog and thier buttons names to internal UI names

 enum {
       DIALOG_TOP, 
       DIALOG_LIST, 
       DIALOG_BUTTON,
       DIALOG_END = -1
      };

 class DialogNames {
 public:
      int  d_type;                                // DIALOG_ /TOP/LIST or BUTTON type
      char displayed_name[CMD_MAX_NAME_LEN];
      char internal_name[CMD_MAX_NAME_LEN];
 };




 static DialogNames CMD_DIALOG_TABLE [] = 
 {
// -----------------------  dialogs ---------------------------------------
//
// The names bellow are relative to the corresponding top shell like browser,
//     project_browser, etc. and can be retrieved by XtNameToWidget(top, name);

    {DIALOG_TOP,  "Project-Files",                 "*project_browser"},
    {DIALOG_TOP,  "Open Project",                  "*open_project"},
    {DIALOG_TOP,  "New Project",                   "*prompt_popup.prompt"},
    {DIALOG_TOP,  "Filter Options",                "*filter_options"},

// -----------------------  dialog lists ---------------------------------------
//

    {DIALOG_LIST,  "Project-Files.list",            "*run_time_listSW.run_time_list"},
    {DIALOG_LIST,  "Open Project.list",             "*list_dialog_list"},
    {DIALOG_LIST,  "New Project.list",              "*list_dialog_list"},


// -----------------------  dialog buttons ---------------------------------------

    {DIALOG_BUTTON,  "All Files",                     "*show_all_files"},
    {DIALOG_BUTTON,  "Functions",                     "*contained_functions"},
    {DIALOG_BUTTON,  "Classes",                       "*contained_classes"},
    {DIALOG_BUTTON,  "Structs",                       "*contained_structs"},
    {DIALOG_BUTTON,  "Variables",                     "*contained_variables"},
    {DIALOG_BUTTON,  "BUTTONs",                         "*contained_BUTTONs"},
    {DIALOG_BUTTON,  "Places Where Called",           "*places_where_used"},
    {DIALOG_BUTTON,  "Unloaded Objects",              "*show_unloaded_objects"},
    {DIALOG_BUTTON,  "Loaded Objects",                "*show_loaded_objects"},


    {DIALOG_BUTTON,  "ok",                            "ok"},
    {DIALOG_BUTTON,  "cancel",                        "cancel"},
    
    {DIALOG_END,     "$END",                          "dummy"}
  };



/*

   START-LOG-------------------------------------------

   $Log: cmd_dialog_names.h  $
   Revision 1.1 1992/12/07 19:17:42EST builder 
   made from unix file
 * Revision 1.3  1992/12/08  00:19:52  sergey
 * Added dialog types.
 *
 * Revision 1.2  1992/12/04  22:02:15  sergey
 * Added more dialog and dialog buttons names.
 *
 * Revision 1.1  1992/12/03  19:18:17  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/






