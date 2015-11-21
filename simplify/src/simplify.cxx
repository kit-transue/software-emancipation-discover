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
#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include "InclusionTree.h"
#include "astnodeList.h"
#include "astnodeStruct.h"
#include "attributeList.h"
#include "attributeStruct.h"
#include "cleanup.h"
#include "locationList.h"
#include "maplineStruct.h"
#include "simplify_globals.h"
#include "symbolStruct.h"
#include "uintList.h"

#ifdef _WIN32
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif

bool CompareMaplines(mapline *, mapline *);
bool CompareRelations(Relation *, Relation *);
void SimplifyTree(int, vector<int> &, vector<SymLoc *> &);
void FindAllDependencies();
int MarkAllDependencies(InclusionNode*);
int MarkIfHasMarkedSubIncludes();
void RunCanExcludeCmd(const char*);
void OutputDependency(InclusionNode*, InclusionNode*, int symid);
void Output(const char* str);
void AddInclusionLinesToTree();
void AddMapLinesToTree();
void ModifySrc(const char* file, vector<int>& remove, vector<SymLoc*>& subs, 
               int nocomments, int short_sub_names);
int  QueryResults(int level, InclusionNode *node);

int Simplify(int do_query, int allow_sub, const char* can_exclude_cmd, 
	     int modify, int nocomments, int short_sub_names)
{
  FindAllDependencies();

  GetInclusionTree()->GetRoot()->Mark();
  if (can_exclude_cmd)
    {
      RunCanExcludeCmd(can_exclude_cmd);
    }

  if (!allow_sub)
    MarkIfHasMarkedSubIncludes();

  int marked_new1 = 1;
  int marked_new2 = 0;
  while (marked_new1 || marked_new2)
    {
      marked_new2 = 0;
      marked_new1 = MarkAllDependencies(GetInclusionTree()->GetRoot());
      if (marked_new1 && !allow_sub)
	marked_new2 = MarkIfHasMarkedSubIncludes();
    }

  if(do_query != 0) {
      int cont = QueryResults(0, GetInclusionTree()->GetRoot()->GetFirst());
      if(cont == 0)
	  return NO_CHANGES;

      marked_new1 = 1;
      marked_new2 = 0;
      while (marked_new1 || marked_new2){
	  marked_new2 = 0;
	  marked_new1 = MarkAllDependencies(GetInclusionTree()->GetRoot());
	  if (marked_new1 && !allow_sub)
	      marked_new2 = MarkIfHasMarkedSubIncludes();
      }
  }

  vector<int> rem;
  vector<SymLoc *> sub;

  SimplifyTree(allow_sub, rem, sub);

  if (modify && rem.size() > 0)
    ModifySrc(GetSourceName(), rem, sub, nocomments, short_sub_names);
  else
    return NO_CHANGES;

  int size = sub.size();
  for (int i = 0; i < size; ++i)
    {
      SymLoc* s = (SymLoc*)sub[i];
      delete s;
    }

  return NO_ERRORS;
}

////////////////////////////////////////////////////////////////
//This function constructs the inclusion tree only if it has not
//already been constructed.  It is called each time SMT file "" 
//is seen in the IF;  wmm says this will first be seen after all
//inclusion information is generated.  sschmidt 9/30/98
void ConstructInclusionTree()
{
  if (GetInclusionTree()->GetRoot() == 0)
    {
        vector<Relation *> &rels = GetIncludeRels();
        sort(rels.begin(), rels.end(), CompareRelations);

      InclusionNode* root = new InclusionNode(source_id);
      GetInclusionTree()->SetRoot(root);

      int size = GetIncludeRels().size();
      for (int i = 0; i < size; ++i)
        {
          Relation* rel = (Relation*) GetIncludeRels()[i];

          int parentid = rel->source;
          int childid = rel->target;
          InclusionNode* node = new InclusionNode(childid); 
          InclusionNode* parent = GetInclusionTree()->FindNodeById(parentid);
	  if(rel->preinclude)
	      node->SetPreinclude(1);

          assert(parent);

          if (parent)
	    {
	      InclusionTree::AddChild(parent, node);
	    }
        }

      DestroyIncludeRels();

      AddMapLinesToTree();
      AddInclusionLinesToTree();
    }
}
/////////////////////////////////////////////////
void AddInclusionLinesToTree()
{
  InclusionNode* root = GetInclusionTree()->GetRoot();
  InclusionNode* node = root->GetFirst();
  
  int i = 0;
  while (node){
      if(!node->IsPreinclude()) {
	  int line = GetIncludeLines()[i];
	  node->SetIncludedAtLine(line);
	  ++i;
      }
      node = node->GetNext();
  }
}

