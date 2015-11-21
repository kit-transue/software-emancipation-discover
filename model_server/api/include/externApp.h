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
#ifndef _externApp_h
#define _externApp_h
#include <objArr.h>
#include <Relational.h>
#include <app.h>
#include <genArrCharPtr.h>
#include <msg.h>

#ifdef _WIN32
#define PATH_SEPARATOR ';'
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_STR "\\"
#define UNIX_DIR_SEPARATOR '/'
#else
#define PATH_SEPARATOR ':'
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_STR "/"
#endif

class projModule;
class objNameSet;
RelClass(externApp);
class externFile;

typedef externApp* (*externCreator)(const char* name, externFile * extf);


class externInfo {
  genString suffix;// attr, filt, list, ...
  genString type;  //attribute, filter, list ...
  genString subdir; // if NULL then type, e,g filter 
  static class externInfoPtrArr* type_list;
  objArr all;
  objNameSet* apps;

  externCreator create;

  /* The following variable holds all the directories specified in the 
     groupsPath preference. New groups are created in the first directory 
     while existing groups are loaded from all the directories in the path.
  */
  static genArrCharPtr *extern_dirs;

  void init();
public:
  objNameSet* get_apps(){ load(); return apps;}

  void app_delete(externApp*ap);
  static void load();
  externInfo(externCreator, const char*suf, const char*typ, const char*sub=0);
  void new_fname(const char *name, genString&fn) const;
  void fname(char const *name, char const *extdir, genString& fn) const;
  void get_extdir_from_physname(char const *nm, char const *fn, genString& extdir) const;

  externFile* find_extfile(char const *name);  
  externFile* new_extfile(const char*, char const *);
  bool delete_extfile(const char* name);

  static void write_error_message(msg_sev);
  static bool enabled();

  objArr& modules() {return all;}
  static const externInfo* find(const char* type);
  static externInfoPtrArr* list();
  static void add(externInfo*);
};
class objRemover;
class objInserter;

class externApp : public app {
 public:
   define_relational(externApp,app);
   virtual bool externSave() = 0;
   virtual externInfo* info() const = 0;
   void remove_obj(objRemover*, objRemover*);  // dummy
   void insert_obj(objInserter*, objInserter*); // dummy

   enum extStatus {SAVED, UNLOADED, MODIFIED, NEW, TEMP};
   virtual void set_status(extStatus st);
   virtual extStatus get_status() { return ext_stat;}
   void make_savable(externFile *extf = NULL);
 protected:
   extStatus ext_stat;
   externApp(const char *name, const char *fname, extStatus=TEMP);
};

enum groupType {GR_NONE, GR_OLD, GR_TMP, GR_NEW};

class extGroupApp : public externApp {
 public:
   define_relational(extGroupApp,externApp);
   static objNameSet& get_all();
   static extGroupApp* create_tmp(const char*name, symbolArr*arr);
   static extGroupApp* create(const char*name, externFile* ef = NULL);
   static extGroupApp* create(const char*name, symbolArr*arr);

   symbolArr* get_sym_arr();
   void set_sym_arr(symbolArr*);
   extGroupApp* rename(const char* new_name);
   virtual bool externSave();
   virtual externInfo* info() const;
   static  externInfo* type_info();
   symbolArr* load();
   static symbolArr* find(const char*name);
   static extGroupApp* get(const char*name);
   ~extGroupApp();
   static void get_all(symbolArr&);
   virtual ddKind get_symbolPtr_kind() const;
   virtual int is_symbolPtr_def () const;

 protected:
   extGroupApp(const char *name, const char *fname, extStatus = TEMP);
   static extGroupApp* current;
   symbolArr* arr;
};

class externFile : public Relational {
 public:
  char const *get_name() const;
  virtual int is_symbolPtr_def () const; 
  virtual ddKind get_symbolPtr_kind() const;
  ddKind get_kind () const;

  void delete_file();
  bool has_name(char const *nm) const;
  char const *get_phys_filename() const;

  externFile(char const *nm, char const *fn);

 private:
  genString name;
  genString phys_filename;
};

int read_group (const char* file_name, 
		symbolArr* arr, 
		char delim = '\0', 
		boolean skip_first_line = false, 
		boolean include_null_symbols = false);
int write_group (const char* file_name, 
		 symbolArr* arr);

bool make_dir(const char*physname);

#endif
