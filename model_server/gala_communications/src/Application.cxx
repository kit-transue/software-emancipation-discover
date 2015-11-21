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
#include "cLibraryFunctions.h"
#include "machdep.h"
#include <vport.h>

#include vstartupHEADER
#include vstdlibHEADER
#include vsessionHEADER
#include vserviceHEADER
#include vscrapHEADER
#include vdasservHEADER
#include vstdioHEADER
#include vlistHEADER
#include vregexpHEADER
#include vnoticeHEADER
#include vcommHEADER

#define CBFN

#include "../include/Application.h"
#include "../include/Args.h"
#include <tcl.h>

#include "machdep.h"

#include <iostream.h>

/* function defined at the bottom of this file. useful for debugging communications */

StartNotifier* Application::starter_;
int Application::stopCountAction = 0; //initialize static class member
int Application::nCalledCount = 0; //initialize static class member
int Application::traceMode = 0; //initialize static class member
vbool Application::cannot_send = vFALSE; //initialize static class member
vsession* Application::sessionBroker_;
Application* Application::list_=NULL;
#ifdef _WIN32
   #ifdef CHILD_BROKER
      static PROCESS_INFORMATION pi;
   #endif
#endif

vlist* Application::service_list = NULL;
vchar* Application::regexp_match = NULL;

void Application::matchNotify (vsessionStatement *statement,
				vsessionStatementEvent *event)
{

    Application* app = (Application*) statement->GetData ();
    vsession* the_session = NULL;
    
    vsessionStatementEventCode code =
	vsessionStatement::GetEventCode(event);
    
    
    if (code == vsessionStatementEVENT_COMPLETED) {
	   SetCannotSend(vFALSE);
       int nMatches;
	   vscrap **arrayMatches=0;
	   vsessionStatement::GetReturnValue (statement,&nMatches, &arrayMatches);
	   if (nMatches > 0) {
	      the_session = new vsession;
	      the_session->SetAttributesFromScrap (arrayMatches[0]);
          the_session->SetAttributeTag(vsession_Transport, vcommAddress::GetTcpTransportName());
	      app->setSession (the_session);
	      int ii;
	      for(ii=0;ii<nMatches;ii++) {
		   // comment for HP (crash on startup otherwise)
		   // delete arrayMatches[ii];
	      }
	      if(arrayMatches!=NULL)
		  vmemFree(arrayMatches);
	      the_session->Begin();
	   } 
	   app->send_status = 1;
    } else {
	   app->send_status = 0;
    }
    app->statement_to_send = NULL;
}

int Application::SendBlock(vsessionStatement *statement)
{
    statement_to_send = statement;
    statement->Send();
    if(statement_to_send)
	send_status = !statement->Block(NULL);
    return send_status;
}

void Application::setSession (vsession* session)
{
    the_session_ = session;
}

void Application::BrokerNotify(vsession *session, vsessionEvent *event)
{
    vsessionEventCode code = vsessionGetEventCode(event);
    if(code == vsessionEVENT_CANNOT_BEGIN) {
	    SetCannotSend(vTRUE);
    }
}

