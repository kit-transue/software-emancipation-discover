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
//
// Smt_cppparm.C - get compiler flags from Makefile
//
#include <smt_cppparm.h>

#ifndef ISO_CPP_HEADERS
#include <errno.h>
#include <strstream.h>
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
using namespace std;
#include <sstream>
#include <cstdlib>
#include <cassert>
#endif /* ISO_CPP_HEADERS */
#include <sys/stat.h>

#include <CompilerOptions.h>
#include <Interpreter.h>
#include <Question.h>
#include <TclList.h>
#include <cLibraryFunctions.h>
#include <customize.h>
#include <driver_mode.h>
#include <evalMetaChars.h>
#include <fileCache.h>
#include <genError.h>
#include <machdep.h>
#include <messages.h>
#include <msg.h>
#include <proj.h>
#include <scopeMgr.h>
#include <systemconstants.h>
#include <timer.h>
#include <vpopen.h>

extern char const *els_flags(char const *lan);
extern void        extern_els_init ();

genArr(genString);

class ParamValue : public objDictionary {
public:
    ParamValue(char const *name, char const *val) : objDictionary(name), value(val)
    {}
    genString value;
};

class ParamConvertTable {
public:
    static char const *ConvertParameter(char const *param);
    
private:
    static void LoadTable();

    static objNameSet parameters;
    static int        table_loaded;
};

objNameSet ParamConvertTable::parameters;
int        ParamConvertTable::table_loaded = 0;

genString smt_make_compile_line;
genString cc_execution_directory;

extern "C" int set_common_tentative_defs(int);
extern "C" int dos2unix(char const *, int);
extern char const *smt_get_name_macros_file();
static int  cpp_debug;
static int  assertion_flag;

#ifdef _WIN32
// Flag indicating that case should be ignored for compiler names
static int ignore_case = 1;
#endif

char const *pset_get_current_directory();
void pset_set_current_directory(char const *);

static genString c_flags;
static genString cp_flags;
static genString directory_of_file;

static objArr_Int saved_compiler_flags;

static char *copy(char const *src, int len = 0)
{
    if(len)
    {
	char *w = strncpy(new char[len + 1], src, len);
	w[len] = 0;
	return w;
    }
    else
	return strcpy(new char[strlen(src) + 1], src);
}

//------------------------------------------------------------------------------

char const *ParamConvertTable::ConvertParameter(char const *param)
{
    char const *ret = NULL;

    if(!table_loaded)
	LoadTable();
    ParamValue* pv = (ParamValue *)parameters.lookup(param);
    if(pv){
	ret = pv->value;
    } else
	ret = param;
    return ret;
}

//------------------------------------------------------------------------------

void ParamConvertTable::LoadTable(void)
{
    enum ReadStates { ORIGINAL_OPTION, SPACES_AFTER_ORIGINAL, NEW_OPTION, SPACES_AFTER_NEW };
    char buf[256];
    ReadStates state               = SPACES_AFTER_NEW;
    static char const *env_psethome = OSapi_getenv("PSETHOME");
    genString param_conv_file_name = env_psethome;
    param_conv_file_name          += "/lib/options.map";
    FILE *fl = fopen(param_conv_file_name, "r");
    if(fl == NULL){
	table_loaded = 1;
	msg("Unable to open $1 file.", normal_sev) << (char const *)param_conv_file_name << eom;
	return;
    }
    genString original_opt;
    genString new_opt;
    int ch;
    int pos = 0;
    while((ch = getc(fl)) != EOF){
	switch(state){
	case SPACES_AFTER_NEW:
	    if(!isspace(ch)){
		buf[pos++] = ch;
		state      = ORIGINAL_OPTION;
	    }
	    break;
	case ORIGINAL_OPTION:
	    if(!isspace(ch))
		buf[pos++] = ch;
	    else {
		buf[pos]     = 0;
		original_opt = buf;
		pos          = 0;
		state        = SPACES_AFTER_ORIGINAL;
		ungetc(ch, fl);
	    }
	    break;
	case SPACES_AFTER_ORIGINAL:
	    if(!isspace(ch)){
		buf[pos++] = ch;
		state      = NEW_OPTION;
	    } else if(ch == '\n') {
		new_opt  = "";
		pos      = 0;
		state    = SPACES_AFTER_NEW;
		ParamValue* pv = (ParamValue *)parameters.lookup(original_opt);
		if(!pv){
		    pv = new ParamValue(original_opt, new_opt);
		    parameters.insert(pv);
		} else {
		    ; // Report error here
		}
	    }
	    break;
	case NEW_OPTION:
	    if(!isspace(ch))
		buf[pos++] = ch;
	    else if(ch == '\n') {
		if ((pos > 0) && (buf[pos - 1] == '\r')) --pos;
		buf[pos] = 0;
		new_opt  = buf;
		pos      = 0;
		state    = SPACES_AFTER_NEW;
		ParamValue* pv = (ParamValue *)parameters.lookup(original_opt);
		if(!pv){
		    pv = new ParamValue(original_opt, new_opt);
		    parameters.insert(pv);
		} else {
		    ; // Report error here
		}
	    } else
		buf[pos++] = ch;
	    break;
	}
	if(pos == 255) {
	    // Parameter too long
	    pos = 0;
	}
    }
    if(state == NEW_OPTION){
	if ((pos > 0) && (buf[pos - 1] == '\r')) --pos;
	buf[pos] = 0;
	new_opt  = buf;
	pos      = 0;
	ParamValue* pv = (ParamValue *)parameters.lookup(original_opt);
	if(!pv){
	    pv = new ParamValue(original_opt, new_opt);
	    parameters.insert(pv);
	} else {
	    ; // Report error here
	} 
    }
    fclose(fl);
    table_loaded = 1;
}

//------------------------------------------------------------------------------

static objArr_Int saved_esql_flags;

static void discard_esql_flags() {
    for (int i = 0; i < saved_esql_flags.size(); i++) {
	delete [] (char *) saved_esql_flags[i];
    }
    saved_esql_flags.removeAll();
}

static void save_esql_flags(char const *line) {
    char const *p; 
    char const *ep;
    for (p = line; p && *p; p = ep) {
	while (isspace(*p)) {
	    if (*p == '\n') {
		return;
	    }
	    p++;
	}
	for (ep = p; *ep && !isspace(*ep); ep++)
		{}

#define is(s) strncmp(p, s, sizeof(s) - 1) == 0

	if (is("INCLUDE=") || is("SYSINCLUDE=") || is("DEFINE=") ||
	    is("CONFIG=") || is("DEF_SQLCODE=")) {
	    saved_esql_flags.insert_last(int(copy(p, ep - p)));
	}
    }
#undef is
}
  

extern "C" int smt_push_arg (void* array, char const *data)
{
    objArr_Int *argv = (objArr_Int*) array;
    argv->insert_last ((int)data);
    return (int)argv->size ();
}

