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
 * Title:        CodeRover Integration with JBuilder<p>
 * Description:  Version 0.1<p>
 * Copyright:    Copyright (c) Dmitry Ryachovsky<p>
 * Company:      UNPSPRING Software<p>
 * @author Dmitry Ryachovsky
 * @version 1.0
 */
package JBuilderAddIn;

import java.net.*;
import java.io.*;
import java.util.*;
import javax.swing.SwingUtilities;


import com.swemancipation.CodeRover.Utilities.*;

public class SocketConnection implements Runnable,OpenListener,CloseListener{

  private ServerSocket  serverSocket;
  private Thread        actionsThread;
  private Thread        acceptThread;
  private Vector        clientSockets = new Vector();
  private AddInCommands commands = null;
  private AddInMain     addInMain = null;

  public SocketConnection(AddInMain main, AddInCommands commands) {
    this.commands = commands;
    this.addInMain = main;
  }

  public boolean open() {
    AddInMain.log.println("SocketConnection: openning...");
    try {
      if((serverSocket = createServerSocket())!=null) {
    	AddInMain.log.println("SocketConnection: checking nameserver...");
        if(isNameServerRunning(1)==false) {
    	  AddInMain.log.println("SocketConnection: nameserver is not running.");
          if(startNameServer()==false) {
            serverSocket.close();
    		AddInMain.log.println("SocketConnection: failed.");
            return false;
          } else {
    		AddInMain.log.println("SocketConnection: trying to connect to nameserver...");
            if(isNameServerRunning(1000)==false) {
              serverSocket.close();
    		  AddInMain.log.println("SocketConnection: failed.");
              return false;
            }
          }
        }
    	AddInMain.log.println("SocketConnection: registering " + getServiceName() + " service...");
        if(NameServerService.registerService( getServiceName(), serverSocket.getLocalPort()) == true) {
    	  AddInMain.log.println("SocketConnection: registered.");
          acceptThread = new Thread(new AcceptConnection());
          acceptThread.start();
          actionsThread = new Thread(this);
          actionsThread.start();
          return true;
        } else {
          serverSocket.close();
    	  AddInMain.log.println("SocketConnection: failed.");
          return false;
        }
      } else {
    	AddInMain.log.println("SocketConnection: failed.");
        return false;
      }
    } catch(IOException e) {
    }
    AddInMain.log.println("SocketConnection: failed.");
    return false;
  }

  public boolean close() {
    try {
      if(actionsThread != null) {
        // waiting for finishing thread
        AddInMain.log.println("Interrupting actionsThread...");
        actionsThread.interrupt();
        AddInMain.log.println("actionsThread interrupted.");
        AddInMain.log.println("waiting for finishing actionsThread...");
        while(actionsThread.isAlive()==true){
          synchronized(this) {
            try {
              wait(200);
            } catch (InterruptedException e) {
            }
          }
        }
        AddInMain.log.println("actionsThread finished.");
      }
      if(acceptThread != null) {
        AddInMain.log.println("Interrupting acceptThread...");
        acceptThread.interrupt();
        AddInMain.log.println("acceptThread interrupted");
        // waiting for finishing thread
        AddInMain.log.println("waiting for finishing acceptThread...");
        while(acceptThread.isAlive()==true){
          synchronized(this) {
            try {
              wait(200);
            } catch (InterruptedException e) {
            }
          }
        }
        AddInMain.log.println("acceptThread finished.");
      }
      AddInMain.log.println("Unregistering service "+getServiceName());
      NameServerService.unregisterService(getServiceName());
      AddInMain.log.println("Service unregistered");
      if(serverSocket != null)
        serverSocket.close();
      return true;
    } catch (IOException e) {
      AddInMain.log.println("close: IOException");
    } catch (Exception genExc) {
      AddInMain.log.println("close: Exception");
    }
    return false;
  }

  private ServerSocket createServerSocket() {
    try{
        return new ServerSocket(0);
    } catch (IOException e) {
      return null;
    }
  }

  private boolean isNameServerRunning() {
    return isNameServerRunning(1);
  }

