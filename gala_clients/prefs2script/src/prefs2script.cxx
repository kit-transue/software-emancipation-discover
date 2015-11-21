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
//////////////////////////////////////////////////////////////////////////////////////////
//
// prefs2script --program to read a preference file and churn out batch/sh script commands
// useful for automating /scripting  modelbuilds
//
// Author:			Gregory Bronner
// Date:			2-20-98
//
// usage prefs2script -prefs preffilename -batch|-sh [outputfilename] [-seatbelt]
//
// preffilename:	pathname of input preffile.
// -batch :			produces DOS .BAT script
// -sh :			produces UNIX Bourne Shell script
// outputfile:		file where output gets dumped. if not specified (or invalid), output gets dumped to stdout
// -seatbelt:		causes projects to be built (parsed) one at a time--useful for overcoming parser crashes on big
//					projects...
//
// side effect:     sets	some env variables (ADMINDIR..) to whatever is in the pref file.
//
// notes:			may have memory leaks. Could be rather slow on a prefs with thousands of projects to be built one at
//					a time.
//					The settings parser can be pretty picky--please follow preffile conventions!!!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "settings.h"
#include "parray.h"
#include "pdupath.h"
#include "pdumem.h"
#include "pdustring.h"
#include "cstring.h"
#include "batchscriptfile.h"
#include "shscriptfile.h"

static enum Granularity {ONE, ALL}; // determines how many projects will be built per pass


//visually distinguishes echo statments from the rest of the output...	
void MakeBanner(ScriptFile * myScriptFile, const char * message);
void MakeBanner(ScriptFile * myScriptFile, const char * message)
{ 
	const char * BANNER="----------------------------------------------------------";
	
	myScriptFile->Echo(BANNER);
	myScriptFile->Echo(message);
	myScriptFile->Echo(BANNER);
}



char * recursive_pdexpandenv( const char * string);
//a function to recursively expand environment variables (especially in string...)
char * recursive_pdexpandenv( const char * string)
{
	char * pTempString=NULL;
	char * pTempStringOld=(char *) string;
	
	pTempString=pdexpandenv(string);
	while (strcmp(pTempString, pTempStringOld))
	{
		if (pTempStringOld!=string)
			FREE_MEMORY(pTempStringOld);
		pTempStringOld=pTempString;
		pTempString=pdexpandenv(pTempStringOld);
	}
	if (pTempStringOld!=string)
		FREE_MEMORY(pTempStringOld);
	
	return pTempString ;
}


//gets the value of the argument after number i if it isn't a switch
char * CheckNextArg(int i, int argc, char *argv[]);

char * CheckNextArg(int i, int argc, char *argv[])
{
	char * mypointer=NULL;
	if(i+1<argc)
	{
		if ((argv[i+1])[0]!='-')
		{
			mypointer=argv[i+1];
		}
	}
	return mypointer;
}

//returns the realpath of a given path.  It is the caller's responsibility to delete [] the pointer returned
char * CheckFile(char * inputfile);
char * CheckFile(char *inputfile)
{
	char * pRetVal=NULL;
	//first try using the input described
	pRetVal=pdrealpath(inputfile);
	//next try expanding env variables
	if (!pRetVal)
	{
		char *pTemp=pdexpandenv(inputfile);
		pRetVal=pdrealpath(pTemp);
		FREE_MEMORY(pTemp);
	}
 	// maybe it is in the current directory?
	if (!pRetVal)
	{
		const int MAX_PATH_LEN=1024;
		char *pTemp=new char[MAX_PATH_LEN];
		
		pTemp=pdgetcwd(pTemp, MAX_PATH_LEN);
		char *pTemp2=new char[strlen(pTemp)+strlen(inputfile)+2];
		if (pTemp2 && *pTemp)
		{
			strcpy(pTemp2, pTemp);
#ifdef WIN32
			strcat(pTemp2, "\\");
#else		
			strcat(pTemp2, "/");
#endif		
			strcat(pTemp2, inputfile);
			pRetVal=pdrealpath(pTemp2);
		}
		delete [] pTemp;
		delete [] pTemp2;
	}
	return pRetVal;
}