static int is_cpp_flag (char const *flag, int len)
{
    int ret_val = 0;

#define check(x) \
    (len == sizeof (x) - 1 && !strncmp (flag, x, len))

    if (assertion_flag) {
	assertion_flag = 0;
	ret_val = 1;
    }
    
    if (ret_val == 0 && flag[0] == '-') {
	switch (flag[1]) {
	  case 'p':
	    if (check ("-pedantic")
		|| check ("-pedantic-errors"))
	    ret_val =  1;
	    break;

	  case 't':
	    if (check ("-traditional")
		|| check ("-trigraphs"))
		ret_val = 1;
	    else if (check ("-traditional-cpp"))
		ret_val = 2;
	    break;

	  case 'l':
	    if (check ("-lang-c")
		|| check ("-lang-c++"))
		ret_val = 1;
	    break;

	  case 'n':
	    if (check ("-nostdinc") || check ("-nostdinc++"))
		ret_val = 1;
	    break;
	    
	  case 'w':
	    if (check ("-w"))
		ret_val = 1;
	    break;

      case 'W':
	    if (check ("-Wtrigraphs")
		|| check ("-Wno-trigraphs")
		|| check ("-Wcomment")
		|| check ("-Wno-comment")
		|| check ("-Wno-comments")
		|| check ("-Wtraditional")
		|| check ("-Wno-traditional")
		|| check ("-Werror")
		|| check ("-Wall"))
		ret_val = 1;
	    break;

	  case 'D':
	  case 'U':
	  case 'I':
	    ret_val = 1;
	    break;
	    
	  case 'i':
	    if (check("-idirafter") || check("-igcc") || check("-include")) {
		assertion_flag = 1;
		ret_val = 1;
	    }
	    break;

	  case 'u':
	    if (check ("-undef"))
		ret_val = 1;
	    break;
	
	  case 'A':
	    if (check("-A"))
		assertion_flag = 1;
	    ret_val = 1;
	    break;
	 
	  case 'v' :
	    if (check("-vax-style-include"))
	      ret_val = 1;
	    break;

	  default:
	    break;
	}
    }    
    return ret_val;
#undef check
}

static char *smt_parse_flags (char *line)
{
    Initialize(smt_parse_flags);

    if (!line)
	return NULL;

    char *w = line, *w1;
    
    for (w1 = w; *w1 && *w1!= '\n' ; w1++)
	switch (*w1) {
	  case '|': case '<': case '>': case '&':
	    *w1 = ' ';
	    break;
		
	  default:
	    break;
	}

    *w1 = '\0';

    char *src = w;
    ostringstream os;
    
    int was_nl = 1;
    for (; *src && *src != '\n'; ++src) {
	switch (*src) {
	  case ' ' :
	  case '\t' :
	      if (!was_nl) {
		  was_nl = 1;
		  os << '\n';
	      }
	      continue;
	   
	    case '\\' :
		if (src[1])
		    os << *++src;
	        break;

	    case '\'' :
		src++;
	        for (; *src && *src != '\n' && *src != '\''; ++src) {
		    os << *src;
		}
	        if (*src == '\0' || *src == '\n')
		    src--;
	        break;

	    case '\"' :
		src++;
	        for (; *src && *src != '\n' && *src != '\"'; ++src) {
		    if (*src == '\\' &&
			// Within a quoted argument,
			// the characters backslash (\), double-quote ("), dollar ($), and
			// backquote (`) are escaped with a backslash in Bourne shell.
			// On Windows, only with the double-quote is backslash an escape.
			(src[1] == '"'
#ifndef _WIN32
			    || src[1] == '\\' || src[1] == '$' || src[1] == '`'
#endif
					  )) {
			os << *++src;
		    } else {
			os << *src;
		    }
		}
	        if (*src == '\0' || *src == '\n')
		    src--;
	        break;
			

	    default :
		os << *src;
	        break;
	      
	  }
	was_nl = 0;
    }

    if (!was_nl) {
	os << '\n';
    }
    os << ends;
		
    string ret_buf = os.str();
    char *ret_val = new char[ret_buf.size() + 1];
    strcpy(ret_val, ret_buf.c_str());

    return ret_val;
}


class string_and_int {
public:
  char *string_field;
  int int_field;
  string_and_int () : string_field(0), int_field(0) {}
  ~string_and_int () {}
};

/* the following little class contains the set of valid compiler flags */
class charArr_int_Hash : public nameHash
{
  virtual char const *name(const Object* oo)const 
    { return (char const *) ((string_and_int*)oo)->string_field;}
  virtual bool isEqualObjects(const Object& o1, const Object&o2) const
    { return !strcmp((char const *)((string_and_int*)&o1)->string_field, (char const *)((string_and_int*)&o2)->string_field);}
} valid_compiler_flags_edg;

int dis_script_file(char const *script, genString& fname);
static void load_compiler_flags()
{
    FILE *fp = NULL;
    char buffer[1024];
    genString fname;
    
    int found = dis_script_file("valid_compiler_options_edg.dat", fname);
    if (found) 
        fp = OSapi_fopen(fname, "r");
    else {
	return;
    }
    if (fp) {
	while (fgets(buffer, sizeof(buffer) - 1, fp)) {
	     // find the start...
	     char *start, *end;
	     int flag = 0;
	     for (start = buffer; isspace(*start); ++start);
	     // ...the end...
	     for (end = start; *end && !isspace(*end) && *end != '\r'; ++end);
	     // ...remove the trailing newline (and cr, if on DOS)...
	     for(char *cur = end; *cur && *cur != '\n' && *cur != '\r'; ++cur)
	       if (!isspace(*cur)) {
		 flag = 1;
		 break;
	       }
	     *end = 0;
	     // ...and add to the set:
	     string_and_int tmp;
	     tmp.string_field = (char *)start;
	     if (!valid_compiler_flags_edg.includes(*(Object*)&tmp)) {
	       string_and_int* data = new string_and_int;
	       data->string_field = OSapi_strdup(start);
	       data->int_field = flag;
	       valid_compiler_flags_edg.add(*(Object*)data);
	     }
	}
	OSapi_fclose(fp);
    }
    else {
	msg("Unable to open valid compiler flags file $1", error_sev) << (char const *)fname << eom;
    }
}

static int is_compiler_flag_internal (char const *flag)
{
   static bool flags_loaded_from_file = false;  /* lazy load: function static is better
		than uncontrolled initialization of global statics!  */
   int retval = 0;
   if (flag[0] != '-')
       return 0;

   // load if uninitialized:
   if ( !flags_loaded_from_file ) {
       load_compiler_flags();
       flags_loaded_from_file = true;
   }
 
   string_and_int tmp;
   tmp.string_field = (char *)flag;
   string_and_int *res =  (string_and_int*)valid_compiler_flags_edg.findObjectWithKey(*(Object*)&tmp);
   if (res) 
       retval = res->int_field ? 2 : 1;
   return retval;
}

static int is_compiler_flag(char const *flag)
{
  static int parm_value = 0;

  int retval = 0;
  if (parm_value) {
    retval = 1;
    parm_value = 0;
  } else {
    retval = is_compiler_flag_internal(flag);
    if (retval == 2) {
      retval = 1;
      parm_value = 1;
    }
  }
   
  return retval;
}

