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
#ifndef _gprefs_h
#define _gprefs_h

#include <vport.h>
#include vdialogHEADER
#include vcharHEADER
#include vmenuHEADER
#include vbuttonHEADER
#include vdragHEADER
#include vcomboHEADER



//********************new Color Chooser **********************


class OptionContainer;
class ColorObjectsContainer;
class ColorContainer;
class ColorToggleButton;
class ColorSample;
class vcomboPopdown;
class CustomizeColorChooser;

class ColorDialog:public vdialog
{
  public:
    vkindTYPED_FULL_DECLARATION(ColorDialog);
    vloadableINLINE_CONSTRUCTORS (ColorDialog, vdialog);
    void initialize();
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    static void Apply (vdialogItem * item, vevent * event);
    static void ApplyColors (OptionContainer *);
    static void Save (vresource, OptionContainer *);
    static void SaveandApplyColors(vdialogItem * item, vevent * event);
    static void CreateSchemePref();
    static void NewScheme(vdialogItem * item, vevent * event);
    static void DeleteScheme (vdialogItem * item, vevent * event);
    static void RevertScheme (vdialogItem * item, vevent * event);
    static void Cancel (vdialogItem * item, vevent * event);
    ColorContainer * GetSelected();
    void SetSelected (ColorContainer *);
    ColorObjectsContainer *GetCustomizeContainer();
    ColorObjectsContainer *GetDefaultContainer();
    void Change();
    int GetChanged();
  private:
    int changed;
    ColorContainer * currentContainer;
    ColorObjectsContainer * DefaultContainer;
    ColorObjectsContainer * CustomizeContainer;
};


class OptionContainer:public vcontainer
{
  public:
    vkindTYPED_FULL_DECLARATION(OptionContainer);
    vloadableINLINE_CONSTRUCTORS(OptionContainer, vcontainer);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    void preinitialize();
    void initialize();
    ColorToggleButton *GetSelected();
    void SetSelected(ColorToggleButton *);
  private:
    ColorToggleButton * currentSelection;
};



class ColorObjectsContainer:public vcontainer
{
  public:
    vkindTYPED_FULL_DECLARATION(ColorObjectsContainer);
    vloadableINLINE_CONSTRUCTORS (ColorObjectsContainer, vcontainer);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    void SetSelected(ColorContainer *);
    ColorContainer * GetSelected ();
    void initialize (int customizeable);
    void Change();
    CustomizeColorChooser * GetColorChooser();
    void SetColorChooser(CustomizeColorChooser *);
    int GetChanged();
    ColorContainer *GetFirstContainer();
  private:
    ColorContainer * currentSelection;
    int customizeable;
    int changed;
    CustomizeColorChooser * colorchooser;
    ColorContainer * firstcontainer;
};



class ColorContainer:public vcontainer
{
  public:
    vkindTYPED_FULL_DECLARATION(ColorContainer);
    vloadableINLINE_CONSTRUCTORS (ColorContainer, vcontainer);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    virtual int HandleButtonDown (vevent *);
    virtual void Draw();
    void SetSelected (int);
    void SetCustomizeSelected(int);
    void MakeCustomizeable();
    int GetCustomizeable();
    ColorContainer * GetNext();
    void SetNext (ColorContainer *);
  private:
    int selection; //state to tell if it is the current color or not
    int customize; //state to tell if it is a customizeable color or not
    int customizeselection;
    ColorContainer * next;
};



class CustomizeColorButton:public vbutton
{
  public:
    vkindTYPED_FULL_DECLARATION(CustomizeColorButton);
    vloadableINLINE_CONSTRUCTORS (CustomizeColorButton, vbutton);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    virtual void  ObserveDialogItem (vevent *);
};


class ColorToggleButton:public vcontrolToggle
{
  public:
    vkindTYPED_FULL_DECLARATION(ColorToggleButton);
    vloadableINLINE_CONSTRUCTORS (ColorToggleButton, vcontrolToggle);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    void initialize (OptionContainer *, int multipleColors = 1);
    virtual void ObserveDialogItem (vevent *);
    ColorSample * GetSample ();
    void Change (); //changes the change flag to 1
    int Changed ();
  private:
    int changed; //flag to see if it needs to be updated or not
    ColorSample * mySample;
    OptionContainer * myContainer;
};

class ColorSample:public vdialogLabelItem
{
  public:
    vkindTYPED_FULL_DECLARATION(ColorSample);
    vloadableINLINE_CONSTRUCTORS (ColorSample, vdialogLabelItem);
    virtual void Init();
    virtual void LoadInit (vresource);
    virtual void CopyInit (vloadable * original);
    virtual void Destroy();
    virtual int HandleButtonDown (vevent *);
    void SetBorderColor (vcolorSpec *);
    vcolorSpec * GetBorderColor();
    virtual void Draw();
    void SetToggleButton (ColorToggleButton*);
  private:
    ColorToggleButton * mytoggleButton;
    vcolor * borderColor;
};

class CustomizeColorChooser:public vcolorchsr
{
  public:
    vkindTYPED_FULL_DECLARATION(CustomizeColorChooser);
    vloadableINLINE_CONSTRUCTORS (CustomizeColorChooser, vcolorchsr);
    virtual void ObserveApplyColor (vcolor *);
    void SetCurrentColorContainer (ColorContainer *);
    virtual void Close();
  private:
    ColorContainer * currentContainer;
};
    
class PaletteChooser:public vcomboPopdown
{
  public:
    vkindTYPED_FULL_DECLARATION(PaletteChooser);
    vloadableINLINE_CONSTRUCTORS (PaletteChooser, vcomboPopdown);
    void LoadInit(vresource);
    virtual void ObserveDialogItem (vevent * event);
    void LoadScheme();
    void CancelScheme ();
    void CreateScheme (vchar *);
  private:
    vname * Schemename;
};


class AttributeContainer:public vcontainerExclusiveGroup
{
  public:
    vkindTYPED_FULL_DECLARATION(AttributeContainer);
    vloadableINLINE_CONSTRUCTORS (AttributeContainer, vcontainerExclusiveGroup);
    virtual void preinitialize();
    virtual void LoadInit (vresource);
};

class AttributeToggleButton:public vcontrolToggle
{
  public:
    vkindTYPED_FULL_DECLARATION(AttributeToggleButton);
    vloadableINLINE_CONSTRUCTORS (AttributeToggleButton, vcontrolToggle);
    virtual void ObserveDialogItem(vevent * event);
};   

#endif





/**********     end of gprefs.h     **********/
