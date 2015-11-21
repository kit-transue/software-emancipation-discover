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
/*
    START-LOG-------------------------------

    $Log: projectNode.h  $
    Revision 1.1 1993/06/09 12:33:18EDT builder 
    made from unix file
 * Revision 1.2.1.7  1993/06/09  16:32:27  builder
 * Make moduleNode::is_locked a non-const member function, to
 * avoid syntax errors with new parser.
 *
 * Revision 1.2.1.6  1993/04/02  17:21:02  oak
 * /sysroot/project/include
 *
 * Revision 1.2.1.5  1993/03/30  00:48:55  sergey
 * Put test.
 *
 * Revision 1.2.1.4  1992/12/22  15:49:37  sharris
 * declare get_text_file, get_filename and get_pathname methods const do that they
 * can be called for constant objects
 *
 * Revision 1.2.1.3  1992/11/02  14:14:26  kws
 * Added inode caching
 *
 * Revision 1.2.1.2  92/10/09  18:22:57  builder
 * *** empty log message ***
 * 

    END-LOG---------------------------------
*/

#ifndef _projectNode_h
#define _projectNode_h

#ifndef _genString_h
#include <genString.h>
#endif

#ifndef _projectHeader_h
#include <projectHeader.h>
#endif

RelClass(projectNode);

class projectCommon : public appTree
{
  protected:
    genString name;			// Name of project/module

  public:
    static Relational* restore (void *rec, app *h, char *text = 0);

    define_relational (projectCommon, appTree);
    projectCommon ();
    ~projectCommon ();
    projectCommon (const projectCommon&) {}

    app *get_main_header();
};
generate_descriptor (projectCommon,appTree);

class projectNode : public projectCommon
{
  private:
    genString name;		        // Project name

    // module_of_project relation used to define list of modules
    // project_of_parent relation is used to define list of sub-projects

  public:
    define_relational (projectNode, projectCommon);
    projectNode();
    ~projectNode();
    projectNode (const projectNode&) {}
    copy_member (projectNode);

    int contains (appTreePtr);

    virtual void set_name(char*);
    virtual char* get_name() const;
    virtual void send_string(ostream& stream = cout) const;
    virtual int  get_extype() const;
    virtual void description(genString&);
    virtual objArr* get_tok_list(int mode = 0);
    virtual save (void *buffer, int maxlength);
    static Relational* restore (void *rec, app *h, char *text = 0);

    char *get_directory();
};
generate_descriptor (projectNode,projectCommonn);

class moduleNode : public projectCommon
{
  private:
    short type;				// Type of module
    short lang;				// Language
    unsigned int update : 1;		// Lock needs updating
    unsigned int locked : 1;		// Module is locked
    genString text_file;		// File name of text data
    genString src_dir;			// Source file directory
    int valid_inode;
    long inode;	
    //project_of_module relation used to define container project

  public:
    define_relational (moduleNode, projectCommon);
    moduleNode();
    moduleNode(const char*);
    ~moduleNode();
    moduleNode (const moduleNode&) {}
    copy_member (moduleNode);

    void module_type (int);
    int module_type ();

    void language (int);
    int language ();

    void set_update() {update=1;}
    int is_loaded() const { return projectHeader::is_loaded(name); }
    int is_locked()  { 
	if (update) {locked = projectHeader::is_locked(name); update = 0;}
	return locked;
    }
    int is_parsed() const { return projectHeader::is_parsed(name); }
    int is_modified() const { return projectHeader::is_modified(name); }
    int is_user_file() const { return projectHeader::is_user_file(name); }

    void set_text_file (const char *text_file);
    const char* get_text_file () const;

    const char* get_filename () const;
    const char* get_pathname () const;
    void invalidate_inode() {valid_inode = 0;}
    long get_inode();

    void set_src_dir (const char *src_directory);
    const char* get_directory ();
    const char* get_src_dir ();

    virtual void set_name(char*);
    virtual char* get_name() const;

    virtual void send_string(ostream& stream = cout) const;
    virtual int  get_extype() const;
    virtual void description(genString&);
    virtual objArr* get_tok_list(int mode = 0);
    virtual save (void *buffer, int maxlength);
    static Relational* restore (void *rec, app *h, char *text = 0);
};
generate_descriptor (moduleNode,projectCommon);

define_relation(parent_of_project, project_of_parent);
define_relation(project_of_module, module_of_project);
define_relation(module_of_app, app_of_module);

#endif
