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
#include <vport.h>
#ifndef vmemINCLUDED
   #include vmemHEADER
#endif
#ifndef vmenuINCLUDED
   #include vmenuHEADER
#endif
#ifndef vcolorchsrINCLUDED
   #include vcolorchsrHEADER
#endif
#ifndef vliststrINCLUDED
   #include vliststrHEADER
#endif
#ifndef vapplicationINCLUDED
   #include vapplicationHEADER
#endif
#ifndef vresourceINCLUDED
   #include vresourceHEADER
#endif
#ifndef vprefINCLUDED
   #include vprefHEADER
#endif
#ifndef vcomboINCLUDED
   #include vcomboHEADER
#endif
#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vwindowINCLUDED
   #include vwindowHEADER
#endif
#ifndef vstdlibINCLUDED
   #include vstdlibHEADER
#endif

#include <ggeneric.h>
#include <gprefs.h>
#include <gviewer.h>
#include <ginterp.h>
#include <gdraw.h>
#include <ggenDialog.h>
#include <gglobalFuncs.h>


//bordercolor constants
const int MAX_LIGHT = 150;
const float DARKNESS = 0.25;
const float LIGHTNESS = 1.5;
const int CURRENT_CONTAINER_BORDER_WIDTH = 1; 
const int TOP_COLOR_SPACING = 15;



const int BORDER = 3;
const int COLOR_SIZE = 20;
const int COLOR_SPACING = 3;


/***********************************************************
 *
 *   The New improved color chooser
 *
 **********************************************************/

vkindTYPED_FULL_DEFINITION (ColorDialog, vdialog, "ColorDialog");

// Init, LoadInit, CopyInit, and Destroy can be changed if needed.
void ColorDialog::Init()
{
    vdialog::Init();
}

void ColorDialog::LoadInit(vresource  res)
{
    vdialog::LoadInit(res);
}

void ColorDialog::CopyInit(vloadable *original)
{
    vdialog::CopyInit(original);
}

void ColorDialog::Destroy()
{
    vdialog::Destroy ();
}

// Loads all object for the Color Chooser.
//     Uses sub-loading to do most of the work.
//     Sub-load is necessary because init procs will crash.
void ColorDialog::initialize ()
{
    OptionContainer * newOptionContainer = OptionContainer::RequestSubLoad(
                      this, vnameInternGlobalLiteral("ObjectOptionContainer"));

    // Prep for ColorChooser load.
    newOptionContainer->preinitialize();    
    
    DefaultContainer = ColorObjectsContainer::RequestSubLoad(this,
                       vnameInternGlobalLiteral("DefaultColorContainer"));
    CustomizeContainer = ColorObjectsContainer::RequestSubLoad(this,
                         vnameInternGlobalLiteral("CustomizeColorContainer"));
    
    CustomizeColorButton * newColorButton = CustomizeColorButton::
                           RequestSubLoad(this, vnameInternGlobalLiteral(
                           "CustomizeButton"));
    
    PaletteChooser * newPaletteChooser = PaletteChooser::RequestSubLoad(this,
                     vnameInternGlobalLiteral("PaletteChooser"));
    
    AttributeContainer * newAttributeContainer = AttributeContainer::
                         RequestSubLoad(this, vnameInternGlobalLiteral(
                         "AttributeContainer"));
    
    newAttributeContainer->preinitialize();
    
    // First find the Chooser Dialog resource.
    vresource dialogRes;
    if (getResource("Colors:ChooserDialog", &dialogRes) ) {
        Load(dialogRes);
    } else {
        printf("ColorDialog::initialize:");
        printf("Unable to load Colors:ChooserDialog resource!!!\n");
        return;
    }
    
    
    Open();
    
    DefaultContainer->initialize(0);
    CustomizeContainer->initialize(1);
    
    newOptionContainer->initialize();
    
    // Set up buttons.
    vbutton * currentButton;
    
    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                    "OkButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::SaveandApplyColors);
    
    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                   "CancelButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::Cancel);
    
    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                    "RevertSchemeButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::RevertScheme);
    
    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                    "NewSchemeButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::NewScheme);
    
    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                    "DeleteSchemeButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::DeleteScheme);

    currentButton = vbutton::CastDown(FindItem(vnameInternGlobalLiteral(
                    "ApplyButton")));
    currentButton->SetObserveDialogItemProc(ColorDialog::Apply);
    
    
    currentContainer = NULL;
    changed = 0;

    //Set Colors
    SetForeground (Viewer::GetDialogForeground());
    SetBackground (Viewer::GetDialogBackground());
}    


//static fuction passed in as the observe function for the cancel button
void ColorDialog::Cancel (vdialogItem * item, vevent * event)
{
    Viewer::InitBufferedColors();
    Viewer::ApplyBufferedColors();
    item->DetermineDialog()->Close();
    Viewer::SetCurrentColorChooser (NULL);
}


void ColorDialog::Apply (vdialogItem * item, vevent *)
{
    ApplyColors (OptionContainer::CastDown (item->DetermineDialog()->FindItem (vnameInternGlobalLiteral ("ObjectOptionContainer"))));
}




