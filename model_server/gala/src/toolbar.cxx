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
/**********
 *
 * toolbar.C - ToolBar Classes.
 *
 ***** Classes defined in this file.
 *
 * ToolBarView
 * ToolBar
 * ScrollButton
 * ScrollButtonTimer
 * ToolBarDialog
 * ToolBarDialogSaveButton
 * ToolBarDialogLocalButton
 * ToolBarDialogCancelButton
 * ToolBarButton
 * ToolBarListItem
 * ToolBarButtonDrag
 *
 **********/

#include <vport.h>
#ifndef vcharINCLUDED
#include vcharHEADER
#endif
#ifndef vcontainerINCLUDED
   #include vcontainerHEADER
#endif
#ifndef vapplicationINCLUDED
   #include vapplicationHEADER
#endif

#ifndef _ggeneric_h
   #include <ggeneric.h>
#endif
#ifndef _gglobalFuncs_h
   #include <gglobalFuncs.h>
#endif
#ifndef _viewer_h
   #include <gviewer.h>
#endif
#ifndef _gtoolbar_h
   #include <gtoolbar.h>
#endif
#ifndef _viewerMenu_h
   #include <gviewerMenu.h>
#endif
#ifndef _viewerMenuBar_h
   #include <ViewerMenuBar.h>
#endif
#ifndef _viewerMenuItem
   #include <gviewerMenuItem.h>
#endif

int BUTTONHEIGHT = 22;
int BUTTONWIDTH = 24;
int TOOLBAR_BUTTON_SIZE = 22;

const double SCROLL_WIDTH = 1.0;

// Forward class decls.
class TooBarButton;

void SetToolbarSize()
{ 
#ifdef WIN32
    if(vlaf::Appear() == vlafWIN95)
       TOOLBAR_BUTTON_SIZE = 26;
    else
       TOOLBAR_BUTTON_SIZE = 22;
#else
    TOOLBAR_BUTTON_SIZE = 25;
#endif
}


/**********      toolbarView Class     **********/
toolbarView::toolbarView(Viewer* newViewer, Rect newRect, Pane* newPane)
: View(NULL, newViewer, newRect)

{
  SetPane(newPane);
}




//bordercolor constants
const int MAX_LIGHT = 150;
const double DARKNESS = 0.25;
const double LIGHTNESS = 1.5;
const int CURRENT_CONTAINER_BORDER_WIDTH = 1; 
const int TOP_COLOR_SPACING = 15;

//timer constants
const unsigned long SECONDS = 0;
const unsigned long NANOSECONDS = 500000000;   // half a second
const unsigned long FAST_SECONDS = 0;
const unsigned long FAST_NANOSECONDS = 50000000;   // half a second
const unsigned long WAIT_SECONDS = 1;
const unsigned long WAIT_NANOSECONDS = 0;   // half a second


static charCompare (const void *str1, const void *str2)
{
    return vcharCompare ((const vchar *) str1, (const vchar *) str2);
}





/**********     ToolBar Class     **********/

vkindTYPED_FULL_DEFINITION (ToolBar, vcontainer, "ToolBar");

//initializes the tool bar, basically setting it's viables to null or 0 and then loading the buttons from the preference file (.vgalaxy.1.vr)
void ToolBar::initialize (Viewer *newViewer)
{
    ViewerStateIndex = NumViewerStates - 1; // Last state is "None".
    ViewerStateName = ViewerStateList[ViewerStateIndex];
    myViewer = newViewer;
    Buttons = NULL;
    ScrollButtonState = 0;
    LeftScrollButton = NULL;
    RightScrollButton = NULL;

#ifdef OWNCOLOR
    SetForeground(Viewer::GetToolBarForeground());
    SetBackground(Viewer::GetToolBarBackground());
#endif

    LoadButtons();
    ShowButtonSet();
}

//Changes the state form (0) normal to (2) customize state and vice versa
void ToolBar::SetState(int newState)
{
    ToolBarButton * currentButton = Buttons;
    while (currentButton) {
	currentButton->SetState(newState);
	currentButton = currentButton->GetNext();
    }
}


//adds a new button to the toolbar
void ToolBar::addButton(ToolBarButton *newbutton, int Xcoord, int movedButton)
{
    newbutton->SetState(2);
    
    //finds the button before where the new one will go
    for (ToolBarButton* currentbutton = Buttons; 
	 currentbutton &&
	 (currentbutton->GetNext()) &&
	 (currentbutton->GetX()+ (1.5 *BUTTONWIDTH) < Xcoord); 
	 currentbutton = currentbutton->GetNext());
    //switch to make up for the move ment of the buttons after the button is removed, and before it is added back in
    if (movedButton == 1 && currentbutton->GetPrev())
	currentbutton = currentbutton->GetPrev();
    //case of no buttons in the toolbar
    if (!Buttons) {
	Buttons = newbutton;
	LeftmostShown = newbutton;
	RightmostShown = newbutton;      

    // Buttons in the toolbar, but new button will be the first one.
    } else if (currentbutton == Buttons && Xcoord < (BUTTONWIDTH* .5)) {
	newbutton->SetNext(Buttons);
	newbutton->SetPrev (NULL);
	Buttons->SetPrev (newbutton);
	Buttons = newbutton;
	LeftmostShown = newbutton;
    } else { //all other cases
	if (currentbutton->GetNext())
	    currentbutton->GetNext()->SetPrev(newbutton);
	else
	    RightmostShown = newbutton;
	newbutton->SetNext(currentbutton->GetNext());
	currentbutton->SetNext(newbutton);
	newbutton->SetPrev(currentbutton);
    }
    
    //acutally add the button to the toolbar visually
    AppendItem(newbutton);
    
    currentbutton = newbutton;
    vrect currentbuttonrect;
    //if this is the first button, an original rect must be defined for it
    if (Buttons == newbutton) {
	currentbuttonrect.SetX(0);
	currentbuttonrect.SetY(0);
	currentbuttonrect.SetHeight(BUTTONHEIGHT);
	currentbuttonrect.SetWidth(BUTTONWIDTH);
	currentbutton->SetRect((const vrect *) &currentbuttonrect);
	if (currentbutton->GetNext())
	    currentbutton = currentbutton->GetNext();
	else
	    return;//if there are no other buttons
    }
    
    //sets the rect of every button to be that of the one to it's right
    while (currentbutton->GetNext()) {
	currentbutton->SetRect(currentbutton->GetNext()->GetRect());
	currentbutton = currentbutton->GetNext();
  }

  //if there are scroll bars and then the rightmostshown will have changed if it is not the newbutton
  if (ScrollButtonState == 1 && RightmostShown->GetPrev() && newbutton !=RightmostShown)
    RightmostShown = RightmostShown->GetPrev();
  
  //the rect for the last button, just takes the second to last, and extends its x dimension by button size
  currentbuttonrect = *currentbutton->GetPrev()->GetRect();
  currentbuttonrect.SetX(currentbuttonrect.GetX()+currentbuttonrect.GetWidth());
  currentbutton->SetRect ((const vrect *)&currentbuttonrect);

  // adds scrollbuttons if neccessary
  if (ScrollButtonState == 0 && currentbutton->GetX()+(SCROLL_WIDTH*BUTTONWIDTH) > GetWidth())
      addScrollButtons();
}



