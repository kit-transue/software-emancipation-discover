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
import com.swemancipation.CodeRover.DataSource;

import java.net.InetAddress;
import java.net.UnknownHostException;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class ModelConnection {
private boolean running=true;
private boolean inUse;
private String  service;
private String  host;
private String  model;
private String  command;
private boolean dead = false;
private DataSource dataSource = new DataSource();
private Thread modelServerThread = null;

  public ModelConnection(String service, String host, String model,String command) {
      this.service = service;
      this.host = host;
      this.model = model;
      this.command = command;
  }

  //----------------------------------------------------------------------------
  // This flag reflects current status of this connection.  The client must
  // set it to TRUE in the thread-safe method before evaluating an access
  // command using this connection and set it to FALSE when results are retrived.
  //----------------------------------------------------------------------------
  public void setInUse(boolean value) {
     inUse = value;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // Checks the current status of this connection.  Ifsome client is
  // currently evaluating an access command using this connection, the flag is
  // set to true.
  //----------------------------------------------------------------------------
  public synchronized boolean isInUse() {
     return inUse;
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This flag reflects the status of the server connection. If the server
  // socket alive this flag will be set to TRUE. The deamon will break the
  // connection and set this flag to FALSE if client request/reply time exeeds
  // given timeout.
  //----------------------------------------------------------------------------
  public synchronized boolean isAlive() {
      return dataSource.isConnected();
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This flag is set to TRUE if model server application is running.
  //----------------------------------------------------------------------------
  public synchronized void setRunning(boolean value) {
      running=value;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This flag is set to TRUE if model server application is running.
  //----------------------------------------------------------------------------
  public synchronized boolean isRunning() {
      return running;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This method will try to establich connection to the model
  //----------------------------------------------------------------------------
  public synchronized boolean connect() {
      dataSource.setHostName(host);
      dataSource.setServiceName("Dislite:"+model);
      dataSource.setConnected(true);
      if(dataSource.isConnected()==false) {
         running = false;
      }
      return dataSource.isConnected();
  }
  //----------------------------------------------------------------------------

  public synchronized void disconnect() {
      dataSource.setConnected(false);
  }

  public synchronized void kill() {
      dataSource.setConnected(false);
      modelServerThread.interrupt();
  }


  //----------------------------------------------------------------------------
  // Returns data source of the model server.
  //----------------------------------------------------------------------------
  public synchronized DataSource getDataSource() {
      return dataSource;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This method will try to run selected service on the given host
  //----------------------------------------------------------------------------
  public boolean run() {

      // If the server was only marked as not running
      if(connect()==true) {
         dead    = false;
         running = true;
         inUse = false;
         return true;
      }

      if(dead==true) return false;

      if(command==null) return false;
      if(command.length()==0) return false;
      InetAddress local;
      try {
          local = InetAddress.getLocalHost();
      } catch(UnknownHostException e) {
          return false;
      }

      // If this connection is not on the local host we will not
      // try to run service.
      String ip   = local.getHostAddress();
      String name = local.getHostName();
      if( (name.equals(host)==false) &&
          (ip.equals(host)==false) &&
          (host.equals("localhost")==false) &&
          (host.equals("80.0.0.127")==false)        ) return false;



      modelServerThread = new Thread(new serverClass());
      modelServerThread.start();

      for(int i=0; i<100;i++) {
          if(connect()==true) {
              running = true;
              inUse = false;
              return true;
          }
          try {
              synchronized (this) {
                  wait(1000);
              }
          } catch(InterruptedException ie) {
          }
      }
      dead = true;
      return false;
  }
  //----------------------------------------------------------------------------

  public synchronized String getService() {
      return service;
  }


  public synchronized String getHost() {
      return host;
  }

  public synchronized String getModel() {
      return model;
  }


  class serverClass implements Runnable {
    public void run() {
      Process p;
      try {
          System.out.println("Starting server using command : "+command);
          p = Runtime.getRuntime().exec(command);
      } catch(IOException ioe) {
          System.out.println("Error starting server.");
          return;
      }
      BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream()));


      class ErrorsChecker implements Runnable {
      BufferedReader errorsStream;
          public ErrorsChecker(BufferedReader stderr){
              errorsStream = stderr;
          }
          public void run() {
             while(Thread.currentThread().isInterrupted()==false) {
                 try {
                     String line = errorsStream.readLine();
                     System.out.println("Error:"+line);
                     if(line==null) {
                         dataSource.setConnected(false);
                         inUse=false;
                         return;
                     }
                 } catch(IOException ioe) {
                     dataSource.setConnected(false);
                     inUse=false;
                     return;
                 }
             }
          }
      };

      Thread errReader = new Thread(new ErrorsChecker(new BufferedReader(new InputStreamReader(p.getErrorStream()))));
      errReader.start();


      while(Thread.currentThread().isInterrupted()==false) {
          try {
              String line = br.readLine();
              System.out.println("Line:"+line);
              if(line==null) {
                  dataSource.setConnected(false);
                  inUse=false;
                  return;
              }
          } catch(IOException ioe) {
              dataSource.setConnected(false);
              inUse=false;
              return;
          }
      }
    }
  };

}