/////////////////////////////////////////////////
//This function adds line information to all inclusion tree
//nodes. This info is used later for mapping.
void AddMapLinesToTree()
{
    sort(GetMapLines().begin(), GetMapLines().end(), CompareMaplines);

  InclusionNode* current = GetInclusionTree()->GetRoot();
  
  int size = GetMapLines().size();

  for (int i = 0; i < size; ++i)
    {
      mapline* ml = (mapline*) GetMapLines()[i];
      const char* file = ml->sourceFile;
      int start = ml->targetStart;
      int end = start + ml->sourceEnd - ml->sourceStart;
      InclusionNode *node = NULL;
      if (ml->sourceStart <= 1) {
	// Looks to be a child.
	node = InclusionTree::FindChildByFile(current, file);

	if (!node)
	  {
	    current->SetDone(); //cannot come back to this node again
	    node = InclusionTree::FindParentByFile(current, file);
	  }
      }
      else {
	// Looks to be parent.
	node = InclusionTree::FindParentByFile(current, file);
	if (node != NULL) {
    	  current->SetDone();
	}
	else {
	  node = InclusionTree::FindChildByFile(current, file);
	}
      }

      assert(node);

      node->AddRegion(start, end, ml->sourceStart);
      current = node;
    }

  DestroyMaps();
}
/////////////////////////////////////////////////
int IsDefinition(int id, int line)
{
  int ret = 0;

  symbol* def = GetSymbols()[id];
  if (def && def->loc)
    {
      if (def->loc->start_line == line)
	ret = 1;
    }

  return ret;
}

inline int IsDefinition(const SymLine* use)
{
  return IsDefinition(use->id, use->line);
}

int IsDeclaration(const SymLine* use)
{
  int ret = 0;

  uintList* decl = GetDeclarations()[use->id];
  if (decl)
    {
      int length = decl->size();
      for (int n = 0; n < length; ++n)
	{
	  int line = (*decl)[n];
	  if (line == use->line)
	    {
	      ret = 1;
	      break;
	    }
	}
    }

  return ret;
}
/////////////////////////////////////////////////
int Unmap(int line, int& unmappedline, const char*& file)
{
  int ret = 0;

  InclusionNode* node = GetInclusionTree()->FindNodeByLine(line);
  if (node)
    {
      ret = 1;
      symbol* sym = GetSymbols()[node->GetId()];
      file = sym->name;

      Region* reg = node->FindRegion(line);
      unmappedline = reg->GetSourceStart() + line - reg->GetStart();
    }

  return ret;
}
/////////////////////////////////////////////////
int HasDuplicate(int line, uintList& list)
{
  int ret = 0;

  int oline1, oline2;
  const char* ofile1;
  const char* ofile2;

  if (Unmap(line, oline1, ofile1))
    {
      int size = list.size();
      for (int i = 0; i < size; ++i)
	{
	  int line = list[i];
	  if (Unmap(line, oline2, ofile2))
	    {
	      if (oline1 == oline2 && strcmp(ofile1, ofile2) == 0)
		{
		  ret = 1;
		  break; 
		}
	    }
	}
    }

  return ret;
}

/////////////////////////////////////////////////
void ProcessDeclaration(int id, location* loc)
{
  int size = GetDeclarations().size();
  uintList* list = 0;
  if (id >= size || GetDeclarations()[id] == 0)
    {
      list = CreateUintList();
      if (list == 0)
	{
	  Error(OUT_OF_MEMORY);
	  return;
	}
      GetDeclarations()[id] = list;

      InsertLastIntoUintList(list, loc->start_line);
    }
  else
    {
      list = GetDeclarations()[id];

      symbol* sym = GetSymbols()[id];

      /* do not insert duplicate declarations for functions
	 duplicate declarations some from the same file and line
	 it can happen if the same header is inlcluded in more than one place,
	 and it does not have #ifndef #endif wrappers to protect multiple 
	 inclusions */
      if (strcmp(sym->kind, "function") != 0 || 
	  !HasDuplicate(loc->start_line, *list))
	{
	  InsertLastIntoUintList(list, loc->start_line);
	}
    }
}

