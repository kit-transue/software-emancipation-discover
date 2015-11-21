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
//////////////////////////   FILE steHeader.h   ///////////////////////////
//
// -- Contains table of interface commands to EPOCH.            
//
#ifndef steHeader_h
#define steHeader_h
#include <transaction.h>
#include <objCollection.h>

#include <ste_transaction.h>

ostream& steHeaderStream(int fd = -1);
ostream& steHeader(int command);
ostream& operator << (ostream& os, struct ste_end_flush_handler*);

int edt_new_question_id_out();

#define steFlush steHeaderStream() << flush
#define steWithout '\2'

#define steEnd   '\2' << (ste_end_flush_handler*)0

enum {
  Message           =   0,  //(string);
  SteNewBuffer      =   1,  //(name) => buffer;
  SteUxid           =   2,  //(UIMX_Window_ID);
  SteClearBuffer    =   3,  //(buffer);
  SteSetPoint       =   4,  //(buf, point) => but, rel_point;
  SteCreateButton   =   5,  //(buf, begin, end, style) => epoch_button; 
  SteCreateScreen   =   6,  //(buffer) => screen;
  SteCreateStyle    =   7,  //(char bg,char fg) => style;
  SteStartTextInsert =  8,  //(buffer-id);
  SteEndTextInsert  =   9,  //();
  SplitButton       =  10,  //(but, point) => int but1, int but2;
  CreateTempButton  =  11,  //( s_point, e_point) => temp_but;
  GetPoint          =  12,  //(buf) => but, pnt, rel_point
  GetWindowPoint =  13,  //(scr) => but, point
  SetPointBegin     =  14,  //(buf) => epoch_point_button, rel_point
  GetBufferPointButton = 15,  // (buf-id, point) => real_but, prev_but
  GetButtonMargins  =  16,  //(but) => strat, end;
  DeleteButton      =  17,  //(but);
  DeleteButtonText  =  18,  //(but);
  ChangeButtonStyle =  19,  //(but, style);
  GetNextButton     =  20,  //(but) => but_next;
  SteInsertButton   =  21,  //(buf, char str)
  SteButtonEnd      =  22,  //Finish of a string. => button;
  GetPointButton    =  23,  //get button of point => but;
  MakeFormat        =  24,  //formating of the inserted string;
                            // button1, button2, left_tab, right_tab,
                            // line_before, first_indent
  SteCollapseNode   =  25,  //(but1, but2);
  SteExplodeNode    =  26,  //(but1, but2);
  GetButtonLength   =  27,  // ( but ) => int len;
  GetButtonAscii    =  28,  // (but ) => char *str;
  ReplaceButtonString = 29,//(but, string);
  SteMoveRegion     =  30, //(st_src_but, en_src_but, en_trg_but); 
  SteRemoveRegion   =  31, //(st_but, en_but) 
  ReplaceButtonEnd  =  32,  
  MovePointButtonEnd = 33, //( but )
  GetTempButton     =  34, // scr, pnt=>temp_but
  SteDeleteBuffer   =  35, // buf;
  SteDeleteScreen   =  36, // scr;
  MoveButtonStartEnd = 37, // (but, st_point, en_point);
  SteSearchString   =  38, // ( st-pnt, str ) => pnt1, but1, pnt2, but2
  SteSearchWord     =  39, // ( st-pnt, str ) => pnt1, but1, pnt2, but2
  HiliteWordRegion  =  40, // (buf_id,pos1,pos2)=>temp_but,pnt1,but1,pnt2,but2
                           // or "0" if there is no word between pos1 and pos2;
  SteHiliteWord     =  41, // (buf_id, pos) => temp_but, pnt1, but1, pnt2, but2
                           // or "0" if there is no word on position pos;
  OodtSearchWord    =  42, // (buf_id,st-pnt(0,-2,-1),direction(1,-1),str)
                           // => pnt1, but1, pnt2, but2
  SteScrollScreen   =  43, // (int scr_id, int command, int parametr) 
  SteKillEmacs      =  44, // kills emacs no query.
  SteGetRelPoint    =  45, // 45  (int but, int point) => int rel-point,
                           // returns -1 if point is not in the button but.
  SteMovePointBegin =  46, // ( but, offset - 0, int, -1 )
  SteSetMarksFlag   =  47, // (buff)
  SteUnsetMarksFlag =  48, // (buff)
  SteCheckButtons   =  49, // (buff)
  SteCheckPointButton = 50, // (buff point)
  SteFreeze           = 51, // (int flag: 1 - up, 0 - down)
  SteAddStyle         = 52, // (fg bg AT SZ) => epoch_idx;
  SteModifyStyle      = 53, // (epoch_idx fg bg AT SZ)
  SteGetEpochVersion  = 54, // () => int version;
  SteRedrawDisplay    = 55, // ( scr_id );
  SteAddDefaultStyle  = 56, // ( scr_id, fg bg AT SZ )
  SteGetBufferText    = 57, // ( buf_id ) => int len, char* text
  SteSelectScreen     = 58, // (scr_id);
  SteRestorePoint     = 59, // (buf_id, but_id, rel_point);
  SteEndInsertMemSeg  = 60, // (mem_ptr, size);
  SteSetMemoryShared  = 61, // (aset_pid) => mem_flag;
  SteEndFormatMemSeg  = 62, // (mem_ptr, size); 
  SteGraphStyleCreate = 63, // ((char *)X-resource) => int style_index;
  SteGraphStyleModify = 64, // (int style_index, (char *)X-resource);
  SteGetScreenDim     = 65, // (int scr_id, => width, higth);
  StePrintMessage     = 66, // (char *str );
  SteSetMinibuffer    = 67, // (char *str = "new_xid");
  SteSetRegionMode    = 68, // (int but-id1, int but-id2, int mode);

//  Tab Rule handler
  SteAddRuleItem      = 69, // ( int rul_index, int after_ind, int col );
  SteChangeRuleItem   = 70, // ( int rul_index, int item_index, int col );
  SteRemoveRuleItem   = 71, // ( int rul_index, int item_index );
  SteCreateRule       = 72, // ( int col_count, (int , ... , int) )=>rule_index
//SteDeleteRule       = 73, // ( int rule_index );
  SteListSendAllSelect= 73, // () => buf off len string buf off len string ...

//
  SteFinalize         = 74, // (int buf_id ) =>int result (0/1);
  SteSetArrow         = 75, // (buf-id, &option but-id);(buf-id == 0 - unset) 
  SteSetFormatStyle   = 76, // (tp = 1(indented), 2(exdented), 3(K&R));
  SteFixPoint         = 77, // (buf-id, but-id (0,-1,>0));
  SteSearchRegexp     = 78, // (buf-id) => (pnt1 but1 pnt2 but2)
  SteEndBufferInsert  = 79, // (buf_id, char *str);
  SteRawFocus         = 80, // (buf_id, line, arrow_flag);
  SteSetPrompt        = 81, // (buf_id, char *prmt);
  SteInsertShmemButton = 82, // (char *shmemptr)
  StePutAction        = 83, // (but-id, oper, (null or many int));
  GetRegionOffsets    = 84, // ( temp_but_id ) =>st_but,st_off,end_but,end_off;
  SteStyleMemSeg      = 85, // (char *shmemptr, int sz);
  SteStyleString      = 86, // (char *str = {int but_id, int st_id})
  SteActionMemSeg     = 87, // (char *shmemptr, int sz);
  SteActionString     = 88, // (char *str = {int but_id, int ch_a, int mv_a})
  BufferSetFlag       = 89, // (buffer-id, flag index, flag value)
  SteRawCompile       = 90, // (buffer-id);
  HiliteWithOffset    = 91, // (buf_id, zn_id1, word_ind1, off1,
                            //  zn_id2, word_ind2, off2,
                            //  node1 off, node2 off) ==> temp_but,
                            //  pnt1,but1,pnt2,but2,off1,off2; if 0 - wrong
  SteShiftedZones     = 92, // (current_index);
  SteZonesMemSeg      = 93, // regenerate zones from MemSeg(st,en,memseg);
  SteZonesString      = 94, // regenerate zones from String(st,en,string);
  SteSaveBuffer       = 95, // (ste-raw-index); 
  GetCurrentPoint     = 96, // () => buf_id, pnt, zn-id/0, offset/0
  SetCurrentPoint     = 97, // (buf_id, zn-id/0, pnt/offset/0, scr_id/0);
  SetBusyCursor       = 98, // (set_flag); sets/unsets  Epoch busy cursor
  SteRefreshMinibuf   = 99, // (oper: 0 - Expose, 1 - resize);
  SteTryBuffer        = 100, // (buff-id/0, pnt/0) => zn-id/0, off, scr-id;
  SteArbitraryAnswer  = 101, // (up to five integers);
  SteRawFinalize      = 102, // (ste_raw_index);
  SteSetPreferences   = 103, // (pref-id, pref-val);
  SmodReplaceRemove   = 104, // (st, off1, en, off2, str("")); 
  SteQueryPixels      = 105, // (buf_id) => x,y 
  SteOpenWindow       = 106, // (buf_id, &optional other-flag);
  SteToggleEditMode   = 107, // (buf_id, new_mode);
  SteMapScreen        = 108, // (screen-num, on/off-flag);
  SteCompilationRouter =109, // (command, string);
  SteReserv           = 110, // ();
  SteExecuteMacro     = 111, // (shared memory pointer);
  SteEvalForm         = 112, // (form string);
  SteEvalFormSync     = 113, // (form string) => len of result out-str, the out-str(if len > 0)
  SteQueryBuffer      = 114, // (buf-id) => point, point-max, point line, point column, line-max
  SteListSendSelect   = 115, // () => ((buf-id pairs... -1) (buf-id pairs... -1) ... -2)
  SteListCommand      = 116, // (subcommand); 
                             //   see enum PSET_LIST_subCOMMAND
  SteFlushPipe        = 117, // Error occured in communication.  Flush pipe.
  SteHiliteRegion     = 118,
  SteListSendRawSelect= 119, // () => buf off len string buf off len string ...
  Reserv              = 120 // Last one 
};


