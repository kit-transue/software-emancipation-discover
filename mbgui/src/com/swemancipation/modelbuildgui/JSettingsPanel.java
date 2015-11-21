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

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c)
 * Company:
 * @author
 * @version 1.0
 */

import java.awt.*;
import java.awt.event.*;
import javax.swing.border.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.help.HelpSet;
import javax.help.*;
import java.util.*;

import com.swemancipation.util.HelpBundle;
import com.swemancipation.util.MessageBundle;

public class JSettingsPanel extends JPanel {
  private JComboBox uiMode = new JComboBox(new Object[] {MessageBundle.getBundle().getString("IDR_MSG_SIMPLE"),MessageBundle.getBundle().getString("IDR_MSG_PROJECT_EDITOR")});
  private JEnabledTextField killTimeField = new JEnabledTextField();
  private JCheckBox enableDFACheck = new JCheckBox(MessageBundle.getBundle().getString("IDR_MSG_ENABLE_DFA"),false);
  private JCheckBox keepLogCheck = new JCheckBox(MessageBundle.getBundle().getString("IDR_MSG_KEEP_BUILD_LOG"),false);
  private JCheckBox silentBuildCheck = new JCheckBox(MessageBundle.getBundle().getString("IDR_MSG_SILENT_MODEL_BUILD"),false);
  private JCheckBox expandMessagesCheck = new JCheckBox(MessageBundle.getBundle().getString("IDR_MSG_EXPAND_MESSAGE_ON_ERROR"),true);

  private Vector listeners = null;

  public JSettingsPanel() {
    init();
  }

