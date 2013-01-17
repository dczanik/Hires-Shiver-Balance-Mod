/*
 * This file is written by, and published with permission from,
 * Death 999 on the Ur-Quan-Masters Discussion Forum. All code
 * (and copyright) is strictly his.
 *
 * This program will replace 50% of black pixels in a directory
 * full of images with random RGB. Example:
 *
 * $ javac UmgahReconinator.java
 * $ java UmgahReconinator /path/to/images/
 */

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Transparency;

import java.awt.image.BufferedImage;

import java.util.*;

import javax.imageio.ImageIO;

import java.io.*;



public class UmgahReconinator {
 
 
  public static void main(String[] args) {
    try {
      File dir = new File(args[0]);
      if (! dir.exists()) { System.err.println("pass a directory!"); System.exit(1); }
      if (! dir.isDirectory()) { System.err.println("pass a directory!"); System.exit(1); }
      for (File f: dir.listFiles()) {
        speckleBlack(f, f);
      }
    } catch (IOException ioe) {}
  }
 

  public static void speckleBlack(File image, File target) throws IOException {
    BufferedImage src = ImageIO.read(image);
   
    int dw = src.getWidth();
    int dh = src.getHeight();
    BufferedImage dst = new BufferedImage(dw, dh, BufferedImage.TYPE_INT_ARGB); // this last is crucial.
   
    Random r = new Random();
    int[] cols = src.getRGB(0,0, dw, dh, null, 0, dw);
    for (int j=0; j < cols.length; j++) {
      /*
      if ((cols[j] & 0xff000000) != 0) {
        if (r.nextBoolean()) cols[j] = 0xff000000;
        else cols[j] = 0xff000000 + (r.nextInt() & 0x00ffffff);
      }
       */

      if ((cols[j] & 0x00ffffff) == 0) {
        if (r.nextBoolean())
          cols[j] = (cols[j] & 0xff000000) + (r.nextInt() & 0x00ffffff);
        else
          cols[j] = cols[j] & 0xff000000;
      }
      else cols[j] = 0x00000000;
      
    } // end for every pixel
   
    dst.setRGB(0,0,dw,dh,cols,0,dw);
     
    ImageIO.write(dst, "png", new FileOutputStream(target));
  } // end speckleBlack

 
} // end class def