//Scrolls right
void ToolBar::ScrollRight ()
{
  //do it only if it has a next or the rightmost button is not totally shown
    if (RightmostShown->GetNext() ||  RightmostShown->GetX()+(SCROLL_WIDTH * BUTTONWIDTH) > GetWidth())
    {
	if (Buttons == NULL)
	    return;
      //move the leftmostshown and rightmostshown
	LeftmostShown = LeftmostShown->GetNext();
	if (RightmostShown->GetNext())
	    RightmostShown = RightmostShown->GetNext();

	//go to the end of the buttonlist
	for (ToolBarButton * currentbutton = Buttons;
                             currentbutton->GetNext();
                             currentbutton = currentbutton->GetNext());
	
	//move all the buttons
	while (currentbutton->GetPrev())
	{
	    currentbutton->SetRect(currentbutton->GetPrev()->GetRect());
	    currentbutton = currentbutton->GetPrev();
	}
	vrect currentbuttonrect;
	//move the last one one more button size
	currentbuttonrect.Copy (currentbutton->GetRect());
	currentbuttonrect.SetX(currentbuttonrect.GetX()-BUTTONWIDTH);
	currentbutton->SetRect (&currentbuttonrect);
    }
  else
      RightScrollButton->DisableButton(); //if it is scrolled all the way, disable the button

  // Always enable the left scroll button;
  LeftScrollButton->EnableButton();
}
//Scrolls left
void ToolBar::ScrollLeft()
{
//do only if there is a previous button
    if (LeftmostShown->GetPrev())
    {
	if (Buttons == NULL)
	    return;
      // move the left and right most shown accordingly
	LeftmostShown = LeftmostShown->GetPrev();
	RightmostShown = RightmostShown->GetPrev();

	ToolBarButton * currentbutton = Buttons;
	//move all the rects
	while (currentbutton->GetNext())
	{
	    currentbutton->SetRect(currentbutton->GetNext()->GetRect());
	    currentbutton = currentbutton->GetNext();
	}
	vrect currentbuttonrect;
	//extend the last button one buttonsize farther
	currentbuttonrect.Copy (currentbutton->GetRect());
	currentbuttonrect.SetX(currentbuttonrect.GetX() + BUTTONWIDTH);
	currentbutton->SetRect (&currentbuttonrect);
    }
  else
      LeftScrollButton->DisableButton(); // if you can't scroll disable the button

  // Always enable the right scroll button;
  RightScrollButton->EnableButton();
}

//removes a button
void ToolBar::removeButton (ToolBarButton *oldbutton)
{
  //if there are scoll buttons, the precedure is a little more in depth
    if (ScrollButtonState == 1)
    {
      //check to see if there is a extra button to the left
	if (LeftmostShown->GetPrev())
	{
	    LeftmostShown = LeftmostShown->GetPrev();
	    if (RightmostShown == oldbutton)
		RightmostShown = oldbutton->GetPrev();
	    /*if there are no unshownbuttons, remove the scroll buttons..(needs to modified to check to see if the tool bar 
	      would fit without the scroll buttons*/ 
	    if (!LeftmostShown->GetPrev() && !RightmostShown->GetNext())
		removeScrollButtons();
	    //move all the buttons up till the removed button to the right
	    ToolBarButton *currentbutton = Buttons;
	    while (currentbutton != oldbutton)
	    {
		currentbutton->SetRect(currentbutton->GetNext()->GetRect());
		currentbutton = currentbutton->GetNext();
	    }
	}
	else
	{
	  //if left has no prev, right must have a next in order to have scroll buttons
	  RightmostShown = RightmostShown->GetNext();
	  
	  if (LeftmostShown == oldbutton)
		LeftmostShown = oldbutton->GetNext();
	  //if there are no unshown buttons remove the scroll buttons
	  if (!RightmostShown->GetNext())
	    removeScrollButtons();
	  
	  for (ToolBarButton *currentbutton = Buttons; currentbutton && currentbutton->GetNext(); currentbutton = currentbutton->GetNext());
	  //go all the way to the end, and then move all the buttons in
	  while (currentbutton != oldbutton)
	    {
	      currentbutton->SetRect(currentbutton->GetPrev()->GetRect());
		currentbutton = currentbutton->GetPrev();
	    }
	}
      }
    else  //if there are not scoll buttons things are simpler
      {
	if (oldbutton == RightmostShown)
	    RightmostShown = oldbutton->GetPrev();
	if (oldbutton == LeftmostShown)
	    LeftmostShown = oldbutton->GetNext();
	for (ToolBarButton *currentbutton = Buttons; currentbutton && currentbutton->GetNext(); currentbutton = currentbutton->GetNext());
	//moves all the buttons to the right of the removed button, in to the right
	while (currentbutton != oldbutton)
	  {
	    currentbutton->SetRect(currentbutton->GetPrev()->GetRect());
	    currentbutton = currentbutton->GetPrev();
	  }
      }	
    //set all the previous and nexts for the nearby buttons
    if (oldbutton->GetPrev())
      oldbutton->GetPrev()->SetNext(oldbutton->GetNext());
    else
	Buttons = oldbutton->GetNext();
    
    if (oldbutton->GetNext())
	oldbutton->GetNext()->SetPrev(oldbutton->GetPrev());
    //actually remove the button from the toolbar
    DeleteItem (oldbutton);
}