  private boolean isNameServerRunning(int numAttempts) {
    for(int i=0;i<numAttempts;i++) {
        if(NameServerService.connect()!=null) {
          return true;
        }
    }
    return false;
  }

  private boolean startNameServer() {
    AddInMain.log.println("startNameServer: starting nameserver...");
    String  command = "\"" + AddInMain.homeDir + "/bin/nameserv\"";
    RunExeResult res = JStartExternalExe.runExe(command,"");
    if(res.isError()) {
      AddInMain.log.println("startNameServer: cannot start nameserver: " + res.getErrorDescription());
      return false;
    }
    AddInMain.log.println("startNameServer: nameserver started.");
    return true;
  }

  private String getServiceName() {
    try {
      String      userName = System.getProperty("user.name");
      InetAddress ia = InetAddress.getLocalHost();
      if(ia!=null && userName!=null) {
        String      hostName = ia.getHostName();
        return  hostName + ":" + userName + ":" + "Editor:JBuilder";
      }
    } catch(UnknownHostException e) {
    }
    return null;
  }

  // this method converts OS specific separators
  // and resolves shortcuts in pathes (like "..", "." ...)
  public static String resolvePathSeparators(String fileName) {
/*    String converted;
    // converting path separators
    if(File.separatorChar == '\\') {
      converted = fileName.replace('/','\\');
    } else {
      converted = fileName.replace('\\','/');
    }

    //------------------------------------------------
    // IMPORTANT: we should resolve two dots at first
    // otherwise we may corrupt path by partly resolving
    // the path string because dot is a substring of two
    // dots string
    //------------------------------------------------

    // resolving two dots in the path - remove these two dots and the directory
    // which lead these dots
    // if two dots inside the path
    int doubleDotIdx = 0;
    while((doubleDotIdx = converted.indexOf(File.separatorChar+".."))!=-1) {
      AddInMain.log.println("resolvePathSeparators: converted="+converted);
      AddInMain.log.println("resolvePathSeparators: doubleDotIdx="+doubleDotIdx);
      String tmp = converted.substring(0,doubleDotIdx-1);
      AddInMain.log.println("resolvePathSeparators: tmp="+tmp);
      int lastSeparatorIdx = tmp.lastIndexOf(File.separatorChar);
      AddInMain.log.println("resolvePathSeparators: lastSeparatorIdx="+lastSeparatorIdx);
      tmp = tmp.substring(0,lastSeparatorIdx);
      AddInMain.log.println("resolvePathSeparators: tmp="+tmp);
      tmp += converted.substring(doubleDotIdx+3);
      AddInMain.log.println("resolvePathSeparators: tmp="+tmp);
      converted = tmp;
    }
    // resolving dot in the path - just remove these dots
    // if dot inside path
    int dotIdx = 0;
    while((dotIdx = converted.indexOf(File.separatorChar+"."))!=-1) {
      String tmp = converted.substring(0,dotIdx-1);
      tmp += converted.substring(dotIdx+2,converted.length());
      converted = tmp;
    }
    // if path begins with dot
    if(converted.startsWith("."+File.separatorChar)==true) {
      converted = converted.substring(2);
    }
*/
    AddInMain.log.println("resolvePathSeparators: resolving " + fileName + "...");
    // remove quotes around the file name if any
    if(fileName.charAt(0)=='"') fileName = fileName.substring(1);
    if(fileName.charAt(fileName.length()-1)=='"') fileName = fileName.substring(0,fileName.length()-1);
    // convert to OS specific path
    File file = new File(fileName);
    AddInMain.log.println("resolvePathSeparators: result is " + file.getAbsolutePath() + ".");
    // return it
    return file.getAbsolutePath();
  }

