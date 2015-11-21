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
/////////     pref_registry.C     //////////


#include <prefFile.h>
#include <userPrefFile.h>

#include <pref_registry.h>
#include <cLibraryFunctions.h>    
#include <msg.h>
#include <machdep.h>   // Needed for OSapi_getenv.
#include <messages.h>
#include <evalMetaChars.h>
#include <Args.h>    // Needed for DIS_args.
#include <stdlib.h>
#include <genError.h>

extern Args* DIS_args;


// Init static class constants.
const genString prefRegistry::UNINITIALIZED_STRING_PREF = "";
const int   prefRegistry::UNINITIALIZED_INT_PREF    = -1;

const char* prefRegistry::DebugLevelEnvVar = "DIS_INTERNAL_PREF_DEBUGLEVEL";



prefRegistry::prefRegistry()
{
    Initialize(prefRegistry::prefRegistry);
    prefBtree = new avl_tree();

    initSystemPreferences();
    initUserPreferences();

    char const * levelStr = OSapi_getenv((char*)DebugLevelEnvVar);
    if (levelStr && *levelStr)
         debugLevel = OSapi_atoi(levelStr);
    else
         debugLevel = 0;
}


// destructor.
prefRegistry::~prefRegistry()
{
    if (SysPrefFileName_) OSapi_free(SysPrefFileName_);
    if (UserPrefFileName_) OSapi_free(UserPrefFileName_);
    if (SysPrefFile_) delete SysPrefFile_;
    if (UserPrefFile_) delete UserPrefFile_;

    delete prefBtree;
}


// Initialize the System preferences.
bool prefRegistry::initSystemPreferences()
{
    Initialize(prefRegistry::initSystemPreferences);
    bool status = true;

    // Create the user preference file.
    SysPrefFile_ = new prefFile();
 
    // Get the system preference file name.
    // -sysPrefs is used to change the system resource file.
    // If it wasn't given the default file name will be used.
    char sys_pref_name[1024];
    sys_pref_name[0] = '\0';
    DIS_args->findValue("-sysPrefs", sys_pref_name);
    status = SysPrefFile_->Open(sys_pref_name);

    return(status);
}


// Initialize the User preferences.
bool prefRegistry::initUserPreferences()
{
    Initialize(prefRegistry::initUserPreferences);
    bool status = true;

    // For now, always will have user prefs.
    HasUserPrefs_ = true;

    // Create the user preference file.
    UserPrefFile_ = new userPrefFile();

    // Get the user preference file name.
    // Comes from -prefs, -userPrefs, or uses default user pref file.
    char user_pref_name[1024];
    user_pref_name[0] = '\0';
    DIS_args->findValue("-prefs", user_pref_name);
    if (*user_pref_name == '\0') DIS_args->findValue("-userPrefs", user_pref_name);
    status = UserPrefFile_->Open(user_pref_name);

    return(status);
}

static bool is_integer(const char*val, int*ival)
{
  char*ptr=NULL;
  *ival = strtol(val, &ptr, 10);
  return ptr[0] == '\0';
}
static int bool_value(const char*val)
{
  if(!val || !*val)
    return 0;  

  int int_val;

  if(is_integer(val, &int_val))
    ;
  else if(val[1] =='\0')
    int_val = (val[0] == 'n');
  else if (strcmp(val,"no")==0)
    int_val = 0;
  else if (strcmp(val,"off")==0)
    int_val = 0;
  else if (strcmp(val,"false")==0)
    int_val = 0;
  else
    int_val = 1;
  return int_val;
}


