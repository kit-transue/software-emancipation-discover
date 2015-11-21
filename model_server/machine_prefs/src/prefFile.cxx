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
// prefFile.cxx - Preference File Class methods.
//

// cLibraryFunctions must be first to define file i/o stuff for NT.
// Unfortunately, this result in required vbool to be used instead of bool.
#include <cLibraryFunctions.h>
#include <msg.h>
#include <prefFile.h>
#include <machdep.h>
#include <messages.h>
#include <evalMetaChars.h>
#include <genError.h>

#ifdef _WIN32
#define is_path_absolute(f) (*(f) == '/' || *(f) == '\\' || (f)[1] == ':')
#else
#define is_path_absolute(f) (*(f) == '/')
#endif

extern "C" int remove_all_dots_and_double_slashes(char const *, char*);

// Class static default sytem and user preference file names.
#if defined(WIN32)
const genString prefFile::DEFAULT_PREFERENCE_FILE =
             "$PSETHOME\\lib\\psetPrefs.default";
#else    // WIN32 not defined
const genString prefFile::DEFAULT_PREFERENCE_FILE =
             "$PSETHOME/lib/psetPrefs.default";
#endif    // WIN32 defined

// Class static Preference key prefix and suffix
const genString prefFile::PREF_KEY_PREFIX = "*psetPrefs.";
const genString prefFile::PREF_KEY_SUFFIX = ":";

#define MAXPATHLEN 1024

// Constructor.
prefFile::prefFile()
{
    baseName_ = NULL;
    fullName_ = NULL;
    prefPath_ = NULL;
    prefFD_ = NULL;
    prefText_ = NULL;
    prefText_l = NULL;
}

// Destructor.
prefFile::~prefFile()
{
    // Make sure any changes are saved.
    Close();
}

bool prefFile::Open(const char* name)
{
    Initialize(prefFile::Open);
    bool status = InitFile(name);
    return(status);
}


// Read methods.
bool prefFile::Read(const char* key, int& value)
{
  Initialize(prefFile::Read(const char* key, int& value));
  bool status = true;
  
  // Select the preference info.
  long pref_len = 0;
  char * pref_st = SelectPref(key, pref_len);

  if (pref_st) {
    if (pref_len == 0)
      value = 0;  //found a null value;
    else {        //pref_len > 0
      char * pref_val = new char[pref_len+1];
      OSapi_strncpy(pref_val, pref_st, pref_len);
      pref_val[pref_len] = '\0';
      
      // Convert to integer.
      value = OSapi_atoi(pref_val);
      delete pref_val;
    }
  }
  else
    status = false;
  
  return (status);
}

bool prefFile::Read(const char* key, genString& value)
{
  Initialize(prefFile::Read(const char* key, genString& value));
  bool status = true;
  
  // Select the preference info.
  long pref_len = 0;
  char *pref_st = SelectPref(key, pref_len);
  
  if (pref_st) {
    if (pref_len == 0) // found a null value
      value = "";
    else {
      char * pref_val = new char[pref_len+1];
      OSapi_strncpy(pref_val, pref_st, pref_len);
      pref_val[pref_len] = '\0';
      
      // Get the index to the end of the preference.
      long index = pref_len-1;
      
      // Remove the <LF> from the last line.
      if ( (index >= 0) && (pref_val[index] == 0x0A) ) 
	pref_val[index--] = 0x00;
      
      // If it is there, convert the <CR> into a space.
      if ((index >= 0) && (pref_val[index] == 0x0D) ) 
	pref_val[index--] = 0x00;
      
      // If it is there, remove the backslash from the last line.
      if ((index >= 0) && (pref_val[index] == '\\') )
	pref_val[index--] = 0x00;
      
      // Now, step backwards through the value, 
      // converting all newlines to spaces.
      while (index >= 0) {
	// Find the next <LF>.
	while ( (index >= 0) && (pref_val[index] != 0x0A /*newline*/) )
	  index--;
	if (index >= 0) {
	  // Convert the <LF> into a space.
	  pref_val[index--] = ' ';
	  
	  // If it is there, convert the <CR> into a space.
	  if ((index >= 0) && (pref_val[index] == 0x0D /*carriage return*/) ) 
	    pref_val[index--] = ' ';
	  
	  // Convert the backslash into a space.
	  if ((index >= 0) && (pref_val[index] == '\\'))
	    pref_val[index--] = ' ';
	}
      }
      
      // skip any white space at the start of the string.
      char* bufPtr = pref_val;
      while (isspace(*bufPtr)) bufPtr++;
      
      //trim the back end of whitespace
      long index2 = pref_len -1;
      while ((index2 >= 0) && isspace(pref_val[index2])) {
	pref_val[index2]= '\0';
	index2--;
      }
      if (bufPtr && *bufPtr) {
	// Create a genString with the value in it.
	value = bufPtr;
      }
      else
	value = "";

      if (pref_val) 
	delete pref_val;
    }
  }
  else
    status = false;
  
  return (status);
}