Application::Application () {
    service_ = NULL;
    name_[0] = 0;
    the_session_ = NULL;
    interp_ = Tcl_CreateInterp();
    Tcl_Init (interp_);
    eval_result_ = NULL;

    next_ = Application::list_;
    Application::list_ = this;

    OSapi_time( (time_t *)&m_RegisterTime );

    connected = 0;
    cannot_send = vFALSE;

    main_sig_ = new vsignature;
    main_sig_->SetTag (vnameInternGlobalLiteral("main"));
    main_sig_->SetReturnDatatag (vdatatag::GetScrap());
    main_sig_->ConstructArgs (vdatatag::GetScrap(), NULL);
    
    if (sessionBroker_ == NULL ) {
        #ifdef _WIN32
           #ifdef CHILD_BROKER
		      STARTUPINFO si;
		      SECURITY_ATTRIBUTES saProcess, saThread;
		      ZeroMemory(&si,sizeof(si));
		      si.cb=sizeof(si);
			  si.wShowWindow=SW_HIDE;
			  si.dwFlags=STARTF_USESHOWWINDOW;


		      saProcess.nLength = sizeof(saProcess);
		      saProcess.lpSecurityDescriptor = NULL;
		      saProcess.bInheritHandle=TRUE;

		      saThread.nLength=sizeof(saThread);
		      saThread.lpSecurityDescriptor =NULL;
		      saThread.bInheritHandle =FALSE;

		      int code=CreateProcess(NULL,"vdasserv",&saProcess,&saThread,FALSE,0,NULL,NULL,&si,&pi);
			  CloseHandle(pi.hThread);
           #endif
        #endif

	    sessionBroker_ = new vsession;
	    sessionBroker_->SetAttributesFromScrap(vdasserv::CreateAttributeScrap());
        sessionBroker_->SetAttributeTag(vsession_Transport, vcommAddress::GetTcpTransportName() );
	    sessionBroker_->SetObserveSessionProc(BrokerNotify);
	    sessionBroker_->Begin();
	}

	vsessionStatement *reqstatement;
	vscrap *scrapSpec;
	
	scrapSpec = vscrapCreateDictionary();
	reqstatement = vdasserv::MakeMatchStatement(sessionBroker_,scrapSpec);
	reqstatement->Send();

	reqstatement->Block (NULL);
	delete reqstatement;
}


Application::~Application () {
	// if service attached is not removed up to this moment
	if(service_!=NULL) unregisterService();

    // we need to end this class session and delete it
    if (the_session_) {
		the_session_->End();
        delete the_session_;
        the_session_ = NULL;
    }

	// removing this class from the linked list
    if (Application::list_ == this) {
	   Application::list_ = this->next_;
	} else {
	for (Application* app = Application::list_; app->next_; app = app->next_)
	    if (app->next_ == this) {
		app->next_ = this->next_;
                break;
            }
    }


	// if linked list is empty, we need to stop broker session and disable the service
    if (Application::list_ == NULL) {
	   // cleaning and removing service list if any
        if (service_list) {
	      if (service_list->GetRowCount()) {
	         vsession* next;
	         for (int i = 0; i < service_list->GetColumnCount();i++) {
		        next = (vsession*) service_list->GetCellValue(0, i);
                if(next!=NULL) {
                   next->End();
		           delete next;
			    }
	         }
	      }
	      delete service_list;
	      service_list=NULL;
        }
		// close broker session
		if(sessionBroker_!=NULL) {
	       sessionBroker_->End();
	       delete sessionBroker_;
	       sessionBroker_ = NULL;
           #ifdef _WIN32
             #ifdef CHILD_BROKER
		      TerminateProcess(pi.hProcess,0);
             #endif
           #endif
		}
    }

}

//-----------------------------------------------------------------------------------------
// This is for abnormal exit in the case pset_server regular timer detected that client
// connection lost and more than this, we can't exit pset_server using normal GALAXY
// StopProcessing(). In this case QuitTimer will run this function after 5 seconds delay
// to terminate all opened connections.
//------------------------------------------------------------------------------------------
void Application::ShutdownOnError() {
Application* pApp;

    // We will observe all linked list of applications and we will close all elements
    // sessions.
	pApp=Application::list_;
	while(pApp != NULL) {
		delete pApp;
		pApp=Application::list_;
	}
}
//------------------------------------------------------------------------------------------


vsession* Application::mySession ()
{
    return the_session_;
}

vsignature* Application::getSignature ()
{
    return main_sig_;
}

Application* Application::findApplication (vsession* session)
{
    for (Application* app = Application::list_; app; app = app->next_)  {
        if (session->service == app->service_)
	    return app;
    }
    return NULL;
}

Application* Application::findApplication (const char* name, const char* group)
{
    return findApplication ((vchar*)name, (vchar*)group);
}

