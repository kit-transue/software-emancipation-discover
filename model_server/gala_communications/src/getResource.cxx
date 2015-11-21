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
// **********
//
// getResource.cxx - global functions to get Galaxy resources.
//
// Note - This is used by all executables.
//
// **********

#include <vport.h>
#include <getResource.h>
#include <ResourceNameIterator.h>



// Resource info, static to this file.
vbool resourcesLoaded = vFALSE;
vresource systemResources = NULL;


// Note - Need to leave this stuff in until gala uses prefFile.cxx stuff.
static void DoRealFlush( vresource src );
#ifdef _WIN32
static const char *USER_PREFERENCE_FILE = "vgalusr1.vr";
#else
static const char *USER_PREFERENCE_FILE = ".vgalaxy.1.vr";
#endif




//Static function to handle GetResource Not-Found expection.
static vbool gala_GetResourceExp(vresource dict, const vname* name,
                            vresource * dest);



/**********
 *
 * gala_GetResource(name, dest)
 *    Returns vTRUE and fills in dest if name is found.
 *    Returns vFALSE if name is not found.
 *    Name can contain a hierarchy of dictionaries seperated by ':'s.
 *    Searches Preferences then Resources files.
 *
 ***** History
 *
 * 12/27/95 TLW - Created.
 *
 **********/

int gala_GetResource(const vname* resourceName, 
		     vresource* destResource)
{
    // First check in preferences file.
    if (gala_GetResource(vapplication::GetCurrent()->GetPreferences(),
                         resourceName, destResource) ) {
        return (vTRUE);
    } 

    // Make sure that resources have been loaded.
    if (!resourcesLoaded) {
	systemResources = vapplication::GetCurrent()->GetResources();
	resourcesLoaded = vTRUE;
    }

    // Now, check in the system resource file.
    if (gala_GetResource(systemResources,
                         resourceName, destResource) ) {
        return (vTRUE);
    }

    // Unable to find the resource.
    return (vFALSE);
}




/**********
 *
 * gala_GetResource(dict, name, dest)
 *    Returns vTRUE and fills in dest if name is found in dict.
 *    Returns vFALSE if name is not found.
 *    Handles all not-found (vexGetArg) exceptions.
 *
 ***** Notes
 *
 * This function uses ResourceNameIterator class.
 *
 **********/

int gala_GetResource(vresource resourceDict,
                     const vname* resourceName, 
		     vresource* resourceDest)
{
    vbool found = vTRUE;
    vresource dict = resourceDict;
    ResourceNameIterator iter(resourceName);
    vresource dest;
    while (iter.Next() ) {
        if (gala_GetResourceExp(dict, iter.GetNext(), &dest) ) {
            dict = dest;
        } else {
            found = vFALSE;
            break;
        }
    }

    if (found == vTRUE) *resourceDest = dest;
    return(found);
}




/**********
 *
 * gala_GetResourceExp(dict, name, dest)
 *    Returns vTRUE and fills in dest if name is found in dict.
 *    Returns vFALSE if name is not found.
 *    Handles all not-found (vexGetArg) exceptions.
 *
 **********/

static vbool gala_GetResourceExp(vresource resourceDict,
                              const vname* resourceName, 
                              vresource* resourceDest)
{
    vbool found = vTRUE;

    vexWITH_HANDLING {
        *resourceDest = vresourceGet(resourceDict, resourceName);
    }
    vexON_EXCEPTION {
        if (vexGetArgException()) {    // Name not found in Dict.
            found = vFALSE;
        } else    // Handle the expection.
            vexPropagate(vexGetRootClass(), vexNULL);
    } vexEND_HANDLING;

    return (found);
}


// Sets the system resource file to the given file name.
vbool setResourceFile(const char* fileName)
{
    vbool status = vTRUE;

    // Convert the file name into a path.
    vfsPath* path = new vfsPath;
    path->Set(vnameInternGlobalLiteral(fileName) );
    path->ExpandVariables();

    // Try looking in the current and the app dir for the file.
    status = path->Access(vfsFILE_EXISTS);
    if (!status && (path->GetType() != vfsPATH_TYPE_FULL) ) {
        // File not found and is not a full path, try looking in the appdir.
        vfsPath* tmpPath = path;
        path = vfsPath::CloneAppDir();
        path->Join(tmpPath);
        path->MakeCanonical();
    }

    // If we can read the file, open it as a resource file.
    status = path->Access(vfsCAN_READ);
    if (status) {
        systemResources = vresourceOpenFile(path, vfsOPEN_READ_ONLY);
        resourcesLoaded = vTRUE;
    } else {
        printf("'%s' is an invalid .vr file.  Reverting to default .vr file.\n", fileName); 
    }

    return (status);
}


// Sets the user's preference folder to the given name.
vbool setPreferences(const char* folderName)
{
    // Get root of current preferences.
    vresource currentPrefs = vapplication::GetCurrent()->GetPreferences();
    vresource prefParent = vresourceGetParent(currentPrefs);
    // Create a new folder with the given name.
    currentPrefs = vresourceCreate(prefParent, vnameInternGlobalLiteral(folderName), vresourceDICT);
    vresourceFlush(vresourceGetRoot(prefParent) );
    // Make the new folder the current preferences.
    vapplication::GetCurrent()->SetPreferences(currentPrefs);
    return(vTRUE);
}


