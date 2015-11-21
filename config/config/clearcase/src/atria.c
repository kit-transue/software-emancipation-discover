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
# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#define yywrap() 1
main()
{
   printf("proj: * dir: * cleartool lsvtree\n");
   yylex();
   exit(0);
}
void process_branch();
# define VER 2
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
 {BEGIN VER;}
break;
case 2:
{}
break;
case 3:
   {}
break;
case 4:
{process_branch(yytext);}
break;
case 5:
{}
break;
case 6:
  {BEGIN 0;}
break;
case 7:
{printf("<%c>", yytext[0]);}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

7,
0,

7,
0,

7,
0,

4,
7,
0,

5,
7,
0,

6,
0,

3,
4,
7,
0,

7,
0,

4,
7,
0,

5,
7,
0,

3,
4,
7,
0,

4,
0,

5,
0,

3,
4,
0,

4,
0,

4,
0,

5,
0,

5,
0,

3,
4,
0,

1,
0,

4,
0,

5,
0,

1,
0,

4,
0,

1,
5,
0,

4,
0,

1,
5,
0,

4,
0,

2,
4,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,5,	0,0,	
2,6,	0,0,	3,8,	0,0,	
4,13,	6,16,	1,5,	1,0,	
2,7,	2,0,	3,9,	3,10,	
4,14,	6,17,	6,0,	7,18,	
7,0,	9,20,	16,0,	8,19,	
20,0,	11,19,	26,0,	12,19,	
13,23,	9,20,	9,0,	8,0,	
8,0,	11,0,	11,0,	12,0,	
12,0,	13,0,	14,24,	18,18,	
18,0,	15,23,	19,0,	19,0,	
0,0,	30,30,	14,25,	14,0,	
1,5,	15,17,	15,0,	3,11,	
3,12,	4,15,	8,0,	6,0,	
11,0,	7,0,	12,0,	16,0,	
21,0,	21,0,	26,26,	26,0,	
0,0,	19,0,	17,27,	9,20,	
30,30,	8,0,	11,21,	11,0,	
12,21,	12,0,	13,0,	17,0,	
22,19,	18,0,	0,0,	23,23,	
19,0,	24,24,	0,0,	21,0,	
22,0,	22,0,	15,26,	15,0,	
23,0,	24,29,	24,0,	25,24,	
32,34,	32,0,	0,0,	27,27,	
0,0,	21,21,	21,0,	25,25,	
25,0,	28,19,	12,22,	27,30,	
27,0,	29,32,	0,0,	22,0,	
31,19,	28,0,	28,0,	33,19,	
17,27,	29,29,	29,0,	0,0,	
31,0,	31,0,	0,0,	33,0,	
33,0,	0,0,	22,0,	34,34,	
34,0,	23,0,	35,0,	35,0,	
0,0,	36,0,	36,0,	0,0,	
28,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	31,0,	
22,28,	27,27,	33,0,	0,0,	
0,0,	0,0,	0,0,	28,0,	
0,0,	35,0,	0,0,	29,32,	
36,0,	0,0,	31,0,	0,0,	
0,0,	33,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
35,0,	0,0,	0,0,	36,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
31,33,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
28,31,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	33,35,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	35,36,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-3,	yysvec+1,	0,	
yycrank+-5,	0,		0,	
yycrank+-7,	yysvec+3,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+-8,	0,		yyvstop+3,
yycrank+-10,	yysvec+6,	yyvstop+5,
yycrank+-22,	0,		yyvstop+7,
yycrank+-20,	0,		yyvstop+10,
yycrank+0,	0,		yyvstop+13,
yycrank+-24,	0,		yyvstop+15,
yycrank+-26,	0,		yyvstop+19,
yycrank+-27,	yysvec+6,	yyvstop+21,
yycrank+-37,	yysvec+9,	yyvstop+24,
yycrank+-40,	0,		yyvstop+27,
yycrank+-12,	yysvec+6,	0,	
yycrank+-65,	yysvec+6,	0,	
yycrank+-30,	yysvec+6,	0,	
yycrank+-33,	yysvec+8,	yyvstop+31,
yycrank+-14,	yysvec+9,	yyvstop+33,
yycrank+-51,	yysvec+11,	yyvstop+35,
yycrank+-75,	0,		yyvstop+38,
yycrank+-78,	yysvec+6,	yyvstop+40,
yycrank+-80,	yysvec+9,	yyvstop+42,
yycrank+-90,	yysvec+9,	yyvstop+44,
yycrank+-16,	yysvec+15,	yyvstop+46,
yycrank+-94,	0,		yyvstop+49,
yycrank+-100,	0,		yyvstop+51,
yycrank+-104,	0,		yyvstop+53,
yycrank+36,	0,		yyvstop+55,
yycrank+-107,	0,		yyvstop+57,
yycrank+-83,	yysvec+29,	yyvstop+59,
yycrank+-110,	0,		yyvstop+62,
yycrank+-114,	yysvec+9,	yyvstop+64,
yycrank+-117,	yysvec+28,	yyvstop+67,
yycrank+-120,	yysvec+8,	yyvstop+69,
0,	0,	0};
struct yywork *yytop = yycrank+201;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,'/' ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
