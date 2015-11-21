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

import javax.swing.filechooser.*;


/**
 * Title:
 * Description:
 * Copyright:    Copyright (c)
 * Company:
 * @author Dmitry Ryachovsky
 * @version 1.0
 */
import java.io.File;

public class FileFilterAdapter extends FileFilter {
  private String description = null;
  private String[] exts = null;

  public FileFilterAdapter(String description,String ext) {
    this(description,new String[] {ext});
  }

  public FileFilterAdapter(String description,String[] exts) {
    this.description = description;
    this.exts = exts;
  }

  public boolean accept(File file) {
    if(file == null)
      return false;
    if(file.isDirectory())
      return true;
    String name = file.getName();
    for(int i=0;i<exts.length;i++) {
      if(name.endsWith(exts[i]))
        return true;
    }
    return false;
  }

  public String getDescription() {
    return description;
  }
}
