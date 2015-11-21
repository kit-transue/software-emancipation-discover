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

import java.awt.*;
import java.beans.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.net.URL;

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.border.*;

import com.swemancipation.CodeRover.Utilities.*;

import com.swemancipation.CodeRover.Frame1_AboutBox;
import com.swemancipation.CodeRover.LicenceConfiguration;
import com.swemancipation.CodeRover.CodeRoverHelpBroker;
import com.swemancipation.ProjectEditor.ImportExport;

import com.swemancipation.GUIClasses.*;

import com.swemancipation.util.HelpBundle;
import com.swemancipation.util.MessageBundle;

import javax.help.HelpSet;
import javax.help.*;


public class MainFrame extends JFrame implements ServiceMessageHandler, ActionListener,
						 SettingsChangeListener {
  private RecentUsedFiles m_usedFiles = new RecentUsedFiles();
  private boolean m_detailsOn = false;
  private Dimension showMessageDimension = new Dimension(465, 350);
  private Dimension noMessageDimension = new Dimension(365, 55);
  private Vector m_messageProcessorListener = new Vector();
  private String m_dspFile;
  private String m_modelDir;
  private String m_taskFile;
  private boolean iconifiedFlag = false;
  public  HelpSet hs;
  private CodeRoverHelpBroker hb;
  private int jbInit = 10;

  JMenuBar menuBar = new JMenuBar();
  JMenu menuView =new JMenu(MessageBundle.getBundle().getString("IDR_MENU_VIEW"));
  JMenuItem menuSwitch = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU__ITEM_SWITCH"));
  JMenu menuFile = new JMenu(MessageBundle.getBundle().getString("IDR_MENU_FILE"));
  JMenuItem menuFileSave = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU_ITEM_SAVE"));
  JMenu menuRun =new JMenu(MessageBundle.getBundle().getString("IDR_MENU_RUN"));
  JMenuItem menuFileExit = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU_ITEM_EXIT"));
  JMenuItem menuBuild = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU_ITEM_BUILD"));
  JMenuItem menuBrowse = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU_ITEM_BROWSE"));
  JMenu menuHelp = new JMenu(MessageBundle.getBundle().getString("IDR_MENU_HELP"));
  JMenuItem codeRoverHelp =new JMenuItem (MessageBundle.getBundle().getString("IDR_MENU_HELP_ITEM_SOURCE_ANALYSIS"));
  JMenuItem helpOnItem = new JMenuItem (MessageBundle.getBundle().getString("IDR_MENU_HELP_ITEM_CONTEXT_HELP"));
  JMenuItem menuHelpAbout = new JMenuItem(MessageBundle.getBundle().getString("IDR_MENU_HELP_ITEM_ABOUT"));
  BorderLayout progressBorderLayout = new BorderLayout();
  JMessageTreePanel TreePanel;

  JButton lookupProjectButton = new JButton(MessageBundle.getBundle().getString("IDR_LBL_DOTS"));
  JLabel projectFileLabel = new JLabel(MessageBundle.getBundle().getString("IDR_LBL_PROJECT_FILE"));
  JComboBox projectFileField = new JComboBox();
  JPanel DSPPanel = new JPanel();
  JPanel CaliperPanel = new JPanel();
  JLabel modelDirectoryLabel = new JLabel(MessageBundle.getBundle().getString("IDR_LBL_MODEL_DIRECTORY"));
  JTextField modelDirectoryField = new JTextField();
  JButton lookupModelDirButton = new JButton(MessageBundle.getBundle().getString("IDR_LBL_DOTS"));

  JPanel startButtonsPanel = new JPanel();
  JPanel startUPPanel = new JPanel();
  GridBagLayout startButtonsGridBagLayout = new GridBagLayout();
  JStandardButton exitButton = new JStandardButton(MessageBundle.getBundle().getString("IDR_MSG_EXIT"));
  JStandardButton updateAndRunButton = new JStandardButton(MessageBundle.getBundle().getString("IDR_MSG_BUILD"));
  JStandardButton runButton = new JStandardButton(MessageBundle.getBundle().getString("IDR_MSG_BROWSE"));
  JLabel errorsIndicator = new JLabel();
  JPanel progressButtonsPanel = new JPanel();
  JPanel progressPane = new JPanel();
  JStandardButton cancelWaitingButton = new JStandardButton(MessageBundle.getBundle().getString("IDR_BTN_CANCEL"));
  JButton cancelButton = new JButton(MessageBundle.getBundle().getString("IDR_BTN_CANCEL"));
  JButton detailsButton = new JButton(MessageBundle.getBundle().getString("IDR_MSG_SHOW_DETAILS"));
  GridLayout progressButtonsGridLayout = new GridLayout(2,1);
  JPanel progressBarPanel = new JPanel();
  JProgressBar	ProgressBar = new JProgressBar();

  JLabel taskLabel = new JLabel(MessageBundle.getBundle().getString("IDR_MSG_TASK"));
  JComboBox taskFileField = new JComboBox();
  JButton lookupTaskButton = new JButton(MessageBundle.getBundle().getString("IDR_LBL_DOTS"));
  JCheckBox includeQACheckBox = new JCheckBox(MessageBundle.getBundle().getString("IDR_MSG_INCLUDE_QA_INFORMATION"));
  ComponentListener m_sizeListener;
  JPanel mbInfoPanel = new JPanel();
  JSettingsPanel mbSettingsPanel = new JSettingsPanel();
  JTabbedPane mbGUIPanel = new JTabbedPane();

  LicenceConfiguration licence = new LicenceConfiguration();

  public static boolean allowJavaQueries() {
    return true;
  }

  //Construct the frame
  public MainFrame(String service) {
    int jsjs = jbInit;
    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    TreePanel = new JMessageTreePanel(service);
    TreePanel.addServiceMessageHandler(this);
    try {
      jbInit();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
    mbSettingsPanel.setExpandOnErrors(ModelBuildGUI.m_preferences.expandWhenError());
    mbSettingsPanel.setSilentModelBuild(ModelBuildGUI.m_preferences.isBuildSilent());
    mbSettingsPanel.setKeepLog(ModelBuildGUI.m_preferences.isKeepLog());
    mbSettingsPanel.setKillTime(ModelBuildGUI.m_preferences.getKillTimeout());

    TreePanel.setExpandOnError(mbSettingsPanel.expandOnErrors());
    TreePanel.setSilenceMode(mbSettingsPanel.silentModelBuild());
    detailsButton.setEnabled(!TreePanel.keepSilence());
  }

  //Component initialization
  private void jbInit() throws Exception  {
    mbSettingsPanel.addSettingsChangeListener(this);

    this.setTitle(MessageBundle.getBundle().getString("IDR_MSG_STARTUP_TITLE"));
    ImageIcon frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerricon.gif");
    if(frameIcon!=null)
      setIconImage(frameIcon.getImage());
    setResizable(true);
    mbInfoPanel.setLayout(new BorderLayout());
    this.getContentPane().setLayout(new BorderLayout());
    if (hs == null) {
	createHelpSet();
	if(hs!=null)  hb = new CodeRoverHelpBroker(hs,getIconImage());
    }
    if(hb!=null) {
	 hb.enableHelpKey(this.getRootPane(), HelpBundle.getBundle().getString("IDR_HELP_MODEL_BUILD"), null);
    }
    this.getContentPane().add(mbGUIPanel,BorderLayout.CENTER);


    menuBuild.addActionListener(new ActionListener(){
      public void actionPerformed (ActionEvent e){
	  updateAndRunButton_actionPerformed(e);
      }
    });
    menuRun.add(menuBuild);
    menuBrowse.addActionListener(new ActionListener(){
      public void actionPerformed (ActionEvent e){
	  runButton_actionPerformed(e);
      }
    });
    menuRun.add(menuBrowse);
    menuRun.addSeparator();
    menuFileExit.addActionListener(new ActionListener (){
       public void actionPerformed(ActionEvent e){
	   fileExit_actionPerformed(e);
       }
    });
    menuFileSave.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
	fileSave_actionPerformed(e);
      }
    });

    if (hb!= null){
       codeRoverHelp.addActionListener(new ActionListener (){
	  public void actionPerformed (ActionEvent e){
	     codeRoverHelp_actionPerformed (e);
	  }
       });
       menuHelp.add(codeRoverHelp);
       helpOnItem.addActionListener(new CSH.DisplayHelpAfterTracking(hb));
       menuHelp.add(helpOnItem);
       menuHelp.addSeparator();
    }
    menuHelpAbout.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
	helpAbout_actionPerformed(e);
      }
    });
    menuHelp.add(menuHelpAbout);
    menuFileSave.addActionListener(new ActionListener()  {
      public void actionPerformed(ActionEvent e) {
	fileSave_actionPerformed(e);
      }
    });
    menuFile.add(menuFileSave);
    menuFile.addSeparator();
    menuFile.add(menuFileExit);
    // CSH.setHelpIDString(menuRun, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    menuBar.add(menuFile);

    menuSwitch.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	fileExit_actionPerformed(new ActionEvent(this,0,"closeWindow"));
	if(ModelBuildGUI.switchToProjectEditor())
	  MainFrame.this.dispose();
	else
	  MainFrame.this.setVisible(true);
      }
    });
    menuView.add(menuSwitch);

    menuBar.add(menuRun);

    menuBar.add(menuView);

    // CSH.setHelpIDString(menuHelp, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    menuBar.add(menuHelp);
    // CSH.setHelpIDString(menuBar, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    setJMenuBar(menuBar);
    CSH.setHelpIDString(mbInfoPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_MODEL_BUILD"));
    mbGUIPanel.add(MessageBundle.getBundle().getString("IDR_MSG_PANEL_INFO_TITLE"),mbInfoPanel);
    CSH.setHelpIDString(mbSettingsPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_SETTINGS"));
    mbGUIPanel.add(MessageBundle.getBundle().getString("IDR_MSG_PANEL_SETTINGS_TITLE"),mbSettingsPanel);
    createStartWindow(mbInfoPanel);
    Dimension infoSize = mbInfoPanel.getPreferredSize();
    Dimension settingsSize = mbSettingsPanel.getPreferredSize();
    Dimension preferredSize = new Dimension(infoSize.width>settingsSize.width?infoSize.width:settingsSize.width,
					    infoSize.height>settingsSize.height?infoSize.height:settingsSize.height);
    mbInfoPanel.setPreferredSize(preferredSize);
    mbSettingsPanel.setPreferredSize(preferredSize);
    pack();
  }

  public void createStartWindow(JPanel insertTo) {
    JPanel  contentPane = insertTo;
    lookupProjectButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	lookupProjectButton_actionPerformed(e);
      }
    });
    lookupModelDirButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	lookupModelDirButton_actionPerformed(e);
      }
    });

    startButtonsPanel.setLayout(startButtonsGridBagLayout);
    exitButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	fileExit_actionPerformed(e);
      }
    });
    updateAndRunButton.setEnabled(false);
    updateAndRunButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	updateAndRunButton_actionPerformed(e);
      }
    });
    runButton.setEnabled(false);
    runButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	runButton_actionPerformed(e);
      }
    });
    int desiredHeight = 21;
    int comboHeight   = desiredHeight + GUIConstants.getComboBoxHeightDelta();
    int buttonHeight  = desiredHeight + GUIConstants.getButtonHeightDelta();

    if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
      lookupTaskButton.setEnabled(includeQACheckBox.isSelected());
      lookupTaskButton.addActionListener(new ActionListener() {
	public void actionPerformed(ActionEvent e) {
	  lookupTaskDirButton_actionPerformed(e);
	}
      });
      taskFileField.setEnabled(includeQACheckBox.isSelected());
      taskLabel.setEnabled(includeQACheckBox.isSelected());
      includeQACheckBox.addActionListener(new ActionListener() {
	public void actionPerformed(ActionEvent e) {
	  lookupTaskButton.setEnabled(includeQACheckBox.isSelected());
	  taskFileField.setEnabled(includeQACheckBox.isSelected());
	  taskFileField.setEditable(taskFileField.isEnabled());
	  taskFileField.setSelectedIndex(m_usedFiles.getActiveTask());
	  taskLabel.setEnabled(includeQACheckBox.isSelected());
	  m_usedFiles.setCaliper(includeQACheckBox.isSelected());
	}
      });
      includeQACheckBox.setMinimumSize(new Dimension(200, GUIConstants.getButtonHeight()));
      includeQACheckBox.setMaximumSize(new Dimension(GUIConstants.INFINITY, GUIConstants.getButtonHeight()));
      includeQACheckBox.setPreferredSize(new Dimension(200, GUIConstants.getButtonHeight()));
      taskLabel.setMinimumSize(new Dimension(40, comboHeight));
      taskLabel.setMaximumSize(new Dimension(40, comboHeight));
      taskLabel.setPreferredSize(new Dimension(40, comboHeight));
      taskFileField.setMinimumSize(new Dimension(200, comboHeight));
      taskFileField.setMaximumSize(new Dimension(GUIConstants.INFINITY, comboHeight));
      taskFileField.setPreferredSize(new Dimension(200, comboHeight));
      lookupTaskButton.setMinimumSize(new Dimension(40, buttonHeight));
      lookupTaskButton.setMaximumSize(new Dimension(GUIConstants.INFINITY, buttonHeight));
      lookupTaskButton.setPreferredSize(new Dimension(40, buttonHeight));
    } //if Caliper

    projectFileLabel.setMinimumSize(new Dimension(101, comboHeight));
    projectFileLabel.setMaximumSize(new Dimension(101, comboHeight));
    projectFileLabel.setPreferredSize(new Dimension(101, comboHeight));
    projectFileField.setMinimumSize(new Dimension(100, comboHeight));
    projectFileField.setMaximumSize(new Dimension(GUIConstants.INFINITY, comboHeight));
    projectFileField.setPreferredSize(new Dimension(100, comboHeight));
    lookupProjectButton.setMinimumSize(new Dimension(40, buttonHeight));
    lookupProjectButton.setMaximumSize(new Dimension(GUIConstants.INFINITY, buttonHeight));
    lookupProjectButton.setPreferredSize(new Dimension(40, buttonHeight));

    modelDirectoryLabel.setMinimumSize(new Dimension(101, comboHeight));
    modelDirectoryLabel.setMaximumSize(new Dimension(101, comboHeight));
    modelDirectoryLabel.setPreferredSize(new Dimension(101, comboHeight));
    modelDirectoryField.setMinimumSize(new Dimension(100, comboHeight));
    modelDirectoryField.setMaximumSize(new Dimension(GUIConstants.INFINITY, comboHeight));
    modelDirectoryField.setPreferredSize(new Dimension(100, comboHeight));
    lookupModelDirButton.setMinimumSize(new Dimension(40, buttonHeight));
    lookupModelDirButton.setMaximumSize(new Dimension(GUIConstants.INFINITY, buttonHeight));
    lookupModelDirButton.setPreferredSize(new Dimension(40, buttonHeight));

    DSPPanel.setLayout(new GridBagLayout());
    DSPPanel.add(projectFileLabel,
		 new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0,
					GridBagConstraints.WEST, GridBagConstraints.NONE,
					new Insets(3, 4, 3, 0), 0, 0));
    DSPPanel.add(projectFileField,
		 new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0,
					GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL,
					new Insets(3, 0, 3, 4), 0, 0));
    DSPPanel.add(lookupProjectButton,
		 new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0,
					GridBagConstraints.CENTER, GridBagConstraints.NONE,
					new Insets(0, 0, 0, 2), 0, 0));

    DSPPanel.add(modelDirectoryLabel,
		 new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0,
					GridBagConstraints.WEST, GridBagConstraints.NONE,
					new Insets(3, 4, 3, 0), 0, 0));
    DSPPanel.add(modelDirectoryField,
		 new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0,
					GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL,
					new Insets(3, 0, 3, 4), 0, 0));
    DSPPanel.add(lookupModelDirButton,
		 new GridBagConstraints(2, 1, 1, 1, 0.0, 0.0,
					GridBagConstraints.CENTER, GridBagConstraints.NONE,
					new Insets(0, 0, 0, 2), 0, 0));

    DSPPanel.setBorder(new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(134, 134, 134)),
                                           MessageBundle.getBundle().getString("IDR_MSG_BORDER_MODEL_TITLE")));
    startButtonsPanel.add(updateAndRunButton,
			  new GridBagConstraints(2, 0, 1, 1, 0.0, 0.0,
						 GridBagConstraints.CENTER,
						 GridBagConstraints.NONE,
						 new Insets(0, 0, 0, 0), 0, 0));
    startButtonsPanel.add(new JPanel(),
			  new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0,
						 GridBagConstraints.CENTER,
						 GridBagConstraints.NONE,
						 new Insets(0, 0, 0, 5), 0, 0));
    startButtonsPanel.add(runButton,
			  new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0,
						 GridBagConstraints.CENTER,
						 GridBagConstraints.NONE,
						 new Insets(0, 0, 0, 0), 0, 0));
    startButtonsPanel.add(new JPanel(),
			  new GridBagConstraints(3, 0, 1, 1, 0.0, 0.0,
						 GridBagConstraints.CENTER,
						 GridBagConstraints.NONE,
						 new Insets(0, 0, 0, 5), 0, 0));
    startButtonsPanel.add(exitButton,
			  new GridBagConstraints(4, 0, 1, 1, 0.0, 0.0,
						 GridBagConstraints.CENTER,
						 GridBagConstraints.NONE,
						 new Insets(0, 0, 0, 0), 0, 0));
    CSH.setHelpIDString(DSPPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_MODEL_BUILD"));
    // CSH.setHelpIDString(startButtonsPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    CaliperPanel.setBorder(new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(134, 134, 134)),
                                             MessageBundle.getBundle().getString("IDR_MSG_BORDER_CALIPEL_TITLE")));
    CaliperPanel.setLayout(new GridBagLayout());
    CSH.setHelpIDString(CaliperPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_CALIPER_SETTINGS"));
    contentPane.setPreferredSize(new Dimension(365, 143));
    if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
      CaliperPanel.add(includeQACheckBox,
		       new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.NONE,
					      new Insets(0, 0, 0, 0), 0, 0));
      CaliperPanel.add(taskLabel,
		       new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0,
					      GridBagConstraints.WEST,
					      GridBagConstraints.NONE,
					      new Insets(3, 4, 3, 0), 0, 0));
      CaliperPanel.add(taskFileField,
		       new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0,
					      GridBagConstraints.CENTER,
					      GridBagConstraints.HORIZONTAL,
					      new Insets(3, 0, 3, 4), 0, 0));
      CaliperPanel.add(lookupTaskButton,
		       new GridBagConstraints(2, 1, 1, 1, 0.0, 0.0,
					      GridBagConstraints.EAST,
					      GridBagConstraints.NONE,
					      new Insets(0, 0, 0, 2), 0, 0));
      contentPane.add(CaliperPanel, BorderLayout.CENTER);
      contentPane.add(DSPPanel, BorderLayout.NORTH);
      contentPane.setPreferredSize(new Dimension(365, 210));
    } else
      contentPane.add(DSPPanel, BorderLayout.CENTER);
    contentPane.add(startButtonsPanel, BorderLayout.SOUTH);
    projectFileField.setRenderer(new JPathListRenderer());
    projectFileField.addItem("");
    projectFileField.setEditable(true);
    for(int i=0;i < m_usedFiles.projectsCount();i++) {
      projectFileField.addItem(m_usedFiles.getRUProject(i));
    }
    projectFileField.addActionListener(this);
    projectFileField.setMaximumRowCount(3);

    if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
      taskFileField.setRenderer(new JPathListRenderer());
      taskFileField.addItem("");
      taskFileField.setEditable(taskFileField.isEnabled());
      if(m_usedFiles.tasksCount()>0) {
	for(int i=0;i < m_usedFiles.tasksCount();i++) {
	  taskFileField.addItem(m_usedFiles.getRUTask(i));
	}
      } else {
	String defaultTask = ModelBuildGUI.getCodeRoverHome();
	if(defaultTask!=null) {
	  if(!defaultTask.endsWith("\\") && !defaultTask.endsWith("/"))
	    defaultTask += "/";
	  defaultTask += "lib/policy/default.policy.xml";
	  taskFileField.addItem(defaultTask);
	  m_usedFiles.saveActiveTask(1);
	}
      }
      taskFileField.setMaximumRowCount(2);
      if(m_usedFiles.isCaliperChecked())
	includeQACheckBox.doClick();
    }
    m_sizeListener = new ComponentAdapter() {
      Dimension m_preferredSize = null;
      public void componentResized(ComponentEvent e) {
	// store preferred size first time
	if(m_preferredSize==null) {
	  m_preferredSize = getPreferredSize();
	  return;
	}
	// check for width and height...
	Dimension size = getSize();
	boolean needUpdate = false;
	if(size.height != m_preferredSize.height) {
	  size.height = m_preferredSize.height;
	  needUpdate = true;  Dimension m_preferredSize = null;

	}
	if(size.width<m_preferredSize.width) {
	  size.width = m_preferredSize.width;
	  needUpdate = true;
	}
	// ...and if necesary - change the size
	if(needUpdate==true)
	  setSize(size);
      }
    };
    menuFile.remove(menuFileExit);
    menuFile.setVisible(false);
    menuRun.add(menuFileExit);
    this.addComponentListener(m_sizeListener);
    pack();
  }

  public void createProgressWindow(JPanel insertTo) {
    this.removeComponentListener(m_sizeListener);
    this.setTitle(MessageBundle.getBundle().getString("IDR_MSG_PROGRESS_WINDOW_TITLE"));
    if(hb!=null) {
	 hb.enableHelpKey(this.getRootPane(), HelpBundle.getBundle().getString("IDR_HELP_MODEL_BUILD"), null);
    }
    ImageIcon frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerricon.gif");
    if(frameIcon!=null)
      setIconImage(frameIcon.getImage());

    JPanel  progressContentPane = insertTo;

    errorsIndicator.setIcon(ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerrors.gif"));
    // CSH.setHelpIDString(ProgressBar, HelpBundle.getBundle().getString("IDR_HELP_MB_APPEND_SHOOTING"));
    ProgressBar.setStringPainted(true);
    progressBarPanel.setLayout(new GridBagLayout());
    progressButtonsPanel.setLayout(progressButtonsGridLayout);
    // CSH.setHelpIDString(cancelButton, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    progressButtonsPanel.add(cancelButton, null);
    // CSH.setHelpIDString(detailsButton, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    progressButtonsPanel.add(detailsButton, null);
    detailsButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	JPanel contentPane = mbInfoPanel;
	if(m_detailsOn == true) {
	  contentPane.remove(TreePanel);
	  showMessageDimension = contentPane.getSize();
	  contentPane.setPreferredSize(noMessageDimension);
	  detailsButton.setText(MessageBundle.getBundle().getString("IDR_MSG_SHOW_DETAILS"));
	  m_detailsOn = false;
	} else {
	  // CSH.setHelpIDString(TreePanel, HelpBundle.getBundle().getString("IDR_HELP_MB_APPEND_MESSAGE"));
	  contentPane.add(TreePanel, BorderLayout.CENTER);
	  noMessageDimension = contentPane.getSize();
	  contentPane.setPreferredSize(showMessageDimension);
	  detailsButton.setText(MessageBundle.getBundle().getString("IDR_MSG_HIDE_DETAILS"));
	  m_detailsOn = true;
	}
	pack();
	setResizable(m_detailsOn);
      }
    });
    // CSH.setHelpIDString(menuBar, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    // CSH.setHelpIDString(menuFile, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    // CSH.setHelpIDString(menuHelp, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    progressBarPanel.add(ProgressBar, new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0
	    ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 5, 0, 5), 0, 0));
    progressPane.setLayout(new BorderLayout());
    progressPane.add(errorsIndicator, BorderLayout.WEST);
    progressPane.add(progressBarPanel, BorderLayout.CENTER);
    progressPane.add(progressButtonsPanel, BorderLayout.EAST);

    progressContentPane.setLayout(progressBorderLayout);

    cancelButton.setToolTipText(MessageBundle.getBundle().getString("IDR_MSG_CANCEL_BUILD"));
    cancelButton.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
	cancelButton_actionPerformed(e);
      }
    });
    menuView.setVisible(false);
    menuRun.remove(menuFileExit);
    menuRun.setVisible(false);
    menuFile.add(menuFileExit);
    menuFile.setVisible(true);

    progressContentPane.add(progressPane, BorderLayout.NORTH);
    progressContentPane.setPreferredSize(new Dimension(365, 71));
    cancelButton.setEnabled(false);
    menuFileSave.setEnabled(true);
  }

  public void actionPerformed(ActionEvent event) {
    if(event.getSource() instanceof JComboBox) {
      if(event.getActionCommand().equals("comboBoxChanged")==true &&
	projectFileField.getSelectedItem()!=null) {
	String projFile = projectFileField.getSelectedItem().toString();
	int pathIndex = projectFileField.getSelectedIndex()-1;
	String path="";
	if(pathIndex>=0 &&  m_usedFiles!=null)
	  path = m_usedFiles.getRUPath(pathIndex);
	if((path == null) || (path.length()==0)) {
	  if(projectFileField.getSelectedItem()==null)	return;
	  File tmpF = new File(projFile);
	  path = tmpF.getAbsolutePath();
	  updateModelDirectory(path);
	} else {
	  modelDirectoryField.setText(path);
	}
	if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
	  if(projFile.trim().endsWith(".jpr") && !allowJavaQueries()) {
	    if(includeQACheckBox.isSelected())
	      includeQACheckBox.doClick();
	    includeQACheckBox.setEnabled(false);
	  } else
	    includeQACheckBox.setEnabled(true);
	}
	if(modelDirectoryField.isEnabled()==true) {
	  updateAndRunButton.setEnabled(true);
	  runButton.setEnabled(true);
	}
      }
    }
  }

  private boolean saveLog(String outFileName) {
    File    outFile;
    if(outFileName!=null) {
      if(outFileName.toUpperCase().endsWith(".XML")==false)
	outFile = new File(outFileName+".xml");
      else
	outFile = new File(outFileName);
      try{
	FileOutputStream  theLocalStream = new FileOutputStream(outFile);
	new TreeToXMLWriter(theLocalStream,TreePanel.getMessagesRoot());
	theLocalStream.close();
      } catch(Exception exception) {
	JOptionPane.showMessageDialog(this, MessageBundle.getBundle().getString("IDR_MSG_CAN_NOT_OPEN_XML_FILE") + exception);
	return false;
      }
      return true;
    }
    return false;
  }

  //File | Save action performed
  public void fileSave_actionPerformed(ActionEvent e) {
    JFileChooser fc = new JFileChooser();

    fc.addChoosableFileFilter(new XMLFileFilter());
    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
    fc.setDialogType(JFileChooser.SAVE_DIALOG);
    int retVal = fc.showSaveDialog(this);
    if(retVal == JFileChooser.APPROVE_OPTION) {
      String  outFileName = fc.getSelectedFile().toString();
      saveLog(outFileName);
    }
    this.repaint();
  }

  //File | Exit action performed
  public void fileExit_actionPerformed(ActionEvent e) {
    m_usedFiles.save();
    ModelBuildGUI.m_preferences.save();
    String actionCommand = e.getActionCommand();
    if(actionCommand.equals(menuFileExit.getActionCommand()))
      System.exit(0);
  }

  //Help | About action performed
  public void helpAbout_actionPerformed(ActionEvent e) {

  // Get name
    String productName; // the current licensed product
    StringBuffer tmpName = new StringBuffer("SET ");

    if(licence.isCI() && licence.isEI())
      tmpName.append("Code/Engineer");
    else
      if(licence.isCI())
	tmpName.append("Code");
      else
	tmpName.append("Engineer");
    tmpName.append(" Integrity");
    if(licence.isEnterprise())
	tmpName.append(" Enterprise");
    else
	tmpName.append(" Standard");
    tmpName.append(" Edition");

    if(licence.isEvaluation())
      tmpName.append("(Evaluation copy)");

    productName = tmpName.toString();

    Frame1_AboutBox dlg = new Frame1_AboutBox(this, licence.getVersionNumber(), licence.getBuildNumber(), licence.getSerialNumber(), productName);
    Dimension dlgSize = dlg.getPreferredSize();
    Dimension frmSize = getSize();
    Point loc = getLocation();
    dlg.setLocation((frmSize.width - dlgSize.width) / 2 + loc.x,
		    (frmSize.height - dlgSize.height) / 2 + loc.y);
    dlg.setModal(true);
    dlg.show();
    this.repaint();
  }

  //Overridden so we can exit when window is closed
  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    switch(e.getID()) {
      case WindowEvent.WINDOW_CLOSING:
	fileExit_actionPerformed(new ActionEvent(this,0,menuFileExit.getActionCommand()));
	break;
      case WindowEvent.WINDOW_ACTIVATED:
	this.repaint();
	break;
      case WindowEvent.WINDOW_ICONIFIED:
	iconifiedFlag = true;
	break;
      case WindowEvent.WINDOW_DEICONIFIED:
	iconifiedFlag = false;
	removePercentFromTitle();
	break;
    } /*switch*/
  } /*processWindowEvent*/

  void setProject(String projectName, String projectFileName) {
    if((projectFileName != null) && (projectFileName.length() != 0)) {
      String modelName;
      if((m_usedFiles!=null) && ((modelName=m_usedFiles.getRUPath(projectFileName))!=null)) {
	projectFileField.setSelectedItem(projectFileName);
      } else {
	projectFileField.removeItemAt(0);
	projectFileField.insertItemAt(projectFileName,0);
	projectFileField.setSelectedIndex(0);
	if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
	  if(projectFileName.trim().endsWith(".jpr") && !allowJavaQueries()) {
	    if(includeQACheckBox.isSelected())
	      includeQACheckBox.doClick();
	    includeQACheckBox.setEnabled(false);
	  }
	}
	updateModelDirectory(projectFileName);
      }
      updateAndRunButton.setEnabled(true);
      runButton.setEnabled(true);
    } else {
      if(projectFileField.getItemCount() > m_usedFiles.getActiveProject()) {
	projectFileField.setSelectedIndex(m_usedFiles.getActiveProject());
	updateAndRunButton.setEnabled(true);
	runButton.setEnabled(true);
      }
    }
    if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
      if(taskFileField.getItemCount() > m_usedFiles.getActiveTask())
	taskFileField.setSelectedIndex(m_usedFiles.getActiveTask());
    }
  }

  void updateModelDirectory(String projectFileName) {
    java.io.File projectFile = new java.io.File(projectFileName);
    String parent	   = projectFile.getParent();
    if(parent != null) {
      parent += projectFile.separatorChar + "Model";
      modelDirectoryField.setText(parent);
    }
  }

  void lookupProjectButton_actionPerformed(ActionEvent e) {
    DSPFileChooser fc = new DSPFileChooser();
    fc.setDialogTitle(MessageBundle.getBundle().getString("IDR_MSG_OPEN_PROJECT_FILE"));
    // get directory from current project file
    Object selectedItem = projectFileField.getSelectedItem();
    if(selectedItem!=null) {
      String projectFileName   = selectedItem.toString();
      if(projectFileName.length() != 0) {
	java.io.File projectFile = new java.io.File(projectFileName);
	String parent = projectFile.getParent();
	if(parent != null)
	  fc.setCurrentDirectory(new java.io.File(parent));
      }
    } else {
      projectFileField.insertItemAt("",0);
    }

    fc.addChoosableFileFilter(new JPRFileFilter());
    fc.addChoosableFileFilter(new DSPFileFilter());
    fc.addChoosableFileFilter(new DSWFileFilter());
    fc.addChoosableFileFilter(new DPFFileFilter());
    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
    fc.setDialogType(JFileChooser.OPEN_DIALOG);
    int retVal = fc.showOpenDialog(this);
    if(retVal == JFileChooser.APPROVE_OPTION) {
      projectFileField.removeItemAt(0);
      projectFileField.insertItemAt(fc.getSelectedFile().getAbsolutePath(),0);
      projectFileField.setSelectedIndex(0);
      if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
	if(fc.getSelectedFile().getName().trim().endsWith(".jpr") && !allowJavaQueries()) {
	  if(includeQACheckBox.isSelected())
	    includeQACheckBox.doClick();
	  includeQACheckBox.setEnabled(false);
	}
      }
      updateModelDirectory(fc.getSelectedFile().getAbsolutePath());
      updateAndRunButton.setEnabled(true);
      runButton.setEnabled(true);
    }
  }

  void lookupModelDirButton_actionPerformed(ActionEvent e) {
    JFileChooser fc = new JFileChooser();
    fc.setDialogTitle(MessageBundle.getBundle().getString("IDR_MSG_SELECT_MODEL_DIRECTORY"));
    String modelDir = modelDirectoryField.getText();
    if(modelDir.length() != 0)
      fc.setCurrentDirectory(new java.io.File(modelDir));
    fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    fc.setDialogType(JFileChooser.OPEN_DIALOG);
    int retVal = fc.showOpenDialog(this);
    if(retVal == JFileChooser.APPROVE_OPTION) {
      modelDirectoryField.setText(fc.getSelectedFile().getAbsolutePath());
    }
  }

  void lookupTaskDirButton_actionPerformed(ActionEvent e) {
    DSPFileChooser fc = new DSPFileChooser();

    StringBuffer tmpBuff = new StringBuffer(ModelBuildGUI.getCodeRoverHome());
    tmpBuff.append(File.separator);
    tmpBuff.append("lib");
    tmpBuff.append(File.separator);
    tmpBuff.append("policy");

    fc.addPropertyChangeListener(DSPFileChooser.DIRECTORY_CHANGED_PROPERTY,new PropertyChangeListener(){
      private boolean firstTime = true;
      public void propertyChange(PropertyChangeEvent evt) {
	StringBuffer tmpBuff = new StringBuffer(ModelBuildGUI.getCodeRoverHome());
	tmpBuff.append(File.separator);
	tmpBuff.append("lib");
	tmpBuff.append(File.separator);
	tmpBuff.append("policy");
	String fullPath = tmpBuff.toString();
	String oldPath = evt.getOldValue().toString();
	String newPath = evt.getNewValue().toString();
	if(!newPath.equalsIgnoreCase(fullPath)) {
	  DSPFileChooser fc = (DSPFileChooser)evt.getSource();
	  if(!newPath.equalsIgnoreCase(oldPath))
	    JOptionPane.showMessageDialog(fc,MessageBundle.getBundle().getString("IDR_MSG_CANT_CHANGE_POLICY_DIRECTORY"));
	  fc.setCurrentDirectory(new File(fullPath));
	}
      }
    });
    fc.setCurrentDirectory(new File(tmpBuff.toString()));
    fc.setDialogTitle(MessageBundle.getBundle().getString("IDR_MSG_OPEN_POLICY_FILE"));
    // get directory from current project file
    Object selectedItem = taskFileField.getSelectedItem();
    if(selectedItem!=null) {
      String taskFileName   = selectedItem.toString();
      if(taskFileName.length() != 0) {
	File taskFile = new File(taskFileName);
	String parent = taskFile.getParent();
	if(parent != null)
	  fc.setCurrentDirectory(new File(parent));
      }
    } else {
      taskFileField.insertItemAt("",0);
    }

    fc.addChoosableFileFilter(new TaskFileFilter());
    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
    fc.setDialogType(JFileChooser.OPEN_DIALOG);
    int retVal = fc.showOpenDialog(this);
    if(retVal == JFileChooser.APPROVE_OPTION) {
//	taskFileField.removeItemAt(0);
      taskFileField.insertItemAt(fc.getSelectedFile().getAbsolutePath(),1);
      taskFileField.setSelectedIndex(1);
    }
  }

  void startBuildButton_actionPerformed(ActionEvent e) {
    m_dspFile = projectFileField.getSelectedItem().toString();
    m_modelDir = modelDirectoryField.getText();

    if(ModelBuildGUI.isAllowedCaliperInfo() && includeQACheckBox.isSelected()) {
      if(taskFileField.getSelectedItem() != null) {
	m_taskFile = taskFileField.getSelectedItem().toString();
      }
    } else
      m_taskFile = null;

    if((m_dspFile == null) || (m_dspFile.length() == 0) ||
	(m_modelDir == null) || (m_modelDir.length() == 0)) {
      JOptionPane.showMessageDialog(this,
      MessageBundle.getBundle().getString("IDR_MSG_PATH_SHOULD_NOT_BE_EMPTY"),
      MessageBundle.getBundle().getString("IDR_MSG_ERROR"),
      JOptionPane.ERROR_MESSAGE);
      return;
    }

    if(m_taskFile!=null && m_taskFile.length()!=0)
      m_usedFiles.addTaskFile(m_taskFile);
    boolean added = m_usedFiles.addProjectAndPath(m_dspFile, m_modelDir);
    projectFileField.setSelectedIndex(0);
    if(added == true) {
      projectFileField.removeItemAt(0);
      projectFileField.insertItemAt("",0);
      projectFileField.addItem(m_dspFile);
    }
    projectFileField.setSelectedItem(m_dspFile);
    m_usedFiles.saveActiveProject(projectFileField.getSelectedIndex());
    if(ModelBuildGUI.isAllowedCaliperInfo()==true)
      m_usedFiles.saveActiveTask(taskFileField.getSelectedIndex());
    lookupProjectButton.setEnabled(false);
    lookupModelDirButton.setEnabled(false);
    ProgressBar.setVisible(true);
    ProgressBar.setValue(0);
    ProgressBar.setString(null);
    cancelButton.setEnabled(true);
    projectFileField.setEnabled(false);
    modelDirectoryField.setEnabled(false);
    menuFileSave.setEnabled(false);
    fireClearMessages();

    new Thread(new Runnable() {
      public void run() {
	// if it is a DPF project - we'll convert it to a tab separated file
	File projectFile = new File(m_dspFile);
	boolean ok = false;
	if(ImportExport.isDPFProject(projectFile)) {
	  TreePanel.processMessage("<message help=\"0\" node=\"1\" parent=\"0\" severity=\"normal\">Importing " + m_dspFile + " project....</message>");
	  File outFile = new File(System.getProperty("user.home") + File.separator + "dpf.out");
	  ImportExport.exportToTabSeparatedFormat(projectFile,outFile);
	  ok = TreePanel.sendCommand("<dsp>" + outFile.getAbsolutePath() + "</dsp>");
	} else {
	  ok = TreePanel.sendCommand("<dsp>" + m_dspFile + "</dsp>");
	}

	if(ok)
	  ok = TreePanel.sendCommand("<output>" +  m_modelDir + "</output>");
	if(ok) {
	  if(m_taskFile!=null && m_taskFile.length()!=0)
	    ok = TreePanel.sendCommand("<qatask>" +  m_taskFile + "</qatask>");
	  if(ok)
	    ok = TreePanel.sendCommand("<build/>");
	}
	if(!ok) {
	  JOptionPane.showMessageDialog(MainFrame.this,
                              MessageBundle.getBundle().getString("IDR_MSG_UNABLE_COMMUNICATE_WITH_MODEL_BUILD_ENGINE"),
                              MessageBundle.getBundle().getString("IDR_MSG_ALERT"), JOptionPane.ERROR_MESSAGE);
	}
      }
    }).start();
  }

  public void addMessageProcessorListener(MessageProcessorListener listnr) {
    if(m_messageProcessorListener.contains(listnr)==false)
      m_messageProcessorListener.add(listnr);
  }

  private void fireClearMessages() {
    if(m_messageProcessorListener!=null) {
      for(int i=0;i<m_messageProcessorListener.size();i++)
	((MessageProcessorListener)m_messageProcessorListener.get(i)).clearAllMessages();
    }
  }

  public void DoneMessage() {
    cancelButton.setEnabled(false);
    lookupProjectButton.setEnabled(true);
    lookupModelDirButton.setEnabled(true);
    projectFileField.setEnabled(true);
    modelDirectoryField.setEnabled(true);
    menuFileSave.setEnabled(true);
    ProgressBar.setValue(0);
    ProgressBar.setString("");

    // saving a log if necessary....
    if(ModelBuildGUI.m_preferences.isKeepLog()) {
      StringBuffer logBuff = new StringBuffer(m_modelDir);
      char lastChar = logBuff.charAt(logBuff.length()-1);
      if(lastChar!='\\' &&  lastChar!='/')
	logBuff.append(File.separator);
      logBuff.append("lastbuildlog.xml");
      String logName = logBuff.toString();
      if(saveLog(logName))
	TreePanel.processMessage("<message help=\"0\" node=\"1\" parent=\"0\" severity=\"normal\">Log file saved as \"" + logName + "\".</message>");
    }

    // if there is no errors - run pmod server and CodeRover
    DefaultMutableTreeNode rootNode = TreePanel.getMessagesRoot();
    if(rootNode!=null) {
      MessageTreeNode messNode = (MessageTreeNode)rootNode.getUserObject();
      if(messNode.m_severity.equals("catastrophe")==false && messNode.m_severity.equals("error")==false){
	if((m_dspFile == null) || (m_dspFile.length() == 0) ||
	    (m_modelDir == null) || (m_modelDir.length() == 0)) {
	  JOptionPane.showMessageDialog(this,
                            MessageBundle.getBundle().getString("IDR_MSG_PATH_SHOULD_NOT_BE_EMPTY"),
                            MessageBundle.getBundle().getString("IDR_MSG_ERROR"),JOptionPane.ERROR_MESSAGE);
	  return;
	}
	TreePanel.sendCommand("<dsp>" + m_dspFile + "</dsp>");
	TreePanel.sendCommand("<output>" + m_modelDir + "</output>");
	double timeout = ModelBuildGUI.m_preferences.getKillTimeout();
	if(timeout>0)
	  TreePanel.sendCommand("<autoshutdown>"+Integer.toString((int)(timeout*3600))+"</autoshutdown>");
	TreePanel.sendCommand("<start_pmod_server/>");

	JPanel contentPane = mbInfoPanel;
	contentPane.remove(progressPane);
	this.setJMenuBar(new JMenuBar());
	waitForReady();
      }
    }
  }

  private void waitForReady() {
    if(m_detailsOn==true)
      detailsButton.doClick();

    JPanel  waitPane = new JPanel();
    JPanel  waitBarPanel = new JPanel();

    this.removeComponentListener(m_sizeListener);
    ProgressBar.setString("");
    ProgressBar.setStringPainted(true);
    waitBarPanel.setLayout(new GridBagLayout());
    waitBarPanel.add(ProgressBar,
		     new GridBagConstraints(0, 0, 1, 1, 1.0, 1.0,
					    GridBagConstraints.CENTER,
					    GridBagConstraints.HORIZONTAL,
					    new Insets(0, 5, 0, 5), 0, 0));

    waitPane.setLayout(new GridBagLayout());
    waitPane.add(waitBarPanel,
		 new GridBagConstraints(0, 1, 1, 1, 1.0, 0.0,
					GridBagConstraints.CENTER, GridBagConstraints.BOTH,
					new Insets(0, 0, 0, 0), 0, 0));
    waitPane.add(cancelWaitingButton,
		 new GridBagConstraints(1, 1, 1, 1, 0.0, 0.0,
					GridBagConstraints.CENTER, GridBagConstraints.NONE,
					new Insets(0, 0, 0, 0), 0, 0));

    cancelWaitingButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	fileExit_actionPerformed(new ActionEvent(this,0,"Exit"));
      }
    });

    JPanel contentPane = mbInfoPanel;
    contentPane.add(waitPane);
    contentPane.setPreferredSize(new Dimension(365, 55));
    this.setTitle(MessageBundle.getBundle().getString("IDR_MSG_STARTING_TITLE"));
    ImageIcon frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerricon.gif");
    if(frameIcon!=null)
      setIconImage(frameIcon.getImage());
    this.setResizable(false);
    pack();
    Thread waitThread = new Thread() {
      private int currentValue = 0;
      private boolean forward = true;
      private javax.swing.Timer m_timer;

      class UpdateProgress implements Runnable {
	private int m_value;
	public UpdateProgress(int value) {
	  m_value = value;
	}
	public void run() {
	  ProgressBar.setForeground(new Color(0,(150-m_value)*8/5,m_value*8/5));
	  ProgressBar.setValue(m_value);
	}
      }

      public void run() {
	m_timer = new javax.swing.Timer(35, new ActionListener() {
	  public void actionPerformed(ActionEvent e) {
	    if(forward) {
	      currentValue++;
	      if(currentValue>=100)
		forward = false;
	    } else {
	      currentValue--;
	      if(currentValue<=0)
		forward = true;
	    }
	    SwingUtilities.invokeLater(new UpdateProgress(currentValue));
	  }
	});
	m_timer.start();
      }
    };
    waitThread.start();
  }

  public void ReadyMessage() {
    fileExit_actionPerformed(new ActionEvent(this,0,"Exit"));
  }

  public void FailedMessage(String reason) {
    JOptionPane.showMessageDialog(this,
                  MessageBundle.getBundle().getString("IDR_MSG_START_FAILED")+reason,
                  MessageBundle.getBundle().getString("IDR_MSG_ERROR"),
                  JOptionPane.ERROR_MESSAGE);
    fileExit_actionPerformed(new ActionEvent(this,0,MessageBundle.getBundle().getString("IDR_MENU_ITEM_EXIT")));
  }

  //---------- SettingsChangeListener interface implementation -------------
  public void setUIMode(String mode) {
    ModelBuildGUI.m_preferences.setUIMode(mode);
  }
  public void setKeepLog(boolean state) {
    ModelBuildGUI.m_preferences.setKeepLog(state);
  }
  public void setKillTime(double hours) {
    ModelBuildGUI.m_preferences.setKillTimeout(hours);
  }
  public void setSilentModelBuild(boolean state) {
    TreePanel.setSilenceMode(state);
    ModelBuildGUI.m_preferences.setSilentBuild(TreePanel.keepSilence());
    detailsButton.setEnabled(!TreePanel.keepSilence());
  }
  public void setExpandMessageOnErrors(boolean state){
    TreePanel.setExpandOnError(state);
    ModelBuildGUI.m_preferences.setExpandWhenError(TreePanel.isExpandOnError());
  }
  //------------------------------------------------------------------------

  public void setProgress(int percent) {
    addPercentToTitle(percent);
    ProgressBar.setValue(percent);
  }

  public void setWindowTitle(String title) {
    this.setTitle(title);
  }

  void cancelButton_actionPerformed(ActionEvent e) {
    TreePanel.sendCommand("<cancel/>");
  }

  public void updateNode(DefaultMutableTreeNode node) {
    DefaultMutableTreeNode rootNode = TreePanel.getMessagesRoot();
    ImageIcon frameIcon;
    if(rootNode!=null) {
      MessageTreeNode messNode = (MessageTreeNode)rootNode.getUserObject();
      if(messNode.m_severity.equals("catastrophe")) {
	frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/ctstricon.gif");
	if(frameIcon!=null)
	  setIconImage(frameIcon.getImage());
	errorsIndicator.setIcon(ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/catastrophes.gif"));
      } else if(messNode.m_severity.equals("error")) {
	frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/erricon.gif");
	if(frameIcon!=null)
	  setIconImage(frameIcon.getImage());
	errorsIndicator.setIcon(ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/errors.gif"));
      } else if(messNode.m_severity.equals("warning")) {
	frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/warnicon.gif");
	if(frameIcon!=null)
	  setIconImage(frameIcon.getImage());
	errorsIndicator.setIcon(ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/warnings.gif"));
      } else if(messNode.m_severity.equals("normal")) {
	frameIcon = ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerricon.gif");
	if(frameIcon!=null)
	  setIconImage(frameIcon.getImage());
	errorsIndicator.setIcon(ImageFactory.getImage("/com/swemancipation/modelbuildgui/Images/noerrors.gif"));
      }
    }
  } /*updateNode*/

  private void addPercentToTitle(int percent) {
    if(iconifiedFlag) {
      String title = getOriginalTitle();
      this.setTitle(title + " - " + new Integer(percent) + "%");
    }
  } /*addPercentToTitle*/

  private void removePercentFromTitle() {
    String title = getOriginalTitle();
    this.setTitle(title);
  } /*removePercentFromTitle*/

  private String getOriginalTitle() {
    String title = this.getTitle();
    if(!title.endsWith("%"))
      return title;
    int pos = title.lastIndexOf(" - ");
    if(pos < 0)
      return title;
    return title.substring(0, pos);
  } /*getOriginalTitle*/

 /* class HelpAfterTracking implements ActionListener {
    private CSH.DisplayHelpFromFocus track = null;
    public HelpAfterTracking(HelpBroker hb) {
      track = new CSH.DisplayHelpFromFocus(hb);
    }
    public void actionPerformed(ActionEvent e) {
      System.out.println("Action performed");
      track.actionPerformed(e);
    }
  }*/

// CodeRover  help action_performed
  public void codeRoverHelp_actionPerformed(ActionEvent e) {
    try {
      hb.setCurrentID(HelpBundle.getBundle().getString("IDR_HELP_MB_MODEL_BUILD"));
      hb.setDisplayed(true);
    } catch(Exception ex) {
      JOptionPane.showMessageDialog(this,
              MessageBundle.getBundle().getString("IDR_MSG_HELP_NOT_AVAILABLE"),
              MessageBundle.getBundle().getString("IDR_MSG_ERROR"),
              JOptionPane.ERROR_MESSAGE);
    }
  }

// update and run button action_performed
  public void updateAndRunButton_actionPerformed(ActionEvent e){
      JPanel listenerContentPane = mbInfoPanel;
      listenerContentPane.remove(DSPPanel);
      listenerContentPane.remove(CaliperPanel);
      listenerContentPane.remove(startButtonsPanel);
      createProgressWindow(listenerContentPane);
      setContentPane(listenerContentPane);
      pack();
      setResizable(m_detailsOn);
      startBuildButton_actionPerformed(e);
  }

  // run button action_performed
  public void runButton_actionPerformed(ActionEvent e){
      m_dspFile = projectFileField.getSelectedItem().toString();
      m_modelDir = modelDirectoryField.getText();
      if(ModelBuildGUI.isAllowedCaliperInfo()==true) {
	if(taskFileField.getSelectedItem()!=null)
	  m_taskFile = taskFileField.getSelectedItem().toString();
      }
      if((m_dspFile == null) || (m_dspFile.length() == 0) ||
	  (m_modelDir == null) || (m_modelDir.length() == 0)) {
	JOptionPane.showMessageDialog(this,
              MessageBundle.getBundle().getString("IDR_MSG_PATH_SHOULD_NOT_BE_EMPTY"),
              MessageBundle.getBundle().getString("IDR_MSG_ERROR"),
              JOptionPane.ERROR_MESSAGE);
	return;
      }
      if(m_taskFile!=null && m_taskFile.length()!=0)
	m_usedFiles.addTaskFile(m_taskFile);
      boolean added = m_usedFiles.addProjectAndPath(m_dspFile, m_modelDir);
      projectFileField.setSelectedIndex(0);
      if(added == true) {
	projectFileField.removeItemAt(0);
	projectFileField.insertItemAt("",0);
	projectFileField.addItem(m_dspFile);
      }
      projectFileField.setSelectedItem(m_dspFile);

      m_usedFiles.saveActiveProject(projectFileField.getSelectedIndex());
      if(ModelBuildGUI.isAllowedCaliperInfo()==true)
	m_usedFiles.saveActiveTask(taskFileField.getSelectedIndex());

      TreePanel.sendCommand("<dsp>" + m_dspFile + "</dsp>");
      DebugLog.Add("Command <dsp> has been sent.");
      TreePanel.sendCommand("<output>" + m_modelDir + "</output>");
      DebugLog.Add("Command <output> has been sent.");

      double timeout = ModelBuildGUI.m_preferences.getKillTimeout();
      if(timeout>0) {
	TreePanel.sendCommand("<autoshutdown>"+Integer.toString((int)(timeout*3600))+"</autoshutdown>");
	DebugLog.Add("Command <autoshutdown> has been sent.");
      }

      TreePanel.sendCommand("<start_pmod_server/>");
      DebugLog.Add("Command <start_pmod_server> has been sent.");
      JPanel buttonContentPane = mbInfoPanel;
      buttonContentPane.remove(DSPPanel);
      buttonContentPane.remove(CaliperPanel);
      buttonContentPane.remove(startButtonsPanel);
      setContentPane(buttonContentPane);
      menuRun.setVisible(false);
      menuHelp.setVisible(false);
      menuFile.setVisible(false);
      menuView.setVisible(false);
      waitForReady();
  }

   private void createHelpSet() {
	ClassLoader loader = this.getClass().getClassLoader();

//Find URL in Dislite\classes(debug version)
	 // URL urlFinder=loader.getResource("");
	 // File help=new File(urlFinder.getPath()+"Help/"+"CodeRoverHelp.hs");//end


//Find URL in the CodeRover install (.jar file)
	URL urlFinder=loader.getResource("CodeRoverHelp.hs");//end
	if(null != urlFinder ) {
	    try {
	    //Find URL in Dislite\classes(debug version)
	       // URL urlHelp = help.toURL();
	       // hs = new HelpSet(loader, urlHelp);//end
	    //Find URL in the CodeRover install (.jar file)
		hs = new HelpSet(loader, urlFinder);//end
	    }catch (Exception ee) {
		System.out.println (MessageBundle.getBundle().getString("IDR_MSG_CAN_NOT_CREATE_HELPSET")+ee);
		ee.printStackTrace();
		return;
	    }
       }
  }
} /*class MainFrame*/
