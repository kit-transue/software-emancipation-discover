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
#include <QueryTree.h>

#include <sstream>

#include <TclList.h>
#include <expat.h>
#include <genArrInt.h>
#include <machdep.h>
#include <cLibraryFunctions.h>
#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <msg.h>

#ifdef _WIN32  /* make the NT linker to behave */
int querytree_init_flag = 0;
#endif

QTree::QTree() : GenericTree(),
  treecode(GENERIC)
{
}

QTree::QTree(const QTree &that)
{
  *this = that;
}

QTree::~QTree()
{
}

void QTree::operator=(const QTree &that)
{
  GenericTree::operator=(that);

  treecode = that.treecode;
}

GenericTree *QTree::CloneNode() const
{
  return new QTree(*this);
}

//
// RootNode:
//

RootNode::RootNode() : QTree()
{
  treecode = ROOT;
  appcode = QueryApp::INVALID;
}

RootNode::RootNode(const RootNode &that) { *this = that; }
RootNode::~RootNode() {}

void RootNode::operator=(const RootNode &that)
{
  QTree::operator=(that);

  appcode = that.appcode;
  domain = that.domain;
}

GenericTree *RootNode::CloneNode() const { return new RootNode(*this); }

//
// HeaderNode:
//

HeaderNode::HeaderNode() : QTree(), enabled(1), visible(1) {}
HeaderNode::HeaderNode(const HeaderNode &that) { *this = that; }
HeaderNode::~HeaderNode() {}

void HeaderNode::operator=(const HeaderNode &that)
{
  QTree::operator=(that);

  enabled = that.enabled;
  visible = that.visible;
  name = that.name;
  info = that.info;
  tag = that.tag;
}

//
// FolderNode:
//

FolderNode::FolderNode() : HeaderNode()
{
  treecode = FOLDER;
  name = "Untitled Folder";
}

FolderNode::FolderNode(const FolderNode &that) { *this = that; }
FolderNode::~FolderNode() {}

void FolderNode::operator=(const FolderNode &that) { HeaderNode::operator=(that); }
GenericTree *FolderNode::CloneNode() const { return new FolderNode(*this); }

//
// QueryNode:
//

QueryNode::QueryNode() : HeaderNode()
{
  treecode = QUERY;
  name = "Untitled Query";
  command = "{}";
  exec = 0;
}

QueryNode::QueryNode(const QueryNode &that) { *this = that; }
QueryNode::~QueryNode() {}

void QueryNode::operator=(const QueryNode &that)
{ 
  HeaderNode::operator=(that);

  command = that.command;
  exec = that.exec;
}

GenericTree *QueryNode::CloneNode() const { return new QueryNode(*this); }

objNameSet QTree::exchangeSet;

DataField *QTree::RegisterField(char const *key, DataField::DataFieldFunc fn,
                                char const *typeName, char const *title, int mask)
{
  DataField *fld = NULL;
  DataExchange *dex = GetExchange();
  if (dex) {
    WfValueType *vt = WfValueType::lookup(typeName);
    fld = new DataField(key, fn, vt, title, mask);
    if (fld)
      dex->Add(fld);
  }
  return fld;
}

void QTree::InstallFields()
{
}

void RootNode::InstallFields()
{
  RegisterField("domain", DEX_Domain, "set", "Domain", DEX_INVISIBLE);
}

void HeaderNode::InstallFields()
{
  RegisterField("name",    DEX_Name,    "string", "Name"                      );
  RegisterField("info",    DEX_Info,    "string", "Description", DEX_INVISIBLE);
  RegisterField("enabled", DEX_Enabled, "bool",   "Enabled",     DEX_INVISIBLE);
  RegisterField("visible", DEX_Visible, "bool",   "Visible",     DEX_INVISIBLE);
  RegisterField("tag",     DEX_Tag,     "string", "Tag",         DEX_INVISIBLE);
}

void QueryNode::InstallFields()
{
  HeaderNode::InstallFields();

  RegisterField("cmd",    DEX_Command, "string", "Command Line"                       );
  RegisterField("exec",   DEX_Exec,    "bool",   "Executed",             DEX_INVISIBLE);
}

// Question: Why not use pointer-to-members (i.e.: member_T class_T::*)?
//
// Answer: It's incredibly hard to do w/out templates (see ~abover/
//         save/nov24/DataExchange.h). Plus, making it flexible would
//         require alot of work. Below, we trade elegance (madness,
//         really) for a bowl of soup.
//
// The following functions serve one purpose -- to bind a specific
// exported member to an instance of a class:

void HeaderNode  ::DEX_Enabled (int m, genString &s, void *p) { DEX_Bool   (m, s, ( (HeaderNode *)  p)->enabled ); }
void HeaderNode  ::DEX_Visible (int m, genString &s, void *p) { DEX_Bool   (m, s, ( (HeaderNode *)  p)->visible ); }
void HeaderNode  ::DEX_Tag     (int m, genString &s, void *p) { DEX_String (m, s, ( (HeaderNode *)  p)->tag     ); }
void HeaderNode  ::DEX_Name    (int m, genString &s, void *p) { DEX_String (m, s, ( (HeaderNode *)  p)->name    ); }
void HeaderNode  ::DEX_Info    (int m, genString &s, void *p) { DEX_String (m, s, ( (HeaderNode *)  p)->info    ); }
void QueryNode   ::DEX_Command (int m, genString &s, void *p) { DEX_String (m, s, ( (QueryNode  *)  p)->command ); }
void QueryNode   ::DEX_Exec    (int m, genString &s, void *p) { DEX_Bool   (m, s, ( (QueryNode  *)  p)->exec    ); }

// Here's an example of where we need flexibility:

void RootNode::DEX_Domain(int m, genString &s, void *p)
{
  RootNode *root = (RootNode *)p;

  if (m == DEX_SET || m == DEX_WRITE) {
    symbolArr syms;
    DEX_Symbols(m, s, syms);
    root->SetDomain(syms);
  } else {
    DEX_Symbols(m, s, root->domain);
  }
}

void RootNode::SetDomain(symbolArr &syms)
{
  domain = syms;
}

int RootNode::Run(Interpreter *shell, QTree *tree)
{
  QTreePtrArr trees;
  trees.append(&tree);
  return RunNodes(shell, trees);
}

int RootNode::RunNodes(Interpreter *, QTreePtrArr &trees)
{
  Clean();
  for (int i = 0; i < trees.size(); i++) {
    QTree *tree = *(trees[i]);
    tree->DeepClean();
  }
  return TCL_OK;
}

void QTree::Clean()
{
}

void QTree::DeepClean()
{
  Clean();
  QTree *cur = (QTree *)firstChild;
  while (cur != NULL) {
    cur->DeepClean();
    cur = (QTree *)cur->nextSibling;
  }
}

void QueryNode::Clean()
{
  exec = 0;
}

int QTree::Build()
{
  return 1;
}

QTree *QTree::Create(TreeCode treecode, QueryApp::AppCode appcode)
{
  QTree *tree = NULL;

  tree = QueryApp::CreateNode(treecode, appcode);

  return tree;
}

QTree *QTree::Find(int mask, int depth) const
{
  QTree *ret = NULL;

  // Specify -1 to search the whole subtree; may sizeof(int) be with you:

  if (depth != 0) {
    QTree *cur = (QTree *)firstChild;
    while (cur != NULL)
    {
      if (cur->treecode & mask) {
        ret = cur;
        break;
      } else {
        ret = cur->Find(mask, depth - 1);
      }
      cur = (QTree *)cur->nextSibling;
    }
  }

  return ret;
}

int QTree::Find(int mask, QTreePtrArr &items, int depth) const
{
  int n = 0;

  if (depth != 0) {
    QTree *cur = (QTree *)firstChild;
    while (cur != NULL)
    {
      if (cur->treecode & mask) {
        items.append(&cur);
        ++n;
      }
      n += cur->Find(mask, items, depth - 1);
      cur = (QTree *)cur->nextSibling;
    }
  }

  return n;
}

//
// Convenience routine to sniff out all of the queries that
// are enabled, and live in active portions of the tree:
//

int QTree::FindActiveQueries(QTreePtrArr &items, int depth) const
{
  int n = 0;

  if (depth != 0) {
    if (treecode == QUERY) {
      if (((QueryNode *)this)->enabled) {
        QTree *t = (QTree *)this;
        items.append(&t);
        ++n;
      }
    }
    bool descend = true;
    if (treecode == FOLDER) {
      if (!((FolderNode *)this)->enabled)
        descend = false;
    }
    if (descend) {
      QTree *cur = (QTree *)firstChild;
      while (cur != NULL) {
        n += cur->FindActiveQueries(items, depth - 1);
        cur = (QTree *)cur->nextSibling;
      }
    }
  }

  return n;
}