static void save_parser_flags(char const *line) {
   saved_compiler_flags.removeAll();
   genString tmp = line;
   char *buf = smt_parse_flags((char *)tmp);
   char const *w;
   char const *w1;
   for(w = buf; *w; w = w1) {
       for (w1 = w; *w1 && *w1 != '\n'; ++w1);
       if(w1 != w) {
	   char *candidate = copy(w, w1 - w);
	   if (is_compiler_flag(candidate)) {
	    // Note: the memory allocated by "copy" in the preceeding line
	    // will be deleted, along with the copies of the parameters
	    // from the preferences, after constructing the command line.
	    saved_compiler_flags.insert_last(int(candidate));
	   } else delete(candidate);
       }
       if (*w1 == '\n')
	   ++w1;
   }
   delete [] buf;
}

//
// smt_make_cpp_parm
//
// When source code is preloaded, obtains all necessary compiler flags,
// include paths, and pre-processor definitions, by calling make action for
// the object file (while the object file is moved to a temporary file
// name). Appends extra flags, include paths and pre-processor definitions
// from the customize class.
//
void get_make_string (char const *filename, genString& gs);
void get_o_file (char const *filename, genString& gs);
void get_cc_direct (char const *filename, genString& cc_direct);

static void push_flags (int& argc, void* parm, char const *flags)
{
    if (!flags || *flags == 0)
	return;

    char const *w, *w1;
    assertion_flag = 0;
    
    for(w = flags; *w; w = w1) {
	for (w1 = w; *w1 && *w1 != '\n'; ++w1);
	if(w1 != w) {
	    switch(is_cpp_flag(w, w1 - w)) {
	      case 1:
		argc = smt_push_arg (parm, copy(w, w1 - w));
		break;

	      case 2:
		/* case -traditional-cpp */
		argc = smt_push_arg (parm, copy(w, w1 - w - 4));
		break;
	    }
	}
	if (*w1 == '\n')
	    ++w1;
    }
}

static void default_flags (int& argc, void* parm, char const *flags,
			   int sw, int lang)
{
    if (!flags || *flags == 0)
	return;

    genString tmp = flags;
    char *buf = smt_parse_flags ((char *)tmp);
    if (buf) {
	push_flags (argc, parm, buf);
	if (sw)
	    if (lang == 1)
		c_flags = buf;
	    else if (lang == 2)
		cp_flags = buf;
	delete [] buf;
    }
}

static bool matched (char const *token, char const *string)
{
    if (!token || !*token || !string || !*string)
	return 0;

    int len = strlen (token);

    char const *w = string;

    while (1) {
	for (; *w && isspace(*w); ++w);

	if (*w == 0)
	    return 0;

#ifdef _WIN32
	if (ignore_case) {
	    if (!_strnicmp (token, w, len))
		  return 1;
	} else
#endif
	if (!strncmp (token, w, len))
	    return 1;

	for (; *w && !isspace(*w); ++w);
    }
}

static void merge_prefs_and_make_output(genString& prefs,
					char const *make_output) {
    char const *prefs_str = prefs;
    char const *make_loc = strstr(prefs_str,
	  CompilerOptions::make_output_location_string);
    if (make_loc) {
	genString combo;
	char const *newline = strchr(make_output, '\n');
	size_t make_output_len = newline ? newline - make_output :
	        strlen(make_output);
	combo.printf("%.*s %.*s %s", make_loc - prefs_str, prefs_str,
		     make_output_len, make_output,
		     make_loc +
		     CompilerOptions::make_output_location_string_len);
	prefs = combo;
    }
    else prefs += make_output;
}

