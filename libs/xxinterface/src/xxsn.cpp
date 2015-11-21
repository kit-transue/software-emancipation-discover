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
#ifdef WIN32
// For registry functions
#include <Windows.h>
#include <tchar.h>
// what to do about version??
#define CIEIVERSION	"3.0"
#define CIEIKEY		"Software\\THECOMPANY\\Source Analysis\\" CIEIVERSION
#define CIEISERIAL	"SerialNumber"
#endif

// for artificial timeout function:
#include <time.h>

#include <stdlib.h>
#include <xxsn.h>

char serialErrorString[] = "Encountered error in validation of serial number";
static char regSerialNumber[11];

////////////////////////////////////////////////////////////////////////////////
// DIScover/DIScover
//	
#define CI		1
#define EI		2
#define CIEI		3
// functions needed:
//	current program:
//		bool checkoutFeature(flexlm_id/id);
//	serial number:
//		CI/EI/CIEI productLicense(); (serialno)
//	bridge:
//		bool isFeatureSupported(int); (CI/EI/CIEI)
//
static license_id CIFeatures[] = {
	LIC_CR_TRENDS,
	LIC_CR_CALIPER,
	LIC_NULL
};
static license_id EIFeatures[] = {
	LIC_CR_IMPACT,
	LIC_CR_TPM,
	LIC_TPM,
	LIC_NULL
};
static license_id CIEIFeatures[] = {
	LIC_ACCESS,
	LIC_ADMIN,
	LIC_CLIENT,
	LIC_C_CPP,
	LIC_JAVA,
	LIC_CR_BUILD,
	LIC_DISCOVER,
	LIC_NULL
};

int getFeatureType(license_id currID) {
	int index=0;
	// check CIEI features first
	while(CIEIFeatures[index] != LIC_NULL) {
		if(CIEIFeatures[index] == currID) {
			//msg("this is a CIEI feature\n") << eoarg << endl;
			return(CIEI);
		}
		index++;
	}
	index=0;
	while(CIFeatures[index] != LIC_NULL) {
		if(CIFeatures[index] == currID) {
			//msg("this is a CI feature\n") << eoarg << endl;
			return(CI);
		}
		index++;
	}
	index=0;
	while(EIFeatures[index] != LIC_NULL) {
		if(EIFeatures[index] == currID) {
			//msg("this is a EI feature\n") << eoarg << endl;
			return(EI);
		}
		index++;
	}
	return(LIC_FAIL);
}

char* getSerialNumber() {
	return regSerialNumber;
}

int CIsupported(void) {
	static int returnValue=LIC_FAIL, firstTime=1;
	if(firstTime) {
		if(getChecksum()) {
			// 2nd number is determining number
			if(regSerialNumber[1] == '1' || regSerialNumber[1] == '5') { returnValue = LIC_SUCCESS; }
		}
		firstTime = 0;
	}
	return(returnValue);	
}

int IsEnterprise(void) {
	static int returnValue=LIC_FAIL, firstTime=1;
	if(firstTime) {
		if(getChecksum()) {
			// 3rd number is determining number
			if(regSerialNumber[2] == '1' || regSerialNumber[2] == '3') { 
				returnValue = LIC_SUCCESS; 
			}
		}
		firstTime = 0;
	}
	return(returnValue);	
}

int EIsupported(void) {
	static int returnValue=LIC_FAIL, firstTime=1;
	if(firstTime) {
		if(getChecksum()) {
			// 2nd number is determining number
			if(regSerialNumber[1] == '3' || regSerialNumber[1] == '5') { returnValue = LIC_SUCCESS; }
		}
		firstTime = 0;
	}
	return(returnValue);	
}

int usingSerialNumber(void) {
	static int firstTime=1;
	static int retval = 0; // false

	#ifdef WIN32
	if(firstTime) {
		firstTime=0;
		int regRetVal;
		DWORD type=NULL;
		HKEY regKey=NULL;
		// our 10 digit string
		const int stringSize=11;
		TCHAR keyValue[stringSize];
		DWORD keySize=stringSize;
		regRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE, CIEIKEY, 0, KEY_READ, &regKey);
		if(regRetVal == ERROR_SUCCESS) {
			regRetVal = RegQueryValueEx(regKey, CIEISERIAL, 0, &type, (PBYTE)&keyValue, &keySize);
			if(regRetVal == ERROR_SUCCESS) {
				// succesfully opened key
				//printf("the serial number is %s\n", &keyValue);
				// Keep regSerialNumber around for CI/EI-supported
				_tcscpy(regSerialNumber,keyValue);
				retval=1;
			}
		}
	}
	#endif

	return(retval);

}