Application* Application::findApplication (const vchar* name, const vchar* group)
{    
	vchar fullname[256];

	if (vcharSearchChar (name, ':')) {
		sprintf ((char*)fullname, "%s", name);
	} else {
		if (group == NULL)
		    group = (const vchar*)Application::getGroupName();

	    sprintf ((char*)fullname, "%s:%s", group, name);
	}

    for (Application* app = Application::list_; app; app = app->next_)  {
        if (vcharCompare(fullname, app->name_) == 0)
	    return app;
    }
    return NULL;
}

int Application::registerService (const char* group, const char* service, Server* server, StartNotifier* notifier) {
    vchar name[1024];

	if(service_!=NULL) return -1;
	if (vcharSearchChar (service, ':')) {
		vcharCopyFast (service, name);
	} else {    
		if (!*group)
			group = Application::getGroupName();
        vcharCopyFast (group, name);
        vcharAppendFromLiteral (name, ":");
        vcharAppendFast (name, service);
	}

    if (notifier) starter_ = notifier;

    service_ = new vservice;

    vservicePrimitive* prim = new vservicePrimitive;
    prim->SetHandler (server);
    prim->SetSignature (main_sig_);
    service_->AddPrimitiveOwned (prim);
  
    service_->SetAttributeScribe(vname_Name, vcharScribeLiteral("DISCOVER"));
    service_->SetAttributeScribe(vname_Title, vcharScribeLiteral(name));
    
	//for debugging only!!!!
	//(service_->GetTemplateSession())->SetObserveSessionProc(ObserveSession);
	// end debugging
	service_->Enable();
    vdasserv::RegisterService(service_);

    vcharCopyFromSystem (name, name_);

    return 0;
}

int Application::unregisterService () {
	if(service_==NULL) return -1;
    vdasserv::UnregisterService (service_);
    service_->Disable();
	delete service_;
	service_=NULL;
    return 0;
}

int Application::connect (const char* group, const char* service)
{
    vchar name[1024];
    
	// full name of the service will be formed: <group name> : <service name>
	if (vcharSearchChar (service, ':')) {
		vcharCopyFast (service, name);
	} else { 
		if (!group || !*group)
			group = Application::getGroupName();
        vcharCopyFast (group, name);
        vcharAppendFromLiteral (name, ":");
        vcharAppendFast (name, service);
	}

    vsessionStatement	*statement;

    // creating specification of the service to call broker
    vdict dictValues(vnameHash);    
    vdatatag *datatagValues = vdatatag::CreateDict ();
    datatagValues->SetDictElementDatatag (vname_Title, vdatatag::GetTag());
    dictValues.SetElem (vname_Title, vnameInternGlobalLiteral ((const char*)name));
    datatagValues->SetDictElementDatatag (vname_Name, vdatatag::GetTag());
    dictValues.SetElem (vname_Name, vnameInternGlobalLiteral ("DISCOVER"));
    
    // translating this spec into service broker format
    statement = vdasserv::MakeMatchStatement(sessionBroker_,
					                         vdatatagScrap::CreateFromValue(datatagValues, &dictValues));
    
    // result will be obtained by matchNotify callback
    statement->SetObserveStatementProc (matchNotify);
    // statement->SetDestroyWhenFinished (vTRUE);
    // callback will put it's results into this class
    statement->SetData (this);
    // we do not need dictionary any more - vdasserv statement is already formed.
    delete datatagValues;
    // calling service broker syncronously - new session will be started and pointer to it will
	// be placed into the_session variable.
	SetCannotSend(vTRUE);
    connected = SendBlock(statement);
    // we do not need statement any more
	delete statement;
    vcharCopyFromSystem (name, name_);

    return connected;
}

int Application::disconnect ()
{
     connected = 0;
     return 0;
}

vbool Application::IsConnectionLost()
{
	return(cannot_send);
}

void Application::SetCannotSend(vbool value)
{
	cannot_send = value;
}
 

int Application::eval (vstr* str)
{
    int code = Tcl_Eval (interp_, (char*)str);
    if (eval_result_) vstrDestroy (eval_result_);
    eval_result_ = vstrClone ((vchar*)Tcl_GetStringResult(interp_));
    return NULL;
}

