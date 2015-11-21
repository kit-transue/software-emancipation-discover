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



#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "all.h"
#include "defaults.h"

/* external functions */



/* global vars */

defaults *default_vals = NULL;


/* functions for class graph_default */

/*
 * constructor
*/
gne_default::gne_default (char *tn, void *o, void *s, void *d, void *t)
{
  type_name = NULL;
  settypename (tn);
  orig = o;
  style = s;
  def = d;
  tmp = t;
  next = NULL;
}

/*
 * destructor
*/
gne_default::~gne_default ()
{
  if (type_name != NULL)
    delete type_name;
  if (orig != NULL)
    delete orig;
  if (style != NULL)
    delete style;
  if (def != NULL)
    delete def;
  if (tmp != NULL)
    delete tmp;
}

/*
 * settypename
*/
void gne_default::settypename (char *t)
{
  if (t != NULL) {
    if (type_name != NULL)
      delete type_name;
    type_name = new char [strlen (t) + 1];
    strcpy (type_name, t);
  }
}

/*
 * setnext
*/
void gne_default::setnext (gne_default *nd)
{
  if (nd != NULL) {
    next = nd;
  }
}


/* functions for class defaults */

/*
 * constructor
*/
defaults::defaults ()
{
  graph_defaults = NULL;
  node_defaults = NULL;
  edge_defaults = NULL;
}

/*
 * destructor
*/
defaults::~defaults ()
{
  if (graph_defaults != NULL) {
    gne_default *d;
    while (graph_defaults != NULL) {
      d = graph_defaults;
      graph_defaults = graph_defaults->getnext ();
      delete d;
    }
  }
  if (node_defaults != NULL) {
    gne_default *d;
    while (node_defaults != NULL) {
      d = node_defaults;
      node_defaults = node_defaults->getnext ();
      delete d;
    }
  }
  if (edge_defaults != NULL) {
    gne_default *d;
    while (edge_defaults != NULL) {
      d = edge_defaults;
      edge_defaults = edge_defaults->getnext ();
      delete d;
    }
  }
}

/*
 * addgraph_default
*/
void defaults::addgraph_default (char *t, graph *orig, graph *style, 
                                          graph *def, graph *tmp)
{
  if ((t != NULL) && (orig != NULL) && (style != NULL) && (def != NULL) 
      && (tmp != NULL)) {
    gne_default *nd = new gne_default (t, orig, style, def, tmp);

    nd->setnext (graph_defaults);
    graph_defaults = nd;
  }
}

/*
 * addnode_default
*/
void defaults::addnode_default (char *t, node *orig, node *style, 
                                         node *def, node *tmp)
{
  if ((t != NULL) && (orig != NULL) && (style != NULL) && (def != NULL) && 
      (tmp != NULL)) {
    gne_default *nd = new gne_default (t, orig, style, def, tmp);

    nd->setnext (node_defaults);
    node_defaults = nd;
  }
}

/*
 * addedge_default
*/
void defaults::addedge_default (char *t, edge *orig, edge *style, 
                                         edge *def, edge *tmp)
{
  if ((t != NULL) && (orig != NULL) && (style != NULL) && (def != NULL) && 
      (tmp != NULL)) {
    gne_default *nd = new gne_default (t, orig, style, def, tmp);

    nd->setnext (edge_defaults);
    edge_defaults = nd;
  }
}

