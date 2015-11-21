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
#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <Interpreter.h>
#include <externApp.h>
#include <externAttribute.h>
#include <proj.h>
#include <genString.h>
#include <scopeMgr.h> 
#include <genError.h>

#ifndef ISO_CPP_HEADERS
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <fstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <path.h>

#include <Iterator.h>
#include <shell_calls.h>
#include <driver_mode.h>

#ifdef _WIN32
#define STDERR_DEV_NULL  "nul"
#else
#define STDERR_DEV_NULL  "/dev/null"
#endif

/*
 * externApp related functions
 */

init_relational(externApp,app);   
void externApp::remove_obj(objRemover*, objRemover*) {}
void externApp::insert_obj(objInserter*, objInserter*){}

externApp::externApp(const char *name, const char *fname, extStatus st)
     : app((char*)name, (char*)fname, App_RAW, FILE_LANGUAGE_RAW ), ext_stat(st)
{
}

void externApp::make_savable(externFile *extf)
{
  bool ok = TRUE;
  if (!extf) {
    externInfo* ti = info();
    const char*name = get_name();
    const char*physname = get_phys_name();
    ok = (ti->new_extfile(name, physname)) ? TRUE: FALSE;
  }

  if (ok) {
    set_status(SAVED);
  }
}

void externApp::set_status(extStatus st) 
{ 
  if(ext_stat == TEMP) {
    if(st == SAVED)
      ext_stat = SAVED;
  } else {
    if(st==MODIFIED)
      set_modified();
    else if(st != NEW)
      clear_modified();
    
    ext_stat = st;
  }
  
}


/*
 * externInfo related functions
 */

genArrCharPtr * externInfo::extern_dirs = NULL;

typedef externInfo * externInfoPtr;
genArr(externInfoPtr);

externInfoPtrArr* externInfo::type_list;

externInfoPtrArr* externInfo::list()
{
  if(!type_list) type_list =  new externInfoPtrArr;
  return type_list;
}

const externInfo* externInfo::find(const char* app_type)
{
  int sz = type_list->size();
  for(int ii=0; ii<sz; ++ii){
   const externInfo* cur = *((*type_list)[ii]);
    if(cur->type == app_type)
      return cur;
  }
  return NULL;
}

externInfo::externInfo(externCreator cr, const char*suf, const char*typ, const char*sub)
  : create(cr), suffix(suf), type(typ), subdir(sub ? sub : typ) 
{
  apps = new objNameSet;
}

/* A new group is being created, use the first directory from the list
   of directories in extern_dirs to come up with a file name.*/  
void externInfo::new_fname(const char *name, genString&fn) const
{
  if (extern_dirs && extern_dirs->size() > 0) {
    fn.printf("%s%s%s%s%s.%s", *(*extern_dirs)[0], DIR_SEPARATOR_STR, subdir.str(), DIR_SEPARATOR_STR, name, suffix.str());
  }
  else fn = "";  
}

/* Use the given extdir to come up with a file name. */
void externInfo::fname(const char *name, char const *extdir, genString&fn) const
{
  if (name && extdir) {
    fn.printf("%s%s%s%s%s.%s", extdir, DIR_SEPARATOR_STR, subdir.str(), DIR_SEPARATOR_STR, name, suffix.str());
  }
  else fn = "";  
}

void externInfo::get_extdir_from_physname(char const *name, char const *phys_filename, genString& extdir) const
{
  if (name && phys_filename) {
    genString subdir_name;
    subdir_name.printf("%s%s%s%s.%s", DIR_SEPARATOR_STR, subdir.str(), DIR_SEPARATOR_STR, name, suffix.str()); 
    extdir = phys_filename;
    char *fn = (char *)extdir;
    char *pos = strstr(fn, subdir_name.str());
    if (!pos) {
      pos = fn;
    }
    *pos = '\0';
  }
  else extdir = "";

  return;
}

void externInfo::add(externInfoPtr ai)
{
  list()->append(&ai);
}