#define MAX_ANCESTORS 100
static location *astnode_ancestors[MAX_ANCESTORS];
static int astnode_ancestor_count = 0;

int AddToCppUses(int id, location* loc)
{
  static location* prev_loc = 0;
  
  //if a macro expands another macro, loc is 0, but we can use previous
  //location
  if (loc == 0) 
    loc = prev_loc;

  if (!loc)
    return 0;

  prev_loc = loc;

  // A 'defined' pseudo-macro shows up with its location in the file
  // named "".  Look up to the ancestor that reflects its real position.
  if (0 < astnode_ancestor_count && astnode_ancestor_count <= MAX_ANCESTORS) {
      location *aloc = astnode_ancestors[0];
      char *afilename = aloc != 0 ? aloc->filename : 0;  // the file name to match
      int a = astnode_ancestor_count;
      location *xloc = loc;   // the current potential match
      for (;;) {
          if (xloc != 0) {
	      char *xfilename = xloc->filename;
	      if (xfilename == afilename || (xfilename != 0 && afilename != 0 && strcmp(xfilename, afilename) == 0)) {
    	    	  loc = xloc;
    	          break;
	      }
	  }
    	  // The current location is in some other file.  Check the next ancestor.
    	  if (a == 0) {
    	      break;
	  }
    	  a -= 1;
    	  xloc = astnode_ancestors[a];
      }
  }

  int ret = 1;
  
  SymLoc* newuse = new SymLoc(id, loc->start_line, loc->filename);

  if (newuse)
    {
      GetCppUses().push_back(newuse);
    }
  else
    {
      ret = 0;
      Error(OUT_OF_MEMORY);
    }

  return ret;
}

void ProcessInclude(int id, const location* loc)
{
  if (strcmp(loc->filename, GetSourceName()) == 0) //only interested in direct inclusions
    {
      GetIncludeLines().push_back(loc->start_line);
    }
}

int IsDeclarationNode(const char* node)
{
  return 
#if 0 /* If no GNU_compatible_AST option is specified when parsing a file
	 use this comparsion */
    strcmp(node, "declarator") == 0 ||
    strcmp(node, "name_clause") == 0;
#endif

/* if GNU_compatible_AST option is specified when parsing a file with
   edg, then node names below should be used instead */

    strcmp(node, "function_decl") == 0 ||
    strcmp(node, "name_clause") == 0 ||
    strcmp(node, "field_decl") == 0 ||
    strcmp(node, "var_decl") == 0;
}

int IsCppUseNode(const char* node)
{
  return 
    strcmp(node, "cpp_macro_call") == 0 ||
    strcmp(node, "cpp_ifndef") == 0 ||
    strcmp(node, "cpp_ifdef") == 0 ||
    strcmp(node, "defined") == 0;
}

inline int IsIncludeNode(const char* node)
{
  return strcmp(node, "cpp_include") == 0;
}

/* if #include is inside some statement, it should not be removed */
void MarkNotTopLevelIncludes(location* loc, int id)
{
  InclusionNode* root = GetInclusionTree()->GetRoot();

  InclusionNode* node = root->GetLast();
  while (node)
    {
      if (!node->IsMarked())
	{
          vector<Region *> &regions = node->GetRegions();
	  int size = regions.size();
	  for (int i = size - 1; i >= 0; --i)
	    {
	      Region* reg = regions[i];
	      
	      //since we start with the last node, if this node proceeds
	      //loc, then all the nodes before this one do too, so there
	      //is no need to check any more
	      if (reg->GetEnd() < loc->start_line)
		return;

	      if (reg->GetStart() >= loc->start_line &&
		  reg->GetStart() <= loc->end_line)
		{
		  node->Mark();
		  OutputDependency(root, node, id);
		  break;
		}
	    }
	}
      
      node = node->GetPrev();
    }
}