// The observe function for the Ok/Save buttons.
void ColorDialog::SaveandApplyColors(vdialogItem * item, vevent * event)
{
    OptionContainer * objectContainer = OptionContainer::CastDown(
	item->DetermineDialog()->FindItem(
	vnameInternGlobalLiteral("ObjectOptionContainer")));
   
    const vname *newSchemeName = vnameInternGlobalScribed(
	PaletteChooser::CastDown(item->DetermineDialog()->FindItem(
	vnameInternGlobalLiteral("PaletteChooser")))->GetTextScribed());
    

    // Can only save to the Preferences file. (~/.vgalaxy.1.vr)
    CreateSchemePref();
    vprefDict * rootPrefDict = vpref::GetRootDict();
    vresource userRes = vapplication::GetCurrent()->GetPreferences();
    if (!getResource(userRes, "Colors:Schems", &userRes) ) {   // Not found.
        printf("@SaveAndApply: No 'Schemes' Preference folder found.\n");
        return;
    }

    printf("@SaveandApplyColors:newSchemeName is '%s'\n", 
        (char *)newSchemeName);

    if (getResource(userRes, newSchemeName, &userRes) ) {
        ApplyColors(objectContainer);
        ColorDialog::Save(userRes, objectContainer);
        ColorObjectsContainer * CurrentCustomizeContainer =
            ColorDialog::CastDown(item->DetermineDialog())->
                                  GetCustomizeContainer();
        // If the custom colors were changed, then recursively go
        // through all the colors and save them.
        if (CurrentCustomizeContainer->GetChanged() == 1) {
            if (getResource(vapplication::GetCurrent()->GetPreferences(),
                            "Colors:DefaultPalette", &userRes) ) {
                vcolorPalette * customizePalette = new vcolorPalette(userRes);
                int i = 0;
                for (ColorContainer *
                     currentcontainer = CurrentCustomizeContainer->
                                        GetFirstContainer();
                     currentcontainer;
                     currentcontainer = currentcontainer->GetNext(), i++) {
                            customizePalette->GetSpecAt(i)->Copy(
                            currentcontainer->GetBackground()->GetSpec());
                }
            customizePalette->Store(userRes);
            } else {
                printf("@SaveandApplyColors:");
                printf("Unable to Colors:DefaultPalette implemented.\n");
            }   
        }
    } else {
        printf("@SaveandApplyColors:Saving new Schemes not implemented.\n");
    }
    

//    vliststr * currentscheme = new vliststr(currentschemeres);
//    currentscheme->SetCellValueScribed(0, 0, vnameScribeGlobal(newSchemeName));
//    currentscheme->Store(currentschemeres);

    //delete currentscheme;

    //closes the dialog
    item->DetermineDialog()->Close();
    Viewer::SetCurrentColorChooser(NULL);
}

// Create the Scheme dict in pref file, if necessary.
void ColorDialog::CreateSchemePref()
{
    
    // Make sure there is a Colors Dictionary in the Preferences file.
    vresource userRes = vapplication::GetCurrent()->GetPreferences();
    vresourceMake(userRes, vnameInternGlobalLiteral("Colors") );
    vprefDict * rootPrefDict = vpref::GetRootDict();
    rootPrefDict->StorePreferences(vnameInternGlobalLiteral("Colors"));
    if (!getResource(userRes, "Colors", &userRes) ) {   // Not found.
        printf("@CreateSchemePref: No 'Colors' Pref folder found.\n");
        return;
    }

    // Make sure CurrentScheme string is in the Colors dictionary.
    vresourceSetString(userRes, vnameInternGlobalLiteral("CurrentScheme"),
                  (vchar *) "CurrentScheme" );
    rootPrefDict->StorePreferences(vnameInternGlobalLiteral("Colors"));

    // Make sure Schemes dictionary is in the Colors dictionary.
    vresourceMake(userRes, vnameInternGlobalLiteral("Schemes") );
    rootPrefDict->StorePreferences(vnameInternGlobalLiteral("Colors"));
}



ColorContainer * ColorDialog::GetSelected()
{
    return currentContainer;
}

void ColorDialog::SetSelected(ColorContainer * newcurrentContainer)
{
    if (newcurrentContainer && newcurrentContainer->GetCustomizeable())
    {
	if (currentContainer && !currentContainer->GetCustomizeable())
	    DefaultContainer->SetSelected(NULL);
	CustomizeContainer->SetSelected (newcurrentContainer);
    }
    else
    {
	if (currentContainer)
	    currentContainer->SetSelected(0);
	DefaultContainer->SetSelected (newcurrentContainer);
    }
    currentContainer = newcurrentContainer;
}


ColorObjectsContainer * ColorDialog::GetDefaultContainer()
{
    return DefaultContainer;
}

ColorObjectsContainer * ColorDialog::GetCustomizeContainer()
{
    return CustomizeContainer;
}

void ColorDialog::NewScheme (vdialogItem * item, vevent *)
{
    gala_eval (g_global_interp, "dis_launch_dialog NewSchemeDialog");
}

void ColorDialog::Save(vresource res, OptionContainer * objectContainer)
{
    ColorToggleButton * currentButton;
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral("Dialog")));
    
    Viewer::SaveDialogColors(currentButton->GetSample()->GetForeground(),
	                      currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral("ToolBar")));
    Viewer::SaveToolBarColors(currentButton->GetSample()->GetForeground(),
	                      currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral("ProgressIndicator")));
    Viewer::SaveProgressColors(currentButton->GetSample()->GetForeground(),
	                       currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral("Menu")));
    Viewer::SaveMenuColors(currentButton->GetSample()->GetForeground(),
	                   currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral("Sash")));
    Viewer::SaveSashColors(currentButton->GetSample()->GetForeground(),
	                   currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral ("Pane")));
    Viewer::SavePaneColors(currentButton->GetSample()->GetForeground(),
	                   currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(vnameInternGlobalLiteral ("CommandLine")));
    Viewer::SaveCommandLineColors(currentButton->GetSample()->GetForeground(),
	                          currentButton->GetSample()->GetBackground(), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(
	vnameInternGlobalLiteral("TargetPaneBorder")));
    Viewer::SaveTargetPaneBorderColor(
	vcolor::Intern(currentButton->GetSample()->GetBorderColor()), res);
    currentButton = ColorToggleButton::CastDown(
	objectContainer->FindItem(
	vnameInternGlobalLiteral("CurrentPaneBorder")));
    Viewer::SaveCurrentPaneBorderColor(
	vcolor::Intern(currentButton->GetSample()->GetBorderColor()), res);
}