  private String processRequest(String request) {
    Vector  args = new Vector();
    String  command;
    // Tokenizing string query and placing all tokens into the
    // vals matrix
    StringTokenizer requestParams = new StringTokenizer(request,"\t");
    if(requestParams.hasMoreTokens())
      command = requestParams.nextToken();
    else
      return "Unknown command.";
    while(requestParams.hasMoreTokens()) args.add(requestParams.nextToken());

    // Converts given model file line into modified file line
    if(command.equals("model_line_to_private_line")) {
      AddInMain.log.println("processRequest: model_line_to_private_line command received");
      int line = commands.getPrivateLine(resolvePathSeparators(args.get(0).toString()),
                                            resolvePathSeparators(args.get(1).toString()),
                                            Integer.parseInt(args.get(2).toString()));
      AddInMain.log.println("processRequest: reply is '" + line + "'");
      return Integer.toString(line);
    }

    // Converts given model file line into modified file line
    if(command.equals("private_line_to_model_line")) {
      AddInMain.log.println("processRequest: private_line_to_model_line command received");
      int line = commands.getModelLine(resolvePathSeparators(args.get(0).toString()),
                                            resolvePathSeparators(args.get(1).toString()),
                                            Integer.parseInt(args.get(2).toString()));
      AddInMain.log.println("processRequest: reply is '" + line + "'");
      return Integer.toString(line);
    }

    // Open file using given line number
    if(command.equals("open")) {
      String file = resolvePathSeparators(args.get(0).toString());
      int line = -1;
	  try {
		line = Integer.parseInt(args.get(1).toString());
      } catch(NumberFormatException nfe) {
        AddInMain.log.println("EXCEPTION: " + nfe.getMessage());
      }
      AddInMain.log.println("processRequest: open command received: file " + file + " line " + line);
      if(commands.openFile(file,line)==true) {
        AddInMain.log.println("processRequest: reply is 'done'");
        return "done";
      } else {
        return "File does not exist. Check server source root. "+args.get(0).toString();
      }
    }

    // Selects the token on the given line
    if(command.equals("mark")) {
      AddInMain.log.println("processRequest: mark command received");
      AddInMain.log.println("processRequest: reply is 'done'");
      return "done";
    }

    // Open file using given offset (not implemented under DeveloperStudio)
    if(command.equals("openoffset")) {
      AddInMain.log.println("processRequest: openoffset command received");
      AddInMain.log.println("processRequest: reply is 'not implemented'");
      return "not implemented";
    }

    // Selects the token using the start and end offsets (not implemented under DeveloperStudio)
    if(command.equals("markoffset")) {
      AddInMain.log.println("processRequest: markoffset command received");
      AddInMain.log.println("processRequest: reply is 'not implemented'");
      return "not implemented";
    }

    // Shows string from the client in the editor status window
    if(command.equals("info")) {
      AddInMain.log.println("processRequest: info command received");
      commands.showMessage(args.get(0).toString());
      AddInMain.log.println("processRequest: reply is 'done'");
      return "done";
    }

    // Saves current editor selection in the buffer. We will query selection parameters
    // later and they must be unchanged.
    if(command.equals("freeze")) {
      AddInMain.log.println("processRequest: freeze command received");
      commands.freezeSelection();
      AddInMain.log.println("processRequest: reply is 'done'");
      return "done";
    }

    // Returns the name of the file on which we freeze our selection.
    if(command.equals("file")) {
      AddInMain.log.println("processRequest: file command received");
      String file = commands.getSelectedFile();
      AddInMain.log.println("processRequest: reply is '" + file + "'");
      return file;
    }

    // Returns the selected line. Must not be implemented under Emacs
    if(command.equals("line")) {
      AddInMain.log.println("processRequest: line command received");
      int line = commands.getSelectedLine();
      AddInMain.log.println("processRequest: reply is '" + line + "'");
      return Integer.toString(line);
    }

    // Returns the offset to the selection. Do not exist under DeveloperStudio
    if(command.equals("offset")) {
      AddInMain.log.println("processRequest: offset command received");
      AddInMain.log.println("processRequest: reply is 'not implemented'");
      return "not implemented";
    }

    // Returns the offset in the selected line. Must not be implemented under Emacs
    if(command.equals("lineoffset")) {
      AddInMain.log.println("processRequest: lineoffset command received");
      int column = commands.getSelectedColumn();
      AddInMain.log.println("processRequest: reply is '" + column + "'");
      return Integer.toString(column);
    }

    // register a new client
    if(command.equals("register")) {
      AddInMain.log.println("processRequest: lineoffset command received");
      AddInMain.log.println("processRequest: reply is 'done'");
      addInMain.updateActions();
      return "done";
    }

    // uregister an old client
    if(command.equals("unregister")) {
      AddInMain.log.println("processRequest: unregister command received");
      AddInMain.log.println("processRequest: reply is 'done'");
      addInMain.updateActions();
      return "done";
    }

    // We do not know this command
    AddInMain.log.println("processRequest: UNKNOWN command received");
    return "Unknown command.";
  }