int ProcessAstNode(astnode* node, int root)
{
  if (!node)
    return 0;

  location* loc = 0;
  if (node->location && node->location->size() > 0)
    loc = (*node->location)[0];

  if (node->identifier)
    {
      const char* name = node->node;
      int id = node->identifier;
      
      if (IsDeclarationNode(name) && loc && 
	  !node->definition && !IsDefinition(id, loc->start_line))
	{
	  ProcessDeclaration(id, loc);
	}
      else if (IsCppUseNode(name))
	{
	  AddToCppUses(id, loc);
	}
      else if (IsIncludeNode(name) && loc)
	{
	  ProcessInclude(id, loc);
	}

      if (root && loc)
	MarkNotTopLevelIncludes(loc, id);
    }

  astnodeList* children = node->children;
  if (children)
    {
      if (astnode_ancestor_count < MAX_ANCESTORS) {
          astnode_ancestors[astnode_ancestor_count] = loc;
      }
      astnode_ancestor_count += 1;

      int size = children->size();
      for (int i = 0; i < size; ++i)
	{
	  astnode* node = (*children)[i];
	  ProcessAstNode(node, root);
	}
      astnode_ancestor_count -= 1;
    }
  return 1;
}

bool CompareMaplines(mapline *m1, mapline *m2)
{
    if (m1->targetStart < m2->targetStart) {
        return true;
    }
    return false;
}

bool CompareRelations(Relation *r1, Relation *r2)
{
  if (r1->source < r2->source)
      return true;
  else if (r1->source > r2->source)
      return false;
  else if (r1->target < r2->target)
      return true;
  else if (r1->target > r2->target)
      return false;
  else
      return false;
}

void MarkSubnodes(InclusionNode *node)
{
  InclusionNode *child = node->GetFirst();
  while(child)
    {
      child->Mark();
      MarkSubnodes(child);
      child = child->GetNext();
    }
}

int MarkDependencies(InclusionNode *node)
{
  int new_node_marked = 0;
  
  vector<InclusionNode *> &depend = node->GetDependencies();
  int size = depend.size();
  for (int i = 0; i < size; ++i)
    {
      InclusionNode* n = depend[i];
      if(n->IsMarked() == 0)
	{
	  new_node_marked  = 1;
	  n->Mark();
	  MarkSubnodes(n);
	}
    }

  return new_node_marked;
}

int MarkAllDependencies(InclusionNode *node)
{
  int nodes_marked = 0;

  if(node->IsMarked())
    if(MarkDependencies(node))
      nodes_marked = 1;

  InclusionNode *child = node->GetFirst();

  if(child)
    if(MarkAllDependencies(child))
      nodes_marked = 1;

  node = node->GetNext();
  if(node)
    if(MarkAllDependencies(node))
	    nodes_marked = 1;

  return nodes_marked;
}

int CheckAttribute(int id, const char* name)
{
  int ret = 0;

  attributeList* list = (attributeList*) GetAttributes()[id];
  if (list)
    {
      int size = list->size();
      for (int i = 0; i < size; ++i)
	{
	  attribute* attr = (*list)[i];
	  if (attr && strcmp(attr->name, name) == 0)
	    {
	      ret = 1;
	      break;
	    }
	}
    }

  return ret;
}

void AddDefinitions()
{
  int size = GetSymbols().size();
  for (int i = 0; i < size; ++i)
    {
      symbol* sym = GetSymbols()[i];
      
      if (sym && sym->loc)
	{
	  if (strcmp(sym->kind, "variable") == 0)
	    {
	      //do not add definitions of local variables
	      if (CheckAttribute(i, "auto"))
		continue;

	      //do not add definitions of static global variables
	      if (CheckAttribute(i, "static"))
		continue;
	    }
	  else if (strcmp(sym->kind, "function") == 0)
	    {
	      //do not add definitoins of inline functions
	      if (CheckAttribute(i, "inline"))
		continue;
	    }
	  else
	    continue; //only need definitions of global vars and functions

	  InclusionNode* node = GetInclusionTree()->FindNodeByLine(sym->loc->start_line);
	  assert(node);
	  node->Mark();
	  OutputDependency(GetInclusionTree()->GetRoot(), node, i);
	}
    }
  
  DestroyAttributes();

}

