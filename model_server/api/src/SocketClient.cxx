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
#include "tcl.h"
#include "Interpreter.h"
#include "charstream.h"
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#include <ctype.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#include <cctype>
#include <fstream>
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include <xxinterface.h>
#ifndef _WIN32
   #include <poll.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netinet/tcp.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <unistd.h>
// XXX:   #include <gtAppShellXm.h>
   #ifdef irix6
      #include <sys/poll.h>
   #endif
   #ifdef hp10
      #include <sys/poll.h>
      #include <time.h>
   #endif
#else
   #include <windows.h>
   #include <winsock.h>
   #include <ws2tcpip.h> // for socklen_t
   #include <io.h>
#endif

#include "msg.h"
#include "setprocess.h"
#include "genStringPlus.h"
#include "nameServCalls.h"
#include "Interpreter.h"

extern char* cli_service_name; 

extern char *server_name;
#ifndef _WIN32
typedef void (*hook_type) (int);
extern void *pmod_server_register_listener(int socket, hook_type proc);
extern void pmod_server_unregister_listener(void *fd); 
#endif

void notify_first_client_started();
void notify_last_client_closed();


#ifdef _WIN32
static HWND sock_hwnd;
#endif


#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif

#ifndef _WIN32
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif

//---------------------------------------------------------------------------

#define NS_PORT 28948
#define REGISTER_SERVER   0
#define LIST_OF_SERVERS   1
#define FIND_SERVER       2
#define UNREGISTER_SERVER 3
#define STOP              4

#define DATA_PACKET       1
#define DATA_FINAL_PACKET 2

#define STDOUT_BATCH_CMD_TYPE       1
#define STDOUT_STREAM_CMD_TYPE      2
#define RESULT_BATCH_CMD_TYPE       3
#define STDOUT_DISH_STREAM_CMD_TYPE 4
#define CANCEL_CMD_TYPE             5

//---------------------------------------------------------------------------

int ns_port            = NS_PORT;
static int server_port = -1;

//---------------------------------------------------------------------------

class CommPacket {
public:
  CommPacket(int number, unsigned char type, int size, char *data);
  ~CommPacket();

  int           packet_number;
  unsigned char packet_type;
  int           packet_size;
  char          *packet_data;
};
 

CommPacket::CommPacket(int number, unsigned char type, int size, char *data)
{
  packet_number = number;
  packet_type   = type;
  packet_size   = size;
  packet_data   = data;
}

CommPacket::~CommPacket()
{
  delete [] packet_data;
}

typedef CommPacket *CommPacketPtr;
genArr(CommPacketPtr);

//---------------------------------------------------------------------------

class cliNotificationListener {
public:
    cliNotificationListener(int s) { socket = s; }

    int socket;
};

typedef cliNotificationListener *cliNotificationListenerPtr;
genArr(cliNotificationListenerPtr);

//---------------------------------------------------------------------------

class cliSocketClient;
typedef cliSocketClient *cliSocketClientPtr;
genArr(cliSocketClientPtr);

class cliSocketClient {
public:
  ~cliSocketClient();

  static cliSocketClient *addNewClient(int socket, sockaddr *from, Interpreter *i);
  static cliSocketClient *findClient(int socket);
  static cliSocketClient *findClient(Interpreter *i);
  static int              generateNewClientId(void);
  static void             removeClient(int socket);
  static char            *notifyAllClients(char const *str);

  int  addNotificationListener(int port);
  void addNotificationListener(cliNotificationListener *l);
  void removeNotificationListener(cliNotificationListener *l);
  char *notifyAllListeners(char const *str);

  Interpreter    *interpreter;
  int            socket;
  int            busy;
  int            close;
  int            id;
  int            tcp_addr;
  int            packet_canceled;
  int            lic;
  CommPacket     *current_packet; 
  void           *pmod_server_data;
 
  genArrOf(CommPacketPtr)              packets;
  genArrOf(cliNotificationListenerPtr) notification_listeners;

private:
  cliSocketClient(int socket, sockaddr *from, Interpreter *i);
  static genArrOf(cliSocketClientPtr) socket_clients;
  static int                          last_id;
};

//---------------------------------------------------------------------------

genArrOf(cliSocketClientPtr) cliSocketClient::socket_clients;
int cliSocketClient::last_id = 0;

//---------------------------------------------------------------------------

