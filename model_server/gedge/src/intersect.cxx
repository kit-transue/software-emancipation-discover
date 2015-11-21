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
#include "all.h"
#include "intersect.h"
#include "globdef.h"
#include "sugilevel.h"

int ilower = 4;
int iupper = 100;


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */


int equalset(nlist*, nlist*);
int subset(nlist*, nlist*);
int superset(nlist*, nlist*);

nlist* intersection(node*, node*);
nlist* sortsucc(node*);
int count(nlist*);


void sortsucclevel(level* l)
{
    for (levelitem* li = l->getfirstitem(); li != NULL; li = li->getnextitem()) 
	if (!li->getnode()->isdummy())	// dummys only have one successor 
	    li->getnode()->setssucclist(sortsucc(li->getnode()));
}

nlist* sortsucc(node* n1)
{
    node* n;
    nlist* result = new nlist();

    /* sort successors of node 1 */
    nlist* s1 = new nlist();	
    node* tmpnode;
    int addmatch = 1;
    if (n1->getsucclist() != NULL) {
      addmatch = 1;
      slist_iterator_forw nexts1(*(slist*)n1->getsucclist());
      while ( n = (node*) nexts1() ) {
	  if (n->isdummy()) 
	     n = n->getrealtarget();
          slist_iterator_forw tmplist(*(slist*)s1);
          while ( tmpnode = (node*) tmplist() ) {
	        if (tmpnode == n) {
		  addmatch = 0;
		  continue;
		}
	  }
	  if (addmatch) s1->addbefore(n, tmpnode);
      }
    } 
    return s1;
}


intersect::intersect()
{
    c_succlist = 0;
    c_predlist = 0;
    succlist = new nlist();
    predlist = new nlist();
    nextintersect = 0;
    previntersect = 0;
    concname = new char[50];
    strcpy(concname, "");
}

intersect* intersect::addtolist(intersect* newintersect, intersect* headintersect)
{
    intersect* p_i;
    intersect* p_li;

    // find position where this sized intersection should fit in
    for (p_i = headintersect, p_li = NULL; 
        ((p_i != NULL) && (p_i->c_succlist <= newintersect->c_succlist));
	p_i = p_i->nextintersect) {
	    p_li = p_i;
    }

    if (headintersect == NULL) {	// create new list
    	headintersect = newintersect;
    	newintersect->nextintersect = NULL;
    	newintersect->previntersect = NULL;
    } else  if (p_li != NULL) {		// append to current list
        newintersect->nextintersect = p_li->nextintersect;
        newintersect->previntersect = p_li;
        if (p_li->nextintersect != NULL) 
	        p_li->nextintersect->previntersect = newintersect;
        p_li->nextintersect = newintersect;
    } else {				// prepend to current list
	headintersect->previntersect = newintersect;
        newintersect->nextintersect = headintersect;
        newintersect->previntersect = NULL;
	headintersect = newintersect;
    }
    return(headintersect);
}

intersect* intersect::add(intersect* oldintersect, intersect* headconc)
{
    intersect* newintersect = new intersect();
    newintersect->c_succlist = oldintersect->c_succlist;
    newintersect->c_predlist = oldintersect->c_predlist;
    newintersect->succlist = oldintersect->succlist;
    newintersect->predlist = oldintersect->predlist;
    int cs = newintersect->c_succlist;
    int cp = newintersect->c_predlist;
    newintersect->nextintersect = NULL;
    newintersect->previntersect = NULL;
    slist_iterator_forw nextpred(*(slist*)newintersect->predlist);
    node* n1 = (node*) nextpred();
    node* n2 = (node*) nextpred();

    if (((cs-1)*cp >= ilower) && ((cs-1)*cp <= iupper)) {
	int tmp = headconc->remove_supersets(n1, n2, newintersect);
 	if (tmp == 1) {
	    return(headconc);
	} else {
            newintersect->nextintersect = headconc;
            newintersect->previntersect = 0;
            if (headconc != NULL)
                headconc->previntersect = newintersect;
            headconc = newintersect;
	}
    }
    return(headconc);
}

// ? is it ever invoked ??

nlist* intersection(node* n1, node* n2)
{
    node* tn1;
    node* tn2;
    nlist* result = new nlist();

    nlist* s1 = n1->getssucclist();
    nlist* s2 = n2->getssucclist();

    /* merge sorted list */

    if ((s1 != NULL) && (s2 != NULL)) {
      	slist_iterator_forw mynexts1(*(slist*)s1);
      	slist_iterator_forw mynexts2(*(slist*)s2);
      	tn1 = (node*) mynexts1();
 	tn2 = (node*) mynexts2();
      	while ((tn1 != NULL) && (tn2 != NULL)) {
            result->append_unique(tn1);	    
      	    tn1 = (node*) mynexts1();
      	    tn2 = (node*) mynexts2();
	}
    }
    return result;
}

int count(nlist* list)
{
    int i= 0;
    if (list != NULL) {
      slist_iterator_forw count(*(slist*)list);
      while ( count() ) 
	i++;
    } 
    return (i);
}