QTree *QTree::PerformInsert(QTree *child, int index)
{
  QTree *ret = NULL;

  QTree *newChild = (QTree *)(child->CloneTree());  // always copy
  if (newChild) {
    bool ok = true;
    if (newChild->treecode == QTree::ROOT) {
      /* If we are trying to insert a ROOT node into an
       * existing ROOT or FOLDER, what we really want is
       * a new FOLDER whose contents are the same as the
       * specified ROOT's.
       */
      FolderNode *folder = (FolderNode *)Create(FOLDER, ((RootNode *)child)->appcode);
      if (folder) {
        QModule *module = tree_get_module((QTree *)child);
        char const *name = module ? module->get_name() : NULL;
        if (name)
          folder->name = name;
        QTree *cur = (QTree *)newChild->firstChild;
        while (cur != NULL) {
          QTree *grandChild = (QTree *)cur->CloneTree();
          if (grandChild)
            folder->InsertChild(grandChild);
          cur = (QTree *)cur->nextSibling;
        }
        newChild->Remove();
        delete newChild;
        newChild = folder;
      } else
        ok = false;
    }
    if (ok) {
      ret = (QTree *)InsertChild(newChild, index);
    }
  }

  return ret;
}

DataField *QTree::FindField(char const *key)
{
  DataField *field = NULL;

  DataExchange *dex = GetExchange();
  if (dex) {
    field = dex->Find(key);
  }

  return field;
}

int QTree::GetField(char const *key, genString &s)
{
  int ret = 0;

  DataExchange *dex = GetExchange();
  if (dex) {
    ret = dex->Get(this, key, s);
  }

  return ret;
}

int QTree::SetField(char const *key, char const *value, int persistent)
{
  int ret = 0;

  genString s = value;

  DataExchange *dex = GetExchange();
  if (dex) {
    ret = persistent ? dex->Write(this, key, s) : dex->Set(this, key, s);
  }

  return ret;
}

void QTree::GetAllFields(objArr &arr)
{
  DataExchange *dex = GetExchange();
  if (dex) {
    objArr *fields = dex->GetAllFields();
    if (fields) {
      arr = *fields;
    }
  }
}

char const *QTree::tags[] = {
  "generic",
  "root",
  "folder", 
  "query",
};

const int QTree::allows[] = {
  0xffff,
  0xffff,
  FOLDER | QUERY,
  0
};

char const *QTree::dislib[] = {
  "lang.dis",
  "port.dis",
  "glob.dis",
  "struct.dis",
  "stat.dis",
  "sevutil.dis",
  "misc.dis",
  "subchk.dis",
  "workflow_aux.dis",
  "closure.dis"
};

int QTree::SourceLib(Interpreter *shell)
{
  int ret = TCL_OK;

  int sz = sizeof(dislib) / sizeof(char const *);
  genString cmd = "source_dis ";
  for (int i = 0; i < sz; i++) {
    cmd += dislib[i];
    cmd += " ";
  }
  ret = shell->EvalCmd(cmd);

  return ret;
}

//
// Convenience routines to calculate index <-> bitmask:
//

inline QTree::TreeCode IndexToCode(int index)
{
  int code = 1;

  for (int j = 0; j < index; j++)
    code *= 2;

  return (QTree::TreeCode)code;
}

inline int CodeToIndex(QTree::TreeCode code)
{
  int x = 1;
  int index = 0;

  while (x < code) {
    x *= 2;
    ++index;
  }

  if (x != code)
    index = 0;

  return index;
}

//
// Convenience routines to convert tag <-> code:
//

inline QTree::TreeCode TagToCode(char const *tag)
{
  QTree::TreeCode ret = QTree::GENERIC;

  int size = sizeof(QTree::tags) / sizeof(char const *);
  for (int i = 0; i < size; i++) {
    if (OSapi_strcmp((char *)QTree::tags[i], (char *)tag) == 0) {
      ret = IndexToCode(i);
      break;
    }
  }

  return ret;
}

char const *CodeToTag(QTree::TreeCode code)
{
  char const *ret = QTree::tags[CodeToIndex(code)];
  return ret;
}

DataExchange *QTree::GetExchange()
{
  char const *tag = CodeToTag(treecode);
  DataExchange *dex = (DataExchange *)(exchangeSet.lookup(tag));

  if (!dex) {
    dex = new DataExchange(tag);
    exchangeSet.insert(dex);
    InstallFields();
  }

  return dex;
}

// Incredibly unsafe routines for associating tree objects with
// identification strings (uses memory addresses). This will have
// to do for now:

inline void TreeToId(const QTree *tree, genString &id)
{
  id.printf("0x%x", tree);
}

QTree *IdToTree(char const *id)
{
  QTree *tree = NULL;

  void *address = 0;
  if (OSapi_sscanf(id, "0x%x", &address) > 0)
    tree = (QTree *)address;

  return tree;
}

void QTree::TreeToId(QTree *tree, genString &id)
{
  ::TreeToId(tree, id);
}

QTree *QTree::IdToTree(char const *id)
{
  return ::IdToTree(id);
}

