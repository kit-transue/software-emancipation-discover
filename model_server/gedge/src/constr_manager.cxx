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
#include "constr.h"
extern int debug;

constr_manager::constr_manager() { consistent = 1; }

constr_manager::~constr_manager() {}

int constr_manager::equal(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO : 
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.equal(cd->node1(), cd->node2(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.equal(cd->node1(), cd->node2(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.equal(cd->node1(), cd->node2(), UNDO);
  }
  return 0;
}

void constr_manager::set_dimension(dimension d) {
  dim = d;
  constr_1D.set_dimension(d);
}

int constr_manager::smaller(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.order(cd->node1(), cd->node2(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.order(cd->node1(), cd->node2(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.order(cd->node1(), cd->node2(), UNDO);
  }
  return 0;
}

int constr_manager::greater(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.order(cd->node2(), cd->node1(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.order(cd->node2(), cd->node1(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.order(cd->node2(), cd->node1(), UNDO);
  }
  return 0;
}

int constr_manager::neighbors(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.neighbors(cd->node1(), cd->node2(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.neighbors(cd->node1(), cd->node2(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.neighbors(cd->node1(), cd->node2(), UNDO);
  }
  return 0;
}

int constr_manager::low_margin(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.left_margin(cd->node1(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.left_margin(cd->node1(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.left_margin(cd->node1(), UNDO);
  }
  return 0;
}

int constr_manager::high_margin(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.right_margin(cd->node1(), QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.right_margin(cd->node1(), DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.right_margin(cd->node1(), UNDO);
  }
  return 0;
}

int constr_manager::range(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    if (cd->node1()) return constr_1D.range(cd->node1(), *cd->i, QUERY);
    else return constr_1D.range(*cd->i, QUERY);
  case CONSISTENT :
  case WAKEUP :
    if (cd->node1()) return constr_1D.range(cd->node1(), *cd->i, DO);
    else return constr_1D.range(*cd->i, DO);
  case SLEEP :
  case INCONSISTENT : 
    {
      int r;
      if (cd->node1()) r = constr_1D.range(cd->node1(), *cd->i, UNDO);
      else r = constr_1D.range(*cd->i, UNDO);
      // the range information goes totally lost (regrettably)
      // recover it by going through the constraint queue;
      slist_iterator_forw s(*(slist*) &constraints);

      constr_descriptor* cd2;
      while (cd2 = (constr_descriptor*)s()) {
      if (
	    ((cd2->m == WAKEUP) || (cd2->m == CONSISTENT)) &&
/***********************************/
/* Call of member function range without any parameter !!!!!!!! Can't compile this */
/* with version 2.0 of C++ compiler                                                */
//	    (cd2->cf == constr_manager::range) &&
/***********************************/
	    (cd2->node1() == cd->node1()))
	  {
	    if (cd2->node1()) 
                r &= constr_1D.range(cd2->node1(), *(cd2->i), DO);
	    else 
                r &= constr_1D.range( *(cd2->i), DO);
	  }
      }
      return r;
    }
  }
  return 0;
}

int constr_manager::cluster(constr_descriptor* cd, mode m) {
  constr_node* n;
  constr_node_list* nl;
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    nl = cd->nodelist();
    n = nl->get();
    return constr_1D.box(n, nl, QUERY);
  case CONSISTENT :
  case WAKEUP :
    nl = cd->nodelist();
    n = nl->get();
    return constr_1D.box(n, nl, DO);
  case SLEEP :
  case INCONSISTENT :
    nl = cd->nodelist();
    n = nl->get();
    return constr_1D.box(n, nl, UNDO);
  }
  return 0;
}

int constr_manager::limit(constr_descriptor* cd, mode m) {
  cd->net = &constr_1D;
  switch (m) {
  case DO :
    constraints.insert(cd, cd->priority);
    return 1;
  case UNDO :
    return constraints.del(cd,this);
  case QUERY :
    return constr_1D.bound(cd->node1(), cd->distance, QUERY);
  case CONSISTENT :
  case WAKEUP :
    return constr_1D.bound(cd->node1(), cd->distance, DO);
  case SLEEP :
  case INCONSISTENT :
    return constr_1D.bound(cd->node1(), cd->distance, UNDO);
  }
  return 0;
}

int constr_manager::delete_constraint(char* name, constr_type type, 
				      nlist* nl, int i[2], int d, int prio,
				      int name_f, int type_f, int nl_f, 
				      int i_f, int d_f, int prio_f) 
{
  constr_descriptor* cd;
  int r = 0;
  while (cd = constraints.match(name,type,nl,i,d,prio,
				name_f,type_f,nl_f,i_f,d_f,prio_f)) {
    r |= constraints.del(cd,this);
  }
  return r;
}


void constr_manager::delete_constraints(constr_type t, node* n) {
  constraints.delete_all(t,this,n);
}


/*
 * constr_manager::consistency
 * implements a binary search of inconsistent constraints
 * in the constraint queue
 */
 
int constr_manager::consistency() {
  if ((constraints.evaluated) && (constr_1D.consistency(QUERY)))
    return consistent;

  int n = constraints.length();
  
  int c = 0;
  slist_iterator_forw sc(*(slist*) &constraints);
  constr_descriptor* cdc = (constr_descriptor*) sc();
  int p = 0;
  slist_iterator_forw sp(*(slist*) &constraints);
  constr_descriptor* cdp = (constr_descriptor*) sp();

  while ((c < n) && (cdc->m != SLEEP)) {
    if (cdc->m == INCONSISTENT) consistent = 0;
    cdc = (constr_descriptor*) sc();
    c++;
    cdp = (constr_descriptor*) sp();
    p++;
  }

  int check = 1;
  
  while (p < n) {
    switch (cdp->m) {
    case SLEEP :
      cdp->m = WAKEUP;
      check &= (this->*(cdp->cf)) (cdp,WAKEUP);  // pointer to member
      break;
    case INCONSISTENT :
      consistent = 0;
      break;
    }
    cdp = (constr_descriptor*) sp();
    p++;
  }
  
  int d = p;

  // there are n constraints in the queue;
  // constraints [0 .. c-1] are not SLEEPing, i.e. CONSISTENT or INCONSISTENT;
  // constraints [c .. p-1] are WAKEdUP;
  // constraints [p .. n-1] are SLEEPing;
  // if constraints are not consistent, then in constraints [c .. d-1] is
  // at least one inconsistent constraint;
  // cdc points to constraints [c];
  // cdp points to constraints [p];
  // these assumptions hold at all marked //A// places;

  //A//;
  
  // printf("making %d constraints consistent\n",n);
  while (c < n) {
    // printf("c = %d    p = %d\n",c,p);
    //A//;
    
    // there are still constraints not evaluated;

    if ((check) && (constr_1D.consistency())) {
      //A//;
      while (c < p) {
	//A//;
	cdc->m = CONSISTENT;
	cdc = (constr_descriptor*) sc();
	c++;
	//A//;
      }
      check = 1;
      //A//;
      while (p < d) {
	//A//;
	cdp->m = WAKEUP;
	check &= (this->*(cdp->cf)) (cdp,WAKEUP);
	cdp = (constr_descriptor*) sp();
	p++;
	//A//;
      }
      //A//;
    }
    else {
      //A//;
      if (p == c+1) {
	//A//;
	cdc->m = INCONSISTENT;
	(this->*(cdc->cf)) (cdc,INCONSISTENT); // pointer to member
	cdc = (constr_descriptor*) sc();
	c++;
	consistent = 0;
	check = 1;
	//A//;
	while (p < n) {
	  //A//;
	  cdp->m = WAKEUP;
	  check &= (this->*(cdp->cf)) (cdp,WAKEUP);  // pointer to member
	  cdp = (constr_descriptor*) sp();
	  p++;
	  //A//;
	}
	d = p;
	// A //;
      }
      else {
	//A//;
	int m = (c+p)/2;
	d = p;
	while (p > m) {
	  //A//;
	  cdp = (constr_descriptor*) sp.back();
	  p--;
	  cdp->m = SLEEP;
	  (this->*(cdp->cf)) (cdp,SLEEP); // pointer to member
	  //A//;
	}
      //A//;
      }
      //A//;
    }
    //A//;
  }

  if (! constr_1D.consistency(QUERY)) {
    if (debug) {
      printf("----------FATAL ERROR----------\n\n");
    }
  }
  constraints.evaluated = 1;
  // printf("made %d constraints consistent\n\n",n);
  return consistent;
}

/*
 * old version of constr_manager::consistency
 * uses brute force algorithm to find inconsistent constraints:
 * in case of inconsistency try one after another
 *
int constr_manager::consistency() {
  if ((constraints.evaluated) && (constr_1D.consistency(QUERY))) 
    return consistent;

  // first try to use all sleeping constraints, 
  // assume all together were consistent;
  int c = 1;
  consistent = 1;
  slist_iterator_forw q1(*(slist*) &constraints);
  constr_descriptor* cd;
  while (cd = (constr_descriptor*) q1()) {
    if (cd->m == SLEEP) {
      cd->m = WAKEUP;
      c &= (*cd->cf) (cd,WAKEUP);  // pointer to member
    }
    else if (cd->m == INCONSISTENT) consistent = 0;
  }
  if ((c) && (constr_1D.consistency(DO))) {
    slist_iterator_forw q2(*(slist*) &constraints);
    while (cd = (constr_descriptor*) q2()) {
      if (cd->m == WAKEUP) {
	cd->m = CONSISTENT;
      }
    }
    constraints.evaluated = 1;
    return consistent;
  }

  // constraint network is not consistent, try to find optimimum solution,
  // brute force algorithm: try each constraint separately;

  slist_iterator_forw q3(*(slist*) &constraints);
  // find first WAKEUP constraint ... ;
  while (cd = (constr_descriptor*) q3()) {
    if (cd->m == WAKEUP) break;
  }
  // and make all subsequent constraints sleeping;
  while (cd) {
    cd->m = SLEEP;
    (*cd->cf) (cd,SLEEP); // ptr to member
    cd = (constr_descriptor*) q3();
  }

  // now try one constraint after another ... ;
  slist_iterator_forw q4(*(slist*) &constraints);
  while (cd = (constr_descriptor*) q4()) {
    if (cd->m == SLEEP) break;
  }
  // starting with the first sleeping constraint;
  while (cd) {
    cd->m = CONSISTENT;
    c = (*cd->cf) (cd,CONSISTENT); // pointer to memeber
    if ((!c) || (! constr_1D.consistency(DO))) {
      cd->m = INCONSISTENT;
      (*cd->cf) (cd,INCONSISTENT);  // pointer to member
    }
    cd = (constr_descriptor*) q4();
  }

  constraints.evaluated = 1;
  consistent = 0;
  return consistent;
}
*/

void constr_manager::output(ostream &to, int indent) {
  constraints.output(to,indent);
}