cliSocketClient::cliSocketClient(int s, sockaddr *from, Interpreter *i)
{
  socket                 = s;
  interpreter            = i;
  busy                   = 0;
  close                  = 0;
  packet_canceled        = 0;
  id                     = generateNewClientId();
  pmod_server_data       = NULL;
  struct sockaddr_in *in = (sockaddr_in *)from;  
  tcp_addr               = ntohl(in->sin_addr.s_addr);
}

cliSocketClient::~cliSocketClient()
{
    int i;
    for(i = 0; i < packets.size(); i++){
	CommPacket *pkt = *packets[i];
	if(pkt != NULL)
	    delete pkt;
    }
    for(i = 0; i < notification_listeners.size(); i++){
	cliNotificationListener *l = *notification_listeners[i];
	if(l != NULL)
	    delete l;
    }
}

//---------------------------------------------------------------------------

int cliSocketClient::generateNewClientId(void)
{
    return last_id++;
}

cliSocketClient *cliSocketClient::addNewClient(int s, sockaddr *from, Interpreter *i)
{
    cliSocketClient *socket_client = new cliSocketClient(s, from, i);
    socket_clients.append(&socket_client);
    if (socket_clients.size() == 1) {
      notify_first_client_started();
    }
    return socket_client;
}

void cliSocketClient::removeClient(int socket) {

    for(int i = 0; i < socket_clients.size(); i++){
		cliSocketClient *client = *socket_clients[i];
		if(client->socket == socket) {
		    if ( client->lic == 1) {
				int j;
				for(j = 0; j < socket_clients.size(); j++) {
			    	cliSocketClient *val = *socket_clients[j];
			    	if(val->lic == 0) {
						val->lic = 1;
						break;
			    	}
				}
			}
			socket_clients.remove(i);
	        if (socket_clients.size() <= 0) {
	        	notify_last_client_closed();
	        }
		    return;
		}
    }
}

cliSocketClient *cliSocketClient::findClient(int socket)
{
  for(int i = 0; i < socket_clients.size(); i++){
    cliSocketClient *client = *socket_clients[i];
    if(client->socket == socket)
      return client;
  }
  return NULL;
}

cliSocketClient *cliSocketClient::findClient(Interpreter *interp)
{
  for(int i = 0; i < socket_clients.size(); i++){
    cliSocketClient *client = *socket_clients[i];
    if(client->interpreter == interp)
      return client;
  }
  return NULL;
}

void cliSocketClient::addNotificationListener(cliNotificationListener *l)
{
    notification_listeners.append(&l);
}

void cliSocketClient::removeNotificationListener(cliNotificationListener *l)
{
    for(int i = 0; i < notification_listeners.size(); i++){
	if(*notification_listeners[i] == l) {
	    notification_listeners.remove(i);
	    return;
	}
    }
}

char *cliSocketClient::notifyAllListeners(char const *str)
{
    static char *buf   = NULL;
    static int buf_len = 0;
    
    int len = strlen(str) + 1;
    for(int i = 0; i < notification_listeners.size(); i++){
	cliNotificationListener *l = *notification_listeners[i];
	SendInteger(l->socket, len);
	SendBuffer(l->socket, str, len);
	int reply_len = 0;
	int result    = ReceiveInteger(l->socket, &reply_len);
	if(result == 1 && reply_len != 0){
	    if(reply_len > buf_len) {
		if(buf != NULL)
		    free(buf);
		buf     = new char[reply_len + 1];
		buf_len = reply_len;
	    } 
	    result = ReceiveBuffer(l->socket, buf, reply_len);
	    if(result != 0)
		return buf;
	}
    }
    return NULL;
}

int cliSocketClient::addNotificationListener(int port)
{
    int sock;
    struct sockaddr_in name;
    
    /* Create the socket. */
    sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return 1;
    /* Give the socket a name. */
    name.sin_family      = AF_INET;
    name.sin_port        = htons(port);
    name.sin_addr.s_addr = htonl(tcp_addr);
    if(connect(sock,  (struct sockaddr *) &name, sizeof (name)) < 0)
	return -1;
    int tmp_switch = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
    /* connection established, add this connection to list of notification channels */
    cliNotificationListener *l = new cliNotificationListener(sock);
    addNotificationListener(l);
    return sock;
}