  public void init() {
    JPanel  contentPane = this;
    double weighty = 1;
    contentPane.setLayout(new GridBagLayout());

    uiMode.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent event) {
        Object selectedItem = uiMode.getSelectedItem();
        if(selectedItem!=null)
          fireUIModeChanged(selectedItem.toString());
      }
    });

    JPanel uiPanel = new JPanel(new BorderLayout());
    CSH.setHelpIDString(uiMode, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    uiPanel.add(new JLabel(MessageBundle.getBundle().getString("IDR_LBL_BUILD_UI_MODE")),BorderLayout.WEST);
    uiPanel.add(uiMode,BorderLayout.CENTER);
    contentPane.add(uiPanel,new GridBagConstraints(0, 0, 1, 1, 1.0, weighty
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));

    enableDFACheck.setEnabled(false);
    CSH.setHelpIDString(enableDFACheck, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    contentPane.add(enableDFACheck,new GridBagConstraints(0, 1, 1, 1, 1.0, weighty
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));
    JEnabledPanel killPanel = new JEnabledPanel(new BorderLayout());
    CSH.setHelpIDString(killPanel, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    killTimeField.setMinimumSize(new Dimension(50,0));
    killTimeField.setPreferredSize(new Dimension(50,0));
    killTimeField.addInputMethodListener(new InputMethodListener() {
      public void inputMethodTextChanged(InputMethodEvent event){
        JTextField editor = (JTextField)event.getSource();
        String text = editor.getText();
        try {
          double time = Double.parseDouble(text);
          fireKillTimeChanged(time);
        } catch(Exception ex) {
        }
      }
      public void caretPositionChanged(InputMethodEvent event) {
      }
    });
    killTimeField.addCaretListener(new CaretListener() {
      public void caretUpdate(CaretEvent e) {
        JTextField editor = (JTextField)e.getSource();
        String text = editor.getText();
        try {
          double time = Double.parseDouble(text);
          fireKillTimeChanged(time);
        } catch(Exception ex) {
        }
      }
    });
    killPanel.add(killTimeField,BorderLayout.CENTER);

    JLabel killLabel = new JLabel(MessageBundle.getBundle().getString("IDR_LBL_KILL_UNUSED_SERVER_TIMEOUT"));
    killLabel.setBorder(new EmptyBorder(0,0,0,10));

    JLabel hoursLabel = new JLabel(MessageBundle.getBundle().getString("IDR_LBL_HOURS"));
    hoursLabel.setBorder(new EmptyBorder(0,10,0,0));
    killPanel.add(hoursLabel,BorderLayout.EAST);
    killPanel.add(killLabel,BorderLayout.WEST);
    contentPane.add(killPanel,new GridBagConstraints(0, 2, 1, 1, 1.0, weighty
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));

    CSH.setHelpIDString(keepLogCheck, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    keepLogCheck.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        fireKeepLogChanged(((JCheckBox)e.getSource()).isSelected());
      }
    });
    contentPane.add(keepLogCheck,new GridBagConstraints(0, 3, 1, 1, 1.0, 1.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));
    silentBuildCheck.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        fireSilentModeBuildChanged(((JCheckBox)e.getSource()).isSelected());
      }
    });
    CSH.setHelpIDString(silentBuildCheck, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    contentPane.add(silentBuildCheck,new GridBagConstraints(0, 4, 1, 1, 1.0, weighty
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));
    expandMessagesCheck.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        fireExpandMessageOnErrorsChanged(((JCheckBox)e.getSource()).isSelected());
      }
    });
    CSH.setHelpIDString(expandMessagesCheck, HelpBundle.getBundle().getString("IDR_HELP_MB_INFO_MODEL"));
    contentPane.add(expandMessagesCheck,new GridBagConstraints(0, 5, 1, 1, 1.0, weighty
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(2, 3, 2, 0), 0, 0));
  }

  public void addSettingsChangeListener(SettingsChangeListener listener) {
    if(listeners == null) listeners = new Vector();
    listeners.addElement(listener);
  }

  public void removeSettingsChangeListener(SettingsChangeListener listener) {
    if(listeners == null) return;
    listeners.removeElement(listener);
  }

  protected void fireUIModeChanged(String mode) {
    if(listeners == null) return;

    for(int i=listeners.size()-1;i>=0;i--) {
      ((SettingsChangeListener)listeners.get(i)).setUIMode(mode);
    }
  }

  protected void fireSilentModeBuildChanged(boolean state) {
    if(listeners == null) return;

    for(int i=listeners.size()-1;i>=0;i--) {
      ((SettingsChangeListener)listeners.get(i)).setSilentModelBuild(state);
    }
  }

  protected void fireExpandMessageOnErrorsChanged(boolean state) {
    if(listeners == null) return;

    for(int i=listeners.size()-1;i>=0;i--) {
      ((SettingsChangeListener)listeners.get(i)).setExpandMessageOnErrors(state);
    }
  }

  protected void fireKeepLogChanged(boolean state) {
    if(listeners == null) return;

    for(int i=listeners.size()-1;i>=0;i--) {
      ((SettingsChangeListener)listeners.get(i)).setKeepLog(state);
    }
  }

  protected void fireKillTimeChanged(double hours) {
    if(listeners == null) return;

    for(int i=listeners.size()-1;i>=0;i--) {
      ((SettingsChangeListener)listeners.get(i)).setKillTime(hours);
    }
  }

  public void setUIMode(String mode) {
    uiMode.setSelectedItem(mode);
    fireUIModeChanged(mode);
  }

  public String getUIMode() {
    return uiMode.getSelectedItem().toString();
  }

  public boolean silentModelBuild() {
    return silentBuildCheck.isSelected();
  }

  public boolean expandOnErrors() {
    return expandMessagesCheck.isSelected();
  }

  public void setExpandOnErrors(boolean state) {
    expandMessagesCheck.setSelected(state);
    fireExpandMessageOnErrorsChanged(state);
  }

  public void setSilentModelBuild(boolean state){
    silentBuildCheck.setSelected(state);
    fireSilentModeBuildChanged(state);
  }

  public boolean keepLog() {
    return keepLogCheck.isSelected();
  }

  public void setKeepLog(boolean state) {
    keepLogCheck.setSelected(state);
    fireKeepLogChanged(state);
  }

  public double getKillTime() {
    double time = -1;
    try {
      time = Double.parseDouble(killTimeField.getText());
    } catch(Exception ex) {
      time = -1;
    }
    return time;
  }

  public void setKillTime(double time) {
    killTimeField.setText(Double.toString(time));
    fireKillTimeChanged(time);
  }
}
