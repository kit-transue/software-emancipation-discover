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
#ifndef _gDrawingArea_h
#define _gDrawingArea_h

#include <vport.h>
#include vdialogHEADER
#include vdomainitemHEADER
#include vimageHEADER

#include <gcontrolObjects.h>

class DrawAreaItemDesc {
public:
    ~DrawAreaItemDesc();
    
    char          *tipwin_text;
    char          *title;
    char          *callback;
    vimage        *image;
    int           number;
    int           x, y;
    int           selected;
    vdomainObject *object;
};

typedef DrawAreaItemDesc *DrawAreaItemDescPtr;

gArray (DrawAreaItemDescPtr);

class gdDrawingArea : public vdomainitem, public gdWidget 
{
  protected:
    virtual void Init();
    virtual void LoadInit(vresource resource);
    virtual void Destroy();
    virtual void CommonInit();

  public:
    vkindTYPED_FULL_DECLARATION(gdDrawingArea);
    vloadableINLINE_CONSTRUCTORS(gdDrawingArea, vdomainitem);

    virtual void  Open();
    virtual void  Show();
    virtual void  DrawObject(vdomainObject *object); 
    virtual vbool HandleButtonDownEvent(vdomainviewButtonDownEvent *event);
    virtual vbool HandleButtonUpEvent(vdomainviewButtonUpEvent *event);
    virtual vbool HandleDoubleClickEvent(vdomainviewDblClickEvent *event);
    virtual int   HandleMotion(vevent *event);

    void SetGrid(int w, int h, char *orientation);
    void SetGridDimensions(int rows, int cols);
    void CalcRowsAndColumns();
    void FindRowColumnCell(vpoint &pt, int &row, int &col, int &cell);
    void MoveInsertItem(DrawAreaItemDesc *desc, int from_pos, int to_pos);
    void GetItemPosition(int &x, int &y, int w, int h, int number);
    void ShiftItems(int to_pos);
    int  AddItem(char *image_name);
    void RemoveItem(int item_num);
    void SetIconTitle(int item_num, char *title);
    void SetIconTip(int item_num, char *tip);
    void Clear();
    void SetClickCmd(char *cmd);
    void SetMoveCmd(char *cmd);
    void SelectItem(int item, int select_flag);


    int  button_down_row;
    int  button_down_col;
    int  current_rows;
    int  current_cols;
    int  current_w;
    int  current_h;
    int  grid_w;
    int  grid_h;
    int  vertical;
    int  was_opened;
    char *click_command;
    char *move_command;
    gArrayOf (DrawAreaItemDescPtr) items;
};

#endif