void externInfo::app_delete(externApp*ap)
{
  if (ap) {
    genString name = ap->get_name();

    if(apps) {
      apps->remove(*ap);
    }
    obj_unload(ap);

    externFile *extf = find_extfile(name.str());
    if (extf) {
      extf->delete_file();
      all.remove(extf);
    }
  } 
}


externFile* externInfo::find_extfile(char const *name)
{
  externFile *ret_ef = NULL;

  externFile *extf;
  for (int i = 0; i < all.size(); i++) {
    extf = (externFile *)all[i];
    if (extf && extf->has_name(name)) {
      ret_ef = extf;
      break;
    } 
  }
  return ret_ef;
}

externFile* externInfo::new_extfile(const char* name, char const* physname)
{
  externFile* extf = NULL;

  if (physname && *physname){
      bool dir_ok = make_dir(physname);
      if(dir_ok) {
          if (OSapi_access(physname, 0) != -1) {
              msg("File $1 already exists.", error_sev) << physname << eom;
          } 
          else if (errno != ENOENT) {
              msg("Error while creating file $1.", error_sev) << physname << eom;
          }
          else {
              if (!find_extfile(name)) {
                  extf = new externFile(name, physname);
                  all.insert_last(extf);
              }
              else {
                  msg("Extern file $1 already exists.", error_sev) << name << eom;
              }
          }
      }
  }
  else {
    msg("Invalid physical filename for extern file \"$1\".", error_sev) << name << eom;
  }

  return extf;
}

//-------------------------------------------------------------------------
// This method will remove the module with the name "name" from the disk and 
// from all Discover cache tables.
//-------------------------------------------------------------------------
bool externInfo::delete_extfile(const char* name) {
  bool ret_value = FALSE;
  externFile *extf = find_extfile(name);
  if (extf) {
    extf->delete_file();
    all.remove(extf);
    ret_value = TRUE;
  }
  else {
    msg("Failed to find extern file \"$1\".", error_sev) << name << eom;
  }
  return ret_value;
}

void externInfo::write_error_message(msg_sev severity)
{
    msg("Cannot save groups, please specify a valid directory in \"groupsPath\" preference.", severity) << eom;   
}

//
// Checks to see if the whole ext facility is enabled:
//

bool externInfo::enabled()
{
  bool is_enabled = (extern_dirs && extern_dirs->size() > 0) ? TRUE : FALSE;
  if (!is_enabled) {
      write_error_message(error_sev);
  }
  return is_enabled;
}

void externInfo::load()
{
  Initialize(externInfo::load);
  if(extern_dirs)
    return;

  extern_dirs = new genArrCharPtr;
  char const *ext_path = customize::getStrPref("groupsPath");
  if (ext_path && OSapi_strcmp(ext_path, "") != 0) {
    char const *st_pos;
    char const *next_pos;
    bool is_first_dir = TRUE;
    for (st_pos = ext_path; st_pos && *st_pos != '\0'; st_pos = next_pos) { 
      char const *end_pos = OSapi_strchr(st_pos, PATH_SEPARATOR);
      if (!end_pos) {
        end_pos = st_pos + OSapi_strlen(st_pos);
        next_pos = end_pos; 
      }
      else {
	next_pos = end_pos + 1;
      }
      if (st_pos == end_pos) continue;

      int length = end_pos - st_pos;
      char *dir = new char[length+1];
      OSapi_strncpy(dir, st_pos, length);
      dir[length] = '\0';

      bool dir_ok = FALSE;
      if (OSapi_access(dir, 0) == -1) {
         msg("Groups directory \"$1\" specified in \"groupsPath\" preference does not exist.", warning_sev) << dir << eom;
         /* If this is the first directory make an attempt to create it. */
         if (is_first_dir) {
            if (OSapi_mkdir(dir, 0777) == 0) {
                dir_ok = TRUE;
                msg("Created directory $1.", normal_sev) << dir << eom;
            }
            else {
                msg("Failed to create directory $1.", error_sev) << dir << eom;
            }
         }
      }
      else dir_ok = TRUE;

      if (dir_ok) {
          extern_dirs->append(&dir);
      }
      else delete dir;

      is_first_dir = FALSE;
    }

    if (extern_dirs->size() <= 0) {
      msg("No valid directory in \"groupsPath\" preference value: $1", error_sev) << ext_path << eom;
    }
  }
  else {
      msg("Empty value for preference \'groupsPath\'.", warning_sev) << eom;
  }

  if (extern_dirs->size() <= 0) {
      write_error_message(warning_sev);
  }

  int no_types = type_list->size();
  for(int typei = 0; typei<no_types; ++typei){
    externInfo* cur = *((*type_list)[typei]);
    cur ->init();
  }
}