bool prefRegistry::getBoolPref(const char* keyname)
{
   Initialize(prefRegistry::getBoolPref);
   int index;
   bool found = getTableIndex(keyname, index);
   if(found){
     Pref* item = prefTable[index];
     if(item->prefType == Str_Pref && item->intPref == UNINITIALIZED_INT_PREF){
       item->intPref = bool_value(item->strPref);
     }
     return item->intPref;
   }

   genString value;
   int int_val;
   int pref_found_in = searchPrefValue(keyname, value, true);
   bool persistent = (pref_found_in != 0);
   
   if(pref_found_in >= 0){
     const char*val = value;
     if(is_integer(val, &int_val)){
       insertTable(keyname, int_val, persistent);
     } else {
       insertTable(keyname, value, persistent);
       int_val = bool_value(value);
     }
   } else {
     int_val = 0;
     insertTable(keyname, int_val, persistent);
   }
   return int_val;
}

const int& prefRegistry::getIntPref(const char* keyname)
{
   Initialize(prefRegistry::getIntPref);
   int* pointer = (int*) &UNINITIALIZED_INT_PREF;

   // First, check for preference already in the table.
   // Note - If searchTable returns true, pointer is set.
   bool status = searchTable(keyname, &pointer);
   if (status == false) {
     genString value;
     int pref_found_in = searchPrefValue(keyname, value);
     bool persistent = (pref_found_in != 0);
     int int_value = UNINITIALIZED_INT_PREF;
     if(pref_found_in >= 0){
       const char*val = value;
       if (isdigit(*val) )
	 int_value = OSapi_atoi(val);
       else
	 int_value = 0;
     }
     status = insertTable(keyname, int_value, persistent);
       
     status = searchTable(keyname, &pointer);
   }
   if (status == false)
     msg("Problem getting pref \'$1\'", error_sev) << keyname << eom;

   return (*pointer);
}

int pref_found_in = -1;
const genString& prefRegistry::getStrPref(const char* keyname,
                                const int expansion /*=EXPAND_NONE_TO_META*/) 
{
  Initialize(prefRegistry::getStrPref);
  genString* pointer = NULL;
  
  // First, check for preference already in the table.
  // Note - If searchTable returns true, pointer is set.
  bool status = searchTable(keyname, &pointer, expansion);
  pref_found_in = 0;
  if (status == false) {
    genString value;
    pref_found_in = searchPrefValue(keyname, value);
    bool persistent = (pref_found_in != 0);
    if(pref_found_in < 0)
      value = UNINITIALIZED_STRING_PREF;

    status = insertTable(keyname, value, expansion, persistent);
    status = searchTable(keyname, &pointer, expansion);
  }
  if (status == false)
    msg("Problem getting pref \'$1\'", error_sev) << keyname << eom;

  return (*pointer);
}

int prefRegistry::searchPrefValue(const char* keyname, genString&value, bool none_ok)
{
  Initialize(prefRegistry::SearchPrefValue);
  genString prefString = NULL;
  pref_found_in = -1; // not found (yet)

  bool persistent = true;

  // Not in the table, check the for a command line arg.
  genString option("-");
  option += keyname;
  char argStr[256];
  argStr[0] = '\0';
  DIS_args->findValue((char*)option, argStr);
  if (*argStr && (*argStr != '-') ) {
    persistent = false;
    value = argStr;
    pref_found_in = 1;
    if (debugLevel > 1)
      msg("Got pref \'$1\' from command line option.", normal_sev) << keyname << eom;
    
    // Not in the table, try the User preference file.
  } else if ( UserPrefFile_->Read(keyname, prefString) ) {
    value = prefString;
    pref_found_in = 2;
    if (debugLevel > 1)
      msg("Got pref \'$1\' from user preference file.", normal_sev) << keyname << eom;
    
    // Next, try the System preference file.
  } else if ( SysPrefFile_->Read(keyname, prefString) )  {
    value = prefString;
    pref_found_in = 3;
    if (debugLevel > 1)
      msg("Got pref \'$1\' from system preference file.", normal_sev) << keyname << eom;
    
  } else if(!none_ok){
    // Print a message if it is not an internal preference.
    if (OSapi_strncmp(keyname, "DIS_internal", 12) != 0)
      msg("Unable to find Preference \'$1\'", warning_sev) << keyname << eom;
    
    // Set a default value.
    pref_found_in = -1;
  }
  
  return pref_found_in;
}