void ColorDialog::ApplyColors (OptionContainer * objectContainer)
{
    ColorToggleButton * currentButton;

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral ("Dialog")));
    Viewer::BufferDialogColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral ("ToolBar")));
    Viewer::BufferToolBarColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral ("ProgressIndicator")));
    Viewer::BufferProgressColors(currentButton->GetSample()->GetForeground(),
			         currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("Menu")));
    Viewer::BufferMenuColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("Sash")));
    Viewer::BufferSashColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("Pane")));
    Viewer::BufferPaneColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("CommandLine")));
    Viewer::BufferCommandLineColors(currentButton->GetSample()->GetForeground(),
			currentButton->GetSample()->GetBackground());

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("TargetPaneBorder")));
    Viewer::BufferTargetPaneBorderColor(vcolor::Intern(
			currentButton->GetSample()->GetBorderColor()));

    currentButton = ColorToggleButton::CastDown(
			objectContainer->FindItem(
			vnameInternGlobalLiteral("CurrentPaneBorder")));
    Viewer::BufferCurrentPaneBorderColor(vcolor::Intern(
			currentButton->GetSample()->GetBorderColor()));

    Viewer::ApplyBufferedColors();
}



void ColorDialog::DeleteScheme (vdialogItem * item, vevent *)
{
    PaletteChooser * currentChooser = PaletteChooser::CastDown(
                     item->DetermineDialog()->FindItem(
                     vnameInternGlobalLiteral("PaletteChooser")));
    vlistitem * mylistitem = currentChooser->GetListitem();
    if (mylistitem->GetItemCount() == 1) {
	printf ("Cannot delete the last Scheme\n");
	return;
    }

    vresource res;
    if (getResource("Colors", &res) ) {
        if (getResource(res, "Schemes", &res) ) {
            const vname * schemeName = vnameInternGlobal(
                currentChooser->GetTextAsString() );
            vresourceRemoveIfExists(res, schemeName);
            for (int i = mylistitem->GetItemCount() - 1;
                i >= 0; i--) {
                if (vcharCompare(schemeName,
                    mylistitem->GetItemValue(i)) == 0) {
                    mylistitem->RemoveItem(mylistitem->GetSelectedItem());
                    break;
                }
            }
            if (i == 0) {
                currentChooser->SelectText(mylistitem->GetItemValue(0));
            } else {
                currentChooser->SelectText(mylistitem->GetItemValue(i-1));
            }
        }
    }
}

void ColorDialog::RevertScheme(vdialogItem * item, vevent *)
{
    PaletteChooser * currentChooser = PaletteChooser::CastDown(
                     item->DetermineDialog()->FindItem(
                     vnameInternGlobalLiteral("PaletteChooser")));
    
    const vname * schemeName = vnameInternGlobal(
                  currentChooser->GetTextAsString());

    int foundListRes = 0;
    vresource defaultSchemeRes;
    vresource listRes;
    if (getResource("ColorSchemes", &defaultSchemeRes) ) {
        if (getResource(defaultSchemeRes, schemeName, &defaultSchemeRes) ) {
        vresource res;
        if (getResource("Colors", &res) ) {
            if (getResource(res, "PaletteList", &listRes) ) {
                foundListRes = 1;
                }
            }
        }
    }
    
    if (!foundListRes) return;


    vliststr * colorList = new vliststr(listRes);
    
    OptionContainer * myOptionContainer = (OptionContainer *) item->
                      DetermineDialog()->FindItem(
                      vnameInternGlobalLiteral("ObjectOptionContainer"));
    
    // Go through a string list with the name of all the current objects.
    // Load the colors for each of them.
	
    for (int i= colorList->GetRowCount() - 1; i >= 0; i--) {
	const vname * currentObjectName = vnameInternGlobalScribed(
                      colorList->ScribeCellValue (i, 0) );
	vresource objectRes;
        if (getResource(defaultSchemeRes, currentObjectName, &objectRes) ) {
	    ColorSample * currentObject = ColorToggleButton::CastDown(
                          myOptionContainer->FindItem(currentObjectName)
                          )->GetSample();
            if (currentObject->GetBorderColor()) {
                vcolorSpec * borderColor = new vcolorSpec(objectRes);
                currentObject->SetBorderColor(borderColor);
	        delete borderColor;
            } else {
                vresource colorRes;
                vcolorSpec * currentColorSpec;
                vcolor * currentColor;
                if (getResource(objectRes, "Foreground", &colorRes) ) {
                    currentColorSpec = new vcolorSpec(colorRes);
                    currentColor = vcolor::Intern(currentColorSpec);
                    currentObject->SetForeground(currentColor);
                    delete currentColorSpec;
                }
                if (getResource(objectRes, "Background", &colorRes) ) {
                    currentColorSpec = new vcolorSpec(colorRes);
                    currentColor = vcolor::Intern(currentColorSpec);
                    currentObject->SetBackground(currentColor);
                    delete currentColorSpec;
                }
            }
        }
    }
    delete colorList;
}

void ColorDialog::Change()
{
    changed = 1;
}

int ColorDialog::GetChanged()
{
    return changed;
}



/***********************************OptionContainer***********************************/

vkindTYPED_FULL_DEFINITION (OptionContainer, vcontainer, "OptionContainer");

//again init fuctions overode in case I needed to use them

void OptionContainer::Init ()
{
    vcontainer::Init();
}

void OptionContainer::LoadInit (vresource  res)
{
    vcontainer::LoadInit(res);
    
}

void OptionContainer::CopyInit (vloadable *original)
{
    vcontainer::CopyInit(original);
}

void OptionContainer::Destroy ()
{
    vcontainer::Destroy ();
}