static int old_make_cpp_parm(
    objArr_Int& parm, int& np, char const *lname, char const *filename,
    genString& lang)
{
    Initialize(old_make_cpp_parm);

    int i, j;
    char *buf = NULL;

    // Use make action to find -I, -D, and other miscellaneous flags.
    genString make_string;
    get_make_string (filename, make_string);
    int use_same_switch = 0;;
    if (make_string.str() && (make_string.str())[0] == '[' 
	&& (make_string.str())[1] == 's' && (make_string.str())[2] ==']') {
	use_same_switch = 1;
        make_string = make_string.str() + 3;
        char const *slash = strrchr(filename, '/');
        if (directory_of_file.str() && strlen ((char *)directory_of_file) == slash - (char *)filename
	    && !strncmp((char *)filename, (char *)directory_of_file,slash - (char *)filename)) {
	    char *fl;
	    if (!strcmp((char *)lang,"CP"))
		fl = cp_flags;
	    else
		fl = c_flags;
	    if (fl) {
		smt_push_arg (&parm, copy("aset_cpp"));
                if(lname) {
                    smt_push_arg (&parm, copy(filename));
                } else {
                    char *parm_tmp = new char[24];
                    strcpy(parm_tmp, "<Temporary>");
		    void *header = 0;
                    OS_dependent::bcopy(&header, parm_tmp + strlen(parm_tmp) + 1, sizeof header);
                    smt_push_arg (&parm, parm_tmp);
                }
                np = 2;

		push_flags(np, &parm, fl);
		save_parser_flags(fl);
		return 0;
	    }
	} else {
	    cp_flags = 0;
	    c_flags = 0;
	    directory_of_file.put_value (filename, slash - (char *)filename);
	}
    }
    discard_esql_flags();
    char const *esql_pp_cmd = NULL;
    genString cc_direct;
    get_cc_direct (filename, cc_direct);
    genString target;
    get_o_file (filename, target);
    genString protocol = "\t"; // collect information to this variable
    if(lname) {
	static char *tfn = OSapi_tempnam(0, "maken");
	static char *tfn_err = OSapi_tempnam(0, "maken_err");
	genString cmd_line;
	
	if (make_string.length ()) {
	    cmd_line.printf("(%s) 1> \"%s\" 2> \"%s\"", make_string.str(), OSPATH(tfn), OSPATH(tfn_err));
	    protocol.printf("Make action from pdf:\n%s", (char *)make_string);
        } else {   
            msg("No make rule for $1: Will call make -n.", warning_sev) << filename << eom;
	    cc_direct = customize::makef_dir();

	    // Find beginning of base filename.
	    char const *basename = strrchr(filename, '/');
	    if(basename)
	        basename++;
	    else
	        basename = filename;

	    // Construct name of make target.
	    char const *dot = strrchr(basename, '.'); // Beginning of extension
	    genString tmp;
	    tmp.put_value(basename, dot ? dot - basename : strlen(basename));
	    tmp += ".o";

	    // Construct command line to call make -n
	    cmd_line.printf("%s -n %s 1> \"%s\" 2> \"%s\"", customize::make(), tmp.str(), tfn, tfn_err);
	    protocol.printf("Call make -n:\n%s -n", customize::make());
	    target = cc_direct;
	    int len;
	    if ((len = target.length ()) && target.str ()[len - 1] != '/')
		target += "/";
	    target += tmp;
	}

	// Construct temporary backup name for make target.
	genString backup_target = target;
	backup_target += ".~~PSETBACKUP~~";

        if (target.length())
        {
            // Rename target file if it exists.
            if ((i = OSapi_access(target, F_OK)) == 0)
            {
                int  status;
                struct OStype_stat buf;

                status = global_cache.stat(target,&buf);
                if (status != -1 && OSapi_S_ISDIR(&buf)) // bad
                {
		    msg("The .o file '$1' defined in PDF is a directory.", warning_sev) << target.str() << eom;
                    i = -1;
                }
                else if(i = OSapi_rename(target, backup_target))
                {
		    msg("Could not temporarily rename \"$1\" to \"$2\".", warning_sev) << target.str() << eoarg << backup_target.str() << eom;
                    perror("");
		    genString tmp;
		    tmp.printf("\nCould not temporarily rename %s to %s.", target.str(), backup_target.str());
		    protocol += tmp;
                }
            }
            else
            {
                i = -1;
            }
        }
        else
            i = -1;

	// Perform the make action
        vsystem(cmd_line);

	// Restore target file if it was previously renamed.
	if(i == 0)
	    OSapi_rename(backup_target, target);


	// Read the output of make action
	int fd = OSapi_open(tfn, O_RDONLY);
	int fd_err = OSapi_open(tfn_err, O_RDONLY);
	{
	    static int count = 0;
	    if(fd > 0) {
#ifndef _WIN32
	        struct stat stat_buffer;
#else
		struct _stat stat_buffer;
#endif
		OSapi_fstat(fd, &stat_buffer);
		if(buf = new char [stat_buffer.st_size + 1]) {
		    int n = OSapi_read(fd, buf, (int)stat_buffer.st_size);
		    if(n < 0  ||  n > stat_buffer.st_size)
		        n = 0;
		    else 
		        n = dos2unix(buf,n);
		    buf[n] = 0;

		    cnv_argstr_path_2OS(buf);
		    convert_back_slashes(buf);
		    
		    int have_make_output = (buf[0] != (char)0);
		
		    if (!count) {
		        cpp_debug = OSapi_getenv("CPP_DEBUG") ? 1 : 0;
			count = 1;
		    }
		    if(have_make_output > 0) {
		        protocol += "\nMake output:\n";
		        protocol += buf;
		        if (cpp_debug) {
			    msg("DIAG: Make output:\n$1", normal_sev) << buf << eom;
		        }
		    } else {
		        protocol += "\nNo Make output.\n\n";
		        if (cpp_debug) {
			    msg("DIAG: No output from make action.", normal_sev) << eom;
		        }
		    }
		} else {
		    msg("Could not allocate buffer to read make action output.", warning_sev) << eom;
		    buf = new char [1]; buf[0] = '\000';
		    perror("");
		}
		OSapi_close(fd);
	    } else {
		msg("Could not open make action output file '$1'.", warning_sev) << tfn << eom;
		perror("");
		genString tmp;
		protocol += "\nCould not open output file for make action";
		buf = new char [1]; buf[0] = '\000';
	    }
	    if(fd_err > 0) {
#ifndef _WIN32
	        struct stat stat_buffer;
#else
		struct _stat stat_buffer;
#endif
		OSapi_fstat(fd_err, &stat_buffer);
		char *err_buf;
		if(stat_buffer.st_size) {
		  if (err_buf = new char [stat_buffer.st_size + 1]) {
		    int n = OSapi_read(fd_err, err_buf, (int)stat_buffer.st_size);
		    if((n < 0) || (n > stat_buffer.st_size))
		        n = 0;
		    else 
		        n = dos2unix(err_buf,n);
		    err_buf[n] = 0;
		    
		    cnv_argstr_path_2OS(err_buf);
		    convert_back_slashes(err_buf);
		      
		    int have_error_output = (err_buf[0] != (char)0);

		    if (!count) {
		        cpp_debug = OSapi_getenv("CPP_DEBUG") ? 1 : 0;
			count = 1;
		    }
		    if (have_error_output && cpp_debug) {
		        msg("Make action reported the following error(s):\n$1", error_sev) << err_buf << eom;
		    }
		  } else {
		    msg("Make action sent output to stderr: can't allocate a buffer to read it!", warning_sev) << eom;
		  }
		}
		OSapi_close(fd_err);
	    }
	}

	if (!OSapi_chdir (cc_direct)) {
	    char *str = (char *)cc_direct;
	    if (str[0] == '/')
		pset_set_current_directory(cc_direct);
	    else {
		genString w = pset_get_current_directory();
		w += '/';
		w += cc_direct;
		pset_set_current_directory(w);
	    }
	}
	char cwd[MAXPATHLEN + 1];
	OSapi_getwd(cwd);
	cc_execution_directory = cwd;
    }

    // Clear parameters array

    smt_push_arg (&parm, copy("aset_cpp"));

    if(lname) {
	smt_push_arg (&parm, copy(filename));
    } else {
	char *parm_tmp = new char[24];
	strcpy(parm_tmp, "<Temporary>");
	void *header = 0;
	OS_dependent::bcopy(&header, parm_tmp + strlen(parm_tmp) + 1, sizeof header);
	smt_push_arg (&parm, parm_tmp);
    }
    np = 2;

    // Frequently re-used variables for parsing output.
    char const *w;
    char const *w1;

    // Look for command line and compiler name.
    // Call customize class on all potential compiler names.
    genString flags;
    CompilerOptions compilerOptions;
    char const *compiler_name_c = compilerOptions.getName(CompilerOptions::C_COMPILER);
    char const *compiler_name_cp =compilerOptions.getName(CompilerOptions::CPP_COMPILER);
    char const *esql_pp_name_c = customize::c_esql_pp();
    char const *esql_pp_name_cp = customize::cplusplus_esql_pp();
    if (strcmp((char *)lang, "CP") == 0)    // C++
        flags = compilerOptions.getOptions(CompilerOptions::CPP_COMPILER);
    else if (strcmp((char *)lang, "C") == 0)  // C
        flags = compilerOptions.getOptions(CompilerOptions::C_COMPILER);

    // Copy default flags, includes, and defines given by customize class.
    // In make action output, delete linebreaks which have
    // backslash immediately before them.  Then turn any semicolons into
    // newlines.
    smt_make_compile_line = buf;
    char *wb              = new char[strlen(buf) + 2];
    for(i = j = 0; buf[i]; i++) {
	if(buf[i] == '\\'  &&  buf[i + 1] == '\n') {
	    i++;
	    continue;
	}
	if (buf[i] == ';')             // change any semicolons into newlines
	    buf[i] = '\n';
        if ((buf[i] == '&' && buf[i+1] == '&') || (buf[i] == '|' && buf[i+1] == '|')) {
	    buf[i] = '\n';
	    buf[i+1] = '\n';
        } 
	if (buf[i]=='t'&&buf[i+1]=='h'&&buf[i+2]=='e'&&buf[i+3]=='n'&&isspace(buf[i+4]) && (!i||isspace(buf[i-1]))) {
	    buf[i]=buf[i+1]=buf[i+2]=buf[i+3] = '\n';
	}
	if (buf[i]=='e'&&buf[i+1]=='l'&&buf[i+2]=='s'&&buf[i+3]=='e'&&isspace(buf[i+4])&& (!i||isspace(buf[i-1]))) {
	    buf[i]=buf[i+1]=buf[i+2]=buf[i+3] = '\n';
	}
	if (buf[i]=='e'&&buf[i+1]=='l'&&buf[i+2]=='i'&&buf[i+3]=='f'&&isspace(buf[i+4])&& (!i||isspace(buf[i-1]))) {
	    buf[i]=buf[i+1]=buf[i+2]=buf[i+3] = '\n';
	}
	    
	wb[j++] = buf[i];
    }
    wb[j++] = 0;
    wb[j] = 0;
    delete(buf);
    buf = 0;

#ifdef _WIN32
    // Check pref value used in matched() and below
    // indicates that compiler name compares should be case-insensitive
    char const *icpref = (char *)customize::getStrPref("compilerIgnoreCase");
    if (icpref) {
        if (!_stricmp (icpref, "no") || !strcmp (icpref, "0") ) {
	    ignore_case = 0;
        }
    }
#endif

    int found_comp = 0;
    genString envn;
    // Seek to line of output that contains compile command.
    char *line;
    for(line = wb; *line; line++) {
	if (*line == '+') // "sh -x" print such +
	    line++;
	while(isspace(*line))
	    line++;
	for(w = line; *w && !isspace(*w); w++);
	for(w1 = w - 2; w1 >= line && *w1 != '/'; w1--);

	envn.put_value(w1 + 1, w - w1 - 1);
	
	// Temporary patch until we get the new preferences interface fixed.
	if (matched (envn, compiler_name_c)) {
	    if (!matched (envn, compiler_name_cp))
		found_comp = 1; /* C */
	    else {
		found_comp = -1; /* not recognized yet */
#ifdef _WIN32
	        // Both C and C++ compiler names match -- use file extension
		// Bug#15102 1998/11/09 mkamin
	        if (ignore_case) {
		    if (strcmp((char *)lang, "CP") == 0)    // C++
		        found_comp = 2;
		    else if (strcmp((char *)lang, "C") == 0)  // C
			found_comp = 1;
		}
#endif
	    }
	    break;
	}
	else if (matched(envn, compiler_name_cp)) {
	    found_comp = 2;       /* C++ */
	    break;
	}
	else if (matched(envn, esql_pp_name_c)) { 
	    esql_pp_cmd = w;
	    found_comp = 3;
	}
	else if (matched(envn, esql_pp_name_cp)) {
	    esql_pp_cmd = w;
	    found_comp = 4;
	}
	else if (envn == (char const *)"cc") {
	    found_comp = 1;	      /* C */
	    break;
	}
	else if (envn == (char const *)"CC" 
		   || envn == (char const *)"OSCC"
		   || envn == (char const *)"g++"
		   || envn == (char const *)"c++") {
	    found_comp = 2;       /* C++ */
	    break;
	} else if (envn == (char const *)"gcc" || envn == (char const *)"gcc68") {
	    found_comp = -1;      /* not recognized yet */
	    break;		
	}

	// Skip to end of line.
	while(*line  &&  *line != '\n')
	    line++;
    }

    if (found_comp) {
	genString esql_prefs;
	protocol += "\nFound compiler: ";
	protocol += envn;
	DBG {
	    msg("Found compiler: $1", normal_sev) << envn.str() << eom;
	}
	// Get remaining flags, includes, and defines from output of make action.
	switch (found_comp) {
	  case 1:
	    if (esql_pp_cmd) {
		lang = "ESQLC";
		esql_prefs = customize::c_esql_args();
	    }
	    else lang = "C";
	    flags = compilerOptions.getOptions(CompilerOptions::C_COMPILER);
	    break;
	    
	  case 2:
	    if (esql_pp_cmd) {
		lang = "ESQLCP";
		esql_prefs = customize::cplusplus_esql_args();
	    }
	    else lang = "CP";
	    flags = compilerOptions.getOptions(CompilerOptions::CPP_COMPILER);
	    break;

	  default:
	    if (esql_pp_cmd) {
		if (lang[1u] == 'P') {
		    lang = "ESQLCP";
		    esql_prefs = customize::cplusplus_esql_args();
		}
		else {
		    lang = "ESQLC";
		    esql_prefs = customize::c_esql_args();
		}
	    }
	    break;
	}
	if (esql_pp_cmd) {
	    merge_prefs_and_make_output(esql_prefs, esql_pp_cmd);
	    save_esql_flags(esql_prefs);
	}
	DBG {
	    msg("default flags $1", normal_sev) << flags.str() << eom;
	}
	for (; *line && !isspace(*line); ++line);

	merge_prefs_and_make_output(flags, line);
	default_flags (np, &parm, flags, use_same_switch, found_comp);
	save_parser_flags(line);
    } else {
	msg("Cannot recognize compiler name for $1 :\n$2", error_sev) << filename << eoarg << (char const *)protocol << eom;
	if (wb) delete [] wb;
	return 1;
        default_flags (np, &parm, flags, 0, 0);
    }
    if (wb) delete [] wb;
    char const *nm = smt_get_name_macros_file();
    np = smt_push_arg (&parm,  copy("-pcd"));
    np = smt_push_arg (&parm, copy(nm));

    char const *deb = customize::getStrPref("debugShowMakeRuleOutput");
    if (deb && strcmp(deb, "yes") == 0) {
	msg("$1", normal_sev) << (char const *)protocol << eom;
    }
    return 0;
}
    