vstr* Application::getEvalResult()
{
    return eval_result_;
}

const char* Application::getGroupName()
{
	static char groupname[256];
    static int inited = 0;
    
    char group[256];
	char *name;

    if (!inited) {
	    inited = 1;

	    DIS_args->findValue("-group", group);
	    char* user = getenv("USER");
	    char* username = getenv("USERNAME");
	    char* disgroup = getenv("DIS_GROUP");
	    if (!group[0]) {
	        if (disgroup)
		        name = disgroup;
	        else if (user)
		        name = user;
	        else if (username)
		        name = username;
    	    else {
	    	    fprintf (stderr, "Application group name not found; exiting...\n");
		        exit (EXIT_FAILURE);
	    }
	    char host[1000];
            OSapi_gethostname (host, 1000);
	    sprintf (groupname, "%s-%s-%u", name, host, OSapi_getpid());

        } else strcpy (groupname, group);

    }


    return &groupname[0];
}


Tcl_Interp* Application::interp ()
{
    return interp_;
}

void Application::set_interp (Tcl_Interp* interp)
{
    if (interp_) Tcl_DeleteInterp(interp_);
    interp_ = interp;
}


void Application::StopCountAction(int flag)
{
    if (flag)
	stopCountAction++;
    else
	stopCountAction--;
}

void Application::IncrementCount()
{
    if (!stopCountAction)
        if ((nCalledCount == 0) && starter_) 
	    (starter_)(1);

    nCalledCount++;
}

void Application::DecrementCount()
{
    if ((nCalledCount == 1) && starter_)
        (*starter_)(0);

    nCalledCount = (nCalledCount > 0) ? nCalledCount - 1 : 0;
    // Add change button state code here   
}


/*------- Building service list code - jwr 7/96 ---------*/

void Application::buildServiceList(const vchar* name,
				   const vchar* titleRegexp)
{
    delete regexp_match;

	// cleaning and removing service list
    if (service_list) {
	   if (service_list->GetRowCount()) {
	      vsession* next;
	      for (int i = 0; i < service_list->GetColumnCount();i++) { 
		     next = (vsession*) service_list->GetCellValue(0, i);
			 next->End();
		     delete next;
	      }
	   }
	   delete service_list;
    }

    regexp_match = new vchar[vcharLength(titleRegexp) + 1];
    vcharCopy(titleRegexp, regexp_match);

    vsessionStatement* statement;
    vscrap* scrapSpec;

    scrapSpec = vscrapCreateDictionary();
    vscrapStoreElement(scrapSpec, vname_Name, vscrapFromString(name));
    statement = vdasserv::MakeMatchStatement(sessionBroker_,scrapSpec);
    statement->SetObserveStatementProc(Application::serviceListNotify);
    //statement->SetDestroyWhenFinished (vTRUE);

    statement->Send();
    statement->Block (NULL);
	delete statement;
}



    
void Application::serviceListNotify(vsessionStatement* statement,
				    vsessionStatementEvent *event)
{
    int code = vsessionGetStatementEventCode(event);
    if (code == vsessionStatementEVENT_COMPLETED) {
	vsession* session;
	int nArray;
	vscrap** array;
	
	vregexp regexp;
	
	vexWITH_HANDLING {
	    regexp.SetPatternScribed(vcharScribe(regexp_match));
	}
	vexON_EXCEPTION {
	    cerr << "Bad Regular expression" << endl;
	    return;
	}
	vexEND_HANDLING;
	
	vsessionGetStatementReturnValue(statement, &nArray, &array);
	vscribe* value;
	service_list = new vlist;
	for (int i = 0; i < nArray; i++) {
	    vexWITH_HANDLING {
		session = new vsession;
		session->SetAttributesFromScrap(array[i]);
                session->SetAttributeTag(vsession_Transport, vcommAddress::GetTcpTransportName());
		value = session->FindAttributeScribe(vname_Title);
		int dummySize;
		vchar* string = (vchar*) value->GetString();
		if (value &&
		    (regexp.Search(string, vcharLength(string), &dummySize))) 
    {
         //Make sure that string is not already there
        int fFound = 0;
        for (int ii = 0; ii < service_list->GetColumnCount(); ii++)
        {
          vsession* session = (vsession*) service_list->GetCellValue(0, ii);
          vscribe* valueScribe = session->FindAttributeScribe(vname_Title);
          if (valueScribe) 
          {
            vchar* valueChar = (vchar*) valueScribe->GetString();
            if (vcharCompare(valueChar, string) ==0)
              fFound = 1;
          }
        }
        if( !fFound )
        {
		      int col = service_list->GetColumnCount();
		      service_list->InsertColumn(col);
		      service_list->SetCellValue(0, col, session);
        }
		}
		else delete session;
		delete value;
	    }
	    vexON_EXCEPTION;
	    vexEND_HANDLING;
	}
	delete regexp_match;
	regexp_match = NULL;
    }
}


