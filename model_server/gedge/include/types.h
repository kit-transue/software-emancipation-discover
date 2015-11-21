#ifndef _TYPES_H
#define _TYPES_H

/* types.h  (C) Stefan Manke 1990 */

class node;
class edge;

class node_type {
  char *type_name;
  int typenr;
  int size;
  node *defaults;
  node *parserdefaults;
  node_type *next;
  node_type *previous;
 public:
  node_type ();
  node_type (char *, int);
  node_type (char *, int, node *, node *);
  node_type (char *, int, node *, node *, int);
  ~node_type ();
  void SetTypename (char *);
  char *GetTypename ()                { return type_name; }
  void SetTypenr (int i)              { typenr = i; }
  int GetTypenr ()                    { return typenr; }
  void SetSize (int i)                { size = i; }
  int GetSize ()                      { return size; }
  void SetDefaults (node *n)          { defaults = n; }
  node *GetDefaults ()                { return defaults; }
  void SetParserDefaults (node *n)    { parserdefaults = n; }
  node *GetParserDefaults ()          { return parserdefaults; }
  void SetNext (node_type *nt)        { next = nt; }
  node_type *GetNext ()               { return next; }
  void SetPrevious (node_type *nt)    { previous = nt; }
  node_type *GetPrevious ()           { return previous; }

  node *CopyDefault ();
  node *CopyParserDefault ();
};

class edge_type {
  char *type_name;
  int typenr;
  int size;
  edge *defaults;
  edge *parserdefaults;
  edge_type *next;
  edge_type *previous;
 public:
  edge_type ();
  edge_type (char *, int);
  edge_type (char *, int, edge *, edge *);
  edge_type (char *, int, edge *, edge *, int);
  ~edge_type ();
  void SetTypename (char *);
  char *GetTypename ()                { return type_name; }
  void SetTypenr (int i)              { typenr = i; }
  int GetTypenr ()                    { return typenr; }
  void SetSize (int i)                { size = i; }
  int GetSize ()                      { return size; }
  void SetDefaults (edge *e)          { defaults = e; }
  edge *GetDefaults ()                { return defaults; }
  void SetParserDefaults (edge *e)    { parserdefaults = e; }
  edge *GetParserDefaults ()          { return parserdefaults; }
  void SetNext (edge_type *et)        { next = et; }
  edge_type *GetNext ()               { return next; }
  void SetPrevious (edge_type *et)    { previous = et; }
  edge_type *GetPrevious ()           { return previous; }

  edge *CopyDefault ();
  edge *CopyParserDefault ();
};

#endif
