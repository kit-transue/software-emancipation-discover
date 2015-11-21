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
// customize.h.C
//------------------------------------------
// synopsis:
// Storage, access, and interface for user-customizable
// portions of application.
 

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <machdep.h>
#include <genError.h>
#include <systemMessages.h>
#include <genTmpfile.h>
#include <genString.h>
#include <genArrCharPtr.h>
#include <RegExp.h>
#include <shell_calls.h>
#include <customize_util.h>
#include <customize.h>
#include <pref_registry.h>
#include <objCommon.h>
//#include <ste_interface.h>    // Needed for ste_set_preferences.
#include <Interpreter.h>
#include <elsLanguage.h>
#include <vpopen.h>
#include <charstream.h>
#include <driver_mode.h>

// EXTERNAL VARIABLES
// This variable is defined here but used directly in proj, ui, and xref. Yuk!
int multiple_psets = 0;

// EXTERNAL FUNCTIONS
extern int setup_configurator_envs();

extern "C" void driver_exit(int exit_status);
void  init_env_var_chars();


// CONST DEFINITIONS
const int PREFERENCE_FILE_VERSION = 2;


// VARIABLE DEFINITIONS

// Static pointer to the Preferences registry.
prefRegistry* customize::dicptr = NULL;

// Flag to tell DAVE whether the pmod is writable or not !
static int write_flag;


// C file suffix regexp.
static Regexp* cSuffixRegExp;
// H file suffix regexp.
static Regexp* cHeaderSuffixRegExp;
// esql file suffix regexp.
static Regexp* cEsqlSuffixRegExp;

// C++ file suffix regexp
static Regexp* cPlusSuffixRegExp;
// H++ file suffix regexp.
static Regexp* cPlusHeaderSuffixRegExp;
// C++ esql file suffix regexp
static Regexp* cPlusEsqlSuffixRegExp;
// ELS non ASCII file suffix regexp
static Regexp* elsNonAsciiSuffixRegExp;

bool customize::init_flag = false;

// "instance" must be declared AFTER all the other static variables.
customize* customize::instance;



//////////     Extern C Functions Exported from this file.    //////////
// These use class static functions. They are externed in customize_extern.h .
extern "C" char const* customize_getStrPref(char const * key)
{
    return customize::getStrPref(key, customize::EXPAND_META);
}
extern "C" char const* customize_getStrPref_AsIs(char const * key)
{
    return customize::getStrPref(key, customize::EXPAND_NONE);
}
extern "C" const int customize_getIntPref(char const * key)
{
    return customize::getIntPref(key);
}

//////////     FUNCTION DEFINITIONS     //////////


static Regexp* suffix_regexp(const genString& suffix)
{
    static char SEPARATORS[] = " \t"; // Separators passed to strtok().

    Initialize(suffix_regexp);

    genString input(suffix);	// Copy of suffix to modify with strtok().

    char buffer[256];		// Buffer to hold regexp string as it is built.
    buffer[0] = '\0';

    char* bufptr = buffer;	// Insertion-point for buffer.

    int num_tokens = 0;		// Count of tokens found in input.

    for(char const* token = strtok(input, SEPARATORS);
	token; token = strtok(NULL, SEPARATORS))
    {
	if(*token == '\0')
	    continue;

	*bufptr++ = (++num_tokens == 1 ? '(' : '|');

	for( ; *token; ++token)
	{
	    switch(*token)
	    {
	      case '?':
		*bufptr++ = '.';
		break;

	      case '*':
		*bufptr++ = '.';
		*bufptr++ = '*';
		break;

	      case '\\':
	      case '|':
	      case '+':
	      case '^':
	      case '$':
	      case '(':
	      case ')':
	      case '.':
		*bufptr++ = '\\';
		*bufptr++ = *token;
		break;

	      default:
		*bufptr++ = *token;
		break;
	    }
	}
    }
    if(num_tokens > 0)
    {
	*bufptr++ = ')';
	*bufptr++ = '$';
	*bufptr = '\0';
    }

    return new Regexp(buffer);
}

static void els_init();

#ifdef _WIN32
extern "C" int g_DisFN_RealCase;
#endif

customize::customize()
// Loads the user's file directory and language preferences
// from their home directory.
{
    Initialize(customize::customize);

    // Set the global customize pointer.
    instance = this;

    // Create an instanc for prefRegistry class
    dicptr = new prefRegistry;

    // Initialize special characters for environment variables
    init_env_var_chars();

#ifdef _WIN32
    // Initialize switch for file name case on NT
    g_DisFN_RealCase = 0;
    char const* fnCase = getStrPref("PreserveFilenameCase");
    if ( fnCase && *fnCase && ( *fnCase == '1' || OS_dependent::strcasecmp(fnCase, "Yes") == 0 ) )
      g_DisFN_RealCase = 1;
#endif

    char const* installRoot = install_root();
    if(!installRoot || (strlen(installRoot) == 0) ) {
	msg("The environment variable PSETHOME is not set.", error_sev) << eom;

        driver_exit(1);
    }
 

    setup_configurator_envs();

    // Initialize the default configuration management system

    int cppDisabled = getIntPref("cPlusDisabled");
    if (cppDisabled == 1)
      {
	handle_language_enabled(smt_CPLUSPLUS, 1, 0);
	msg("Disabling CPP mode", normal_sev) << eom;
      }
    int cDisabled = getIntPref("cDisabled");
    if (cDisabled == 1)
      {
	handle_language_enabled(smt_C, 1, 0);
	msg("Disabling C mode", normal_sev) << eom;
      }

    els_init();
    init_flag = true;
}


customize::~customize()
{
    Initialize(customize::~customize);
}





// Preference dictionary get methods.
const genString& customize::getStrPref(char const * key,
                            const int expansion /*prefRegistry::EXPAND_META*/ )
{
    const genString& prefRef = prefRegistry::UNINITIALIZED_STRING_PREF;
    if (dicptr)
        return(dicptr->getStrPref(key, convertExpansion(expansion) ));
    else 
        return(prefRef);
}

const int& customize::getIntPref(char const * key)
{
    const int& intRef = prefRegistry::UNINITIALIZED_INT_PREF;
    if (dicptr)
        return (dicptr->getIntPref(key));
    else 
        return (intRef);
}

