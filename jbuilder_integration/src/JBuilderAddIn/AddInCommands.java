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

import java.io.*;
import java.util.Vector;

import com.swemancipation.CodeRover.Utilities.*;

public class AddInCommands implements CloseListener{
  public static final int NO_HOME_VAR = -1;
  public static final int CANT_START_CR = -2;
  public static final int OK = 0;

  private String   selectedFile;
  private int      selectedLine;
  private int      selectedOffset;
  private int      selectedColumn;
  private Vector   openFileListeners = new Vector();
  private Vector   showMessageListeners = new Vector();

  //--------------------------------------------------------------------------------------------
  // This fuction will execute the mrg_diff to create diff file.
  //---------------------------------------------------------------------------------------------
  private boolean makeDiff(String localfile, String modelfile, String diffile) {
    String command;
    String crHome = AddInMain.homeDir;
    if(crHome == null || crHome.length()==0)
      command="C:/Discover/Bin/mrg_diff ";
    else {
      command="\"" + crHome;
      command+="/bin/mrg_diff\" ";
    }
    command+="\"";
    command+=modelfile;
    command+="\"";
    command+=" \"";
    command+=localfile;
    command+="\"";
    command+=" \"";
    command+=diffile;
    command+="\"";

    try{
      Process mrg_diff = Runtime.getRuntime().exec(command);
      mrg_diff.waitFor();
      return true;
    } catch (InterruptedException e) {
    } catch (IOException ioe) {
    }
    return false;
  }
  //---------------------------------------------------------------------------------------------


  //--------------------------------------------------------------------------------------------
  // This fuction will execute the mrg_update to map lines
  //---------------------------------------------------------------------------------------------
  private int mapLines(String localfile, String basefile, String diffile, int localline) {
    String command;

    String crHome = AddInMain.homeDir;
    if(crHome == null || crHome.length()==0)
      command="C:/Discover/Bin/mrg_update -mapL_to_L ";
    else {
      command="\"" + crHome;
      command+="/bin/mrg_update\" -mapL_to_L ";
    }
    command+=Integer.toString(localline);
    command+=" \"";
    command+=basefile;
    command+="\"";
    command+=" \"";
    command+=diffile;
    command+="\"";
    command+=" \"";
    command+=localfile;
    command+="\"";

    try{
      Process mrg_update = Runtime.getRuntime().exec(command);
      mrg_update.waitFor();
      InputStream is = mrg_update.getInputStream();
      String  line = "-1";
      if(is.available() > 0) {
        byte[]  lineStr = new byte[200];
        int readed = is.read(lineStr);
        line = new String(lineStr,0,readed-2);  // last two charactesr is linefeed and carriage return
      }
      return Integer.parseInt(line);
    } catch (InterruptedException e) {
    } catch (IOException ioe) {
    } catch (NumberFormatException nfe) {
    }
    return -1;
  }
  //---------------------------------------------------------------------------------------------

  public int startCR(String projectName, String projectFileName) {
    AddInMain.log.println("startCR: Starting CodeRover...");
    String startCommand = "\"" + AddInMain.homeDir + "/bin/mbdriver.exe\" -projectName \"" + projectName +
                              "\" -projectFile \"" + projectFileName + "\"";
    RunExeResult res = JStartExternalExe.runExe(startCommand,"");
    if(res.isError()) {
      AddInMain.log.println("startCR: Error starting mbdriver: " + res.getErrorDescription());
      return CANT_START_CR;
    }
    AddInMain.log.println("startCR: mbdriver started: -projectName \"" + projectName +
                              "\" -projectFile \"" + projectFileName + "\"");
    return OK;
  }

  //------------------------------------------------------------------------------------------
  // This method will use mrg_update to map line in the original file into line
  // in the modified file
  //------------------------------------------------------------------------------------------
  public int getPrivateLine(String localfile, String basefile, int localline) {
    String diffile;
    File localFile = new File(localfile);
    File baseFile = new File(basefile);
    // First, check if the given files exists.
    if(localFile==null || localFile.exists()==false) return  0;
    if(baseFile==null || baseFile.exists()==false)  return  0;

    // This function will create diff file which we will need
    // to map lines
    diffile = basefile;
    diffile+=".diff";
    makeDiff(localfile,basefile,diffile);
    return mapLines(localfile,basefile,diffile,localline);
  }
  //------------------------------------------------------------------------------------------

