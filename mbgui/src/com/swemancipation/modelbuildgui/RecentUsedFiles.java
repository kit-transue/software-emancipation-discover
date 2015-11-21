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

import javax.swing.JComboBox;
import java.util.*;
import java.io.*;
import java.lang.*;

public class RecentUsedFiles extends Object{
  private Vector  m_projects = new Vector();
  private Vector  m_paths = new Vector();
  private Vector  m_taskFiles = new Vector();
  private boolean m_caliperChecked = false;
  private int     m_taskNum = 0;
  private int     m_projectNum = 0;

  public RecentUsedFiles() {
    restore();
  }

  public void setCaliper(boolean on) {
    m_caliperChecked = on;
  }

  public boolean isCaliperChecked() {
    return m_caliperChecked;
  }

  public boolean addProjectAndPath(String project, String path){
    if(m_projects.contains(project)==true) {
      for(int i=0;i<m_projects.size();i++) {
        if(m_projects.get(i).toString().equals(project) == true) {
          m_paths.set(i,path);
          return false;
        }
      }
    }
    m_projects.insertElementAt(project,0);
    m_paths.insertElementAt(path,0);
    return true;
  }

  public boolean addTaskFile(String task){
    if(m_taskFiles.contains(task)==true) {
      return false;
    }
    m_taskFiles.insertElementAt(task,0);
    return true;
  }

  public void save(){
    String  workingDir = System.getProperty("user.home",null);
    String  pathSeparator = System.getProperty("file.separator");
    if(workingDir==null)
      return;
    else {
      workingDir += pathSeparator + "ruf.dat";
    }

    File file = new File(workingDir);
    try{
      if(file.createNewFile()==false) {
        file.delete();
        file.createNewFile();
      }
      FileOutputStream  theRUFStream = new FileOutputStream(file);
      ObjectOutputStream  theRUFSave = new ObjectOutputStream(theRUFStream);
      theRUFSave.writeObject("RUF data");
      theRUFSave.writeObject (m_projects);
      theRUFSave.writeObject (m_paths);
      theRUFSave.writeObject (m_taskFiles);
      theRUFSave.writeObject (new Boolean(m_caliperChecked));
      theRUFSave.writeObject (new Integer(m_projectNum));
      theRUFSave.writeObject (new Integer(m_taskNum));
      theRUFSave.close();
    }catch(FileNotFoundException e){
      System.out.println("Can't open file stream for saving RUF.");
      System.out.println(e.getMessage());
    }catch(IOException e){
      System.out.println("Can't save RUF.");
      System.out.println(e.getMessage());
    }
  }

  private void restore(){
    String  workingDir = System.getProperty("user.home",null);
    String  pathSeparator = System.getProperty("file.separator");
    if(workingDir==null)
      return;
    else {
      workingDir += pathSeparator + "ruf.dat";
    }

    File file = new File(workingDir);
    if( file.exists() == false)
      return;

    try{
      FileInputStream  theRUFStream = new FileInputStream(file);
      ObjectInputStream  theRUFLoad = new ObjectInputStream(theRUFStream);
      String description = (String)theRUFLoad.readObject();
      m_projects = (Vector)theRUFLoad.readObject();
      m_paths = (Vector)theRUFLoad.readObject();
      m_taskFiles = (Vector)theRUFLoad.readObject();
      Boolean caliperState = (Boolean)theRUFLoad.readObject();
      m_caliperChecked = caliperState.booleanValue();
      Integer projectNum = (Integer)theRUFLoad.readObject();
      m_projectNum = projectNum.intValue();
      Integer taskNum = (Integer)theRUFLoad.readObject();
      m_taskNum = taskNum.intValue();
      theRUFLoad.close();
    }catch(FileNotFoundException e){
      System.out.println("Can't open file stream for loading RUF.");
      System.out.println(e.getMessage());
    }catch(IOException e){
      System.out.println("Can't load RUF.");
      System.out.println(e.getMessage());
    }catch(ClassNotFoundException e){
      System.out.println("It's not a RUF file.");
      System.out.println(e.getMessage());
    }
    for(int i=0;i<m_projects.size();i++) {
      if(m_projects.get(i)!=null) {
        File tstFile = new File(m_projects.get(i).toString());
        if(tstFile.exists())
          continue;
      }
      m_projects.remove(i);
      m_paths.remove(i--);
    }
    for(int i=0;i<m_taskFiles.size();i++) {
      if(m_taskFiles.get(i)!=null) {
        File tstFile = new File(m_taskFiles.get(i).toString());
        if(tstFile.exists())
          continue;
      }
      m_taskFiles.remove(i--);
    }
  }

  public int projectsCount() {
    return m_projects.size();
  }

  public int tasksCount() {
    return m_taskFiles.size();
  }

  public String getRUProject(int idx) {
    if(idx < m_projects.size())
      return m_projects.get(idx).toString();
    else
      return null;
  }

  public String getRUPath(int idx) {
    if(idx < m_paths.size())
      return m_paths.get(idx).toString();
    else
      return null;
  }

  public String getRUTask(int idx) {
    if(idx < m_taskFiles.size())
      return m_taskFiles.get(idx).toString();
    else
      return null;
  }

  public String getRUPath(String project) {
    if(m_projects.contains(project)==true) {
      for(int i=0;i<m_projects.size();i++) {
        if(m_projects.get(i).toString().equals(project) == true) {
          return m_paths.get(i).toString();
        }
      }
    }
    return null;
  }

  public int getActiveProject() {
    return m_projectNum;
  }

  public int getActiveTask() {
    return m_taskNum;
  }

  public void saveActiveProject(int projectNum) {
    m_projectNum = projectNum;
  }

  public void saveActiveTask(int taskNum) {
    m_taskNum = taskNum;
  }
}