// Deletes the current user's preference folder.
vbool deletePreferences()
{
    // Get current preference dictionary and delete it.
    vresource currentPrefs = vapplication::GetCurrent()->GetPreferences();
    vresourceRemove(currentPrefs, vresourceTAG_NULL);
    vresourceFlush(vresourceGetRoot(currentPrefs) );
    // Note, we may need to set the preferences back to the default folder.
    return(vTRUE);
}


// Preference utility functions.



// Get integer value from the preference dictionary.
// Fills the integer preference with the string value.
// Returns true if the preference is found, false otherwise.
vbool getPreference(const char* keyname, int& value)
{
    // See if we can get the resource.
    vresource prefRes;
    vbool status = getResource(vnameInternGlobalLiteral(keyname), &prefRes);
    if (status) {
        // Get the resource tag, it is the last token in the name.
        const vname * tag;
        ResourceNameIterator iter(keyname);
        while (iter.Next() ) tag = iter.GetNext();
        // Get the resource value.
        vexWITH_HANDLING {
            status = vresourceTestGetInteger(vresourceGetParent(prefRes),
                                             tag, &value);
        }
        vexON_EXCEPTION {
            printf("Problem getting preference '%s'\n", keyname);
            if (vexGetArgException()){    
                printf("Probably an invalid folder name.\n");
            } else    // Let system handle the expection.
                vexPropagate(vexGetRootClass(), vexNULL);
        } vexEND_HANDLING;
    }
    return (status);
}


// Get string value from the preference dictionary.
// Fills the vstr** preference with the preference value.
// Returns true if the preference is found, false otherwise.
vbool getPreference(const char* keyname, vstr** value)
{
    vstr* prefStr = NULL;
    // See if we can get the resource.
    vresource prefRes;
    vbool status = getResource(vnameInternGlobalLiteral(keyname), &prefRes);
    if (status) {
        // Get the resource tag, it is the last token in the name.
        ResourceNameIterator iter(keyname);
        const vname * tag;
        while (iter.Next() ) tag = iter.GetNext();
        // Get the resource string.
        vexWITH_HANDLING {
            status = vresourceTestGetString(vresourceGetParent(prefRes),
                                            tag, &prefStr);
        }
        vexON_EXCEPTION {
            printf("Problem getting preference '%s'\n", keyname);
            if (vexGetArgException()){    
                printf("Probably an invalid folder name.\n");
            } else    // Let system handle the expection.
                vexPropagate(vexGetRootClass(), vexNULL);
        } vexEND_HANDLING;
    }
    if (status) *value = prefStr;
    return (status);
}






// Put an integer value into the preference dictionary.
vbool putPreference(const char* keyname, const int value)
{
    ResourceNameIterator iter(keyname);
    const vname* tag = NULL;
    // Start out with the application preferences folder.
    vresource prefRes = vapplication::GetCurrent()->GetPreferences();

    vexWITH_HANDLING {
        // Step through the name, making resources as needed.
        while(iter.Next() ) {    
            tag = iter.GetNext();
            prefRes = vresourceMake(prefRes, tag);
        }
        // Set the Integer value.
        vresourceSetInteger(vresourceGetParent(prefRes), tag, value);
    }
    vexON_EXCEPTION {
        printf("Problem saving preference '%s', value '%d'\n",
                     keyname, value);
        if (vexGetArgException()){    
            printf("Probably an invalid folder name.\n");
        } else    // Let system handle the expection.
            vexPropagate(vexGetRootClass(), vexNULL);
    } vexEND_HANDLING;

   // Force storage of the change.
   DoRealFlush( vresourceGetRoot(prefRes) );

   return (vTRUE);
}


// Put an String value into the preference dictionary.
vbool putPreference(const char* keyname, const char* value)
{
    vresource prefRes = vapplication::GetCurrent()->GetPreferences();
    vexWITH_HANDLING {
        const vname* tag = NULL;
        ResourceNameIterator iter(keyname);
        // Step through the name, making resources as needed.
        while(iter.Next() ) {    
            tag = iter.GetNext();
            prefRes = vresourceMake(prefRes, tag);
        }
        // Set the Sstring value.
        vresourceSetString(vresourceGetParent(prefRes), tag,
                           vnameInternGlobalLiteral(value));
        
        
    }
    vexON_EXCEPTION {
        printf("Problem saving preference '%s', value '%s'\n",
                     keyname, value);
        if (vexGetArgException()){    
            printf("Probably an invalid folder name.\n");
        } else    // Let system handle the expection.
            vexPropagate(vexGetRootClass(), vexNULL);
    } vexEND_HANDLING;

   // Force storage of the change.
   DoRealFlush( vresourceGetRoot(prefRes) );

   return (vTRUE);
}

void DoRealFlush( vresource src )
{
  vexWITH_HANDLING 
  {
    vfsPath path;
    path.CopyInit( vfsGetHomeDirPath() );
    path.Append( (const vchar *) USER_PREFERENCE_FILE, NULL );
    vresource rFile = vresourceCreateFileFromMem( src, &path,
                      vfsOWNER_READ|vfsOWNER_WRITE ); 
    if( rFile )
      vresourceCloseFile( rFile );
  }
  vexON_EXCEPTION 
  {
    printf("Could not save memory resource file on disk\n" );
  } vexEND_HANDLING;

}






/**********     end of getResource.cxx     **********/