char *cliSocketClient::notifyAllClients(char const *str)
{
    for(int i = 0; i < socket_clients.size(); i++){
	cliSocketClient *client = *socket_clients[i];
	if(client) {
	    char *reply = client->notifyAllListeners(str);
	    if(reply != NULL)
		return reply;
	}
    }
    return NULL;
}

//---------------------------------------------------------------------------

int send_reply_to_packet(int socket, CommPacket *pkt, unsigned char type, int result, const char *data);

class packetbuf : public streambuf {
protected:
  char            buf[256];
  cliSocketClient *client;
  char            *pp;
public:
  packetbuf (cliSocketClient *cl) { client = cl; pp = buf; }

  int sync () { return 0; }
  int overflow (int ch);
  int underflow () { return 0; }
};

class opacketstream : public packetbuf, public ostream
{
public:
  opacketstream(cliSocketClient *cli) : packetbuf(cli), ostream(this) {}
  
  void     reset() {}
  ostream& flush();
};

int packetbuf::overflow(int ch)
{
    if (ch == EOF) {
	sync();
    } else if (client->close == 0) {
	*pp++ = ch;
	if(pp == buf + 255){
	    pp       = buf;
	    buf[255] = 0;
	    if(!send_reply_to_packet(client->socket, client->current_packet, DATA_PACKET, 1, buf)) {
		client->close = 1;
	    }
	}
    }
    return 0;
}

ostream& opacketstream::flush()
{
  *pp = 0;
  pp  = buf;
  send_reply_to_packet(client->socket, client->current_packet, DATA_PACKET, 1, buf);
  return *this;
}

//---------------------------------------------------------------------------

void close_socket_client(cliSocketClient *client)
{
    if (client) {
		Interpreter* curInterpreter = GetActiveInterpreter();
		SetActiveInterpreter(client->interpreter);
		msg("Closing client", normal_sev) << eom;
		cliSocketClient::removeClient(client->socket);
#ifdef _WIN32
		closesocket(client->socket);
#else
		if(client->pmod_server_data)
		    pmod_server_unregister_listener(client->pmod_server_data);
		closesocket(client->socket);
#endif
		if(client->interpreter!=NULL) {
			if(client->lic) _lis("mc");
			delete client->interpreter;
	 	}
		delete client;
		SetActiveInterpreter(curInterpreter);
    }
}

int send_with_error_check(int socket, const char *buf, int len)
{
  fd_set socket_set;
  int    nfsd = 0;
  int    retval = 1;

#ifdef _WIN32
  WSAAsyncSelect(socket, sock_hwnd, 0, 0);
  unsigned long disable_nonblocking = 0;
  ioctlsocket(socket, FIONBIO, &disable_nonblocking);
#endif

#ifndef _WIN32
  nfsd = FD_SETSIZE;
#endif

  FD_ZERO(&socket_set);
  FD_SET(socket, &socket_set);
  const char *out_ptr = buf;
  while(len > 0){
    int selected = select(nfsd, NULL, &socket_set, NULL, NULL);
    if(selected != 1)
      break;
    int sent = send(socket, out_ptr, len, 0);
    if(sent == SOCKET_ERROR) {
      break;
    }
    len     -= sent;
    out_ptr += sent;
  }
  if(len != 0) {
    retval = 0;
  }
#ifdef _WIN32
  WSAAsyncSelect(socket, sock_hwnd, 32701, FD_READ | FD_CLOSE);
#endif
  return retval;
}

int send_reply_to_packet(int socket, CommPacket *pkt, unsigned char type, int result, const char *data)
{
  unsigned char buf[7];
  
  buf[0]  = (unsigned char)pkt->packet_number;
  buf[1]  = type;
  buf[2]  = (unsigned char)result;
  unsigned int len = strlen(data) + 1;
  buf[3]           = (len >> 24) & 255;
  buf[4]           = (len >> 16) & 255;
  buf[5]           = (len >> 8) & 255;
  buf[6]           = (len & 255);
  int success      = send_with_error_check(socket, (char *)buf, sizeof(buf));
  if(!success) {
    return 0;
  }
  success = send_with_error_check(socket, data, len);
  return success;
}