//returns the index of an argument matching myarg or 0 if it isn't found.
int findarg(const char * myarg, int argc, char *argv[]);

int findarg(const char * myarg, int argc, char *argv[])
{
	int i, nRet=0;
	for (i=1; i<argc; i++)
	{
		if (!(strcmp(myarg, argv[i])))
		{
			nRet=i;
			break;
		}

	}
	return nRet;
}


int main(int argc, char *argv[])
{
	const char * MESSAGE= "Syntax: prefs2script -prefs <prefsfilename> -batch|-sh [<outputfilename>] [-all]";
	const char * GROUPNAME = "psetPrefs";
	const char * PDFSEPS=" ";		   //used to separate pdfs in the buildpdf field
	const char * PROJSEPS="\\ \n\t";   //used to separate projects in project list fields
	char * pPrefFile=NULL;
	char * pOutputFile=NULL;
	
	enum Granularity {ONE, ALL}; // determines how many projects will be built per pass
	enum ScriptType {NONE, BATCH, SH};	 //add your own here.
	ScriptType OutputFileType=NONE;
	Granularity ProjPerPass=ONE;
	
	//parse the arguments...
	{
		int i;
		if (i=findarg("-prefs", argc, argv))
		{pPrefFile=CheckFile(argv[i+1]);}
		
		if (findarg("-all", argc, argv)) ProjPerPass=ALL;
		
		if (i=findarg("-batch", argc, argv))
		{ 
			OutputFileType=BATCH;
			pOutputFile=CheckNextArg(i,argc, argv);
		}
		else if (i=findarg("-sh", argc, argv))
		{
			OutputFileType=SH;
			pOutputFile=CheckNextArg(i, argc, argv);
		}
		
		if (!pPrefFile ||OutputFileType==NULL)
		{
			cerr << MESSAGE;
			exit(3);
		}
	}
	// now we open the settings file...
	
	Settings *mySettings=new Settings(GROUPNAME);
	if ((mySettings->open(pPrefFile)) == -1)
	{
		cerr << MESSAGE;
		exit(3);
	}
	
	ScriptFile * myScriptFile;
	switch (OutputFileType)
	{
	case BATCH:
		myScriptFile=new BatchScriptFile;
		break;
	case SH:
		myScriptFile=new ShScriptFile;
		break;
	default:
		cerr << "Reached end of switch statement";
		exit(3);
	}
	
	//now we set the env variables in both this program and the target script (to make
	// them expand correctly
	{
		const char * curmessage = "Setting environment variables needed to run DISCOVER";
		myScriptFile->Comment(curmessage);
		MakeBanner(myScriptFile, curmessage);
	}
	const int NUMENVVARIABLES =5;
	const char * ENVVARIABLES[NUMENVVARIABLES] = {"ADMINDIR", "sharedSrcRoot", "sharedModelRoot", "privateSrcRoot", "privateModelRoot"};
	
	
	for (int i=0; i<NUMENVVARIABLES; i++)
	{
		
		
		Settings::Item item( ENVVARIABLES[i]);
		if (mySettings->read(item) != -1)
		{  
			const char *pszValue=NULL;
			
			if (item.read(&pszValue) != -1 && pszValue)
			{
				//now we should have the environment variable's value...
				
				char * pExpandedString= recursive_pdexpandenv(pszValue);
				cstring varstring=ENVVARIABLES[i] +cstring("=") + pExpandedString;
				int nresult=pdputenv(varstring);
				
				myScriptFile->SetEnvironmentVariable(ENVVARIABLES[i], pExpandedString);
				FREE_MEMORY(pExpandedString);
			}
		}
	}
	
	//adding code to deal with CPP_DEBUG--env variable set to allow make rule output.
	{
		Settings::Item item("debugShowMakeRuleOutput");
		if (mySettings->read(item)!=-1)
		{
			const char *pszValue=NULL;
			if (item.read(&pszValue) != -1 && pszValue)
			{
				if (strcmp(pszValue, "yes")==0)
				{myScriptFile->SetEnvironmentVariable("CPP_DEBUG", "1");}
			}
		}
	}
	

	myScriptFile->Comment("End of Environment variables section.");
	
	// Env variables should have been taken care of.  Now we do the various passes:
	
	// need to first get the pdf (s)
	char  * pPDFString=NULL;
	{
		Settings::Item item("pdfFileBuild");
		
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				pPDFString=pdstrdup(tempstring);
			}
			else
			{
				cerr<<"No Build PDF specified in "<< pPrefFile <<"."<<endl<<"Aborting"<<endl;
				exit(3);
			}
			
		}
	}
	
	
	//now we'll set them up properly for passing on to our script file..(note that delimeters are handled by the script file).
	
	cstring pPDFVals;
	char *temppointer;
	char *tempstring=new char[strlen(pPDFString)+1];
	strcpy(tempstring, pPDFString);
	temppointer=strtok(tempstring, PDFSEPS);
	while (temppointer!=NULL)
	{
		char * expfile=NULL;
		expfile=recursive_pdexpandenv(temppointer);
		
		pPDFVals+=" -pdf ";	  //the string has to be written "-pdf pdffile1 -pdf pdffile2"
		pPDFVals+=expfile;
		FREE_MEMORY(expfile);
		//advance token
		temppointer=strtok(NULL, PDFSEPS);
	}
	delete [] tempstring;
	tempstring=NULL;
	FREE_MEMORY(pPDFString);
	
	
	// set up the preferences string
	
	cstring pPrefsString = " -prefs ";
	pPrefsString+=pPrefFile;
	
	
	//now we set up the command string used to invoke the executible
