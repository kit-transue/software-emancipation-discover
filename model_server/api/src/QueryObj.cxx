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
#include <QueryObj.h>

init_relational(QueryObj, Relational);
init_relational(intObj, Relational);

init_rel_or_ptr(intObj,id,0,QueryObj,node,0);

intSet QueryObj::idents;
QueryObj *QueryObj::root = NULL;

QueryObj::QueryObj()
{
  Associate(this);
}

QueryObj::QueryObj(const QueryObj &other)
{
  Associate(this);
  *this = other;
}

QueryObj::~QueryObj()
{
  intObj *idptr = (intObj *)node_get_id(this);
  if (idptr) {
    idents.remove(*idptr);
    delete idptr;
  }
}

QueryObj &QueryObj::operator=(const QueryObj &other)
{
  if (this != &other) {
    name = other.name;
    attributes = other.attributes;
  }

  return *this;
}

GenericTree *QueryObj::CloneNode() const
{
  return new QueryObj(*this);
}

int QueryObj::Valid() const
{
  return (NodeToId(this) != INVALID_ID);
}

int QueryObj::HasAttribute(char const *key)
{
  int exists = (attributes.Get(key) != NULL);
  return exists;
}

int QueryObj::GetAttribute(char const *key, genString &val)
{
  int ret = 0;

  char const *cp = attributes.Get(key);
  if (cp) {
    val = attributes.Get(key);
    ret = 1;
  }

  return ret;
}

void QueryObj::SetAttribute(char const *key, char const *val)
{
  attributes.Put(key, val);
}

void QueryObj::RemoveAttribute(char const *key)
{
  attributes.Remove(key);
}

QueryObj *QueryObj::AttachRoot()
{
  QueryObj *ret = NULL;

  if (!root) {
    root = Create();
  }

  if (root) {
    DetachTree();
    ret = (QueryObj *)root->InsertChild(this);
  }

  return ret;
}

QueryObj *QueryObj::Lookup(char const * const * path)
{
  char const *cp;
  int i = 0;
  QueryObj *cur = (QueryObj *)this;

  if (path) {
    while (cp = path[i++]) {
      int found = 0;
      cur = (QueryObj *)cur->firstChild;
      while (cur && !found) {
        if (cur->name == cp) {
          found = 1;
        } else {
          cur = (QueryObj *)cur->nextSibling;
        }
      }
      if (!found) {
        break;
      }
    }
  }

  return cur;
}

int QueryObj::Define(TclList &def)
{
  int ret = 1;

  int size = def.Size();
  int pos = 1;
  if (pos < size) {
    do {
      char const *cp = def[pos];
      if ((OSapi_strcmp((char *)cp, "=") == 0) && (pos + 1 < size)) {
        /* attribute */
        char const *key = def[pos - 1];
        char const *val = def[pos + 1];
        SetAttribute(key, val);
        pos += 3;
      } else {
        /* tree */
        char const *entry = def[pos - 1];
        TclList subdef(def[pos]);
        QueryObj *child = Create();
        if (child) {
          child->SetName(entry);
          if (subdef.Length() > 0) {
            if (! (child->Define(subdef))) {
              /* subtree definition failed */
              child->Remove();
              delete child;
              ret = 0; break;
            }
	  }
          child = (QueryObj *)InsertChild(child);
          if (!child) {
            /* insertion failed */
            child->Remove();
            delete child;
            ret = 0; break;
	  }
	} else {
          /* creation failed */
          ret = 0; break;
        }
        pos += 2;
      }
    } while (pos < size);
  } else {
    /* not enough list elements for a valid definition */
    ret = 0;
  }

  return ret;
}

int QueryObj::Dump(TclList &def)
{
  char const *entry = name;
  def += entry ? entry : "";

  TclList subdef;

  /* attributes */
  Object *obj;
  ForEach (obj, attributes) {
    Property *prop = (Property *)obj;
    char const *key = prop->get_name();
    char const *val = prop->get_value();
    if (!key) key = "";
    if (!val) val = "";
    subdef += key;
    subdef += "=";
    subdef += val;
  }

  /* tree */
  QueryObj *cur = (QueryObj *)firstChild;
  if (!cur) {
    subdef += (char *)"";
  } else {
    do {
      cur->Dump(subdef);
      cur = (QueryObj *)cur->nextSibling;
    } while (cur);
  }

  def += subdef;

  return 1;
}