bool customize::getBoolPref(char const * key)
{
    if (dicptr)
        return (dicptr->getBoolPref(key));
    else 
        return false;
}


// Preference put methods.
bool customize::putPref(char const * key,
                        const genString value,
                        const int expansion /*prefRegistry::EXPAND_META*/,
                        const bool persistent /*true*/ )
{
    bool status = false;
    if (dicptr)
        status = dicptr->putPref(key, value, expansion, persistent);
    return(status);
}

bool customize::putPref(char const * key, const int value,
                        const bool persistent /*true*/ )
{
    bool status = false;
    if (dicptr)
        status = dicptr->putPref(key, value, persistent);
    return(status);
}


// Utility to translate expansion into prefRegistry valid value.
int customize::convertExpansion(const int expansion)
{
    int prefExp;
    expandString custExp = (expandString) expansion;
    switch (custExp) {
        case EXPAND_NONE:
            prefExp = prefRegistry::EXPAND_NONE;
            break;
        case EXPAND_META:
            prefExp = prefRegistry::EXPAND_NONE_TO_META;
            break;
        case EXPAND_COLONS:
            prefExp = prefRegistry::EXPAND_NONE_TO_COLONS;
            break;
        case UNEXPAND_COLONS:
            prefExp = prefRegistry::EXPAND_COLONS_TO_NONE;
            break;
    }
    
    return prefExp;
}
    




/*-------------------------------------------------------------------
!
! Function:	save_preferences
!
! Synopsis:	Saves the resources to the resource file.
!
_______________________________________________________________________*/

void customize::save_preferences(char const* )
{
    Initialize(customize::save_preferences);
}

//_______________________________________________________________________

void customize::save_boilerplate(char const* project)
{
    Initialize(customize::save_boilerplate);
    errno = 0;
}

void customize::save_specific_boilerplate(int i,genString save_string,char const* project)
{
    Initialize(customize::save_specific_boilerplate);
    errno = 0;
}




void customize::save_configurator(char const* project)
// Saves the resources for the configurator dialog.
//
// project: it is NULL now.
{
    Initialize(customize::save_configurator);
}


//_______________________________________________________________________

void customize::save_buttonbar(char const* project)
{
}






char const* customize::xrefExcludedDirs()
{
    return(getStrPref("excludedDirectories") );
}

void customize::xrefExcludedDirs(char const* dirs)
{
    putPref("excludedDirectories", dirs);
}

//_______________________________________________________________________
void customize::set_write_flag(int flag)
{
    write_flag = flag;
}


//_______________________________________________________________________
int  customize::get_write_flag()
{
    return write_flag;
}

//_______________________________________________________________________
char const* customize::install_root()
{
    static char const* rootPtr = OSapi_getenv("PSETHOME");
    return rootPtr;
}

//_______________________________________________________________________
bool customize::is_init()
{
    return init_flag;
}

//_______________________________________________________________________

char const* customize::subsys_setting_file(int)
{
    return (getStrPref("subsys_setting_file") );
}

void customize::subsys_setting_file(char const* pref)
{
    putPref("subsys_setting_file", pref);
}

//_______________________________________________________________________
char const* customize::configurator()
{
    char const* rootPtr = getStrPref("configurator");

    // Because older versions of preferences may have a null value,
    // We must make sure that a valid root is found.
    if (OSapi_strlen(rootPtr) < 1) {
        msg("Setting configurator to \'$PSETHOME/bin/configurator\'", normal_sev) << eom;
        putPref("configurator", "$PSETHOME/bin/configurator");
        rootPtr = getStrPref("configurator");
        msg("configurator set to \'$1\'", normal_sev) << rootPtr << eom;
    }

    return(rootPtr);
}

void customize::configurator(char const* pref)
{
    putPref("configurator", pref);
}


//_______________________________________________________________________
char const* customize::configurator_diff()
{
    return(getStrPref("configuratorDiff"));
}

void customize::configurator_diff(char const* pref)
{
    putPref("configuratorDiff", pref);
}

//_______________________________________________________________________
char const* customize::configurator_stat()
{
    return(getStrPref("configuratorStat"));
}

void customize::configurator_stat(char const* pref)
{
    putPref("configuratorStat", pref);
}


//_______________________________________________________________________
char const* customize::configurator_get()
{
    return(getStrPref("configuratorGet"));
}

void customize::configurator_get(char const* pref)
{
    putPref("configuratorGet", pref);
}

//_______________________________________________________________________
char const* customize::configurator_copy()
{
    return(getStrPref("configuratorCopy"));
}

void customize::configurator_copy(char const* pref)
{      
    putPref("configuratorCopy", pref);
}

//_______________________________________________________________________
char const* customize::configurator_put()
{
    return(getStrPref("configuratorPut"));
}

void customize::configurator_put(char const* pref)
{
    putPref("configuratorPut", pref);
}

//_______________________________________________________________________
char const* customize::configurator_lock()
{
    return(getStrPref("configuratorLock"));
}

void customize::configurator_lock(char const* pref)
{
    putPref("configuratorLock", pref);
}


//_______________________________________________________________________
char const* customize::configurator_unlock()
{
    return(getStrPref("configuratorUnlock"));
}

void customize::configurator_unlock(char const* pref)
{
    putPref("configuratorUnlock", pref);
}

//_______________________________________________________________________
char const* customize::configurator_unget()
{
    return(getStrPref("configuratorUnget"));
}

//_______________________________________________________________________
char const* customize::configurator_create()
{
    return(getStrPref("configuratorCreate"));
}

//_______________________________________________________________________
char const* customize::configurator_delete()
{
    return(getStrPref("configuratorDelete"));
}

void customize::configurator_delete(char const* pref)
{
    putPref("configuratorDelete", pref);
}

//_______________________________________________________________________
int customize::configurator_terse()
{
    return (getIntPref("configuratorTerse"));
}

void customize::configurator_terse(const int flag)
{
    putPref("configuratorTerse", flag);
}

//_______________________________________________________________________
bool customize::no_cm()
{
    char *val = getStrPref("configuratorSystem");
    if (!val || !val[0] || strcmp(val, "no_cm") == 0)
        return true;
    else
        return false;
}