int Application::getServiceListSize()
{
    if (service_list)
	return service_list->GetColumnCount();
    else
	return 0;
}


const vchar* Application::getServiceListAttributeValue(int index, const vchar* tagName)
{
    if (service_list) {
	if ((index < 0) || (index >= service_list->GetColumnCount()))
	    return NULL;
	else {
	    vsession* session = (vsession*) service_list->GetCellValue(0, index);
	    vscribe* value = session->FindAttributeScribe(vnameInternGlobal(tagName));
	    if (value)
		return (vchar*) value->GetString();
	    else return NULL;
	}
    }
    else return NULL;
}



vsession* Application::getServiceListSession(int index)
{
    if (service_list && (index >= 0) && 
	(index < service_list->GetColumnCount())) {
	return (vsession*) service_list->GetCellValue(0, index);
    }
    else return NULL;
}



// returns -1 if the item is not found, index otherwise
int Application::getServiceListItemIndex(const vchar* attributeName,
			      const vchar* attributeValue)
{
    int valToRet = -1;
    
    if (service_list) {
	for (int i = 0; i < service_list->GetColumnCount(); i++){
	    vsession* session = (vsession*) service_list->GetCellValue(0, i);
	    vscribe* valueScribe = session->FindAttributeScribe(vnameInternGlobal(attributeName));
	    
	    if (valueScribe) {
		vchar* valueChar = (vchar*) valueScribe->GetString();
		if (vcharCompare(valueChar, attributeValue) ==0)
		    valToRet = i;
	    }
	}
    }

    return valToRet;
}
		    

// This function shows or hides Windows taskbar window
// for the current GALAXY application. It uses undocumented
// GALAXY global variable _vsysMSWTaskWindow
// The function does nothing on UNIX	    

#ifdef _WIN32
extern "C" int __stdcall ShowWindow( void *, int );
extern "C" void *_vsysMSWTaskWindow;
#endif /*_WIN32*/

void Application::ShowTaskbarWindow(int fShow)
{
#ifdef _WIN32
  int nShow = 0;//SW_HIDE
  if( fShow )
    nShow = 1;//SW_SHOWNORMAL
  if( _vsysMSWTaskWindow )
    ShowWindow( _vsysMSWTaskWindow, nShow );
#endif /*_WIN32*/
}

/* -----------getAllClients  added 1/27/98 by gbronner-------------------------
Counts clients attached to "cli" and "pmod" services.
we don't count the clients on the inform service because
a) it is just used for querying
b) there is some bug which keeps on adding sessions without deleting them.  I couldn't
figure it out-- any significant rewrite of the Application class should deal with this.
----------------------------------------------------------------------------------*/
int Application::getAllClients()
{
	Application* pCurrentApp=Application::list_;
	int nRet =-1;
	int nTotal=0;
	int nServiceNotFound =0;
	while (pCurrentApp)
	{	
		if (!strstr(((const char *)pCurrentApp->name_), "inform:"))
		{
			if(pCurrentApp->service_ )
			{
				vserviceSessionIterator iter;
				iter.Start( pCurrentApp->service_ );
				while( iter.Next() )
				{
					vsession *ssn = iter.GetValue();
					if( ssn && ssn->IsActive() )
						nTotal++;
				}
				iter.Finish();
			} 
			else 
			{ 
				nServiceNotFound=-1;
			}	
		}
		
		pCurrentApp=pCurrentApp->next_;	
	}
	if (!nServiceNotFound)
		nRet = nTotal;

	return nRet;	
}