// Try to get preference value from the environment.
// Converts and folder seperators into '_'s.
// Searches both case sensitive and all caps.
const char* prefRegistry::getPrefFromEnv(const char* prefName)
{
    Initialize(prefRegistry::getPrefFromEnv);
    const char * envStr = NULL;

    // Env. name is keyname with  ":" or "." substituted with an "_".
    // Note - This should probably get list of seperators from the
    int len = strlen(prefName);
    char* envName = new char[len + 1];
    for (int i=0; i<len; i++) {
        if ( (prefName[i] == '.') || (prefName[i] == ':') )
            envName[i] = '_';
        else
            envName[i] = prefName[i];
    }
    envName[len] = '\0';

    envStr = (char *)OSapi_getenv(envName);

    if (envStr) {
       if (debugLevel > 1)
          msg("Got pref \'$1\' from env var \'$2.", normal_sev) << prefName << eoarg << envName << eom;

    } else {
        // Didn't find it. Try again with all caps.
        for (int i=0; i<len; i++) {
            if ( islower(envName[i]) )
                envName[i] = toupper(envName[i]);
        }
        envStr = (char *)OSapi_getenv(envName);

       if (envStr && (debugLevel > 1) )
            msg("Got pref \'$1\' from env var \'$2.", normal_sev) << prefName << eoarg << envName << eom;
    }
    delete [] envName;
    return(envStr);
}



// Put a integer value into the preference dictionary.
bool prefRegistry::putPref(const char* keyname,
                           const int value,
                           const bool persistent /*=true*/)
{
   Initialize(prefRegistry::putPref);

     // If no user prefs are available, nothing can be persistent.
     bool saveIt = persistent && hasUserPrefs();

     bool status = insertTable(keyname, value, saveIt);

    if (status && saveIt) {
        Pref* item;
        int index;
        status = getTableIndex(keyname, index);
        if (status) {
            item = prefTable[index];
            status = UserPrefFile_->Write(keyname, item->intPref);
        }
    }

    if (!status)
       msg("Problem putting pref \'$1\'", error_sev) << keyname << eom;

    return (status);
}



// Put a string value into the preference dictionary.
bool prefRegistry::putPref(const char* keyname,
                            const genString value,
                            const int expansion /*=EXPAND_NONE_TO_META*/,
                            const bool persistent /*=true*/)
{
    Initialize(prefRegistry::putPref);

     // If no user prefs are available, nothing can be persistent.
     bool saveIt = persistent && hasUserPrefs();

     // Create or update the table entry.
     bool status = insertTable(keyname, value, expansion, saveIt);

    // Update the file if the value is persistent.
    // Only save persistent, unexpanded preferences.
    if (status && saveIt) {
       Pref* item;
        int index;
        status = getTableIndex(keyname, index);
        if (status) {
            item = prefTable[index];
            status = UserPrefFile_->Write(keyname, item->intPref);
           if (expansion == prefRegistry::EXPAND_COLONS_TO_NONE)
               status = UserPrefFile_->Write(keyname, item->unexpandedStrPref);
           else
               status = UserPrefFile_->Write(keyname, item->strPref);
        }
   }

   if (status == false)
      msg("Problem putting pref \'$1\'", error_sev) << keyname << eom;
   return (status);
}