#ifdef _WIN32
	const char *pDiscoverCommand ="pset_server.exe";
#else
	const char *pDiscoverCommand ="discover";
#endif;
	
	cstring pPSETHOME;
	const char * pszPSETHOME= pdgetenv("PSETHOME"); //memory leak?
	pPSETHOME=cstring(pszPSETHOME);
	
	cstring pCommand= pPSETHOME+ "/bin/" +  pDiscoverCommand + " -batch ";
	
	// now we figure out the forgive parsing errors tag...
	cstring pForgiveString="";
	
	{
		Settings::Item item("acceptSyntaxErrors");
		
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no") && !strstr(tempstring, "No"))
					pForgiveString+=" -forgive ";		
			}
		}
		
	}
	
	// the general idea of each pass is
	// 1. Should I do this pass?
	// 2  if yes, what projects should I do
	// 3 if I should do projects, parse project list
	// 4 print a banner and execute relevant commands
	
	//pass one: -save_proj
	{
		Settings::Item item("doPassSaveProj");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no")&& !strstr(tempstring, "No"))
				{	
					const char * curmessage="Peforming Save Project Pass";
					myScriptFile->Comment(curmessage);
					MakeBanner(myScriptFile, curmessage);
					myScriptFile->Command((const char *)(pCommand),(const char*)( pPrefsString + pPDFVals + " -save_proj")  );
				}
			}
			//delete tempstring;
			tempstring=NULL;
		}
	}
	
	//pass two : dependency
	{
		Settings::Item item("doPassDependency");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no")&& !strstr(tempstring, "No"))
				{
					const char * curmessage="Performing Analyze Dependencies pass";
					myScriptFile->Comment(curmessage);
					MakeBanner(myScriptFile, curmessage);
					cstring TokenString=" ";
					
					Settings::Item subItem("projectListDependency");
					if (mySettings->read(subItem) != -1)
					{
						
						const char * tempstring2=NULL;
						if(subItem.read(&tempstring2)!=-1 && tempstring2)
						{
							
							//note:all this crap is implemented to make sure that we convert newlines
							// and crap into a legitimate one -line project list...
							char * tempprojstring= new char[strlen(tempstring2)+1];
							strcpy(tempprojstring, tempstring2);
							
							char * CurProj=strtok(tempprojstring, PROJSEPS);
							
							while (CurProj!=NULL)
							{
								
								TokenString+=CurProj;
								TokenString+=" ";
								CurProj=strtok(NULL, PROJSEPS);
							}
							delete [] tempprojstring;
							tempprojstring=NULL;	
						}
						tempstring2=NULL;	
					}
					
					myScriptFile->Command((const char *) (pCommand) ,
						(const char*)( pPrefsString + pPDFVals + " -dependency " + TokenString +pForgiveString));
				}
			}
			tempstring=NULL;
		}
	}
	//pass three: build				
	{
		Settings::Item item("doPassProj");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no")&& !strstr(tempstring, "No"))
				{
					Settings::Item subItem("projectList");
					if (mySettings->read(subItem) != -1)
					{
						const char * tempstring2=NULL;
						if(subItem.read(&tempstring2)!=-1 && tempstring2)
						{
							const char * curmessage="Performing Build Project pass";
							myScriptFile->Comment(curmessage);
							MakeBanner(myScriptFile, curmessage);
							
							//note:all this crap is implemented to make sure that we convert newlines
							// and crap into a legitimate one -line project list...
							char *tempprojstring=pdstrdup(tempstring2);
							char * CurProj = strtok(tempprojstring, PROJSEPS);
							if (ProjPerPass==ONE)
							{ //can't use a loop with strtok because ScriptFile uses it.

								parray * pa = new parray(256);
							
								

								while (CurProj)
								{
									pa->insert((void *) (CurProj));
									CurProj=strtok(NULL, PROJSEPS);
								}

								int nSize=pa->size();
								for (int i=0; i<nSize; i++)
								{
									const char * pLine = (const char *) pPrefsString+pPDFVals+" -proj "+ (const char *) (*pa)[i]+pForgiveString;
									myScriptFile->Command( (const char *) (pCommand), pLine);
								}
								delete pa;

							

							}
							else
							{
								cstring TokenString="";

								while (CurProj!=NULL)
								{
									TokenString+=CurProj;
									TokenString+=" ";
									CurProj=strtok(NULL, PROJSEPS);
								}
								
								myScriptFile->Command((const char *) (pCommand ),
								(const char *)( pPrefsString + pPDFVals + " -proj "+TokenString +pForgiveString));
	
							}
		
							FREE_MEMORY(tempprojstring);

							tempprojstring=NULL;
							
						}
						tempstring2=NULL;
					}
				}
			}
		}	
	}
	//pass four: headers
	{
		Settings::Item item("doPassHeaders");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no")&& !strstr(tempstring, "No"))
				{
					const char * curmessage="Performing Build Headers pass";
					myScriptFile->Comment(curmessage);
					MakeBanner(myScriptFile, curmessage);
					
					cstring TokenString=" ";
					
					Settings::Item subItem("projectListHeaders");
					if (mySettings->read(subItem) != -1)
					{
						
						const char * tempstring2=NULL;
						if(subItem.read(&tempstring2)!=-1 && tempstring2)
						{
							
							//note:all this crap is implemented to make sure that we convert newlines
							// and crap into a legitimate one -line project list...
							char * tempprojstring= new char[strlen(tempstring2)+1];
							strcpy(tempprojstring, tempstring2);
							
							char * CurProj=strtok(tempprojstring, PROJSEPS);
							
							while (CurProj!=NULL)
							{
								TokenString+=CurProj;
								TokenString+=" ";
								CurProj=strtok(NULL, PROJSEPS);
							}
							
							delete [] tempprojstring;
							tempprojstring=NULL;	
						}
						tempstring2=NULL;	
					
					}
					//this is outside the brace because it is perfectly legal to do -headers with no explicitly specified projects.
					myScriptFile->Command((const char *) (pCommand),(const char *)( pPrefsString + pPDFVals + " -headers " + TokenString +pForgiveString));
					
				}
			}
			tempstring=NULL;
		}
	}
	//pass 5: dfa				
	{
		Settings::Item item("doBuildDFA");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "0")&& !strstr(tempstring, "no"))
				{
					//you have to build the dfa for all projects.
					Settings::Item subItem("projectList");
					if (mySettings->read(subItem) != -1)
					{
						const char * tempstring2=NULL;
						if(subItem.read(&tempstring2)!=-1 && tempstring2)
						{
							const char * curmessage="Performing Build DFA pass";
							myScriptFile->Comment(curmessage);
							MakeBanner(myScriptFile, curmessage);
							
							//note:all this crap is implemented to make sure that we convert newlines
							// and crap into a legitimate one -line project list...
							char *tempprojstring=pdstrdup(tempstring2);
							char * CurProj = strtok(tempprojstring, PROJSEPS);
							if (ProjPerPass==ONE)
							{ //can't use a loop with strtok because ScriptFile uses it.

								parray * pa = new parray(256);
								while (CurProj)
								{
									pa->insert((void *) (CurProj));
									CurProj=strtok(NULL, PROJSEPS);
								}

								int i = 0;
								int nSize=pa->size();
								// -- 1. initialize
								for (i=0; i<nSize; i++)
								{
									const char * pLine = (const char *) pPrefsString+pPDFVals+" -dfa_init "+
										(const char *) (*pa)[i];
									myScriptFile->Command( (const char *) (pCommand), pLine);
								}
								// -- 2. link
								for (i=0; i<nSize; i++)
								{
									const char * pLine = (const char *) pPrefsString+pPDFVals+" -dfa "+
										(const char *) (*pa)[i];
									myScriptFile->Command( (const char *) (pCommand), pLine);
								}
								delete pa;
							}
							else
							{
								cstring TokenString="";
								while (CurProj!=NULL)
								{
									TokenString+=CurProj;
									TokenString+=" ";
									CurProj=strtok(NULL, PROJSEPS);
								}
								// -- 1. initialize
								myScriptFile->Command((const char *) (pCommand ),
								(const char *)( pPrefsString + pPDFVals + " -dfa_init "+TokenString));
								// -- 2. link
								myScriptFile->Command((const char *) (pCommand ),
								(const char *)( pPrefsString + pPDFVals + " -dfa "+TokenString));
							}
							FREE_MEMORY(tempprojstring);

							tempprojstring=NULL;
						}
						tempstring2=NULL;
					}
				}
			}
		}	
	}
	//pass six : save root			
	{
		Settings::Item item("doPassSaveRoot");
		if (mySettings->read(item) !=-1 )
		{
			const char *tempstring=NULL;
			if (item.read(&tempstring)!=-1 && tempstring)
			{
				if (!strstr(tempstring, "no")&& !strstr(tempstring, "No"))
				{
					const char * curmessage="Performing Save Root pass";
					myScriptFile->Comment(curmessage);
					MakeBanner(myScriptFile, curmessage);
					myScriptFile->Command((const char *)(pCommand ),(const char *)( pPrefsString + pPDFVals + " -save_root")  );
				}
			}
			//delete tempstring;
			tempstring=NULL;
		}
	}
	
	MakeBanner(myScriptFile, "End of model building script");
	myScriptFile->EchoCommands(1);
	
	ostream *mystream;
	
	if (pOutputFile)
	{
		mystream= new ofstream(pOutputFile);
		if (!mystream)
		{
			mystream=&cout;
			pOutputFile=NULL;
		}
		
	}
	else
		mystream=&cout;
	
	// print the whole damn thing out...
	*mystream << *myScriptFile;
	mySettings->close(0);
	if (pOutputFile)
		delete mystream;
	delete myScriptFile;
	FREE_MEMORY(pPrefFile);
	return 0;	
}