int QTree::Dump(ostream &os, int flags, int level)
{
  int ret = 0;

  genString margin = "";
  for (int i = 0; i < level; i++)
    margin += "  "; 

  genString tag = CodeToTag(treecode);

  os << (char *)margin << "<" << (char *)tag;
  if (flags & DUMP_VERBOSE) {
    genString id;
    TreeToId(this, id);
    os << " id=" << (char *)id;
  }
  os << ">" << endl;

  objArr fields;
  GetAllFields(fields);
  Obj *obj;
  ForEach (obj, fields) {
    DataField *field = (DataField *)obj;
    if ( ( flags & DUMP_VERBOSE ) || ( ! ( field->flags & DEX_DERIVED ) )) {
      char const *name = field->get_name();
      genString value;
      (flags & DUMP_PERSISTENT) ? field->Read(this, value) : field->Get(this, value);
      if (value.is_null())
        value = "";
      os << (char *)margin << "  <" << name << ">";
      os << (char *)value;
      os << "</" << name << ">" << endl;        
    }
  }

  QTree *cur = (QTree *)firstChild;
  while (cur != NULL) {
    cur->Dump(os, flags, level + 1);
    cur = (QTree *)cur->nextSibling;
  }

  os << (char *)margin << "</" << (char *)tag << ">" << endl;

  if (os.bad())
    ret = 1;

  return ret;
}

typedef struct {
  QTreePtrArr treeStack;
  intArr      modeStack;
  genString   fieldName;
  genString   fieldVal;
  int         append;
  XML_Parser  parser;
} ParserClientData;

static void ParserStartElement(void *, char const *, char const **);
static void ParserEndElement(void *, char const *);
static void ParserCharacterData(void *, char const *, int);

QTree *QTree::Load(istream &is)
{
  QTree *root = new RootNode;

  int zero = 0;
  ParserClientData cdata;
  cdata.treeStack.append(&root);
  cdata.modeStack.append(&zero);

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(parser, ParserStartElement, ParserEndElement);
  XML_SetCharacterDataHandler(parser, ParserCharacterData);
  XML_SetUserData(parser, &cdata);

  cdata.parser = parser;

  streambuf *sbuf = is.rdbuf();
  char buf[BUFSIZ];
  int done;

  do {
    int len = sbuf->sgetn(buf, sizeof(buf));
    done = len < sizeof(buf);
    if (!XML_Parse(parser, buf, len, done)) {
      int lineno = XML_GetCurrentLineNumber(parser);
      char const *errstr = XML_ErrorString(XML_GetErrorCode(parser));
      msg("Error occured at line $1 while parsing data stream: $2", error_sev)
	<< lineno << eoarg
	<< (errstr ? errstr : "(nil)") << eoarg << eom;
      done = -1;
    }
  } while (!done);

  XML_ParserFree(parser);

  int ok = 0;

  if (root->GetChildCount() == 1) {
    QTree *child = (QTree *)root->GetFirstChild();
    if (child && child->treecode == ROOT) {
      child->DetachTree();
      delete root;
      root = child;
      ok = 1;
    }
  }

  if (!ok) {
    delete root;
    root = NULL;
  }

  return root;
}

static void ParserStartElement(void *cdatap, char const *tag, char const ** /*atts*/)
{
  ParserClientData &state = *((ParserClientData *)cdatap);
  QTree &lastTree = *(*(state.treeStack[state.treeStack.size() - 1]));
  int &lastMode = *(state.modeStack[state.modeStack.size() - 1]);
  genString &fieldName = state.fieldName;
  int &append = state.append;

  int newMode = 0;

  QTree::TreeCode treecode = TagToCode(tag);

  if (lastMode > 0) {
    // error: element is child of terminal field:
    if (lastMode == 1) {
      int lineno = XML_GetCurrentLineNumber(state.parser);
      msg ("Unexpected entry found at line $1 of data stream", error_sev)
	<< lineno << eoarg << eom;
    }
    newMode = 2;
  } else if (treecode == QTree::GENERIC) {
    // don't create a new node, use the element as field data:
    fieldName = tag;
    append = 0;
    newMode = 1;
  } else {
    // create a new node:
    QTree *treePtr = QTree::Create(treecode, QueryApp::SUBCHK);  // add logic to detect app
    lastTree.InsertChild(treePtr);
    state.treeStack.append(&treePtr);
    newMode = 0;
  }

  state.modeStack.append(&newMode);
}

inline int allws(char const *cp)
{
  if (cp) {
    char c;
    while (c = *cp++) {
      if (!isspace(c))
        return 0;
    }
  }

  return 1;
}

