/*************************************************************************
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
*************************************************************************/
// FilterList.cpp : implementation file
//

#include "stdafx.h"
#include "FilterSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterList property page
IMPLEMENT_SERIAL(CFilterEntry,CObject,10 )

IMPLEMENT_DYNCREATE(CFilterList, CPropertyPage)

CFilterList::CFilterList() : CPropertyPage(CFilterList::IDD)
{
	//{{AFX_DATA_INIT(CFilterList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_AllFilters.SetSize(0,100);
}

CFilterList::~CFilterList() {
}

void CFilterList::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterList)
	DDX_Control(pDX, IDC_REMOVEBUTTON, m_RemoveButton);
	DDX_Control(pDX, IDC_CHANGEBUTTON, m_ChangeButton);
	DDX_Control(pDX, IDC_NAMEEDIT, m_NameEdit);
	DDX_Control(pDX, IDC_COMMANDEDIT, m_CommandEdit);
	DDX_Control(pDX, IDC_SAVELIST, m_SaveList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilterList, CPropertyPage)
	//{{AFX_MSG_MAP(CFilterList)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	ON_LBN_SELCHANGE(IDC_SAVELIST, OnSelchangeSavelist)
	ON_BN_CLICKED(IDC_CHANGEBUTTON, OnChangebutton)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnRemovebutton)
	ON_EN_CHANGE(IDC_COMMANDEDIT, OnChangeCommandedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterList message handlers



//----------------------------------------------------------------------------------------
// This function will load all filters list from the archive. Will store this list
// in the m_AllFilters array.
//----------------------------------------------------------------------------------------
void CFilterList::LoadFilters(void) {
char* pFileName = "filters.dat";
CFilterEntry* entry;
char buf[1024];


  CString settingsPath;
  if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
      settingsPath=buf;
      settingsPath+="/Settings/";
  } else {
      settingsPath="C:/Discover/Settings/";
  }
  settingsPath+=pFileName;

  TRY  {
	  CFile file(settingsPath, CFile::modeRead);
	  TRY {
         CArchive storage(&file,CArchive::load);
         do {
	        storage >> entry; 
		    m_AllFilters.Add(entry);
         } while(1);
	  } CATCH(CArchiveException, ae) {
		  return;
	  }
      END_CATCH
  }
  CATCH( CFileException, e ) {
	  return;
  }
  END_CATCH

}