void AddCppUses()
{
  /* for cpp derectives, such as cpp_ifdef [id], cpp_macro_call [id], etc.
     we use "define" relation to find dependencies */

  const char* prev_file = "";
  vector<InclusionNode *> nodes;

  int sz = GetCppUses().size();
  for (int n = 0; n < sz; ++n)
    {
      SymLoc* use = (SymLoc*)GetCppUses()[n];
      
      if (strcmp(use->file, prev_file) != 0)
	{
	  nodes.clear();
	  GetInclusionTree()->FindAllByFile(use->file, nodes);
	}

      prev_file = use->file;

      int size = nodes.size();
      for (int i = 0; i < size; ++i)
	{
	  InclusionNode* use_node = (InclusionNode*) nodes[i];
	  if (use_node->ContainsSourceLine(use->line))
	    {
	      int def_id = (int) GetDefineRels()[use->id];

	      // use_node is an occurrence of the file containing use.
    	      // def_id is the file defining the used symbol.
    	      // Now handle the case of 
    	      // #ifdef X     where X is subsequently defined in an inclusion.
    	      // It should not create a dependency.
	      if (def_id > 0 && def_id != use->id)
		{
    	    	  // Search for the symbol's defining file among the use file's explicit #includes.
		  InclusionNode* def_node = 
		    InclusionTree::FindNodeById(use_node->GetFirst(), def_id);

		  if (def_node) //def node is a child of use node
		    {
		      if (GetInclusionTree()->GetRoot() == use_node) // main 'c' file is user of definition
			{
			    if(def_node->IsPreinclude() == 0) {  // preinclude file does not have line number info
				InclusionNode* cur = def_node;
				while (cur->GetIncludedAtLine() < 0) // is -1 except for files
    	    	    	    	    	    	    	    	     // directly included from the main file
				    cur = cur->GetParent();
				
				if (use->line < cur->GetIncludedAtLine())
				    {
					//if definition is included after use, skip
					continue;
				    } 
			    }
			}
		    }
		  else
		    {
		      // The symbol is not defined by one of use_node's #includes.
		      def_node = GetInclusionTree()->FindNodeById(def_id);
		      if (use_node->GetId() <= def_id)
			continue; //definition is later than use, skip
		    }

		  use_node->AddDependency(def_node);
		  OutputDependency(use_node, def_node, use->id);
		}
	    }
	}
    }
  DestroyCppUses();
}

void FindAllDependencies()
{
  AddDefinitions();
  AddCppUses();

  int size = GetUses().size();
  for (int i = 0; i < size; ++i)
    {
      SymLine* use = (SymLine*) GetUses()[i];
      assert(use);
      
      if (IsDefinition(use) || IsDeclaration(use))
	continue;

      InclusionNode* node = GetInclusionTree()->FindNodeByLine(use->line);
      assert(node);
      
      symbol* s = GetSymbols()[use->id];
      assert(s);

      int only_first = 1;

      //for functions, having definition is not enough. Need to have all declarations
      //because of possible default arguments
      //for all other symbols, if definition is found, do not need declarations
      if (strcmp(s->kind, "function") == 0)
	only_first = 0;
      
      //check if has definition
      symbol* def = GetSymbols()[use->id];
      if (def && def->loc)
	{
	  assert(def->loc);
	  if (def->loc->start_line < use->line)
	    {
	      InclusionNode* defnode = GetInclusionTree()->FindNodeByLine(def->loc->start_line);
	      assert(defnode);
	      node->AddDependency(defnode);
	      OutputDependency(node, defnode, use->id);
	      
	      if (only_first)
		continue;
	    }
	}

      uintList* decl = GetDeclarations()[use->id];
      if (decl)
	{
	  int length = decl->size();
	  for (int n = 0; n < length; ++n)
	    {
	      int line = (*decl)[n];
	      if (line < use->line)
		{
		  InclusionNode* declnode = GetInclusionTree()->FindNodeByLine(line);
		  assert(declnode);
		  node->AddDependency(declnode);
		  OutputDependency(node, declnode, use->id);

		  if (only_first)
		    continue;
		}
	    }
	}
    }

  DestroyUses();
  DestroyDeclarations();

}

void AddMarkedSubIncludes(InclusionNode* node, const char* sub_for, 
			  int line, vector<SymLoc *> &subarr)
{
  InclusionNode* child = node->GetFirst();

  while (child)
    {
      if (child->IsMarked())
	{
	  symbol* sym = GetSymbols()[child->GetId()];
	  
	  SymLoc* newsub = new SymLoc(-1, line, sym->name);
	  if (newsub)
	    subarr.push_back(newsub);
	  else
	    Error(OUT_OF_MEMORY);

	  //logging
	  char str[256];
	  sprintf(str, "substitute %s for %s\n", sym->name, sub_for);
	  Output(str);
	}
      else
	AddMarkedSubIncludes(child, sub_for, line, subarr);
      
      child = child->GetNext();
    }
}