bool prefRegistry::insertTable(const char *key,
                                const int value,
                                const bool persistent /*true*/)
{
  Initialize(prefRegistry::insertTable(const char *, const int, ...));
   // Get a pointer to the item.
   int size = prefTable.size();
   int index = -1;
   Pref* item;
   if (getTableIndex(key, index) ) {
      item = prefTable[index];
      // Only persistent items can change their longevity.
      if (item->persistent) item->persistent = persistent;
   } else {
      item = prefTable.grow();
      item->key = key;
      item->prefType = Int_Pref;
      item->intPref = UNINITIALIZED_INT_PREF;
      item->unexpandedStrPref = UNINITIALIZED_STRING_PREF;
      item->strPref = UNINITIALIZED_STRING_PREF;
      item->expandType = EXPAND_NONE;
      item->persistent = persistent;
      putTableIndex(key, size);
      index = size;
   }

   // Set the integer value
   if (item->prefType == Int_Pref) 
       item->intPref = value;
   else {
       genString strValue;
       strValue.printf("%d", value);
       expandPref(index, strValue);
    }

   return(true);
}


bool prefRegistry::insertTable(const char *key,
                                const genString value,
                                const int expansion /*=EXPAND_NONE_TO_META*/,
                                const bool persistent /*true*/ )
{
   Initialize(prefRegistry::insertTable(const char *, const genString, ...));
   int size = prefTable.size();
   int index = -1;
   Pref* item;
   if (getTableIndex(key, index) ) {
      item = prefTable[index];
      // Only persistent items can change their longevity.
      if (item->persistent) item->persistent = persistent;

   } else {
      // Item was not in the table, create a new entry for it.
      item = prefTable.grow();
      item->key = key;
      item->prefType = Str_Pref;
      item->intPref = UNINITIALIZED_INT_PREF;
      item->unexpandedStrPref = UNINITIALIZED_STRING_PREF;
      item->strPref = UNINITIALIZED_STRING_PREF;
      item->expandType = EXPAND_NONE;
      item->persistent = persistent;
      putTableIndex(key, size);
      index = size;
   }


   // Got the item, now update it.
   if (item->prefType == Str_Pref) {
       item->expandType = expansion;
       expandPref(index, value);
   } else { 
       item->intPref = OSapi_atoi(value);
   }
   return(true);
}


// Utility to expand a string preference.
bool prefRegistry::expandPref(int index, const genString value)
{
    bool status = true;

    prefExpansion expansion = (prefExpansion) prefTable[index]->expandType;

    switch(expansion) {
        case EXPAND_NONE:
            prefTable[index]->unexpandedStrPref = value;
            prefTable[index]->strPref = prefTable[index]->unexpandedStrPref;
            break;
        case EXPAND_NONE_TO_META:
            prefTable[index]->unexpandedStrPref = value;
            status = expandMeta(index);
            break;
        case EXPAND_NONE_TO_COLONS:
            prefTable[index]->unexpandedStrPref = value;
            status = expandMeta(index);
            if (status) {
                genString tmpStr;
                if (expandColons(prefTable[index]->strPref, tmpStr) )
                    prefTable[index]->strPref = tmpStr;
            }
            break;
        case EXPAND_COLONS_TO_NONE:
            prefTable[index]->strPref = value;
            status = unexpandColons(prefTable[index]->unexpandedStrPref, prefTable[index]->strPref);
            break;
    }

    if (!status) {
        msg("Problem expanding value \'$1\' for preference \'$2\'.", error_sev) << (char*) prefTable[index]->unexpandedStrPref << eoarg << (char*) prefTable[index]->key << eom;
    }

    return(status);
}


// Utility to expand meta chars in a string preference.
bool prefRegistry::expandMeta(int index)
{
    bool status = true;
    const genString unexpanded = prefTable[index]->unexpandedStrPref;
    genString &expanded = prefTable[index]->strPref;
    // Make sure a non-null, non-empty value was found.
    if ((char*)unexpanded && (unexpanded.length() > 0) ) {
        // expand the value.
        status = eval_shell_metachars(unexpanded, expanded);
        // If the expansion failed, use the unexpanded value.
        if (!status) prefTable[index]->strPref = unexpanded;
    } else {
        expanded = UNINITIALIZED_STRING_PREF;
    }

    return(status);
}


