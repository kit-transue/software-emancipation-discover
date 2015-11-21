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
#include <unistd.h>
#include <sys/wait.h>
#include "CMIntegrator.h"
#include "msg.h"


CCMIntegrator *
CCMIntegrator::theInstance = 0;


CCMIntegrator *
CCMIntegrator::getIntegrator()
{
	if ( !theInstance) {
		theInstance = new CCMIntegrator;
	}
	return theInstance;
}

void CCMIntegrator::removeIntegrator()
{
	if(theInstance != NULL) {
		delete theInstance;
		theInstance = NULL;
	}
}

CCMIntegrator::CCMIntegrator() :
	m_szCMName("Generic"),
	m_bConnected(false),
	driverPID(0)
{
	ConnectToService(true);
}

CCMIntegrator::~CCMIntegrator(){
}

void CCMIntegrator::OnIntegratorEvent() {
}

string CCMIntegrator::GetCMName() 
{
	return m_szCMName;
}

void CCMIntegrator::SetCMName(string& szName) 
{
	m_szCMName = szName;
}

string CCMIntegrator::GetIntegratorType() {
	return "CM";
}

string CCMIntegrator::GetIntegratorName() {
	return m_szCMName;
}

bool CCMIntegrator::RunDriver() {
	string execName;
	char* args[2];
	char* psethomeEnv;

	psethomeEnv = getenv("PSETHOME");
	if(psethomeEnv==NULL) {
		execName="/";
	} else {
		execName=psethomeEnv;
	}
	execName+="/bin/cmdriver";

	pid_t childID = fork();
	if(childID==0) {
		args[0] = (char*)execName.c_str();
		args[1] = NULL;
		execv(execName.c_str(),args);
	} else {
		driverPID = childID;
                msg("'cmdriver' has been started.", normal_sev) << eom;
        }
	return true;
}

string CCMIntegrator::GetCMs() 
{
	string strResult;
	if(IsAvailable()) {
		string cmd("get_cm_systems");
		if(m_ConnectionChannel->SendCommand(cmd))
			strResult = m_ConnectionChannel->GetReply();
	}
	return strResult;
}

string CCMIntegrator::GetCommands() 
{
	string strResult;
	if(IsAvailable() && IsConnected()) {
		string cmd("get_commands");
		if(m_ConnectionChannel->SendCommand(cmd))
			strResult = m_ConnectionChannel->GetReply();
	}
	return strResult;
}

bool CCMIntegrator::Connect(string& szName) 
{
	if(IsAvailable()) {
		string szCommand = "connect\t";
		szCommand+=szName;
		if(m_ConnectionChannel->SendCommand(szCommand)) {
			string strResult = m_ConnectionChannel->GetReply();
			m_bConnected = strResult.empty() || strResult.compare("done")==0;
			return m_bConnected;
		}
	}
	return false;
}

bool CCMIntegrator::IsConnected() 
{
	return m_bConnected;
}

bool CCMIntegrator::IsAvailable() 
{
	return m_ConnectionChannel!=NULL;
}

string CCMIntegrator::GetAttributes() 
{
	string strResult;
	if(IsAvailable() && IsConnected()) {
		string cmd("get_attributes");
		if(m_ConnectionChannel->SendCommand(cmd))
			strResult = m_ConnectionChannel->GetReply();
	}
	return strResult;
}

string CCMIntegrator::ExecCommand(string& szCommand) 
{
	string strResult;
	string szError("Error");
	if(IsAvailable() && IsConnected()) {
		if(m_ConnectionChannel->SendCommand(szCommand)) {
			strResult = m_ConnectionChannel->GetReply();
			if(szError.compare(strResult)==0) {
				strResult = "Error:"+m_ConnectionChannel->GetReply();
			}
		} else
			strResult = "Error:Communication error.";
	}
	return strResult;
}

bool CCMIntegrator::Reconnect() 
{
	return ConnectToService(true);
}

bool
CCMIntegrator::Unregister()
{
	bool ret = SUPER::Unregister();
	if (ret) {
		// this may hang, but we should wait for now.
		waitpid(driverPID, 0, 0);
	}
	return ret;
}