//takes off scroll buttons
void ToolBar::removeScrollButtons()
{
    if (ScrollButtonState == 1)
    {
	//adjusts the tool bar size
	vrect myrect;
	myrect.Copy (GetRect());
	myrect.SetX(0);
	myrect.SetWidth(myrect.GetWidth()+(2*BUTTONWIDTH));
	SetRect (&myrect);
	
	//acutally remove the scroll buttons
	DetermineDialog()->DeleteItem(LeftScrollButton);
	DetermineDialog()->DeleteItem(RightScrollButton);
	//delete LeftScrollButton;
	//delete RightScrollButton;
	LeftScrollButton = NULL;
	RightScrollButton = NULL;
	ScrollButtonState = 0;
    }
}

//adds scrollbuttons
void ToolBar::addScrollButtons()
{
    if (ScrollButtonState == 0)
    {
	ScrollButtonState = 1;
	LeftScrollButton = new ScrollButton;
	RightScrollButton = new ScrollButton;
	LeftScrollButton->initialize (0, this);
	RightScrollButton->initialize (1, this);
	
	//adjust the toolbar size
	vrect myrect;
	myrect.Copy(GetRect());
	myrect.SetX(BUTTONWIDTH);
	myrect.SetWidth(myrect.GetWidth()-(2*BUTTONWIDTH));
	SetRect (&myrect);
	
	for (ToolBarButton * currentbutton = Buttons; currentbutton && currentbutton->GetNext() &&currentbutton->GetX()+(SCROLL_WIDTH * BUTTONWIDTH) < GetWidth(); currentbutton = currentbutton->GetNext());
	
	RightmostShown = currentbutton;
	
	LeftmostShown = Buttons;
    }
}

//used when customizing, incase the user wants to cancel his or her changes

void ToolBar::BufferButtons()
{
    ToolBarButton * currentButton = Buttons, *currentBufferButton = NULL, *prevBufferButton = NULL;

    //incase there are no buttons
	ButtonsBuffer = NULL;
    
    //goes throught buttons and copies all the attributes
	while (currentButton)
	{
	    currentBufferButton = currentButton->gCopy();
	    currentBufferButton->SetState(2);
	    vrect * buttonrect = new vrect;
	    buttonrect->Copy (currentButton->GetRect());
	    currentBufferButton->SetRect(buttonrect);
	    currentBufferButton->SetNext(NULL);
	    if (prevBufferButton)
	    {
		currentBufferButton->SetPrev (prevBufferButton);
		prevBufferButton->SetNext (currentBufferButton);
	    }
	    else
	    {
		currentBufferButton->SetPrev(NULL);
		ButtonsBuffer = currentBufferButton;
	    }
	    prevBufferButton = currentBufferButton;
	    currentButton = currentButton->GetNext();
	    //delete buttonrect;
	}
}    

//if the user cancels his customize changes, puts the old buttons back in place
void ToolBar::UnBufferButtons()
{
    RemoveAllButtons();
    Buttons = ButtonsBuffer;
    int i = 1;
    for (ToolBarButton * currentButton = Buttons; currentButton; currentButton = currentButton->GetNext(), i++);

    if ((i+SCROLL_WIDTH)*BUTTONWIDTH > GetWidth())
    {
	if (ScrollButtonState == 0)
	    addScrollButtons();
    }
    else
    {
	if (ScrollButtonState == 1)
	    removeScrollButtons();
    }
    AddAllButtons();
    ButtonsBuffer = NULL;
}


//if the user accepts his changes, deletes the buffer

void ToolBar::DeleteBufferedButtons()
{
    ToolBarButton * currentButton = ButtonsBuffer, *nextButton = ButtonsBuffer;
    while (nextButton)
    {
	nextButton = currentButton->GetNext();
	//delete currentButton;
	currentButton = nextButton;
    }
    ButtonsBuffer = NULL;
}
    
//removes all the buttons from the tool bar
void ToolBar::RemoveAllButtons()
{
    ToolBarButton * currentButton = Buttons, *prevButton = Buttons;
    while (currentButton)
    {
	DeleteItem (currentButton);
	prevButton = currentButton;
	currentButton = currentButton->GetNext();
	//delete prevButton;
    }
    LeftmostShown = NULL;
    RightmostShown = NULL;
    Buttons = NULL;
}

//visually adds all the buttons pointed to by Buttons to the tool bar
void ToolBar::AddAllButtons()
{
    ToolBarButton * currentButton = Buttons;
    LeftmostShown = currentButton;
    RightmostShown = NULL;
    while (currentButton)
    {
	if (currentButton->GetX()> GetWidth())
	    RightmostShown = currentButton->GetPrev();
	AppendItem (currentButton);
	currentButton = currentButton->GetNext();
    }
    if (!RightmostShown)
    {
	for (currentButton = Buttons; currentButton && currentButton->GetNext(); currentButton = currentButton->GetNext());
	RightmostShown = currentButton;
    }
}

//saves the buttons into the preference file
void ToolBar::SaveButtons()
{
    ToolBarButton * currentButton = Buttons;
    vliststr *      buttonList = new vliststr;
    int rownum = 0;

    while (currentButton) {
	currentButton->Save(buttonList, rownum);
	currentButton = currentButton->GetNext();
	rownum++;
    }

    vresource toolBarRes = vapplication::GetCurrent()->GetPreferences();
    toolBarRes = vresourceCreate (toolBarRes, vnameInternGlobalLiteral ("ToolBar"), vresourceDICT) ;
    toolBarRes = vresourceCreate (toolBarRes, vnameInternGlobal (ViewerStateName), vresourceUNTYPED);
    buttonList->Store (toolBarRes);

#if 0
    vresource toolBarRes;
    vstr * resName = vstrCopyMulti(vnameInternGlobalLiteral("ToolBar:"),
                     ViewerStateName, NULL, NULL);
    if (getResource(resName, &toolBarRes) ) {
        buttonList->Store(toolBarRes);
    }
    delete buttonList;
#endif
}



//loads the buttons in from memory
void ToolBar::SetViewerState(const vchar * newState)
{
    if (vcharCompare(ViewerStateName, newState) != 0) {
        for (int i=0; i<NumViewerStates; i++) {
            if (vcharCompare(ViewerStateList[i], newState) == 0) {
                //bufferButton = ViewerStateButtons[i];
                //ViewerStateButtons[i] = Buttons;
                HideButtonSet();
                ViewerStateIndex = i;
                ViewerStateName = ViewerStateList[ViewerStateIndex];
                ShowButtonSet();
                break;
            }
        }
    }
}