int QueryObj::Print(genString &str, char const **keys, int curdepth, int maxdepth)
{
  if (curdepth != maxdepth) {
    /* attributes */
    TclList list;
    genString s;
    s.printf("%d", curdepth);
    list += (char *)s;
    s.printf("%d", NodeToId(this));
    list += (char *)s;
    char *cp = (char *)name;
    list += cp ? cp : (char *)"";
    if (keys) {
      char const *cp; int i = 0;
      while (cp = keys[i++]) {
        char *val = NULL;
        if (GetAttribute(cp, s)) {
          val = (char *)s;
        }
        list += val ? val : (char *)"";
      }
    }
    s.printf("%s\n", (char *)list);
    str += s;

    /* tree */
    QueryObj *cur = (QueryObj *)firstChild;
    while (cur) {
      cur->Print(str, keys, curdepth + 1, maxdepth);
      cur = (QueryObj *)cur->nextSibling;
    }
  }

  return 1;
}

QueryObj *QueryObj::Create()
{
  QueryObj *node = new QueryObj;

  if (!node->Valid()) {
    delete node;
    node = NULL;
  }

  return node;
}

QueryObj *QueryObj::RootLookup(char const ** path)
{
  QueryObj *match = root ? root->Lookup(path) : NULL;
  return match;
}

QueryObj *QueryObj::IdToNode(QueryObjId id)
{
  QueryObj *node = NULL;

  intObj t(id);
  intObj *idptr = (intObj *)idents.findObjectWithKey(t);

  if (idptr) {
    node = id_get_node(idptr);
  }

  return node;
}

QueryObjId QueryObj::NodeToId(const QueryObj *node)
{
  QueryObjId id = INVALID_ID;

  intObj *idptr = node_get_id(node);

  if (idptr) {
    id = idptr->value();
  }

  return id;
}

QueryObjId QueryObj::NextId()
{
  QueryObjId ret = INVALID_ID;

  static QueryObjId cursor = 1;

  if (cursor == INVALID_ID)
    ++cursor;

  QueryObjId start = cursor;

  int ok = 1;
  while (IdToNode(cursor)) {
    if (cursor + 1 == INVALID_ID) {
      cursor += 2;
    } else {
      cursor += 1;
    }
    if (cursor == start) {
      ok = 0;
      break;
    }
  }

  if (ok) {
    ret = cursor;
    ++cursor;
  }

  return ret;
}

QueryObjId QueryObj::Associate(QueryObj *node)
{
  QueryObjId id = NextId();

  if (id != INVALID_ID) {
    intObj *idptr = new intObj(id);
    idents.add(*idptr);
    id_put_node(idptr, node);
  }

  return id;
}

int QueryObjCmd::Register()
{
  new cliCommandInfo("odb", QueryObjCmd::Process);

  return 0;
}

static int dummy = QueryObjCmd::Register();

inline int streq(char const *a, char const *b) \
  { return (OSapi_strcmp((char *)a, (char *)b) == 0); }

static void setresult(Interpreter *shell, char const *fmt, ...)
{
  genString msg;
  va_list ap1;
  va_list ap2;
  va_start(ap1, fmt);
  va_start(ap2, fmt);
  msg.vsprintf(fmt, ap1, ap2);
  va_end(ap2);
  va_end(ap1);
  Tcl_SetResult(shell->interp, (char *)msg, TCL_VOLATILE);
}

inline void error_cantfind(genString &err, char const *id)
{
  err.printf("cannot find item with id: \"%s\".", id);
}

inline void error_failure(genString &err)
{
  err = "operation failed.";
}

inline int getid(QueryObj *node)
  { return (int)QueryObj::NodeToId(node); }

inline QueryObj *getnode(int id)
  { return QueryObj::IdToNode((QueryObjId)id); }