// Function called before the dialog load, it subloads some objects into
// the option container.
void OptionContainer::preinitialize()
{
    currentSelection = NULL;
    
    ColorToggleButton * DialogToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "Dialog"));
    ColorToggleButton * ToolBarToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "ToolBar"));
    ColorToggleButton * ProgressIndicatorToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "ProgressIndicator"));
    ColorToggleButton * MenuToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "Menu"));
    ColorToggleButton * SashToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "Sash"));
    ColorToggleButton * PaneToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "Pane"));
    ColorToggleButton * CommandLineToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "CommandLine"));
    ColorToggleButton * TargetPaneBorderToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "TargetPaneBorder"));
    ColorToggleButton * CurrentPaneBorderToggleButton = ColorToggleButton::
                        RequestSubLoad(this, vnameInternGlobalLiteral(
                            "CurrentPaneBorder"));
}


// Initializes all the objects in the option container.
void OptionContainer::initialize()
{
    ColorToggleButton * currentbutton;
    currentbutton = ColorToggleButton::CastDown(FindItem(
                    vnameInternGlobalLiteral("Dialog")));
    currentbutton->initialize (this);

    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "ToolBar")))->initialize(this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "ProgressIndicator")))->initialize (this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "Menu")))->initialize(this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "Sash")))->initialize(this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "Pane")))->initialize(this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "CommandLine")))->initialize(this);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "TargetPaneBorder")))->initialize(this, 0);
    ColorToggleButton::CastDown(FindItem(vnameInternGlobalLiteral(
                                "CurrentPaneBorder")))->initialize(this, 0);

    SetSelected (currentbutton);
    currentbutton->SetValue(1);
}


ColorToggleButton * OptionContainer::GetSelected ()
{
    return currentSelection;
}

void OptionContainer::SetSelected (ColorToggleButton * newSelection)
{
    currentSelection = newSelection;
}

/**********************************ColorToggleButton******************************/

vkindTYPED_FULL_DEFINITION (ColorToggleButton, vcontrolToggle, "ColorToggleButton");

//again init fuctions overode in case I needed to use them

void ColorToggleButton::Init ()
{
    vcontrolToggle::Init();
}

void ColorToggleButton::LoadInit (vresource  res)
{
    vcontrolToggle::LoadInit(res);
}

void ColorToggleButton::CopyInit (vloadable *original)
{
    vcontrolToggle::CopyInit(original);
}

void ColorToggleButton::Destroy ()
{
    vcontrolToggle::Destroy ();
}

//things that can't be put in the constructor, because they require the dialog to be already loaded

void ColorToggleButton::initialize(OptionContainer * newContainer,
                                   int multipleColors)
{
    changed = 0;
    myContainer = newContainer;
    vchar * myTag = (vchar *) vmemAlloc(vcharLength(GetTag())*sizeof(vchar)+1);
    vcharCopy(GetTag(), myTag);
    
    /* The tag names of the samples of the color choies are the same as
       the tags of the corresponding button with "Sample" added to the end
      this just gets the tag, and appends sample, and finds the item*/
    
    vchar* sampleName = (vchar *) vmemAlloc((32 +
		vcharLength(myTag))*sizeof(vchar));
    vcharCopy(myTag, sampleName);
    vcharAppendFromLiteral(sampleName, "Sample");

    mySample = ColorSample::CastDown(
	newContainer->FindItem(vnameInternGlobal(sampleName)));
    mySample->SetToggleButton(this);
    vmemFree(sampleName);

    
    if (vcharCompare(myTag, vnameInternGlobalLiteral("Dialog")) == 0) {
	mySample->SetForeground(Viewer::GetDialogForeground());
	mySample->SetBackground(Viewer::GetDialogBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral("Menu")) == 0) {
	mySample->SetForeground(Viewer::GetMenuForeground());
	mySample->SetBackground(Viewer::GetMenuBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral("Pane")) == 0) {
	mySample->SetForeground(Viewer::GetPaneForeground());
	mySample->SetBackground(Viewer::GetPaneBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral("Sash")) == 0) {
	mySample->SetForeground(Viewer::GetSashForeground());
	mySample->SetBackground(Viewer::GetSashBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral("ToolBar")) == 0) {
	mySample->SetForeground(Viewer::GetToolBarForeground());
	mySample->SetBackground(Viewer::GetToolBarBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral(
                            "ProgressIndicator")) == 0) {
	mySample->SetForeground(Viewer::GetProgressForeground());
	mySample->SetBackground(Viewer::GetProgressBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral(
                            "CommandLine")) == 0) {
	mySample->SetForeground(Viewer::GetCommandLineForeground());
	mySample->SetBackground(Viewer::GetCommandLineBackground());

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral(
                            "CurrentPaneBorder")) == 0) {
 	mySample->SetBackground(newContainer->FindItem(
                  vnameInternGlobalLiteral("PaneSample"))->GetBackground());
	mySample->SetForeground(newContainer->FindItem(
                  vnameInternGlobalLiteral("PaneSample"))->GetForeground());
	vcolorSpec *borderColor = new vcolorSpec;
	borderColor->Copy(Viewer::GetTargetPaneBorder()->GetSpec());
	mySample->SetBorderColor(borderColor);
        delete borderColor;

    } else if (vcharCompare(myTag, vnameInternGlobalLiteral(
                            "TargetPaneBorder")) == 0) {
 	mySample->SetBackground(newContainer->FindItem(
                  vnameInternGlobalLiteral("PaneSample"))->GetBackground());
	mySample->SetForeground(newContainer->FindItem(
                  vnameInternGlobalLiteral("PaneSample"))->GetForeground());
	vcolorSpec *borderColor = new vcolorSpec;
	borderColor->Copy(Viewer::GetTargetPaneBorder()->GetSpec());
	mySample->SetBorderColor(borderColor);
        delete borderColor;

    } else {
	printf ("No method for that Sample\n");
    }
    vmemFree(myTag);
}

