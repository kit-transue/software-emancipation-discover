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
#ifndef BreakCond_h
#define BreakCond_h

//#include "treei.h"
/* ----------------------- Classes for break conditions ----------------- */
class astBreakCond;
class astBreakCondBlocks;
class astBreakCondBlockList;

class astBreakCond {
  astExp* exp;
  astBreakCond* next_cond;

  public:
  astExp*
    get_expr () {return exp;}
  void
    set_expr (astExp* e) {exp = e;}
  astBreakCond*
    get_next () {return next_cond;}
  void
    set_next (astBreakCond* new_cond) {next_cond = new_cond;}
};

class astBreakCondBlock {
  astBlock *block;
  class astBreakCond* cond;
  class astBreakCondBlock* prev_block;
  
  public:
  astBlock* 
    get_block () { return block;}
  void
    set_block (astBlock* b) {block = b;}
  astBreakCond*
    get_cond () {return cond; }
  void
    set_cond (astBreakCond* c) {cond = c;}
  int
    is_expr_in_chain (astExp* e);
  void 
    add_expr_to_chain (astExp* exp);
  void 
    remove_expr_from_chain (astExp* e);
  void 
    set_prev (astBreakCondBlock* b) {prev_block = b;}
  astBreakCondBlock* 
    get_prev () {return prev_block;}
};


class astBreakCondBlockList {
  astBreakCondBlock* chain;
  astBreakCondBlock* current_break_cond_block;
  astBreakCond* current_break_cond;

  public:
  
  astBreakCondBlock* 
    get_chain () { return chain; }
  void
    set_chain (astBreakCondBlock* c) { chain = c; }
  int
    is_block_in_chain ( astBlock* b);
  astBreakCondBlock* 
    get_break_cond_block (astBlock* b);
  void 
    add_block_to_chain (astBlock* bl, astExp* ex = 0);
  void 
    remove_block_from_chain (astBlock* b);
  void 
    add_exp_in_block (astBlock* bl, astExp* ex);
  void 
    remove_exp_in_block (astBlock* bl, astExp* ex);

  astBlock* 
    get_current_block () 
      {
	astBlock* b;
	if (current_break_cond_block) b = current_break_cond_block->get_block ();
        else b = 0; 
	return b;
      }
  
  void
    reset_current_block (astBlock* b);

  void
    reset_break_cond ()
      {	current_break_cond = (current_break_cond_block ?
			      current_break_cond_block->get_cond () : 0);      }
  
  astExp*
    get_first_expr ()
      {
	reset_break_cond ();
	if (current_break_cond)
	  return current_break_cond->get_expr ();
	else return 0;
      }
  astExp*
    get_next_expr ()
      {
	if (current_break_cond)
	  current_break_cond = current_break_cond->get_next ();
	if (current_break_cond)	
	  return current_break_cond->get_expr ();
	else return 0;
      }
};
#endif