void process_packets(cliSocketClient *socket_client, int socket)
{
	if(socket_client->busy) {
	    // aleady doing something
	    // only check if we got any canceled packets
	    int pkt_index = 0;
	    do {
	    	CommPacket *pkt = *socket_client->packets[pkt_index];
	      	if(pkt->packet_type == CANCEL_CMD_TYPE){
				unsigned char *ptr   = (unsigned char *)pkt->packet_data;
				int packet_to_cancel = ntohl(*((uint32_t*)ptr));
				if(socket_client->current_packet->packet_number == packet_to_cancel){
		  			socket_client->packet_canceled = 1;
		  			break;
				}
	    	}
	      	pkt_index++;
	    } while(socket_client->packets.size() > pkt_index);
	} else {
		socket_client->busy = 1;
		int pkt_index       = 0;
	    do {
			// Get the packet and check the queue that this packet was not canceled
			// before executing it.
			CommPacket *pkt = *socket_client->packets[pkt_index];
			if(pkt != NULL){
				msg("Packet index $1", normal_sev) << pkt_index << eoarg << eom;
				socket_client->current_packet = pkt;
				Interpreter *prev_interpreter = GetActiveInterpreter();
			  	if(socket_client->interpreter==NULL) {
					Interpreter *i = Interpreter::Create();
					Tcl_SetVar(i->interp, "cli_service", server_name, TCL_GLOBAL_ONLY);
					genString id_buf;
					id_buf.printf("%d", socket_client->id);
					Tcl_SetVar(i->interp, "client_id", (char *)id_buf, TCL_GLOBAL_ONLY);
					socket_client->interpreter = i;
				    if(_los("mc")!=LIC_SUCCESS) {
						socket_client->lic = 0;
				    } else {
						socket_client->lic = 1;
				    }
				}
				SetActiveInterpreter(socket_client->interpreter);
				int success = 1;
				switch(pkt->packet_type) {
					case STDOUT_BATCH_CMD_TYPE: {
						ocharstream tmp_stream;
						msg("Evaluating string: $1", normal_sev) << (char *)pkt->packet_data << eoarg << eom;
						if(socket_client->lic==1) {
							cli_process_input((char *)pkt->packet_data, tmp_stream, socket_client->interpreter, 0);
						} else  {
							msg("No licenses available.", error_sev) << eom;
							tmp_stream << "No licenses available.";
						}
						unsigned char result_code = 1;
						if(socket_client->interpreter->code == TCL_ERROR)   result_code = 0;
						tmp_stream << ends;
						char *out = tmp_stream.str();
						msg("Sending result: $1", normal_sev) << out << eoarg << eom;
						success = send_reply_to_packet(socket, pkt, DATA_FINAL_PACKET, result_code, out);
						delete [] out;
						break; }
					case STDOUT_STREAM_CMD_TYPE: {
						opacketstream tmp_stream(socket_client);
						msg("Evaluating string: $1", normal_sev) << (char *)pkt->packet_data << eoarg << eom;
						unsigned char result_code = 1;
						char *out = "";
						if(socket_client->lic==1) {
							cli_process_input((char *)pkt->packet_data, tmp_stream, socket_client->interpreter, 0);
							if(socket_client->interpreter->code == TCL_ERROR)   result_code = 0;
						} else {
							msg("No licenses available.", error_sev) << eom;
							out = "No licenses available.";
							result_code = 0;
						}
						tmp_stream.flush();
						msg("Sending result: $1", normal_sev) << out << eoarg << eom;
						success = send_reply_to_packet(socket, pkt, DATA_FINAL_PACKET, result_code, out);
						break; }
					case RESULT_BATCH_CMD_TYPE: {
						ocharstream tmp_stream;
		  				msg("Evaluating string: $1", normal_sev) << (char *)pkt->packet_data << eoarg << eom;
		  				unsigned char result_code = 1;
		  				char const *out;
						if(socket_client->lic==1) {
							cli_process_input((char *)pkt->packet_data, tmp_stream, socket_client->interpreter, 0);
		  					if(socket_client->interpreter->code == TCL_ERROR)   result_code = 0;
							out = Tcl_GetStringResult(socket_client->interpreter->interp);
						} else {
							msg("No licenses available.", error_sev) << eom;
							out = "No licenses available.";
							result_code = 0;
						}
						msg("Sending result: $1", normal_sev) << out << eoarg << eom;
						success = send_reply_to_packet(socket, pkt, DATA_FINAL_PACKET, result_code, out);
						break; }
	      			case STDOUT_DISH_STREAM_CMD_TYPE: {
				  		opacketstream tmp_stream(socket_client);
						msg("Evaluating string: $1", normal_sev) << (char *)pkt->packet_data << eoarg << eom;
						if(socket_client->lic==1) {
							cli_process_input((char *)pkt->packet_data, tmp_stream, socket_client->interpreter, 1);
						} else {
							msg("No licenses available.", error_sev) << eom;
						}
						tmp_stream.flush();
						unsigned char result_code = 1;
						if(socket_client->interpreter->code == TCL_ERROR)   result_code = 0;
						const char *out = "";
						msg("Sending result: $1", normal_sev) << out << eoarg << eom;
						success = send_reply_to_packet(socket, pkt, DATA_FINAL_PACKET, result_code, out);
						break; }
				}
				SetActiveInterpreter(prev_interpreter);
				delete pkt;
				pkt = NULL;
				*(socket_client->packets[pkt_index]) = NULL;
				socket_client->packet_canceled       = 0;
				if(socket_client->close) break;
	  		}
			pkt_index++;
		} while (socket_client->packets.size() > pkt_index);
		socket_client->busy = 0;
    	if(socket_client->close) {
			close_socket_client(socket_client);
		}
	}
} 