//---------------------- Private prefFile methods -----------------------



bool prefFile::InitFile(const char* name)
{
    Initialize(prefFile::InitFile);
    bool status = true;

    if (name && *name)
        InitBaseName(name);
    else
        InitBaseName(GetDefaultFileName());

    InitPath();

    if (prefPath_.is_null()) {
        status = false;
    } 
    else {
        status = CheckAccess();

        if (prefFD_ == NULL) {
            prefFD_ = OSapi_open(prefPath_, openMode_);
            if (prefText_.is_null()) InitText();
        }
    }

    return(status);
}


// InitBaseName - Initialize the base file name.
bool prefFile::InitBaseName(const char* name)
{
    Initialize(prefFile::InitBaseName);
    bool status = true;

    if (!fileExists(name)) {
        // File doesn't exist, create it.
        msg("The file \'$1\' was not found.", warning_sev) << name << eom;
        baseName_ = name;
        baseName_ += ".pr";
        if (fileExists(baseName_) )
            msg("Switching to \'$1\'.", warning_sev) << (char *)baseName_ << eom;
        else
            baseName_ = name;

    }
    else {
      // File exists and is OK.
      baseName_ = name;
    }

    return (status);
}




// InitPath - Initialize the file path.
bool prefFile::InitPath()
{
    Initialize(prefFile::InitPath);
    bool status = true;
    // Get the path and full path name.
    genString value;
    if (eval_shell_metachars(baseName_.str(), value)) {
      if (!is_path_absolute(value.str())) {
        // It is a relative path, try looking in the working dir.
	genString tmpPath = value;
	char wd[1024];
        value = OSapi_getwd(wd);
	value += '/';
        value += tmpPath;
      }
      char canonical_name[4*MAXPATHLEN];
      if (remove_all_dots_and_double_slashes((char *)value, canonical_name) == 0)
	prefPath_ = canonical_name;
      else
	prefPath_ = value;
      fullName_ = prefPath_;
    }
    return (status);
}


// InitText - Read in text from preference file.
bool prefFile::InitText()
{
  Initialize(prefFile::InitText);
  bool status = (prefFD_ != NULL);
  
  if (status) status = prefText_.is_null();
  
  if (status) {
    char buf[1024];
    int num_read = 0;
    num_read = OSapi_read(prefFD_, buf, sizeof(char)*1023);
    while (num_read > 0 && num_read <= 1023) {
      buf[num_read] = '\0';
      prefText_ += buf;
      num_read = OSapi_read(prefFD_, buf, sizeof(char)*1023);
    }
    unsigned int len = prefText_.length();
    // if the file does not have new line at the end, add it
    if (len == 0 || prefText_[len -1] != '\n')
      prefText_ += '\n';
  }
  if (status)
    prefText_is_changed();

  return(status);
}

void prefFile::prefText_is_changed()
{
  Initialize(prefFile::prefText_is_changed);
  // prefText_l is the "tolower" version of prefText_, that is used 
  // in doing case in-sensitive search for preference pattern
  prefText_l = prefText_;
  char * str = (char *)prefText_l;
  while(*str){
    *str = tolower(*str);
    str++;
  }
  
  return;
}


// Close - Close the given preference file.
bool prefFile::Close()
{
  Initialize(prefFile::Close);
  bool status = true;
  
  if (prefFD_) 
    OSapi_close(prefFD_);
  
  return(status);
}

// CheckAccess - Checks the preference access mode.
bool prefFile::CheckAccess()
{
  Initialize(prefFile::CheckAccess);
  bool status = true;
  //rewriting this so that it works with with HP
  // Make sure that the file exists.
  if (OSapi_access(prefPath_, F_OK) != 0 ) {
    int fd;
    fd = OSapi_open(prefPath_, O_RDWR|O_CREAT, 0644);
    if (fd == -1) {
      status = false;
      msg("Problem creating preference file \'$1\'", error_sev) << (char *)prefPath_ << eom;
    }
    else
      fd = OSapi_close(fd);
  }
  if (status) {
    if (OSapi_access(prefPath_, R_OK) == 0)
      {
	openMode_= O_RDONLY;
      }
    else
      {
	status=false;
	openMode_=NULL;
	msg("Unable to read from preference file $1", error_sev) << prefPath_.str() << eom;
      }
  }
  
  return status;
}