int intersect::remove_supersets(node* n1, node* n2, intersect* newintersect)
{
    int diff;
    int equalflag = 0;
    int c_list;
    int c_ilist;
    node* t;
    nlist* list = newintersect->getsucclist();
    c_list = count(list);

    // try to find an exact match first
    intersect *i;
    for (i = this ; i != 0; i = i->nextintersect) {
	diff = 0;
	if (i->succlist != 0) 
	    c_ilist = count(i->succlist);
	else
	    c_ilist = 0;
	    if (equalset(list, i->succlist)) {
    		equalflag = 1;
      	        slist_iterator_forw pred1(*(slist*)i->predlist);
      	        int match = 0;
  	        node* n;
      	        while ( n = (node*) pred1() ) 
	        if (match == 0) {  // n1 not on predlist 
		    i->predlist->insert(n1);
                    i->c_predlist = count(i->predlist);
	        }
      	        slist_iterator_forw pred2(*(slist*)i->predlist);
      	        match = 0;
	        if (match == 0) { // n2 not on predlist
		    i->predlist->insert(n2);
                    i->c_predlist = count(i->predlist);
 	        }
	        return 1;
	    } 
    }

    // only check sub/supersets if could not find an exact match
    if (equalflag == 0) {
    for(i = this ; i != 0; i = i->nextintersect) {
    	if (subset(list, i->succlist)) {
	  	//subset, split C, add I and C'
		// remove all of nodes in list from i->succlist
		slist_iterator_forw tmpsucc1(*(slist*)list);
        	while(t = (node*) tmpsucc1()) 
		    i->succlist->del(t);
		// add all of nodes in i->predlist to newintersect->predlist
		slist_iterator_forw tmpsucc2(*(slist*)i->predlist);
        	while( t = (node*) tmpsucc2()) 
		    newintersect->predlist->insert(t);
		i->c_succlist = count(i->succlist);
		i->c_predlist = count(i->predlist);
		newintersect->c_succlist = count(newintersect->succlist);
		newintersect->c_predlist = count(newintersect->predlist);
		break;
	    } else if (superset(list, i->succlist)) {
	  	//superset, split I, add I' and C
		// remove all of nodes in list from list
		slist_iterator_forw tmpsucc1(*(slist*)i->succlist);
        	while(t = (node*) tmpsucc1()) 
		    newintersect->succlist->del(t);
		// add all of nodes in newintersect->predlist to i->predlist
		slist_iterator_forw tmpsucc2(*(slist*)newintersect->predlist);
        	while( t = (node*) tmpsucc2()) 
		    i->predlist->insert(t);
		i->c_succlist = count(i->succlist);
		i->c_predlist = count(i->predlist);
		newintersect->c_succlist = count(newintersect->succlist);
		newintersect->c_predlist = count(newintersect->predlist);
		break;
	    } 
        }
    }
    return 0;
}

intersect* intersect::genintersection(node* n1, node* n2)
{
    nlist* pl = new nlist();
    pl->insert(n1);
    pl->insert(n2);
    nlist* sl = intersection(n1, n2);
    intersect* newintersect = new intersect();
    newintersect->c_succlist = count(sl);
    newintersect->c_predlist = count(pl);
    newintersect->succlist = sl;
    newintersect->predlist = pl;
    return newintersect;
}

int equalset(nlist* n1, nlist* n2) 
{
    slist_iterator_forw first(*(slist*)n1);
    slist_iterator_forw second(*(slist*)n2);
    node* one = (node*) first();
    node* two = (node*) second();

    while ((one != NULL) && (two != NULL)) {
        one = (node*) first();
        two = (node*) second();
    }
    if ((one == NULL) && (two == NULL)) 
	return 1;
    else
	return 0;
}

int subset(nlist* n1, nlist* n2) 
{
    slist_iterator_forw first(*(slist*)n1);
    slist_iterator_forw second(*(slist*)n2);
    node* one = (node*) first();
    node* two = (node*) second();

    while ((one != NULL) && (two != NULL)) {
            one = (node*) first();
            two = (node*) second();
    }
    if (one == NULL) 
	return 1;
    else
	return 0;
}

int superset(nlist* n1, nlist* n2) 
{
    slist_iterator_forw first(*(slist*)n1);
    slist_iterator_forw second(*(slist*)n2);
    node* one = (node*) first();
    node* two = (node*) second();
    while ((one != NULL) && (two != NULL)) {
            one = (node*) first();
            two = (node*) second();
    }
    if (two == NULL) 
	return 1;
    else
	return 0;
}


void intersect::printsucclist()
{
    if (succlist != NULL) {
      slist_iterator_forw nextsucc(*(slist*)this->succlist);
    } 
    printf("\n");
}

void intersect::printpredlist()
{
    if (succlist != NULL) {
      slist_iterator_forw nextpred(*(slist*)this->predlist);
    } 
    printf("\n");
}

void intersect::concentrate()
{
    node* tn1;
    node* tn2;
#if 0  // unused
    static int dummycount = 0;                   
#endif
    char* dummykey = new char [20];
    if (((this->c_succlist -1)* this->c_predlist >= ilower) && ((this->c_succlist-1) * this->c_predlist <= iupper)) {
	// create edge concentrator node
	strcpy(this->concname, dummykey);

	slist_iterator_forw myynexts1(*(slist*)this->predlist);
        tn1 = (node*) myynexts1();
        while (tn1 != NULL) {
            tn1 = (node*) myynexts1();
	}

	slist_iterator_forw myynexts2(*(slist*)this->succlist);
        tn2 = (node*) myynexts2();
        while (tn2 != NULL) {
            tn2 = (node*) myynexts2();
	}
	slist_iterator_forw mynexts1(*(slist*)this->predlist);
        tn1 = (node*) mynexts1();
        while (tn1 != NULL) {
            slist_iterator_forw mynexts2(*(slist*)this->succlist);
            tn2 = (node*) mynexts2();
            while (tn2 != NULL) {
                tn2 = (node*) mynexts2();
	    }
            tn1 = (node*) mynexts1();
       	}
    } 
}