void ToolBar::ShowButtonSet ()
{
	
    Buttons = ViewerStateButtons[ViewerStateIndex];
    LeftmostShown = Buttons;
    RightmostShown = NULL;
    
    
    vrect currentrect;
    currentrect.SetX(0);
    currentrect.SetY(0);
    currentrect.SetHeight(BUTTONHEIGHT);
    currentrect.SetWidth(BUTTONWIDTH);
    
    ToolBarButton * currentButton = Buttons;
    ToolBarButton * prevButton = NULL;
    while (currentButton) {
	currentButton->SetRect(&currentrect);
	if (currentButton->GetX() > GetWidth() && 
            (!prevButton || 
            (prevButton && prevButton->GetX() < GetWidth())) )
	    RightmostShown = prevButton;
	AppendItem(currentButton);
        currentButton->Show();
        currentButton->Enable();
	currentrect.SetX(currentrect.GetX()+BUTTONWIDTH);
	prevButton = currentButton;
	currentButton = currentButton->GetNext();
    }
    
    // Update the right-most button.
    if (!RightmostShown)
	RightmostShown = currentButton;

    // Check to see if scroll buttons are neccessary.
    int i = 0;
    for (ToolBarButton * currentbutton = Buttons;
                         currentbutton;
                         currentbutton = currentbutton->GetNext() )
        i++;

    if (((i+SCROLL_WIDTH) * BUTTONWIDTH) > GetWidth()) 
	addScrollButtons();
    else
	ScrollButtonState = 0;
}    


void ToolBar::HideButtonSet()
{
    for (ToolBarButton * currentButton = Buttons;
                         currentButton;
                         currentButton = currentButton->GetNext() ) {
        currentButton->Hide();
        currentButton->Disable();
	DeleteItem(currentButton);
    }

    Buttons = NULL;
    if (ScrollButtonState == 1)
	removeScrollButtons();
}



void ToolBar::LoadButtons()
{
    Buttons = NULL;
    LeftmostShown = NULL;
    RightmostShown = NULL;

    vresource toolBarRes;
    if (getResource("ToolBar", &toolBarRes)) {
        for (int j = 0; j < NumViewerStates; j++) {
            ViewerStateButtons[j] = NULL;
            vliststr * buttonList;
            vresource listRes;
            if (getResource(toolBarRes, ViewerStateList[j], &listRes) ) {
                buttonList = new vliststr(listRes);
	
                Buttons = NULL;
                LeftmostShown = NULL;
                RightmostShown = NULL;
	
                ToolBarButton * currentButton = NULL;
                ToolBarButton * prevButton = NULL;
	
                int rownum = 0;
                int rowcount = buttonList->GetRowCount();
                
                vrect currentrect;
                currentrect.SetX(0);
                currentrect.SetY(0);
                currentrect.SetHeight(BUTTONHEIGHT);
                currentrect.SetWidth (BUTTONWIDTH);
                
                while (rownum < rowcount) {
                    ViewerMenu * menuList = NULL;
                    if (GetViewer() && GetViewer()->GetMenuBar())
                        menuList = GetViewer()->GetMenuBar()->GetMenuList();
                    else {
                        printf("@ToolBar::LoadButtons:No Menu List found!!!\n");
                        break;
                    }
                    currentButton = new ToolBarButton;
                    if (!currentButton->initialize(vnameInternGlobal( (const vchar *)
                                        buttonList->GetCellValue(rownum, 0)),
                                        this, menuList) ) {
                	printf("@ToolBar::LoadButtons:");
                        printf("missing button (%d,0)=\"%s\"\n", rownum, buttonList->GetCellValue(rownum, 0));
                        break;
                    }
                    
                    currentButton->SetRect(&currentrect);

                    if (rownum == 0) {
                	Buttons = currentButton;
                	LeftmostShown = currentButton;
                    }
                    if (currentButton->GetX() > GetWidth() && 
                       (!prevButton || prevButton->GetX() < GetWidth()) )
                	RightmostShown = prevButton;
                    currentButton->SetPrev(prevButton);
                    if (prevButton)  
                               prevButton->SetNext(currentButton);
                    prevButton = currentButton;
                    currentButton->SetState(0);
                    currentrect.SetX(currentrect.GetX()+BUTTONWIDTH);
                    rownum++;
                }
                
                // If we don't need scroll buttons, then put rightmost button next.
                if (!RightmostShown)
                    RightmostShown = currentButton;

                //check to see if scroll buttons are neccessary
                int i = 0;
                for (ToolBarButton * currentbutton = Buttons;
                             currentbutton;
                             currentbutton = currentbutton->GetNext()
                           ) i++;
                if (((i+SCROLL_WIDTH) * BUTTONWIDTH) > GetWidth()) 
                    addScrollButtons();
                else
                    ScrollButtonState = 0;

                ViewerStateButtons[j] = Buttons;

                delete buttonList;
            } else {
                printf("@ToolBar::LoadButtons:Unable to load list for '%s'\n",
                    ViewerStateList[j]);
            }
        }
    }
}


//resize function called viewer::resize 
void ToolBar::Resize (int newWidth, int newY)
{
    //change the tool bar rect
    vrect newrect;
    newrect.Copy (GetRect());
    newrect.SetY (newY);
    
    if (ScrollButtonState == 0)
	newrect.SetWidth (newWidth);
    else
	newrect.SetWidth (newWidth - (2*BUTTONWIDTH));
    SetRect(&newrect);

    if (Buttons == NULL)
	return;
    
    //finds the number of buttons, and a pointer to the last button
    
    int i = 0;
    for (ToolBarButton * currentButton = Buttons; currentButton && currentButton->GetNext(); currentButton = currentButton->GetNext())
	i++;
    
    //checks the new size and the state of the scroll buttons, and adjusts accordingly
    
    if ((i+SCROLL_WIDTH) * BUTTONWIDTH < newWidth)
    {
	if (ScrollButtonState == 1)
	{
	    removeScrollButtons();
	    if (Buttons->GetX() != 0)
	    {
		vrect currentButtonrect;
		currentButtonrect.Copy(Buttons->GetRect());
		currentButtonrect.SetX(0);
		for (currentButton = Buttons; currentButton; currentButton = currentButton->GetNext())
		{
		    currentButton->SetRect (&currentButtonrect);
		    currentButtonrect.SetX (currentButtonrect.GetX() + BUTTONWIDTH);
		}
	    }
	}
    }
    else
    {
	if (ScrollButtonState == 0)
	    addScrollButtons();
	else
	{
	    vrect newScrollButtonRect;
	    newScrollButtonRect.Copy (RightScrollButton->GetRect());
	    newScrollButtonRect.SetX (newWidth - BUTTONWIDTH);
	    newScrollButtonRect.SetY(newY);
	    RightScrollButton->SetRect (&newScrollButtonRect);
	    
	    newScrollButtonRect.Copy (LeftScrollButton->GetRect());
	    newScrollButtonRect.SetY (newY);
	    LeftScrollButton->SetRect (&newScrollButtonRect);
	}
	if (currentButton->GetX() + BUTTONWIDTH < GetWidth())
	{
	    RightmostShown = currentButton;
	    vrect currentButtonrect;
	    currentButtonrect.Copy (currentButton->GetRect());
	    currentButtonrect.SetX(GetWidth() - BUTTONWIDTH);
	    while (currentButton)
	    {
		currentButton->SetRect(&currentButtonrect);
		currentButtonrect.SetX(currentButtonrect.GetX() - BUTTONWIDTH);
		if (currentButton->GetX() <= 0 && currentButton->GetNext()->GetX() >= 0)
		    LeftmostShown = currentButton->GetNext();
		currentButton = currentButton->GetPrev();
	    }
	}
	else
	{
	    while (currentButton && currentButton->GetX()+(SCROLL_WIDTH*BUTTONWIDTH) > GetWidth())
		currentButton = currentButton->GetPrev();
	    RightmostShown = currentButton;
	}
    }
}