//----------------------------------------------------------------------------------------
// This function will save all filters from m_AllFilters array into the archive.
//----------------------------------------------------------------------------------------
void CFilterList::SaveFilters(void) {
char* pFileName = "filters.dat";
int i=0;
char buf[1024];


  CString settingsPath;
  if(GetEnvironmentVariable("PSETHOME_LOCAL",buf,1024)!=0) {
      settingsPath=buf;
      settingsPath+="/Settings/";
  } else {
      settingsPath="C:/Discover/Settings/";
  }
  settingsPath+=pFileName;

  TRY  {
	  CFile file(settingsPath,CFile::modeCreate | CFile::modeWrite);
      CArchive storage(&file,CArchive::store);
      for(i=0;i<m_AllFilters.GetSize();i++) {
		  storage << m_AllFilters[i];
      } 
  }
  CATCH( CFileException, e ) {
	  return;
  }
  END_CATCH

}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Initialize the dialog page and loads all filters from the filter archive.
//----------------------------------------------------------------------------------------
BOOL CFilterList::OnInitDialog()  {
	CPropertyPage::OnInitDialog(); 
	LoadFilters();
	for(register i=0;i<m_AllFilters.GetSize();i++) {
		m_SaveList.SetItemData(m_SaveList.AddString(m_AllFilters[i]->Name),i);
	}
	m_SaveList.SetCurSel(0);
	OnSelchangeSavelist();
	if(m_SaveList.GetCount()==0) {
		m_ChangeButton.EnableWindow(FALSE);
		m_RemoveButton.EnableWindow(FALSE);
	} else {
		m_ChangeButton.EnableWindow(TRUE);
		m_RemoveButton.EnableWindow(TRUE);
    }
	UpdateParentName();
	return TRUE;  
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This callback runs when user press "Add" button. Will insert the name from the
// name edit field and command from the command edit field into filter list and will
// save this list in the archive.
//----------------------------------------------------------------------------------------
void CFilterList::OnAddbutton()  {
CFilterEntry* newEntry = new CFilterEntry;

    m_NameEdit.GetWindowText(newEntry->Name);
	newEntry->Name.TrimLeft();
	newEntry->Name.TrimRight();
	if(newEntry->Name.GetLength()==0) {
		MessageBox("Can't save unnamed filter.","Add",MB_ICONSTOP|MB_OK);
		return;
	}
    m_CommandEdit.GetWindowText(newEntry->Command);
    int pos = m_AllFilters.Add(newEntry);
    int index = m_SaveList.AddString(newEntry->Name);
	m_SaveList.SetItemData(index,pos);
    m_SaveList.SetCurSel(index);

	m_ChangeButton.EnableWindow(TRUE);
	m_RemoveButton.EnableWindow(TRUE);
    SaveFilters();
	SetModified(TRUE);
	UpdateParentName();
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// Callback runs every time the user selected different filter in the filter list.
// It will update the name edit field and command edit field to show the current filter
// name and command.
//----------------------------------------------------------------------------------------
void CFilterList::OnSelchangeSavelist()  {
int index=m_SaveList.GetCurSel();

    if(index==-1) return;
    index=m_SaveList.GetItemData(index);
	m_NameEdit.SetWindowText(m_AllFilters[index]->Name);
	m_CommandEdit.SetWindowText(m_AllFilters[index]->Command);
	SetModified(TRUE);
	UpdateParentName();
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
// This callback runs when user press "Change" button. Will change the current filter
// name getting the new one from the name edit field. Will update the information in the
// m_AllFilters array.
//----------------------------------------------------------------------------------------
void CFilterList::OnChangebutton()  {
int index = m_SaveList.GetCurSel();
int pos   = m_SaveList.GetItemData(index);
CString name;
CString command;

    m_NameEdit.GetWindowText(name);
	name.TrimLeft();
	name.TrimRight();
	if(name.GetLength()==0) {
		MessageBox("Can't save unnamed filter.","Add",MB_ICONSTOP|MB_OK);
		return;
	}
    m_CommandEdit.GetWindowText(command);
	m_AllFilters[pos]->Name    = name;
	m_AllFilters[pos]->Command = command;

    m_SaveList.DeleteString(index);
	index = m_SaveList.AddString(m_AllFilters[pos]->Name);
	m_SaveList.SetItemData(index,pos);
    m_SaveList.SetCurSel(index);
	SetModified(TRUE);

    SaveFilters();
	UpdateParentName();
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This callback runs when user press "Change" button. Will remove current filter from 
// the filter list and from the filter array as well.
//----------------------------------------------------------------------------------------
void CFilterList::OnRemovebutton()  {
int index = m_SaveList.GetCurSel();
CString name;
CString command;

    m_AllFilters.RemoveAt(m_SaveList.GetItemData(index));
    m_SaveList.ResetContent();
	for(register i=0;i<m_AllFilters.GetSize();i++) {
		m_SaveList.SetItemData(m_SaveList.AddString(m_AllFilters[i]->Name),i);
	}

	if(m_SaveList.GetCount()==0) {
		m_ChangeButton.EnableWindow(FALSE);
		m_RemoveButton.EnableWindow(FALSE);
	    m_NameEdit.SetWindowText("");
	    m_CommandEdit.SetWindowText("");
	    SetModified(FALSE);
        SaveFilters();
		return;
	}
    if(m_SaveList.GetCount()<=index) index--;
    int pos = m_SaveList.GetItemData(index);
	m_NameEdit.SetWindowText(m_AllFilters[pos]->Name);
	m_CommandEdit.SetWindowText(m_AllFilters[pos]->Command);
	m_SaveList.SetCurSel(index);
    SaveFilters();
	UpdateParentName();
	SetModified(TRUE);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// This function will set current name and current command in the parent (FilterSheet)
// members  m_Name and m_Command;
//----------------------------------------------------------------------------------------
void CFilterList::UpdateParentName(void) {
FilterSheet* parent;

    parent=(FilterSheet *)GetParent();
    m_NameEdit.GetWindowText(parent->m_Name);
    m_CommandEdit.GetWindowText(parent->m_Command);
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// When this list activated, the command from the parent will be re-directed to the
// editor in order to use FilterWizard generated command.
//----------------------------------------------------------------------------------------
BOOL CFilterList::OnSetActive()  {
FilterSheet* parent;
    parent=(FilterSheet *)GetParent();
	m_CommandEdit.SetWindowText(parent->m_Command);
	return CPropertyPage::OnSetActive();
}
//----------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// All real work we need to do will be done in the OnApply function - it will be called
// automatically by the framework when user press "OK" button.
//---------------------------------------------------------------------------------------
void CFilterList::OnOK()  {
	CPropertyPage::OnOK();
}
//---------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------
// When user press "Apply" button all changes will be saved in the parent and filter
// will be applyed. The same will happen when user press OK button.
//---------------------------------------------------------------------------------------
BOOL CFilterList::OnApply() {
FilterSheet*   parent;
CQueryResults* ctrl;

    parent=(FilterSheet *)GetParent();
    if(parent->GetActiveIndex()==0) {
	   UpdateParentName();
	   ctrl=parent->m_QueryResults;
	   if(ctrl!=NULL) {
	      CString filterQuery;
	      if(parent->m_Command.GetLength()>0)
	         filterQuery.Format("filter {%s}",parent->m_Command);
	      else
		     filterQuery=""; 
          ctrl->SetFilter(filterQuery);
          ctrl->Update();
	   }
	}
	return CPropertyPage::OnApply();
}
//--------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// every time the user type something in the command editor, the new string will be send
// to the parent.
//----------------------------------------------------------------------------------------
void CFilterList::OnChangeCommandedit()  {
  UpdateParentName();
}
//----------------------------------------------------------------------------------------
