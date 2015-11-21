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
// packageOjbs.h
// -------------
//

#ifndef _PACKAGEOBJS_H_
#define _PACKAGEOBJS_H_


class galaMonAppls : public updateApplList {

  public:

    galaMonAppls(const char *tag, char *session, int layer_id);
    virtual ~galaMonAppls();
    int GetLayerId(){return m_nLayerId;}

    virtual int update(parray *);
  private: 
    int m_nLayerId;
    vstr *m_Tag;

};

class galaMonCCls : public updateCClList {

  public:

    galaMonCCls(const char *tag, char *session, int layer_id);
    virtual ~galaMonCCls();
    int GetLayerId(){return m_nLayerId;}

    virtual int update(parray *);
  private: 
    int m_nLayerId;
    vstr *m_Tag;

};

class galaMonDCls : public updateDClList {

  public:

    galaMonDCls(const char *tag, char *session, int layer_id);
    virtual ~galaMonDCls();
    int GetLayerId(){return m_nLayerId;}

    virtual int update(parray *);
  private: 
    int m_nLayerId;
    vstr *m_Tag;

};

class galaMonElems : public updateElemsList {

  public:

    galaMonElems(const char *tag, char *session, int layer_id);
    virtual ~galaMonElems();
    int GetLayerId(){return m_nLayerId;}

    virtual int update(parray *);
  private: 
    int m_nLayerId;
    vstr *m_Tag;

};

class rqstStatGala:public rqstStat
{
  int m_nLayerId;
  int m_fAutoDiaplay;
  int updating_rqst;
public:
  rqstStatGala():rqstStat(){m_nLayerId = -1;m_fAutoDiaplay=0;updating_rqst=0;}
  
  virtual int statEnterRqst(char *);
  virtual int statEnterUpdate(char *);
  virtual int statWrRqstName(char *);
  virtual int statWrCompletion(double);
  virtual int statWrEstTime(double);
  virtual int statWrStatus(char *);
  virtual int statWrOutput(char *);
  virtual int statExitRqst(int);
  virtual char * statRdSession();

  void ChangeStatusState(int);
  
  void SetLayerId( int id ){ m_nLayerId = id; }
  void SetAuto( int display ){ m_fAutoDiaplay = display; }
  int IsAuto(){ return m_fAutoDiaplay; }
private:
  static const char *STATUS_MSG_TAG;
  static const char *OUTPUT_MSG_TAG;
  static const char *RQST_NAME_TAG;
  static const char *COMPL_PERCENT_TAG;
  static const char *STATUS_PROGRESS_TAG;
  static const char *COMPL_TIME_TAG;
};

class galaSessionOpen : public updateSOpen {
  int m_nLayerId;
  public:
    galaSessionOpen( int id ):updateSOpen(NULL){ m_nLayerId = id; }
    virtual int update(char *);

};

class galaSessionClose : public updateSClose {
  int m_nLayerId;
  public:
    galaSessionClose( int id ):updateSClose(NULL){ m_nLayerId = id; }
    virtual int update(char *);

};

class galaSessionRemove : public updateSRemove {
  int m_nLayerId;
  public:
    galaSessionRemove( int id ):updateSRemove(NULL){ m_nLayerId = id; }
    virtual int update(char *);

};


class tmpFDEleteCbk: public uicbck
{
  char *m_pszFileName;
public:
  tmpFDEleteCbk(const char *);
  virtual ~tmpFDEleteCbk();
  virtual long observe(int);

};

static const char *PACK_APP_LIST = "pkApplList";
static const char *PACK_REP_LIST = "rptElementsList";

vdialogItem *LayerFindItem( int nLayerId, const char *tag );
int vlistitem_GetStrSelection(vlistitem * list, parray * str_arr);


#endif