int read_packet(cliSocketClient *socket_client, int socket)
{
  unsigned char buf[6];

  int received = 0;
  char *ptr    = (char *)buf;
  while(received != sizeof(buf)){
    int len = recv(socket, ptr, sizeof(buf) - received, 0);
    if(len == 0 || len == SOCKET_ERROR){
      return 0;
    }
    received += len;
    ptr      += len;
  }
  // 1st - 1 byte packet number
  unsigned char packet_number = buf[0];
  // 2nd - 1 byte packet type
  unsigned char packet_type = buf[1];
  // 3d-6 - 4 byte data packet size (size of data that will follow packet)
  int packet_size = ntohl(*((uint32_t*)(buf+2)));
  // Read data
  char *data_buf = new char[packet_size];
  if(data_buf == NULL){
    // Out of memory, need to gracefuly handle this
    return 0;
  }
  received = 0;
  ptr      = (char *)data_buf;
  while(received != packet_size){
    int len = recv(socket, ptr, packet_size - received, 0);
    if(len == 0 || len == SOCKET_ERROR){
      return 0;
    }
    received += len;
    ptr      += len;
  }
  CommPacket *pkt = new CommPacket(packet_number, packet_type, packet_size, data_buf);
  // Append packet at the end of queue
  int i;
  for(i = socket_client->packets.size() - 1; i >= 0 && *(socket_client->packets[i]) == NULL; i--) ;
  if(i < 0) {
    if(socket_client->packets.size())
      *(socket_client->packets[0]) = pkt;
    else
      socket_client->packets.append(&pkt);
  } else {
    if(i < socket_client->packets.size() - 1)
      *(socket_client->packets[i + 1]) = pkt;
    else
      socket_client->packets.append(&pkt);
  }
  return 1;
}

#ifdef _WIN32

LRESULT CALLBACK SocketWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
  if(umsg == 32700){
    msg("Accepting connection", normal_sev) << eom;
    struct sockaddr s;
    int s_size                     = sizeof(s);
    int new_socket                 = accept(wparam, &s, &s_size);
    msg("Adding new client", normal_sev) << eom;
    cliSocketClient *socket_client = cliSocketClient::addNewClient(new_socket, &s, NULL);
    WSAAsyncSelect(new_socket, sock_hwnd, 32701, FD_READ | FD_CLOSE);
    int tmp_switch = 1;
    setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
  }
  if(umsg == 32701){
    WORD event = WSAGETSELECTEVENT(lparam);
    cliSocketClient *socket_client = cliSocketClient::findClient(wparam);
    if (socket_client) {
      if (event == FD_CLOSE) {
	if(socket_client->busy)
	    socket_client->close = 1;
	else
	    close_socket_client(socket_client);
      } else {
	WSAAsyncSelect(wparam, sock_hwnd, 0, 0);
	unsigned long disable_nonblocking = 0;
	ioctlsocket(wparam, FIONBIO, &disable_nonblocking);
	int success = read_packet(socket_client, wparam);
	WSAAsyncSelect(wparam, sock_hwnd, 32701, FD_READ | FD_CLOSE);
	if(success)
	  process_packets(socket_client, wparam);
      }
    }
  }
  return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void InitializeWindowsSocketSupport(void)
{
    WORD wVersionRequested;  
    WSADATA wsaData; 
    int err; 
   
    sock_hwnd         = NULL;
    wVersionRequested = MAKEWORD(1, 1); 
    err               = WSAStartup(wVersionRequested, &wsaData); 
    if (err != 0) {
      printf("no sockets available\n");
      return;
    }
    WNDCLASS socket_window;
    socket_window.style         = 0;
    socket_window.lpfnWndProc   = SocketWindowProc;
    socket_window.cbClsExtra    = 0;
    socket_window.cbWndExtra    = 0;
    socket_window.hInstance     = GetModuleHandle(NULL);
    socket_window.hIcon         = NULL;
    socket_window.hCursor       = NULL;
    socket_window.hbrBackground = NULL;
    socket_window.lpszMenuName  = NULL;
    socket_window.lpszClassName = "_DISCOVER_SOCKET_WIN";
    ATOM cl   = RegisterClass(&socket_window);
    sock_hwnd = CreateWindow("_DISCOVER_SOCKET_WIN", "_DISCOVER_SOCKET_WIN", 0, 0, 0, 0, 0,
			     NULL, NULL, GetModuleHandle(NULL), NULL);
}