Viewer *ToolBar::GetViewer()
{
    return myViewer;
}

void ToolBar::UpdateColors()
{
#ifdef OWNCOLOR
    SetForeground (Viewer::GetToolBarForeground());
    SetBackground (Viewer::GetToolBarBackground());
#endif

    if (LeftScrollButton)
    {
#ifdef OWNCOLOR
	LeftScrollButton->SetForeground (Viewer::GetToolBarForeground());
	LeftScrollButton->SetBackground (Viewer::GetToolBarBackground());
	RightScrollButton->SetForeground (Viewer::GetToolBarForeground());
	RightScrollButton->SetBackground (Viewer::GetToolBarBackground());
#endif
    }
}

int ToolBar::GetHeight ()
{
    return vcontainer::GetHeight();
}


//***************ScrollButton*******************************

vkindTYPED_FULL_DEFINITION (ScrollButton, vbutton, "ScrollButton");

//sets it's rect and appends itself to the dialog

void ScrollButton::initialize(int newSide, ToolBar * newToolBar)
{

  Side = newSide;
  myToolBar = newToolBar;

  timer = new ScrollButtonTimer;

  vrect bufferRect;
  bufferRect.SetWidth (BUTTONWIDTH);
  bufferRect.SetHeight (myToolBar->GetHeight());
  bufferRect.SetY(myToolBar->GetY());

  if (Side == 0) {
    bufferRect.SetX(0);
    SetTitleScribed (vcharScribeLiteral ("<<"));
  }
  else {
    bufferRect.SetX(myToolBar->GetWidth() - BUTTONWIDTH);
    SetTitleScribed (vcharScribeLiteral (">>"));
  }

  SetRect (&bufferRect);

  myToolBar->DetermineDialog()->AppendItem(this);

#ifdef OWNCOLOR
  SetForeground (Viewer::GetToolBarForeground());
  SetBackground (Viewer::GetToolBarBackground());
#endif
}


void ScrollButton::ActivateButton ()
{
    if (Side == 0)
	myToolBar->ScrollLeft();
    else
	myToolBar->ScrollRight();
}

int ScrollButton::HandleButtonDown(vevent* event)
{
    vbutton::HandleButtonDown(event);

    if (!timer)
	timer = new ScrollButtonTimer;

    timer->SetOwner(this);
    timer->Start();

    ActivateButton();

    return 1;
}


int ScrollButton::HandleButtonUp(vevent* event)
{
    vbutton::HandleButtonUp(event);

    if (timer)
	timer->Stop();

    return 1;
}


void ScrollButton::DisableButton()
{
	Disable();
}

void ScrollButton::EnableButton()
{
	Enable();
}


void ScrollButton::Destroy (void)
{
    if (timer)
        delete timer;

    vbutton::Destroy ();
}

//**********************ScrollButtonTimer****************************


void ScrollButtonTimer::SetOwner(ScrollButton* newowner)
{
    SetRecurrent();
    SetPeriod(SECONDS, NANOSECONDS);
    owner = newowner;
    startTime = vtimestampGetNow();
    speedUpTime = vtimestampAdd(startTime, WAIT_SECONDS, WAIT_NANOSECONDS);
    hasAccelerated = vFALSE;
}


void ScrollButtonTimer::ObserveTimer()
{
    if (!hasAccelerated)
	if (vtimestampCompare(vtimestampGetNow(), speedUpTime) >= 0)
	    SetPeriod(FAST_SECONDS, FAST_NANOSECONDS);
    owner->ActivateButton();
}


//****************ToolBarDialog*******************************

vkindTYPED_FULL_DEFINITION (ToolBarDialog, vdialog, "ToolBarDialog");



// Subloads all its contained objects, loads the buttons, 
// buffers the current button list in case cancel is picked,
// and then sets the toolbar state to modifiable.

void ToolBarDialog::initialize(ToolBar * toolBar)
{
    ToolBarListItem * newlistview  = ToolBarListItem::RequestSubLoad (this, 
                      vnameInternGlobalLiteral("ToolBarList"));
    
    ToolBarDialogSaveButton * saveButton = ToolBarDialogSaveButton::
                              RequestSubLoad(this,
                              vnameInternGlobalLiteral ("SaveButton"));
    saveButton->initialize(toolBar);

    ToolBarDialogCancelButton * cancelButton = ToolBarDialogCancelButton::
                                RequestSubLoad(this,
                                vnameInternGlobalLiteral ("CancelButton"));
    cancelButton->initialize (toolBar);

    ToolBarDialogLocalButton* localButton = ToolBarDialogLocalButton::
                              RequestSubLoad(this,
                              vnameInternGlobalLiteral ("UseLocalButton"));
    localButton->initialize (toolBar);

    vresource res;
    if (getResource("ToolBar:ToolBarDialog", &res) ) {
        Load (res);
        Open();
        LoadButtons(toolBar->GetViewer()->GetMenuBar()->GetMenuList(),
                    toolBar, NULL);

        toolBar->BufferButtons();
        toolBar->SetState (2);

        //Set Colors
#ifdef OWNCOLOR
        SetForeground(Viewer::GetDialogForeground());
        SetBackground(Viewer::GetDialogBackground());
#endif
    }
}