  public boolean sendEvent(String event) {
    boolean sentOk = false;
    Socket  clientSocket = null;
    for(int i=0;i<clientSockets.size();i++) {
      if((clientSocket=(Socket)clientSockets.get(i))!=null) {
        try {
          OutputStream  os = clientSocket.getOutputStream();
          if(SocketCommand.sendEvent(os,event)==false) {
            clientSockets.remove(i);
            i--;
          } else {
            sentOk = true;
          }
        } catch (IOException e) {
          clientSockets.remove(i);
          i--;
        }
      } else {
        clientSockets.remove(i);
        i--;
      }
    }
    return sentOk;
  }

  private void getClientsRequests() {
    Socket  clientSocket = null;
    for(int i=0;i<clientSockets.size();i++) {
      if((clientSocket=(Socket)clientSockets.get(i))!=null) {
        try {
          InputStream is = clientSocket.getInputStream();
          if(is.available() > 0) {
            String request = null;
            if((request=SocketCommand.receiveCommand(is))!=null && request.length()>0) {
              String reply = processRequest(request);
              if(reply!=null && reply.length()>0) {
                OutputStream  os = clientSocket.getOutputStream();
                if(SocketCommand.sendReply(os,reply)==false) {
                  clientSockets.remove(i);
                  i--;
                }
              }
            } else {
              clientSockets.remove(i);
              i--;
            }
          }
        } catch (IOException e) {
          clientSockets.remove(i);
          i--;
        }
      } else {
        clientSockets.remove(i);
        i--;
      }
    }
  }

  public synchronized boolean hasConnections() {
    return clientSockets.size()>0;
  }

  private void closeClients() {
    // close all socket connections
    Socket  clientSocket = null;
    for(int i=0;i<clientSockets.size();i++) {
      if((clientSocket=(Socket)clientSockets.get(i))!=null) {
        try {
          clientSocket.close();
        } catch(IOException e) {
        }
      }
    }
  }

  public void run() {
    try{
      do {
        synchronized(this) {
          try {
            wait(100);
          } catch(InterruptedException e) {
            AddInMain.log.println("run: wait interrupted ");
            closeClients();
            return;
          }
        }
        getClientsRequests();
      }while(!Thread.currentThread().interrupted());
      closeClients();
    } catch(Exception genExc) {
      AddInMain.log.println("run: general exception: ");
      genExc.printStackTrace(new PrintStream(AddInMain.log.getStream()));
    }
  }

  class AcceptConnection implements Runnable {
    public void run() {
      Socket  clientSocket = null;

      if(serverSocket == null)  return;

      try{
        serverSocket.setSoTimeout(10000);
      } catch(SocketException sockE) {
        AddInMain.log.println("Failed to set timeout: "+ sockE.getMessage());
      }

     do{
        try {
          try{
            clientSocket = serverSocket.accept();
          } catch (InterruptedIOException intrptE) {
            clientSocket = null;
            AddInMain.log.println("Accept interrupted ");
          }
        } catch (IOException e) {
          clientSocket = null;
          AddInMain.log.println("Accept: IOException " + e.getMessage());
          return;
        }
        if(clientSocket!=null) {
          synchronized(clientSockets) {
            try {
              clientSocket.setTcpNoDelay(true);
            } catch(SocketException sockExc) {
            }
            clientSockets.add(clientSocket);
          }
        }
      }while(!Thread.currentThread().interrupted());
    }
  }
}
