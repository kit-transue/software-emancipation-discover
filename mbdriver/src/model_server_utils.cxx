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
#include "transport.h"
#include "service.h"
#include "model_server_msgser.h"
#include "msg.h"
#include "util.h"
#include "model_server_utils.h"
#ifdef _WIN32
#include <io.h>
#endif //_WIN32

using namespace nTransport;
namespace MBDriver {

#define SLASH "/"
#define BACK_SLASH "\\"

// These are functions written to help determine the home project in the preference file

/* readLine() reads a line up to MAX_LINE_SIZE, newline, or EOF from a file, strips a potential newline off of it, and returns the string in "returnString".  If the first character it encounters is EOF, then that is what it returns */
int readLine(FILE * openFile, char returnString[]) {
	int character, endOfLine=0, counter=0;

	/* test for endOfLine should be first, so the next char doesn't
		go on the stack */
	while(endOfLine != 1 && (character = getc(openFile)) != EOF) {
		if((counter == MAX_LINE_SIZE-2) || character == '\n') {
			endOfLine = 1;
		} else {
			if(character != '\r') {
				returnString[counter++] = character;
			}
		}
	}

	returnString[counter] = '\0';

	// if EOF is found, return it to indicate such
	if(character == EOF) {
		return(character);
	} else {
		return 0;
	}
}

int getPreference(const char * preference, const char * prefFileName, char prefValue[]) {

	FILE * preferenceFile = fopen(prefFileName, "r");
	char * prefPointer = NULL, tempLine[MAX_PREF_VALUE];
	int prefFound = 0;

	if(preferenceFile == NULL) {
		printf("ERROR: The given preference file, \"%s\", is invalid\n", prefFileName);
		return(0);
	}

	while(prefFound != 1 && readLine(preferenceFile, tempLine) != EOF) {
		if((prefPointer = strstr(tempLine, preference)) != NULL) {
			int count=0;
			prefPointer += strlen(preference);
			while(*prefPointer != '\0') {
				if(*prefPointer != ' ' && *prefPointer != '\t') {
					prefValue[count++] = *prefPointer;
				}
				prefPointer++;
			}
			prefValue[count] = '\0';
			prefFound = 1;
		}
	}
	fclose(preferenceFile);
	return(prefFound);
}

static void replace_slash_with_backslash(string& dir)
{
  int ind = dir.find(SLASH);
  while(ind != -1) {
    dir.replace(ind, 1, BACK_SLASH);
    ind = dir.find(SLASH, ind);
  }
}

#ifdef _WIN32
//Same as "ls"(unix) command expect the results are not passed along.
//Only a bool value to returned.
static bool dir_has_file(const string& directory, const string& pattern)
{
  bool ret_value = false;
  FILE* file;
  string dir = directory;
  replace_slash_with_backslash(dir);
  string cmd = "cmd /C \" cd /D ";
  cmd.append(dir);
  cmd.append(" && dir /B ");
  cmd.append(pattern);
  cmd.append(" \"");
  file = _popen(cmd.c_str(), "rt");
  if (file != NULL) {
    char line[1024] = "";
    string str;
    while (!feof(file) ) {
	if (fgets(line, 1024, file) != NULL) { 
	  if (strcmp(line, "") != 0) {//return true if result is non empty
	    ret_value = true;
	    break;
	  }
	}
    }
    _pclose(file);
  }
  return ret_value;
}
#endif


//returns true if dir exists
static bool dir_exists(const char *dir)
{
    bool exists = true;
    if (dir) {
#ifdef _WIN32
	if (_access(dir, 00) == -1) //had to include <io.h>
#else
	if (access(dir, F_OK) == -1)
#endif
	    exists = false;
    }

    return exists;
}


void set_environment_for_model_server(const char * admindir, const char *psethome)
{
    if (psethome) {
		//set PATH=$PSETHOME/bin;$PATH
		int psethomebin_len = strlen(psethome) + 5; // 5 is "/bin" + null terminator
		char* psethomebin = new char[psethomebin_len]; 
		const char *old_path = getenv("PATH");
		if(strncmp(psethomebin,old_path,psethomebin_len)!=0) {
			static string *new_path = new string("PATH=");
			new_path->append(psethome);
			new_path->append("/bin");
#ifdef _WIN32
			new_path->append(";");
#else
			new_path->append(":");
#endif
			new_path->append(old_path);
			putenv((char*)new_path->c_str()); 
		}
    }

    //set ADMINDIR environment variable
    static string *admindir_var = new string();
	admindir_var->assign("ADMINDIR=");
    admindir_var->append(admindir);
    putenv((char *)admindir_var->c_str()); 

    //set TMPDIR environment variable
    string tmpdir;
    const char* rover_tmpdir = getenv("ROVER_TMPDIR");
    if (rover_tmpdir) {
		tmpdir = rover_tmpdir;
    } else {
		tmpdir = admindir;
		tmpdir.append("/tmp");
    }
    if (dir_exists(tmpdir.c_str())) {
		static string *tmpdir_var = new string();
		tmpdir_var->assign("TMPDIR=");
		tmpdir_var->append(tmpdir);
		putenv((char*)tmpdir_var->c_str());
    }
}


//This function will validate existence of model by checking for
//$ADMINDIR/model/pmod directory.
//Could be made more fool proof by checking for pdf file, pref file,
//pmods(.ind, .sym, .lin) ... 
bool validate_model_dir(const string *model_dir)
{
    bool model_is_present = false;
    if (model_dir) {
	string dir = model_dir->c_str();
	dir.append("/model/pmod");
	model_is_present = dir_exists(dir.c_str());
    }
    return model_is_present;
}

//if model server is running shut it down
void stop_model_server(const string *dsppath, const string *output) {
    string name = get_service_name(dsppath, output);
    string service_name = get_internal_service_name(name);

    int port;
    int ip_addr;
    //bring down model server by sending "stop_server -y" command
    ServiceName sn(service_name.c_str());
    if (sn.getPortAndAddr(port, ip_addr)) {
	char *cmd = "stop_server -y";
	ModelMsgSerializer ser;
	SingleSender<char*> sndr(cmd, ser);
	ServicePort pmodService_port(service_name.c_str());
	connect(pmodService_port, sndr);

	/* Give some time for model_server to shut down so that
	   it has had time to release all the files in use.*/
	int trials = 5;
	while(sn.getPortAndAddr(port, ip_addr) && trials > 0) {
	    sleep_in_seconds(1);
	    --trials;
	}
    }

    //bring down QAEngine by sending "bye" command if it is running
    string qa_service_name = "QA:";
    qa_service_name += name;
    ServiceName qasn(qa_service_name.c_str());
    if (qasn.getPortAndAddr(port, ip_addr)) {
	char *cmd = "bye";
	ModelMsgSerializer ser;
	SingleSender<char*> sndr(cmd, ser);
	ServicePort qaservice_port(qa_service_name.c_str());
	connect(qaservice_port, sndr);
    }
}

}
