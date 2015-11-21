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
#include <psetmem.h>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
#ifndef _WIN32 
#include <sys/resource.h>
#else
#include <stdio.h>
#endif
 

int
gettime ()
{
return 0;
}

static aset_stat = 0;
static int rtime0, rtime1, time0, time1, page0, page1, rcv0, rcv1;

static struct result {
  struct result* next;
  char name[25];
  double rtime, time;
  int page, rcv;
} *result;

void init_stat ()
{
}

void pset_getdate(fmt, buf, bufsz)
const char* fmt; char* buf; int bufsz;
{
  time_t clock;
  struct tm * dd;
  time(&clock);
  dd = localtime(&clock);
  strftime(buf, bufsz, fmt, dd);
}

void get_stat (name)
   char *name;
{
}

static double t_rtime, t_time;
static int t_page, t_rcv;

static void out_st (ptr)
  struct result* ptr;
{
  char str[100], buf[80];

  if (ptr->next)
    out_st (ptr->next);
  t_rtime += ptr->rtime;
  t_time += ptr->time;
  t_page += ptr->page;
  t_rcv += ptr->rcv;
  memset (str, ' ', 80);
  sprintf (buf, "%s", ptr->name);
  strncpy (str, buf, strlen (buf));
  sprintf (buf, "%f", ptr->rtime);
  strncpy (str + 25, buf, strlen (buf));
  sprintf (buf, "%f", ptr->time);
  strncpy (str + 39, buf, strlen (buf));
  sprintf (buf, "%d", ptr->page); 
  strncpy (str + 52, buf, strlen (buf));
  sprintf (buf, "%d", ptr->rcv);
  strcpy (str + 62, buf);
  printf ("%s\n", str);
  free (ptr);
}

void out_stat ()
{
   char str[100], buf[80];

   if (!result)
     return;
   t_rtime = 0;
   t_time = 0;
   t_page = 0;
   t_rcv = 0;
   printf ("\n");
   printf ("                           time         cpu        paging    msgrcv\n");
   out_st (result);
   memset (str, ' ', 80);
   strncpy (str, "Total", 5);
   sprintf (buf, "%f", t_rtime);
   strncpy (str + 25, buf, strlen (buf));
   sprintf (buf, "%f", t_time);
   strncpy (str + 39, buf, strlen (buf));
   sprintf (buf, "%d", t_page); 
   strncpy (str + 52, buf, strlen (buf));
   sprintf (buf, "%d", t_rcv);
   strcpy (str + 62, buf);
   printf ("%s\n", str);
   result = 0;
}


/*
   START-LOG-------------------------------------------

   $Log: gettime.c  $
   Revision 1.6 2000/07/07 08:12:05EDT sschmidt 
   Port to SUNpro 5 compiler
 * Revision 1.2.1.10  1994/03/28  14:01:50  kws
 * psetmalloc - compliance
 *
 * Revision 1.2.1.9  1994/03/08  18:35:01  builder
 * Port
 *
 * Revision 1.2.1.8  1994/03/06  16:33:13  mg
 * Bug track: 0
 * validation groups
 *
 * Revision 1.2.1.7  1994/02/16  18:10:31  kws
 * Port
 *
 * Revision 1.2.1.6  1994/02/07  23:44:33  builder
 * Port
 *
 * Revision 1.2.1.5  1993/10/06  14:40:19  kws
 * Port
 *
 * Revision 1.2.1.4  1993/02/05  16:24:54  aharlap
 * fixed bug # 2373
 *
 * Revision 1.2.1.3  1992/11/16  16:26:27  aharlap
 * added get_stat function
 *
 * Revision 1.2.1.2  92/10/09  18:56:20  boris
 * Fix comments
 * 

   END-LOG---------------------------------------------

*/