// Recursive function to get through all the menus and 
// load all the buttons as different groups according to submenus etc.

void ToolBarDialog::LoadButtons(ViewerMenu * currentMenu, ToolBar *toolBar,
                                vchar *indention)
{
    vchar indentionBuffer [1000];

    /*generic container is used only for rect and allow the rects of all the button containers to be 
      changed by changing this on in the vre*/
    
    vcontainer * genericContainer = (vcontainer *)FindItem (vnameInternGlobalLiteral("GenericPane"));
    genericContainer->Hide();
    genericContainer->Disable();
    
    //find the listview object in the dialog, to put the list of all the possible buttons groups in
    
    vlistitem * groupList = ((vlistitem *)FindItem(vnameInternGlobalLiteral("ToolBarList")));

    //create a new list for the listview to put all the choices in
    
    groupList->CreateList();

    while (currentMenu)
    {
	/*the history menu should not show up, because it is constantly changing and wouldn't make sense to 
	have buttons for it*/

	if (vcharCompare (currentMenu->GetTag(), vnameInternGlobalLiteral ("HistoryMenu")) == 0)
	{
	    currentMenu = currentMenu->GetNext();
	    continue;
	}
	
	//creates a new container to put this group of buttons in
	
	vcontainer * newcontainer = new vcontainer;
	newcontainer->SetRect(genericContainer->GetRect());

	//submenus will have indentions of n amount of spaces depending on how deep a submenu they are
	
	if (indention)
	{
	    vcharCopyFast ((const vchar *)indention, indentionBuffer);
	    vcharAppendFast (indentionBuffer, currentMenu->GetTitle());
	}
	else
	    vcharCopyFast (currentMenu->GetTitle(), indentionBuffer);
	newcontainer->SetTag (vnameInternGlobal(indentionBuffer));
	newcontainer->Hide();
	newcontainer->Disable();
	AppendItem(newcontainer);
	vscribe * indentionBufferScribed = vcharScribe (indentionBuffer);
	groupList->AppendScribed (indentionBufferScribed);
	
	
	ToolBarButton * currentButton, * prevButton;
	currentButton = prevButton = NULL;
	
	vrect * buttonRect = new vrect;
	buttonRect->SetHeight (BUTTONHEIGHT);
	buttonRect->SetWidth (BUTTONWIDTH);
	buttonRect->SetX (0);
	buttonRect->SetY (newcontainer->GetHeight() - BUTTONHEIGHT);
	
	ViewerMenuItemList * currentMenuListItem = currentMenu->GetMenuItemList();

	//goes through all the items in the menu adding them unless they have submenus or are just seporator items
	
	while (currentMenuListItem) {
	    vmenuItem * currentMenuItem = currentMenuListItem->GetMenuItem();

	    //check for separator items
	    
	    if (vcharCompare (currentMenuItem->GetTitle(), (vchar *)"-") != 0)
	    {
		ViewerMenu * newMenu;

		//check to see if it has a submenu
		
		if (newMenu = currentMenuListItem->GetSubMenu())
		{
		    //adds one more set of spaces to the indention and then loads the submenu
		    
		    if (indention) {
			vcharCopyFast((const vchar *) indention,
                                      indentionBuffer);
			vcharAppendFromLiteral (indentionBuffer, "   ");
		    } else
			vcharCopyFromLiteral("   ", indentionBuffer);

		    LoadButtons(newMenu, toolBar, indentionBuffer);
		} else {
		    //loading proc for a normal menu item
		    currentButton = new ToolBarButton;
		    vimage* myicon = NULL;
		    vresource iconRes, currentIconRes;

#ifdef WIN32
			if (getResource("ToolBar:NTIcons", &iconRes)){
				if (getResource(iconRes, currentMenuItem->GetTag(), &currentIconRes))
					myicon = new vimage(currentIconRes);
			}
#endif 
			if (!myicon && getResource("ToolBar:Icons", &iconRes)) {
				if (getResource(iconRes, currentMenuItem->GetTag(),	&currentIconRes))
					myicon = new vimage(currentIconRes);
			}

		    currentButton->initialize(currentMenuItem->GetTag(),
					      currentMenuItem->GetTitle(), 
					      ((ViewerMenuItem *)
                                              currentMenuItem)->
                                              GetTcl_Command(),
                                              toolBar, this, myicon );
		    newcontainer->AppendItem(currentButton);
		    if (prevButton == NULL)
			currentButton->SetRect(buttonRect);
		    else {
			// Moves the button rect over each time  
                        // or down a row if it is at the edge of the container
			if (buttonRect->GetX() < (
                           newcontainer->GetWidth()-(2*BUTTONWIDTH))) {
			    buttonRect->SetX(buttonRect->GetX() +
                                             BUTTONWIDTH);
			    currentButton->SetRect (buttonRect);
			} else {
			    buttonRect->SetY(buttonRect->GetY() -
                                             BUTTONHEIGHT);
			    buttonRect->SetX(0);
			    currentButton->SetRect(buttonRect);
			}
		    }
		    prevButton = currentButton;
		}
	    }
	    currentMenuListItem = currentMenuListItem->GetNext();
	}
	delete buttonRect;
	currentMenu = currentMenu->GetNext();
    }  
    
    // Show the current container.
    vcontainer * currentContainer = (vcontainer *) FindItem(
                 vnameInternGlobal(groupList->GetItemValue(
                 groupList->GetSelectedItem())));
    if (currentContainer) {
	currentContainer->Show();
	currentContainer->Enable();
    } else
	printf("Can't find that container %S\n",
              (char *)groupList->GetItemValue (groupList->GetSelectedItem()));
}
  

//****************ToolBarDialogButtons******************************

vkindTYPED_FULL_DEFINITION (ToolBarDialogSaveButton, vbutton, "ToolBarDailogSaveButton");

//button for saveing the changes to the toolbar

void ToolBarDialogSaveButton::initialize (ToolBar * newToolBar)
{
    myToolBar = newToolBar;
}


int ToolBarDialogSaveButton::HandleButtonDown (vevent *event)
{
    myToolBar->SaveButtons();
    myToolBar->DeleteBufferedButtons();
    DetermineDialog()->Close();
    myToolBar->SetState (0);
    return vTRUE;
}

vkindTYPED_FULL_DEFINITION (ToolBarDialogLocalButton, vbutton, "ToolBarDialogSaveButton");

//button for using the changed toolbar only on this session of discover