static genString cpp_directory;
static genString cpp_command;
static genString pcc_command;
static genString as_output;
static genString parser_command;

int smt_make_cpp_parm(
    objArr_Int& parm, int& np, char const *lname, char const *filename,
    genString& lang)
{
    Initialize(smt_make_cpp_parm);
    timer::init(1, "make_cpp", lname);

    set_common_tentative_defs(0);

    int retcode = old_make_cpp_parm(parm, np, lname, filename, lang);
    timer::showtime(1, "make_cpp", lname);
    return retcode;
}
    
static
void make_comp_parm (  int& argc ,  void* parm , char const *lang)
{
    char const *flags;
    char const *parmend;
    char const *candidate;
    CompilerOptions compInfo;
    bool already_inserted_saved_flags = false;

    if (lang && (strcmp(lang, "C") == 0) ) {
        flags = compInfo.getOptions(CompilerOptions::C_COMPILER);
    } else if (lang && (strcmp(lang, "CP") == 0) ) {
        flags = compInfo.getOptions(CompilerOptions::CPP_COMPILER);
    } else
        flags = 0;
    
    if(flags) {
	for(char const *parmbegin = flags; *parmbegin; parmbegin = parmend) {
	    while(isspace(*parmbegin))
		parmbegin++;
	    for(parmend = parmbegin; *parmend && !isspace(*parmend); parmend++);
	    if(parmend != parmbegin) {
		candidate = copy(parmbegin, parmend - parmbegin);
		if ( is_compiler_flag (candidate)) {
		    argc = smt_push_arg (parm, candidate);
		}
		else {
		    if (strncmp(parmbegin,
			       CompilerOptions::make_output_location_string,
			       CompilerOptions::make_output_location_string_len)
			== 0) {
			for (size_t i = 0; i < saved_compiler_flags.size(); i++) {
			   ((objArr_Int*) parm)->insert_last(saved_compiler_flags[i]);
			}
			already_inserted_saved_flags = true;
		    }
		    delete (candidate);
		}
	    }
	}
    }
    if (!already_inserted_saved_flags) {
	for (size_t i = 0; i < saved_compiler_flags.size(); i++) {
	    ((objArr_Int*) parm)->insert_last(saved_compiler_flags[i]);
	}
    }
    argc += saved_compiler_flags.size();
    saved_compiler_flags.removeAll();
}

