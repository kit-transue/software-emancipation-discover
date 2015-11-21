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
// BatchBuild.cpp: implementation of the BatchBuild class.
//
//////////////////////////////////////////////////////////////////////

#include "BatchBuild.h"

#ifndef _WIN32
  #include <unistd.h>
  #include <sys/time.h>
  #include <sys/resource.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <pwd.h>
  #include <sys/wait.h>
#else
  #include <direct.h>
  #include <process.h>
  #include <io.h>
  #include <string.h>  
#endif
#include <time.h>
#include <fcntl.h>

#define _MESSAGING

#ifdef _MESSAGING
#include "msg.h"
#endif

static ServerInterface* server;
static HitsTable*       previousHits=NULL;

static int callback(TaskDriver* driver,int nodeId, string module, string script, string command,string arguments) {
	Hit* hit = new Hit("Hit",module,10,"path");
	int type = driver->isStatistical(nodeId);
	if(type==NO_TREE || type==NO_NODE) return -1;
	HitsSet queryHits = server->execQuery(type,script,command,module,arguments);
	HitsSet::iterator i;

	for(i=queryHits.begin();i!=queryHits.end();i++) {
		Hit* hit = *i;
		if(previousHits&&!type) {
			HitsSet matches = previousHits->compare(command,*hit);

			// No matching AST path - new hit
			if(matches.size()==0) {
				//cout << "Match not found." << endl;
			}

			// Exact match found  - old hit
			if(matches.size()==1) {
				//cout << "Match found." << endl;
				// Copy user properties to the newly created hit
				Hit* oldHit = *matches.begin();
				PropertiesMap* map = oldHit->getAttributes();
				PropertiesMap::iterator j;
				for(j=map->begin();j!=map->end();j++) {
					hit->setAttribute(j->first,j->second);
				}
			}

			// Multiple matches found - we do not know for shure
			if(matches.size()>1) {
				//cout << "Multiple matches....";

				// Trying to select hit using its line number
				// (will work if file was not modified)
				HitsSet::iterator j;
				for(j=matches.begin();j!=matches.end();j++) {
				   Hit* oldHit = *j;
				   if(oldHit->getLine()==hit->getLine()) {
					   break;
				   }
				}

				// New hit has the same line number as the old one-
				// most likely this is an old hit
				if(j!=matches.end()) {
				    //cout << "Resolved"<<endl;
				    Hit* oldHit = *j;
				    PropertiesMap* map = oldHit->getAttributes();
				    PropertiesMap::iterator j;
				    for(j=map->begin();j!=map->end();j++) {
					   hit->setAttribute(j->first,j->second);
					}
				} else {
					// No matching line numbers - we can do nothing
				    //cout << "Not resolved"<<endl;
				}
			}
		} else {
		}
        driver->addHit(command,hit);
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BatchBuild::BatchBuild() {
   server = &m_Server;
   previousHits=NULL;
   m_BuildRootPath = ".";
}

BatchBuild::~BatchBuild() {

}

//------------------------------------------------------------------
// Main method of this class. Executes batch mode build and user
// attributes propagation.
//------------------------------------------------------------------
int BatchBuild::build(string project) {
string directoryName;
string buildDirectoryFullPath;
string policyFullName;
string projectFullName;
time_t currentTime;

    // We will not be able to build if 
    // we are not connected to the server
    if(!m_Server.isConnected()) return NO_CONNECTION;

    // New directory for this build will reflect current date and time
    time(&currentTime);
    directoryName = timeToDir(currentTime);

	// Setting current directory to the QA model root
    buildDirectoryFullPath = m_BuildRootPath+"/"+directoryName;
	if(chdir(m_BuildRootPath.c_str())!=0) {
		return NO_BUILDROOT;
	}

	// Policy and project names
	if(project.length()>0) {
	    policyFullName  = m_SharedRootPath+"/"+project+".policy.xml";
	    projectFullName = m_SharedRootPath+"/"+project+".project.xml";
	} else {
	    policyFullName  = m_SharedRootPath+"/"+"default.policy.xml";
	    projectFullName = m_SharedRootPath+"/"+"default.project.xml";
	}


	// Creating directory for the current build information
	if(time==NULL) return NO_TIME;

#ifdef WIN32
	if(mkdir(directoryName.c_str())!=0) {
#else
	if(mkdir(directoryName.c_str(),0777)!=0) {
#endif

		return NO_TIME;
	}

	// Trying to open policy file. 
	// Can't build without selected policy
	if(m_Task.openPolicy(policyFullName)!=0) {
		return NO_POLICY;
	}

    // Opening current project. If no project exists
	// we will use default values stored in the policy,
	// so this will not prevent us from build
	m_Task.openProject(projectFullName);


	// Trying to locate previous build (vector is sorted)
	// If we are able to locate, we will open previous build hits 
	// table and we will assign it to the static global variable
	// "previousHits" to be able to detect new hits in this 
	// build while running callback function.
	set<string> builds = getFolders("*");
	set<string>::reverse_iterator i;
	string file = "";
	for(i=builds.rbegin();i!=builds.rend();i++) {
		string folder = *i;
		file = m_BuildRootPath+"/"+folder+"/"+"hits.dat";
		int fd;
		fd=open(file.c_str(),O_RDONLY);
		if(fd!=-1) {
			close(fd);
			break;
		}
	}
	if(i!=builds.rend()) {
		previousHits = new HitsTable();
		previousHits->load(file);
	} else {
		if(previousHits!=NULL) delete previousHits;
		previousHits = NULL;
	}

    // Getting complete list of modules from 
	// the server using ServerInterface class
    ScopeSet scope = m_Server.getBuildScope();

	string fileCountCmd = "qa_file_count:";
	char filesNumStr[6];
	fileCountCmd += itoa(scope.size(),filesNumStr,10);
	report_progress(fileCountCmd.c_str());

	// New empty set of hist
	m_Task.createHits();

	// !!! This function will run execution itself calling callback
	// function "callback" for every query.
 	m_Task.execute(scope,&callback);


	// We do not need previous build hits anymore
	if(previousHits!=NULL) delete previousHits;
	previousHits = NULL;

	// Saving hits into this build directory
	m_Task.saveHits(buildDirectoryFullPath+"/hits.dat");

	// Saving policy file and project file into the same
	// directory. When brousing hits we will use saved
	// policy and project file to build query tree.

	// Policy
	string policyBackupName = buildDirectoryFullPath+"/policy.xml";
	int fd1;
	fd1=open(policyFullName.c_str(),O_RDONLY);
	if(fd1!=-1) {
		int fd2;
	    fd2=open(policyBackupName.c_str(),O_WRONLY|O_CREAT);
		char buf[1024];
		int readed = 0;
		do {
			readed = read(fd1,buf,1024);
			write(fd2,buf,readed);
		} while(readed>0);
		close(fd2);
		close(fd1);
	}

	// Project
	string projectBackupName = buildDirectoryFullPath+"/project.xml";
	fd1=open(projectFullName.c_str(),O_RDONLY);
	if(fd1!=-1) {
		int fd2;
	    fd2=open(projectBackupName.c_str(),O_WRONLY|O_CREAT);
		char buf[1024];
		int readed = 0;
		do {
			readed = read(fd1,buf,1024);
			write(fd2,buf,readed);
		} while(readed>0);
		close(fd2);
		close(fd1);
	}

	return OK;
}

int BatchBuild::connect(string host, string name) {
	if(m_Server.connect(host,name)!=-1) return OK;
	return NO_CONNECTION;

}

void BatchBuild::disconnect() {
	m_Server.disconnect();
}

void BatchBuild::setBuildRoot(string root) {
	m_BuildRootPath=root;

}

string BatchBuild::getBuildRoot() {
	return m_BuildRootPath;
}

void BatchBuild::setSharedRoot(string root) {
	m_SharedRootPath=root;

}

string BatchBuild::getSharedRoot() {
	return m_SharedRootPath;
}


string BatchBuild::timeToDir(time_t time) {
char dir[1024];
struct tm *today;

   today = localtime(&time);
   sprintf(dir,"%d_%02d_%02d_%02d.%02d.%02d",1900+today->tm_year,today->tm_mon+1,today->tm_mday,today->tm_hour,today->tm_min,today->tm_sec);
   return dir;
}



set<string> BatchBuild::getFolders(string dir) {
set<string> folders;
#ifdef _WIN32
    int handle;
    struct _finddata_t fileinfo;
    
        handle = _findfirst(dir.c_str(),&fileinfo);
        if(handle==-1) {
		    return folders;
		}
	    do {
		    if((fileinfo.attrib&_A_SUBDIR)!=0) {
			    string dir = fileinfo.name;
			    if(dir!="." && dir!="..") {
			        folders.insert(dir);
				}
			}
		} while(_findnext(handle,&fileinfo)!=-1);
	    _findclose(handle);
#else
        DIR *dp;
	struct dirent *dirp;
        struct stat statbuf;
	if( (dp = opendir(dir.c_str())) ==NULL) return folders;
	while( (dirp = readdir(dp)) !=NULL) {
  	    if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0) 
		     continue;
	    string dir = dirp->d_name;
		if(lstat(dir.c_str(),&statbuf) <0) continue;
		if(S_ISDIR(statbuf.st_mode) !=0) {
			folders.insert(dir);
		}
	}
	closedir(dp);
#endif
	return folders;

}

