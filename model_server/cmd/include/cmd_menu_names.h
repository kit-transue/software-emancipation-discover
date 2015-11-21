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
// cmd_menu_names.h
//---------------------------------------
// synopsis:
//    A table of visible menu names and corresponding internal names.
//
// description:
// ...
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files

// Table to match visible menu and button names to internal UI names
//
// The internal_name not exactly the name of the widget of the UI but
// in case of arrays of widgets with same names contains the index of
// widget.
// The table reflects the hierarchichal nature of the UI when the same
// button (like menubar) may appear under different parents.

 class MenuNames {
 public:
      char displayed_name[CMD_MAX_NAME_LEN];
      char internal_name[CMD_MAX_NAME_LEN];
      int  level;
      int  flag;                               // if =  0, a child of "browser"
                                               //    = -1, a child of "aset"
                                               //    =  1, top level
      int  par_order;                          //    =  1, parent level should be
                                               //          equal exactly (level -1)
                                               //    =  0, parent level just < level
 };





 static MenuNames CMD_MENU_TABLE [] = 
 {

      {"BrowserShell",                        "*browser",0,1, 0},
      
      // Dialogs  - listed alphabetacally

         {"Associations",                     "*Association Editor",1,-1,1},
              {"Hard",                        "*button_frame*hard",2,0,1},   
              {"sash",                        "*sash",2,0,1},
              {"Soft",                        "*button_frame*soft",2,0,1},   

              {"Category ",                   "*category_list",2,0,1},   
              {"Category Editor",             "*category",2,0,0},   
              {"Create",                      "*create",2,0,1},   
              {"Rename",                      "*rename",2,0,1},   
              {"Delete",                      "*delete",2,0,1},   
              {"Change",                      "*change",2,0,1},   
              {"Revert",                      "*revert",2,0,1},   

              {"Description",                 "*form.texteditSW",2,0,0},
                   {"Description Editor",     "*textedit",3,0,0},
                   {"Vertical Scrollbar",     "*vbar",3,0,0},

              {"Rename Instance",             "*rename_instance",2,0,0},   
              {"Delete Instance",             "*delete_instance",2,0,0},   
              {"Remove Instance",             "*remove_instance",2,0,0},   

              {"Types",                       "*form(1).list",2,0,0},
                   {"List",                   "*type_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Associations",                "*form(2).list(1)",2,0,0},
                   {"List",                   "*instance_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Files",                       "*form(2).list(2)",2,0,0},
                   {"List",                   "*file_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Objects",                     "*form(2).list(3)",2,0,0},
                   {"List",                   "*object_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},


         {"Configurator",                     "*Configurator_popup",1,-1,1},
              {"SetUp",                       "*form",2,0,0},   
                {"Path",                      "*frame_form.form(1).string_editor",3,0,0},
                {"Stat",                      "*frame_form.form(2).string_editor",3,0,0},
                {"Lock",                      "*frame_form.form(3).string_editor",3,0,0},
                {"Unlock",                    "*frame_form.form(4).string_editor",3,0,0},
                {"Commit",                    "*frame_form.form(5).string_editor",3,0,0},
                {"Maker",                     "*frame_form.form(6).string_editor",3,0,0},
                {"Make",                      "*frame_form.form(7).string_editor",3,0,0},
                {"Import",                    "*frame_form.form(8).string_editor",3,0,0},
                {"Export",                    "*frame_form.form(9).string_editor",3,0,0},
                {"Diff",                      "*frame_form.form(10).string_editor",3,0,0},

//              Allow to skip "Set Up" which is not that visibly clear parent

                {"OK",                        "*ok",3,0,0},
                {"Store",                     "*store",3,0,0},
                {"Reset",                     "*reset",3,0,0},
                {"Cancel",                    "*cancel",3,0,0},

         {"Create Association",               "*create_association_popup",1,-1,0},
              {"Types",                       "*form.list(1)",2,0,0},
                   {"List",                   "*type_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Files",                       "*form.list(2)",2,0,0},
                   {"List",                   "*file_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Objects",                     "*form.list(3)",2,0,0},
                   {"List",                   "*object_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Name Editor",                 "*name",2,0,0},
              {"Comment Editor",              "*comment",2,0,0},
              {"OK",                          "*ok",2,0,1},
              {"Lock",                        "*lock",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},

         {"Define Class",                     "*Define_Class_popup",1,-1,0},
              {"Header File Directory",       "*header_dir",2,0,1},
              {"Source File Directory",       "*source_dir",2,0,1},
              {"Class Name",                  "*class_name",2,0,1},
              {"Class Name Aliases",          "*class_name",2,0,1},
              {"Description",                 "*description",2,0,1},
              {"Responsibilities",            "*responsibility",2,0,1},
              {"Search Noun-List",            "*alias_search",2,0,1},

              {"OK",                          "*Define_Class*ok",2,0,1},
              {"Apply",                       "*Define_Class*apply",2,0,1},
              {"Cancel",                      "*Define_Class*cancel",2,0,1},
              {"Help",                        "*Define_Class*help",2,0,1},

         {"Define Relation",                  "*Define_Relation_popup",1,-1,0},     // src/targ names are different from visible !!
              {"Relation Name",               "*dr_relation_name",2,0,1},
              {"Inverse Relation",            "*dr_inverse_relation",2,0,1},
              {"One to One",                  "*dr_one_to_one",2,0,1},
              {"One to Many",                 "*dr_one_to_many",2,0,1},
              {"Many to One",                 "*dr_many_to_one",2,0,1},
              {"Many to Many",                "*dr_many_to_many",2,0,1},
              {"Required",                    "*dr_required",2,0,1},
              {"Source Member",               "*dr_src_member",2,0,1},
              {"Source Header",               "*dr_src_header",2,0,1},
              {"Source Definition",           "*dr_src_definition",2,0,1},
              {"Target Member",               "*dr_trg_member",2,0,1},
              {"Target Header",               "*dr_trg_header",2,0,1},
              {"Target Definition",           "*dr_trg_definition",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},

         {"Define Struct",                    "*Define_Struct_popup",1,-1,0},
              {"Header File Directory",       "*header_dir",2,0,1},
              {"Struct Name",                 "*struct_name",2,0,1},
              {"Description",                 "*description",2,0,1},
              {"Responsibilities",            "*responsibility",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},

         {"Error",                            "*Error_popup",1,-1,0},
              {"OK",                          "*OK",2,0,1},
              {"Help",                        "*Help",2,0,1},


         {"Parser Error",                     "*select_from_rtl_popup",1,-1,0}, 
// ... more
              {"Show",                        "*remove_project.apply",2,0,1},
              {"Done",                        "*remove_project.cancel",2,0,1},
              {"Help",                        "*remove_project.help",2,0,1},


         {"Filter List",                      "*list_utils_popup",1,0,0},
              {"Show entries matching regexp", 
                                              "\0",2,0,0},
              {"Hide entries matching regexp", 
                                              "\0",2,0,0},
              {"Format list entries by field and width",
                                              "\0",2,0,0},
                {"Object Name",               "\0",3,0,0},

              {"Predefined Filters",          "\0",2,0,0},
                {"Configure",                 "\0",3,0,0},
                {"Generic",                   "\0",3,0,0},

              {"Sort By",                     "\0",2,0,0}, 
                {"Formatted String",          "\0",3,0,0}, 
                {"Ascending",                 "\0",3,0,0},

              {"OK",                          "*ok",2,0,0},
              {"Apply",                       "*apply",2,0,0},
              {"Cancel",                      "*cancel",2,0,0},


         {"Create File",                      "*file_language_popup",1,0,0},
              {"External Files",              "*file_area_unix",2,0,0},   
              {"In Project",                  "*file_area_project",2,0,0},   

              {"C Source Code",               "*file_lang_c",2,0,0},
              {"C++ Source Code",             "*file_lang_cplusplus",2,0,0},
              {"Structured Text",             "*file_lang_ste",2,0,0},
              {"Makefile",                    "*file_lang_makefile",2,0,0},
              {"Raw Text",                    "*file_lang_raw",2,0,0},

              {"Enter complete path name",    "*form.answer",2,0,0},


              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


          {"Create Directory",                "*file_language_popup",1,0,0},
              {"Enter directory name",        "*form.answer",2,0,0},

              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


          {"Load Layout",                     "*prompt_popup",1,0,1},
              {"Load layout from the",        "*answer",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"New Project",                      "*prompt_popup",1,0,0},
              {"Enter name for new project",  "*form.answer",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"New Scrapbook",                    "*prompt_popup",1,0,0},
              {"Enter a name for the scrapbook",
                                              "*form.answer",2,0,1},
              {"Cancel",                      "*prompt*cancel",2,0,1},
              {"OK",                          "*prompt*ok",2,0,1},
              {"Help",                        "*prompt*help",2,0,1},

         {"New Subsystem",                    "*prompt_popup",1,0,0},
              {"Enter a name for the subsystem", 
                                              "*form.answer",2,0,1},
              {"Cancel",                      "*prompt*cancel",2,0,1},
              {"OK",                          "*prompt*ok",2,0,1},
              {"Help",                        "*prompt*help",2,0,1},

         {"Open Project",                     "*open_project_popup",1,0,0},
              {"List",                        "*list_dialog_list",2,0,0},
              {"Horizontal Scrollbar",        "*listhScrollBar",2,0,0},
              {"Vertical Scrollbar",          "*listvScrollBar",2,0,0},
              {"OK",                          "*open_project.ok",2,0,1},
              {"Cancel",                      "*open_project.cancel",2,0,1},
              {"Help",                        "*open_project.help",2,0,1},

         {"Preferences",                      "*preferences_popup",1,-1,1},
              {"Installation Directory",      "*frame(1).frame_form.form(1).string_editor",2,0,0},
              {"Shadow Directory",            "*frame(1).frame_form.form(2).string_editor",2,0,0},
              {"Default Window Layout",       "*frame(1).frame_form.form(3).string_editor",2,0,0},
              {"Makefile directory",          "*frame(1).frame_form.form(4).string_editor",2,0,0},
              {"GDB Debugger",                "*frame(1).frame_form.form(5).string_editor",2,0,0},

              {"C Language",                  "*frame(2).dummy",2,0,0},
                 {"#define (-D Flags)",       "*frame(2).frame_form.form(1).string_editor",3,0,0},
                 {"#include (-l Flags)",      "*frame(2).frame_form.form(2).string_editor",3,0,0},
                 {"File Suffix",              "*frame(2).frame_form.form(4).string_editor",3,0,0},
                 {"Other Flags",              "*frame(2).frame_form.form(4).string_editor",3,0,0},
                 {"Output Style",             "*frame(2).frame_form.form(5).dummy",3,0,0},
                   {"ANSII",                  "*one",4,0,0},
                   {"K&R",                    "*two",4,0,0},

              {"C++ Language",                "*frame(3).dummy",2,0,0},
                 {"#define (-D Flags)",       "*frame(3).frame_form.form(1).string_editor",3,0,0},
                 {"#include (-l Flags)",      "*frame(3).frame_form.form(2).string_editor",3,0,0},
                 {"File Suffix",              "*frame(3).frame_form.form(3).string_editor",3,0,0},
                 {"Other Flags",              "*frame(3).frame_form.form(4).string_editor",3,0,0},

              {"Structured Text",             "*frame(4).dummy",2,0,0},
                 {"Full Suffix",              "*frame(4).frame_form.form(1).string_editor",3,0,0},

              {"Cross-Reference Exclude Directories", 
                                    "*frame(5).frame_form.exclude_dirs_listSW.exclude_dir_list",2,0,0},

              {"OK",                          "*ok",2,0,1},
              {"Store",                       "*store",2,0,1},
              {"Reset",                       "*reset",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},


          {"Put Message",                     "*DataEntry_popup",1,-1,0},
              {"Enter message regarding your modification",
                                              "*form*data_text",2,0,0},

              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"Query",                            "*Query",1,-1,1},
              {"New",                         "*new", 2, 0,1},
              {"Load",                        "*load",2,0,1},
              {"Save",                        "*save",2,0,1},
              {"Language",                    "*language",2,0,1},
              {"Case Insensitive",            "*case_insensitive",2,0,1},
              {"sash",                        "*sash",2,0,1},

              {"Clear Domain",                "*clear_domain",2,0,1},
              {"Move Results",                "*move_results",2,0,1},
              {"Clear Results",               "*clear_results",2,0,1},

              {"Query",                       "*text_form",2,0,1},
                  {"Query Editor",            "*query_language_editor",3,0,0},
                  {"Vertical Scrollbar",      "*vbar",3,0,0},

              {"Domain",                      "*domain_form",2,0,1},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,1},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,1},
                  {"List",                    "*domain",3,0,1},
                  {"Filter List",             "*list_status",3,0,1},

              {"Results",                     "*result_form",2,0,1},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,1},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,1},
                  {"List",                    "*results",3,0,1},
                  {"Filter List",             "*list_status",3,0,1},

              {"Execute",                     "*execute",2,0,1},
              {"Done",                        "*done",2,0,1},
              {"Help",                        "*help",2,0,1},

          {"Question Popup",                  "*Question_popup",1,-1,0},
              {"OK",                          "*Question*OK",2,0,1},
              {"Cancel",                      "*Question*Cancel",2,0,1},
              {"Help",                        "*Question*Help",2,0,1},

  
          {"Remove Project",                  "*remove_project_popup",1,0,0},
              {"List",                        "*list_dialog_list",2,0,1},
              {"Cancel",                      "*remove_project.cancel",2,0,1},
              {"OK",                          "*remove_project.ok",2,0,1},
              {"Help",                        "*remove_project.help",2,0,1},


          {"Save Files",                      "*save_files_popup",1,0,0},
              {"Select From Modified Entities","*list(1)",2,0,0},
                   {"List",                   "*modified_list",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},
              {"These Entities Will Be Saved","*list(2)",2,0,0},
                   {"List",                   "*save_list",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},
              {"Save",                        "*apply",2,0,0},
              {"Quit",                        "*apply",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},


          {"Save Layout",                     "*prompt_popup",1,0,1},
              {"Save layout from the",        "*answer",2,0,1},
              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},

          {"Scanned Projects",                "*scanned_projects_popup",1,0,0},
              {"Horizontal Scrollbar",        "*ListhScrollBar",2,0,0},
              {"Vertical Scrollbar",          "*ListvScrollBar",2,0,0},
              {"List",                        "*rtl",2,0,0},
              {"Filter List",                 "*list_status",2,0,0},

              {"Remove",                      "*ok",2,0,1},
              {"Done",                        "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},

// utility->extract subsystem

          {"Set Weights",                     "*set-weights_popup",1,0,0},
              {"Function call",               "*Function call",2,0,0},
              {"Data reference",              "*Data reference",2,0,0},
              {"Instance of",                 "*Instance of",2,0,0},
              {"Argument type",               "*Argument type",2,0,0},
              {"Return type",                 "*Return Type",2,0,0},
              {"Contains",                    "*Contains",2,0,0},
              {"Friend of",                   "*Friend of",2,0,0},
              {"Subclass of",                 "*Subclass of",2,0,0},
              {"Member of",                   "*Member of",2,0,0},
              {"Threshold",                   "*Threshold",2,0,0},
              {"Ignore",                      "*ignore",2,0,0},
              {"Leave alone",                 "*leave-alone",2,0,0},
              {"Include",                     "*include",2,0,0},

              {"Apply",                       "*go",2,0,1},
              {"Undo",                        "*undo",2,0,1},
              {"Close",                       "*done",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},



          {"System Messages",                 "*System_Messages_popup",1,-1,0},
              {"Main Options",                "*main_options",2,0,0},
                        {"Create Log Files",  "*create_log_files",3,0,0},
                        {"Quick Help",        "*quick_help",3,0,0},

              {"Message Filters",             "*filter_options",2,0,0},
                        {"Error",             "*show_errors",3,0,1},
                        {"Warning",           "*show_warnings",3,0,1},
                        {"Informative",       "*show_informative",3,0,1},
                        {"Diagnostic",        "*show_diagnostics",3,0,1},

              {"Job Categories",              "*job_options",2,0,0},
                        {"Importing Files",   "*show_import_files",3,0,0},
                        {"Compilation",       "*show_compilation",3,0,0},
                        {"Propagation",       "*show_propagation",3,0,0},

              {"OK",                          "*ok",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Help",                        "*help",2,0,1},

  
      // Menus

         {"Browser",                          "*browser",1,0,1},

              {"About Paraset",               "*browser_menu.browser_about_paraset",2,0,1},
              {"New Browser",                 "*browser_menu.browser_new_browser_window",2,0,1},
              {"Close",                       "*browser_menu.browser_close_window",2,0,1},
              {"Load Layout",                 "*browser_menu.browser_load_layout",2,0,1},
              {"Save Layout",                 "*browser_menu.browser_save_layout",2,0,1},
              {"Preferences",                 "*browser_menu.browser_preferences",2,0,1},
              {"Configurator",                "*browser_menu.browser_configurator",2,0,1},
              {"Messages",                    "*browser_menu.browser_messages",2,0,1},
              {"Quit ParaSET",                "*browser_menu.browser_quit",2,0,1},


         {"File",                             "*file",1,0,1},
              {"Save All Files",              "*file_menu.file_save_all_files",2,0,1}, 
              {"New File",                    "*file_menu.file_new_file",2,0,1},
              {"New Directory",               "*file_menu.file_new_directory",2,0,1},
              {"Delete",                      "*file_menu.file_delete",2,0,1},
              {"Load",                        "*file_menu.file_preload",2,0,1},
              {"Unload",                      "*file_menu.file_unload",2,0,1},
              {"Language",                    "*file_menu.file_language",2,0,1},
              {"Update Directories",          "*file_menu.file_update_directories",2,0,1},


         {"Manage",                           "*project",1,0,1},
              {"Get",                         "*project_menu.project_get",2,0,1},
              {"Put",                         "*project_menu.project_put",2,0,1},
              {"Lock",                        "*project_menu.project_lock",2,0,1},
              {"Unlock",                      "*project_menu.project_unlock",2,0,1},
              {"Make",                        "*project_menu.project_make",2,0,1}, 
              {"Update Makefile",             "*project_menu.project_update_make",2,0,1}, 
              {"New Project",                 "*project_menu.project_new_project",2,0,1}, 
              {"Scanned Projects",            "*project_menu.project_scanned_projects",2,0,1}, 
              {"Home Project",                "*project_menu.project_home_project",2,0,1}, 


         {"View",                             "*view",1,0,1},
              {"New Viewer",                  "*view_new_viewer",2,0,1},
              {"Text",                        "*view_text",2,0,1},
              {"Outline",                     "*view_outline",2,0,1},
              {"Tree",                        "*view_call_tree",2,0,1},
              {"Inheritance",                 "*view_inheritance",2,0,1},
              {"Relations",                   "*view_relationships",2,0,1},
              {"Structures",                  "*view_structures",2,0,1},
              {"Subsystems",                  "*view_subsystems",2,0,1},


         {"Utility",                          "*utility",1,0,1},
              {"Query",                       "*utility_menu.utility_query",2,0,1},
              {"Association Editor",          "*utility_menu.utility_association_editor",2,0,1},
              {"Error Browser",               "*utility_menu.utility_error_browser",2,0,1},
              {"Extract Subsystems",          "*utility_menu.utility_extract_subsystems",2,0,1},
              {"Find Dormant Code",           "*utility_menu.utility_find_dormant_code",2,0,1},
              {"New Struct",                  "*utility_menu.utility_new_struct",2,0,1},
              {"New Class",                   "*utility_menu.utility_new_class",2,0,1},
              {"New Relation",                "*utility_menu.utility_new_relation",2,0,1},
              {"New Subsystem",               "*utility_menu.utility_new_subsystem",2,0,1},


         {"Journal",                          "*journal",1,0,1},
              {"New Journal",                 "*journal_menu.journal_new_journal",2,0,1},
              {"Insert Command",              "*journal_menu.journal_insert_command",2,0,1},
              {"Save Journal",                "*journal_menu.journal_save_journal",2,0,1},
              {"Save Journal As",             "*journal_menu.journal_save_journal_as",2,0,1},
              {"Execute",                     "*journal_menu.journal_execute_journal",2,0,1},

         {"Debug",                            "*debug",1,0,1},
              {"Break",                       "*debug_break",2,0,1},
              {"Print Node",                  "*debug_print_selected_node",2,0,1},
              {"Follow",                      "*debug_follow",2,0,1},
                   {"0",                      "*debug_follow_0",3,0,0},
                   {"1",                      "*debug_follow_1",3,0,0},
                   {"2",                      "*debug_follow_2",3,0,0},
                   {"3",                      "*debug_follow_3",3,0,0},
                   {"3",                      "*debug_follow_4",3,0,0},
                   {"5",                      "*debug_follow_5",3,0,0},
                   {"6",                      "*debug_follow_6",3,0,0},
                   {"7",                      "*debug_follow_7",3,0,0},
                   {"8",                      "*debug_follow_8",3,0,0},
                   {"9",                      "*debug_follow_9",3,0,0},
              {"Print Tree",                  "*debug_print_selected_tree",2,0,1},
              {"Start Metering",              "*debug_start_metering",2,0,1},
              {"Stop Metering",               "*debug_stop_metering",2,0,1},
              {"Suspend Metering",            "*debug_suspend_metering",2,0,1},
              {"Resume Metering",             "*debug_resume_metering",2,0,1},
              {"Edit Help Index",             "*debug_edit_help_index",2,0,1},
              {"Reset Help Index",            "*debug_reset_help_index",2,0,1},
              {"Enable Quickhelp",            "*debug_enable_quickhelp",2,0,1},
              {"Toggle Help Pathnames",       "*debug_toggle_help_pathnames",2,0,1},
              {"Clear All Selections",        "*debug_clear_all_selections",2,0,1},


         {"Help",                             "*help",1,0,1},
              {"Topics",                      "*help_topics",2,0,1},
              {"On Files",                    "*help_on_files",2,0,1},
              {"On Projects",                 "*help_on_projects",2,0,1},
              {"Index",                       "*help_index",2,0,1},


         {"Directory List",                   "*file_browser*file_browser",1,0,0},
              {"Horizontal Scrollbar",        "*list*ListhScrollBar",2,0,0},
              {"Vertical Scrollbar",          "*list*ListvScrollBar",2,0,0},
              {"List",                        "*list*directory",2,0,0},
              {"Filter List",                 "*list*list_status",2,0,0},

         {"Directory Editor",                 "*file_browser*gt_string_ed",1,0,0},

         {"Filter Files",                     "*file_browser*file_browser*list_status",1,0,0},

         {"Project Editor",                   "*project_browser*gt_string_ed",1,0,0},

// project browser ---------------------------------------------------------------------

         {"Browse",                           "*project_controls*browse",1,0,0},
         {"Scan",                             "*project_controls*scan",1,0,0},
         {"Up",                               "*project_buttons*up",1,0,0},
         {"Reset",                            "*project_buttons*reset",1,0,0},
         {"New Group",                        "*project_buttons*new_group",1,0,0},

         {"Project List",                     "*project_browser*project_browser",1,0,0},
              {"Vertical Scrollbar",          "*hierarchy_mode*list*ListvScrollBar",2,0,0},
              {"Horizontal Scrollbar",        "*hierarchy_mode*list*ListhScrollBar",2,0,0},
              {"List",                        "*hierarchy_mode*list*proj_list",2,0,0},
              {"Filter List",                 "*hierarchy_mode*list*list_status",2,0,0},

         {"Group",                            "*xref_query*list(1)",1,0,0},
              {"Vertical Scrollbar",          "*group_listSW*ListvScrollBar",2,0,1},
              {"Horizontal Scrollbar",        "*group_listSW*ListhScrollBar",2,0,1},
              {"List",                        "*group_list",2,0,1},
              {"Filter List",                 "*list_status",2,0,1},

         {"Domain",                           "*xref_query*list(2)",1,0,0},
              {"Vertical Scrollbar",          "*domain_listSW*ListvScrollBar",2,0,1},
              {"Horizontal Scrollbar",        "*domain_listSW*ListhScrollBar",2,0,1},
              {"List",                        "*domain_list",2,0,1},
              {"Filter List",                 "*list_status",2,0,1},

         {"Show",                             "*xref_query*list(3)",1,0,0},
              {"Vertical Scrollbar",          "*show_listSW*ListvScrollBar",2,0,1},
              {"Horizontal Scrollbar",        "*show_listSW*ListhScrollBar",2,0,1},
              {"List",                        "*show_list",2,0,1},
              {"Filter List",                 "*list_status",2,0,1},

         {"Result",                           "*xref_query*list(4)",1,0,0},
              {"Vertical Scrollbar",          "*result_listSW*ListvScrollBar",2,0,1},
              {"Horizontal Scrollbar",        "*result_listSW*ListhScrollBar",2,0,1},
              {"List",                        "*result_list",2,0,1},
              {"Filter List",                 "*list_status",2,0,1},


/*-------------------------  Viewer Shell ----------------------------- */

      {"ViewerShell",                         "*ViewerShell",0,1,0},

//    Dialogs :

         {"Assign Category",                  "*Single_Select_List_Name0",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select Category From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},

              {"ok",                          "*ok",2,0,1},
              {"apply",                       "*apply",2,0,1},
              {"cancel",                      "*cancel",2,0,1},

         {"Assign Style",                     "*Single_Select_List_Name0",1, -1,0},
              {"Style",                       "*name_text",2,0,0},
              {"Select Style From",           "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,1},
              {"apply",                       "*apply",2,0,1},
              {"cancel",                      "*cancel",2,0,1},

         {"Change Arguments",                 "*Change_Propagator_popup",1, -1,1},
              {"Objects To Operate",          "*objForm",2,0,0},
                  {"Operation",               "*Operation_cascadeBtn",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"List",                    "*operandList",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},           

                  {"Declaration",             "*object_data_field",3,0,0},           

              {"Objects To Update",           "*propForm",2,0,0},
                  {"Propagate",               "*Propagate_cascadeBtn",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"List",                    "*propList",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},           

                  {"Value",                   "*propagation_data_field",3,0,0},           

              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"Create Association",               "*create_association_popup",1,-1,0},
              {"Types",                       "*form.list(1)",2,0,0},
                   {"List",                   "*type_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Files",                       "*form.list(2)",2,0,0},
                   {"List",                   "*file_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Objects",                     "*form.list(3)",2,0,0},
                   {"List",                   "*object_rtl",3,0,0},
                   {"Horizontal Scrollbar",   "*ListhScrollBar",3,0,0},
                   {"Vertical Scrollbar",     "*ListvScrollBar",3,0,0},
                   {"Filter List",            "*list_status",3,0,0},

              {"Name Editor",                 "*name",2,0,0},
              {"Comment Editor",              "*comment",2,0,0},
              {"OK",                          "*ok",2,0,1},
              {"Lock",                        "*lock",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},


         {"Create Category",                  "*RTL_name_based_upon",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select style for the category", 
                                              "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},

         {"Create Style",                     "*RTL_name_based_upon",1, -1,0},
              {"Style",                       "*name_text",2,0,0},
              {"Based Upon Style",            "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,1},
              {"apply",                       "*apply",2,0,1},
              {"cancel",                      "*cancel",2,0,1},


         {"Debugger Executable",              "*FileSelector",1, 0,1},
              {"Filter",                      "*fsb_filter_text",2,0,1},

              {"Directories",                 "*fsb_dir_listSW",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"List",                    "*fsb_dir_list",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},           
              {"Files",                       "*sb_listSW",2,0,1},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,1},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,1},
                  {"List",                    "*sb_list",3,0,1},
                  {"Filter List",             "*list_status",3,0,1},           
              {"Command Line Arguments",      "*file_arguments",2,0,0},              
              {"OK",                          "*OK",2,0,1},
              {"Filter",                      "*Apply",2,0,1},
              {"Cancel",                      "*Cancel",2,0,1},
              {"Help",                        "*Help",2,0,1},


         {"Delete",                           "*Change_Propagator_popup",1, -1,1},
              {"Objects To Operate",          "*objForm",2,0,0},
                  {"Operation",               "*Operation_cascadeBtn",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"List",                    "*operandList",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},           

              {"Objects To Update",           "*propForm",2,0,0},
                  {"Propagate",               "*Propagate_cascadeBtn",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"List",                    "*propList",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},           

              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"Deassign Category",                "*Single_Select_List_Name0",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select Category From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,1},
              {"apply",                       "*apply",2,0,1},
              {"cancel",                      "*cancel",2,0,1},


         {"Deassign Style",                   "*Single_Select_List_Name0",1, -1,0},
              {"Style",                       "*name_text",2,0,0},
              {"Select Style From",           "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"Hide Category",                    "*RTL_multiple_select",1, -1,0},
              {"Select category to turn off", "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"appply",                      "*appply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"Impact Analyzer",                  "*impact_analysis_popup",1, -1,0},
              {"Files",                       "*form.list(1)",2,0,0},
                  {"List",                    "*file_rtl",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},

              {"To be changed",               "*form.list(2)",2,0,0},
                  {"List",                    "*hard_object_rtl",3,0,0},
                  {"Horizontal Scrollbar",    "*ListhScrollBar",3,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"Filter List",             "*list_status",3,0,0},

              {"Replace with",                "*string_editor",2,0,0},   

              {"Ok",                          "*ok",2,0,0},
              {"Local",                       "*local",2,0,0},
              {"Load",                        "*load",2,0,0},
              {"Lock",                        "*lock",2,0,0},
              {"Show",                        "*show",2,0,0},
              {"Cancel",                      "*cancel",2,0,0},


         {"Modify Category",                  "*STE_style_to_cat",1, 0,0},
              {"categories",                  "*form1",2,0,0},
                  {"Category Editor",         "*cat_text",3,0,0},   
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*catList",3,0,0},

              {"styles",                      "*form1",2,0,0},
                  {"Style Editor",            "*style_text",3,0,0},   
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*styleList",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"appply",                      "*appply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},

         {"Print View in Postscript",         "*PROJ_View_Pref_popup",1,0,0},
              {"File",                        "*file_toggle",2,0,1},
              {"File Editor",                 "*filename",2,0,1},
              {"Printer",                     "*printer_toggle",2,0,1},
              {"Printer Editor",              "*printername",2,0,1},
              {"Compute Pages",               "*compute_pages",2,0,1},
              {"Fit to page",                 "*fit_to_page",2,0,1},
              {"Landscape",                   "*ls_toggle",2,0,1},
              {"Scale",                       "\0",2,0,1},
              {"Rows",                        "*rows",2,0,1},
              {"Columns",                     "*columns",2,0,1},
              {"Letter",                      "*popup_std_paper_size_menu",2,0,1},
                  {"Custom",                  "*user_defined",3,0,1},
                  {"Letter",                  "*letter",3,0,1},
                  {"Legal",                   "*legal",3,0,1},
                  {"Ledger",                  "*ledger",3,0,1},

              {"Inches",                      "*popup_units_menu",2,0,1},
                  {"Centimeters",             "*Centimeters",3,0,1},
                  {"Inches",                  "*Inches",3,0,1},
                  {"Pixels",                  "*ps_pixels",3,0,1},

              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*appply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},

         {"Question Popup",                   "*Question_popup",1,-1,0},
              {"OK",                          "*Question*OK",2,0,0},
              {"Cancel",                      "*Question*Cancel",2,0,0},
              {"Help",                        "*Question*Help",2,0,1},

         {"Remove Category",                  "*Single_Select_List_Name0",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select Category From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},

         {"Remove Style",                     "*Single_Select_List_Name0",1, -1,0},
              {"Style",                       "*name_text",2,0,0},
              {"Select Style From",           "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"Search",                           "*quick_search_popup",1, -1,1},
              {"Search Pattern",              "*search_text",2,0,1},
              {"Case Insentive",              "*insensitive",2,0,1},
              {"Find Any",                    "*toggle_reset",2,0,1},
              {"Variable",                    "*vdecl",2,0,1},
              {"Function",                    "*fdecl",2,0,1},
              {"Parameter",                   "*pdecl",2,0,1},           
              {"Class/Struct",                "*cdecl",2,0,1},           
              {"Enumeration",                 "*edecl",2,0,1},
              {"Comment",                     "*comment",2,0,1},
              {"Hard Association",            "*hassoc",2,0,1},
              {"Soft Association",            "*sassoc",2,0,1},

              {"First",                       "*next",2,0,1},
              {"Last",                        "*prev",2,0,1},
              {"Done  ",                      "*done",2,0,1},
              {"Help",                        "*help",2,0,1},


         {"Select Category",                  "*RTL_multiple_select",1, -1,0},
              {"Select Category to Highlight",  
                                              "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"appply",                      "*appply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},

         {"Set Active Category",              "*Single_Select_List_Name0",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select Category From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"Show Category",                    "*RTL_multiple_select",1, -1,0},
              {"Select category to turn on",  "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"appply",                      "*appply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"Show Call Tree",                   "*Single_Select_List_Name0",1, -1,0},
              {"Function",                    "*name_text",2,0,0},
              {"Select Function From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},

         {"Unset Active Category",            "*Single_Select_List_Name0",1, -1,0},
              {"Category",                    "*name_text",2,0,0},
              {"Select Category From",        "*scrolledWindow1",2,0,0},
                  {"Vertical Scrollbar",      "*ListvScrollBar",3,0,0},
                  {"List",                    "*scrolledList1",3,0,0},
              {"ok",                          "*ok",2,0,0},
              {"apply",                       "*apply",2,0,0},
              {"cancel",                      "*cancel",2,0,0},


         {"View Preferences",                 "*PROJ_View_Pref_popup",1, -1,1},
              {"Magnification factor",        "*scale_scrollbar",2,0,0},

              {"OK",                          "*ok",2,0,1},
              {"Apply",                       "*apply",2,0,1},
              {"Cancel",                      "*cancel",2,0,1},
              {"Help",                        "*help",2,0,1},


//     Menus


         {"File",                             "*file",1,0,0},

              {"Open Selected",               "*file_menu.file_open",2,0,1},
              {"Open ERD",                    "*file_menu.file_open_erd",2,0,1},
              {"Open Data Chart",             "*file_menu.file_open_dc",2,0,1},
              {"Open Call Tree",              "*file_menu.file_open_calltree",2,0,1},
              {"Save",                        "*file_menu.file_save",2,0,1},
              {"Save As",                     "*file_menu.file_save_as",2,0,1},
              {"Save Window Setup",           "*file_menu.file_save_btn_bar",2,0,1},
              {"Close Current View",          "*file_menu.file_close_view",2,0,1},
              {"Print Current View",          "*file_menu.file_print",2,0,1},
              {"Close Window",                "*file_menu.file_close_window",2,0,1},

         {"Edit",                             "*edit",1,0,1},

              {"Undo",                        "*edit_menu.edit_undo",2,0,1},
              {"Cut",                         "*edit_menu.edit_cut",2,0,1},
              {"Copy",                        "*edit_menu.edit_copy",2,0,1},
              {"Paste",                       "*edit_menu.edit_paste",2,0,1},
              {"Delete",                      "*edit_menu.edit_delete",2,0,1},
              {"Reference",                   "*edit_menu.edit_refer",2,0,1},
              {"Search",                      "*edit_menu.edit_search",2,0,1},
              {"Replace",                     "*edit_menu.edit_replace",2,0,1},
              {"Rename",                      "*edit_menu.edit_rename",2,0,1},
              {"Change Arguments",            "*edit_menu.edit_change_args",2,0,1},
              {"Formatted",                   "*edit_menu.edit_formatted",2,0,1},
              {"Unformatted",                 "*edit_menu.edit_unformatted",2,0,1},
              {"Clear Selection",             "*edit_menu.edit_clear_selection",2,0,1},

         {"View",                             "*view",1,0,1},

              {"Collapse",                    "*view_menu.view_collapse",2,0,1},     
              {"Expand",                      "*view_menu.view_expand",2,0,1},     
              {"Collapse To",                 "*view_menu.view_collapse_level",2,0,1},     
                   {"1st Level",              "*view_collapse_level_b1",3,0,0},
                   {"2nd Level",              "*view_collapse_level_b2",3,0,0},
                   {"3rd Level",              "*view_collapse_level_b3",3,0,0},
                   {"4th Level",              "*view_collapse_level_b4",3,0,0},
                   {"5th Level",              "*view_collapse_level_b5",3,0,0},
              {"Expand To",                   "*view_menu.view_expand_level",2,0,1},     
                   {"1st Level",              "*view_expand_level_b1",3,0,0},
                   {"2nd Level",              "*view_expand_level_b2",3,0,0},
                   {"3rd Level",              "*view_expand_level_b3",3,0,0},
                   {"4th Level",              "*view_expand_level_b4",3,0,0},
                   {"5th Level",              "*view_expand_level_b5",3,0,0},
              {"Zoom In",                     "*view_menu.view_magnify",2,0,1},     
              {"Zoom Out",                    "*view_menu.view_shrink",2,0,1},     
              {"Zoom To Fit",                 "*view_menu.view_zoomtofit",2,0,1},     
              {"Reset Zoom",                  "*view_menu.view_reset",2,0,1},     
              {"Split Viewer",                "*view_menu.view_split",2,0,1},     
              {"Remove Viewer",               "*view_menu.view_remove",2,0,1},     
              {"Show Mode Buttons",           "*view_menu.view_mode_buttons",2,0,1},     
              {"Show Custom Buttons",         "*view_menu.view_button_bar",2,0,1},     
              {"Decorate Source",             "*view_menu.view_decorate_source",2,0,1},     
              {"View Preferences",            "*view_menu.view_pref",2,0,1},     

         {"Annotation",                       "*annotation",1,0,1},

              {"Note",                        "*annotation_menu.annotation_note",2,0,1},
                   {"Insert",                 "*annotation_note_menu.annotation_note_insert",3,0,0},
                   {"Edit",                   "*annotation_note_menu.annotation_note_edit",3,0,0},
                   {"Delete",                 "*annotation_note_menu.annotation_note_delete",3,0,0},
              {"Delete All Nodes",            "*annotation_menu.annotation_del_notes",2,0,1},
              {"Next Mark",                   "*annotation_menu.annotation_next",2,0,1},
              {"Previous Mark",               "*annotation_menu.annotation_previous",2,0,1},
              {"Traverse Marks",              "*annotation_menu.annotation_traverse",2,0,1},
                   {"Search Hits",            "*annotation_traverse_search_hits",3,0,0},
                   {"Items To Change",        "*annotation_traverse_changes",3,0,0},
                   {"Execution Points",       "*annotation_traverse_exec",3,0,0},
                   {"Breakpoints",            "*annotation_traverse_brkpt",3,0,0},
                   {"Compiler Errors",        "*annotation_traverse_errors",3,0,0},
              {"Show Keywords",               "*annotation_menu.annotation_show_keywords",2,0,1},
              {"Edit Keywords",               "*annotation_menu.annotation_edit_keywords",2,0,1},
              {"Create Keywords",             "*annotation_menu.annotation_create_keyword",2,0,1},
              {"Create Indexed Words",        "*annotation_menu.annotation_create_indexhit",2,0,1},

         {"Link",                             "*link",1,0,1},

              {"Hotlink",                     "*link_menu.link_hotlink",2,0,1},
              {"Next Section",                "*link_menu.link_next_section",2,0,1},
              {"Previous Section",            "*link_menu.link_previous_section",2,0,1},
              {"Start Hotlink",               "*link_menu.link_start_hotlink",2,0,1},
              {"Start Section Link",          "*link_menu.link_start_section_link",2,0,1},
              {"Close Link",                  "*link_menu.link_close_link",2,0,1},
              {"Remove Hyper Links",          "*link_menu.link_remove_links",2,0,1},
              {"Soft Associate",              "*link_menu.link_soft_associate",2,0,1},
              {"Hard Associate",              "*link_menu.link_hard_associate",2,0,1},
              {"Create Association",          "*link_menu.link_user_associate",2,0,1},
              {"Remove Hard Association",     "*link_menu.link_remove_association",2,0,1},
              {"Association Editor",          "*link_menu.link_association_editor",2,0,1},

         {"Debugger",                         "*debug",1,0,1},

              {"Run with Arguments",          "*debug_menu.debug_arguments",2,0,1},
              {"Set Program",                 "*debug_menu.debug_executable",2,0,1},
              {"Attach To Process",           "*debug_menu.debug_attach",2,0,1},
              {"Interpreted Functions",        "*debug_menu.debug_interpret_functions",2,0,1},
              {"Interpreted Files",           "*debug_menu.debug_interpret_files",2,0,1},
              {"Trace Interpreted Code",      "*debug_trace_menu",2,0,1},
                   {"0",                      "*debug_trace_menu.debug_trace_0",3,0,0},
                   {"2",                      "*debug_trace_menu.debug_trace_2",3,0,0},
                   {"4",                      "*debug_trace_menu.debug_trace_4",3,0,0},
                   {"6",                      "*debug_trace_menu.debug_trace_6",3,0,0},
                   {"8",                      "*debug_trace_menu.debug_trace_8",3,0,0},
                   {"10",                     "*debug_trace_menu.debug_trace_10",3,0,0},
                   {"12",                     "*debug_trace_menu.debug_trace_12",3,0,0},
                   {"14",                     "*debug_trace_menu.debug_trace_14",3,0,0},
                   {"16",                     "*debug_trace_menu.debug_trace_16",3,0,0},
                   {"18",                     "*debug_trace_menu.debug_trace_18",3,0,0},
                   {"20",                     "*debug_trace_menu.debug_trace_20",3,0,0},
              {"Breakpoints",                 "*debug_breakpoint_menu",2,0,1},
                   {"At Selection",           "*debug_brkpt_at_selection",3,0,0},
                   {"In Function",            "*debug_brkpt_in_function",3,0,0},
                   {"Show",                   "*debug_breakpoint_menu.debug_show_brkpts",3,0,0},
                   {"Remove All",             "*debug_breakpoint_menu.debug_remove_brkpts",3,0,0},
                   {"Remove Selected",        "*debug_breakpoint_menu.debug_remove_brkpt",3,0,0},
                   {"Enable Selected",        "*debug_breakpoint_menu.debug_enable_brkpt",3,0,0},
                   {"Disable Selected",       "*debug_breakpoint_menu.debug_disable_brkpt",3,0,0},
              {"Watch Variables",             "*debug_menu.debug_show_watchvars",2,0,0},
              {"Kill Process",                "*debug_menu.debug_kill",2,0,0},
              {"Quit Debugger",               "*debug_menu.debug_toggle",2,0,0},
              {"Start Debugger",              "*debug_menu.debug_toggle",2,0,0},

         {"Help",                             "*help",1,0,1},

              {"Topics",                      "*help_menu.help_general",2,0,1},
              {"On Viewers",                  "*help_menu.help_on_viewershell",2,0,1},
              {"Index",                       "*help_menu.help_index",2,0,1},

         {"Category",                         "*Category",1,0,1},

              {"Create",                      "*popup_Category_menu.Category_menu.Create",2,0,1},
              {"Assign",                      "*popup_Category_menu.Category_menu.Assign",2,0,1},
              {"De-assign",                   "*popup_Category_menu.Category_menu.De-assign",2,0,1},
              {"Modify",                      "*popup_Category_menu.Category_menu.Modify",2,0,1},
              {"Select",                      "*popup_Category_menu.Category_menu.Select",2,0,1},
              {"Show",                        "*popup_Category_menu.Category_menu.Show",2,0,1},
              {"Hide",                        "*popup_Category_menu.Category_menu.Hide",2,0,1},
              {"Remove",                      "*popup_Category_menu.Category_menu.Remove",2,0,1},
              {"Set Active",                  "*popup_Category_menu.Category_menu.Set Active",2,0,1},
              {"Unset Active",                "*popup_Category_menu.Category_menu.Unset Active",2,0,1},

         {"Style",                            "*Style",1,0,1},

              {"Create",                      "*Style_menu.Create",2,0,1},
              {"Assign",                      "*Style_menu.Assign",2,0,1},
              {"De-assign",                   "*Style_menu.De-assign",2,0,1},
              {"Modify",                      "*Style_menu.Modify",2,0,1},
              {"Remove",                      "*Style_menu.Remove",2,0,1},
              {"Indentation",                 "*Style_menu.Indentation",2,0,1},
                   {"Indented",               "*popup_submenu0.submenu0.Indented",3,0,0},
                   {"Exdented",               "*popup_submenu0.submenu0.Exdented",3,0,0},
                   {"K & R Style",            "*popup_submenu0.submenu0.K & R Style",3,0,0},

         {"Insert",                           "*Insert",1,0,1},

              {"Variable",                    "*Insert_menu.Variable",2,0,1},
              {"Case",                        "*Insert_menu.Case",2,0,1},
              {"Loop",                        "*Insert_menu.Loop",2,0,1},
              {"If",                          "*Insert_menu.If",2,0,1},
              {"Statement",                   "*Insert_menu.Statement",2,0,1},

         {"Aliases",                          "*Aliases",1,0,1},

              {"Highlighted",                 "*Aliases_menu.Highlighted",2,0,1},
              {"All Nouns",                   "*Aliases_menu.All Nouns",2,0,1},
              {"Created Class",               "*Aliases_menu.Create Class",2,0,1},

         {"General",                          "*General",1,0,1},

              {"Highlight",                   "*General_menu.Highlight",2,0,1},
                   {"Character",              "*popup_submenu0.submenu1.Character",3,0,1},
                   {"Word",                   "*popup_submenu0.submenu1.Word",3,0,1},
              {"Title<->Paragraph",           "*General_menu.Title<->Paragraph",2,0,1},
              {"Show Marks",                  "*General_menu.Show Marks",2,0,1},

         {"Display",                          "*Display",1,0,1},

              {"Related Entities",            "*Display_menu.Related Entities",2,0,0},
              {"Members",                     "*Display_menu.Members",2,0,0},
              {"Sort Members",                "*Display_menu.Sort Members",2,0,0},
              {"Expert Mode",                 "*Display_menu.Expert Mode",2,0,0},

              {"Attributes",                  "*Display_menu(2).Attributes",2,0,0},
              {"Operations",                  "*Display_menu(2).Operations",2,0,0},
              {"Built-in Relations",          "*Display_menu(2).Built-in Relations...",2,0,0},
              {"Relations",                   "*Display_menu(2).Relations...",2,0,0},
              {"Members",                     "*Display_menu(2).Members...",2,0,0},
              {"Add Classes",                 "*Display_menu(2).Add Classes",2,0,0},
              {"Add new class",               "*Display_menu(2).Add new class",2,0,0},
              {"Remove Class",                "*Display_menu(2).Remove Class",2,0,0},
     
              {"Select Related",              "*Display_menu(3).Select Related...",2,0,0},
              {"Select Contents",             "*Display_menu(3).Select Contents...",2,0,0},
              {"Sort Contents",               "*Display_menu(3).Sort Contents...",2,0,0},
     
              {"Select Related",              "*Display_menu(4).Select Related...",2,0,0},
              {"Select Contents",             "*Display_menu(4).Select Contents...",2,0,0},
              {"Sort Contents",               "*Display_menu(4).Sort Contents...",2,0,0},
     
              {"Attributes",                  "*Display_menu(5).Attributes",2,0,0},
              {"Operations",                  "*Display_menu(5).Operations",2,0,0},
              {"Built-in Relations",          "*Display_menu(5).Built-in Relations...",2,0,0},
              {"Relations",                   "*Display_menu(5).Relations...",2,0,0},
              {"Members",                     "*Display_menu(5).Members...",2,0,0},
              {"Add Structs",                 "*Display_menu(5).Add Structs",2,0,0},
              {"Add New Struct",              "*Display_menu(5).Add New Struct",2,0,0},
              {"Remove Struct",               "*Display_menu(5).Remove Struct",2,0,0},


         {"Modify",                           "*Modify",1,0,1},

              {"Change Attributes",           "*Modify_menu.Change Attributes",2,0,1},
              {"Add Member",                  "*Modify_menu.Add Member",2,0,1},
              {"Add Superclass",              "*Modify_menu.Add Superclass",2,0,1},
              {"Add Subclass",                "*Modify_menu.Add Subclass",2,0,1},
              {"Add Relation",                "*Modify_menu.Add Relation",2,0,1},


         {"viewer",                           "*viewWindow.viewer",1,0,1},
                   {"Reparse",                "*reparse_button",2,0,1},
                   {"Horizontal Scrollbar",   "*HorizScrollBar",2,0,1},
                   {"Vertical Scrollbar",     "*VertScrollBar",2,0,1},
                   {"View Options",           "*viewOptionsPane",2,0,1},
                   {"view",                   "*viewArea",2,0,1},


         {"Sash",                             "*viewWindow*sash",1,0,1},

// buttons ( customized )

         {"Add",                              "*custom_buttons_controls.customize_mode",1,0,1},
         {"Remove",                           "*custom_buttons_controls.remove_mode",1,0,1},
         {"Normal",                           "*custom_buttons_controls.normal_mode",1,0,1},

         {"Run",                              "*buttons_form*debug_run",1,0,1},
         {"Step",                             "*buttons_form*debug_step",1,0,1},
         {"Next",                             "*buttons_form*debug_next",1,0,1},
         {"Continue",                         "*buttons_form*debug_continue",1,0,1},
         {"Jump",                             "*buttons_form*debug_jump",1,0,1},
         {"Finish",                           "*buttons_form*debug_finish",1,0,1},
         {"Up",                               "*buttons_form*debug_up",1,0,1},
         {"Down",                             "*buttons_form*debug_down",1,0,1},
         {"Where",                            "*buttons_form*debug_where",1,0,1},
         {"Halt",                             "*buttons_form*debug_halt",1,0,1},
         {"Stop at",                          "*buttons_form*debug_stop_at",1,0,1},
         {"Print",                            "*buttons_form*debug_print",1,0,1},

         {"Search",                           "*buttons_form*hyper_search",1,0,1},
         {"Back",                             "*buttons_form*hyper_back",1,0,1},
         {"History",                          "*buttons_form*hyper_history",1,0,1},
         {"Previous",                         "*buttons_form*hyper_previous",1,0,1},
         {"Next",                             "*buttons_form*hyper_next",1,0,1},

         {"Edit",                             "*modeButtonSlot.base_mode",1,0,1},
         {"1:1",                              "*modeButtonSlot.relmode_1_1",1,0,1},
         {"1:n",                              "*modeButtonSlot.relmode_1_many",1,0,1},
         {"n:1",                              "*modeButtonSlot.relmode_many_1",1,0,1},
         {"n:n",                              "*modeButtonSlot.relmode_many_many",1,0,1},

         {"Thumb",                            "*panner_frame.Panner.thumb",1,0,1},
         {"Status",                           "*status_line",1,0,1},

      



    {"$END",                          "dummy",0,0}  
  };




/*

   START-LOG-------------------------------------------

   $Log: cmd_menu_names.h  $
   Revision 1.2 1994/07/08 17:53:43EDT builder 
   
 * Revision 1.40  1994/06/28  15:08:23  andrea
 * Bug track: 7456
 * fixed some typos
 *
 * Revision 1.39  1993/03/24  00:55:57  sergey
 * Reflected recent UI changes. Added more dialogs.
 *
 * Revision 1.38  1993/02/02  20:25:52  sergey
 * Added more parent level flags.
 *
 * Revision 1.37  1993/01/29  22:08:13  sergey
 * Added more parent level control; some spelling corrections.
 *
 * Revision 1.36  1993/01/26  22:55:13  sergey
 * Added parent levels to more dialogs.
 *
 * Revision 1.35  1993/01/26  16:42:52  sergey
 * Modifed "Query" dialog according new UI, added "Print View..", changed "viewer" level.
 *
 * Revision 1.34  1993/01/25  15:43:09  sergey
 * Added a button to Define Relation; corrected Scan path in Browser.
 *
 * Revision 1.33  1993/01/22  01:29:28  sergey
 * Added 4 dialogs.
 *
 * Revision 1.32  1993/01/21  22:34:07  sergey
 * Corrected more dialogs; added a member data par_order to MenuNames class to deal
 * with the use of same names in different menus and dialogs.
 *
 * Revision 1.31  1993/01/21  00:41:10  sergey
 * More dialogs added and newest UI changes merged.
 *
 * Revision 1.30  1993/01/19  19:57:46  sergey
 * Added "Query", "Select Category", and "Extract Subsystem" dialogs; minor corrections.
 *
 * Revision 1.29  1993/01/18  23:02:18  sergey
 * Added "error" dialog.
 *
 * Revision 1.28  1993/01/18  22:02:14  sergey
 * Modified according to the UI changes; added "Put Message" and "Create Directory" dialogs.
 *
 * Revision 1.27  1993/01/16  00:21:04  sergey
 * Added more stuff; spelling correction.
 *
 * Revision 1.26  1993/01/15  16:21:37  oak
 * Reversed Category and Category Editor.
 *
 * Revision 1.25  1993/01/15  15:06:26  oak
 * Added new dialogs.
 *
 * Revision 1.24  1993/01/14  20:34:48  sergey
 * More stuff added; some errors corrected.
 *
 * Revision 1.23  1993/01/14  19:13:56  oak
 * Added some paths.
 *
 * Revision 1.22  1993/01/12  22:47:40  sergey
 * Made "project" and "Directory" lists look simillar.
 *
 * Revision 1.21  1993/01/12  19:27:03  sergey
 * More additions.
 *
 * Revision 1.20  1993/01/12  17:10:23  sergey
 * Spelling corrections; minor changes.
 *
 * Revision 1.19  1993/01/12  14:51:32  sergey
 * Spelling correction.
 *
 * Revision 1.18  1993/01/11  21:35:46  sergey
 * More dialogs added and some corrections made.
 *
 * Revision 1.17  1993/01/11  17:09:50  sergey
 * More modification to reflect new UI.
 *
 * Revision 1.16  1993/01/10  17:07:05  sergey
 * Added "messages" and more stuff; slightly rearranged.
 *
 * Revision 1.15  1993/01/07  22:32:21  oak
 * Fixed a small error with NULL.
 *
 * Revision 1.14  1993/01/07  22:11:00  oak
 * Changed buttons in browsershell to match
 * new interface.
 *
 * Revision 1.13  1993/01/07  21:49:26  sergey
 * Redone according to the new UI.
 *
 * Revision 1.12  1993/01/07  20:13:04  oak
 * Moved dialogs and made small changes to paths
 * as needed.
 *
 * Revision 1.11  1993/01/07  00:48:04  oak
 * Moved all dialogs out of the menu paths.
 *
 * Revision 1.10  1993/01/05  21:14:57  sergey
 * Added -1 flag data to distinguish dialog direct childs of aset.
 *
 * Revision 1.9  1993/01/04  21:10:24  sergey
 * Added level and flag fields to MenuName class and modified the init table accordingly.
 *
 * Revision 1.8  1992/12/29  22:42:34  sergey
 * Added more menu names and made minor corrections.
 *
 * Revision 1.7  1992/12/23  15:41:16  sergey
 * Added more stuff to the table and * everywhere.
 *
 * Revision 1.6  1992/12/22  22:34:25  sergey
 * Added more dialogs.
 *
 * Revision 1.5  1992/12/21  16:03:27  oak
 * Added some of the new ViewerShell widget paths.
 *
 * Revision 1.4  1992/12/18  23:39:58  sergey
 * More data added to the look up table.
 *
 * Revision 1.3  1992/12/18  19:30:26  sergey
 * *** empty log message ***
 *
 * Revision 1.2  1992/12/04  22:02:33  sergey
 * Added more menu/buttons names.
 *
 * Revision 1.1  1992/12/03  19:18:38  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/