/* this is the old function.  It isn't used*/
int Application::getNumberOfClients()
{
	int nRet =-1;
	int nTotal=0;
	
	if(service_ )
	{
		vserviceSessionIterator iter;
		iter.Start(service_ );
		while( iter.Next() )
		{
			vsession *ssn = iter.GetValue();
			if( ssn && ssn->IsActive() )
				nTotal++;
		}
		iter.Finish();
		nRet=nTotal;
	} 
	
	return nRet;
	
}
//This function returns a pointer to a gloabl static buffer
//allocated by C run-time. The callee should be aware of
//possible ovverrides of this buffer by subsequent calls
//to C run-time time functions
const char *Application::getTimeWhenRegistered()
{
  return OSapi_ctime( (time_t *)&m_RegisterTime );
}



//-------------------------------------------------------------------------------------
// debugging functions... to be relocated...
//Uncomment the following function if you want to be notified when each session starts and ends.
// also uncomment its declaration and its use
/*
void ObserveSession (vsession *mysession, vsessionStatementEvent *myevent)
{
	cerr <<"Observing Session: ";
	switch (vsessionStatement::GetEventCode (myevent))
	{
	case (vsessionEVENT_BEGIN):
		cerr << "New session beginning";
		break;
	case (vsessionEVENT_END):
		cerr << "Session now ending";
		break;
	default:
		cerr << "Unknown event in ObserveSession with code" << vsessionStatement::GetEventCode(myevent) << endl;
		
	}
	cerr<<endl;
	
}
*/




//----------------------------------------------------------------------

#ifdef CBFN

callbackEvent::callbackEvent(callbackFunction fn)
{
    cbfn = fn;
    cbfn1 = 0;
}

callbackEvent::callbackEvent(callbackFunction1 fn, void* myData)
{
    cbfn = 0;
    cbfn1 = fn;
    this->data = myData;
}

callbackEvent::~callbackEvent()
{
}

int callbackEvent::Handle()
{
    if (cbfn) (*cbfn)();
    if (cbfn1) (*cbfn1)(data);
    //destroy event
    //delete this;
    return 0;
}

#endif















 
//----------------------------------------------------------------------

int dis_register_service (const vchar* group, const vchar* service)
{
    return 0;
}

int dis_unregister_service (int app_id)
{
    return 0;
}

static const char *CRITERIA = "PSr\neaCcs/\\mMh.i"; 
int generate_checksum( const char *buff, int nLen )
{
  int nRet = 0;
  int nLetters = strlen( CRITERIA );
  int *Sums = new int [nLetters];
  if( Sums )
  {
    memset( Sums, 0, nLetters*sizeof(int) );
    int i;
    for( i=0; i<nLen; i++ )
    {
      const char *pFound = strchr( CRITERIA, buff[i] );
      if( pFound )
        Sums[ pFound-CRITERIA ]++;
    }
    for( i=0; i<nLetters; i++ )
    {
      if( i%2 == 0 )
        nRet += Sums[i];
      else
        nRet -= Sums[i];
    }
    delete Sums;
  }
  return nRet;
}

const int READ_BUFFER_SIZE = 1024;
int calc_file_checksum(const char *pszFile)
{
  char read_buff[ READ_BUFFER_SIZE];
  int checksum = 0;
  FILE *pFile = NULL;
  if( pszFile && (pFile = OSapi_fopen( pszFile, "r" )) )
  {
    int nCount = 0;
    while( !feof( pFile ) && (nCount = fread(read_buff, 
            sizeof(char), READ_BUFFER_SIZE, pFile)) )
    {
      checksum += nCount;
      checksum += generate_checksum( read_buff, nCount );
    }
    OSapi_fclose( pFile );
  }
  return checksum;
}