// before checkSerialNumber is called, usingSerialNumber() has to be called at least ONCE
int checkSerialNumber(license_id id) {
	int featureType, returnValue=LIC_FAIL;
#ifdef WIN32
	featureType = getFeatureType(id);
	switch(featureType) {
		case CIEI:
			if(CIsupported() == LIC_SUCCESS || EIsupported() == LIC_SUCCESS) { returnValue=LIC_SUCCESS; }
			//msg("case of CIEI ... returning $1") << eoarg << returnValue << endl;
			//printf("case of CIEI ... returning %d\n", returnValue);
			break;
		case CI:
			if(CIsupported() == LIC_SUCCESS) { returnValue=LIC_SUCCESS; }
			//msg("case of CI ... returning $1") << eoarg << returnValue << endl;
			//printf("case of CI ... returning %d\n", returnValue);
			break;
		case EI:
			if(EIsupported() == LIC_SUCCESS) { returnValue=LIC_SUCCESS; }
			//msg("case of EI ... returning $1") << eoarg << returnValue << endl;
			//printf("case of EI ... returning %d\n", returnValue);
			break;
		default:
			//printf("Failure to find that feature %d\n", featureType);
			//msg("Failure to find that feature $1") << featureType << eoarg << eom;
			returnValue=LIC_FAIL;
			break;
	}
#endif
	//printf("after switch in checkSerial\n");
	return(returnValue);
}

int getChecksum() {
	int index=0, returnValue=1;
	char currNumber;
	int serialNumber[11];
	struct tm expiration;
	time_t now, expire;

	while(regSerialNumber[index] != '\0' && returnValue == 1) {
		currNumber = regSerialNumber[index];
		serialNumber[index++] = currNumber - '0';
		if(index > 10) { returnValue = 0; }
	}

	if(serialNumber[2] == 5) { // if its a timebomb
		expiration.tm_year = serialNumber[3] + 100; // 100 years from 1900
		expiration.tm_mon = (serialNumber[4] * 10) + serialNumber[7] - 1; // -1 because months start at 0
		if(expiration.tm_mon > 11) { returnValue = 0; }
		expiration.tm_mday = (serialNumber[5] * 10) + serialNumber[9];
		if(expiration.tm_mday > 31 ) { returnValue = 0; }
		// unused fields
		expiration.tm_isdst = 0;
		expiration.tm_hour = 0;
		expiration.tm_min = 0;
		expiration.tm_sec = 0;
		expiration.tm_wday = 0;
		expiration.tm_yday = 0;
		expire = mktime(&expiration);
	
		time(&now);

		if(difftime(expire, now) <= 0.0) { returnValue = 0; } // if expire - now is less than 0
			
	}

	if(returnValue == 1) {
		// check first checksum digit
		int firstCheck;
		firstCheck = (((serialNumber[0] + 1) * (serialNumber[1] + 3))
			| ((serialNumber[2] + 2) * (serialNumber[3] + 7)))
				+ ((serialNumber[4] + 9) * (serialNumber[5] + 8));
		while (firstCheck >= 100) {
			firstCheck -= 100;
		}
		while (firstCheck >= 10) {
			firstCheck -= 10;
		}
		firstCheck = 9 - firstCheck;

		if (firstCheck != serialNumber[6]) {
			returnValue = 0;
		}

		if(returnValue == 1) {
			// check second checksum digit
			int secondCheck;
			secondCheck = (firstCheck + 4) - ((serialNumber[7] + 6) * (serialNumber[9] + 5));
			while (secondCheck <= -100) {
				secondCheck += 100;
			}
			while (secondCheck < 0) {
				secondCheck += 10;
			}
			while (secondCheck >= 100) {
				secondCheck -= 100;
			}
			while (secondCheck >= 10) {
				secondCheck -= 10;
			}

			if (secondCheck != serialNumber[8]) {
				returnValue = 0;
			}
		}
	}

	return(returnValue);
}
