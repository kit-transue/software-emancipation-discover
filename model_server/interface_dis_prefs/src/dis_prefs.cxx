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
/**********
*
* dis_prefs.C - Main module for dis_prefs executable.
*
**********/

#ifdef WIN32
#include <windows.h>
#endif

#include <vport.h>
#include vstdlibHEADER
#include vapplicationHEADER
#include "Args.h"
#include "getResource.h"
#include "prefFile.h"

#include "galaxy_undefs.h"
//#include "pref_registry.h"
//#include "messages.h"
#include "cLibraryFunctions.h"
#include "genString.h"
#include "evalMetaChars.h"


// Define this to allow puts to the User's preferences.
// This could be commented out to limit availability.
#define ALLOW_PUTS



/**********
*
* main - Entry point for dis_prefs.
*
**********/

int main(int argc, char *argv[])
{
    extern void print_usage(); 
    prefFile* SysPrefFile_ = NULL;
    prefFile* UserPrefFile_ = NULL;

    vprefSetArgs(argc, argv);   // Scan for galaxy specific args.

    // We want to use the same prefs as regular aset, so we change our name.
    vapplication::GetCurrent()->SetName(vnameInternGlobalLiteral("aset") );

    // Init args and flags.
    Args* DIS_args = new Args(argc, argv);


    // Check for override of system preferences file.
    char sysPrefFileName[8192];
    sysPrefFileName[0] = '\0';
    DIS_args->findValue("-sysPrefs", sysPrefFileName);
    if (*sysPrefFileName)
        SysPrefFile_ = new prefFile(sysPrefFileName);
    else
        SysPrefFile_ = new prefFile(prefFile::DEFAULT_SYSTEM_PREFERENCE_FILE);

    // Check for override of user preferences file.
    // This is made more complicated because both the
    // -prefs and -userPrefs options are valid.
    char userPrefFileName[8192];
    userPrefFileName[0] = '\0';
    DIS_args->findValue("-userPrefs", userPrefFileName);
    if (*userPrefFileName == '\0') DIS_args->findValue("-prefs", userPrefFileName);

    // Get preference name and type.
    char prefName[8192];
    prefName[0] = '\0';
    genString prefValue;
    vbool isStrPref = vTRUE;
    vbool isGetReq = vTRUE;
    DIS_args->findValue("-getStr", prefName);
    if (*prefName == '\0') {
        DIS_args->findValue("-getInt", prefName);
        if (*prefName)
            isStrPref = vFALSE;
        else {
#if defined(ALLOW_PUTS)
            DIS_args->findValue("-putStr", prefName);
            if (*prefName) {
                isGetReq = vFALSE;
                isStrPref = vTRUE;
                // The value to put MUST be the last arguement.
                prefValue = argv[argc-1];
            } else {
                DIS_args->findValue("-putInt", prefName);
                if (*prefName) {
                    isGetReq = vFALSE;
                    isStrPref = vFALSE;
                    prefValue = argv[argc-1];
                } else {
                    print_usage();
                }
            }
#else
            print_usage();
#endif    // ALLOW_PUTS
        }
    }

    if (*prefName) {
        vbool gotValue = vFALSE;
        if (*userPrefFileName)
            UserPrefFile_ = new prefFile(userPrefFileName);
        else
            UserPrefFile_ = new prefFile(prefFile::DEFAULT_USER_PREFERENCE_FILE);
        if (isGetReq) {
            if (isStrPref) {
                vstr* vStrValue = NULL;
                gotValue = ( UserPrefFile_->Read(prefName, &vStrValue) ||
                             SysPrefFile_->Read(prefName, &vStrValue) );
                if (gotValue) {
                    // Need to convert to char and expand.
                    char* strValue = (char*) vcharExportAlloc(vstrScribe(vStrValue) );
                    const char * err = eval_shell_metachars(strValue, prefValue);
                    if (err) {
                        OSapi_fprintf(stderr, "*** %s", err);
                        OSapi_fprintf(stderr, "*** Problem expanding value %s for preference %s", 
                                            (char*) strValue, prefName);
                    }
                vcharExportFree(strValue);
                }
            } else {
                int value;
                gotValue = ( UserPrefFile_->Read(prefName, value) ||
                             SysPrefFile_->Read(prefName, value) );
                if (gotValue) prefValue.printf("%d", value);
            }

            // This is the real 'output' of this executable.
            // Echo the value out to stdout for a shell script to use.
            if (gotValue)
                OSapi_fprintf(stdout, "%s\n", (char*) prefValue);

        } else {
#if defined(ALLOW_PUTS)
            if (isStrPref)
                UserPrefFile_->Write(prefName, prefValue);
            else {
                int value = atoi(prefValue);
                UserPrefFile_->Write(prefName, value);
            }
#endif    // ALLOW_PUTS
        }
    }

    if (UserPrefFile_) delete UserPrefFile_;
    if (SysPrefFile_) delete SysPrefFile_;


    exit (EXIT_SUCCESS);
    return EXIT_FAILURE;
}   /* end of main */


// Print out usage information.
void print_usage()
{
    OSapi_fprintf(stderr, "Usage:\n");
    OSapi_fprintf(stderr, "The following will get a preference and output the value to stdout.\n");
    OSapi_fprintf(stderr, "\tdis_prefs -getStr folderName.prefName\n");
    OSapi_fprintf(stderr, "\tdis_prefs -getInt folderName.prefName\n");

#if defined(ALLOW_PUTS)
    OSapi_fprintf(stderr, "The following will store a new preference value in the users preference file.\n");
    OSapi_fprintf(stderr, "\tdis_prefs -putStr folderName.prefName value\n");
    OSapi_fprintf(stderr, "\tdis_prefs -putInt folderName.prefName value\n");
#endif    // ALLOW_PUTS
}





/**********     end of main.C     **********/
