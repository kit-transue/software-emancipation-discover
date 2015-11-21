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
//
// Subclass "vtimer" with class "positionTimer"
//
class positionTimer : public vtimer
{
  private:
  // variables used in tracking mouse between timer checks
  //
  vdialog		*dialog;
  vdialogLabelItem	*idlabel;
  int			oldx;
  int			oldy;
  int			previtem;
  int			contitem;
  vcontainer		*oldcontainer;

  public:
  // subclass constructors
  //
  positionTimer();
  positionTimer(vdialog *dialog, vdialogLabelItem *labelitem, 
		unsigned long int secs,	unsigned long int nanosecs);

  // overload the following vtimer functions
  //
  virtual void    	Init();
  virtual void		ObserveTimer();

  // define set and get "dialog" inline functions
  //
  void			SetDialog(vdialog *d) {dialog = d;};
  vdialog		*GetDialog() {return dialog;};

  // define set and get "idlabel" inline functions
  //
  void			SetIdLabel(vdialogLabelItem *dl) {idlabel = dl;};
  vdialogLabelItem	*GetIdLabel() {return idlabel;};

  // declare function ContainsPoint
  //
  vbool ContainsPoint(int x, int y);

  // declare a function to handle container items
  //
  void HandleContainer(vcontainer *container, int curx, int cury);

};