static void ParserEndElement(void *cdatap, char const * /*tag*/)
{
  ParserClientData &state = *((ParserClientData *)cdatap);

  int mode = *(state.modeStack.pop());

  if (mode == 0) {
    state.treeStack.pop();
  } else if (mode == 1) {
    QTree &lastTree = *(*(state.treeStack[state.treeStack.size() - 1]));
    genString &fieldName = state.fieldName;
    genString &fieldVal = state.fieldVal;
    if (allws(fieldVal)) fieldVal = "";
    if ((fieldName.length() > 0) && (fieldVal.length() > 0)) {
      if (!lastTree.SetField(fieldName, fieldVal, 1)) {
        int lineno = XML_GetCurrentLineNumber(state.parser);
	msg ("Unexpected entry found at line $1 of data stream", error_sev)
		<< lineno << eoarg << eom;
      }
      fieldName = "";
      fieldVal = "";
    }
  } else {
    /* do nothing */
  }
}

static void ParserCharacterData(void *cdatap, char const *cp, int len)
{
  ParserClientData &state = *((ParserClientData *)cdatap);
  int &lastMode = *(state.modeStack[state.modeStack.size() - 1]);
  genString &fieldVal = state.fieldVal;
  int &append = state.append;

  if (lastMode == 1) {
    genString newval;
    if (append) {
      genString s;
      s.put_value(cp, len);
      fieldVal += s;
    } else {
      fieldVal.put_value(cp, len);
      append = 1;
    }
  }
}

int QTree::Write(char const *fname)
{
  int ret = 0;

  ofstream ofs(fname);
  if (!ofs)
    msg("Could not open file for writing: '$1'.", error_sev)
	<< fname << eoarg << eom;
  else
    ret = Dump(ofs, DUMP_PERSISTENT);

  return ret;
}

QTree *QTree::Read(char const *fname)
{
  QTree *ret = NULL;

  ifstream ifs(fname);
  if (!ifs)
    msg("Could not open file for reading: '$1'.", error_sev)
	<< fname << eoarg << eom;
  else
    ret = Load(ifs);

  return ret;
}

int QTree::Print(ostream &os, int mask, char const **fieldNames, int level)
{
  int ret = 0;

  TclList list;

  if (treecode & mask) {
    genString id;
    TreeToId(this, id);
    list += id;    
    list += CodeToTag(treecode);

    if (fieldNames) {
      char const *cp; int i = 0;
      while (cp = fieldNames[i++]) {
        genString value = "";
        DataField *field = FindField(cp);
        if (field && (!field->IsSetObj()))
          field->Get(this, value);
        list += value;
      }
    }

    os << level << " " << (char const *)list << endl;
    ++level;
  }

  QTree *cur = (QTree *)firstChild;
  while (cur != NULL) {
    cur->Print(os, mask, fieldNames, level);
    cur = (QTree *)cur->nextSibling;
  }

  if (os.bad())
    ret = 1;

  return ret;
}

int QTreeCmd::Register()
{
  new cliCommandInfo("querytree", QTreeCmd::Process);

  return 0;
}

inline int streq(char const *a, char const *b) { return (OSapi_strcmp((char *)a, (char *)b) == 0); }