//toggle button observedialogitem that just turns off the previous selected one, and then tells its container, that it is now the current one

void ColorToggleButton::ObserveDialogItem (vevent * event)
{
    myContainer->GetSelected()->SetValue (0);
    SetValue (1);
    myContainer->SetSelected(this);
    const vname * attributename = vcontainerExclusiveGroup::CastDown(
                  DetermineDialog()->FindItem(vnameInternGlobalLiteral(
                  "AttributeContainer")))->GetSelection()->GetTag();
    
    for (int i = 0; i < 2; i++) {
	ColorContainer * currentcontainer;
	if (i==0)
	    currentcontainer = ColorDialog::CastDown(DetermineDialog()
                               )->GetDefaultContainer()->GetFirstContainer();
	else 
	    currentcontainer = ColorDialog::CastDown(DetermineDialog()
                               )->GetCustomizeContainer()->GetFirstContainer();
	
	unsigned int objr, objg, objb, containr, containg, containb;
	if (mySample->GetBorderColor())
	    mySample->GetBorderColor()->GetRGB(&objr, &objg, &objb);
	else {
	    if (vcharCompare(attributename, vnameInternGlobalLiteral(
                             "Background")) == 0)
		mySample->GetBackground()->GetRGB(&objr, &objg, &objb);
	    else
		mySample->GetForeground()->GetRGB(&objr, &objg, &objb);
	}
	while (currentcontainer) {
	    currentcontainer->GetBackground()->GetRGB(&containr, &containg,
                                                      &containb);
	    if (objr == containr && objg == containg && objb == containb) {
		ColorDialog::CastDown(DetermineDialog()
                             )->SetSelected(currentcontainer);
		return;
	    }
	    currentcontainer = currentcontainer->GetNext();
	}
	
    }
    ColorDialog::CastDown(DetermineDialog())->SetSelected (NULL);
}

ColorSample *ColorToggleButton::GetSample()
{
    return mySample;
}

//sets the flag that says the color of this particular object has been changed

void ColorToggleButton::Change()
{
    changed = 1;
}

//checks to see if the togglebutton is changed

int ColorToggleButton::Changed()
{
    return changed;
}


/******************colorsample******************************/

vkindTYPED_FULL_DEFINITION (ColorSample, vdialogLabelItem, "ColorSample");

//initializes with a null bordercolor which can later be changed

void ColorSample::Init ()
{
    vdialogLabelItem::Init();
    borderColor = NULL;
}

void ColorSample::LoadInit (vresource  res)
{
    vdialogLabelItem::LoadInit(res);
    borderColor = NULL;
}

void ColorSample::CopyInit (vloadable *original)
{
    vdialogLabelItem::CopyInit(original);
    borderColor = NULL;
}
void ColorSample::Destroy ()
{
    vdialogLabelItem::Destroy ();
}

vcolorSpec * ColorSample::GetBorderColor ()
{
    return borderColor->GetSpec();
}


void ColorSample::SetBorderColor (vcolorSpec * newborderSpec)
{
    borderColor = vcolor::Intern (newborderSpec);
    Inval(vwindowINVAL_IMMEDIATE);
}


//colorsample's observe function just selects it's toggle button

int ColorSample::HandleButtonDown (vevent * event)
{
    mytoggleButton->ObserveDialogItem (event);
    return 1;
}

//overridden to draw borders if neccesary

void ColorSample::Draw()
{
    vdialogLabelItem::Draw();
    
    if (!borderColor)
	return;
    
    vdraw::GSave();
    DrawDevice();
    vdraw::SetLineWidth(2);
    vcolorSpec* newcolorspec = new vcolorSpec;
    newcolorspec->Copy(borderColor->GetSpec());
    
    unsigned int hue, light, sat;
    newcolorspec->GetHLS(&hue, &light, &sat);
    
    
    vcolor* upperleftcolor, *lowerrightcolor;
    
    if (hue > MAX_LIGHT)
    {
	upperleftcolor = borderColor;
	hue = (unsigned int)((float)hue * LIGHTNESS);
	newcolorspec->SetHLS(hue, light, sat);
	lowerrightcolor  = vcolor::Intern(newcolorspec);
    }
    else
    {
	lowerrightcolor = borderColor;
	hue = (unsigned int)((float)hue * DARKNESS);
 	newcolorspec->SetHLS(hue, light, sat);
	upperleftcolor  = vcolor::Intern(newcolorspec);
    }    
    
    const vrect* myrect = GetRect();
    vrect emboss;
    emboss.Set(myrect->GetX() - BORDER,
	       myrect->GetY() - BORDER,
	       myrect->GetWidth() + 2*BORDER,
	       myrect->GetHeight() + 2*BORDER);
    Draw3DBottomRight(&emboss, BORDER, lowerrightcolor);
    Draw3DTopLeft(&emboss, BORDER, upperleftcolor);
    
    //delete newcolorspec;
    vdraw::GRestore();
}


void ColorSample::SetToggleButton (ColorToggleButton * newToggleButton)
{
    mytoggleButton = newToggleButton;
}


//**********************ColorObjectsContainer****************

vkindTYPED_FULL_DEFINITION (ColorObjectsContainer, vcontainer, "ColorObjectsContainer");


//again init function  just here for convenience

void ColorObjectsContainer::Init()
{
    vcontainer::Init();
}


void ColorObjectsContainer::LoadInit (vresource  res)
{
    vcontainer::LoadInit(res);
    
}

void ColorObjectsContainer::CopyInit (vloadable *original)
{
    vcontainer::CopyInit(original);
}

void ColorObjectsContainer::Destroy ()
{
    if (colorchooser) delete colorchooser;

    if (firstcontainer) delete firstcontainer;

    vcontainer::Destroy ();
}