void SimplifyTree(int allow_sub, vector<int> &rem, vector<SymLoc *> &sub)
{
  Output("The following changes will be made to ");
  Output(GetSourceName());
  Output("\n\n");

  InclusionNode* node = GetInclusionTree()->GetRoot()->GetFirst();
  while(node)
    {
      if (!node->IsMarked() && !node->IsPreinclude())
	{
	  rem.push_back(node->GetIncludedAtLine());

	  //logging
	  char str[256];
	  symbol* sym = GetSymbols()[node->GetId()];
	  sprintf(str, "remove %s at line %d\n", sym->name, node->GetIncludedAtLine());
	  Output(str);

	  if (allow_sub)
	    {
	      AddMarkedSubIncludes(node, sym->name, 
				   node->GetIncludedAtLine(), sub);
	    }
	}
   
      node = node->GetNext();
    }

  Output("\n");
}

int HasMarkedSubIncludes(InclusionNode* node)
{
  InclusionNode* child = node->GetFirst();
  int ret = 0;
  while (child && ret != 1)
    {
      if (child->IsMarked())
	ret = 1;
      else
	ret = HasMarkedSubIncludes(child);

      child = child->GetNext();
    }

  return ret;
}

/* this functions marks all direct inclusions if they have any marked subincludes.
   This is used if substitutions are disabled */
int MarkIfHasMarkedSubIncludes()
{
  InclusionNode* node = GetInclusionTree()->GetRoot()->GetFirst();
  int marked_new = 0;

  while (node)
    {
      if (!node->IsMarked() && HasMarkedSubIncludes(node))
	{
	  node->Mark();
	  marked_new = 1;
	}
      node = node->GetNext();
    }

  return marked_new;
}

void RunCanExcludeCmd(const char* cmd)
{
  InclusionNode* node = GetInclusionTree()->GetRoot()->GetFirst();
  string command = cmd;
  while (node)
    {
      symbol* sym = GetSymbols()[node->GetId()];
      assert(sym);

      const char* name = sym->name;
      command += " \"";
      command += name;
      command += "\"";

      node = node->GetNext();
    }

  FILE* f = popen(command.c_str(), "r");
  if (!f)
    {
      Output("ERROR -- Cannot execute command: ");
      Output(command.c_str());
      Output("\n");
      return;
    }

  while (!feof(f))
    {
      char file[256];
      if (fgets(file, 255, f))
	{
	  int len = strlen(file);
	  if (file[len - 1] == '\n')
	    {
	      file[len - 1] = '\0'; //remove \n
	      if (file[len - 2] == '\r')
		file[len - 2] = '\0'; //remove \r
	    }

          char buffer[PATH_MAX];
	  char* path = realpath(file, buffer);
	  if (path)
	    {
	      node = GetInclusionTree()->GetRoot()->GetFirst();
	      while (node)
		{
		  symbol* sym = GetSymbols()[node->GetId()];
		  if (strcmp(path, sym->name) == 0)
                      {
		      node->Mark();
		    }
		  node = node->GetNext();
		}
	    }
	}
    }

  pclose(f);
}

static int MarkNodeById(InclusionNode *node, int id)
{
    while(node) {
	if(node->GetId() == id){
	    node->Mark();
	    return 1;
	}
	if(MarkNodeById(node->GetFirst(), id) != 0)
	    return 1;
	node = node->GetNext();
    }    
    return 0;
}

static char* ReplaceBackslashes(char *oldname, char *newname)
{
  if(oldname == NULL || newname == NULL)
    return NULL;
  for(int i = 0; ; i++) {
    char curchar = oldname[i];
    newname[i] = (curchar != '\\') ? curchar : '/';
    if(curchar == 0)
      break;
  }
  return newname;
}

static void DumpTree(int level, InclusionNode *node)
{
  char tmp[16000];
    while(node) {
	if(!node->IsPreinclude()){  // do not dump artificially included files
	    symbol* sym = GetSymbols()[node->GetId()];
	    printf("%d@%d@%d@%d@%s\n", level, (int)node->GetIncludedAtLine(), 
             (int)node->IsMarked(), node->GetId(), 
             ReplaceBackslashes(sym->name, tmp));
	    DumpTree(level + 1, node->GetFirst());
	}
	node = node->GetNext();
    }
}

int QueryResults(int level, InclusionNode *node)
{
    DumpTree(level, node);
    printf("-1\n");
    fflush(stdout);
    int id;
    do {
	scanf("%d", &id);
	if(id != 0) {
	    MarkNodeById(node, id);
	}
    } while(id != 0 && id != -1);
    return (id != -1);
}