void externInfo::init()
{
  Initialize(externInfo::init);
  genString dir;
  for (int i=0; i < extern_dirs->size(); i++) {
    genString dir = *(*extern_dirs)[i];
    dir += DIR_SEPARATOR_STR; 
    dir += subdir;

    if (OSapi_access(dir, 0) == 0) {
      genString cmd;
#ifndef _WIN32
      cmd.printf("cd %s; ls *.%s 2>%s", dir.str(),  suffix.str(), STDERR_DEV_NULL);
#else
      char *p = (char *)dir.str();
      while (p && *p) {
        //replace slash with back slash
        if (*p == UNIX_DIR_SEPARATOR)  *p = DIR_SEPARATOR;
        p++;
      }
      cmd.printf("cd /D %s && dir /B *.%s 2>%s", dir.str(), suffix.str(),
STDERR_DEV_NULL);
#endif 
      FILE* pipe = OSapi_popen((char *)cmd.str(), "r");
      if (pipe) {
        char name[256];
        int len;
        while (OSapi_fgets(name, 255, pipe) != NULL) {
          len = OSapi_strlen(name);
	  if (name[len-1] == '\n') {
	    name[len-1] = '\0';
          }
          if (strcmp(name, "") != 0) {
            int j;
            for (j = OSapi_strlen(name); (j >= 0) && (name[j] != '.'); j--) ;
            name[j] = '\0';

            genString full_name;
            full_name.printf("%s%s%s.%s", dir.str(), DIR_SEPARATOR_STR, name, suffix.str());

            externFile *extf = new externFile(name, full_name);
            all.insert_last(extf);
            if (create) {
	      externApp* app = (*create)(name, extf);
            }
          }
        }
        OSapi_pclose(pipe);
      }
    }
  }
}


/*  externFile related funcitons
 *
 */

externFile::externFile(char const *nm, char const *fn) : name(nm), phys_filename(fn)
{
}

char const* externFile::get_phys_filename() const
{
  return phys_filename.str(); 
}

bool externFile::has_name(char const *nm) const 
{ 
  return (nm && OSapi_strcmp(name.str(), nm) == 0) ? TRUE : FALSE;
}

void externFile::delete_file()
{
  OSapi_unlink(get_phys_filename());
}

ddKind externFile::get_kind () const 
{ 
  return DD_EXTFILE; 
}

ddKind externFile::get_symbolPtr_kind() const
{
  return DD_EXTFILE;
}

int externFile::is_symbolPtr_def () const 
{ 
  return 1; 
}

char const *externFile::get_name() const
{
  return get_phys_filename();
}


#define subcommand(nm) if(OSapi_strcmp(cmd, quote(nm)) == 0) \
  return paste3(extfile_,nm,Cmd)(cd, interp, argc-1, argv+1)
#define if_equal(nm) if(OSapi_strcmp(cmd,quote(nm))==0)

