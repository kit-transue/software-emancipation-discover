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


#include <vport.h>
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vstdioINCLUDED
   #include vstdioHEADER
#endif
#ifndef vstdlibINCLUDED
   #include vstdlibHEADER
#endif
#ifndef vapplicationINCLUDED
   #include vapplicationHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif
#include <gString.h>
#include <ggeneric.h>
#include <gglobalFuncs.h>
#include <gapl_menu.h>


class ExtApp
{

public:
  ExtApp ();
  ~ExtApp ();
  void Connect (int index);
  void Disconnect (int index);
  void Status (int index);
  char *GetMessage (int index);
  int GetNumMessages ();
  void ClearMessages ();
  int IsActive(int index);
  char *GetClientStr (int index);
  void Startup();
  int CountClients();

private:
  vliststr messageList;
  vliststr *clients;
  int *active;
  int numClients;
};

//-------------------------------------------------------------------------------------------------


static ExtApp app;

//-------------------------------------------------------------------------------------------------


ExtApp::ExtApp ()
{
    clients = NULL;
    active = NULL;
}

//-------------------------------------------------------------------------------------------------


void ExtApp::Startup() 
{

    if (numClients) return;

    vresource res;
    if (gala_GetResource(vapplication::GetCurrent()->GetResources(), 
			 vnameInternGlobalLiteral ("GenericDialogs"),
			 &res))  {
	if (gala_GetResource(res, 
                             vnameInternGlobalLiteral("externalAppDialog"),
                             &res)) {
            if (gala_GetResource (res,
                                  vnameInternGlobalLiteral("ExternalApplications"),
                                  &res)) {
	        clients = new vliststr(res);
                numClients = clients->GetRowCount();
                active = (int *)malloc (sizeof (int) * numClients);

                for (int i = 0; i < numClients; i++) active[i] = 0;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------


ExtApp::~ExtApp ()
{
    if (clients) delete clients;
    clients = NULL;
    if (active) delete [] active;
    active = NULL;
}

//-------------------------------------------------------------------------------------------------


void ExtApp::Connect (int index) {

    char* message = new char[30];
    int rows = messageList.GetRowCount ();
    sprintf (message, "Connecting to Client %d.", index);
    messageList.SetCellValue (rows, 0, message);
    active[index] = 1;
}

//-------------------------------------------------------------------------------------------------


void ExtApp::Disconnect (int index) {

    char* message = new char[40];
    int rows = messageList.GetRowCount ();
     sprintf (message, "Disconnecting from Client %d.", index);
    messageList.SetCellValue (rows, 0, message);
    active[index] = 0;
}

//-------------------------------------------------------------------------------------------------


void ExtApp::Status (int index) {

    char* message = new char[40];
    int rows = messageList.GetRowCount ();
    sprintf (message, "Status of the Client %d", index);
    messageList.SetCellValue (rows, 0, message);
}

//-------------------------------------------------------------------------------------------------


char *ExtApp::GetMessage (int index) {
    return (char*)messageList.GetCellValue(index, 0);
}

//-------------------------------------------------------------------------------------------------


int ExtApp::GetNumMessages () {
    return messageList.GetRowCount();
}

//-------------------------------------------------------------------------------------------------


void ExtApp::ClearMessages () {
    messageList.SetRowCount (0);
}

//-------------------------------------------------------------------------------------------------


int ExtApp::IsActive (int index) {
    if (active) return active[index];
    else return 0;
}

//-------------------------------------------------------------------------------------------------


char *ExtApp::GetClientStr (int index) {

    char *returnValue;
    #define ACTIVESTR " - ACTIVE"

    if (!IsActive(index)) {
          returnValue = (char *) malloc (sizeof(char) * (strlen ((char *)clients->GetCellValue(index, 0)) + 1));
          strcpy (returnValue, (char *)clients->GetCellValue(index, 0));
    }

    else {
        returnValue = (char *) malloc (sizeof(char) * (strlen ((char *)clients->GetCellValue(index, 0)) + 1 + strlen (ACTIVESTR)));
        sprintf (returnValue, "%s%s", clients->GetCellValue(index, 0), ACTIVESTR);
    }

    return (returnValue);
}

//-------------------------------------------------------------------------------------------------


int ExtApp::CountClients() {

    return (numClients);
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_connect (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    app.Connect (atoi (argv[1]));
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_disconnect (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    app.Disconnect (atoi (argv[1]));
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_init (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    app.Startup ();
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_status (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments")

    app.Status (atoi (argv[1]));
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_getMessages (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{

    const int TEMPINCR = 50;
    char *temp;
    int tempSize = TEMPINCR;
    temp = (char*)malloc (sizeof(char) * tempSize);
    temp[0] = '\0';

    for (int i = 0; i < app.GetNumMessages(); i++) {
        char *newMessage = app.GetMessage (i);
        if (strlen (newMessage) + strlen (temp) + 2 >= tempSize) {
            tempSize += TEMPINCR;
            temp = (char*)realloc (temp, tempSize);
        }
        if (i) strcat (temp, "\n");
        strcat (temp, newMessage);
    }

    Tcl_SetResult (interp, temp, TCL_VOLATILE);
    free (temp);
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_clearMessages (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    app.ClearMessages();
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_countClients (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
  gString tmp;
  tmp.sprintf((vchar *)"%d", app.CountClients());
  Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
  return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_isActive (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2) 
	TCL_EXECERR ("wrong number of arguments");
    gString tmp;
    tmp.sprintf ((vchar *)"%d", app.IsActive(atoi (argv[1])));
    Tcl_SetResult(interp, (char *)tmp, TCL_VOLATILE);
    return TCL_OK;
}

//-------------------------------------------------------------------------------------------------


int dis_gpi_getClientStr (ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
    if (argc != 2)
        TCL_EXECERR ("wrong number of arguments")
    Tcl_SetResult (interp, app.GetClientStr (atoi(argv[1])), TCL_DYNAMIC);
    return TCL_OK;
}
















