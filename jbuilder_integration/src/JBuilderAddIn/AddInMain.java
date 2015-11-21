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


import com.borland.jbuilder.JBuilderToolBar;

import com.borland.primetime.PrimeTime;

import com.borland.primetime.ide.BrowserListener;
import com.borland.primetime.ide.ContextActionProvider;
import com.borland.primetime.ide.Browser;
import com.borland.primetime.ide.BrowserIcons;
import com.borland.primetime.ide.NodeViewer;
import com.borland.primetime.ide.MessageCategory;
import com.borland.primetime.ide.MessageView;

import com.borland.primetime.util.VetoException;

import com.borland.primetime.node.Node;
import com.borland.primetime.node.Project;

import com.borland.primetime.editor.EditorPane;
import com.borland.primetime.editor.EditorAction;

import com.borland.primetime.vfs.Url;

import java.lang.System;
import java.io.*;
import java.util.Vector;

import java.awt.event.ActionEvent;

import javax.swing.*;
import javax.swing.event.*;

import com.swemancipation.CodeRover.Utilities.*;

public class AddInMain implements BrowserListener,
                                  CRActionHandler,
                                  ShowMessageListener,
                                  OpenFileListener {

  public static DebugLog log = new DebugLog();
  public static String homeDir = null;

  private Vector  openListeners = new Vector();
  private Vector  closeListeners = new Vector();
  protected CRCommander actionManager = null;
  protected AddInCommands commands = new AddInCommands();
  protected MessageCategory crCategory  = new MessageCategory("Workbench");
  public static EnvironmentVariables  envVars = new EnvironmentVariables();
  public SocketConnection connection  = null;

  public AddInMain() {
    connection  = new SocketConnection(this,commands);
    addOpenListener(connection);
    addCloseListener(connection);
    addCloseListener(commands);
    commands.addOpenFileListener(this);
    commands.addShowMessageListener(this);
    CRCommander.RegisterStaticCRActionHandler(this);
    actionManager = new CRCommander();
  }

  public static void initOpenTool(byte major, byte minor) {
    // Check OpenTools version number
    if (major != PrimeTime.CURRENT_MAJOR_VERSION)
      return;
    // now listen and listen and listen...
    Browser.addStaticBrowserListener(new AddInMain());
  }

  public void browserViewerDeactivating(Browser parm1, Node parm2, NodeViewer parm3)
                                throws VetoException {
  }

  public void browserViewerActivated(Browser parm1, Node parm2, NodeViewer parm3) {
  }

  public void browserNodeClosed(Browser browser, Node parm2) {
  }

  public void browserNodeActivated(Browser browser, Node activeNode) {
  }

  public void browserProjectClosed(Browser parm1, Project parm2) {
  }

  public void browserProjectActivated(Browser parm1, Project parm2) {
  }

  public void browserClosed(Browser parm1) {
    try {
      fireClose();
    } catch(Exception e) {
      log.println("Close Exception: " + e.getMessage());
      e.printStackTrace(new PrintStream(log.getStream()));
    }
  }

  public void browserClosing(Browser parm1) throws VetoException {
  }

  public void browserDeactivated(Browser parm1) {
  }

  private boolean isCRTabVisible(Browser browser) {
    if(browser.isMessagePaneVisible()==false) {
      return false;
    }
    return isCRTabExist(browser);
  }

  private boolean isCRTabExist(Browser browser) {
    MessageView messageView = browser.getMessageView();
    if(messageView == null) {
      return false;
    }

    MessageCategory[] messageCategories = messageView.getTabs();
    for(int i=0;i<messageCategories.length;i++) {
      if(messageCategories[i] == crCategory) {
        return  true;
      }
    }
    return false;
  }

  private void hideCRWindow(Browser browser) {
    if(browser == null) return;
    MessageView messageView = browser.getMessageView();
    if(messageView == null) return;
    try {
      messageView.removeTab(crCategory);
    } catch(VetoException e) {
    }
  }

  private void showCRWindow(Browser browser) {
    if(browser == null) return;
    if(browser.isMessagePaneVisible()==false)
      browser.setMessagePaneVisible(true);
    if(isCRTabExist(browser)==false)
      browser.getMessageView().addCustomTab(crCategory, actionManager);
    browser.getMessageView().showTab(crCategory);
  }

  public void updateActions() {
    actionManager.updateActions();
  }

  private void showCodeRoverMenu() {
    actionManager.show();
  }

  public void browserActivated(Browser browser) {
  }

  public void browserOpened(Browser browser) {
    Thread startThread = new Thread() {
      public void run() {
        if(startConnection()==true)
          showCodeRoverMenu();
      }
    };

    startThread.start();
  }

  // returns zero if command is unknown
  // otherwise returns current state of the command
  public int  getActionState(int action) {
    int state = 0;
    Browser browser = Browser.getActiveBrowser();
    switch(action) {
      case CRCommander.MENUACTION_INFO:
      case CRCommander.MENUACTION_DEFINITION:
      case CRCommander.MENUACTION_BROWSE:
        if(browser.getOpenNodeCount()>0 && connection.hasConnections()) {
          state |= CRCommander.ENABLEMENU;
        } else {
          state |= CRCommander.DISABLEMENU;
        }
        state |= action;
        break;
      case CRCommander.MENUACTION_ACTIVATE:
        if(connection.hasConnections()) {
          state |= CRCommander.ENABLEMENU;
        } else {
          state |= CRCommander.DISABLEMENU;
        }
        state |= action;
        break;
      case CRCommander.MENUACTION_WINDOW:
        state |= action;
        if(isCRTabVisible(browser) == true) {
          state |= CRCommander.CHECKMENU;
        } else {
          state |= CRCommander.UNCHECKMENU;
        }
        state |= CRCommander.ENABLEMENU;
        break;
      case CRCommander.MENUACTION_EXECUTE_CODEROVER:
        state |= CRCommander.ENABLEMENU;
        if(isCRInstalled()==false)
          state |= CRCommander.DISABLEMENU;
        state |= action;
        break;
    }
    return state;
  }

  public static boolean isCRInstalled() {
    if(envVars.getProperty("CODE_ROVER_HOME")==null)  return false;
    return true;
  }

  public void menuActionPerformed(int action) {
    Browser browser = Browser.getActiveBrowser();
    EditorPane  activeEditor = EditorAction.getFocusedEditor();
    switch(action) {
      case CRCommander.MENUACTION_WINDOW:
        if(isCRTabExist(browser) == false) {
          showCRWindow(browser);
        } else {
          if(isCRTabVisible(browser)==true)
            hideCRWindow(browser);
          else
            browser.setMessagePaneVisible(true);
        }
        break;
      case CRCommander.MENUACTION_EXECUTE_CODEROVER:
        String  projectName = "";
        String  projectFile = "";
        if(browser != null) {
          Project project = browser.getProjectView().getActiveProject();
          if(project != null) {
            Url projectPath = project.getProjectPath();
            projectName = projectPath.getFileObject().getName();
            projectFile = projectPath.getFile().trim();
            if(!projectFile.endsWith("/") && !projectFile.endsWith("\\"))
              projectFile += "/";
            projectFile += projectName;
            String fileExt = projectPath.getFileExtension();
            if(( fileExt == null) || (fileExt.length()==0))
              fileExt = ".jpr";
            projectFile += fileExt;
          }
          switch(commands.startCR(projectName,projectFile)) {
            case AddInCommands.NO_HOME_VAR:
              JOptionPane.showMessageDialog(Browser.getActiveBrowser(), "Can not determine home directory.",
                                "JBuilder Addin", JOptionPane.ERROR_MESSAGE);
              break;
            case AddInCommands.CANT_START_CR:
              JOptionPane.showMessageDialog(Browser.getActiveBrowser(),"Error starting DIScover.","JBuilder AddIn",
                              JOptionPane.ERROR_MESSAGE);
              break;
          }
        }
        break;
      case CRCommander.MENUACTION_INFO:
        commands.freezeSelection();
        connection.sendEvent("inst_def\t"+commands.getSelectedFile()+"\t"+
                                          commands.getSelectedOffset()+"\t"+
                                          commands.getSelectedLine()+"\t"+
                                          commands.getSelectedColumn()+"\n");
        break;
      case CRCommander.MENUACTION_DEFINITION:
        commands.freezeSelection();
        connection.sendEvent("open_def\t"+commands.getSelectedFile()+"\t"+
                                          commands.getSelectedOffset()+"\t"+
                                          commands.getSelectedLine()+"\t"+
                                          commands.getSelectedColumn()+"\n");
        break;
      case CRCommander.MENUACTION_BROWSE:
        commands.freezeSelection();
        connection.sendEvent("query\t"+commands.getSelectedFile()+"\t"+
                                          commands.getSelectedOffset()+"\t"+
                                          commands.getSelectedLine()+"\t"+
                                          commands.getSelectedColumn()+"\n");
        break;
      case CRCommander.MENUACTION_ACTIVATE:
        connection.sendEvent("popup");
        break;
    }
  }

  public static Node getFileNode(Browser browser, String fileName) {
    Node[] openNodes = browser.getAllOpenNodes();
    log.println("getFileNode: Open nodes count " + browser.getOpenNodeCount());
    for(int idx=0;idx<browser.getOpenNodeCount();idx++) {
      String convertedName = SocketConnection.resolvePathSeparators(openNodes[idx].getLongDisplayName());
      log.println("getFileNode: Comparing " + convertedName +
                    " vs " + fileName);
      if(convertedName.compareToIgnoreCase(fileName)==0) {
        return openNodes[idx];
      }
    }
    return null;
  }

  public boolean openFile(String file, int line) {
    try {
      log.println("openFile: opening file...");
      Browser browser = Browser.getActiveBrowser();
      if(browser == null) {
        log.println("openFile: No active browser.");
        Browser[] browsers = Browser.getBrowsers();
        if(browsers.length > 0) {
          log.println("openFile: getting the first one.");
          browser = browsers[0];
        }
        if(browser == null) {
          log.println("openFile: no browser.");
          return false;
        }
      }
      Node fileNode = getFileNode( browser, file);
      if(fileNode == null) {
        SwingUtilities.invokeLater(new OpenAndActivate(browser, file, line));
        return true;
      } else {
        SwingUtilities.invokeLater(new Activate(browser,fileNode, line));
        return true;
      }
    } catch (Exception exc) {
      log.println("openFile: General exception");
    }
    return false;
  }

  public void showMessage(String message) {
    if(!message.endsWith("\n")) message += "\n";
    actionManager.showMessage(message);
  }

  public static String getCurrentSelection(int[] line,int[] column,int[] offset) {
    try{
      EditorPane  activeEditor = EditorAction.getFocusedEditor();
        if(activeEditor != null) {
        offset[0] = activeEditor.getSelectionStart() + (activeEditor.getSelectionEnd() - activeEditor.getSelectionStart())/2;
        line[0] = activeEditor.getLineNumber(offset[0]);
        column[0] = offset[0] - activeEditor.calcCaretPosition(line[0],1);
        Node  fileNode = Browser.getActiveBrowser().getActiveNode();
        return  fileNode.getLongDisplayName().replace('\\','/');
      } else {
        line[0] = -1;
        column[0] = -1;
        offset[0] = -1;
        return null;
      }
    } catch(Exception e) {
      log.println("getCurrentSelection: general exception "+e.getMessage());
    }
    return null;
  }

  protected boolean fireOpen() {
    boolean result = true;
    for( int i=0;i<openListeners.size();i++)
      if(((OpenListener)openListeners.get(i)).open()==false && result==true)
        result = false;
    return result;
  }

  protected void fireClose() {
    for( int i=0;i<closeListeners.size();i++)
      ((CloseListener)closeListeners.get(i)).close();
  }

  public void addCloseListener(CloseListener listener) {
    if(closeListeners.contains(listener)==false)
      closeListeners.add(listener);
  }

  public void addOpenListener(OpenListener listener) {
    if(openListeners.contains(listener)==false)
      openListeners.add(listener);
  }

  public void removeCloseListener(CloseListener listener) {
    if(closeListeners.contains(listener)==true)
      closeListeners.remove(listener);
  }

  public void removeOpenListener(OpenListener listener) {
    if(openListeners.contains(listener)==true)
      openListeners.remove(listener);
  }

  private boolean startConnection() {
    String homeName, jbuilderPath;
    homeName = AddInMain.envVars.getProperty("CODE_ROVER_HOME");
    if(homeName == null) {
      log.println("startConnection: There is no CODE_ROVER_HOME variable");
      homeName = AddInMain.envVars.getProperty("PSETHOME");
      if(homeName == null) {
        log.println("startConnection: No CODE_ROVER_HOME or PSETHOME variable.");
        JOptionPane.showMessageDialog(Browser.getActiveBrowser(), "There is neither 'CODE_ROVER_HOME' nor 'PSETHOME' environment variable set in the system.",
            "Integration disabled.", JOptionPane.ERROR_MESSAGE);
        return false;
      }
    }

    homeDir = homeName;
    log.println("startConnection: Home directory is " + homeDir);

    jbuilderPath = AddInMain.envVars.getProperty("CR_JBUILDER_PATH");
    if(jbuilderPath == null) {
      log.println("startConnection: No CR_JBUILDER_PATH variable.");
      JOptionPane.showMessageDialog(Browser.getActiveBrowser(), "No CR_JBUILDER_PATH variable.",
          "Integration disabled.", JOptionPane.ERROR_MESSAGE);
      return false;
     }

    if(!JStartExternalExe.isConnected() && isCRInstalled()) {
      log.println("startConnection: not connected to StartExe service.");

      if(JStartExternalExe.connectToService()!=JStartExternalExe.OK) {
        log.println("startConnection: DevXLauncher is not running.");

        String commandLine = "\"";
        commandLine += homeName;
        commandLine += "/bin/DevXLauncher.exe\" -norover";
        try {
          log.println("startConnection: Starting DevXLauncher: " + commandLine);
          Process modelBuild = Runtime.getRuntime().exec(commandLine);
          log.println("startConnection: Trying to connect to StartExe service...");
          final int nWaitPeriod = 300; // It's about 45 seconds
          int numAttempts=0;
          int connectCode = 0;
          while((connectCode = JStartExternalExe.connectToService())!=JStartExternalExe.OK &&
		  	numAttempts <= nWaitPeriod) {
            log.println("startConnection: Can not connect to StartExe service: " + connectCode + " Attempt " + numAttempts);
            numAttempts++;
            try {
            	Thread.currentThread().sleep(numAttempts);
            } catch (InterruptedException ie){
            	break;
            }
          }
          if(connectCode!=JStartExternalExe.OK) {
        	JOptionPane.showMessageDialog(Browser.getActiveBrowser(), "Can not connect to StartExe service.",
                    "Integration disabled.", JOptionPane.ERROR_MESSAGE);
            return false;
          } else
	          log.println("startCR: Connection established.");
        } catch(IOException e) {
          log.println("startCR: IOException: "+e.getMessage());
          JOptionPane.showMessageDialog(Browser.getActiveBrowser(), "Exception while connection to StartExe service: " + e.getMessage(),
              "Integration disabled.", JOptionPane.ERROR_MESSAGE);
          return false;
        }
      }
    }
    if(fireOpen()==false) {
      commands.showMessage("Integration disabled: Can not open connection.");
      return false;
    }
    return true;
  }
}

