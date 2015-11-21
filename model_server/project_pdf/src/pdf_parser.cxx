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
#define FLEX_SCANNER
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#include <cstring>
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>

#include <pdf_parser.h>

/* returned upon end-of-file */
#define YY_END_TOK 0

#define YY_NULL 0

#define yyterminate() return ( YY_NULL )

#define YY_FATAL_ERROR(msg) \
        do \
                { \
                (void) fputs( msg, stderr ); \
                (void) putc( '\n', stderr ); \
                exit( 1 ); \
                } \
        while ( 0 )

/* default yywrap function - always treat EOF as an EOF */
#define yywrap() 1

/* enter a start condition.  This macro really ought to take a parameter,
 * but we do it the disgusting crufty way forced on us by the ()-less
 * definition of BEGIN
 */
#define BEGIN yy_start = 1 + 2 *

/* action number for EOF rule of a given start state */
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)

#ifdef YY_USE_PROTOS
#define YY_PROTO(proto) proto
#else
#define YY_PROTO(proto) ()
/* we can't get here if it's an ANSI C compiler, or a C++ compiler,
 * so it's got to be a K&R compiler, and therefore there's no standard
 * place from which to include these definitions
 */
#endif

/* code executed at the end of each rule */
#define YY_BREAK break;

#define YY_END_OF_BUFFER_CHAR 0
#define YY_READ_BUF_SIZE 1024
#define YY_BUF_SIZE (YY_READ_BUF_SIZE * 2) /* size of default input buffer */

typedef struct yy_buffer_state *YY_BUFFER_STATE;

/* done after the current pattern has been matched and before the
 * corresponding action - sets up yytext
 */
#define YY_DO_BEFORE_ACTION \
        yytext = yy_bp; \
        yyleng = yy_cp - yy_bp; \
        yy_hold_char = *yy_cp; \
        *yy_cp = '\0'; \
        yy_c_buf_p = yy_cp;

#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

#define unput(c) yyunput( c, yytext )

/* yy_hold_char holds the character lost when yytext is formed */
static YY_CHAR yy_hold_char;

// static int yy_n_chars; /* number of characters read into yy_ch_buf */

YY_CHAR *yytext;
int yyleng;

static YY_CHAR *yy_c_buf_p = (YY_CHAR *) 0;

// static int yy_init = 1; /* whether we need to initialize */
//static int yy_start = 0; /* start state number */

struct yy_buffer_state
{
    YY_CHAR *yy_ch_buf;  /* input buffer */
    YY_CHAR *yy_buf_pos; /* current position in input buffer */

    /* size of input buffer in bytes, not including room for EOB characters*/
    int yy_buf_size;

    /* number of characters read into yy_ch_buf, not including EOB characters */
    int yy_n_chars;

    int yy_eof_status;   /* whether we've seen an EOF on this buffer */
#define EOF_NOT_SEEN 0
    /* "pending" happens when the EOF has been seen but there's still
     * some text process
     */
#define EOF_PENDING 1
#define EOF_DONE 2
};

static struct yy_buffer_state pdf_current_buffer;
static YY_BUFFER_STATE yy_current_buffer = &pdf_current_buffer;

static char buf[1024];
static int pdf_sz;

static void pdf_init_buffer(char *str)
{
    pdf_sz = strlen(str);
    OSapi_bcopy(str, buf, pdf_sz);
    buf[pdf_sz] = '\t';
    buf[pdf_sz+1] = '\n';

    pdf_current_buffer.yy_ch_buf = buf;
    pdf_current_buffer.yy_buf_pos = buf;
    pdf_current_buffer.yy_buf_size = pdf_sz+2;
    pdf_current_buffer.yy_n_chars = pdf_sz+2;
    pdf_current_buffer.yy_eof_status = EOF_DONE;

/*  void yy_load_buffer_state()  */
    {
//      yy_n_chars = yy_current_buffer->yy_n_chars;
        yytext = yy_c_buf_p = yy_current_buffer->yy_buf_pos;
        yy_hold_char = *yy_c_buf_p;
    }
}

#define YY_CURRENT_BUFFER yy_current_buffer

