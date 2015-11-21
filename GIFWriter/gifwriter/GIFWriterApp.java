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
package gifwriter;

/**
 * Title:        Program for automated version update
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:      MKS
 * @author Dmitry Ryachovsky
 * @version 1.0
 */

import java.io.*;
import javax.swing.ImageIcon;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import Trends.Acme.JPM.Encoders.RGBGIFEncoder;

public class GIFWriterApp {
  static int     textX = 0;
  static int	 textY = 0;
  static String  text = null;
  static String  fontName = "Arial";
  static String  inputFile = null;
  static String  outputFile = null;
  static int     fontSize = 10;
  static int	 fontStyle = 0;
  static boolean isItalic = false;
  static int	 fontWeight = 100;

  private abstract class ImageListener implements ImageObserver {
    public ImageListener( ) {}
    public final boolean imageUpdate( Image img, int infoflags, int x, int y, int width, int height ) {
      if((infoflags & ImageObserver . ABORT ) == 0 &&
         (infoflags & ImageObserver . ERROR ) == 0) {
        return action( img, infoflags, width, height );
      } else {
        return false;
      }
    }

    protected abstract boolean action( Image img, int infoflags, int width, int height );
  }

  private class ContentListener extends ImageListener {
    public ContentListener ( ) { }
    protected boolean action( Image img, int infoflags, int width, int height ) {
      return true;
    }
  }


  public GIFWriterApp() {
    ImageIcon image = new ImageIcon(Toolkit.getDefaultToolkit().getImage(inputFile));

    int width = image.getIconWidth();
    int height = image.getIconHeight();
    BufferedImage bIm = new BufferedImage(width,height,
                                        BufferedImage.TYPE_INT_RGB);
    Graphics g = bIm.getGraphics();
    g.drawImage(image.getImage(),0,0,new ContentListener());
    int fontType = Font.PLAIN;
    if(isItalic)
      fontType += Font.ITALIC;
    if(fontWeight>100)
      fontType+=Font.BOLD;
    Font font = new Font(fontName,fontType,fontSize);
    g.setFont(font);
    g.setColor(Color.black);
    g.drawString(text,textX,textY);
    g.dispose();
    File outputFileHandle = new File(outputFile);

    try {
      BufferedOutputStream fos = new BufferedOutputStream(new FileOutputStream(outputFileHandle),100000);
      RGBGIFEncoder gifEncoder = new RGBGIFEncoder(bIm, fos);
      gifEncoder.encode();
      fos.close();
    } catch(FileNotFoundException fnfEx){
    } catch(IOException ioEx) {
    }
  }


  private static void debug(int ingtVal) {
    System.out.print(ingtVal);
  }

  private static void debug(String str) {
    System.out.print(str);
  }

  private static void debugln(String str) {
    System.out.println(str);
  }

  public static void main(String[] args) {
    boolean stop = false;

    if(args.length < 2) {
      debug("Usage: imagewriter <input image file> <output image file> [-x <x coordinate>] [-y <y coordinate>] [-text <text to write>] [-font <font name>] [-size <font size>]\n");
      return;
    }

    inputFile = args[0];
    outputFile = args[1];

    for(int i=2;i<args.length;i++) {
      if(args[i].equalsIgnoreCase("-x")) {
        textX = Integer.parseInt(args[++i]);
        continue;
      }
      if(args[i].equalsIgnoreCase("-weight")) {
        fontWeight = Integer.parseInt(args[++i]);
        continue;
      }
      if(args[i].equalsIgnoreCase("-italic")) {
        isItalic = true;
        continue;
      }
      if(args[i].equalsIgnoreCase("-stop")) {
        stop = true;
        continue;
      }
      if(args[i].equalsIgnoreCase("-y")) {
        textY = Integer.parseInt(args[++i]);
        continue;
      }
      if(args[i].equalsIgnoreCase("-text")) {
        text = args[++i];
        continue;
      }
      if(args[i].equalsIgnoreCase("-font")) {
        fontName = args[++i];
        continue;
      }
      if(args[i].equalsIgnoreCase("-size")) {
        fontSize = Integer.parseInt(args[++i]);
        continue;
      }
      debug("Unknown: ");
      debug(args[i]);
      debug("\n");
    }

    if(text == null) {
      debug("Nothing to draw\n");
      return ;
    }

    debugln("Converting...");
    debug("   Input image file: \"");
    debug(inputFile);
    debugln("\"");
    debug("   Output image file: \"");
    debug(outputFile);
    debugln("\"");
    debug("   Text: \"");
    debug(text);
    debugln("\"");
    debug("   Coordinates: X=");
    debug(textX);
    debug(" Y=");
    debug(textY);
    debug("\n");
    debug("   Font: \"");
    debug(fontName);
    debug("\"");
    debug(" ");
    if(isItalic) {
      debug("Italic");
      debug(" ");
    }
    debug("   size: ");
    debug(fontSize);
    debug("   weight: ");
    debug(fontWeight);
    debug("\n");

    GIFWriterApp GIFWriterApp = new GIFWriterApp();

    if(stop) {
      BufferedInputStream is = new BufferedInputStream(System.in);
      try{
        is.read();
      } catch(IOException ioe) {
      }
    }
    System.exit(0);
  }
}