// looks for pattern in prefText_l ( the "tolower" version of prefText_ ),
// returns the offset where the pattern occurs,
// check if it is a comment line
long prefFile::searchPref(char * ptn, long st_offset)
{
  Initialize(prefFile::searchPref);
  long ret_val = -1;
  if (ptn) {
    // convert the search pattern to lower case
    genString new_ptn = ptn;
    char * str = (char *)new_ptn;
    while(*str){
      *str = tolower(*str);
      str++;
    }
    if ((char *)new_ptn && st_offset >= 0) {
      char * loc = NULL;
      char * start_ptr = (char*)prefText_l + st_offset;
      if (start_ptr) {
        int plen = OSapi_strlen(new_ptn);
        loc = strstr(start_ptr, new_ptn);
        //skip all the pref matches that are commented out
        while (loc && PrefLineIsComment(loc) <= 0) { 
	  loc += plen; 
	  loc =  strstr(loc, new_ptn);
        }
        // get the offset where the pattern occurs
        if (loc)
	  ret_val = loc - (char*)prefText_l;
      }
    }
  }
  return ret_val;
}
  
// This function returns pointer to the first char of the preference value 
// if it exists, or null if it does not. The length of the value is stored in
// val_len
char* prefFile::SelectPref(const char* prefName, long& val_len)
{
  Initialize(prefFile::SelectPref);
  char * val_ptr = NULL;
  
  // Add the key prefix and suffix to make sure it is unique within the file.
  genString prefTarget = prefFile::PREF_KEY_PREFIX;
  prefTarget += prefName;
  prefTarget += prefFile::PREF_KEY_SUFFIX;
  int targ_len = prefTarget.length();
  
  // Find the pref beginning ( skip commented ones)
  long pref_offset = searchPref((char *)prefTarget, 0);
  if (pref_offset >= 0)
    val_ptr = (char *)prefText_ + pref_offset;

  if (val_ptr) {
    // Adjust the start position to skip the matched preference keyname
    val_ptr += targ_len;
    val_len = PrefLineGetEnd(val_ptr);
    if (val_len >= 0) {
      long p_off = searchPref((char*)prefTarget, pref_offset + targ_len);
      if (p_off >= 0)
	msg("The file \"$1\" defines multiple values for preference \"$2\". Using first definition.", warning_sev) << (char*)fullName_ << eoarg << prefName << eom;
    }
    else 
      val_ptr = NULL;
  }
  
  return(val_ptr);
}


// get till the end of the preference and return the length of the preference
long prefFile::PrefLineGetEnd(const char * str)
{
  Initialize(prefFile::PrefLineGetEnd);
  bool status = true;
  char *ptr = (char *)str;
  bool not_end = false;
  long ret_val = -1;

  do {
    
    //go forward till end of line
    while (ptr && *ptr != 0x0A) 
      ptr++;
    
    //check if the line is continued
    if (ptr) {
      char * nptr = ptr -1;
      if (*nptr == 0x0D)
	nptr--;
      if (*nptr == '\\') { // line has a back slash, go to next line
	ptr++;
	not_end = true;
      }
      else
	not_end = false;
    }
    else
      status = false;
  } 
  while (not_end);
  
  if (status)
    ret_val = ptr - str;
  
  return ret_val;
}


// returns:
//          0 : if pref line is a comment
//          1 : if pref line is not a comment
//         -1 : error, line begins neither with '#' nor '*'
int prefFile::PrefLineIsComment(const char * str)
{
  Initialize(prefFile::PrefLineIsComment);
  int status = -1;
  char * ptr = (char *)str;
  char * bof = (char *)prefText_l;

  //traverse back till the beginning of the line
  while ( ptr && (ptr != bof) && (*ptr != 0x0A))
    ptr--;
  
  //traverse to the first character that is not a space
  while (ptr && isspace(*ptr))
    ptr++;
  
  if (ptr) {
    if (*ptr == '#')
      status = 0;     //if the first character is "#", this is a comment line
    else if ( ptr == str)
      status = 1;     // this line is a good pref line
  }
  
  return (status);
}





//////////     Preference Conversion Routines.     //////////


// See if preference file is in the current version.
bool prefFile::fileExists(const char* filename)
{
  Initialize(prefFile::fileExists);
  bool foundIt = false;
  
  // Make sure a file name was given.
  if (filename) {
    genString value;
    if (eval_shell_metachars(filename, value)) {
      // See if the file exists.
      if (OSapi_access(value.str(), F_OK) == 0)
	foundIt = true;
    }
  }
  return(foundIt);
}

