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

import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.IOException;

public class DebugLog {
  private FileOutputStream os;
  private static final boolean debug = false;
  public DebugLog() {
    if(debug) {
      try {
        os = new FileOutputStream(new File("C:/addin.log"));
      } catch(IOException e) {
      }
    }
  }

  public void println(String str) {
    if(debug)
      print(str+"\n");
  }

  public void print(String str) {
    if(debug) {
      try {
        os.write(str.getBytes());
        os.flush();
      } catch(IOException e) {
      }
    }
  }

  public OutputStream getStream() {
    return os;
  }

  public void close() {
    if(debug) {
      try {
        os.close();
      } catch(IOException e) {
      }
    }
  }
}