int extfileCmd(ClientData cd, Tcl_Interp *interp, int argc, char const *argv[])
{
  externInfo::load();
  Interpreter *i     = (Interpreter *)cd;

  if(argc<2) {
    Tcl_AppendResult(i->interp, "subcommands: list, new, delete, enabled", NULL);
    return TCL_OK;
  }

  const char* cmd = argv[1];
  externInfo* info = NULL;
  if(argc > 2) {
    const char* tp = argv[2];
    info           = (externInfo*) externInfo::find(tp);
    if(!info){
       Tcl_AppendResult(i->interp, "wrong type", NULL);
       return TCL_ERROR;
    }
  }
    
  DI_object dest;
  if_equal(list) {
      if(argc == 2) {
	     Tcl_AppendResult(i->interp, "usage: ", argv[0], " list <application name>", NULL);
	     return TCL_ERROR;
      }
      DI_object_create(&dest);
      symbolArr& roots = *get_DI_symarr(dest);
      i->SetDIResult(dest);
      roots = info->modules();
  } else {
	  if_equal(new) {
         if(argc < 4) {
	         Tcl_AppendResult(i->interp, "usage: ", argv[0], " new <application name> <file name>", NULL);
	         return TCL_ERROR;
		 }
         const char * name = argv[3];
         genString physname;
         info->new_fname(name, physname);
         externFile *extf = info->new_extfile(name, physname);
         if(!extf) return TCL_ERROR;
	     DI_object_create(&dest);
	     symbolArr& roots = *get_DI_symarr(dest);
	     i->SetDIResult(dest);
	     roots.insert_last(extf);
      }  else {
	      if_equal(delete) {
              if(argc < 4) {
	              Tcl_AppendResult(i->interp, "usage: ", argv[0], " delete <application name> <file name>", NULL);
	              return TCL_ERROR;
			  }
              const char * name = argv[3];
              if(info->delete_extfile(name)==FALSE) {
	              Tcl_AppendResult(i->interp, "No file with this name.", NULL);
	              return TCL_ERROR;
			  }
	          Tcl_AppendResult(i->interp, "File deleted.", NULL);
                  } else if_equal(enabled) {
                    if (argc >= 3) {
                      bool enabled = externInfo::enabled();
                      Tcl_AppendResult(i->interp, enabled ? "1" : "0", NULL);
                    } else {
                      Tcl_AppendResult(i->interp, "usage: ", argv[0], " enabled <application name>", NULL);
                    }
                  }  else {
              Tcl_AppendResult(i->interp, "wrong subcommand", NULL);
              return TCL_ERROR;
		  }
	  }
  }
  return TCL_OK;
}  


/*
 * Utility functions
 */

static void getFileNameAndDir(
    const char* pathName, genString* dirName, genString* fileName)
{
    Initialize(getFileNameAndDir);

    //  Test if we have a complete path name or just a file name.
    const char* lastForwardSlash = strrchr(pathName, DIR_SEPARATOR);
    if(lastForwardSlash == NULL) {
	*fileName = pathName;
	*dirName = "";
    } else {
	*fileName = lastForwardSlash + 1;
	dirName->put_value(
	    pathName, lastForwardSlash - (const char*)pathName + 1);
    }
}


bool make_dir(const char*physname)
{
  if(!(physname &&physname[0]))
     return false;
  genString theDirectory;
  genString theFileName;
  getFileNameAndDir(physname, &theDirectory, &theFileName);

  bool retval = true;

  // Valid UNIX directory (either External or Project file)?
  if (OSapi_access(theDirectory, 0) == -1) {
    if(errno == ENOENT)
      shell_mkdir (theDirectory);
  }

  if (OSapi_access(theDirectory, 0) == -1) {
    msg("Failed to create directory: $1", error_sev) << physname << eom;
    retval = false;
  } 
  return retval;
}

static int init() {
   externInfo::add(extAttrApp::type_info());
   externInfo::add(new externInfo(NULL, "flt", "filter", "ext"));
   externInfo::add(new externInfo(NULL, "ctx", "context", "ext"));
   externInfo::add(new externInfo(NULL, "ptn", "pattern", "ext"));
   externInfo::add(new externInfo(NULL, "sev", "sev", "ext"));
   new cliCommandInfo("extfile", extfileCmd);

   return 0;
}

static int dummy = init();

