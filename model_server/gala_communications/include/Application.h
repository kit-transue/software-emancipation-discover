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
#ifndef _Application_h
#define _Application_h

#include <wchar.h>

#ifndef _Args_h
#include "Args.h"
#endif

// Make sure that gInit is defined.
// Note - The other def is in gala/include/ggeneric.h.
//        Someday, one (or both?) of these will be expanded (or deleted?).
#ifndef gInit
#define gInit(name)
#endif

extern Args* DIS_args;

struct Tcl_Interp;

class vservicePrimitive;
class vsignature;
class vsession;
class vsessionStatement;
class vservice;
class vkindTyped;
class vlist;

typedef int vsessionEvent;
typedef int vsessionStatementEvent;
typedef unsigned char vchar;
typedef vchar vstr;
typedef vkindTyped vinstance;
typedef vinstance vscrap;

typedef void (StartNotifier) (int start_end_flag);
typedef vscrap* (Server) (vservicePrimitive *primitive,
			  vsession          *session,
			  vscrap            *scrapArgs
			  );

typedef enum {
    T_CHAR,
    T_SHORT,
    T_INT,
    T_LONG,
    T_FLOAT,
    T_DOUBLE,
    T_CHARPTR,
    T_VSTRPTR,
    T_ARRAY
} Type;

class Args;

class Application {
  public:

    Application ();
    ~Application ();
 
    int registerService (const char* group, const char* service, Server*, StartNotifier* = 0);
    int unregisterService ();

    int connect (const char* group, const char* service);
    int disconnect ();

	// This function was designed to remove all linked list of Applications 
	// and close it's sessions correctly. It will close all services and it
	// will terminate  broker session.
	// Pls use this function only in emergency cases. The correct exit sequence
	// is just to remove all previously created Applications. When removing the 
	// last one service list will be cleared, servise will be unregistered and 
	// broker session will be terminated.
	static void ShutdownOnError(void);

	static void SetCannotSend(bool);
	static bool IsConnectionLost();

	static int getAllClients();
	//this is the older version--it isn't called...
    int getNumberOfClients();

    const char *getTimeWhenRegistered();


    int eval (vstr*);
    vstr* getEvalResult();

    static const char* getGroupName();
	const vchar* getName() {return name_;}



    static void buildServiceList(const vchar*, const vchar*);
    static void serviceListNotify(vsessionStatement*, 
				  vsessionStatementEvent*);
    static int getServiceListSize();
    static const vchar* getServiceListAttributeValue(int, const vchar*);
    static vsession* getServiceListSession(int);
    static int getServiceListItemIndex(const vchar*, const vchar*);

  public:

    Tcl_Interp* interp ();
    void set_interp (Tcl_Interp*);
    vsession*   mySession ();
    vsignature* getSignature ();
    int         SendBlock(vsessionStatement *statement);
    
    static Application* findApplication (vsession*);
     static Application* findApplication (const char*, const char* group=0);
    static Application* findApplication (const vchar*, const vchar* group=0);
    static void IncrementCount();
    static void DecrementCount();
    static void StopCountAction(int flag);
    static void ShowTaskbarWindow(int fShow);//does nothing on UNIX

  private:
    static void applicationServer ();
    static void applicationNotifier ();
    void setSession (vsession*);
    static void matchNotify (vsessionStatement *statement,
					  vsessionStatementEvent *event);

  private:
    static Application* list_;
    static int uniq_id_;
    static vsession* sessionBroker_;
    long m_RegisterTime;

    int id_;
    Application *next_;
    vsession* the_session_;
    vservice* service_;
    vchar name_[50];
    vstr* eval_result_;
    Tcl_Interp* interp_;
    vsignature* main_sig_;
    int connected;
    static bool cannot_send;
    int send_status;
    vsessionStatement *statement_to_send;

    static StartNotifier* starter_;

    // list of services that match the specified parameters given
    // in buildServiceList
    static vlist* service_list;
    static vchar* regexp_match;  // means of choosing matches

    static void BrokerNotify(vsession *session, vsessionEvent *event);

  public:
    static int traceMode;
    static int stopCountAction;
    static int nCalledCount; //static counter indicating if time-consuming proc.
                             //is running, used to implement "Cancel" button
};


typedef void (*callbackFunction)();
typedef void (*callbackFunction1)(void*);

#if 0  // XXX: should not be required for public use.
class callbackEvent : public veventClientEvent
{
  public:
    callbackEvent(callbackFunction);
    callbackEvent(callbackFunction1,void*);
    ~callbackEvent();
    virtual int Handle();

  private:
    callbackFunction cbfn;
    callbackFunction1 cbfn1;
    void* data;
};
#endif

// External interfaces

extern "C" {
    int dis_register_service (const vchar*, const vchar*);
    int dis_unregister_service (int);
    int dis_connect (const vchar*, const vchar*, const vchar*);
    int dis_disconnect (int);
    int dis_register (int, const vchar*, ...);
    int dis_associate ( int, const vchar*, const vchar*);
    const vchar* dis_find_association (int, const vchar*);
}

typedef vstr *dyn_vstrptr;

#endif
