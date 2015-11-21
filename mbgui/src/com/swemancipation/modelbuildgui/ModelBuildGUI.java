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
package com.swemancipation.modelbuildgui;

import javax.swing.*;
import java.awt.*;
import java.io.*;

import com.swemancipation.CodeRover.PreferencesWindow;
import com.swemancipation.GUIClasses.*;

import com.swemancipation.ProjectEditor.SettingsManager;

public class ModelBuildGUI {
  private static String m_CodeRoverHome = null;
  private static boolean m_allowCaliperInformation = false;
  private static MainFrame m_mainFrame = null;
  private static String m_service = null;

  public static SettingsManager m_preferences = new SettingsManager();

  // Construct the application
  public ModelBuildGUI(String[] args) {
    String projectName = "";
    String projectFile = "";
    parseCommandLine(args);
    for(int i = 0; i < args.length; i++) {
      if(args[i].equals("-projectName")) {
        projectName = args[i + 1];
        i++;
      } else if(args[i].equals("-projectFile")) {
        projectFile = args[i + 1];
        i++;
      }
    }
    m_mainFrame = new MainFrame(m_service);
    m_mainFrame.setProject(projectName, projectFile);

    // Center the window
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    Dimension frameSize = m_mainFrame.getSize();
    if (frameSize.height > screenSize.height) {
      frameSize.height = screenSize.height;
    }
    if (frameSize.width > screenSize.width) {
      frameSize.width = screenSize.width;
    }
    m_mainFrame.setLocation((screenSize.width - frameSize.width) / 2, (screenSize.height - frameSize.height) / 2);
    m_mainFrame.setVisible(true);
  } //constructor

  private static void parseCommandLine(String[] args) {
    m_service     = "";
    for(int i = 0; i < args.length; i++) {
      if(args[i].equals("-debug")) {
        DebugLog.enableDebug(true);
      } else if(args[i].equals("-service")) {
        m_service = args[i + 1];
        i++;
      } else if(args[i].equals("-home")) {
        m_CodeRoverHome = args[i + 1];
        i++;
      } else if(args[i].equals("-lmode")) {
        String licenseCodeStr = args[i + 1];
        int licenseCode = 0;
        try {
          licenseCode = Integer.parseInt(licenseCodeStr);
        } catch(Exception e) {
        }
        m_allowCaliperInformation = (licenseCode&020)!=0;
        i++;
      }
    }
  }

  public static Frame getFrame() {
    return m_mainFrame;
  }

  static String getCodeRoverHome() {
    return m_CodeRoverHome;
  }

  public static boolean isAllowedCaliperInfo() {
    return m_allowCaliperInformation;
  }

  public static boolean switchToProjectEditor() {
    try {
      com.swemancipation.ProjectEditor.ProjectEditorApp.main(new String[]{"-service",m_service});
    } catch(Exception e) {
      JOptionPane.showMessageDialog(m_mainFrame,"Can not open Project Editor window.");
      return false;
    }
    return true;
  }

  // Main method
  public static void main(String[] args) {
    PreferencesWindow.load();
    GUIManager.setLookAndFeel(PreferencesWindow.getLookAndFeelIdx());
    if(m_preferences.getUIMode().equals("Simple")) {
      new ModelBuildGUI(args);
    } else {
      parseCommandLine(args);
      if(!switchToProjectEditor())
        new ModelBuildGUI(args);
    }
  } //main
} //class ModelBuildGUI