//loads in all of the colors default and custom
void ColorObjectsContainer::initialize(int newCustomizeable)
{
    // Initialize all the privates.
    currentSelection = NULL;
    customizeable = newCustomizeable;
    changed = 0;
    colorchooser = NULL;

    vresource res;
    int foundPaletteRes = 0;
    
    vcolorPalette * defaultPalette;
    if (customizeable) { // Get the palette from anywhere.
	if (getResource("Colors", &res) ) {
            if (getResource(res, "DefaultPalette", &res) ) {
                foundPaletteRes = 1;
            }
        }
    } else { // Get the palette from the Resource file.
	res = vapplication::GetCurrent()->GetResources();
	if (getResource(res, "Colors", &res) ) {
            if (getResource(res, "DefaultPalette", &res) ) {
                foundPaletteRes = 1;
            }
        }
    }

    if (foundPaletteRes)
        defaultPalette = new vcolorPalette(res);
    else {
        printf("@ColorObjectsContainer::initialize:");
        printf("Unable to find DefaultPalette resource!!!\n");
        return;
    }
    
    
    int colors = defaultPalette->GetSpecCount();
    
    // Make the rect the the colors will go in.
    // Then just keep moving it around as new colors are added.
    
    vrect currentrect;
    currentrect.SetY(GetHeight () - COLOR_SIZE - COLOR_SPACING -
                     TOP_COLOR_SPACING);
    currentrect.SetX(COLOR_SPACING);
    currentrect.SetHeight(COLOR_SIZE);
    currentrect.SetWidth (COLOR_SIZE);
    
    ColorContainer * currentcontainer = NULL;
    ColorContainer * prevcontainer = NULL;
    
    for (int i = 0; i < colors; i++) {
	currentcontainer = new ColorContainer;
	if (i == 0) {
	    if (customizeable) {
		SetSelected(currentcontainer);
		currentcontainer->SetSelected(0);
	    }
	    firstcontainer = currentcontainer;
	} else
	    prevcontainer->SetNext(currentcontainer);
	if (customizeable)
	    currentcontainer->MakeCustomizeable();
	currentcontainer->SetRect(&currentrect);
	currentcontainer->SetBackground(vcolor::Intern(
                                        defaultPalette->GetSpecAt(i)) );
	if (GetWidth() > (currentrect.GetX() + (2*COLOR_SIZE) + COLOR_SPACING))
	    currentrect.SetX(currentrect.GetX() + COLOR_SIZE + COLOR_SPACING);
	else {
	    currentrect.SetX (COLOR_SPACING);
	    currentrect.SetY (currentrect.GetY() - COLOR_SPACING - COLOR_SIZE);
	}
	AppendItem (currentcontainer);
	prevcontainer = currentcontainer;
    }	    
    delete defaultPalette;
}



//deselects the current selection  and selects the new one
void ColorObjectsContainer::SetSelected(ColorContainer * newselection)
{
    if (currentSelection)
	currentSelection->SetSelected (0);
    if (customizeable) {
	if (currentSelection)
	    currentSelection->SetCustomizeSelected (0);
	if (newselection)
	    newselection->SetCustomizeSelected (1);
    }
    currentSelection = newselection;
    if (newselection)
	newselection->SetSelected (1);
}


ColorContainer * ColorObjectsContainer::GetSelected()
{
    return currentSelection;
}

CustomizeColorChooser * ColorObjectsContainer::GetColorChooser ()
{
    return colorchooser;
}

void ColorObjectsContainer::SetColorChooser (CustomizeColorChooser * newchooser)
{
    colorchooser = newchooser;
}

//flag that says the custom colors have changed

void ColorObjectsContainer::Change()
{
    changed = 1;
}

//checks the change flag to see if the container has been changed, for saving purposes

int ColorObjectsContainer::GetChanged()
{
    return changed;
}

//gets a pointer to the linked list of colors 

ColorContainer *ColorObjectsContainer::GetFirstContainer()
{
    return firstcontainer;
}

//*********************ColorContainer****************************

vkindTYPED_FULL_DEFINITION (ColorContainer, vcontainer, "ColorContainer");

//initializes all the variables

void ColorContainer::Init ()
{
    vcontainer::Init();
    SetClickFocusable(1);
    selection = 0;
    customize = 0;
    customizeselection = 0;
    next = NULL;
}

void ColorContainer::LoadInit (vresource  res)
{
    vcontainer::LoadInit(res);
}

void ColorContainer::CopyInit (vloadable *original)
{
    vcontainer::CopyInit(original);
}
void ColorContainer::Destroy ()
{
    if (next) delete next;

    vcontainer::Destroy ();
}

//marks itself as selected and then invalidates the rect so it will immeadiately redraw

void ColorContainer::SetSelected (int newselection)
{
    selection = newselection;
    Inval(0);
}

//changes the color of the currently selected option and also makes itself the currently selected container