int QueryObjCmd::Process(ClientData cd, Tcl_Interp * /*interp*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  Interpreter *shell = (Interpreter *)cd;

  int help = 0;

  if (argc >= 2) {

    char const *subcmd = argv[1];

    genString err;

    /******************************************************************************/
         if (streq(subcmd, "new"))      ret = NewCmd      (shell, argc, argv, err);
    else if (streq(subcmd, "delete"))   ret = DeleteCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "copy"))     ret = CopyCmd     (shell, argc, argv, err);
    else if (streq(subcmd, "insert"))   ret = InsertCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "hasatt"))   ret = HasAttCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "getatt"))   ret = GetAttCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "setatt"))   ret = SetAttCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "delatt"))   ret = DelAttCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "listatt"))  ret = ListAttCmd  (shell, argc, argv, err);
    else if (streq(subcmd, "define"))   ret = DefineCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "dump"))     ret = DumpCmd     (shell, argc, argv, err);
    else if (streq(subcmd, "print"))    ret = PrintCmd    (shell, argc, argv, err);
    else if (streq(subcmd, "getname"))  ret = GetNameCmd  (shell, argc, argv, err);
    else if (streq(subcmd, "setname"))  ret = SetNameCmd  (shell, argc, argv, err);
    else if (streq(subcmd, "makeroot")) ret = MakeRootCmd (shell, argc, argv, err);
    else if (streq(subcmd, "lookup"))   ret = LookupCmd   (shell, argc, argv, err);
    else if (streq(subcmd, "relation")) ret = RelationCmd (shell, argc, argv, err);
    /******************************************************************************/
    else {
      /* invalid subcommand */
      help = 1;
    }

    if ((ret != TCL_OK) && (err.length() > 0)) {
      setresult(shell, "%s %s: %s", argv[0], argv[1], (char *)err);
    }

  } else {
    /* not enough args */
    help = 1;
  }

  if (help == 1) {
    setresult(shell, "%s: commands: %s", argv[0],
	      "new, delete, copy, insert, hasatt, getatt, setatt, delatt, listatt, "
              "define, dump, print, getname, setname, makeroot, lookup, relation");
  }

  return ret;
}

int QueryObjCmd::NewCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  QueryObj *node = QueryObj::Create();

  if (node) {
    if (argc >= 3) {
      node->SetName(argv[2]);
    }
    setresult(shell, "%d", getid(node));
    ret = TCL_OK;
  } else {
    error_failure(err);
  }

  return ret;
}

int QueryObjCmd::DeleteCmd(Interpreter * /* shell */, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      node->Remove();
      delete node;
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err.printf("args: id");
  }

  return ret;
}

int QueryObjCmd::CopyCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      QueryObj *dup = (QueryObj *)node->CloneTree();
      if (dup && dup->Valid()) {
        setresult(shell, "%d", getid(dup));
        ret = TCL_OK;
      } else {
        error_failure(err);
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id";
  }

  return ret;
}

int QueryObjCmd::InsertCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int parent_id = OSapi_atoi(argv[2]);
    int child_id = OSapi_atoi(argv[3]);
    int position = -1;
    if (argc >= 5) {
      position = OSapi_atoi(argv[4]);
    }
    QueryObj *parent = getnode(parent_id);
    if (parent) {
      QueryObj *child = getnode(child_id);
      if (child) {
        QueryObj *node = (QueryObj *)parent->InsertChild(child, position);
        if (node) {
          setresult(shell, "%d", getid(node));
          ret = TCL_OK;
	} else {
          error_failure(err);
	}
      } else {
        error_cantfind(err, argv[3]);
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: parent_id child_id [ position ]";
  }

  return ret;
}

int QueryObjCmd::HasAttCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      setresult(shell, "%d", node->HasAttribute(argv[3]));
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id key";
  }

  return ret;
}

int QueryObjCmd::GetAttCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      char const *key = argv[3];
      genString val;
      if (node->GetAttribute(key, val)) {
        setresult(shell, "%s", (char *)val);
        ret = TCL_OK;
      } else {
        err.printf("no such attribute: \"%s\".", key);
      }
    } else {
      error_cantfind(err, argv[2]);
    }    
  } else {
    err = "args: id key";
  }

  return ret;
}

int QueryObjCmd::SetAttCmd(Interpreter * /* shell */, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 5) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      char const *key = argv[3];
      char const *val = argv[4];
      node->SetAttribute(key, val);
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id key val";
  }

  return ret;
}

int QueryObjCmd::DelAttCmd(Interpreter * /* shell */, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      char const *key = argv[3];
      node->RemoveAttribute(key);
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id key";
  }

  return ret;
}

int QueryObjCmd::ListAttCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      TclList list;
      Object *obj;
      Properties &attributes = node->GetAttributes();
      ForEach (obj, attributes) {
        Property *prop = (Property *)obj;
        char *name = (char *)prop->get_name();
        list += name;
      }
      setresult(shell, "%s", (char *)list);
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id";
  }

  return ret;
}

int QueryObjCmd::DefineCmd(Interpreter * /* shell */, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      TclList def(argv[3]);
      if (node->Define(def)) {
        ret = TCL_OK;
      } else {
        error_failure(err);
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id def";
  }

  return ret;
}

