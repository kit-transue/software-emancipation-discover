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
import com.swemancipation.CodeRover.ServerResults;

import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.util.Enumeration;

import java.net.InetAddress;
import java.net.Socket;

import java.util.Hashtable;
import java.util.StringTokenizer;

//==============================================================================
// This class represent single client connection.
// It starts it's life cycle when the client connection is accepted and it
// end it's life cycle after it sends the reply to the client
//==============================================================================
public class ClientConnection implements Runnable {
public static final int OK                  = 200;
public static final int EVAL_ERROR          = 201;
public static final int COMMUNICATION_ERROR = 202;
public static final int NO_SERVICE          = 400;
public static final int EMPTY_SERVICE       = 401;

private transient ConnectionsListInterface connections;
private transient Socket client;
private ModelConnection connectedServer;
private int timeout = 0;
private String localHostIp;
private boolean chacheConnections;

  //----------------------------------------------------------------------------
  // Constructor will create and run client connection object. It receves the
  // reference to the central application to be able to remove itself from the
  // active connections list when request processing is done.
  //----------------------------------------------------------------------------
  public ClientConnection(ConnectionsListInterface connectionsList,Socket clientSocket, boolean cache) {
     chacheConnections = cache;
     this.connections = connectionsList;
     client = clientSocket;
     try {
        InetAddress local = InetAddress.getLocalHost();
        localHostIp = local.getHostAddress();
     } catch (Exception e) {
        localHostIp = "127.0.0.1";
     }
     System.out.println("Client connected (client number "+(connectionsList.count()+1)+").");
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  public synchronized ModelConnection getConnectedServer() {
      return connectedServer;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  public synchronized void setConnectedServer(ModelConnection server) {
      connectedServer = server;
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  public synchronized int getTimeout() {
      return timeout;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  public synchronized void setTimeout(int val) {
      timeout = val;
  }
  //----------------------------------------------------------------------------

  //----------------------------------------------------------------------------
  // This function will disconnect client from the server and will remove the
  // connection entry from the connections list ( so gc will be able to destroy
  // this connection).
  //----------------------------------------------------------------------------
  public boolean disconnect() {
     try {
        if(this.getConnectedServer()!=null) {
            OutputStream os = client.getOutputStream();
            ServerReply reply = new ServerReply();
            reply.setErrorCode(COMMUNICATION_ERROR);
            reply.write(os);
            os.close();
        }
        client.close();
     } catch(IOException disconnectError) {
        connections.remove(this);
        return false;
     }
     connections.remove(this);
     return true;
  }
  //----------------------------------------------------------------------------


  //----------------------------------------------------------------------------
  // All real work will happen here
  //----------------------------------------------------------------------------
  public void run() {
  ClientRequest request = new ClientRequest();
  ServerReply   reply   = new ServerReply();
  ModelConnection modelConnection=null;
  InputStream   in;
  OutputStream  out;

     try {
         in  = client.getInputStream();
     } catch(IOException inEx) {
         System.out.println("ERROR : unable to get client in stream.");
         return;
     }

     try {
         out  = client.getOutputStream();
     } catch(IOException inEx) {
         System.out.println("ERROR : unable to get client out stream.");
         return;
     }

     try {
        request.read(in);
     } catch(IOException rdEx) {
        System.out.println("ERROR : error reading client request.");
        return;
     }
     String service = request.getObject();
     ModelsCollection models = connections.getModels();
     reply.addHeader("Host",localHostIp);
     reply.addHeader("Accept","*/*");
     reply.addHeader("Accept-Language","en-us");
     if(request.getMethod().toUpperCase().trim().equals("POST")) {
         if(request.isHeaderPresent("USER-AGENT")) {
             String client = request.getHeaderValue("USER-AGENT");
             if(client.toUpperCase().equals("DISCOVER")) {
                 int index  = service.lastIndexOf('/');
                 service = service.substring(index+1);
                 reply.addHeader("Content-Type","application/discover");
                 if(service!=null) {
                     if(models!=null) {
                         try {
                             modelConnection = models.useConnection(service);
                             setConnectedServer(modelConnection);
                         } catch(IllegalArgumentException ilex) {
                             modelConnection = null;
                             setConnectedServer(null);
                         }
                         setConnectedServer(modelConnection);
                         if(modelConnection!=null) {
                             DataSource source = modelConnection.getDataSource();
                             String command = new String(request.getData());
                             ServerResults results =null;
                             try {
                                 results = source.Exec(command);
                                 if(chacheConnections==false)
                                     modelConnection.disconnect();
                             } catch(OutOfMemoryError outmem) {
                                 System.out.println("ERROR: Out of virtual memory while evaluationg command (will try to run GC).");
                                 System.gc();
                                 results=null;
                                 reply.setErrorCode(COMMUNICATION_ERROR);
                             }
                             if(results==null || results.getCommCode() != ServerResults.OK) {
                                 // Error communicationg with the model server
                                 source.setConnected(false);
                                 reply.setErrorCode(COMMUNICATION_ERROR);
                             } else {
                                 try {
                                     reply.setData(((String)results.getValue()).getBytes());
                                     if(results.isOk())
                                         reply.setErrorCode(OK); // Everithing OK
                                     else
                                         reply.setErrorCode(EVAL_ERROR); // Command evaluation failed
                                 } catch(OutOfMemoryError outmemb) {
                                     System.out.println("ERROR: Out of virtual memory while froming reply block (will try to run GC).");
                                     System.gc();
                                     reply.setData(null);
                                     reply.setErrorCode(COMMUNICATION_ERROR);
                                 }
                             }
                         } else {
                             // No service defined in the services file
                             reply.setErrorCode(NO_SERVICE);
                         }
                     } else {
                         // Service file is empty
                         reply.setErrorCode(NO_SERVICE);
                     }
                } else {
                    // Empty service was send
                    reply.setErrorCode(EMPTY_SERVICE);
                }
            } else {
                if((service!=null) && (service.length()>0)) {
                    reply.addHeader("Content-Type","application/discover");
                    String serverSrcRoot = models.getServiceRoot(service);
                    StringBuffer replyData = new StringBuffer();
                    replyData.append("Host  : \""+localHostIp+"\"\r\n");
                    replyData.append("Model : \""+service+"\"\r\n");
                    if(serverSrcRoot!=null) {
                        replyData.append("Source-Root : \""+serverSrcRoot+"\"\r\n");
                    }
                    replyData.append("To view this page you need to download CodeRover browser.\r\n\r\n");
                    reply.setData(replyData.toString().getBytes());
               } else {
                   String message = "To view this page you need to download CodeRover browser.\r\n\r\n";
                   reply.setMessage(message.getBytes());
               }
            }
        }
     }
     if(request.getMethod().toUpperCase().trim().equals("GET")) {
        if(service.charAt(0)=='/') service = service.substring(1);
        if((service!=null) && (service.length()>0)) {
            if(service.equals("shutdown_server_333901")) {
                 reply.removeHeader("Content-Type");
                 reply.addHeader("Content-Type","text/html");
                 reply.setData(new String("Shutting down HTTP server").getBytes());
                 try {
                   reply.write(out);
                   out.flush();
                 } catch(IOException wrEx) {
                     System.out.println("ERROR : Error writing server reply (Operation cancelled on a client?).");
                 } catch(OutOfMemoryError merr) {
                     System.out.println("ERROR : Out of virtual memory while sending reply to the client (will try to run GC).");
                 }
                 if(disconnect()==false) {
                     System.out.println("ERROR : Error disconnecting from the client.");
                 }
                 connections.shutDownServer();
                 System.out.println("FINAL : Shut down request detected.");
                 return;
            }
            reply.addHeader("Content-Type","application/discover");
            String serverSrcRoot = models.getServiceRoot(service);
            StringBuffer replyData = new StringBuffer();
            replyData.append("Host  : \""+localHostIp+"\"\r\n");
            replyData.append("Model : \""+service+"\"\r\n");
            if(serverSrcRoot!=null) {
                replyData.append("Source-Root : \""+serverSrcRoot+"\"\r\n");
            }
            replyData.append("To view this page you need to download CodeRover browser.\r\n\r\n");
            reply.setData(replyData.toString().getBytes());
        } else {
            StringBuffer htmlReply = new StringBuffer();
            htmlReply.append("<html>"+"\r\n");
            htmlReply.append("   <head>"+"\r\n");
            htmlReply.append("       <title>"+"\r\n");
            htmlReply.append("           Available CodeRower services"+"\r\n");
            htmlReply.append("       </title>"+"\r\n");
            htmlReply.append("   </head>"+"\r\n");
            htmlReply.append("   <body bgcolor=\"#EDEDED\" text=\"000000\" link=\"5a3962\" vlink=\"5a3962\" alink=\"5a3962\">"+"\r\n");
            htmlReply.append("       <center>"+"\r\n");
            htmlReply.append("           <header>"+"\r\n");
            htmlReply.append("               Services available on the CodeRower HTTP server:"+"\r\n");
            htmlReply.append("           </header>"+"\r\n");
            htmlReply.append("       </center>"+"\r\n");
            htmlReply.append("       <table width=\"600\" border=\"0\" cellspacing=\"2\" cellpadding=\"2\">"+"\r\n");
            Hashtable services = models.getDescriptions();
            Enumeration keys = services.keys();


            while(keys.hasMoreElements()) {
               htmlReply.append("           <tr>"+"\r\n");
               String key = (String) keys.nextElement();
               String value = (String) services.get(key);
               htmlReply.append("               <td bgcolor=\"#CEC6CE\"><font color=\"#5A3962\" face=\"Arial, Helvetica, sans-serif\" size=\"-1\">"+"\r\n");
               htmlReply.append("                   <a href=\"http://"+localHostIp+"/"+key+"\">"+"\r\n");
               htmlReply.append("                       "+key+"\r\n");
               htmlReply.append("                    </a>"+"\r\n");
               htmlReply.append("               </td>"+"\r\n");
               htmlReply.append("               <td bgcolor=\"#CEC6CE\"><font color=\"#5A3962\" face=\"Arial, Helvetica, sans-serif\" size=\"-1\">"+"\r\n");
               htmlReply.append("                   "+value+"\r\n");
               htmlReply.append("               </td>"+"\r\n");
               htmlReply.append("           </tr>"+"\r\n");
            }
            htmlReply.append("       </table>"+"\r\n");
            htmlReply.append("   </body>"+"\r\n");
            htmlReply.append("</html>"+"\r\n");
            reply.setMessage(htmlReply.toString().getBytes());
        }
     }

     // Sending reply to the client
     try {
         reply.write(out);
         out.flush();
     } catch(IOException wrEx) {
         System.out.println("ERROR : Error writing server reply (Operation cancelled on a client?).");
     } catch(OutOfMemoryError merr) {
         System.out.println("ERROR : Out of virtual memory while sending reply to the client (will try to run GC).");
         System.gc();
     }

     setConnectedServer(null);
     if(modelConnection!=null) modelConnection.setInUse(false);

     if(disconnect()==false) {
        System.out.println("ERROR : Error disconnecting from the client.");
     }
     System.out.println("Client removed ("+connections.count()+" left).");
     System.gc();
  }
  //----------------------------------------------------------------------------
}


class ClientRequest {
public static final int OK = 0 ;
public static final int COMMUNICATION_ERROR = 1;
public static final int NON_HTTP_REQUEST    = 2;
private String     method;
private String     object;
private String     protocol;
private String     version;
private Hashtable  mime=new Hashtable();
private byte[]     data;
private int        error;


     public ClientRequest() {
     }

     private String readLine(InputStream in) {
     StringBuffer buf = new StringBuffer();
     char ch;
         try {
             do {
                 ch = (char)in.read();
                 if(ch!='\n' && ch!='\r') buf.append(ch);
             } while(ch!='\n');
        } catch(IOException ioex) {
             return null;
        }
        return buf.toString();
     }

     //-------------------------------------------------------------------------
     // This method reads all datafrom the given stream and fills all internal
     // class fields.
     //-------------------------------------------------------------------------
     public void read(InputStream in) throws IOException {
     String line;
     int    dataLength;

         method=null;
         object=null;
         protocol=null;
         version=null;
         mime.clear();
         dataLength = 0;
         data=null;
         error = OK;

         // Reading http header
         line = readLine(in);
         System.out.println("REQUEST HEADER : "+line);
         if(line==null) {
             error = COMMUNICATION_ERROR;
             return;
         }
         StringTokenizer headerTokenizer = new StringTokenizer(line," ",false);
         String protAndVer="";
         int field = 0;
         while(headerTokenizer.hasMoreTokens()) {
             switch(field) {
                 case 0  : method     = headerTokenizer.nextToken().trim(); break;
                 case 1  : object     = headerTokenizer.nextToken().trim(); break;
                 case 2  : protAndVer = headerTokenizer.nextToken().trim(); break;
                 default : headerTokenizer.nextToken();
            }
            field++;
         }
         int versionPos = protAndVer.indexOf("/");
         if(versionPos<0) {
            protocol = protAndVer.trim();
            version  = "";
         } else {
            protocol = protAndVer.substring(0,versionPos).toUpperCase().trim();
            version = protAndVer.substring(versionPos+1,protAndVer.length()).trim();
         }
         if(protocol.equals("HTTP")==false) {
             error = NON_HTTP_REQUEST;
             return;
         }
         // Reading MIME headers
         do {
              line = readLine(in);
              System.out.println("MIME HEADER : "+line);
              if(line==null) {
                 error = COMMUNICATION_ERROR;
                 return;
              }
              if(line.length()>0) {
                 int sep = line.indexOf(":");
                 if(sep>0) {
                    String propertyName = line.substring(0,sep).trim().toUpperCase();
                    String propertyValue = line.substring(sep+1,line.length()).trim();
                    mime.put(propertyName,propertyValue);
                 }
              }
         } while(line.length()>0);
         if(method.toUpperCase().equals("POST")) {
            String strLen = (String)mime.get("CONTENT-LENGTH");
            if(strLen!=null) {
                dataLength = Integer.parseInt(strLen);
                int readed = 0;
                data = new byte [dataLength];
                while(readed<dataLength) {
                   readed+=in.read(data,readed,dataLength-readed);
                }
                String clientRequest = new String(data);
            }
         }
     }
     //-------------------------------------------------------------------------


     //-------------------------------------------------------------------------
     // Return the protocol name used by client to generate request.
     // Usually "HTTP"
     //-------------------------------------------------------------------------
     public String getProtocol() {
         return protocol;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the version of the protocol used by client to generate request.
     //-------------------------------------------------------------------------
     public String getVersion() {
         return version;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the HTTP method name used in the request.
     //-------------------------------------------------------------------------
     public String getMethod() {
         return method;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the string describing object requested.
     //-------------------------------------------------------------------------
     public String getObject() {
         return object;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Check if the given MIME header present in the request.
     //-------------------------------------------------------------------------
     public boolean isHeaderPresent(String key) {
        return mime.get(key.toUpperCase().trim())!=null;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the value of the given MIME header or null if no header present.
     //-------------------------------------------------------------------------
     public String getHeaderValue(String key) {
         return (String)mime.get(key.toUpperCase().trim());
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the length of the data attached to the request or 0 if there is
     // no data.
     //-------------------------------------------------------------------------
     public int getDataLength() {
         if(data==null) return 0;
         return data.length;
     }
     //-------------------------------------------------------------------------

     //-------------------------------------------------------------------------
     // Returns the reference to the byte array with the data attached to the
     // request or 0 if no data attached.
     //-------------------------------------------------------------------------
     public byte [] getData() {
         return data;
     }
     //-------------------------------------------------------------------------

}



class ServerReply {
private int errorCode=200;
private boolean operationOk;
private byte [] data;
private Hashtable  mime=new Hashtable();

    public ServerReply() {
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void setErrorCode(int val) {
        errorCode = val;
    }
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public int getErrorCode() {
        return errorCode;
    }
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void setData(byte [] val) {
        removeHeader("Content-Length");
        addHeader("Content-Length",String.valueOf(val.length));
        data = val;
    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void setMessage(byte [] val) {
        removeHeader("Content-Length");
        data = val;
    }
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public byte [] getData() {
        return data;
    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void addHeader(String key,String value) {
       mime.put(key.trim(),value.trim());
    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void removeHeader(String key) {
       mime.remove(key);
    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    private String makeErrorHeader(int code,String text) {
      StringBuffer header = new StringBuffer();
      header.append("HTTP/1.0 "+code+" "+text.trim());
      return header.toString();
    }
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    public void write(OutputStream out) throws IOException {
    StringBuffer reply = new StringBuffer();

       switch(errorCode) {
           case ClientConnection.OK:
                reply.append(makeErrorHeader(errorCode,"OK")+"\r\n");
                break;
           case ClientConnection.EVAL_ERROR:
                reply.append(makeErrorHeader(errorCode,"Error evaluationg command")+"\r\n");
                break;
           case ClientConnection.COMMUNICATION_ERROR:
                reply.append(makeErrorHeader(errorCode,"Server communication error")+"\r\n");
                break;
           case ClientConnection.NO_SERVICE:
                reply.append(makeErrorHeader(errorCode,"Service not found")+"\r\n");
                break;
           case ClientConnection.EMPTY_SERVICE:
                reply.append(makeErrorHeader(errorCode,"Empty service was send")+"\r\n");
                break;
           default:
                reply.append(makeErrorHeader(errorCode,"Unknown error")+"\r\n");
                break;
       }

       Enumeration keys = mime.keys();
       while(keys.hasMoreElements()) {
          String key = (String) keys.nextElement();
          String value = (String)mime.get(key);
          reply.append(key+": "+value+"\r\n");
       }
       reply.append("\r\n");
       if(data!=null) {
           reply.append(new String(data));
       }
       out.write(reply.toString().getBytes());
    }
    //--------------------------------------------------------------------------
}
