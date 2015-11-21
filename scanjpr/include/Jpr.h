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
/*---------------------------------------------------------------------------*/
/* File:   Jpr.h                                                             */
/* Descr.: Declarations for the objects that represent a JBuilder Project    */
/*                                                                           */
/* 09-14-00    Guillermo    $$1   Created                                    */
/*---------------------------------------------------------------------------*/

#ifndef JPR_H
#define JPR_H

#include <stdio.h>
#include <vector>
#include <string>

using namespace std;

class JBuilderProjectVariableValue;
class JBuilderProjectVariable;
class JBuilderProjectObject;
class JBuilderProject;

typedef vector<JBuilderProjectObject*>        JBuilderProjectObjects;
typedef vector<JBuilderProjectVariable*>      JBuilderProjectVariables;
typedef vector<JBuilderProjectVariableValue*> JBuilderProjectVariableValues;

class JBuilderProjectVariableValue
{
    private:
        string                    m_value;
        JBuilderProjectVariables  m_fields;

    public:
        JBuilderProjectVariableValue();
       ~JBuilderProjectVariableValue();

        void          SetValue( const string &value );
        const string& GetValue() const;

        void SetField( JBuilderProjectVariable *field );        
        JBuilderProjectVariable* 
            GetField( const string &field_name ) const;
        unsigned short GetNumberOfFields() const;

        void FPrint( FILE *fp, const string &prefix );
                
    private:
        unsigned short get_field_index( const string &field_name ) const;
};

class JBuilderProjectVariable
{
    private:  
        string                          m_name;
        JBuilderProjectVariableValues   m_values;
        bool                            m_is_array;

    public:
        JBuilderProjectVariable();
       ~JBuilderProjectVariable();

        void SetName( const string &name );
        const string& GetName() const;

        void SetValue( unsigned short                index, 
                       JBuilderProjectVariableValue *value );
        JBuilderProjectVariableValue* GetValue( unsigned short index ) const;

        void SetIsArray( bool is_array );
        bool IsArray() const;

        void FPrint( FILE *fp, const string &prefix );
};

class JBuilderProjectObject
{
    private:
        string  m_name;

    public:
        JBuilderProjectObject();
       ~JBuilderProjectObject();

        void SetName( const string &name );
        const string& GetName() const;

        bool IsJavaSourceFile() const;

        void FPrint( FILE *fp );
};

class JBuilderProject
{
    private:
        JBuilderProjectObjects      m_objects;
        JBuilderProjectVariables    m_variables;

    public:
        JBuilderProject();
       ~JBuilderProject(); 
       
        void SetProjObject( unsigned short         index, 
                            JBuilderProjectObject *object );
        const JBuilderProjectObject* GetProjObject( 
            unsigned short index ) const;

        void AddVariable( JBuilderProjectVariable *variable );
        JBuilderProjectVariable* 
            GetVariable( const string &var_name ) const;

        void FPrintObjects( FILE *fp );
        void FPrintVariables( FILE *fp );

    private:
        unsigned short get_variable_index( const string &var_name ) const;
};


#endif /* JPR_H */
