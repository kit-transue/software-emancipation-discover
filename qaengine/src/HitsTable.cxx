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
// HitsTable.cpp: implementation of the HitsTable class.
//
//////////////////////////////////////////////////////////////////////
#include "HitsTable.h"
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
#include <fcntl.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include <sys/stat.h>

HitsTable::HitsTable() {
    m_HitsCounter = 0;
}

HitsTable::~HitsTable() {
multimap<string,Hit*>::iterator i;

    for(i=m_QueryHash.begin();i!=m_QueryHash.end();i++) {
	    Hit* pHit =i->second;
		delete pHit;
	}

}

int HitsTable::load(string filename) {
char buf[20000];
char* ch;
int inBuffer=0;
int fieldIndex = 0;
char fieldVal[20001];
char* val; 
char* max; 
string userAttrName;
int isName = 1;
string id;
Hit* newHit;


   val = fieldVal;
   max=val+20000;
   int fd = open(filename.c_str(),O_RDONLY);
   if(fd==-1) return -1;
   newHit = new Hit();
   while(1) {
	  if(inBuffer==0) {
		  inBuffer = read(fd,&buf,20000);
		  if(inBuffer<=0) break;
		  ch = buf;
	  } 
	  inBuffer--;
	  if(*ch=='\r') {
		  ch++;
		  continue;
	  }
	  if(*ch=='\n') {
		  if(strlen(fieldVal)>0) {
			  *val=0;
		      switch(fieldIndex) {
		          case 0 : id     = fieldVal;                       break;
		          case 1 : newHit->setName(fieldVal);               break;
		          case 2 : newHit->setModule(fieldVal);             break;
		          case 3 : newHit->setLine(atoi(fieldVal)); break;
		          case 4 : newHit->setAstPath(fieldVal);            break;
			      default: if(isName) {
				               isName = 0;
					    	   userAttrName = fieldVal; 
						   } else {
						       isName = 1;
							   if(userAttrName.length()>0) {
						           (*newHit->getAttributes())[userAttrName]=fieldVal;
							   }
						   }
					       break;
			  }
		  }

		  // Hit is copmpletely filled, we need to put it into 
		  // the hits vector and queries hashtable
		  if(fieldIndex>=4) {
			  append(id,newHit);
		  } else {
			  delete newHit;
		  }
		  fieldIndex=0;
		  val=fieldVal;
		  isName = 1;
		  newHit = new Hit();
		  ch++;
		  continue;
	  }
	  if(*ch=='\t') {
		  // New hit field detected
		  *val = 0;
		  switch(fieldIndex) {
		      case 0 : id     = fieldVal;                       break;
		      case 1 : newHit->setName(fieldVal);               break;
		      case 2 : newHit->setModule(fieldVal);             break;
		      case 3 : newHit->setLine(atoi(fieldVal)); break;
		      case 4 : newHit->setAstPath(fieldVal);            break;
			  default: if(isName) {
				           isName = 0;
						   userAttrName = fieldVal; 
					   } else {
						   isName = 1;
						   if(userAttrName.length()>0&&strlen(fieldVal)>0) {
						       (*newHit->getAttributes())[userAttrName]=fieldVal;
						   }
					   }
					   break;

		  }
		  fieldIndex++;
		  val = fieldVal;
		  ch++;
		  continue;
	  }
	  *val=*ch++;
	  if(val!=max) val++;
   }
   delete newHit;
   close(fd);
   return 0;
}

int HitsTable::save(string filename) {
char buf[20000];

   int fd = creat(filename.c_str(),S_IWRITE);
   if(fd==-1) return -1;
   multimap<string,Hit *>::iterator i;
   for(i= m_QueryHash.begin();i!=m_QueryHash.end();i++) {
	   string id = i->first; // key
	   Hit* hit = i->second; //value

	   strcpy(buf,id.c_str());
	   strcat(buf,"\t");

	   strcat(buf,hit->getName().c_str());
	   strcat(buf,"\t");

	   strcat(buf,hit->getModule().c_str());
	   strcat(buf,"\t");

	   char num[100];
	   sprintf(num,"%d",hit->getLine());
	   strcat(buf,num);
	   strcat(buf,"\t");

	   strcat(buf,hit->getAstPath().c_str());
       PropertiesMap::iterator i;
       for(i=hit->getAttributes()->begin();i!=hit->getAttributes()->end();i++) {
	       strcat(buf,"\t");
	       strcat(buf,i->first.c_str());
	       strcat(buf,"\t");
	       strcat(buf,i->second.c_str());
	   }
	   strcat(buf,"\r\n");
	   write(fd,buf,strlen(buf));
   }
   close(fd);
   return 0;
}

HitsSet HitsTable::search(string queryId, ScopeSet scope) {
HitsSet hits;

   if(queryId=="") return hits;
   pair<multimap<string,Hit *>::iterator,multimap<string,Hit *>::iterator> range;
   range = m_QueryHash.equal_range(queryId);

   ScopeSet::iterator j;
   for(j=scope.begin();j!=scope.end();j++) {
 	   multimap<string,Hit*>::iterator i;
       for(i=range.first;i!=range.second;i++) {
	 	   Hit* pHit =i->second;
           if(pHit->getModule().compare(0,j->length(),*j)==0) {
		   	   hits.push_back(pHit);
		   }
	   }
   }
   return hits;
}


int HitsTable::count(string queryId, ScopeSet scope) {
int hits=0;

   if(queryId=="") return hits;
   pair<multimap<string,Hit *>::iterator,multimap<string,Hit *>::iterator> range;
   range = m_QueryHash.equal_range(queryId);

   ScopeSet::iterator j;
   for(j=scope.begin();j!=scope.end();j++) {
 	   multimap<string,Hit*>::iterator i;
       for(i=range.first;i!=range.second;i++) {
	 	   Hit* pHit =i->second;
           if(pHit->getModule().compare(0,j->length(),*j)==0) {
		   	   hits++;
		   }
	   }
   }
   return hits;
}


void HitsTable::append(string query, Hit *hit) {
   m_Hits.push_back(hit);
   m_QueryHash.insert(multimap<string,Hit*>::value_type(query,hit));
   hit->setIndex(m_HitsCounter);
   m_IndexHash[m_HitsCounter++]=hit;
}

Hit* HitsTable::find(int index) {
	map<int,Hit*>::iterator i = m_IndexHash.find(index);
	if(i==m_IndexHash.end()) return NULL;
	return i->second;

}


//------------------------------------------------------------------
// This member function will try to locate hits in this table 
// which match the given hit (using AST path)
//------------------------------------------------------------------
HitsSet HitsTable::compare(string query,Hit hit) {
HitsSet matchingHits;
HitsSet thisModuleHits;

   ScopeSet scope;
   scope.insert(hit.getModule());
   thisModuleHits = search(query,scope);
   HitsSet::iterator i;
   for(i=thisModuleHits.begin();i!=thisModuleHits.end();i++) {
	   Hit* h = *i;
	   if(*(h->getAstPathPtr())==*(hit.getAstPathPtr())) {
		   matchingHits.push_back(h);
	   }
   }
   return matchingHits;
}
//------------------------------------------------------------------