  //------------------------------------------------------------------------------------------
  // This  function will use mrg_update to map line in the current file into line
  // in the original file which was used during modelbuild.
  //------------------------------------------------------------------------------------------
  public int getModelLine(String localfile, String basefile,int localline) {
    String diffile;
    File localFile = new File(localfile);
    File baseFile = new File(basefile);
    // First, check if the given files exists.
    if(localFile==null || localFile.exists()==false) return  0;
    if(baseFile==null || baseFile.exists()==false)  return  0;

    // This function will create diff file which we will need
    // to map lines
    diffile = basefile;
    diffile+=".diff";
    makeDiff(localfile,basefile,diffile);
    return mapLines(basefile,localfile,diffile,localline);
  }
  //------------------------------------------------------------------------------------------

  public void freezeSelection() {
    int[] line = new int[1];
    int[] column = new int[1];
    int[] offset = new int[1];
    selectedFile = AddInMain.getCurrentSelection(line,column,offset);
    selectedLine = line[0];
    selectedColumn = column[0];
    selectedOffset = offset[0];
    AddInMain.log.println("freezeSelection: File "+selectedFile+" line "+selectedLine+" column "+selectedColumn);
  }

  public int getSelectedOffset() {
    AddInMain.log.println("getSelectedOffset: offset "+selectedOffset);
    return selectedOffset;
  }

  public int getSelectedLine() {
    AddInMain.log.println("getSelectedLine: line "+selectedLine);
    return selectedLine;
  }

  public  int getSelectedColumn() {
    AddInMain.log.println("getSelectedColumn: Column "+selectedColumn);
    return selectedColumn;
  }

  public  String getSelectedFile() {
    AddInMain.log.println("getSelectedFile: File "+selectedFile);
    if(selectedFile == null)
      return "";
    return selectedFile;
  }

  public boolean close() {
    if(JStartExternalExe.isConnected()) {
      AddInMain.log.println("AddInCommand Close: Trying to disconnect from DevXLauncher... ");
      JStartExternalExe.disconnectFromService();
      AddInMain.log.println("AddInCommand Close: disconnected from DevXLauncher.");
    }
    return true;
  }

  public boolean openFile(String fileName, int line) {
    AddInMain.log.println("openFile: Opening file " + fileName + "...");
    File  file = new File(fileName);
    // if this file exists
    if(file.exists()==true) {
      fireOpenFile(file.getAbsolutePath(), line);
      return true;
    }
    AddInMain.log.println("openFile: File " + file.getAbsolutePath() + " does not exist.");
    return false;
  }

  public void showMessage(String message) {
    fireShowMessage(message);
  }

  public void addOpenFileListener(OpenFileListener listener) {
    if(openFileListeners.contains(listener) == false)
      openFileListeners.add(listener);
  }

  public void removeOpenFileListener(OpenFileListener listener) {
    if(openFileListeners.contains(listener) == true)
      openFileListeners.remove(listener);
  }

  public void addShowMessageListener(OpenFileListener listener) {
    if(showMessageListeners.contains(listener) == false)
      showMessageListeners.add(listener);
  }

  public void removeShowMessageListener(OpenFileListener listener) {
    if(showMessageListeners.contains(listener) == true)
      showMessageListeners.remove(listener);
  }

  public boolean fireOpenFile(String file, int line) {
    boolean result = true;
    for(int i=0;i<openFileListeners.size();i++)
      if(((OpenFileListener)openFileListeners.get(i)).openFile(file,line)==false && result==true)
        result = false;
    return result;
  }

  public void fireShowMessage(String message) {
    for(int i=0;i<openFileListeners.size();i++)
      ((ShowMessageListener)showMessageListeners.get(i)).showMessage(message);
  }
}