int QTreeCmd::Process(ClientData cd, Tcl_Interp * /*interp*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  Interpreter *shell = (Interpreter *)cd;

  int help = 0;

  if (argc >= 2) {

    char const *subcmd = argv[1];

    //
    // This may be cleaned up to use a command map:
    //

    /***********************************************************************************/
         if (streq(subcmd, "new"))          ret = NewCmd             (shell, argc, argv);
    else if (streq(subcmd, "copy"))         ret = CopyCmd            (shell, argc, argv);
    else if (streq(subcmd, "del"))          ret = DeleteCmd          (shell, argc, argv);
    else if (streq(subcmd, "insert"))       ret = InsertCmd          (shell, argc, argv);
    else if (streq(subcmd, "type"))         ret = TypeCmd            (shell, argc, argv);
    else if (streq(subcmd, "find"))         ret = FindCmd            (shell, argc, argv);
    else if (streq(subcmd, "root"))         ret = GetRootCmd         (shell, argc, argv);
    else if (streq(subcmd, "inspect"))      ret = InspectCmd         (shell, argc, argv);
    else if (streq(subcmd, "dump"))         ret = DumpCmd            (shell, argc, argv);
    else if (streq(subcmd, "load"))         ret = LoadCmd            (shell, argc, argv);
    else if (streq(subcmd, "write"))        ret = WriteCmd           (shell, argc, argv);
    else if (streq(subcmd, "read"))         ret = ReadCmd            (shell, argc, argv);
    else if (streq(subcmd, "print"))        ret = PrintCmd           (shell, argc, argv);
    else if (streq(subcmd, "get"))          ret = GetFieldCmd        (shell, argc, argv);
    else if (streq(subcmd, "set"))          ret = SetFieldCmd        (shell, argc, argv);
    else if (streq(subcmd, "getall"))       ret = GetFieldsCmd       (shell, argc, argv);
    else if (streq(subcmd, "getfield"))     ret = GetFieldCmd        (shell, argc, argv);
    else if (streq(subcmd, "setfield"))     ret = SetFieldCmd        (shell, argc, argv);
    else if (streq(subcmd, "getfields"))    ret = GetFieldsCmd       (shell, argc, argv);
    else if (streq(subcmd, "getfldtype"))   ret = GetFieldTypeCmd    (shell, argc, argv);   
    else if (streq(subcmd, "bind"))         ret = BindCmd            (shell, argc, argv);
    else if (streq(subcmd, "create"))       ret = CreateCmd          (shell, argc, argv);
    else if (streq(subcmd, "open"))         ret = OpenCmd            (shell, argc, argv);
    else if (streq(subcmd, "close"))        ret = CloseCmd           (shell, argc, argv);
    else if (streq(subcmd, "save"))         ret = SaveCmd            (shell, argc, argv);
    else if (streq(subcmd, "saveas"))       ret = SaveAsCmd          (shell, argc, argv);
    else if (streq(subcmd, "remove"))       ret = RemoveCmd          (shell, argc, argv);
    else if (streq(subcmd, "list"))         ret = ListCmd            (shell, argc, argv);
    else if (streq(subcmd, "moduleof"))     ret = ModuleOfCmd        (shell, argc, argv);
    else if (streq(subcmd, "init"))         ret = InitCmd            (shell, argc, argv);
    else if (streq(subcmd, "source"))       ret = SourceCmd          (shell, argc, argv);
    else if (streq(subcmd, "run"))          ret = RunCmd             (shell, argc, argv);
    else if (streq(subcmd, "runners"))      ret = RunnersCmd         (shell, argc, argv);
    else if (streq(subcmd, "relation"))     ret = RelationCmd        (shell, argc, argv);
    /***********************************************************************************/
    else {
      help = 1;
    }
  } else {
    help = 1;
  }

  if (help == 1) {
    genString usage;
    usage.printf("%s: commands: %s", argv[0],
      "new, copy, del, insert, "
      "type, find, root, "
      "dump, load, write, read, print, "
      "get (getfield), set (setfield), getall (getfields), getfldtype, "
      "bind, create, open, close, save, saveas, remove, list, moduleof "
      "init, source, run, runners");

    Tcl_SetResult(shell->interp, (char *)usage, TCL_VOLATILE);
  }

  return ret;
}

int QTreeCmd::NewCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  QTree *tree = NULL;

  QTree::TreeCode treecode = QTree::ROOT;
  QueryApp::AppCode appcode = QueryApp::SUBCHK;

  if (argc >= 3) {
    treecode = TagToCode(argv[2]);
    if (argc >= 4) {
      QueryApp::AppCode c = QueryApp::NameToCode(argv[3]);
      if (c != QueryApp::INVALID) {
        appcode = c;
      }
    }
  }

  tree = QTree::Create(treecode, appcode);

  if (tree) {
    tree->Build();
    genString id;
    TreeToId(tree, id);
    Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
    ret = TCL_OK;
  }

  return ret;
}

int QTreeCmd::CopyCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QTree *copy = (QTree *)tree->CloneTree();
      genString id;
      TreeToId(copy, id);
      Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
      ret = TCL_OK;      
    }
  }

  return ret;
}

