#ifndef _DEFAULTS_H
#define _DEFAULTS_H

/* defaults.h (C) Stefan Manke 1990 */

class node;
class edge;
class graph;


class gne_default {
  char *type_name;
  void *orig;
  void *style;
  void *def;
  void *tmp;
  gne_default *next;
public:
  gne_default (char *, void *, void *, void *, void *);
  ~gne_default ();

  void settypename (char *);
  char *gettypename ()                     { return type_name; }

  void setorig (void *d)                   { orig = d; }
  void *getorig ()                         { return orig; }

  void setstyle (void *d)                  { style = d; }
  void *getstyle ()                        { return style; }

  void setdefault (void *d)                { def = d; }
  void *getdefault ()                      { return def; }

  void settmp (void *t)                    { tmp = t; }
  void *gettmp ()                          { return tmp; }

  void setnext (gne_default *);
  gne_default *getnext ()                  { return next; }
};


class defaults {
  gne_default *graph_defaults;
  gne_default *node_defaults;
  gne_default *edge_defaults;
public:
  defaults ();
  ~defaults ();

  void addgraph_default (char *, graph *, graph *, graph *, graph *);
  graph *getorig_graph (char *);
  graph *getstyle_graph (char *);
  graph *getdefault_graph (char *);
  graph *gettmp_graph (char *);
  void move_graph_def_to_tmp (char *);
  void compare_graph_and_set (graph *);
  void addnode_default (char *, node *, node *, node *, node *);
  node *getorig_node (char *);
  node *getstyle_node (char *);
  node *getdefault_node (char *);
  node *gettmp_node (char *);
  void move_node_def_to_tmp (char *);
  void compare_node_and_set (node *);
  void addedge_default (char *, edge *, edge *, edge *, edge *);
  edge *getorig_edge (char *);
  edge *getstyle_edge (char *);
  edge *getdefault_edge (char *);
  edge *gettmp_edge (char *);
  void move_edge_def_to_tmp (char *);
  void compare_edge_and_set (edge *);

  void init ();
  void init_node_defaults ();
  void init_edge_defaults ();
  void init_graph_defaults ();

  void reset_graph_to_orig ();
  void reset_node_to_orig ();
  void reset_edge_to_orig ();
  void reset_all_to_orig ();
  void reset_graph_to_style ();
  void reset_node_to_style ();
  void reset_edge_to_style ();
  void reset_all_to_style ();

  void write_defaults (ostream &, int);
  void write_graph_defaults (graph *, graph *, ostream &, int);
  void write_node_defaults (node *, node *, ostream &, int);
  void write_edge_defaults (edge *, edge *, ostream &, int);
};


extern defaults *default_vals;

#endif