void smt_clear_cpp_parm(char const ** parm, int maxpar)
{
    for(int i = 0; i < maxpar; i++) {
	delete parm[i];
	parm[i] = 0;
    }
}

void smt_set_new_language (projModule *mod, fileLanguage& language, genString& lang)
{
    Initialize(smt_set_new_language);
    fileLanguage new_language;
    if (lang == (char const *)"CP")
	new_language = smt_CPLUSPLUS;
    else if (lang == (char const *)"C")
	new_language = smt_C;
    else if (lang == (char const *)"ESQLC")
	new_language = smt_ESQL_C;
    else if (lang == (char const *)"ESQLCP")
	new_language = smt_ESQL_CPLUSPLUS;

    if(language != new_language)
	mod->language(new_language);
}

static void smt_convert_param_array (objArr_Int& parm, int delta, int np, genArrOf(genString) &params)
{
    Initialize(smt_convert_param_array);
    char const **parms = (char const **) & parm[0];
    int start = delta; // aset_cpp $fn
    int end = np - delta; // -pcd ...
    TclList sub;
    params.reset();
    for(int ii=start; ii<end; ++ii){
	char const *par = ParamConvertTable::ConvertParameter(parms[ii]);
	if(par != NULL && par[0] != 0) {
	    genString expanded;
	    eval_shell_metachars (par, expanded);

	    //Tcl_AppendResult(i->interp, " ", (char const *)expanded, NULL);
	    // Catch cases where the transformed parameter is actually
	    // several parameters (e.g.: "--no_restrict --no_rtti"): 

	    sub = (char const *)expanded;
	    int narg = sub.Size();
	    if (narg > 1 && sub[0][0] == '-' && sub[0][1] == '-') {
		for (int jj = 0; jj < narg; jj++) {
		    genString s = sub[jj];
		    params.append(&s);
		}
	    }
	    else {
		params.append(&expanded);
	    }
	}
    }
    smt_clear_cpp_parm(parms, np);
}

static void param_array_to_txt(genArrOf(genString) &params, genString &txt)
{
    Initialize(param_array_to_txt);
    txt = 0;
    int len = params.size();
    if (len > 0) {
	TclList list;
	for(int ii=0; ii<len; ++ii){
	    list += (char const *)(*params[ii]);
	}
	txt = list.Value();
    }
}


//returns: 0 - success
// Sets 'verbatim' to true if the result is returned as text string.
// Otherwise, result is an array which should be escaped for the shell.
int smt_get_esql_param_array (projModule *mod, int &verbatim, genString &txt, 
                              genArrOf(genString) &params)
{
    Initialize(smt_get_esql_param_array);
    txt     = 0; 
    params.reset();
    verbatim = false;
    int err = -1;
    if (!mod) return err;

    err           = 0;
    char const *ln = mod->get_name();
    char const *fn = mod->get_phys_filename();

    genString lang;
    fileLanguage language = mod->language();

    // boris: 04/08/99. Algorithm for flags for esql: 
    //        if the make rule exists, then use it,
    //        otherwise try to get sql flags from Preferences
    //

    if (language == smt_ESQL_CPLUSPLUS || language == smt_ESQL_C) {
	genString make_string;
	get_make_string (fn, make_string);
	if (make_string.length() == 0) {
	    extern_els_init ();
	    txt = els_flags ("sql");
	    if (txt.length() == 0) err = -1;
	    verbatim = true;
	    return err;
	}
    }

    if (language == smt_CPLUSPLUS || language == smt_ESQL_CPLUSPLUS)
	lang = "CP";
    else
	lang = "C"; 

    objArr_Int parm;
    int np = 0;

    err = smt_make_cpp_parm(parm, np, ln, fn, lang);
    if (err == 0) {
	smt_set_new_language (mod, language, lang);
	np = saved_esql_flags.size();
	if (np) smt_convert_param_array (saved_esql_flags, 0, np, params);
    }

    return err;
}

//returns: 0 - success
int smt_get_esql_params (projModule *mod, genString& txt)
{
    Initialize(smt_get_esql_params);
    genArrOf(genString) params;
    int verbatim = false;
    int err = smt_get_esql_param_array(mod, verbatim, txt, params);
    if (verbatim) {
	// Result is in txt.
    }
    else {
	param_array_to_txt(params, txt);
    }
    return err;
}

//returns: 0 - success
int smt_get_cpp_param_array (projModule *mod, genArrOf(genString) &params)
{
    Initialize(smt_get_cpp_param_array);
    params.reset();
    int err = -1;
    if (!mod) return err;

    err           = 0;
    char const *ln      = mod->get_name();
    char const *fn = mod->get_phys_filename();

    genString lang;
    fileLanguage language = mod->language();

    if (language == smt_CPLUSPLUS || language == smt_ESQL_CPLUSPLUS)
	lang = "CP";
    else
	lang = "C"; 

    objArr_Int parm;
    int np = 0;

    err = smt_make_cpp_parm(parm, np, ln, fn, lang);
    if (err == 0) {
	smt_set_new_language (mod, language, lang);
	smt_convert_param_array (parm, 2, np, params); //2 for aset_cpp $fn, for  -pcd ...
    }
    return err;
}

//returns: 0 - success
int smt_get_cpp_params (projModule *mod, genString& txt)
{
    Initialize(smt_get_cpp_params);
    genArrOf(genString) params;
    int err = smt_get_cpp_param_array(mod, params);
    param_array_to_txt(params, txt);
    return err;
}

extern projModule * symbol_get_module( symbolPtr& sym);
static int make_cpp_parmCmd(Interpreter*i, int argc, char const **argv)
{
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  if(arr.size() != 1)
    return TCL_ERROR;
  
  symbolPtr& sym = arr[0];

  projModule* mod = symbol_get_module(sym);
  if(!mod) return TCL_ERROR;

  genString txt;
  int err = smt_get_cpp_params (mod, txt);
  if (err) return TCL_ERROR;

  Tcl_SetResult(i->interp, (char *)txt, TCL_VOLATILE);
  return TCL_OK;
}

