/* ***********************************************************************
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
*********************************************************************** */

/**
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */
package httpserver;

import java.util.Vector;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.IOException;

public class HTTPServerApp implements ConnectionsListInterface, Runnable {
private transient Vector connectionsList = new Vector();
private transient ServerSocket httpSocket;
private transient boolean shuttingDown=false;
private transient boolean shutDown=false;
private transient ModelsCollection models;
private transient String serviceListFileName;
private transient Thread killerThread;
private transient boolean chacheConnections;

  public HTTPServerApp(String fileName, boolean chache) throws IOException {
    chacheConnections=chache;
    serviceListFileName = fileName;
    models = new ModelsCollection();
    httpSocket = new ServerSocket(80);
  }

  //----------------------------------------------------------------------------
  // Force all server to shut down.
  //----------------------------------------------------------------------------
  public void shutDownServer() {
     shuttingDown=true;
     try {
          httpSocket.close();
     } catch(IOException ioex) {
          System.out.println("ERROR : Unable to close HTTP server port.");
     }
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // Returns the amount of clients currently waiting.
  //----------------------------------------------------------------------------
  public synchronized int count() {
      return connectionsList.size();
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This function adds new connection object to the connectios list. Ususlly
  // it must be called after "accept" function returns you new client
  // socket and a new connection object was created using this socket.
  //----------------------------------------------------------------------------
  public synchronized void add(ClientConnection connection) {
      connectionsList.add(connection);
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This function will remove given connection object from the connections
  // list. We will call it from the connection object thread so the main
  // application thread must not track the connection object life cycle.
  //----------------------------------------------------------------------------
  public synchronized void remove(ClientConnection connection) {
      connectionsList.remove(connection);
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This function will inspect connected clients list to determine some
  // with the timeout expired. It will disconnect such clients.
  //----------------------------------------------------------------------------
  public synchronized void removeDeadConnections() {
      for(int i =0; i<connectionsList.size();i++) {
          ClientConnection client = (ClientConnection)connectionsList.elementAt(i);
          int timeout = client.getTimeout();
          if(timeout>=30) {
             ModelConnection server = client.getConnectedServer();
             if(server!=null) {
                 System.out.println("******** Removing connection due to the command evaluation timeout.");
                 server.disconnect();
                 models.lowPriority(server);
             }
             client.disconnect();
             break;
          } else {
             client.setTimeout(timeout+1);
          }
      }
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This function will disconnect all clients during the exit procedure.
  //----------------------------------------------------------------------------
  public synchronized void removeAllConnections() {
      for(int i =0; i<connectionsList.size();i++) {
          ClientConnection client = (ClientConnection)connectionsList.elementAt(i);
          ModelConnection server = client.getConnectedServer();
          if(server!=null) {
              server.kill();
          }
          client.disconnect();
      }
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  //
  //----------------------------------------------------------------------------
  private void killerLoop() {
     while(shuttingDown==false) {
         try {
             synchronized (this){
                 wait(2000);
             }
         } catch(Exception e) {
             e.printStackTrace();
         }
         removeDeadConnections();
     }
     shutDown = true;
     System.out.println("FINAL : Killer thread is aborted.");
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This function will start low priority thread which will remove dead
  // connections.
  //----------------------------------------------------------------------------
  public void startKiller() {
      class Killer implements Runnable{
         public Killer() {
         }
         public void run() {
             killerLoop();
         }
      };
      killerThread = new Thread(new Killer(),"KillerThread");
      killerThread.setPriority(Thread.MIN_PRIORITY);
      killerThread.start();
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // Returns models collection object using which we will be able to locate
  // server for CodeRower cmmand evaluation.
  //----------------------------------------------------------------------------
  public synchronized ModelsCollection getModels() {
      return models;
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This is application main thread. The only task of this thread is to create
  // to accept client connections and on each connection create a connection
  // object and insert a reference to it into the connections list.
  // It must not remove connections from the list. The running connection will
  // do it by itself when the request processing is over.
  //----------------------------------------------------------------------------
  public void run() {
     if(models.loadServerList(serviceListFileName)==false) {
         return;
     }
     while(shutDown==false) {
         // Make a client-server connection and listen again.
         Socket clientSocket = null;
         try {
             clientSocket = httpSocket.accept();
         } catch (IOException acceptError) {
             shuttingDown = true;
             break;
         }
         // Add the new connection object to the connections list
         ClientConnection connection = new ClientConnection(this,clientSocket,chacheConnections);
         add(connection);
         // We will start the connection thread and we will foget about this
         // thread. When the work is over the connection object will remove
         // it's reference from the connections list by itself.
         Thread connectionThread = new Thread(connection);
         connectionThread.start();
     }
     while(shutDown==false) {
         try {
             synchronized (this){
                 wait(2000);
             }
         } catch(Exception e) {
             e.printStackTrace();
         }
     }
     removeAllConnections();
     System.out.println("FINAL : All client connections removed.");
     try {
         httpSocket.close();
     } catch (IOException acceptError) {
     }
     System.out.println("FINAL : HTTP socket closed.");
     System.exit(0);
  }
  //----------------------------------------------------------------------------

  public static void main(String[] args) {
  HTTPServerApp listener = null;
    if(args == null) {
        System.out.println("ERROR : no service list file name passed to the HTTP server.");
        return;
    }
    if(args.length==0) {
        System.out.println("ERROR : no service list file name passed to the HTTP server.");
        return;
    }
    System.out.println("Starting HTTP server with the service file "+args[0]);
    try {
       if(args.length>1 && args[1].equals("-no_cache")) {
           System.out.println("No connections cache mode.");
           listener = new HTTPServerApp(args[0],false);
       } else {
          listener = new HTTPServerApp(args[0],true);
       }
   } catch (IOException ioex) {
       System.out.println("ERROR : Unable to create http server socket.");
       return;
    }
    try {
        listener.startKiller();
        if(listener!=null) listener.run();
    } catch(Exception e) {
        System.out.println("ERROR : Unknown exception in the HTTP server. Exiting.");
        try {
            listener.httpSocket.close();
        } catch(Exception ex) {
        }
        return;
    }
  }


}
