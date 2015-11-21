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
#include "util.h"
#include "xxsn.h"

#include <time.h>
#ifdef ISO_CPP_HEADERS
#include <sstream>
#else
#include <strstream.h>
#endif
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "md5.h"

#include "msg.h"
namespace MBDriver {};
using namespace MBDriver;
using namespace std;

namespace MBDriver {

static const unsigned int HAS_LICENSE_FILE 	= 01;
static const unsigned int HAS_BUILD		= 02;
static const unsigned int HAS_MODEL_CLIENT	= 04;
static const unsigned int HAS_INTEGRITY	= 010;
static const unsigned int HAS_CALIPER		= 020;

unsigned int get_license_mode() {
	static unsigned int ret_val = 0;
	static bool first_time = true;

	if(first_time) {
		cerr << "get_license_mode first time" << endl;
		first_time = false;
		string errmsg;
		if(check_lm_license_file(errmsg) && (_lc() == 0)) {
			ret_val |= HAS_LICENSE_FILE;
			if(_lf(LIC_CR_BUILD) == 0) {
				if((_lf(LIC_C_CPP) == 0) || (_lf(LIC_JAVA) == 0)) {
					ret_val |= HAS_BUILD;
				}
			}

			if(_lf(LIC_CLIENT) == 0) {
				ret_val |= HAS_MODEL_CLIENT;
			}

			if(_lf(LIC_DISCOVER) == 0) {
				// Need Discover license to start pset_server
				if(_lf(LIC_CR_INTEGRITY) == 0 ) {
					ret_val |= HAS_INTEGRITY;
				}

				if(_lf(LIC_CR_CALIPER) == 0) {
					ret_val |= HAS_CALIPER;
				}
			}
		}
		cerr << "get_license_mode first time complete" << endl;
	}
	return ret_val;
}

bool hasCaliperLicense()
{
    unsigned int lic = get_license_mode();
    return (lic & HAS_CALIPER) ? true : false;
}

//=============================================================================
void sleep_in_seconds(unsigned seconds)
{
#ifdef _WIN32
    _sleep(seconds*1000);
#else
    sleep(seconds);
#endif
    return;
}

// ===========================================================================
string &psethome()
{
    static string *str = NULL;
    if (str == NULL) {
		const char *envval = getenv("PSETHOME");
		str = new string(envval != NULL ? envval : "");
		{
			// Set PSETHOME, so that shared Discover code will use the
			// CodeRover installation, not the Discover installation.
			string *set = new string("PSETHOME=");
			set->append(*str);
			putenv((char *)set->c_str());
		}
    }
    return *str;
}

// ===========================================================================
// Return true if LM_LICENSE_FILE is OK, false if it is bad
//
bool check_lm_license_file(string &errmsg)
{
    static bool first_time = true;
    static bool ret_val = true;

    if ( first_time && !usingSerialNumber())
    {
	first_time = false;
	const char *envval = getenv("LM_LICENSE_FILE");
	errmsg = "Environment variable LM_LICENSE_FILE is not set.";
	if(envval != NULL) {
	    ret_val = true;
	} else {
	    ret_val = false;
	}
    }
    return ret_val;
}

//=====================================================================
//Uses MD5 message digest algorithm to calculate checksum.
//
static unsigned int calculate_checksum(const string& filepath, const string& model_dir_path)
{
    //Initialize checksum context
    MD5_CTX context;
    MD5Init(&context);


    unsigned int len = filepath.length();
    MD5Update(&context, (unsigned char *)filepath.c_str(), len);
    len = model_dir_path.length();
    MD5Update(&context, (unsigned char *)model_dir_path.c_str(), len);


    //Caculate checksum
    unsigned char digest[16];
    MD5Final(digest, &context);

    //return only the first byte (sufficient for now...)
    unsigned int check_sum = digest[0];
    return check_sum;
}

//converts a string to lower case and returns it.
static string tolower_string(const string& old_str)
{
    string new_str = old_str;
    int length = new_str.length();
    for (int i = 0; i < length; i++) {
	new_str[i] = tolower(new_str[i]);
    }
    return new_str;
}

//======================================================================
//

//model_server adds "Dislite:" to the service name when registering 
//itself with name_server. Thus while querying name_server "Dislite:"
// should be prepended to the service_name.
string get_internal_service_name(const string& name)
{
    string internal_name("Dislite:");
    internal_name.append(name);
    return internal_name;
}

//======================================================================
//Two things are appended to come up with a service name:
// First,  name of the dsp/dsw file(without the whole path).
// Second, checksum calculated using the complete dsp/dsw path and the 
//         model path($ADMINDIR path). The checksum will help resolve
//         conflicts occuring from two files having same name but with
//         different paths and models built using same dsp file in
//         different places.
//
string get_service_name(const string *dsp_path, const string *output_path)
{
    string filepath;
    string outdir;

    if (dsp_path) {
	filepath = dsp_path->c_str();
    }
    if (output_path) {
	outdir = output_path->c_str();
    }

#ifdef _WIN32
    filepath = tolower_string(filepath);
    outdir = tolower_string(outdir);
#endif

    string pmodservice;
    int pos = filepath.find_last_of("/\\"); //strip directories from dsp name
    int len = filepath.size();
    if (len > ++pos + 4) {
	pmodservice = filepath.substr(pos, len-pos-4); //strip .dsp from dsp name
	int l = pmodservice.size();
	for (int i= 0; i<l; i++) {
	    if (isspace(pmodservice[i]))
		pmodservice[i] = '_';
	}
    }
    if (pmodservice.empty())
	pmodservice = "NONAME";

    //Calculate checksum using dsp/dsw file path and model dir path.
    unsigned int checksum_val = calculate_checksum(filepath, outdir);

    //Add checksum value to service name.
    char checksum_str[10];
    sprintf(checksum_str, "%d", checksum_val);
    pmodservice += checksum_str;

    return pmodservice;
}

}  // namespace MBDriver

