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
#ifndef _EDGE_H
#define _EDGE_H

/*	class definition for edge and associated member functions
 */



class edge {
// type related items
    int type;				// integer type number
    char* type_name;			// string for name of type
// graph structure related items
    class edge* nextedge;		// pointer to next edge in the 
    class edge* prevedge;		// point to previous edge in the
    class graph* backgraph;		// pointer back to the graph this
    node* source;			// pointer to source node
    char *sourcename;                   // title of source node
    node* target;			// pointer to target node
    char *targetname;                   // title of target node
    POINT sourceloc;            	// x,y coor. of source connection pt.
    POINT targetloc;            	// x,y coor. of target connection pt.
    int reversed;			// is it reversed edge (from cycle)?

// related to edge's appearance
    int direction;
    int visible;			// visibility status
    int dummy;				// dummy edge (btwn sugi. layout dummy nodes)

public:

// constructors and destructors
    // construct edge from node s to node t with label l
    // type of edge given by type and typename
    // dummy edge <-- d, reversed <-- r 
public:

// constructors and destructors
    // construct edge from node s to node t with label l
    // type of edge given by type and typename
    // dummy edge <-- d, reversed <-- r
    edge(node* s,node* t,int type,char* type_name);
    edge (char *tname = NULL);
    ~edge();
    void copy(edge*);

// edge user interface functions

    virtual void action();		// some (as yet unspecified) action
    virtual void copy_vals (edge *);
// misc. member functions
    void layout();			// assigns values to source/target-loc
// low level access functions
    void settype(int i)                 {type = i;}
    int gettype()                       {return type;}
    virtual void settypename(char* s);
    char* gettypename()                 {return type_name;}
    void setnextedge(edge* e)           {nextedge = e;}
    edge* getnextedge()               	{return nextedge;}
    void setprevedge(edge* e)           {prevedge = e;}
    edge* getprevedge()               	{return prevedge;}
    void setbackgraph(graph* g)         {backgraph = g;}
    graph* getbackgraph()               {return backgraph;}
    void setsource(node* n)		{source = n;}
    node* getsource()			{return source;}
    virtual void setsourcename(char* s);
    char* getsourcename()                 {return sourcename;}
    void settarget(node* n)		{target = n;}
    node* gettarget()			{return target;}
    virtual void settargetname(char* s);
    char* gettargetname()                 {return targetname;}
    void setreversed(int i)		{reversed = i;}
    int isreversed()			{return (reversed == 1);}
    int isvisible()                     {return (visible == 1);}
    void setdirection (int i)           {direction = i;}
    int getdirection ()                 {return direction;}
    int getsourcex ()                   { return sourceloc.x; }
    int getsourcey ()                   { return sourceloc.y; }
    int gettargetx ()                   { return targetloc.x; }
    int gettargety ()                   { return targetloc.y; }
    void setdummy(int i)		{dummy = i;}
    int isdummy()			{return (dummy == 1);}
};

#endif