class GoToLine implements Runnable {
  private int line;
  public GoToLine(int line) {
    this.line = line;
  }
  public void run() {
    EditorPane  activeEditor = EditorAction.getFocusedEditor();
    if(activeEditor!=null) {
      activeEditor.gotoLine(line,true,0);
      Browser.getActiveBrowser().toFront();
      AddInMain.log.println("openFile: go to line " + line);
      return;
    }
    AddInMain.log.println("openFile: no active editor");
  }
}

class OpenAndActivate implements Runnable {
  private Browser browser;
  private String file;
  private int line;
  public OpenAndActivate(Browser browser, String file, int line) {
    this.browser = browser;
    this.file = file;
    this.line = line;
  }
  public void run() {
    AddInMain.log.println("openFile: no such node " + file);
    try {
      browser.doOpen(new File(file),false);
    } catch (Exception browseExc) {
      AddInMain.log.println("openFile: Browser exception: " + browseExc.getMessage());
      return;
    }
    AddInMain.log.println("openFile: file opened");
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        Node fileNode = AddInMain.getFileNode( browser, file);
        try {
          browser.setActiveNode(fileNode,true);
          browser.requestFocus();
          SwingUtilities.invokeLater(new GoToLine(line));
          AddInMain.log.println("openFile: node finded " + fileNode);
          return;
        } catch (Exception e) {
          AddInMain.log.println("openFile: Activation exception");
        }
      }
    });
  }
}

class Activate implements Runnable {
  private Browser browser;
  private Node fileNode;
  private int line;
  public Activate(Browser browser, Node node, int line) {
    this.browser = browser;
    fileNode = node;
    this.line = line;
  }
  public void run() {
    try {
      synchronized(browser) {
        browser.setActiveNode(fileNode,true);
        browser.requestFocus();
        SwingUtilities.invokeLater(new GoToLine(line));
      }
      return;
    } catch (Exception e) {
      AddInMain.log.println("openFile: Activation exception");
    }
  }
}