int customize_no_cm()
{
  return (int)(customize::no_cm());
}

//_______________________________________________________________________
int customize::put_track_bugs()
{
    return (getIntPref("PutTrackBugs"));
}

void customize::put_track_bugs(const int flag)
{
    putPref("PutTrackBugs", flag);
}

//_______________________________________________________________________
char const* customize::configurator_system()
{
    return(getStrPref("configuratorSystem"));
}

void customize::configurator_system(char const* pref)
{
    putPref("configuratorSystem", pref);
}
//_______________________________________________________________________


void customize::set_multiple_psets()
{
    multiple_psets = 0;
}

//_______________________________________________________________________
char const* customize::configurator_custom_dir()
{
    return(getStrPref("configuratorCustomDirectory"));
}

void customize::configurator_custom_dir(char const* pref)
{
    putPref("configuratorCustomDirectory", pref);
}

//_______________________________________________________________________
int customize::configurator_rm_psets()
{
    return (getIntPref("configuratorRemovePsets"));
}

void customize::configurator_rm_psets(const int flag)
{
      putPref("configuratorRemovePsets", flag);
}

//_______________________________________________________________________
int customize::configurator_fast_putmerge()
{
    return (getIntPref("configuratorFastPutMerge"));
}

void customize::configurator_fast_putmerge(const int flag)
{
      putPref("configuratorFastPutMerge", flag);
}

//_______________________________________________________________________
int customize::configurator_use_locks()
{
    return (getIntPref("configuratorUseLocks"));
}

void customize::configurator_use_locks(const int flag)
{
        putPref("configuratorUseLocks", flag);
}

//_______________________________________________________________________
int customize::configurator_put_locks()
{
    return (getIntPref("configuratorPutLocks"));
}

void customize::configurator_put_locks(const int flag)
{
        putPref("configuratorPutLocks", flag);
}

//_______________________________________________________________________
int customize::configurator_get_comments()
{
    return (getIntPref("configuratorGetWithComments"));
}

void customize::configurator_get_comments(const int flag)
{
        putPref("configuratorGetWithComments", flag);
}

//_______________________________________________________________________
int customize::configurator_use_softlinks()
{
    return (getIntPref("configuratorUseSoftLinks"));
}

void customize::configurator_use_softlinks(const int flag)
{
    putPref("configuratorUseSoftLinks", flag);
}

//_______________________________________________________________________
char const* customize::configurator_optscripts_dir()
{
    return(getStrPref("configuratorOptionalScriptsDir"));
}

void customize::configurator_optscripts_dir(char const* pref)
{
    putPref("configuratorOptionalScriptsDir", pref);
}

//_______________________________________________________________________
int customize::configurator_do_backups()
{
    return (getIntPref("configuratorDoBackups"));
}

void customize::configurator_do_backups(int flag)
{
    putPref("configuratorDoBackups", flag);
}

//_______________________________________________________________________
char const* customize::default_project_definition()
{
    return(getStrPref("defaultPDF"));
}

void customize::default_project_definition(char const* pref)
{
    putPref("defaultPDF", pref);
}


//_______________________________________________________________________
char const* customize::gdb()
{
    return(getStrPref("gdbPath"));
}

void customize::gdb(char const* pref)
{
    putPref("gdbPath", pref);
}


//_______________________________________________________________________
char const* customize::make()
{
    return(getStrPref("makePath"));
}

void customize::make(char const* pref)
{
    putPref("makePath", pref);
}

//_______________________________________________________________________
char const* customize::cpp()
{
    return(getStrPref("DIS_c.CPreprocessor"));
}

void customize::cpp(char const* newValue)
{
    putPref("DIS_c.CPreprocessor", newValue);
}


//_______________________________________________________________________
char const* customize::esql()
{
    return(getStrPref("DIS_c.EsqlPreprocessor"));
}

void customize::esql(char const* newValue)
{
    putPref("DIS_c.EsqlPreprocessor", newValue);
}


//_______________________________________________________________________
char const* customize::c_suffix()
{
    return(getStrPref("cSuffix"));
}

void customize::c_suffix(char const* string)
{
    putPref("cSuffix", string);

    delete cSuffixRegExp;
    cSuffixRegExp = suffix_regexp(getStrPref("cSuffix") );
}

bool is_c_suffix(char const *full_path)
{
    if(cSuffixRegExp == NULL)
	cSuffixRegExp = suffix_regexp(customize::getStrPref("cSuffix") );
    if (cSuffixRegExp->Match(full_path, strlen(full_path), 0) > 0)
        return true;
    return false;
}

//_______________________________________________________________________
char const* customize::c_esql_suffix()
{
    return(getStrPref("cEsqlSuffix"));
}

void customize::c_esql_suffix(char const* string)
{
    putPref("cEsqlSuffix", string);

    delete cEsqlSuffixRegExp;
    cEsqlSuffixRegExp = suffix_regexp(getStrPref("cEsqlSuffix") );
}

//_______________________________________________________________________
char const* customize::c_hdr_suffix()
{
    return(getStrPref("cHdrSuffix"));
}

void customize::c_hdr_suffix(char const* string)
{
    putPref("cHdrSuffix", string);
    delete cHeaderSuffixRegExp;
    cHeaderSuffixRegExp = suffix_regexp(c_hdr_suffix() );
}

bool is_c_hdr_suffix(char const *full_path)
{
    if (cHeaderSuffixRegExp == NULL) {
        cHeaderSuffixRegExp = suffix_regexp(
             customize::getStrPref("cHdrSuffix") );
        }
    if (cHeaderSuffixRegExp->Match(full_path, strlen(full_path), 0) > 0)
	return true;
    return false;
}


//_______________________________________________________________________
void customize::c_default_src_suffix(char const* value)
{
    putPref("cDfltSrcSuffix", value);
}

char const* customize::c_default_src_suffix()
{
    return(getStrPref("cDfltSrcSuffix"));
}


//_______________________________________________________________________
void customize::c_default_hdr_suffix(char const* pref)
{
    putPref("cDfltHdrSuffix", pref);
}

char const* customize::c_default_hdr_suffix()
{
    return(getStrPref("cDfltHdrSuffix"));
}


