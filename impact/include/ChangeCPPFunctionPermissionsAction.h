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
#ifndef __CHANGECPPFUNCTIONPERMISSIONSACTION_HEADER_FILE
#define __CHANGECPPFUNCTIONPERMISSIONSACTION_HEADER_FILE

#include "ImpactAction.h"
#include "Permissions.h"

class CChangeCPPFunctionPermissionsAction : public CImpactAction,CPermissions {
 public:
  CChangeCPPFunctionPermissionsAction();
  ~CChangeCPPFunctionPermissionsAction();
  void setPermission(int permission) { m_nPermission = permission; };
  int getPermission() { return m_nPermission; };
  Widget GetAttributesArea(Widget parent, symbolPtr* pSym);
  virtual bool ActionPerformed(symbolPtr* pSym);

  Widget m_publicInd;
  Widget m_privateInd;
  Widget m_protectedInd;
  Widget m_constInd;
  Widget m_staticInd;
  Widget m_virtualInd;
 private:
  static void OnPermissionChanged(Widget widget,XtPointer client_data,XtPointer call_data); 
  int m_nPermission;
};

#endif //__CHANGECPPFUNCTIONPERMISSIONSACTION_HEADER_FILE