static int make_esql_parmCmd(Interpreter*i, int argc, char const **argv)
{
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  if(arr.size() != 1)
    return TCL_ERROR;
  
  symbolPtr& sym = arr[0];

  projModule* mod = symbol_get_module(sym);
  if(!mod) return TCL_ERROR;

  genString txt;
  int err = smt_get_esql_params (mod, txt);
  if (err) return TCL_ERROR;

  Tcl_SetResult(i->interp, (char *)txt, TCL_VOLATILE);
  return TCL_OK;
}

int smt_make_all_param_array (char const *fn, char const *language, genArrOf(genString) &all_parm)
{
    Initialize(smt_make_all_param_array);
    all_parm.reset();
    char const *ln  = fn;
    genString lang;

    if (strcmp(language, "C") == 0)
	lang = "C";
    else
	lang = "CP"; 

    objArr_Int pre_parm;
    int np = 0;

    int err = smt_make_cpp_parm(pre_parm, np, ln, fn, lang);
    if (err) return err;
    smt_convert_param_array (pre_parm, 2, np, all_parm); //2 for aset_cpp $fn, np-2 for  -pcd ...

    objArr_Int parm;
    genArrOf(genString)  post_param_array;
    np = 0;

    make_comp_parm       (np, &parm, lang);
    smt_convert_param_array   (parm, 0, np, post_param_array);

    for (int i = 0; i < post_param_array.size(); i += 1) {
	all_parm.append(post_param_array[i]);
    }
    return 0;
}

int smt_make_all_parm (char const *fn, char const *language, genString &all_parm)
{
    Initialize(smt_make_all_parm);
    genArrOf(genString) params;
    int err = smt_make_all_param_array(fn, language, params);
    param_array_to_txt(params, all_parm);
    return err;
}

static int make_flagsCmd(Interpreter*i, int argc, char const **argv)
{
  if(argc != 3){
    msg("$1: wrong number of args", error_sev) << argv[0] << eom;
    return TCL_ERROR;
  }
  char const *fn       = argv[1];
  char const *language = argv[2];
  genString       all_flags;

  int err = smt_make_all_parm (fn, language, all_flags);
  if (err) return TCL_ERROR;
  Tcl_AppendResult(i->interp, all_flags.str(), NULL);
  return TCL_OK;
}

int smt_get_comp_param_array (projModule *mod, genArrOf(genString) &params)
{
    Initialize(smt_get_comp_param_array);
    params.reset();
    int err = -1;
    if(!mod) return err;
    
    err           = 0;

    genString lang;
    fileLanguage language = mod->language();
    
    if (language == smt_CPLUSPLUS || language == smt_ESQL_CPLUSPLUS)
	lang = "CP";
    else
	lang = "C"; 

    objArr_Int parm;
    int np = 0;

    make_comp_parm       (np, &parm, lang);
    smt_set_new_language (mod, language, lang);
    smt_convert_param_array   (parm, 0, np, params);
    return err;
}

int smt_get_comp_params (projModule *mod, genString& txt)
{
    Initialize(smt_get_comp_params);
    genArrOf(genString) params;
    int err = smt_get_comp_param_array(mod, params);
    param_array_to_txt(params, txt);
    return err;
}

static int make_comp_parmCmd(Interpreter*i, int argc, char const **argv)
{
  symbolArr arr;
  if(i->ParseArguments(1, argc, argv, arr) != 0)
    return TCL_ERROR;
  if(arr.size() != 1)
    return TCL_ERROR;
  
  symbolPtr& sym = arr[0];

  projModule* mod = symbol_get_module(sym);
  genString txt;
  int err = smt_get_comp_params (mod, txt);
  if (err)
      return TCL_ERROR;

  Tcl_SetResult(i->interp, (char *)txt, TCL_VOLATILE);
  return TCL_OK;
}

projModule* api_find_module(char const *cur_dir, char const *str, genString& ln);
int ATT_outdated( projModule& mod);

extern "C" int remove_all_dots_and_double_slashes (char const *path, char *resolved_path);
#ifdef _WIN32
inline bool is_absolute(char const *path) {return path[0] == '/' || path[1] == ':';}
#else
inline bool is_absolute(char const *path) {return path[0] == '/';}
#endif

projModule* api_find_module(char const *cur_dir, char const *str, genString& ln)
{
  projNode *proj  = NULL;
  projModule *mod = NULL;

  char pathname[MAXPATHLEN];

  char buf[MAXPATHLEN];

  if(!is_absolute(str)){
    //    relative path
    if(!cur_dir)
      return NULL;

    int len = strlen(cur_dir);    
    strcpy(buf, cur_dir);
    if(cur_dir[len-1] != '/'){
      buf[len] = '/';
      len++;
    }
    strcpy(buf+len, str);
    str = buf;
  }
  if(strstr(str, "/.") || strstr(str, "//")){
    int res = remove_all_dots_and_double_slashes (str, pathname);
    if(res)
      return NULL;
    str = pathname;
  } 

  if(str[0] != '/' && str[1] == ':'){
    // WIN drive letter
    char *dis_path = NULL;
    create_path_2DIS( str, &dis_path );
    if ( !dis_path )
		return NULL;
    projHeader::fn_to_ln(dis_path, ln, &proj);
    free(dis_path);
  } else {
    projHeader::fn_to_ln(str, ln, &proj);
  }
  
  if(proj)
    mod = proj->find_module(ln);

  return mod;
}

struct includePath : public objDictionary {
  includePath(char const *nm) : objDictionary(nm) {}
  genString ln;
  projModule*mod;
};

static struct includePathes {
  objSet absolute;
  objSet relative;
  genString current_directory;
  projNode * home;
  fileLanguage language;
  int reset(char const *cur_dir, fileLanguage,int);
  includePath* lookup(char const *path) const;
  bool insert(char const *path, includePath*&);
} include_path_tbl;

int includePathes::reset(char const *cur_dir, fileLanguage lan, int do_force)
{
  char *cur_tmp = strdup(cur_dir);
  int res = 0;
  int len = strlen(cur_tmp);
  if(cur_tmp[len-1] == '.' && cur_tmp[len-2] == '/')
    cur_tmp[len-1] = '\0';

  int force = do_force || is_gui();
  if (force || current_directory != cur_tmp){
    current_directory = cur_tmp;
    relative.purge();
    res += 1;
  }
  projNode*new_home = projNode::get_current_proj();
  if (force || home != new_home){
    home = new_home;
    relative.purge();
    absolute.purge();
    res += 2;
  }
  language = lan;
  free(cur_tmp);
  return res;
}

includePath* includePathes::lookup (char const *path) const
{
  const objSet& tbl = is_absolute(path) ? absolute : relative;
  return (includePath*) obj_search_by_name(path, (objSet&)tbl);
}

bool includePathes::insert (char const *path, includePath*&ip)
{
  objSet& tbl = is_absolute(path) ? absolute : relative;
  ip = (includePath*) obj_search_by_name(path, tbl);
  if(ip)
    return false;
  ip      = new includePath(path);
  ip->mod = api_find_module(current_directory, path, ip->ln);
  if(ip->mod){
      projNode *pr = ip->mod->get_project();
      if(pr && pr->root_project() == projNode::get_home_proj()->root_project())
	  ip->mod->language(language);
  }
  tbl.insert(ip);
  return true;
}