int QueryObjCmd::DumpCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      TclList list;
      if (node->Dump(list)) {
        setresult(shell, "%s", (char *)list);
        ret = TCL_OK;
      } else {
        error_failure(err);
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id";
  }

  return ret;
}

int QueryObjCmd::PrintCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      int ok = 1;
      int depth = -1;
      char const **keys = NULL;
      if (argc >= 4) {
        int depth = OSapi_atoi(argv[3]);
        if (argc >= 5) {
          TclList list(argv[4]);
          int size = list.Size();
          keys = new char const *[size + 1];
          if (keys) {
            for (int i = 0; i < size; i++) {
              keys[i] = list[i];
            }
            keys[size] = NULL;
	  } else {
            error_failure(err);
            ok = 0;
	  }
        }
      }
      if (ok) {
        genString str;
        if (node->Print(str, keys, 0, depth)) {
          setresult(shell, "%s", (char *)str);
          ret = TCL_OK;
	} else {
          error_failure(err);
	}
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id [ depth [ keys ] ]";
  }

  return ret;
}

int QueryObjCmd::GetNameCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int ok = 1;
    TclList list;
    for (int i = 2; i < argc; i++) {
      int id = OSapi_atoi(argv[i]);
      QueryObj *node = getnode(id);
      if (node) {
        genString s;
        node->GetName(s);
        char *name = (char *)s;
        list += name ? name : (char *)"";
      } else {
        error_cantfind(err, argv[i]);
        ok = 0;
        break;
      }
    }
    if (ok) {
      setresult(shell, "%s", (char *)list);
      ret = TCL_OK;
    }
  } else {
    err = "args: id";
  }

  return ret;
}

int QueryObjCmd::SetNameCmd(Interpreter * /* shell */, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      node->SetName(argv[3]);
      ret = TCL_OK;
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id name";
  }

  return ret;
}

int QueryObjCmd::MakeRootCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      QueryObj *volume = node->AttachRoot();
      if (volume) {
        setresult(shell, "%d", getid(volume));
        ret = TCL_OK;
      } else {
        error_failure(err);
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  } else {
    err = "args: id";
  }

  return ret;
}

int QueryObjCmd::LookupCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int ok = 1;
    QueryObj *node = NULL;
    TclList list;
    if (argc >= 4) {
      int id = OSapi_atoi(argv[2]);
      node = getnode(id);
      if (node) {
        list = argv[3];
      } else {
        error_cantfind(err, argv[2]);
        ok = 0;
      }
    } else {
      list = argv[2];
    }
    if (ok) {
      char const **path = NULL;
      int size = list.Size();
      path = new char const *[size + 1];
      if (path) {
        for (int i = 0; i < size; i++) {
          path[i] = list[i];
	}
        path[size] = NULL;
        QueryObj *match = node ? node->Lookup(path) : QueryObj::RootLookup(path);
        if (match) {
          setresult(shell, "%d", getid(match));
	} else {
          setresult(shell, "%s", "");
        }
        ret = TCL_OK;
      } else {
        error_failure(err);
        ok = 0;
      }
    }
  } else {
    err = "args: [ id ] path";
  }


  return ret;
}

int QueryObjCmd::RelationCmd(Interpreter *shell, int argc, char const **argv, genString &err)
{
  int ret = TCL_ERROR;

  int help = 0;

  if (argc >= 4) {
    int id = OSapi_atoi(argv[2]);
    QueryObj *node = getnode(id);
    if (node) {
      char const *rel = argv[3];
      if (streq(rel, "parent")) {
        QueryObj *parent = (QueryObj *)node->GetParent();
        if (parent) {
          setresult(shell, "%d", getid(parent));
        }
        ret = TCL_OK;
      } else if (streq(rel, "children")) {
        TclList list;
        genString s;
        GenericTreePtrArr kids;
        int n = node->GetChildren(kids);
        for (int i = 0; i < n; i++) {
          s.printf("%d", getid((QueryObj *)(*(kids[i]))));
          list += (char *)s;
	}
        setresult(shell, "%s", (char *)list);
        ret = TCL_OK;
      } else if (streq(rel, "root")) {
        QueryObj *root = (QueryObj *)node->GetRoot();
        if (root) {
          setresult(shell, "%d", getid(root));
	}
        ret = TCL_OK;
      } else {
        help = 1;
      }
    } else {
      error_cantfind(err, argv[2]);
    }
  }

  if ((argc < 4) || (help == 1)) {
    err = "args: id [ parent | children | root ]";    
  }

  return ret;
}
