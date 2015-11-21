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

import com.borland.primetime.ide.Browser;

import com.borland.primetime.actions.ActionGroup;
import com.borland.primetime.actions.UpdateAction;

import com.borland.primetime.actions.ActionToolBarPane;

import java.awt.event.ActionEvent;
import java.awt.Dimension;
import java.awt.BorderLayout;

import javax.swing.JScrollPane;
import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JEditorPane;
import javax.swing.JOptionPane;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.text.Document;
import javax.swing.text.BadLocationException;

public class CRCommander extends JComponent {
  public static final int CHECKMENU              = 0x00010000;
  public static final int UNCHECKMENU            = 0x00020000;
  public static final int ENABLEMENU             = 0x00040000;
  public static final int DISABLEMENU            = 0x00080000;

  public static final int MENUACTION_NONE               = 0;
  public static final int MENUACTION_INFO               = 1;
  public static final int MENUACTION_DEFINITION         = 2;
  public static final int MENUACTION_BROWSE             = 4;
  public static final int MENUACTION_ACTIVATE           = 8;
  public static final int MENUACTION_WINDOW             = 16;
  public static final int MENUACTION_EXECUTE_CODEROVER  = 32;

  public static final String menuItems[] = {
      "Symbol Info",
      "Open Definition",
      "Browse Symbol",
      "Activate Browser",
      "Message Window",
      "Start DIScover",
  };

  public static final String menuDescriptions[] = {
    "Displays current symbol description",
    "Opens definition of the current symbol",
    "Sends current symbol to the DIScover",
    "Brings DIScover on top",
    "Shows/hides DIScover message window",
    "Starts DIScover"
  };

  public static final char menuMnemonics[] = {
          'I', 'D', 'B', 'A', 'W', 'S'
  };

  public static final String menuIcons[] = {
    "JBuilderAddIn/images/info.gif",
    "JBuilderAddIn/images/definition.gif",
    "JBuilderAddIn/images/browse.gif",
    "JBuilderAddIn/images/bringontop.gif",
    "JBuilderAddIn/images/checksign.gif",
    "JBuilderAddIn/images/build.gif",
  };

  public static final int menuActions[] = {
    MENUACTION_INFO,
    MENUACTION_DEFINITION,
    MENUACTION_BROWSE,
    MENUACTION_ACTIVATE,
    MENUACTION_WINDOW,
    MENUACTION_EXECUTE_CODEROVER,
  };

  private static ActionGroup crMenuGroup = null;
  private static ActionGroup crActions = null;
  private static ActionGroup crWindow = null;
  private static ActionGroup crExecute = null;

  private ActionToolBarPane  crToolBar;
  private JEditorPane crInfoArea = new JEditorPane();

  private static CRActionHandler menuHandler = null;

  public static void RegisterStaticCRActionHandler(CRActionHandler handler) {
    menuHandler = handler;
  }

  public CRCommander(){
    ActionGroup toolBarActions = new ActionGroup();
    ActionGroup toolBarExecute = new ActionGroup();

    // create the toplevel menu
    crMenuGroup = new ActionGroup("DIScover Worktray", 'M', "DIScover Worktray tools integration");
    crActions = new ActionGroup("Actions");
    crExecute = new ActionGroup("Execute");
    crWindow = new ActionGroup("Window");
    int action = MENUACTION_NONE;
    for(int i=0;i<menuIcons.length;i++) {
        action = menuActions[i];
        ImageIcon icon = null;
        if(menuIcons[i]!=null)
          icon = new ImageIcon(ClassLoader.getSystemResource(menuIcons[i]));
        MenuListener listener = new MenuListener(menuHandler,action,menuItems[i],menuMnemonics[i],menuDescriptions[i],icon);
        switch(i) {
          case 0: case 1: case 2: case 3:
            crActions.add(listener);
            toolBarActions.add(new MenuListener(menuHandler,action,menuItems[i],menuMnemonics[i],menuDescriptions[i],icon));
            break;
          case 4:
            listener.setCheckedType(true);
            crWindow.add(listener);
            break;
          case 5:
            crExecute.add(listener);
            toolBarExecute.add(new MenuListener(menuHandler,action,menuItems[i],menuMnemonics[i],menuDescriptions[i],icon));
            break;
        };
    }
    crMenuGroup.add(crActions);
    crMenuGroup.add(crWindow);
    crMenuGroup.add(crExecute);

    crToolBar = new ActionToolBarPane(menuHandler);
    crToolBar.addGroup(toolBarActions);
    crToolBar.addGroup(toolBarExecute);
    crToolBar.setBorder(BorderFactory.createEmptyBorder());

    crInfoArea.setBorder(BorderFactory.createEtchedBorder());
    crInfoArea.setEditable(false);

    setLayout(new BorderLayout());
    add(crToolBar,BorderLayout.NORTH);
    add(new JScrollPane(crInfoArea),BorderLayout.CENTER);
  }