void ToolBarDialogLocalButton::initialize (ToolBar * newToolBar)
{
    myToolBar = newToolBar;
}


int ToolBarDialogLocalButton::HandleButtonDown (vevent *event)
{
    myToolBar->DeleteBufferedButtons();
    DetermineDialog()->Close();
    myToolBar->SetState (0);
    return vTRUE;
}

vkindTYPED_FULL_DEFINITION (ToolBarDialogCancelButton, vbutton, "ToolBarDialogCancelButton");

//button for canceling any changes made

void ToolBarDialogCancelButton::initialize (ToolBar * newToolBar)
{
    myToolBar = newToolBar;
}


//gets the buttons that where buffered when the dialog initialized, and deletes everything else

int ToolBarDialogCancelButton::HandleButtonDown (vevent *event)
{
    myToolBar->UnBufferButtons();
    DetermineDialog()->Close();
    myToolBar->SetState (0);
    return vTRUE;
}



//*********ToolBarButton********************

vkindTYPED_FULL_DEFINITION (ToolBarButton, vbutton, "ToolBarButton");

//just initializes all it's privates from an outside source with everything passed in

void ToolBarButton::initialize(const vname * newTag, const vchar * newtitle,
                               const vchar * newtcl_command,
                               ToolBar * newToolBar, ToolBarDialog * newDialog,
                               vimage * newimage)
{
    SetTag (newTag);
    if (newimage)
	SetImage(newimage);
    else
	SetTitle (newtitle);

    tip = (vchar*) vmemAlloc (vcharLength(newtitle)+1);
    vcharCopyFast (newtitle, tip);

    tcl_Command = (vchar *) vmemAlloc (vcharLength (newtcl_command)*sizeof(vchar)+1);
    vcharCopyFast (newtcl_command, tcl_Command);
    myToolBar = newToolBar;
    next = NULL;
    prev = NULL;
    State = 1;
    myDialog = newDialog;
}

// Initialize the button using its tag to find the equivalent menu item and
// then initializes itself from there.

int ToolBarButton::initialize(const vname *newTag, ToolBar * newToolBar,
                              ViewerMenu * currentMenu)
{
    while (currentMenu) {
#if 0
	if (vcharCompare(currentMenu->GetTag(),
                         vnameInternGlobalLiteral("HistoryMenu")) == 0) {
	    currentMenu = currentMenu->GetNext();
	    continue;
	}
#endif

	ViewerMenuItemList * currentMenuListItem = currentMenu->GetMenuItemList();
	while (currentMenuListItem) {
	    vmenuItem * currentMenuItem = currentMenuListItem->GetMenuItem();
	    ViewerMenu * newMenu;
	    if (newMenu = currentMenuListItem->GetSubMenu())
	    {
		if (initialize (newTag, newToolBar, newMenu))
		    return 1;
	    }
	    else if (currentMenuItem->GetTag() && vcharCompare (newTag, currentMenuItem->GetTag()) == 0)
	    {
		SetTag(newTag);
		tcl_Command = (vchar *) vmemAlloc (vcharLength (((ViewerMenuItem *)currentMenuItem)->GetTcl_Command())*sizeof(vchar)+1);
		contents_copy = ((ViewerMenuItem *)currentMenuItem)->GetArgList()->GetArg((vchar*) "qhelp");
		vcharCopyFast (((ViewerMenuItem *)currentMenuItem)->GetTcl_Command(), tcl_Command);
		myToolBar = newToolBar;
		next = NULL;
		prev = NULL;
		State = 1;
		myDialog = NULL;

		vimage* myicon = NULL;
		vresource iconRes;
		vresource currentIconRes;
#ifdef WIN32
		if (getResource("ToolBar:NTIcons", &iconRes) ) {
            if (getResource(iconRes, currentMenuItem->GetTag(), &currentIconRes))
                myicon = new vimage(currentIconRes);
		}
#endif
		if (!myicon && getResource("ToolBar:Icons", &iconRes) ) {
            if (getResource(iconRes, currentMenuItem->GetTag(), &currentIconRes))
                myicon = new vimage(currentIconRes);
		}
		if (myicon)
		    SetImageOwned(myicon);
		else
		    SetTitle(currentMenuItem->GetTitle());
	        const vchar* newtitle = currentMenuItem->GetTitle();
    		tip = (vchar*) vmemAlloc (vcharLength(newtitle)+1);
                vcharCopyFast (newtitle, tip);

		return 1;
	    }
	    currentMenuListItem = currentMenuListItem->GetNext();
	}
	currentMenu = currentMenu->GetNext();
    }

    // It went through all the menus and could not find the button.
    myDialog = NULL;
    myToolBar = NULL;
    next = NULL;
    prev = NULL;
    tcl_Command = NULL;
    State = 0;

    Disable();
    Hide ();

    return 0;
}


// Destroy a Tool Bar Button.
void ToolBarButton::Destroy() 
{
    if (tcl_Command)
        vmemFree(tcl_Command);
    vbutton::Destroy();
}






//makes a copy of the button, used to buffer the button

ToolBarButton * ToolBarButton::gCopy()
{
  ToolBarButton * newToolBarButton = new ToolBarButton;
  vimage * newimage = NULL;
  vimage * oldimage = GetImage();
  if (oldimage) {
      newimage = new vimage;
      newimage->Copy (oldimage);
  }
  newToolBarButton->initialize (GetTag(), GetTitle(), tcl_Command, myToolBar, myDialog, newimage);
  return newToolBarButton;
}

//calls the Tcl command if it is in normal state

void ToolBarButton::ObserveDialogItem (vevent * event)
{
    if (State == 0) {
	int idCode = myToolBar->GetViewer()->GetViewCode();
	View *v = myToolBar->GetViewer()->find_view(idCode);
	if (v) {
	    int code = gala_eval (v->interp(), (char*)tcl_Command);

		// Because the viewer might have been deleted (remove view
		// command) look for the view again.
	        v = myToolBar->GetViewer()->find_view(idCode);
	}
    }
}


/*if the button is in the drag state, either in the tool bar or the tool bar dialog, it drags its and 
  exibts the correct behavior according to it starting place/state*/