/* this enumeration is used both ways for
   communication between aset and epoch */
enum PSET_LIST_subCOMMAND {
    PSET_OPEN_DEF = 0,   // from epoch: (buffer, line no)
    PSET_SHOW_DEF = 1, // from epoch: (buffer, line no)
    PSET_CLEAR_SELECTION = 2, // from epoch: () clear all selections
                              // to epoch: () clear zones in all ste-list-mode buffers
    PSET_FIND_FUNCTION = 3, // from epoch: (regexp)
    PSET_FIND_CLASS = 4,    //        "
    PSET_FIND_VARIABLE = 5, //        "
    PSET_FIND_OBJECT = 6,   //        "
    PSET_FIND_PROJECT = 7,  //        "
    PSET_FIND_FILE = 8,     //        "
    PSET_ERASE_BUFFER = 9,  // to epoch: (buffer) erase contents of a buffer
    PSET_FOCUS_DEF = 10    // from epoch: (buffer, line no)
};

#endif 

 /*
   START-LOG-------------------------------------------

   $Log: steHeader.h  $
   Revision 1.7 1997/08/14 20:18:30EDT abover 
   Added click-drag DISCOVER selection capability
 * Revision 1.2.1.26  1994/07/27  22:44:33  bakshi
 * Bug track: n/a
 * epoch list project
 *
 * Revision 1.2.1.25  1994/04/04  18:42:21  boris
 * Added epoch macro scripting functionality
 *
 * Revision 1.2.1.23  1993/12/13  21:11:58  azaparov
 * Bug track: 5537
 * 5537
 *
 * Revision 1.2.1.22  1993/11/19  04:16:12  boris
 * Emacs windows
 *
 * Revision 1.2.1.21  1993/08/17  17:58:14  boris
 * Added Epoch X arrow moving functionality
 *
 * Revision 1.2.1.20  1993/08/03  23:44:46  boris
 * added REPLACE_REGION functionality
 *
 * Revision 1.2.1.19  1993/07/23  00:21:38  swu
 * vi mode support
 *
 * Revision 1.2.1.18  1993/06/15  00:44:29  boris
 * Fixed bug #3681 with Raw buffer loosing duplicated buffer changes
 *
 * Revision 1.2.1.17  1993/06/08  23:44:10  boris
 * Added arbitary answer to ParaSET epoch communication
 *
 * Revision 1.2.1.16  1993/05/19  22:01:06  boris
 * Fixed purify memory complains
 *
 * Revision 1.2.1.15  1993/05/17  13:54:27  boris
 * added steRefreshMinibuf
 *
 * Revision 1.2.1.14  1993/05/11  00:05:00  boris
 * Added SetBusyCursor call
 *
 * Revision 1.2.1.13  1993/04/15  15:24:38  boris
 * Fixed bug #2931, with focusing while debugging
 *
 * Revision 1.2.1.12  1993/04/13  23:08:05  boris
 * Fixed bug #3326. Save on Raw View
 *
 * Revision 1.2.1.11  1993/04/05  15:53:48  boris
 * added two parameters to HiliteWithOffset
 *
 * Revision 1.2.1.10  1993/03/26  04:51:05  boris
 * Added Smod Short
 *
 * Revision 1.2.1.9  1993/02/25  16:57:59  mg
 * streamlined steEnd to avoid instabilities.
 *
 * Revision 1.2.1.8  1993/01/21  23:06:15  boris
 * Added region_report functionality
 *
 * Revision 1.2.1.7  1993/01/18  23:42:22  boris
 * Added word offset hiliting.
 *
 * Revision 1.2.1.6  1992/12/11  22:19:59  boris
 * Added Raw app/ldr/view
 *
 * Revision 1.2.1.5  1992/11/20  19:39:17  boris
 * Added Decorate Source facilities
 *
 * Revision 1.2.1.4  1992/11/01  20:15:13  boris
 * Added region_offset reguest to epoch
 *
 * Revision 1.2.1.3  1992/10/20  00:12:43  boris
 * Added  StePutAction command
 *
 * Revision 1.2.1.2  92/10/09  20:01:01  boris
 * Fix comment
 * 


   END-LOG---------------------------------------------

*/