//_______________________________________________________________________
int customize::c_style()
{
    return (getIntPref("cStyle"));
}

void customize::c_style(int value)
{
    putPref("cStyle", value);
}


//_______________________________________________________________________
char const* customize::c_compiler()
{
    return(getStrPref("cCompiler"));
}

void customize::c_compiler(char const* pref)
{
    putPref("cCompiler", pref);
}


//_______________________________________________________________________
char const* customize::c_esql_pp()
{
    return(getStrPref("cEsqlPp"));
}

void customize::c_esql_pp(char const* pref)
{
    putPref("cEsqlPp", pref);
}


//_______________________________________________________________________
char const* customize::c_flags()
{
    return(getStrPref("cFlags"));
}

void customize::c_flags(char const* pref)
{
    putPref("cFlags", pref);
}


//_______________________________________________________________________
char const* customize::c_defines()
{
    return(getStrPref("cDefines"));
}

void customize::c_defines(char const* pref)
{
    putPref("cDefines", pref);
}


//_______________________________________________________________________
char const* customize::c_esql_args()
{
    return(getStrPref("cEsqlArgs"));
}

void customize::c_esql_args(char const* pref)
{
    putPref("cEsqlArgs", pref);
}


//_______________________________________________________________________
char const* customize::c_includes()
{
    return(getStrPref("cIncludes"));
}

