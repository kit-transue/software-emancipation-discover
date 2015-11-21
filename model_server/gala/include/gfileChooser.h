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
 * gfileChooser.h
 *
 **********/

#ifndef _gfileChooser_h
#define _gfileChooser_h

#ifndef vcharINCLUDED
   #include vcharHEADER
#endif
#ifndef vfilechsrINCLUDED
   #include vfilechsrHEADER
#endif


class fileChooser : public vfilechsr
{
  public:
    fileChooser() {returnValue_ = NULL;multipleSelections=0;}
	~fileChooser() {if (returnValue_) vstrDestroy (returnValue_);}
    virtual int HookApplyFile(vfsPath*);
    void SetCallers (int, 
                     int, 
                     vchar*, 
                     vchar*, 
                     const vchar*, 
                     const vchar*, 
                     const vchar*,
					 int = 0);
    vchar *GetResult () {return (returnValue_);}
    
  private:
    int viewer_, view_;
    vstr* returnValue_;
    vchar* pathVar_, *filterVar_;
	int multipleSelections;
};

const char *choose_file_simple( const char *title );

#endif





/**********     end of gfileChooser.h     **********/