// Utility to convert ':'s into '\n's.
bool prefRegistry::expandColons(const genString unexpanded, genString& expanded)
{
    bool status = true;

    // Loop through tokens seperated by ':'
    // Add a '\n' at the end of each token but the last one.
    if ((char*)unexpanded && (unexpanded.length() > 0) ) {
        char* newlineStr = OSapi_strdup(unexpanded);
        char* token = strtok(newlineStr, ":");
        expanded = token;

        while (token = strtok(NULL, ":") ) {
            expanded += '\n';
            expanded += token;
        }
        OSapi_free(newlineStr);
    } else {
        expanded = UNINITIALIZED_STRING_PREF;
    }

    return(status);
}


// Utility to convert '\n's into ':'s.
bool prefRegistry::unexpandColons(genString& unexpanded, const genString expanded)
{
    bool status = true;

    if ((char*)expanded && (expanded.length() > 0) ) {
        char* newlineStr = OSapi_strdup(expanded);
        char* token = strtok(newlineStr, "\n");
        unexpanded = token;

        while (token = strtok(NULL, "\n") ) {
            unexpanded += ':';
            unexpanded += token;
        }
        OSapi_free(newlineStr);
    } else {
        unexpanded = UNINITIALIZED_STRING_PREF;
    }

    
    return(status);
}


bool prefRegistry::searchTable(const char* key,
                                int** handle)
{
    // Find the index.
    int index;
    bool found = getTableIndex(key, index);
    if (found) {
        // Set up a pointer to the value.
        *handle = &prefTable[index]->intPref;

        // If it is a string pref, try converting the value to int.
        if (prefTable[index]->prefType == Str_Pref) 
            **handle = OSapi_atoi(prefTable[index]->strPref);
    }

    return found;
}


bool prefRegistry::searchTable(const char* key,
                                genString** handle,
                                const int expansion /*=EXPAND_NONE_TO_META*/)
{
    // Find the index.
    int index;
    bool found = getTableIndex(key, index);
    if (found) {
        // Set up a pointer to the string value.
        if (expansion == prefRegistry::EXPAND_NONE)
            *handle = &prefTable[index]->unexpandedStrPref;
        else
            *handle = &prefTable[index]->strPref;

        // If it is a integer pref, convert the value to a string.
        if (prefTable[index]->prefType == Int_Pref) 
            (*handle)->printf("%d", prefTable[index]->intPref);
    }

   return found;
}






// Returns true/false if the given keyname is persistent.
// Also returns true if the keyname was not found.
bool prefRegistry::isPersistent(const char* key)
{
    bool persistent = true;
    int index;
    if (getTableIndex(key, index) )
        persistent = prefTable[index]->persistent;

    return(persistent);
}


// Method to get the index for keyname.
bool prefRegistry::getTableIndex(const char* key, int& index)
{
    Initialize(prefRegistry::getTableIndex);
    prefNode* found = NULL;

    if (key) {
      prefKey k(key);
      found = (prefNode*)prefBtree->find(k);
    }

    if (found)
      index = found->get_index();

    return(found != NULL);
}


// Method to put the index for keyname.
bool prefRegistry::putTableIndex(const char* key, const int index)
{
  Initialize(prefRegistry::putTableIndex);
  bool status = false;
  if (key) {
    prefNode * pn = new prefNode(prefBtree, key, index);
    status = true;
  }
  return(status);
}

prefNode::prefNode(avl_treeptr root, const char * str, const int ind): avl_node(root, *(pref_key = new prefKey(str))), index(ind)
{
}

int prefNode::compare(const avl_key& dest) const
{
  Initialize(prefNode::compare);
  int ret_val = -1;
  prefKey& dest_key = (prefKey &)dest;
  char *dest_str = dest_key.get_key();
  char *my_str = key();
  if (dest_str && my_str)
    ret_val = OSapi_strcmp(dest_str, my_str);
  if (ret_val > 0) ret_val = 1;
  if (ret_val < 0) ret_val = -1;
  return ret_val;
}





//////////     end of pref_registry.C     /////////
