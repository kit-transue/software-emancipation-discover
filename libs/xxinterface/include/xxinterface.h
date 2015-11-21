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
#ifndef _xxinterface_h_
#define _xxinterface_h_

/* The different licenses */

typedef enum {
  LIC_NULL = 0,
  LIC_ACCESS,
  LIC_ADMIN,
  LIC_AUTODOC,
  LIC_CLIENT,
  LIC_C_CPP,
  LIC_DEBUG,
  LIC_DEFECTLINK_RO,
  LIC_DEFECTLINK_RW,
  LIC_DELTA,
  LIC_DESIGN,
  LIC_DFA,
  LIC_DISCOVER,
  LIC_DOCLINK_RO,
  LIC_DOCLINK_RW,
  LIC_DORMANT,
  LIC_EXTRACT,
  LIC_JAVA,
  LIC_METRICS,
  LIC_MODULARITY,
  LIC_PACKAGE,
  LIC_PARTITION,
  LIC_QAC,
  LIC_QFS_BROWSE,
  LIC_QFS_GLOBAL,
  LIC_QFS_MASTER,
  LIC_QFS_PORT,
  LIC_QFS_REPORT,
  LIC_QFS_RUN,
  LIC_QFS_STAND,
  LIC_QFS_STATS,
  LIC_QFS_STRUCT,
  LIC_QFS_TESTCOV,
  LIC_REPORTS,
  LIC_SIMPLIFY,
  LIC_SQL,
  LIC_STATISTICS,
  LIC_SUBCHECK,
  LIC_TASKFLOW,
  LIC_TESTLINK_RO,
  LIC_TESTLINK_RW,
  LIC_TPM,
  LIC_TPM_REMED,
  LIC_WEBVIEW,
  LIC_Y2K,
  LIC_CR_BUILD,
  LIC_CR_CALIPER,
  LIC_CR_INTEGRITY,
  LIC_CR_DEVXPRESS,
  LIC_CR_TPM,
  LIC_CR_TRENDS,
  LIC_CR_IMPACT,
  LIC_BUILD,
  LIC_DEVELOPER,
  LIC_QUALITY,
  LIC_ARCHITECT,
  LIC_SERVER,
  LIC_NUM_LICENSES
} license_id;

/* license return codes */

typedef enum
{
  LIC_SUCCESS = 0,
  LIC_FAIL = 1
} lic_retval;

/* Function prototypes for the license management functions */

#ifdef __cplusplus
extern "C"
{
#endif

  int _lc(void);
  int _lhb(void);
  int _le(void);
  int _lo(license_id);
  int _li(license_id);
  int _lf(license_id);
  int _lh(license_id);
  int _lm(license_id);
  int _los(const char *s);
  int _lis(const char *s); 
  int _lfs(const char *s); 
  int _lhs(const char *s); 
  int _lms(const char *s); 

#ifdef __cplusplus
}
#endif

#endif