int ColorContainer::HandleButtonDown (vevent * event)
{
    ColorDialog::CastDown (DetermineDialog())->SetSelected (this);
    
    //if it is a customizeable color
    
    if (customize)
    {
	CustomizeColorChooser * myChooser;
	
	//if there is a color chooser up sets this too be the color that is currently being modified
	
	if ((myChooser = (ColorDialog::CastDown(DetermineDialog())->GetCustomizeContainer()->GetColorChooser())) !=NULL)
	{
	    myChooser->SetCurrentColorContainer(this);
	    myChooser->SetColor(GetBackground());
	}
    }
    
    ColorToggleButton * currentToggle = OptionContainer::CastDown (DetermineDialog()->FindItem (vnameInternGlobalLiteral ("ObjectOptionContainer")))->GetSelected ();
    if (currentToggle)
    {
	currentToggle->Change();
	ColorSample * currentSample = currentToggle->GetSample();
	if (currentSample->GetBorderColor())
	    currentSample->SetBorderColor (GetBackground()->GetSpec());
	else {
	    // Check if foreground or background attribute is selected.
	    if (vcharCompare(vcontainerExclusiveGroup::CastDown(
                DetermineDialog()->FindItem(vnameInternGlobalLiteral(
                "AttributeContainer")))->GetSelection()->GetTag(),
                vnameInternGlobalLiteral("Foreground")) == 0) {
		if (vcharCompare(currentToggle->GetTag(),
                    vnameInternGlobalLiteral("Pane")) == 0) {
		    ColorToggleButton::CastDown(OptionContainer::CastDown(
                         currentSample->GetParentObject())->FindItem(
                         vnameInternGlobalLiteral("CurrentPaneBorder"))
                         )->GetSample()->SetForeground(GetBackground());
		    ColorToggleButton::CastDown(OptionContainer::CastDown(
                         currentSample->GetParentObject())->FindItem(
                         vnameInternGlobalLiteral("TargetPaneBorder"))
                         )->GetSample()->SetForeground(GetBackground());
		}
		currentSample->SetForeground(GetBackground());
	    } else {
		if (vcharCompare(currentToggle->GetTag(),
                    vnameInternGlobalLiteral ("Pane")) == 0) {
		    ColorToggleButton::CastDown(OptionContainer::CastDown(
                         currentSample->GetParentObject())->FindItem(
                         vnameInternGlobalLiteral("TargetPaneBorder"))
                         )->GetSample()->SetBackground(GetBackground());
		    ColorToggleButton::CastDown(OptionContainer::CastDown(
                         currentSample->GetParentObject())->FindItem(
                         vnameInternGlobalLiteral("CurrentPaneBorder"))
                         )->GetSample()->SetBackground(GetBackground());
		}
		currentSample->SetBackground(GetBackground ());
	    }
	}
    }
    ColorDialog::CastDown (DetermineDialog())->Change();
    return 1;
}

void ColorContainer::Draw()
{
    vcontainer::Draw();
    
    vdraw::GSave();
    
    DrawDevice();
    
    if (selection)
	vdraw::SetColor(vcolor::GetWhite());
    else if (customizeselection)
	vdraw::SetColor (vcolor::GetBlack());
    else { // If it is not selected in any way,
           // draw it with the border color of it's parents background
           // in other words without a background.
        vdialog * mydialog = DetermineDialog();
        vcolor * dialogcolor = mydialog->DetermineBackground();
        vdraw::SetColor(dialogcolor);
    }
    
    vdraw::SetLineWidth(CURRENT_CONTAINER_BORDER_WIDTH);
    
    vdraw::RectStroke (GetX(), GetY(), GetWidth(), GetHeight());
    
    vdraw::GRestore();
}

//sets the customizeable color flag to be on
void ColorContainer::MakeCustomizeable()
{
    customize = 1;
}

int ColorContainer::GetCustomizeable()
{
    return customize;
}


void ColorContainer::SetCustomizeSelected (int customizeable)
{
    customizeselection = customizeable;
    Inval(0);
}

// Get the next container in the linked list of customizeable colors.
// (Used for saving.)
ColorContainer * ColorContainer::GetNext()
{
    return next;
}

void ColorContainer::SetNext (ColorContainer * newcontainer)
{
    next = newcontainer;
}

//****************************CustomizeColorButton****************


vkindTYPED_FULL_DEFINITION (CustomizeColorButton, vbutton, "CustomizeColorButton");

//init fuctions overode only for convenience

void CustomizeColorButton::Init ()
{
    vbutton::Init();
}

void CustomizeColorButton::LoadInit (vresource  res)
{
    vbutton::LoadInit(res);
}

void CustomizeColorButton::CopyInit (vloadable *original)
{
    vbutton::CopyInit(original);
}

void CustomizeColorButton::Destroy ()
{
    vbutton::Destroy ();
}



//button to customize colors brings up a color chooser with the correct colors, and correct observe function

void  CustomizeColorButton::ObserveDialogItem (vevent * event)
{
    vbutton::ObserveDialogItem(event);
    ColorObjectsContainer * currentColorObjectsContainer = ColorDialog::CastDown (DetermineDialog())->GetCustomizeContainer();
    
    if (currentColorObjectsContainer->GetColorChooser())
	return;
    
    vcolor * currentcolor = currentColorObjectsContainer->GetSelected()->GetBackground();
    CustomizeColorChooser * mychooser = new CustomizeColorChooser;
    
    currentColorObjectsContainer->SetColorChooser (mychooser);
    mychooser->SetCurrentColorContainer(currentColorObjectsContainer->GetSelected());
    mychooser->SetModal (0);
    mychooser->SetMethod (vcolorchsrHLS);
    
    mychooser->SetColorSpec (currentcolor->GetSpec());
    
    mychooser->Open();
}

//**************************CustomizeColorChooser*****************************

vkindTYPED_FULL_DEFINITION (CustomizeColorChooser, vcolorchsr, "CustomizeColorChooser");


//subclass of vcolorchsr only to give it a private pointer pointing to the currently selected color and a observed function to use it

void CustomizeColorChooser::ObserveApplyColor (vcolor * color)
{
    currentContainer->SetBackground (color);
    currentContainer->HandleButtonDown(NULL);
    ColorObjectsContainer::CastDown (currentContainer->GetParentObject())->Change();
}


void CustomizeColorChooser::SetCurrentColorContainer (ColorContainer * newContainer)
{
    currentContainer = newContainer;
}

void CustomizeColorChooser::Close()
{
    ColorObjectsContainer::CastDown(currentContainer->GetParentObject())->SetColorChooser (NULL);
    
    printf ("The chooser has returned\n");
    
    vwindow::Close();
}

//****************************PaletteChooser*******************************

vkindTYPED_FULL_DEFINITION (PaletteChooser, vcomboPopdown, "PaletteChooser");

