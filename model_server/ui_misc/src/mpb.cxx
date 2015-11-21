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
#include <mpb.h>
#include <ProgressBar.h>
#include <math.h> // for floor and ceil
#include <customize.h>
#include <symbolArr.h>
#include <proj.h>
#ifndef ISO_CPP_HEADERS
#include <time.h>  // for time sys call
#else /* ISO_CPP_HEADERS */
#include <ctime>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#define mpb_base_val 79
#define init_psetProjects_val 6

ProgressBar *mpb = NULL;
int get_pb_max_len();
int get_option_genStrings(const char* , genString* , int );

void mpb_init() 
{
  int mpb_limit = mpb_base_val;
  genString pdf_file[1024];
  int num_pdf_files = get_option_genStrings ("-P", pdf_file, 1024);
  num_pdf_files    += get_option_genStrings ("-pdf", 
                                             pdf_file + num_pdf_files, 
                                             1024 - num_pdf_files);
  mpb_limit += num_pdf_files*init_psetProjects_val;
  mpb = new ProgressBar(0, mpb_limit, 1);
}

void mpb_incr (int n)        
{ 
  if (mpb) mpb->Increment(n);
}

void mpb_info(const char *cp)          
{ 
  if (mpb) mpb->SetMessage(cp);
}

void mpb_refresh()               
{
  if (mpb) mpb->Refresh();
}


void mpb_hide()
{
  if (mpb) mpb->Hide();
}

void mpb_popup()
{
  if (mpb) mpb->Popup();
}

void mpb_destroy()               
{ 
  if (mpb) {
    mpb->Finish();
    delete mpb; 
  }
  mpb = NULL; 
}

void get_progressBar_incr_value(int lsize, int break_val, int& incrval, int& val)
{
  if (!lsize || !break_val) return;
 
  if (lsize < break_val)
    incrval = ceil((float)break_val/lsize);
  else
    val = floor((float)lsize/break_val);
}

int step_progress_bar(int break_val, int incrval, int val, int num_incs, int ind)
{
  int ret_val = 0;
  if (!(ind%val) && (num_incs < break_val)) {
    if ((num_incs + incrval) > break_val)
      ret_val =  break_val - num_incs;
    else
      ret_val = incrval;
  }
  return ret_val;
}

int get_pb_max_len()
{
  int bar_limit = mpb_base_val;
  return bar_limit;
}

void mpb_incr_values_init(int break_val, int& val, int& incr_val)
{
  symbolArr sym_arr;
  projList::search_list->get_search_list(sym_arr);
  int list_size = sym_arr.size();   
  get_progressBar_incr_value(list_size, break_val, incr_val, val); 
} 

void mpb_step(int break_val, int incrval, int val, int& num_incs, int ind)
{
  int ret_val;
  ret_val = step_progress_bar(break_val, incrval, val, num_incs, ind);
  if (ret_val > 0) {
    mpb_incr(ret_val);
    num_incs += ret_val;
  }
   
}


void mpb_segment(int len, int init_flag = -1)
{
 static int total_ticks, num_ticks, drift_period = 2, empty=0, itrns = 0;
 static time_t last_update_time, cur_time;

 if (init_flag > 0) {
   empty = 0;
   total_ticks = len;
   num_ticks = 0;
   time(&last_update_time);
 }
 else {
   if (len == 0) {
     int diff = total_ticks - num_ticks;
     if (diff > 0)
       mpb_incr(diff);
     empty = 1;
   }
   else if (len > 0 && !empty) {
     time(&cur_time);
     if ((cur_time - last_update_time >= drift_period) && ((num_ticks+len) <= total_ticks)) {
       mpb_incr(len);
       num_ticks += len;
       if (++itrns >= drift_period) {
	 ++drift_period;
	 itrns = 0;
       }
       last_update_time = cur_time;
     }
   }
 }
}
     
void mpb_mpb_segment(int len, int init_flag)
{
  if (mpb)
     mpb_segment(len, init_flag);
}
