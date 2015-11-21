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
#ifndef _NODE_H
#define _NODE_H
class node {
// type related items
    int type;				// integer type number
    char* type_name;			// string for name of type
// graph structure related items
    nlist* succlist;		// pointer to primary list of succ nodes
    nlist* ssucclist;		// pointer to sorted list of succ nodes
    nlist* predlist;		// pointer to primary list of pred nodes
    node* nextnode;			// pointer to next node in the
					// list of nodes in this graph
    node* prevnode;			// pointer to previous node in the
					// list of nodes in this graph
    graph* backgraph;		// pointer back to the graph this
					// node is in.
    int reached;
    int straight;                       // nonzero if there is a layout 
                                        // constraint (node on a straight line)

    int width;             	      	// width of displayed node 
    int height;             	      	// height of displayed node 
    int borderwidth;                  	// borderwidth of displayed node
    POINT loc;			      	// location of the node 

    int level;			      	// level assigned 	
    int visible;		      	// toggle if visible		
    int dfnum;			      	// depth first number	
    int displayed;	      	      	// whether it's been displayed recently
    int dummy;			      	// dummy node	

    long standardiconpixmap;		// Original Pixmap
    unsigned int standardiconwidth;
    unsigned int standardiconheight;
    
// abstraction related items
    graph* rgraph;		        // abstraction's subgraph
    int status;                       	// black/grey/white box
    int sgdummy;                        // NODUMMY/INDUMMY/OUTDUMMY      
    int ncon;                           // "needs connection": true when this has to be
                                        // reconnected before backgraph->layout()
// separate view items

// internal use
    node* realsource;
    node* realtarget;

// friends
    // take a number of nodes into another (sub)graph
    friend void take_nodes(graph* newgraph, nlist* nodes, int duplicate = 0);

    // private
    friend void merge_edges (elist*, nlist*, node*);

    // create a new (abstraction)dummy node
    friend node* new_dummynode(graph*, node*, int, int=1);
 
    // set a node's abstraction status, creates window for graph if necessary
    friend void setnodestatus(node*, int);

public:

// constructors and destructors
    node(int type, char* type_name, int width =0, int height =0, int borderwidth =0);
    node (char *tname = NULL);
    virtual ~node();

    int finishconstructor();


    virtual void action();		// some (as yet unspecified) action
					// invokes APPL_ActionNode

    void copy (node *);

					// of abstractions it may be in
    void movenode(int x, int y);	// move node's window to position x,y
    int isabstraction()			{return (rgraph != NULL);}
    int isnormalnode()                  {return (!(dummy || (rgraph != NULL))); }

// access functions
    void settype(int t)			{type = t;}
    int gettype()			{return type;}

    void settypename(char* s);
    char* gettypename()			{return type_name;}

    void addsucc(node* n);		// adds node n to primary succ. list 
    void deletesucc(node* n);		// deletes n from primary succ. list
    nlist* getsucclist()		{return succlist;}  
    nlist* getssucclist()		{return ssucclist;}  
    void setssucclist(nlist* l)		{ssucclist = l;}  
    int countsucc();			// return # nodes in primary succ. list
    elist* find_edges_to_succs(); // return list of edges of primary succs.

    void addpred(node* n);		// adds node n to primary pred. list
    void deletepred(node* n);		// deletes node n to primary pred. list
    nlist* getpredlist()		{return predlist;} 
    int countpred();			// return # nodes in primary pred. list
    elist* find_edges_from_preds(); // return list of edges of primary preds

    void setnextnode(node* n)		{nextnode = n;}
    node* getnextnode()			{return nextnode;}

    void setprevnode(node* n)		{prevnode = n;}
    node* getprevnode()			{return prevnode;}

    void setbackgraph(graph* g)         {backgraph = g;}
    graph* getbackgraph()               {return backgraph;}

    void setreached(int i)              {reached = i;}
    int isreached()                     {return reached;}

    void setstraight(int s)             {straight = s;}
    int getstraight()                   {return straight;}

    void setx(int i)			{loc.x = i;}
    int getx()				{return loc.x;}

    void sety(int i)			{loc.y = i;}
    int gety()				{return loc.y;}

    void setlevel(int l)		{level = l;}
    int getlevel()			{return level;}


    void setdfnum(int i)		{dfnum = i;}
    int getdfnum()			{return dfnum;}

    void setdisplayed(int i)            {displayed = i;}
    int getdisplayed()                  {return displayed;}

    void setdummy(int i);
    int isdummy()			{return dummy;}
    int isdummy(int i)                  {return (dummy == i);}

    void setwidth(int i);
    int getwidth();
    int calcwidth();

    void setheight(int i);
    int getheight();
    int calcheight();

    void setborderwidth(int i)		{borderwidth = i;}
    int getborderwidth()		{return borderwidth;}


virtual void setstandardiconpixmap(long l)          {standardiconpixmap = l;}
    long getstandardiconpixmap()                {return standardiconpixmap;}

virtual void setstandardiconwidth(int i)            {standardiconwidth = i;}
    int getstandardiconwidth()                  {return standardiconwidth;}

virtual void setstandardiconheight(int i)           {standardiconheight = i;}
    int getstandardiconheight()                 {return standardiconheight;}
// abstraction related functions
    node* connected_dummy();
    void connect();                        
    void disconnect();                    

    void setrgraph(graph* g)		{rgraph = g;}
    graph* getrgraph()                  {return rgraph;}       

    void setstatus(int s)               {status = s;}
    int getstatus()                     {return status;}

    void setsgdummy(int i)              {sgdummy = i;}     
    int getsgdummy()                    {return sgdummy;} 

    void setncon(int i)                 {ncon = i;}
    int getncon()              		{return ncon;}

    void setrealsource(node* n)		{ realsource = n;}
    node* getrealsource()		{ return realsource;}

    void setrealtarget(node* n)		{ realtarget = n;}
    node* getrealtarget()		{ return realtarget;}

// constraint related items and functions 
    class constr_node* constr[3];       // 3-dimensional constraints
    int stable;                         // true if node has a fixed position
                                        // in the graph
};


// include the application-specific variations (if any)
#include "interface.h"
/*
   START-LOG-------------------------------------------

   $Log: node.h  $
   Revision 1.2 1998/08/10 18:19:43EDT pero 
   port to VC 5.0: removal of typename, or, etc.
 * Revision 1.2.1.2  1992/10/09  19:03:28  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/

#endif
