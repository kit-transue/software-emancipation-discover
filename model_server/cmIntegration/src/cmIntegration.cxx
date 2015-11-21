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
#include "CMIntegrator.h"
#include "CMCommand.h"

int main(int argc,char* argv[]) {
	CCMIntegrator integrator;

	string szCMs = integrator.GetCMs();
	if(!szCMs.empty()) {
		int nStart = 0;
		int nEnd = 0;
		while((nEnd = szCMs.find('\n',nStart)) > 0) {
			string szName = szCMs.substr(nStart,nEnd-nStart);  
			printf("%s\n",szName.c_str());
			if(nStart == 0) {
				if(integrator.Connect(szName)) {
					// retriving CM attributes...
					CCMAttributes* m_pCMAttributes = new CCMAttributes(integrator.GetAttributes());
					delete m_pCMAttributes;

					// retriving CM commands...
					CCMCommandsList* m_pCMCommands = new CCMCommandsList(integrator.GetCommands());
					delete m_pCMCommands;
				}
			}
			nStart = nEnd+1;
		}
		printf("%s\n",szCMs.substr(nStart).c_str());
	}			
	
	return 0;
}