char const *els_include_path_ln(char const *path)
{
  char const *ln;
  includePath*ip = include_path_tbl.lookup(path);
  if (!ip) {
      ip = new includePath(path);
      ip->mod = api_find_module("/", path, ip->ln);
      objSet& tbl = is_absolute(path) ? include_path_tbl.absolute : include_path_tbl.relative;
      tbl.insert(ip);
  }
  ln = ip->ln;
  return ln;
}

static int scan_iff_lnCmd(Interpreter*i, int argc, char const **argv)
{
  Initialize(scan_iff_lnCmd);
  IF(argc <2)
    return TCL_ERROR;

  for(int ii=1; ii<argc; ++ii){
    char const *path = argv[ii];
    char const *ln =  els_include_path_ln(path);
    if(!ln)
      return TCL_ERROR;
    Tcl_AppendElement(i->interp, ln);
  }
  return TCL_OK;
}

static int scan_iff_resetCmd (Interpreter*i, int argc, char const **argv)
{
  Initialize(scan_iff_resetCmd);
  
  IF(argc < 3)
    return TCL_ERROR;

  char const *cur_dir = argv[1];
  char const *lan = argv[2];

  fileLanguage ll;
  if(strcmp(lan, "C")==0)
    ll = FILE_LANGUAGE_C;
  else if(strcmp(lan, "CPP")==0)
    ll = FILE_LANGUAGE_CPP;
  else if(strcmp(lan, "ESQL_C")==0)
    ll = FILE_LANGUAGE_ESQL_C;
  else if(strcmp(lan, "ESQL_CPP")==0)
    ll = FILE_LANGUAGE_ESQL_CPP;
  else if(strcmp(lan, "ELS")==0)
    ll = FILE_LANGUAGE_ELS;
  else {
    msg("$1: unknown language: $2", error_sev) << argv[0] << eoarg << lan << eom; 
    return TCL_ERROR;
  }
  
  include_path_tbl.reset(cur_dir, ll, 1);

  return TCL_OK;
}

int shell_configurator_reset();
int projModule_set_version_start (projModule* pm);

static int scan_iff_fileCmd(Interpreter*i, int argc, char const **argv)
{
  Initialize(scan_iff_fileCmd);
  shell_configurator_reset();
  int cur_ind = 1;
  bool home = false;
  bool outdated = false;
  char const *fn = NULL;
  for(; cur_ind < argc; ++cur_ind){
    fn = argv[cur_ind];    
    if(strcmp(fn, "-home") == 0)
      home = true;
    else if(strcmp(fn, "-outdated") == 0)
      outdated = true;
    else
      break;
  }
  
  IF(cur_ind + 1>= argc)
    return TCL_ERROR;
  char const *cur_dir = argv[cur_ind + 1];

  char const *lan = argv[cur_ind + 2];
  fileLanguage ll;
  if(strcmp(lan, "C")==0)
    ll = FILE_LANGUAGE_C;
  else if(strcmp(lan, "CPP")==0)
    ll = FILE_LANGUAGE_CPP;
  else if(strcmp(lan, "ESQL_C")==0)
    ll = FILE_LANGUAGE_ESQL_C;
  else if(strcmp(lan, "ESQL_CPP")==0)
    ll = FILE_LANGUAGE_ESQL_CPP;
  else if(strcmp(lan, "ELS")==0)
    ll = FILE_LANGUAGE_ELS;
  else {
    msg("$1: unknown language: $2", error_sev) << argv[0] << eoarg << lan << eom; 
    return TCL_ERROR;
  }

  
  include_path_tbl.reset(cur_dir, ll, 0);

  FILE*ff = OSapi_fopen(fn, "r");
  IF(!ff) 
    return TCL_ERROR;

  projNode*home_proj = NULL;
  if(home)
    home_proj = projNode::get_current_proj();
  char buf[1024];
  char const *head = "SMT file \"";
  const int full_match = strlen(head);
  char *f;

  int c;
  int match = 0;
  int eof_seen = 0;

  while (!eof_seen && (c=getc(ff)) != EOF) {
    if (c != head[match]) {
      match = 0;
      while (c != '\n') {
	c = getc(ff);
	if (c == EOF) {
	  eof_seen = 1;
	  break;
	}
      }
      continue;
    }
    match++;
    if (match < full_match)
      continue;

    match = 0;
    f=fgets(buf, 1023, ff);
    if (!f)
      break;
    int len = strlen(f);
    int ch;
    while(len > 0 && (ch = f[len-1])){
      if(ch != '\"' && ch != ' ' && ch != '\r' && ch != '\n' )
	break;
      f[len-1] = '\0';
      --len;
    }
    if(len == 0)
      break;
    
    includePath*ip;
#ifdef XXX_always_ignore_files_after_one_parse
    bool newpath = include_path_tbl.insert(f, ip);
    if(!newpath)
      continue;
#else
    include_path_tbl.insert(f, ip);
#endif
    
    projModule*mod = ip->mod;
    if(!mod)
      continue;
    
    if(home && (mod->get_project() != home_proj))
      continue;
    if(outdated && !ATT_outdated(*mod))
      continue;
    
    projModule_set_version_start (mod);

    Tcl_AppendElement(i->interp, f);
  }

  OSapi_fclose(ff);
  return TCL_OK;
}

symbolPtr xref_header_file_included_from(char const *ln);
static void get_parsable(symbolPtr&mod)
{
  char const *ln = mod.get_name();
  symbolPtr inc = xref_header_file_included_from(ln);
  if(inc.isnotnull()){
    mod = inc;
    get_parsable(mod);
  } else if(!xref_not_header_file(ln)) {
    mod = NULL_symbolPtr;
  }
}

struct ParsableQuery : public cliQueryCommand {
  ParsableQuery(char const *nm) : cliQueryCommand(nm){}
  int query(ostream&os, const symbolArr& in, symbolArr& out);
};
int ParsableQuery::query(ostream&os, const symbolArr& in, symbolArr& out)
{
  int sz = in.size();
  for(int ii=0; ii<sz; ++ii){
    symbolPtr&sym = in[ii];
    symbolPtr mod;
    if(sym.get_kind() != DD_MODULE){
      if(sym.is_xrefSymbol())
	mod = sym->get_def_file();
    } else {
      mod = sym;
    }
    if(mod.isnull())
	continue;

    get_parsable(mod);
    if(mod.isnotnull())
      out.insert_last(mod);
  }
  return TCL_OK;
}
static int add_commands()
{
  new ParsableQuery("src_get_parsable_file");
  new cliCommand("parms_all",      make_flagsCmd);
  new cliCommand("parms_esql",     make_esql_parmCmd);
  new cliCommand("parms_cpp",      make_cpp_parmCmd);
  new cliCommand("parms_comp",     make_comp_parmCmd);
  new cliCommand("scan_iff_file",  scan_iff_fileCmd);
  new cliCommand("scan_iff_ln",    scan_iff_lnCmd);
  new cliCommand("scan_iff_reset", scan_iff_resetCmd);
  return 0;
}


static int dummy = add_commands();