/*
 * getdefault_graph
*/
graph *defaults::getdefault_graph (char *t)
{
  if (t != NULL) {
    gne_default *loop = graph_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (graph *)loop->getdefault ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getdefault_node
*/
node *defaults::getdefault_node (char *t)
{
  if (t != NULL) {
    gne_default *loop = node_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (node *)loop->getdefault ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getdefault_edge
*/
edge *defaults::getdefault_edge (char *t)
{
  if (t != NULL) {
    gne_default *loop = edge_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (edge *)loop->getdefault ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}

/*
 * getstyle_graph
*/
graph *defaults::getstyle_graph (char *t)
{
  if (t != NULL) {
    gne_default *loop = graph_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (graph *)loop->getstyle ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getstyle_node
*/
node *defaults::getstyle_node (char *t)
{
  if (t != NULL) {
    gne_default *loop = node_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (node *)loop->getstyle ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getstyle_edge
*/
edge *defaults::getstyle_edge (char *t)
{
  if (t != NULL) {
    gne_default *loop = edge_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (edge *)loop->getstyle ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}

/*
 * gettmp_graph
*/
graph *defaults::gettmp_graph (char *t)
{
  if (t != NULL) {
    gne_default *loop = graph_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (graph *)loop->gettmp ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * gettmp_node
*/
node *defaults::gettmp_node (char *t)
{
  if (t != NULL) {
    gne_default *loop = node_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (node *)loop->gettmp ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * gettmpt_edge
*/
edge *defaults::gettmp_edge (char *t)
{
  if (t != NULL) {
    gne_default *loop = edge_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (edge *)loop->gettmp ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}

/*
 * getorig_graph
*/
graph *defaults::getorig_graph (char *t)
{
  if (t != NULL) {
    gne_default *loop = graph_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (graph *)loop->getorig ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getorig_node
*/
node *defaults::getorig_node (char *t)
{
  if (t != NULL) {
    gne_default *loop = node_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (node *)loop->getorig ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * getorig_edge
*/
edge *defaults::getorig_edge (char *t)
{
  if (t != NULL) {
    gne_default *loop = edge_defaults;

    while (loop != NULL) {
      if (loop->gettypename () != NULL) {
        if (strcmp (loop->gettypename (), t) == 0)
          return (edge *)loop->getorig ();
      }
      loop = loop->getnext ();
    }
    return NULL;
  }
  else 
   return NULL;
}


/*
 * init
*/
void defaults::init ()
{
  init_node_defaults ();
  init_edge_defaults ();
  init_graph_defaults ();
}


/*
 * reset_graph_to_orig
*/
void defaults::reset_graph_to_orig ()
{
  graph *orig, *style, *def;
  gne_default *loop = graph_defaults;
  
  while (loop != NULL) {
    orig = (graph *)loop->getorig ();
    style = (graph *)loop->getstyle ();
    def = (graph *)loop->getdefault ();

    loop = loop->getnext ();
  }
}

/*
 * reset_node_to_orig
*/
void defaults::reset_node_to_orig ()
{
  node *orig, *style, *def;
  gne_default *loop = node_defaults;
  
  while (loop != NULL) {
    orig = (node *)loop->getorig ();
    style = (node *)loop->getstyle ();
    def = (node *)loop->getdefault ();

    loop = loop->getnext ();
  }
}

/*
 * reset_edge_to_orig
*/
void defaults::reset_edge_to_orig ()
{
  edge *orig, *style, *def;
  gne_default *loop = edge_defaults;
  
  while (loop != NULL) {
    orig = (edge *)loop->getorig ();
    style = (edge *)loop->getstyle ();
    def = (edge *)loop->getdefault ();

    loop = loop->getnext ();
  }
}

/*
 * reset_all_to_orig
*/
void defaults::reset_all_to_orig ()
{
  this->reset_graph_to_orig ();
  this->reset_node_to_orig ();
  this->reset_edge_to_orig ();
}

/*
 * reset_graph_to_style
*/
void defaults::reset_graph_to_style ()
{
  graph *style, *def;
  gne_default *loop = graph_defaults;
  
  while (loop != NULL) {
    style = (graph *)loop->getstyle ();
    def = (graph *)loop->getdefault ();

    loop = loop->getnext ();
  }
}

/*
 * reset_node_to_style
*/
void defaults::reset_node_to_style ()
{
  node *style, *def;
  gne_default *loop = node_defaults;
  
  while (loop != NULL) {
    style = (node *)loop->getstyle ();
    def = (node *)loop->getdefault ();

    loop = loop->getnext ();
  }
}




