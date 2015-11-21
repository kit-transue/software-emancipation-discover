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
// userPrefFile.cxx - User Preference File Class methods.
//

#include <cLibraryFunctions.h>
#include <msg.h>
#include <userPrefFile.h>
#include <machdep.h>
#include <genError.h>

// Class static default sytem and user preference file names.
#if defined(WIN32)
const genString userPrefFile::DEFAULT_PREFERENCE_FILE = "\\$HOME\\.psetPrefs";
#else    // WIN32 not defined
const genString userPrefFile::DEFAULT_PREFERENCE_FILE = "$HOME/.psetPrefs";
#endif    // WIN32 defined



// Constructor.
userPrefFile::userPrefFile() { }

//open methods.  These simply check to see if we have write permissions on the file.

bool userPrefFile::Open(const char* name)
{
  Initialize(userPrefFile::Open);
  bool status=prefFile::Open(name);
  if (status)
    {
      const char * pref_file = prefPath_.str();
      if ( OSapi_access(pref_file, W_OK) ) {
	msg("Preference file \'$1\' is write protected. Any changes made will not be saved.", warning_sev) << (char *)fullName_ << eom;
	status = false;
      }
    } 
  else
    status = false;
  
  return status;
}

		
// Write methods.
//    There are two: 
//        char* key and int value,
//        char* key and char* value.
bool userPrefFile::Write(const char* key, const int value)
{
  bool status = true;
  
  // Convert integer value into string.
  genString intBuffer;
  intBuffer.printf("    %d\n", value);
  
  status = write_value(key, intBuffer);
  return (status);
}

bool userPrefFile::Write(const char* key, const char* value)
{
  bool status = true;
  
  // Prepend a tab to the string.
  genString strBuffer;
  strBuffer.printf("    %s\n", value);

  status = write_value(key, strBuffer);
  
  return status;
}

bool userPrefFile::write_value(const char* key, genString& value)
{
  Initialize(userPrefFile::write_value);
  bool status = true;
  
  // Select the preference info.
  long pref_len;
  char * pref_st = SelectPref(key, pref_len);

  if (pref_st) {
    // Preference was already there, change the value.
    char * start = (char *)prefText_;

    // get the length before the preference occurs
    long len_before = pref_st - start;
    char *buf = new char[len_before+1];
    OSapi_strncpy(buf, start, len_before);
    buf[len_before] = '\0';
    
    //copy all the stuff from the beginning to the start of preference value
    genString new_text = buf;
    // copy the new preference value
    new_text += value;
    // copy the stuff after the preference value to the end
    new_text += (start+len_before+pref_len);
    
    prefText_ = new_text;
    delete buf;
  } else {
    genString new_line =  PREF_KEY_PREFIX;
    new_line += key;
    new_line += PREF_KEY_SUFFIX;
    new_line += value;
    
    // Insert the new line.
    prefText_ += new_line;
  }
  
  if (status)
    prefText_is_changed();

  // Force storage of the change.
  status = Flush();
  
  return (status);
}



//---------------------- Private userPrefFile methods -----------------------


// Flush - Force output of the preference file to disk.
bool userPrefFile::Flush()
{
  Initialize(userPrefFile::Flush);
  bool status = (prefFD_ != NULL);
  
  if (status) {
    // Make sure that we have write privelages before continuing.
    if ( OSapi_access(prefPath_, W_OK) ) {
      status = false;
    }
  } 
  else
    status = false;
  
  if (status) {
    long length = prefText_.length();
    OSapi_lseek(prefFD_, 0, 0);
    if (OSapi_write(prefFD_, prefText_, length) < length) {
      status = false;
      msg("Problem writing to preference file \'$1\'", error_sev) << (char *)fullName_ << eom;
    }
  }
  
  return (status);
}



// CheckAccess - Checks the preference access mode.
bool userPrefFile::CheckAccess()
{
  Initialize(userPrefFile::CheckAccess);
  bool status = true;
  
  // Make sure that the file exists.
  if (OSapi_access(prefPath_, F_OK) != 0) {
    int createMode = 0644;
    
    int fd =  OSapi_open(prefPath_, O_RDWR|O_CREAT, createMode);
    if (fd == -1) {
      status = false;
      msg("Problem creating preference file \'$1\'", error_sev) << (char *)fullName_ << eom;
    }
    else
      fd = OSapi_close(fd);
  }
  
  if (status) {
    openMode_ = NULL;
    status    = false;

    if (!(OSapi_access(prefPath_, R_OK|W_OK))) {
      openMode_ = O_RDWR;
      status    = true;
    } else if ( !(OSapi_access(prefPath_, R_OK)) ) {
      openMode_ =  O_RDONLY;
      status    = true;
    }
  }
  return(status);
}