// This function is needed because we need to pass a sort function into
// vlistitem::SortItems in the PaletteChooser::LoadInit.
// The vcharCompare function cannot be used because it is really a macro.
static myVcharCompare(const vchar * str1, const vchar * str2)
{
return(vcharCompare(str1, str2));
}


void PaletteChooser::LoadInit(vresource Res)
{
    vcomboPopdown::LoadInit (Res);
    vlistitem * mylistitem = GetListitem();
    
    vresource res, schemeRes;
    if (getResource("Colors", &res) ) {
        if (getResource(res, "CurrentScheme", &schemeRes) ) {
            Schemename = vresourceGetString(vresourceGetParent(schemeRes),
                         vnameInternGlobalLiteral("CurrentScheme") );
            if (getResource(res, "Schemes", &res) ) {
                size_t length = vresourceCountComponents(res);
                const vname * currentTag;
                for (int i = 0; i < length; i++) {
                    currentTag = vresourceGetNthTag(res, i);
                    mylistitem->AppendScribed(vnameScribeGlobal(currentTag));
                }
                mylistitem->SortItems((int(*)(const void*, const void*))
                                      myVcharCompare);
            }
        }
    }
}

void PaletteChooser::CancelScheme()
{
    SelectText (Schemename);
}


void PaletteChooser::LoadScheme ()
{
    vresource currentSchemeRes, listRes, res;
    vresource paletteRes;
    vliststr * colorList;
    const vname * schemeName;
    OptionContainer * myOptionContainer;
    int foundPaletteRes;

    if (getResource("Colors:PaletteList", &res) ) {
        colorList = new vliststr(res);
        myOptionContainer = (OptionContainer *)DetermineDialog()->
                            FindItem(vnameInternGlobalLiteral(
                            "ObjectOptionContainer") );
        vresource currentPaletteRes;
        if (getResource(res, "Colors:Schemes", &currentPaletteRes) ) {
            schemeName = vnameInternGlobal( this->GetTextAsString());
            if (getResource(currentPaletteRes, schemeName,
                            &paletteRes) ) {
                foundPaletteRes = 1;
            }
        }
    }


    if (!foundPaletteRes)
        return;


    // Go through a string list with the name of all the current objects.
    // Load the colors for each of them.
    for (int i= colorList->GetRowCount() - 1; i >= 0; i--) {
	const vname * currentObjectName = vnameInternGlobalScribed(
                      colorList->ScribeCellValue (i, 0));
	vresource objectRes;
        if (getResource(paletteRes, currentObjectName, &objectRes) ) {
	    ColorSample * currentObject = ColorToggleButton::CastDown(
                          myOptionContainer->FindItem(currentObjectName)
                          )->GetSample();
            if (currentObject->GetBorderColor() ) {
                vcolorSpec * borderColor = new vcolorSpec(objectRes);
                currentObject->SetBorderColor(borderColor);
                delete borderColor;
            } else {
                vresource colorRes;
                vcolorSpec * currentColorSpec;
                vcolor * currentColor;
                if (getResource(objectRes, "Foreground", &colorRes) ) {
                    currentColorSpec = new vcolorSpec(colorRes);
                    currentColor = vcolor::Intern(currentColorSpec);
                    currentObject->SetForeground(currentColor);
                    delete currentColorSpec;
                }
                if (getResource(objectRes, "Background", &colorRes) ) {
                    currentColorSpec = new vcolorSpec(colorRes);
                    currentColor = vcolor::Intern(currentColorSpec);
                    currentObject->SetBackground(currentColor);
                    delete currentColorSpec;
                }
            }
        }
    }
    Schemename = (vname *)schemeName;
    delete colorList;
}


void PaletteChooser::CreateScheme (vname * newSchemeName)
{
    vresource res, schemeRes, defaultSchemeRes;

    if (getResource("Colors", &res) ) {
        if (getResource(res, "Schemes", &res) ) {
            if (!getResource(res, newSchemeName, &schemeRes) ) {
                if (getResource(res, "DISCOVER", &defaultSchemeRes) ) {
                    schemeRes = vresourceCreate(res, newSchemeName,
                                                vresourceUNTYPED);
                    vresourceCopy(defaultSchemeRes, schemeRes);
                    AppendText(newSchemeName);
                }
            }
        SelectText (newSchemeName);
        LoadScheme();
        }
    }
}
    

//loads in any palettes the user may want to load
void PaletteChooser::ObserveDialogItem(vevent * event)
{
    if (ColorDialog::CastDown (DetermineDialog())->GetChanged())
    {
	gala_eval (g_global_interp, "dis_launch_dialog ChangeSchemeDialog");
    }
    else
	LoadScheme();
}

//**************************Attribute Container***********************

vkindTYPED_FULL_DEFINITION (AttributeContainer, vcontainerExclusiveGroup, "AttributeContainer");


void AttributeContainer::preinitialize()
{
    AttributeToggleButton::RequestSubLoad (this, vnameInternGlobalLiteral ("Foreground"));
    AttributeToggleButton::RequestSubLoad (this, vnameInternGlobalLiteral ("Background"));
}

void AttributeContainer::LoadInit(vresource res)
{
    vcontainerExclusiveGroup::LoadInit(res);
    SetSelection (AttributeToggleButton::CastDown (FindItem(vnameInternGlobalLiteral ("Background"))));
}


//***************************AttributeToggleButton********************

vkindTYPED_FULL_DEFINITION (AttributeToggleButton, vcontrolToggle, "AttributeToggleButton");


void AttributeToggleButton::ObserveDialogItem (vevent * event)
{
    vcontrolToggle::ObserveDialogItem(event);
    OptionContainer::CastDown (DetermineDialog()->FindItem (vnameInternGlobalLiteral ("ObjectOptionContainer")))->GetSelected()->ObserveDialogItem (event);
}





/*********     end of prefs.C     **********/
