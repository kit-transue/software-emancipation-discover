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
#ifndef _INTERSECT_H
#define _INTERSECT_H

class intersect {
    int c_succlist;
    int c_predlist;
    nlist* succlist;
    nlist* predlist;
    class intersect* nextintersect;
    class intersect* previntersect;
    char* concname;
public:
    intersect();
    void printsucclist();
    void printpredlist();
    intersect* add(intersect*, intersect*);
    intersect* addtolist(intersect*, intersect*);
    intersect* genintersection(node*, node*);
    void concentrate();
    intersect* inlist(nlist*);
    intersect* getnextintersect()	{return nextintersect;}
    intersect* getprevintersect()	{return previntersect;}
    void setnextintersect(intersect* i)	{nextintersect = i;}
    void setprevintersect(intersect* i)	{previntersect = i;}
    nlist* getsucclist()	{return succlist;}
    void setsucclist(nlist* l)	{succlist = l;}
    void setc_succlist(int i)	{c_succlist = i;}
    void setc_predlist(int i)	{c_predlist = i;}
    nlist* getpredlist()	{return predlist;}
    void setpredlist(nlist* l)	{predlist = l;}
    int remove_supersets(node*, node*, intersect*);
};

#endif
