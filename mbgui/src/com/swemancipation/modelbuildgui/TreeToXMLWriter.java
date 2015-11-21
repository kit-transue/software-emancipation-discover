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
 * Title:        Model build gui<p>
 * Description:  <p>
 * Copyright:    <p>
 * Company:      UPSPRING Software<p>
 * @author       Dmitry Ryachovsky
 * @version 1.0
 */
package com.swemancipation.modelbuildgui;

import com.swemancipation.CodeRover.Utilities.MessageTreeNode;

import java.io.*;
import javax.swing.tree.*;
import javax.swing.*;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Attr;
import org.apache.xerces.domx.XGrammarWriter;
import org.apache.xerces.dom.TextImpl;
import org.apache.xerces.dom.ElementImpl;
import org.apache.xerces.dom.DocumentImpl;

public class TreeToXMLWriter extends XGrammarWriter{
  private DocumentImpl  m_xmlDocument = new DocumentImpl();
  public TreeToXMLWriter(OutputStream stream, DefaultMutableTreeNode treeRoot) {
    super(stream);
    ElementImpl xmlRoot = (ElementImpl)m_xmlDocument.createElement("BuildLog");
    m_xmlDocument.appendChild(xmlRoot);
    createXMLTree(xmlRoot,treeRoot);
    printGrammar("",m_xmlDocument.getDoctype());
    printElement(xmlRoot);
  }

  public static String replaceToTags(String text, String tag, String newTag) {
    String messText;
    int tagLen = tag.length();

    if(text.indexOf(tag) == -1)
      messText = text;
    else {
      int startIdx = text.indexOf(tag);
      messText = text.substring(0,startIdx);
      messText += newTag;
      while(text.indexOf(tag,startIdx+tagLen)!=-1) {
        messText += text.substring(startIdx+tagLen,text.indexOf(tag,startIdx+tagLen));
        messText += newTag;
        startIdx = text.indexOf(tag,startIdx+tagLen);
      }
      if((startIdx+tagLen) < text.length())
        messText += text.substring(startIdx+tagLen);
    }
    return messText;
  }

  private boolean createXMLTree(ElementImpl xmlRoot, DefaultMutableTreeNode treeRoot) {
    for(int i=0;i<treeRoot.getChildCount();i++) {
      DefaultMutableTreeNode treeCurrent = (DefaultMutableTreeNode)treeRoot.getChildAt(i);
      MessageTreeNode message = (MessageTreeNode)treeCurrent.getUserObject();
      ElementImpl xmlCurrent = null;
      try{
        if(message.m_severity.length() != 0 && message.m_severity.charAt(0) != ' ')
          xmlCurrent = (ElementImpl)xmlRoot.appendChild(m_xmlDocument.createElement(message.m_severity.toUpperCase()));
        else
          xmlCurrent = (ElementImpl)xmlRoot.appendChild(m_xmlDocument.createElement("BAD"));
      } catch(Exception exception) {
        JOptionPane.showMessageDialog(ModelBuildGUI.getFrame(), "Create element exception" + exception + "\nElement " + message.m_severity.toUpperCase()+ ".");
      }
      String messText = replaceToTags(message.m_text, "<br/>", "\n");
      try{
        if(xmlCurrent != null)
          xmlCurrent.appendChild(m_xmlDocument.createTextNode(messText));
      } catch(Exception exception) {
        JOptionPane.showMessageDialog(ModelBuildGUI.getFrame(), "Create text exception" + exception);
      }

      if(treeCurrent.isLeaf()==false)
        createXMLTree(xmlCurrent, treeCurrent);
    }
    return true;
  }
}