void customize::c_includes(char const* pref)
{
    putPref("cIncludes", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_suffix()
{
    return(getStrPref("cPlusSuffix"));
}

void customize::cplusplus_suffix(char const* pref)
{
    putPref("cPlusSuffix", pref);

    delete cPlusSuffixRegExp;
    cPlusSuffixRegExp = suffix_regexp(getStrPref("cPlusSuffix") );
}

bool is_cplusplus_suffix(char const *full_path)
{
    if(cPlusSuffixRegExp == NULL)
	cPlusSuffixRegExp = suffix_regexp(customize_getStrPref("cPlusSuffix"));
    if (cPlusSuffixRegExp && cPlusSuffixRegExp->Match(full_path, strlen(full_path), 0) > 0)
        return true;
    return false;
}

//_______________________________________________________________________

bool is_els_non_ascii_suffix (char const *full_path)
{
    bool res = false;
    if (elsNonAsciiSuffixRegExp == NULL)
        elsNonAsciiSuffixRegExp = suffix_regexp(customize::getStrPref("ELS.nonAscii.Suffix"));
    if (elsNonAsciiSuffixRegExp && elsNonAsciiSuffixRegExp->Match(full_path, strlen(full_path), 0) > 0)
	res = true;
    return res;
}

//_______________________________________________________________________
char const* customize::cplusplus_esql_suffix()
{
    return(getStrPref("cPlusEsqlSuffix"));
}

void customize::cplusplus_esql_suffix(char const* pref)
{
    putPref("cPlusEsqlSuffix", pref);

    delete cPlusEsqlSuffixRegExp;
    cPlusEsqlSuffixRegExp = suffix_regexp(getStrPref("cPlusEsqlSuffix") );
}

//_______________________________________________________________________
char const* customize::cplusplus_hdr_suffix()
{
    return(getStrPref("cPlusHdrSuffix"));
}

void customize::cplusplus_hdr_suffix(char const* pref)
{
    putPref("cPlusHdrSuffix", pref);

    delete cPlusHeaderSuffixRegExp;
    cPlusHeaderSuffixRegExp = suffix_regexp(
        customize::getStrPref("cPlusHdrSuffix"));
}

bool is_cplusplus_hdr_suffix(char const *full_path)
{
    if (cPlusHeaderSuffixRegExp == NULL)
        cPlusHeaderSuffixRegExp = suffix_regexp(customize::getStrPref("cPlusHdrSuffix"));
    if (cPlusHeaderSuffixRegExp && cPlusHeaderSuffixRegExp->Match(full_path, strlen(full_path), 0) > 0)
	return true;
    return false;
}

//_______________________________________________________________________
void customize::cplusplus_default_src_suffix(char const* pref)
{
    putPref("cPlusDfltSrcSuffix", pref);
}

char const* customize::cplusplus_default_src_suffix()
{
    return(getStrPref("cPlusDfltSrcSuffix"));
}


//_______________________________________________________________________
void customize::cplusplus_default_hdr_suffix(char const* pref)
{
    putPref("cPlusDfltHdrSuffix", pref);
}

char const* customize::cplusplus_default_hdr_suffix()
{
    return(getStrPref("cPlusDfltHdrSuffix"));
}


//_______________________________________________________________________
char const* customize::cplusplus_flags()
{
    return(getStrPref("cPlusFlags"));
}

void customize::cplusplus_flags(char const* pref)
{
    putPref("cPlusFlags", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_compiler()
{
    return(getStrPref("cPlusCompiler"));
}

void customize::cplusplus_compiler(char const* pref)
{
    putPref("cPlusCompiler", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_esql_pp()
{
    return(getStrPref("cPlusEsqlPp"));
}

void customize::cplusplus_esql_pp(char const* pref)
{
    putPref("cPlusEsqlPp", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_defines()
{
    return(getStrPref("cPlusDefines"));
}

void customize::cplusplus_defines(char const* pref)
{
    putPref("cPlusDefines", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_esql_args()
{
    return(getStrPref("cPlusEsqlArgs"));
}

void customize::cplusplus_esql_args(char const* pref)
{
    putPref("cPlusEsqlArgs", pref);
}


//_______________________________________________________________________
char const* customize::cplusplus_includes()
{
    return(getStrPref("cPlusIncludes"));
}

void customize::cplusplus_includes(char const* pref)
{
    putPref("cPlusIncludes", pref);
}

//_______________________________________________________________________
char const* customize::editor()
{
    return(getStrPref("DIS_Editor.EditorPath"));
}

void customize::editor(char const* pref)
{
    putPref("DIS_Editor.EditorPath", pref);
}


//_______________________________________________________________________
char const* customize::paraDoc_name()
{
    return(getStrPref("paraDocName"));
}

void customize::paraDoc_name(char const* pref)
{
    putPref("paraDocName", pref);
}

//_______________________________________________________________________
char const* customize::paraDoc_commandLine()
{
    return(getStrPref("paraDocCommandLine"));
}

void customize::paraDoc_commandLine(char const* pref)
{
    putPref("paraDocCommandLine", pref);
}

//_______________________________________________________________________
int customize::editorMode()
{
    return(getIntPref("steEditMode"));
}

void customize::editorMode(int value)
{
    // Update Database.
    putPref("steEditMode", value);

    // Update STE. THIS SHOULD BE MOVED INTO THE STE CODE.
    // XXX: already in stubs:    ste_set_preferences(STE_VI_DEFAULT, value);
}


//_______________________________________________________________________
int customize::handle_language_enabled(unsigned int lang, int set = 0, int flag = 0)
//
// Which languages are enabled.
//
{
    static unsigned long aset_enabled_languages = (
	(1 << smt_C)         |
	(1 << smt_CPLUSPLUS) |
	(1 << smt_FORTRAN)   |
	(1 << FILE_LANGUAGE_ELS)   |
	(1 << smt_COBOL)     |
        (1 << smt_ESQL_C)    |
        (1 << smt_ESQL_CPLUSPLUS) |
        (1 << smt_ELS));

    if (set)
    {
	if (flag)
	    aset_enabled_languages |= (1 << lang);
	else
	    aset_enabled_languages &= ~(1 << lang);
	return 0;		// caller won't use this value, but buildxref wants it
    }
    else
	return (1 << lang) & aset_enabled_languages;
}


int customize::language_enabled(unsigned int lang)
{
   return handle_language_enabled(lang, 0, 0);
}

void customize::language_enabled(unsigned int lang, int flag)
{
    // lang should really be smtLanguage.

    handle_language_enabled(lang, 1, flag);
}


//_______________________________________________________________________
char const* els_guess_language(char const*fn);

fileLanguage guess_file_language(char const* name)
{
    Initialize(guess_file_language);

    const int length = strlen(name);

    if (customize::language_enabled(smt_CPLUSPLUS))
    {
        if (cPlusSuffixRegExp == NULL)
            cPlusSuffixRegExp = suffix_regexp(
                customize::getStrPref("cPlusSuffix") );
	if (cPlusSuffixRegExp->Match(name, length, 0) > 0)
	    return FILE_LANGUAGE_CPP;
        if (cPlusEsqlSuffixRegExp == NULL)
            cPlusEsqlSuffixRegExp = suffix_regexp(
                customize::getStrPref("cPlusEsqlSuffix") );
	if (cPlusEsqlSuffixRegExp->Match(name, length, 0) > 0)
	    return FILE_LANGUAGE_ESQL_CPP;
    }
    if (customize::language_enabled(smt_C)) {
        if (cSuffixRegExp == NULL)
            cSuffixRegExp = suffix_regexp(
                      customize::getStrPref("cSuffix") );
	if (cSuffixRegExp->Match(name, length, 0) > 0)
	    return FILE_LANGUAGE_C;
        if (cEsqlSuffixRegExp == NULL)
            cEsqlSuffixRegExp = suffix_regexp(
                      customize::getStrPref("cEsqlSuffix") );
	if (cEsqlSuffixRegExp->Match(name, length, 0) > 0)
	    return FILE_LANGUAGE_ESQL_C;
    }

    char const* els = els_guess_language(name);
    if(els)
      return FILE_LANGUAGE_ELS;

    return FILE_LANGUAGE_UNKNOWN;
}


//_______________________________________________________________________
char const* customize::makef_dir()
{
    return(getStrPref("makefileDir"));
}

void customize::makef_dir(char const* pref)
{
    putPref("makefileDir", pref);
}


//_______________________________________________________________________
char const* customize::makefile_targets()
{
    return (getStrPref("makefileTargets", EXPAND_COLONS) );
}


//_______________________________________________________________________
void customize::makefile_targets(char const* targets)
{
    putPref("makefileTargets", targets, UNEXPAND_COLONS);
}


//_______________________________________________________________________
char const* customize::home()
{
    return(getStrPref("DIS_env.Home"));
}

void customize::home(char const* newValue)
{
    putPref("DIS_env.Home", newValue);
}


//_______________________________________________________________________
char const* customize::user()
{
    return (getStrPref("DIS_env.UserLoginName"));
}

void customize::user(char const* newValue)
{
    putPref("DIS_env.UserLoginName", newValue);
}



//_______________________________________________________________________
static char const *handle_user_real_name(int set = 0, char const *string = NULL)
{
    static genString aset_user_real_name("");
    if(strcmp((char*)aset_user_real_name, "") == 0){
      genString exe;
      exe.printf("%s/bin/realname", customize::install_root());
      genTmpfile tmpfile;
      genString tmp_name = tmpfile.name();
      vsysteml_redirect (tmp_name, exe, NULL);

      FILE* fp = OSapi_fopen((char *)tmp_name,"r");
      int i;
      char buffer[513];
      while ((i = OSapi_fread(buffer,sizeof(char),512,fp))> 0)
	{
	  buffer[i] = '\0';
	  aset_user_real_name += buffer;
	}
      
      OSapi_fclose(fp);
      OSapi_unlink(tmp_name);
      if (set)
	aset_user_real_name = string;
      else if (!aset_user_real_name.length())
	aset_user_real_name = "";
    }
    return aset_user_real_name;
}

char const* customize::user_real_name()
{
    return ::handle_user_real_name();
}

void customize::user_real_name(char const* string)
{
    ::handle_user_real_name(1, string);
}



//_______________________________________________________________________
char const* customize::date()
{
    time_t tp;
    if(OSapi_time(&tp) > 0)
	return OSapi_ctime(&tp);
    return NULL;
}


//_______________________________________________________________________
char const* customize::host()
{
    return (getStrPref("DIS_env.Host"));
}

void customize::host(char const* newValue)
{
    putPref("DIS_env.Host", newValue);
}


//_______________________________________________________________________
int customize::rtl_file_stats()
{
    return (getIntPref("rtlFileStat"));
}

void customize::rtl_file_stats(int value)
{
    putPref("rtlFileStat", value);
}

int customize::forgive()
{
    int ret_val = 0;

    char const * ase = getStrPref("acceptSyntaxErrors");
    if (ase) {
	if (!OSapi_strcasecmp(ase, "yes"))
	    ret_val = 1;
    }

    return ret_val;
}

void customize::forgive (int frg)
{
    if (frg > 0)
	putPref("acceptSyntaxErrors", "yes");
    else
	putPref("acceptSyntaxErrors", "no");
}

bool is_forgiving()
{
    return customize::forgive();
}

int customize::miniBrowser_history_length()
{
    return (getIntPref("miniBrowserHistory"));
}

void customize::miniBrowser_history_length(int len)
{
    putPref("miniBrowserHistory", len);
}

int customize::miniBrowser_show_titles()
{
    return (getIntPref("miniBrowserTitles"));
}

void customize::miniBrowser_show_titles(int flag)
{
    putPref("miniBrowserTitles", flag);
}

//_______________________________________________________________________
char const* customize::printer()
{
    return (getStrPref("DIS_env.Printer"));
}

void customize::printer(char const* newValue)
{
    putPref("DIS_env.Printer", newValue);
}


//_______________________________________________________________________
char const* customize::c_parser()
{
    return (getStrPref("DIS_c.C_Parser"));
}

void customize::c_parser(char const* newValue)
{
    putPref("DIS_c.C_Parser", newValue);
}


//_______________________________________________________________________
char const* customize::cp_parser()
{
    return (getStrPref("DIS_cp.Cp_Parser"));
}

void customize::cp_parser(char const* newValue)
{
    putPref("DIS_cp.Cp_Parser", newValue);
}


//_______________________________________________________________________
char const* customize::ste_buttonbar()
{
    return("");
}

void customize::ste_buttonbar(char const* value)
{
}


//_______________________________________________________________________
char const* customize::smt_buttonbar()
{
    return(getStrPref("buttonbarSMT"));
}

void customize::smt_buttonbar(char const* pref)
{
   putPref("buttonbarSMT", pref);
}


//_______________________________________________________________________
char const* customize::calltree_buttonbar()
{
    return(getStrPref("buttonbarCalltree"));
}

void customize::calltree_buttonbar(char const* pref)
{
    putPref("buttonbarCalltree", pref);
}


//_______________________________________________________________________
char const* customize::class_buttonbar()
{
    return(getStrPref("buttonbarClass"));
}

void customize::class_buttonbar(char const* pref)
{
    putPref("buttonbarClass", pref);
}


//_______________________________________________________________________
char const* customize::erd_buttonbar()
{
    return(getStrPref("buttonbarERD"));
}

void customize::erd_buttonbar(char const* pref)
{
    putPref("buttonbarERD", pref);
}



//_______________________________________________________________________
char const* customize::subsysBrowser_buttonbar()
{
    return(getStrPref("buttonbarSSBrowser"));
}

void customize::subsysBrowser_buttonbar(char const* pref)
{
    putPref("buttonbarSSBrowser", pref);

}


//_______________________________________________________________________
char const* customize::subsysMap_buttonbar()
{
    return(getStrPref("buttonbarSSMap"));
}

void customize::subsysMap_buttonbar(char const* pref)
{
    putPref("buttonbarSSMap", pref);
}


//_______________________________________________________________________
char const* customize::dc_buttonbar()
{
    return(getStrPref("buttonbarDatachart"));
}

void customize::dc_buttonbar(char const* pref)
{
    putPref("buttonbarDatachart", pref);
}


//_______________________________________________________________________
int customize::button_bar_visible()
{
    return (getIntPref("buttonbarVisible"));
}

void customize::button_bar_visible(int flag)
{
  putPref("buttonbarVisible", flag);  
}





// Functions to set and read boilerplate defaults

//_______________________________________________________________________
char const* customize::func_init()
{
    return(getStrPref("func-init"));
}

void customize::func_init(char const* pref)
{
    putPref("func-init", pref);
}

//_______________________________________________________________________
char const* customize::func_final()
{
    return(getStrPref("func-final"));
}

void customize::func_final(char const* pref)
{
     putPref("func-final", pref);
}

//_______________________________________________________________________
char const* customize::before_func_decl()
{
    return(getStrPref("before-func-decl"));
}

void customize::before_func_decl(char const* pref)
{
    putPref("before-func-decl", pref);
}


//_______________________________________________________________________
char const* customize::after_func_decl()
{
    return(getStrPref("after-func-decl"));
}

void customize::after_func_decl(char const* pref)
{
    putPref("after-func-decl", pref);
}


//_______________________________________________________________________
char const* customize::before_func_def()
{
    return(getStrPref("before-func-def"));
}

void customize::before_func_def(char const* pref)
{
    putPref("before-func-def", pref);
}


//_______________________________________________________________________
char const* customize::after_func_def()
{
    return(getStrPref("after-func-def"));
}

void customize::after_func_def(char const* pref)
{
    putPref("after-func-def", pref);
}


//_______________________________________________________________________
char const* customize::pub_decl()
{
    return(getStrPref("pub-decl"));
}

void customize::pub_decl(char const* pref)
{
    putPref("pub-decl", pref);
}


//_______________________________________________________________________
char const* customize::prot_decl()
{
    return(getStrPref("prot-decl"));
}

void customize::prot_decl(char const* pref)
{
    putPref("prot-decl", pref);
}


//_______________________________________________________________________
char const* customize::priv_decl()
{
    return(getStrPref("priv-decl"));
}

void customize::priv_decl(char const* pref)
{
    putPref("priv-decl", pref);
}


//_______________________________________________________________________
char const* customize::before_class_decl()
{
    return(getStrPref("before-class-decl"));
}

void customize::before_class_decl(char const* pref)
{
    putPref("before-class-decl", pref);
}


//_______________________________________________________________________
char const* customize::after_class_decl()
{
    return(getStrPref("after-class-decl"));
}

void customize::after_class_decl(char const* pref)
{
    putPref("after-class-decl", pref);
}


//_______________________________________________________________________
char const* customize::before_class_def()
{
    return(getStrPref("before-class-def"));
}

void customize::before_class_def(char const* pref)
{
    putPref("before-class-def", pref);
}


//_______________________________________________________________________
char const* customize::after_class_def()
{
    return(getStrPref("after-class-def"));    
}

void customize::after_class_def(char const* pref)
{
    putPref("after-class-def", pref);
}

//_______________________________________________________________________
char const* customize::before_struct()
{
    return(getStrPref("before-struct"));
}

void customize::before_struct(char const* pref)
{
    putPref("before-struct", pref);
}

//_______________________________________________________________________
char const* customize::after_struct()
{
    return(getStrPref("after-struct"));
}

void customize::after_struct(char const* pref)
{
    putPref("after-struct", pref);
}

//_______________________________________________________________________
char const* customize::member_def_init()
{
    return(getStrPref("member-def-init"));
}

void customize::member_def_init(char const* pref)
{
    putPref("member-def-init", pref);
}

//_______________________________________________________________________
char const* customize::member_def_final()
{
    return(getStrPref("member-def-final"));
}

void customize::member_def_final(char const* pref)
{
    putPref("member-def-final", pref);
}


//_______________________________________________________________________
char const* customize::before_member_def()
{
    return(getStrPref("before-member-def"));
}

void customize::before_member_def(char const* pref)
{
    putPref("before-member-def", pref);
}

//_______________________________________________________________________
char const* customize::after_member_def()
{
    return(getStrPref("after-member-def"));
}

void customize::after_member_def(char const* pref)
{
    putPref("after-member-def", pref);
}

//_______________________________________________________________________
char const* customize::before_member_decl()
{
    return(getStrPref("before-member-decl"));
}

void customize::before_member_decl(char const* pref)
{
    putPref("before-member-decl", pref);
}

//_______________________________________________________________________
char const* customize::after_member_decl()
{
    return(getStrPref("after-member-decl"));
}

void customize::after_member_decl(char const* pref)
{
    putPref("after-member-decl", pref);
}

//_______________________________________________________________________
char const* customize::relation_src_member()
{
    return(getStrPref("relation-src-member"));
}

void customize::relation_src_member(char const* pref)
{
    putPref("relation-src-member", pref);
}

//_______________________________________________________________________
char const* customize::relation_src_header()
{
    return(getStrPref("relation-src-header"));
}

void customize::relation_src_header(char const* pref)
{
    putPref("relation-src-header", pref);
}


//_______________________________________________________________________
char const* customize::relation_src_definition()
{
    return(getStrPref("relation-src-definition"));
}

void customize::relation_src_definition(char const* pref)
{
    putPref("relation-src-definition", pref);
}

//_______________________________________________________________________
char const* customize::relation_trg_member()
{
    return(getStrPref("relation-trg-member"));
}

void customize::relation_trg_member(char const* pref)
{
    putPref("relation-trg-member", pref);
}

//_______________________________________________________________________
char const* customize::relation_trg_header()
{
    return(getStrPref("relation-trg-header"));
}

void customize::relation_trg_header(char const* pref)
{
    putPref("relation-trg-header", pref);
}


//_______________________________________________________________________
char const* customize::relation_trg_definition()
{
    return(getStrPref("relation-trg-definition"));
}

void customize::relation_trg_definition(char const* pref)
{
    putPref("relation-trg-definition", pref);
}

//_______________________________________________________________________
char const* customize::c_src_file_top()
{
    return(getStrPref("c-src-file-top"));
}

void customize::c_src_file_top(char const* pref)
{
    putPref("c-src-file-top", pref);
}

//_______________________________________________________________________
char const* customize::c_src_file_bottom()
{
    return(getStrPref("c-src-file-bottom"));
}

void customize::c_src_file_bottom(char const* pref)
{
    putPref("c-src-file-bottom", pref);
}

//_______________________________________________________________________
char const* customize::c_hdr_file_top()
{
    return(getStrPref("c-hdr-file-top"));
}

void customize::c_hdr_file_top(char const* pref)
{
    putPref("c-hdr-file-top", pref);
}

//_______________________________________________________________________
char const* customize::c_hdr_file_bottom()
{
    return(getStrPref("c-hdr-file-bottom"));
}

void customize::c_hdr_file_bottom(char const* pref)
{
    putPref("c-hdr-file-bottom", pref);
}

//_______________________________________________________________________
char const* customize::cpp_src_file_top()
{
    return(getStrPref("cpp-src-file-top"));
}

void customize::cpp_src_file_top(char const* pref)
{
    putPref("cpp-src-file-top", pref);
}

//_______________________________________________________________________
char const* customize::cpp_src_file_bottom()
{
    return(getStrPref("cpp-src-file-bottom"));
}

void customize::cpp_src_file_bottom(char const* pref)
{
    putPref("cpp-src-file-bottom", pref);
}

//_______________________________________________________________________
char const* customize::cpp_hdr_file_top()
{
    return(getStrPref("cpp-hdr-file-top"));
}

void customize::cpp_hdr_file_top(char const* pref)
{
    putPref("cpp-hdr-file-top", pref);
}

//_______________________________________________________________________
char const* customize::cpp_hdr_file_bottom()
{
    return(getStrPref("cpp-hdr-file-bottom"));
}

void customize::cpp_hdr_file_bottom(char const* pref)
{
    putPref("cpp-hdr-file-bottom", pref);
}

////////////////////////////////////////////////////////////////////

const int  customize_get_write_flag()
{
    return customize::get_write_flag ();
}

char const* customize_default_project_definition()
{
    return customize::default_project_definition ();
}

char const* customize_install_root()
{
   return customize::install_root () ;
}

char const* customize_ste_suffix()
{
    return ("");
}

void elsLanguage::print(ostream& st, int) const
{
  st << "els language " << get_name() << " flags " << flags.str() << " suff " << suffixes.str() << endl;
}
void elsSuffix::print(ostream& st, int) const
{
  st << "els suffix " << get_name() << " language " << els->get_name() << endl;
}

static objArr suffixes;
static objArr languages;

const objArr& elsLanguage::list()
{
  return languages;
}

const objArr& elsSuffix::list()
{
  return suffixes;
}

char const* els_flags(char const* lan)
{
  elsLanguage* els = (elsLanguage*) obj_search_by_name(lan, languages);
  if(!els)
    return NULL;

  return  els->flags;
}

const elsLanguage* els_get_language(char const*fn)
{
  int sz = suffixes.size();
  int len  = strlen(fn);
  char const*end = fn + len;
  for(int ii=0; ii<sz; ++ii){
    elsSuffix* sf = (elsSuffix*) suffixes[ii];
    char const* suff = sf->get_name();
    char const * tail = end - sf->len;
    if(strcmp(suff, tail)==0)
      return sf->els;
  }
  return NULL;
}

char const* els_guess_language(char const* fn)
{
  const elsLanguage* lang = els_get_language(fn);
  if (lang)
    return lang->get_name();
  else
    return NULL;
}

static int els_tokenize(char const* str, void(*cb)(char const*, elsLanguage*), elsLanguage*els)
{
  int token_count = 0;
  char* st = (char*) str;
  for(;;){
    while(*st == ' ' || *st == '\t')
      ++st;

    if(*st == '\0')
      break;

    char *en = st+1;
    int ch;
    while ((ch = *en) != ' ' && ch != '\t' && ch != '\0')
      ++en;

    *en = '\0';

    (*cb)(st, els);
    ++ token_count;

    if(ch) {
      *en = ch;
      st = en + 1;
    } else {
      break;
    }
  }
  return token_count;
}

static void report_language(char const* nm, elsLanguage*)
{
  elsLanguage*els = new elsLanguage(nm);
  genString pref;
  pref.printf("ELS.%s.Suffix", nm);
  els->suffixes = customize::getStrPref(pref);
  pref.printf("ELS.%s.Flags", nm);
  els->flags = customize::getStrPref(pref);
  pref.printf("ELS.%s.FileIcon", nm);
  els->fileIconCode = customize::getIntPref(pref);
  languages.insert_last(els);
}

static void report_suffix(char const* suff, elsLanguage*els)
{
  elsSuffix* sf = (elsSuffix*) obj_search_by_name(suff, suffixes);
  if(sf)
    return;

  sf = new elsSuffix(suff);
  sf->len = strlen(suff);
  sf->els = els;
  suffixes.insert_last(sf);
}

static void els_init()
{
  static bool done = false;
  if(done)
    return;

  done = true;

  char const* list = customize::getStrPref("ELS.List");
  if(!list)
    return;
  els_tokenize(list, report_language, NULL);
  int sz = languages.size();
  for(int ii=0; ii<sz; ++ii){
    elsLanguage* els = (elsLanguage*)languages[ii];
    char const*suff = els->suffixes;
    els_tokenize(suff, report_suffix, els);
  }  
//  node_prt(&languages);
//  node_prt(&suffixes);
}

void extern_els_init () { els_init(); }

char const* get_parser_output_filename();
static int elsCmd(ClientData cd, Tcl_Interp *interp, int argc, char const **argv)
{
  Interpreter *i = (Interpreter*) cd;
  if(argc<2) {
    ostream& os = i->GetOutputStream();
    os << "usage: {language|flags|tmpname} <file_name>" << endl;
    return TCL_ERROR;
  }
  els_init();

  char const * sub = argv[1];
  char const * fn = argv[2];
  char const * answer = 0;
  if(strcmp(sub, "language") == 0){
    answer = els_guess_language(fn);
  } else if(strcmp(sub, "flags") == 0){
    char const* lan = (fn[0]=='/') ? els_guess_language(fn) : fn;
    if(lan)
      answer = els_flags(lan);
  } else if(strcmp(sub, "tmpname") == 0){
    answer = OSPATH(get_parser_output_filename());
  } else {
    ostream& os = i->GetOutputStream();
    os << "usage: {language flags tmpname} <file_name>" << endl;
    return TCL_ERROR;
  }
  if (answer)
    Tcl_SetResult(i->interp, (char*) answer, TCL_VOLATILE);
  return TCL_OK;
}

int els_parse_file(const symbolPtr& mod)
{
  genString cmd;
  cmd.printf("dis_parse_els_file 0X%X", &mod);
  int code = cli_eval_string(cmd);
  return code;
}

// The next two functions were added to allow gala to display the output file.
static char *tfn, *delete_tfn;
void init_parser_output_filename()
{
    if (!tfn) {
        genString envname = customize::getStrPref("DIS_c.Cpp_LogFile");
        if (envname.length()) {
            tfn = OSapi_strdup((char *)envname);
        } else {
            tfn = OSapi_tempnam(0, "cpp_out");
            delete_tfn = tfn;
        }
        // Open and close the file to make sure it exists.
        // Gala bug - will not monitor the file if it isn't initially there.
        FILE* fd = OSapi_fopen(tfn, "w");
        OSapi_fclose(fd);
    }
}

char const* get_parser_output_filename()
{
    if (!tfn) init_parser_output_filename();
    return(tfn);
}
void smt_clear_cpp_out ()
{
    if (delete_tfn) {
        OSapi_unlink (delete_tfn);
        delete_tfn = 0;
    }
}

static int systemCmd (Interpreter*i, int argc, char const ** argv)
{
  if(argc==1) {
    return TCL_ERROR;
  }
  ocharstream os;
  for(int ii=1; ii<argc; ++ii){
    if(ii > 1)
      os << ' ';
    os << argv[ii];
  }
  os << ends;
  char const*cmd = os.ptr();

  int status = vsystem(cmd);
  
  char buf[9];
  sprintf(buf, "%d", status);
  Tcl_SetResult(i->interp, buf, TCL_VOLATILE);
  return TCL_OK;
}

static int bool_prefValueCmd(Interpreter *i, int argc, char const *argv[])
{
    if(argc != 2){
	Tcl_SetResult(i->interp, "get_prefValueCmd needs 1 argument", TCL_VOLATILE);
	return TCL_ERROR;
    }
    char const *key = argv[1];
    bool val = customize::getBoolPref(key);
    Tcl_SetResult(i->interp, (char *)(val?"1":"0"), TCL_VOLATILE);
    return TCL_OK;
}

static int add_commands()
{
    new cliCommandInfo("els", elsCmd);
    new cliCommand("system", systemCmd);
    new cliCommand("get_bool_pref", bool_prefValueCmd);
    return 0;
}


static int dummy = add_commands();