int check_socket_client_canceled(Interpreter *i)
{ 
  MSG msg;

  // Pass all messages to socket handling window
  while(PeekMessage(&msg, sock_hwnd, 32700, 32701, PM_REMOVE)){
    DispatchMessage(&msg);
  }
  cliSocketClient *socket_client = cliSocketClient::findClient(i);
  if(socket_client){
    if(socket_client->packet_canceled == 1)
      return 1;
    else
      return 0;
  } else 
    return 0;
}


HWND get_socket_window_handle(void)
{
  return sock_hwnd;
}

void SetupAsyncSocketListener(int socket)
{
  WSAAsyncSelect(socket, sock_hwnd, 32700, FD_ACCEPT);
}

#else

static void SocketReadHandler(int socket)
{
    cliSocketClient *socket_client = cliSocketClient::findClient(socket);
    int success = read_packet(socket_client, socket);
    if(success){
	process_packets(socket_client, socket);
    } else {
	if(socket_client->busy)
	    socket_client->close = 1;
	else {
	    close_socket_client(socket_client);
	}
    }
}

static void SocketReadFromPoll(int socket)
{
    cliSocketClient *socket_client = cliSocketClient::findClient(socket);
    int success = read_packet(socket_client, socket);
    if(success){
	process_packets(socket_client, socket);
    } else {
	if(socket_client->busy)
	    socket_client->close = 1;
	else 
	    close_socket_client(socket_client);
    }
}

static void SocketAcceptHandler(int socket)    
{
    struct sockaddr s;
    socklen_t s_size = sizeof(s);
    msg("Accepting connection", normal_sev) << eom;
    int new_socket                 = accept(socket, &s, &s_size);
    msg("Adding new client", normal_sev) << eom;
    cliSocketClient *socket_client = cliSocketClient::addNewClient(new_socket, &s, NULL);
    int tmp_switch                 = 1;
    setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&tmp_switch, sizeof(tmp_switch));
    socket_client->pmod_server_data = pmod_server_register_listener(new_socket, SocketReadHandler);
}

int check_socket_client_canceled(Interpreter *i)
{ 
    struct pollfd fds;

    cliSocketClient *socket_client = cliSocketClient::findClient(i);
    if(socket_client){
	if(socket_client->packet_canceled == 1 || socket_client->close == 1)
	    return 1;
	fds.fd     = socket_client->socket;
	fds.events = POLLIN | POLLRDNORM | POLLPRI;
	if(poll(&fds, 1, 0) > 0){
	    SocketReadFromPoll(socket_client->socket);
	}
	if(socket_client->packet_canceled == 1 || socket_client->close == 1)
	    return 1;
	else
	    return 0;
    } else 
	return 0;
}

void SetupAsyncSocketListener(int socket)
{
    pmod_server_register_listener(socket, SocketAcceptHandler);
}

#endif

int add_client_notificator(int port, Interpreter *i)
{
    cliSocketClient *socket_client = cliSocketClient::findClient(i);
    if(socket_client){
	int socket = socket_client->addNotificationListener(port);
	return socket;
    } else
	return -1;
}

char *send_client_notification(char const *str)
{
    return cliSocketClient::notifyAllClients(str);
}

