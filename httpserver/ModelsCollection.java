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

/*
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */
package httpserver;
import java.util.Hashtable;
import java.util.Vector;
import java.io.*;
import java.lang.IllegalArgumentException;


//==============================================================================
// This class designed to represent all available models on this HTTP server.
// Each model represented as a vector ot the model connections. Each model
// connection represend separated model server. Connections vectors are
// located in the hashtable, so yopu will be avle to access the connections
// vector using model name.
//==============================================================================
public class ModelsCollection {
private transient Hashtable models        =  new Hashtable();
private transient Hashtable roots         =  new Hashtable();
private transient Hashtable descriptions  =  new Hashtable();

  public ModelsCollection() {
  }


  //----------------------------------------------------------------------------
  // This function will set lowest possible priority for the selected server.
  //----------------------------------------------------------------------------
  public void lowPriority(ModelConnection server) {
      Vector servers = (Vector)models.get(server.getService());
      if(servers!=null) {
          if(servers.contains(server)) {
              servers.remove(server);
              servers.add(server);
          }
      }
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This function returns server root used by the given service.
  // or null if the root was not set.
  //----------------------------------------------------------------------------
  public String getServiceRoot(String service) {
      return (String)roots.get(service);
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This function returns hashtable with all available services and description.
  //----------------------------------------------------------------------------
  public Hashtable getDescriptions() {
      return descriptions;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This method will load connections description file. The file has the
  // following format:
  // HTTPSERVICENAME HOST LOCALSERVICENAME\n
  // ....
  // HTTPSERVICENAME HOST LOCALSERVICENAME\n
  // It will return true if everything was OK.
  //----------------------------------------------------------------------------
  public boolean loadServerList(String filename) {
      File service = null;
      service = new File(filename);

      if(service.exists()==false) {
         System.out.println("ERROR : unable to locate service settings file : "+filename);
         return false;
      }

      try {
          FileInputStream is = new FileInputStream(service);
          boolean skipSpaces = true;
          int fieldCount = 0;
          int line = 0;
          StringBuffer serviceName = new StringBuffer();
          StringBuffer hostName    = new StringBuffer();
          StringBuffer modelName   = new StringBuffer();
          StringBuffer serverRoot  = new StringBuffer();
          StringBuffer command     = new StringBuffer();
          StringBuffer description = new StringBuffer();
          boolean inString = false;
          while(is.available()>0) {
             char ch = (char)is.read();
             if(ch=='\r') continue;
             if(ch=='\"') {
                 if(inString == false) inString=true;
                 else {
                     skipSpaces=false;
                     inString = false;
                 }
                 continue;
             }
             if(((ch==' ') || (ch=='\t')) && !inString) {
                 if(skipSpaces==true) continue;
                 fieldCount++;
                 skipSpaces = true;
                 continue;
             }

             if(ch=='\n') {
                if(fieldCount>=2) {
                    if(serverRoot.length()>0) {
                        roots.put(serviceName.toString().trim(),serverRoot.toString().trim());
                    }
                    if(description.length()>0) {
                         descriptions.put(serviceName.toString().trim(),description.toString().trim());
                    } else {
                       if(descriptions.get(serviceName.toString().trim())==null) {
                          descriptions.put(serviceName.toString().trim(),"");
                       }
                    }
                    ModelConnection connection = new ModelConnection(serviceName.toString(),hostName.toString(),modelName.toString(),command.toString());
                    Vector modelsList = (Vector)models.get(serviceName.toString());
                    if(modelsList==null) {
                        modelsList = new Vector();
                        modelsList.add(connection);
                        models.put(serviceName.toString(),modelsList);
                    } else {
                       int i;
                       for(i=0; i<modelsList.size();i++) {
                           ModelConnection p = (ModelConnection)modelsList.get(i);
                           if( (p.getHost().toLowerCase().equals(hostName.toString().toLowerCase())) &&
                               (p.getModel().toLowerCase().equals(modelName.toString().toLowerCase())) ) {
                               break;
                           }
                       }
                       if(i==modelsList.size()) {
                           modelsList.add(connection);
                       }
                    }
                } else {
                    System.out.println("WARNING : wrong line in the service settings file :"+(line+1));
                }
                serviceName = new StringBuffer();
                hostName    = new StringBuffer();
                modelName   = new StringBuffer();
                serverRoot  = new StringBuffer();
                command     = new StringBuffer();
                description = new StringBuffer();
                skipSpaces = true;
                fieldCount = 0;
                line++;
                continue;
             }

             skipSpaces = false;
             switch(fieldCount) {
                 case 0  : serviceName.append(ch); break;
                 case 1  : hostName.append(ch);    break;
                 case 2  : modelName.append(ch);   break;
                 case 3  : serverRoot.append(ch);  break;
                 case 4  : command.append(ch);     break;
                 case 5  : description.append(ch); break;
             }
          }
          if(fieldCount>=2) {
              if(serverRoot.length()>0) {
                  roots.put(serviceName.toString().trim(),serverRoot.toString().trim());
              }
              if(description.length()>0) {
                   descriptions.put(serviceName.toString().trim(),description.toString().trim());
              } else {
                  if(descriptions.get(serviceName.toString().trim())==null) {
                     descriptions.put(serviceName.toString().trim(),"");
                  }
              }
              ModelConnection connection = new ModelConnection(serviceName.toString(),hostName.toString(),modelName.toString(),command.toString());
              Vector modelsList = (Vector)models.get(serviceName.toString());
              if(modelsList==null) {
                  modelsList = new Vector();
                  modelsList.add(connection);
              } else {
                 int i;
                 for(i=0; i<modelsList.size();i++) {
                     ModelConnection p = (ModelConnection)modelsList.get(i);
                     if( (p.getHost().toLowerCase().equals(hostName.toString().toLowerCase())) &&
                         (p.getModel().toLowerCase().equals(modelName.toString().toLowerCase())) ) {
                         System.out.println("WARNING : Duplicate string in the service settings file :"+(line+1)+" ("+hostName.toString()+" : "+modelName.toString()+")"  );
                         break;
                     }
                 }
                 if(i==modelsList.size()) {
                     modelsList.add(connection);
                 }
              }
          }
       } catch(IOException ioe) {
           System.out.println("ERROR : error reading service settings file.");
           return false;
       }
       return true;
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // This method will try to find connections list for the given model and
  // then it will try to find free connection.
  // It will :
  // 1. Try to find alive connection which is not in use.
  // 2. Try to find broken connection and re-connect.
  // 3. Try to find not started connection, start model server and then connect
  //    to it.
  // Method will use isInUse() connection property to determine if connection
  // is in use by some other client. This is just a flag.
  // Method will use isAlive() connection property to determine if connection
  // alive (this is flag,too)
  // Method will use IsRunning() connection property to determine if server
  // is running. It will use name server call to do this so this is expensive
  // operation
  //----------------------------------------------------------------------------
  public synchronized ModelConnection useConnection(String serviceName)
                         throws IllegalArgumentException {
  int i;
  Vector connections;
        // Getting the connections vector assosiated with the given model name.
        connections =  (Vector)models.get(serviceName);

        if(connections == null) {
           throw new IllegalArgumentException("Unknown service name");
        }

        int timeout = 0;
        while(connections.size()>0 && timeout<50) {
            // Pass 1 -  trying to locate alive connection which is not in use.
            for(i=0;i<connections.size();i++) {
                ModelConnection connection = (ModelConnection)connections.get(i);
                if(connection.isAlive() == true) {
                     if(connection.isInUse() == false) {
                         connection.setInUse(true);
                         System.out.println(">>> Evaluating on the connected server.");
                         return connection;
                      }
                }
            }

            // Pass 2 -  trying to locate broken connection with running server
            for(i=0;i<connections.size();i++) {
                ModelConnection connection;
                connection = (ModelConnection)connections.get(i);
                if(connection.isAlive() == false) {
                     if(connection.isRunning() == true) {
                          if(connection.connect()==false) {
                             // If we fail to connect we must mark this connection as not running.
                             connection.setRunning(false);
                             continue;
                         }
                         System.out.println(">>> Connecting and evaluationg on the running server.");
                         connection.setInUse(true);
                         return connection;
                      }
                }
            }

            // Pass 3 - if nothing else, ve will try to start server
            for(i=0;i<connections.size();i++) {
                ModelConnection connection;
                connection = (ModelConnection)connections.get(i);
                if(connection.isRunning() == false) {
                     if(connection.run()==false) {
                         System.out.println("ERROR : Attempt to run server fails.");
                         continue;
                     }
                     System.out.println(">>> Evaluating on the newly started server.");
                     connection.setInUse(true);
                     return connection;
                }
            }
            try {
               wait(250);
            } catch(Exception e) {
            }
            timeout++;
      }
      System.out.println("WARNING : Unable to find available model server, timeout expiered.");
      return null; // We were unable to detect free connection.

  }
  //----------------------------------------------------------------------------

}