  public void show() {
    Browser.addMenuGroup(6,crMenuGroup);  // now add the whole menu
  }

  public void showMessage(String text) {
    Document  document = crInfoArea.getDocument();
    try{
      document.insertString(0,text,null);
    } catch (BadLocationException e) {
    }
  }

  public void updateActions() {
    int nActionGroupsCount = crMenuGroup.getActionCount();
    AddInMain.log.println("updateActions: Action groups count is " + nActionGroupsCount);
    for(int i=0;i<nActionGroupsCount;i++) {
      ActionGroup group = (ActionGroup)crMenuGroup.getAction(i);
      int nActionsCount = group.getActionCount();
      AddInMain.log.println("updateActions: group is " + group.getShortText() + " actions=" + nActionsCount);
      for(int j = 0; j < nActionsCount; j++) {
        AddInMain.log.println("updateActions: Action #" + j + "is " + group.getAction(j).toString());
        MenuListener action = (MenuListener)group.getAction(j);
        action.update(this);
      }
    }
  }
}

class MenuListener extends UpdateAction {
  protected CRActionHandler handler = null;
  protected int     action = -1;
  protected boolean checkedType = false;
  protected Icon    checkIcon = null;
  protected boolean checked = false;
  public MenuListener (CRActionHandler handler, int action, String menuText,
                          char mnemonic, String description, Icon icon) {
    this(handler, action, menuText, mnemonic, description, icon,false);
  }

  public MenuListener (CRActionHandler handler, int action, String menuText,
                          char mnemonic, String description, Icon icon, boolean checkedType) {
    super(menuText,mnemonic,description,icon);
    this.action = action;
    this.handler = handler;
    setCheckedType(checkedType);
  }

  public int getAction() {
    return action;
  }

  public CRActionHandler getHandler() {
    return handler;
  }

  public boolean isCheckedType() {
    return checkedType;
  }

  public void setCheckedType(boolean checkedType) {
    this.checkedType = checkedType;
    if(checkedType == true)
      checkIcon = getSmallIcon();
    setChecked(false);
  }

  protected void setIcon(Icon icon) {
    setLargeIcon(icon);
    setSmallIcon(icon);
  }

  public void setChecked(boolean checked) {
    if(checkedType == true) {
      this.checked = checked;
      if(checked == true)
        setIcon(checkIcon);
      else
        setIcon(null);
    }
  }

  public boolean isChecked() {
    return checked;
  }

  public void update(Object source) {
    int state = handler.getActionState(action);
    // only if action is known
    if(state!=0) {
      int operation = state & 0xffff0000;
      if((state & action)==action){
        if((operation & CRCommander.CHECKMENU)==CRCommander.CHECKMENU) {
          setChecked(true);
        }
        if((operation & CRCommander.UNCHECKMENU)==CRCommander.UNCHECKMENU) {
          setChecked(false);
        }
        if((operation & CRCommander.ENABLEMENU)==CRCommander.ENABLEMENU) {
          setEnabled(true);
        }
        if((operation & CRCommander.DISABLEMENU)==CRCommander.DISABLEMENU) {
          setEnabled(false);
        }
      }
    }
  }

  public void actionPerformed(ActionEvent e) {
    if(handler != null) {
      handler.menuActionPerformed(action);
    }
  }
}
