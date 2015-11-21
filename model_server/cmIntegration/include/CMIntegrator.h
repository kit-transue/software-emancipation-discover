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
#ifndef __CMINTEGRATOR_HEADER_FILE
#define __CMINTEGRATOR_HEADER_FILE

#include "Integrator.h"
#include "IntegratorSocket.h"

class CCMIntegrator : public CIntegrator {
	typedef CIntegrator SUPER;
public:
	CCMIntegrator();
	~CCMIntegrator();

	virtual void OnIntegratorEvent(void);

	string GetCMName();
	void SetCMName(string& szName);
	string GetCMs();
	string GetCommands();
	bool Connect(string& szName);
	bool IsConnected();
	bool IsAvailable();
	string GetAttributes();
	string ExecCommand(string& szCommand);
	bool Reconnect();
	virtual bool Unregister();
	static CCMIntegrator * getIntegrator();
	static void removeIntegrator();

protected:
	virtual bool RunDriver();
	virtual string GetIntegratorType();
	virtual string GetIntegratorName();
private:
	string m_szCMName;
	bool m_bConnected;
	pid_t driverPID;

	static CCMIntegrator * theInstance;
};

#endif //__CMINTEGRATOR_HEADER_FILE
