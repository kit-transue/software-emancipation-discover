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
package com.swemancipation.modelbuildgui;

import javax.swing.ListCellRenderer;
import java.awt.Component;
import javax.swing.JList;
import javax.swing.JTextField;
import javax.swing.BorderFactory;
import java.awt.Color;
import javax.swing.border.Border;
import javax.swing.UIManager;
import java.awt.FontMetrics;
import java.awt.Dimension;

public class JPathListRenderer extends JTextField implements ListCellRenderer {
  public JPathListRenderer() {
    setOpaque(true);
    setBorder(BorderFactory.createEmptyBorder());
    setEditable(true);
  }

  public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
    String visibleValue = value.toString();

    FontMetrics metrics = this.getFontMetrics(this.getFont());
    while(list.getWidth()<metrics.stringWidth(visibleValue))
      visibleValue = visibleValue.substring(1);

    if(isSelected == true) {
      this.setBackground(UIManager.getColor("ComboBox.selectionBackground"));
      this.setForeground(UIManager.getColor("ComboBox.selectionForeground"));
    } else {
      this.setBackground(UIManager.getColor("ComboBox.background"));
      this.setForeground(UIManager.getColor("ComboBox.foreground"));
    }
    this.setText(visibleValue);
    return this;
  }
}
