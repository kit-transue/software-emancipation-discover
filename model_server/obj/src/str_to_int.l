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
%{

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#define yylex   my_yylex123
#define yylook  my_yylook12
#define yyback  my_yyback12
#define yyunput my_yyunput12
#define yyinput my_yyinput12
#define yyoutput my_yyoutput12

/*
   these macros define the limits of the integer values
   and the type of integer that these routines will work with
   NOTE: if you change these then go change them in source int_to_string.C
*/
#define MAX_VAL INT_MAX
#define MIN_VAL INT_MIN
#define VAL_TYPE int

/* controls range of numbers parser will accept */
#if MAX_VAL > 32768
static big_range = 1;
#else
static big_range = 0;
#endif

#ifdef __cplusplus
  extern int yylook();
  extern int yylex();
  extern int yyback(int *yystops,int yyfnd);
#endif

 static char *buf;
 static int len = 0;
 static VAL_TYPE val = 0;
 static int sign = 1;
 static VAL_TYPE thousands = 0;
 static VAL_TYPE millions = 0;
 static VAL_TYPE billions = 0;
 static int wrong_input =0;
 static int count = 0;
 

#if 0
char tolower(c)
char c;
 { return (isupper(c) ? (c + 'a' - 'A') : c);}
#endif

#define input() yyinput()
#define unput(c) yyunput(c)
#undef EOF
#define EOF 0
#undef yygetc
#define yygetc() (count < len ? tolower(buf[count++]) : 0)

#define yywrap() 1

%}
D       [0-9]
W       [\n\t ]
%%
zero      {val += 0;}
one       {val += 1;}
two       {val += 2;}
three     {val += 3;}
four      {val += 4;}
five      {val += 5;}
six       {val += 6;}
seven     {val += 7;}
eight     {val += 8;}
nine      {val += 9;}
ten       {val += 10;}
eleven    {val += 11;}
twelve    {val += 12;}
thirteen  {val += 13;}
fourteen  {val += 14;}
fifteen   {val += 15;}
sixteen   {val += 16;}
seventeen {val += 17;}
eighteen  {val += 18;}
nineteen  {val += 19;}


twenty    {val += 20;}
thirty    {val += 30;}
forty     {val += 40;}
fifty     {val += 50;}
sixty     {val += 60;}
seventy   {val += 70;}
eighty    {val += 80;}
ninety    {val += 90;}

hundred   {val *= 100;}
thousand  {thousands =  val * 1000; val = 0;}
million   {millions = (big_range) ? val * 1000000 : 0; val = 0;}
billion   {billions = (big_range) ? val * 1000000000 : 0; val = 0;}


negative  {sign = -1;}

{D}       {val = val * 10 + (yytext[0] - '0');}
{W}+      {}
'\0'      {return val;}
.         {wrong_input = 1; return 0;}
%%

VAL_TYPE str_to_int(str,err_stat) 
char *str;
int  *err_stat;
{

VAL_TYPE sum;

        /* reset all the static storage */
        buf = str; 
        len =strlen(str); 
#if 0  //XXX
        YY_INIT;
#endif
        count = 0; val = 0; 
        sign = 1;
        thousands = 0; 
        millions = 0;
        billions = 0;
        wrong_input = 0;

        yylex(); if(wrong_input) *err_stat = wrong_input;
        sum = billions + millions + thousands + val;
        return (sign * sum);

}