int get_server_port(void)
{
    return server_port;
}



int start_name_server_process(void) {
genString command;
#ifdef _WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;     
  char buf[1024];

	  genString serverFileDir;
      if(GetEnvironmentVariable("PSETHOME",buf,1024)!=0) {
          serverFileDir=buf;
          serverFileDir+="\\Bin\\";
	  } else {
          serverFileDir="C:\\Discover\\Bin\\";
	  }
      command.printf("%snameserv",(char *)serverFileDir);

      memset(&si,0,sizeof(STARTUPINFO));
	  si.wShowWindow=SW_HIDE;
	  si.cb=sizeof(STARTUPINFO);
	  if(::CreateProcess(NULL,(char *)command,NULL,NULL,FALSE, CREATE_NO_WINDOW,NULL,NULL,&si,&pi)==FALSE) {
	      return 0;
	  }
      CloseHandle(pi.hThread);
      CloseHandle(pi.hProcess);
	  return 1;
#else
    char * dir;
    dir = getenv("PSETHOME");
    if(dir==NULL) {
       command = "nameserv";
    }else {
       command.printf("%s/bin/nameserv",dir);
    }
    char const * arglist[2];
    arglist[0] = "nameserv";
    arglist[1] = NULL;
    const pid_t pid = vfork();
    if(pid == 0) {
		OSapi_closeChildren();
	    setpgid(0,0);
	    OSapi_execv(command, (char * const *)arglist);
    } 
	return 1;
#endif
}



void report_to_nameserver(int port, const char *service)
{
    struct sockaddr_in self_addr;

    char buf[1024];
    if(gethostname(buf, 1024) != 0)
	return;
    if(server_name == NULL) {
	genString tmp;
	if(service!=NULL && strlen(service) > 0)
	    tmp.printf("%s", service);
	else tmp.printf("Discover:%s", buf);

	server_name = strdup((char *)tmp);
    }
    struct hostent *ent = gethostbyname(buf);
    if(ent == NULL)
	return;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port   = htons(ns_port);
    self_addr.sin_addr   = *(in_addr *)*ent->h_addr_list;
    /* Create the socket. */
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
	return;
    /* Give the socket a name. */
    if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) < 0) {
		if(start_name_server_process()==0) return;
		int i;
		for(i=0;i<3000;i++) {
		     closesocket(sock);
             sock = socket (PF_INET, SOCK_STREAM, 0);
             if(connect(sock,  (struct sockaddr *)&self_addr, sizeof (sockaddr_in)) >=0) 
	            break;
		}
	    if(i==3000) return;
	}
    SendCommand(sock, REGISTER_SERVER);
    if(!SendInteger(sock, port)){
	closesocket(sock);
	return;
    }
    if(!SendInteger(sock, strlen(server_name))) {
	closesocket(sock);
	return;
    }
    if(!SendBuffer(sock, server_name, strlen(server_name))) {
	closesocket(sock);
	return;
    }
    closesocket(sock);
}

int make_socket(const char* service)
{
  int sock;
  struct sockaddr_in name;
  
  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;
  /* Give the socket a name. */
  name.sin_family      = AF_INET;
  name.sin_port        = 0; //htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
    return -1;
  struct sockaddr_in assigned;
  socklen_t len = sizeof(assigned);
  if(getsockname(sock, (struct sockaddr *)&assigned, &len) == 0){
      server_port = ntohs(assigned.sin_port);
      msg("PORT: $1", normal_sev) << server_port << eoarg << eom;
      report_to_nameserver(server_port, service);
  }
  return sock;
}

int cli_init_shared_interp(bool);
void InitializeSocketServer(const char* service)
{
  if(service!=NULL && strlen(service)!=0) {
#ifdef _WIN32
      InitializeWindowsSocketSupport();
#endif
      int i;
      for(i=0;i<strlen(service);i++) {
	  if(service[i]==':') {
	      i++;
	      break;
	  }
      }
      if(i==strlen(service)) i=0;
      cli_service_name = strdup(service+i);
      //Create symbol cache if it is a model server and the service name
      //has "Dislite:" prefix.
      bool create_symbol_cache = (strncmp(service, "Dislite:", 8) == 0);

      cli_init_shared_interp(create_symbol_cache);
      int socket = make_socket(service);
      int result = listen(socket, 20);
      SetupAsyncSocketListener(socket);
  }
}