int QTreeCmd::DeleteCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      tree->Remove();
      delete tree;
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::InsertCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QTree *child = IdToTree(argv[3]);
      if (child) {
        if (argc >= 5) {
          int index = OSapi_atoi(argv[4]);
          child = tree->PerformInsert(child, index);
        } else {
          child = tree->PerformInsert(child);
        }
        genString id;
        TreeToId(child, id);
        Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

int QTreeCmd::TypeCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      genString tag = CodeToTag(tree->treecode);
      Tcl_SetResult(shell->interp, (char *)tag, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::FindCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QTree::TreeCode treecode = TagToCode(argv[3]);
      int mask = treecode;
      int depth = 0;
      if (argc >= 5) {
        depth = OSapi_atoi(argv[4]);
      }
      QTreePtrArr items;
      int n = tree->Find(mask, items, depth);
      if (tree->treecode & mask) {
        items.append(&tree);
        ++n;
      }
      TclList list;
      for (int i = 0; i < n; i++) {
        QTree *t = *items[i];
        genString id;
	TreeToId(t, id);
        list += id;
      }
      Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::GetRootCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QTree *root = (QTree *)tree->GetRoot();
      if (root) {
        genString id;
        TreeToId(root, id);
        Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

// Convenience conversion function:

inline void OssToGs(ostringstream &oss, genString &gs)
{
  genString s = oss.str().c_str();
  gs.put_value(s, s.length());
}

int QTreeCmd::InspectCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      ostringstream oss;
      tree->Dump(oss, QTree::DUMP_VERBOSE);
      genString gs;
      OssToGs(oss, gs);
      Tcl_SetResult(shell->interp, (char *)gs, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::DumpCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      ostringstream oss;
      tree->Dump(oss, 0);
      genString gs;
      OssToGs(oss, gs);
      Tcl_SetResult(shell->interp, (char *)gs, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::LoadCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    char const *text = argv[3]; 
    istringstream iss(text);
    QTree *tree = QTree::Load(iss);
    if (tree) {
      genString id;
      TreeToId(tree, id);
      Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::WriteCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      if (tree->Write(argv[3]) == 0)
        ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::ReadCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = QTree::Read(argv[2]);
    if (tree) {
      genString id;
      TreeToId(tree, id);
      Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::PrintCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      int mask = QTree::FOLDER | QTree::QUERY;
      static char const *defaultFields[] = { "enabled", "name", NULL };      
      char const **fields = NULL;

      TclList list;

      /* collect the node types to display */

      if (argc >= 4) {
        mask = 0;
        list = argv[3];
        int sz = list.Size();
        for (int i = 0; i < sz; i++) {
          char const *tag = list[i];
          if (OSapi_strcmp(tag, "all") == 0) {
            mask = 0xffff;
            break;
	  } else {
            mask |= TagToCode(tag);            
	  }
	}
      }

      /* collect the fields to display */

      if (argc >= 5) {
        list = argv[4];
        int sz = list.Size();
        fields = new char const *[sz + 1];
        if (fields) {
          for (int i = 0; i < sz; i++) {
            fields[i] = list[i];
	  }
          fields[sz] = NULL;
        }
      }

      /* do the printing */

      ostringstream oss;
      tree->Print(oss, mask, fields ? fields : defaultFields);
      delete[] fields;
      genString gs;
      OssToGs(oss, gs);
      Tcl_SetResult(shell->interp, (char *)gs, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::GetFieldCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      DataField *field = tree->FindField(argv[3]);
      if (field) {
        genString value;
        field->Get(tree, value);
        // If it's a set object, it's already in the interpreter's result:
        if (!field->IsSetObj()) {
          Tcl_SetResult(shell->interp, (char *)value, TCL_VOLATILE);
	}
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

int QTreeCmd::SetFieldCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 5) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      tree->SetField(argv[3], argv[4]);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::GetFieldsCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      int verbose = 0;
      if (argc >= 4) {
        verbose = OSapi_atoi(argv[3]);
      }
      objArr fields;
      tree->GetAllFields(fields);
      Obj *obj;
      TclList list;
      ForEach (obj, fields) {
        DataField *field = (DataField *)obj;
        if ( verbose || ( ! ( field->flags & DEX_INVISIBLE ) )) {
          TclList sublist;
          char const *name = field->get_name();
          char const *title = field->title;
          genString value = "";
          if (!field->IsSetObj()) field->Get(tree, value);
          genString type;
          field->GetTypeNameLong(type);
          sublist += name;
          sublist += title;
          sublist += type;
          sublist += value;
          list += sublist;
        }
      }
      Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::GetFieldTypeCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      objArr fields;
      tree->GetAllFields(fields);
      DataField *field = (DataField *)(obj_search_by_name(argv[3], fields));
      if (field) {
        genString type;
        field->GetTypeNameLong(type);
        TclList list((char const *)type);
        Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

int QTreeCmd::BindCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QModule *module = QModule::Find(argv[3]);
      if (module)
        module_put_tree(module, tree);
      else
        module = QModule::Create(argv[3], tree);
      if (module) {
        genString id;
        TreeToId(tree, id);
        TclList list;
        list += module->get_name();
        list += id;
        Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

int QTreeCmd::CreateCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  char const *name = NULL;
  QTree *tree = NULL;

  if (argc >= 3)
    name = argv[2];

  QueryApp::AppCode appcode = QueryApp::SUBCHK;

  if (argc >= 4) {
    QueryApp::AppCode c = QueryApp::NameToCode(argv[3]);
    if (c != QueryApp::INVALID) {
      appcode = c;
    }
  }

  tree = QTree::Create(QTree::ROOT, appcode);

  QModule *module = QModule::Create(name, tree);

  if (module) {
    genString id;
    TreeToId(tree, id);
    TclList list;
    list += module->get_name();
    list += id;
    Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
    ret = TCL_OK;
  }

  return ret;
}

int QTreeCmd::OpenCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QModule *module = QModule::Find(argv[2]);
    if (module) {
      QTree *tree = module->Open();
      if (tree) {
        genString id;
        TreeToId(tree, id);
        Tcl_SetResult(shell->interp, (char *)id, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}

int QTreeCmd::CloseCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QModule *module = QModule::Find(argv[2]);
    if (module) {
      module->Close();
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::SaveCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QModule *module = QModule::Find(argv[2]);
    if (module && module->Save()) {
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::SaveAsCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QModule *oldModule = QModule::Find(argv[2]);
    if (oldModule) {
      RootNode *oldRoot = (RootNode *)oldModule->Open();
      if (oldRoot) {
        RootNode *newRoot = (RootNode *)oldRoot->CloneTree();
        if (newRoot) {
          QModule *newModule = QModule::Create(argv[3], newRoot);
          if (newModule) {
            TclList list;
            genString id;
            TreeToId(newRoot, id);
            list += newModule->get_name();
            list += id;
            Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
            ret = TCL_OK;
          }
        }
      }
    }
  }

  return ret;
}

int QTreeCmd::RemoveCmd(Interpreter * /*shell*/, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QModule *module = QModule::Find(argv[2]);
    if (module) {
      module->Close();
      QModule::Remove(module->get_name());
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::ListCmd(Interpreter *shell, int /*argc*/, char const ** /*argv*/)
{
  int ret = TCL_ERROR;

  TclList list;

  if (QModule::List(list)) {
    Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
    ret = TCL_OK;
  }

  return ret;
}

int QTreeCmd::ModuleOfCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {  
      QModule *module = tree_get_module(tree);
      Tcl_SetResult(shell->interp, (char *)(module ? module->get_name() : ""), TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::InitCmd(Interpreter * /*shell*/, int /*argc*/, char const ** /*argv*/)
{
  QModule::LoadDefaults();

  return TCL_OK;
}

int QTreeCmd::SourceCmd(Interpreter *shell, int /*argc*/, char const ** /*argv*/)
{
  int ret = TCL_ERROR;

  ret = QTree::SourceLib(shell);

  return ret;
}

int QTreeCmd::RunCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    int ok = 1;
    QTreePtrArr trees;
    for (int i = 2; (i < argc) && ok; i++) {
      QTree *tree = IdToTree(argv[i]);
      if (tree) {
        trees.append(&tree);
      } else {
        ok = 0;
      }
    }
    if (ok && (trees.size() > 0)) {
      QTree *t = *(trees[0]);
      RootNode *root = (RootNode *)t->GetRoot();
      /* sanity check: */
      if (root->treecode == QTree::ROOT) {
        int code = root->RunNodes(shell, trees);
        if (code == TCL_OK) {
          genString msg;
          shell->GetResult(msg);
          Tcl_SetResult(shell->interp, (char *)msg, TCL_VOLATILE);
        } else {
          char const *errorInfo = Tcl_GetVar(shell->interp, "errorInfo", TCL_GLOBAL_ONLY);
          if (errorInfo) {
            Tcl_SetResult(shell->interp, (char *)errorInfo, TCL_VOLATILE);
          }
        }
        ret = code;
      }
    }
  }

  return ret;
}

int QTreeCmd::RunnersCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 3) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      QTreePtrArr queries;
      int n = tree->FindActiveQueries(queries);
      TclList list;
      for (int i = 0; i < n; i++) {
        QTree *t = *queries[i];
        genString id;
        TreeToId(t, id);
        list += id;
      }
      Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
      ret = TCL_OK;
    }
  }

  return ret;
}

int QTreeCmd::RelationCmd(Interpreter *shell, int argc, char const **argv)
{
  int ret = TCL_ERROR;

  if (argc >= 4) {
    QTree *tree = IdToTree(argv[2]);
    if (tree) {
      int ok = 1;
      TclList list;
      char const *rel = argv[3];
      if (streq(rel, "children")) {
        GenericTreePtrArr nodes;
        tree->GetChildren(nodes);
        genString id;
        for (int i = 0; i < nodes.size(); i++) {
          QTree *child = (QTree *)(*(nodes[i]));
          TreeToId(child, id);
          list += id;
	}
      } else if (streq(rel, "parent")) {
        QTree *parent = (QTree *)tree->GetParent();
        if (parent) {
          genString id;
          TreeToId(parent, id);
          list += id;
	}
      } else {
        ok = 0;
      }
      if (ok) {
        Tcl_SetResult(shell->interp, (char *)list, TCL_VOLATILE);
        ret = TCL_OK;
      }
    }
  }

  return ret;
}