int ToolBarButton::HandleButtonDown(vevent * event)
{
    if (event->GetBinding() != vname_Select || State == 0) {
//	myToolBar->GetViewer()->DisplayQuickHelp(contents_copy);
	return vbutton::HandleButtonDown(event);
    }

    if (State == 1)
	myDialog->FindItem (vnameInternGlobalLiteral ("QuickHelp"))->SetTitle (tcl_Command);
    
    const vrect *r = GetRect();
    
    ToolBarButtonDrag *drag = new ToolBarButtonDrag;
    
    drag->SetButton (this);
    
    /*Maxsize is the area that gets dragged around, in case you want to drag
      an object and 10 pixels on each side or something*/
    drag->SetMaxSize(r->GetWidth(), r->GetHeight());
    
    /*Offset dictates what the drag object gets drawn relative to the cursor 
      position*/
    drag->SetOffset(event->GetX() - r->GetX(), event->GetY() - r->GetY());
    drag->Start(event);
    return 1;
}


ToolBarButton * ToolBarButton::GetNext()
{
  return next;
}

void ToolBarButton::SetNext (ToolBarButton * newButton)
{
  next = newButton;
}

void ToolBarButton::SetPrev (ToolBarButton *newButton)
{
  prev = newButton;
}

ToolBarButton* ToolBarButton::GetPrev ()
{
  return prev;
}
ToolBar * ToolBarButton::GetToolBar()
{
  return myToolBar;
}

void ToolBarButton::SetState (int newState)
{
  State = newState;
}

int ToolBarButton::GetState ()
{
  return State;
}

//saves it's tag and it's title (which will at one point instead be the name of its icon) and its Tcl command (for custom buttons)

void ToolBarButton::Save (vliststr * buttonList, int rownum)
{
    buttonList->SetCellValueScribed (rownum, 0, vnameScribeGlobal(GetTag()));
    buttonList->SetCellValueScribed (rownum, 1, vcharScribe (GetTitle()));
    buttonList->SetCellValueScribed (rownum, 2, vcharScribe (tcl_Command));
}

void ToolBarButton::Hilite () 		// ( const vrect* bounds, const vrect* content)
{
    myToolBar->GetViewer()->DisplayQuickHelp(contents_copy);
    vbutton::Hilite(); 			// (bounds, content);
}

void ToolBarButton::Unhilite () 	// ( const vrect* bounds, const vrect* content)
{
    myToolBar->GetViewer()->DisplayQuickHelp((vchar*) " ");
    vbutton::Unhilite(); 		// (bounds, content);
}


//***************ToolBarListItem******************************

vkindTYPED_FULL_DEFINITION (ToolBarListItem, vlistitem, "ToolBarListItem");


//changes the container that is shown as you scroll up and down the list

void ToolBarListItem::ObserveSelect (int state)
{
    if (state == vlistviewSELECTION_IS_CHANGING || state == vlistviewSELECTION_WILL_CHANGE)
    {
      if (GetSelectedItem() != vlistitemNO_CELLS_SELECTED)
      {
	  const vname * oldcurrentcontainerName = vnameInternGlobal (GetItemValue (GetSelectedItem()));
	  const vname * newcurrentcontainerName = vnameInternGlobal (GetItemValue (GetVolatileSelectedItem()));
	  
	  vcontainer * oldcurrentContainer = (vcontainer *) DetermineDialog()->FindItem (vnameInternGlobal (oldcurrentcontainerName));

	  if (oldcurrentContainer)
	  {
	      oldcurrentContainer->Hide();
	      oldcurrentContainer->Disable();
	      printf ("Hiding container %s\n", (char *)oldcurrentContainer->GetTag());
	  }
	  else
	      printf ("Can't find container to hide\n");

	  vcontainer * newcurrentContainer = (vcontainer *) DetermineDialog()->FindItem (vnameInternGlobal (newcurrentcontainerName));
	  if (newcurrentContainer)
	  {
	      newcurrentContainer->Show();
	      newcurrentContainer->Enable();
	      DetermineDialog()->SetItemIndex (newcurrentContainer, 0);
	      printf ("Showing container %s\n", (char *)newcurrentContainer->GetTag());
	  }
	  else
	      printf ("Can't find container to show\n");
      }
  }
}


void ToolBarListItem::initialize ()
{
}


//***************ToolBarButtonDrag******************************


vkindTYPED_FULL_DEFINITION (ToolBarButtonDrag, vdrag, "ToolBarButtonDrag");

//draws the button as a rect as it is being draged

void ToolBarButtonDrag::Draw (vrect *rect)
{
  gInit (ToolBarButtonDrag::Draw);
  vdraw::GSave();
  vdraw::SetLineWidth(3);
  vdraw::SetColor(vcolor::GetWhite());
  vdraw::RectStrokeInside(rect->GetX(), rect->GetY(), rect->GetWidth(), rect->GetHeight());
  vdraw::GRestore(); 
}

/*if the button is draged to the tool bar it it placed in it, if it is draged anywhere else, 
and was in the tool bar, it is removed from it, otherwise, nothing is done*/

void ToolBarButtonDrag::ObserveDrag (vwindow * frm, vwindow * to, vevent *event)
{
    if (myButton->GetToolBar()->GetRect()->ContainsPoint (event->GetX() - GetOffsetX(), event->GetY() - GetOffsetY()) ||
      myButton->GetToolBar()->GetRect()->ContainsPoint (event->GetX() - GetOffsetX() + BUTTONWIDTH, event->GetY() - GetOffsetY()) ||
      myButton->GetToolBar()->GetRect()->ContainsPoint (event->GetX() - GetOffsetX(), event->GetY() - GetOffsetY() + BUTTONHEIGHT) ||
      myButton->GetToolBar()->GetRect()->ContainsPoint (event->GetX() - GetOffsetX() + BUTTONWIDTH, event->GetY() - GetOffsetY() + BUTTONHEIGHT))
    {
      if (myButton->GetState() == 2)
	{
	  if (myButton->GetX() - (.5 * BUTTONWIDTH) < event->GetX() - GetOffsetX() && myButton->GetX() + (.5 * BUTTONWIDTH) > event->GetX() - GetOffsetX())
	    return;
	  myButton->GetToolBar()->removeButton (myButton);
	  myButton->GetToolBar()->addButton(myButton, event->GetX() - GetOffsetX(), 1);
	}
      else
	{
	  myButton = myButton->gCopy();
	  myButton->GetToolBar()->addButton(myButton, event->GetX() - GetOffsetX(), 0);
	}
    }
  else
    {
      if (myButton->GetState() == 2)
	  myButton->GetToolBar()->removeButton (myButton);
    }
}

void ToolBarButtonDrag::SetButton (ToolBarButton* Button)
{
  myButton = Button;
}





/**********     end of toolbar.C     **********/