typedef long yy_state_type;

static long int YY_END_OF_BUFFER; /* #define */
static long int YY_NO_STATES; /* = sizeof(yy_accept)-1 */

static long int * yy_acclist;
static long int * yy_accept;
static YY_CHAR * yy_ec;
static YY_CHAR * yy_meta;
static long int * yy_base;
static long int * yy_def;
static long int * yy_nxt;
static long int * yy_chk;

// static yy_state_type yy_last_accepting_state;
//static YY_CHAR *yy_last_accepting_cpos;

static yy_state_type yy_state_buf[YY_BUF_SIZE + 2], *yy_state_ptr;
static YY_CHAR *yy_full_match;
static long yy_lp;
#define REJECT \
{ \
*yy_cp = yy_hold_char; /* undo effects of setting up yytext */ \
yy_cp = yy_full_match; /* restore poss. backed-over text */ \
++yy_lp; \
goto find_rule; \
}

#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0

//static int pdf_off;
int pdf_parse(char *str, int off, int* skip_table)
{
    register yy_state_type yy_current_state;
    register YY_CHAR *yy_cp, *yy_bp;
    register long yy_act;

//  pdf_off = off;
    pdf_init_buffer(str);

    long idx = 0;

    {
        yy_cp = yy_c_buf_p;

        /* support of yytext */
        *yy_cp = yy_hold_char;

        /* yy_bp points to the position in yy_ch_buf of the start of the
         * current run.
         */
        yy_bp = yy_cp;

        yy_current_state = 1; /* yy_start; */
        yy_state_ptr = yy_state_buf;
        *yy_state_ptr++ = yy_current_state;

//yy_match:
        do
        {
            register YY_CHAR yy_c = yy_ec[*yy_cp];
            while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
            {
                yy_current_state = yy_def[yy_current_state];
                if ( yy_current_state > YY_NO_STATES )
                    yy_c = yy_meta[yy_c];
            }
            yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
            *yy_state_ptr++ = yy_current_state;
            ++yy_cp;
        }
        while ( yy_current_state != YY_NO_STATES );

//yy_find_action:
        yy_current_state = *--yy_state_ptr;
        yy_lp = yy_accept[yy_current_state];
find_rule: /* we branch to this label when backtracking */
        for ( ; ; ) /* until we find what rule we matched */
        {
            if ( yy_lp && yy_lp < yy_accept[yy_current_state + 1] )
            {
                yy_act = yy_acclist[yy_lp];
                {
                    yy_full_match = yy_cp;
                    break;
                }
            }
            --yy_cp;
            yy_current_state = *--yy_state_ptr;
            yy_lp = yy_accept[yy_current_state];
        }

        YY_DO_BEFORE_ACTION;

//do_action:        /* this label is used only to access EOF actions */
        if (yy_act < YY_END_OF_BUFFER - 1) {
	    if (skip_table[yy_act] <= off) {
		REJECT;
	    }
	}
        /* accept */
        idx = (yyleng < pdf_sz || yy_act >= YY_END_OF_BUFFER -1) ? 0 : skip_table[yy_act];
    }	

    return idx;
}

void pdf_set_parser(struct flex_state_table *pdf_tbl)
{

 YY_END_OF_BUFFER = pdf_tbl->yy_END_OF_BUFFER;
 YY_NO_STATES = pdf_tbl->yy_NO_STATES;
 yy_acclist = pdf_tbl->yy_acclist;
 yy_accept = pdf_tbl->yy_accept;
 yy_ec = pdf_tbl->yy_ec;
 yy_meta = pdf_tbl->yy_meta;
 yy_base = pdf_tbl->yy_base;
 yy_def = pdf_tbl->yy_def;
 yy_nxt = pdf_tbl->yy_nxt;
 yy_chk = pdf_tbl->yy_chk;

}

/*
$Log: pdf_parser.cxx  $
Revision 1.7 2000/07/10 23:08:18EDT ktrans 
mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.2  1994/05/24  20:51:28  builder
 * Port
 *
 * Revision 1.1  1994/05/23  20:18:42  so
 * Initial revision
 *
*/

