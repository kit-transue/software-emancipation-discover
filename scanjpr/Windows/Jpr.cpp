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
/* File:   Jpr.cpp                                                           */
/* Descr.: Implementation for the objects that represent a JBuilder Project  */
/*                                                                           */
/* 09-15-00    Guillermo    $$1   Created                                    */
/*---------------------------------------------------------------------------*/

#include "jprlex.h" //just for general declarations such as strcasecmp
#include "jpr.h"

/*---------------------------------------------------------*
 *    Functions for class JBuilderProjectVariableValue     *
 *---------------------------------------------------------*/
JBuilderProjectVariableValue :: 
    JBuilderProjectVariableValue()
{
    m_value = "";
}

JBuilderProjectVariableValue :: 
   ~JBuilderProjectVariableValue()
{
    unsigned short numb_fields;
    unsigned short i;

    numb_fields = m_fields.size();
    for( i=0; i<numb_fields; i++ )
        delete m_fields[i];
}

void JBuilderProjectVariableValue :: 
    SetValue( const string &value )
{
    m_value = value;
}

const string& JBuilderProjectVariableValue :: 
    GetValue() const
{
    return m_value;
}

void JBuilderProjectVariableValue :: 
    SetField( JBuilderProjectVariable *field )
{
    unsigned short field_index;

    if( field->GetName().length() == 0 )
        return;

    field_index = get_field_index( field->GetName() );
    if( field_index == (unsigned short)-1 )
    {
        /*
         * New Field
         */
        m_fields.push_back( field );
    }
    else
    {
        /*
         * Field already exists
         */
        if( field != m_fields[field_index] )
        {
            delete m_fields[field_index];
            m_fields[field_index] = field;
        }
    }
}

JBuilderProjectVariable* JBuilderProjectVariableValue :: 
    GetField( const string &field_name ) const
{
    unsigned short field_index;

    field_index = get_field_index( field_name );
    if( field_index != (unsigned short)-1 )
        return m_fields[field_index];
    else
        return NULL;
}

unsigned short JBuilderProjectVariableValue :: 
    GetNumberOfFields() const
{
    return m_fields.size();
}

void JBuilderProjectVariableValue :: 
    FPrint( FILE *fp, const string &prefix )
{
    unsigned short numb_fields;
    unsigned short i;

    numb_fields = m_fields.size();

    if( numb_fields == 0 )
    {
        fprintf( fp, "%s=%s\n", prefix.c_str(), m_value.c_str() );
    }
    else
    {
        for( i=0; i<numb_fields; i++ )
            m_fields[i]->FPrint( fp, prefix );
    }
}

unsigned short JBuilderProjectVariableValue :: 
    get_field_index( const string &field_name ) const
{
    unsigned short numb_fields;
    unsigned short i;

    numb_fields = m_fields.size();
    for( i=0; i<numb_fields; i++ )    
        if( strcasecmp(field_name.c_str(), 
                       m_fields[i]->GetName().c_str()) == 0 )
            return i;

    return (unsigned short)-1;
}


/*---------------------------------------------------------*
 *    Functions for class JBuilderProjectVariable          *
 *---------------------------------------------------------*/
JBuilderProjectVariable :: 
    JBuilderProjectVariable()
{
    m_name     = "";
    m_is_array = false;
}

JBuilderProjectVariable :: 
   ~JBuilderProjectVariable()
{
    unsigned short numb_values;
    unsigned short i;

    numb_values = m_values.size();
    for( i=0; i<numb_values; i++ )
        delete m_values[i];
}

void JBuilderProjectVariable :: 
    SetName( const string &name )
{
    m_name = name;
}

const string& JBuilderProjectVariable :: 
    GetName() const
{
    return m_name;
}

void JBuilderProjectVariable :: 
    SetValue( unsigned short                index, 
              JBuilderProjectVariableValue *value )
{
    unsigned short numb_values;
    unsigned short i;

    if( index > 0 )
        SetIsArray( true );

    numb_values = m_values.size();

    if( index >= numb_values )
    {
        /*
         * If the index is larger than the number of values, fill in all the
         * values in between with empty strings
         */
        for( i=numb_values; i<index; i++ )
            m_values.push_back( new JBuilderProjectVariableValue );

        m_values.push_back( value );
    }
    else if( value != m_values[index] )
    {
        delete m_values[index];
        m_values[index] = value;
    }
}
        
JBuilderProjectVariableValue* JBuilderProjectVariable :: 
    GetValue( unsigned short index ) const
{
    if( index >= m_values.size() )
        return NULL;
    else 
        return m_values[index];
}

void JBuilderProjectVariable :: 
    SetIsArray( bool is_array )
{
    m_is_array = is_array;
}

bool JBuilderProjectVariable :: 
    IsArray() const
{
    return m_is_array;
}

void JBuilderProjectVariable :: 
    FPrint( FILE *fp, const string &prefix )
{
    unsigned short numb_values;
    unsigned short i;
    string         new_prefix;
    string         indexed_new_prefix;
    char           index_str[32];

    numb_values = m_values.size();
    if( numb_values == 0 )
        return;
    
    if( prefix.length() == 0 )
        new_prefix = m_name;
    else    
        new_prefix = prefix + "." + m_name;

    if( !m_is_array )
        m_values[0]->FPrint( fp, new_prefix );
    else
    {
        for( i=0; i<numb_values; i++ )
        {
            if( m_values[i]->GetValue().length() > 0 ||
                m_values[i]->GetNumberOfFields() > 0 )
            {
                sprintf( index_str, "%d", i );
                indexed_new_prefix = new_prefix + "[" + index_str + "]";

                m_values[i]->FPrint( fp, indexed_new_prefix );
            }
        }
    }
}

/*---------------------------------------------------------*
 *    Functions for class JBuilderProjectObject            *
 *---------------------------------------------------------*/
JBuilderProjectObject :: 
    JBuilderProjectObject()
{
    m_name = "";
}

JBuilderProjectObject :: 
   ~JBuilderProjectObject()
{}

void JBuilderProjectObject :: 
    SetName( const string &name )
{
    m_name = name;
}

const string& JBuilderProjectObject :: 
    GetName() const
{
    return m_name;
}

bool JBuilderProjectObject :: 
    IsJavaSourceFile() const
{
    bool        result;
    char       *name_copy;
    const char *substr;

    result = false;

    name_copy = new char [m_name.length()+1];
    strcpy( name_copy, m_name.c_str() );

    substr = strstr( name_copy, ".java" );
    if( substr != NULL )
    {
        if( strcasecmp(substr, ".java") == 0 )
            result = true;;
    }

    delete [] name_copy;

    return result;
}

void JBuilderProjectObject :: 
    FPrint( FILE *fp )
{
    fprintf( fp, "%s", m_name.c_str() );
}

/*---------------------------------------------------------*
 *          Functions for class JBuilderProject            *
 *---------------------------------------------------------*/
JBuilderProject :: 
    JBuilderProject()
{}

JBuilderProject :: 
   ~JBuilderProject()
{
    unsigned short numb_objects;
    unsigned short numb_variables;
    unsigned short i;

    numb_objects   = m_objects.size();
    numb_variables = m_variables.size();

    for( i=0; i<numb_objects; i++ )
        delete m_objects[i];

    for( i=0; i<numb_variables; i++ )
        delete m_variables[i];
}

void JBuilderProject :: 
    SetProjObject( unsigned short         index, 
                   JBuilderProjectObject *object )
{
    unsigned short numb_objects;
    unsigned short i;

    numb_objects = m_objects.size();

    if( index >= numb_objects )
    {
        /*
         * If the index is larger than the number of objects, fill in all the
         * values in between with empty objects
         */
        for( i=numb_objects; i<index; i++ )
            m_objects.push_back( new JBuilderProjectObject );

        m_objects.push_back( object );
    }
    else if( object != m_objects[index] )
    {
        delete m_objects[index];
        m_objects[index] = object;
    }
}

const JBuilderProjectObject* JBuilderProject :: 
    GetProjObject( unsigned short index ) const    
{
    if( index >= m_objects.size() )
        return NULL;
    else
        return m_objects[index];
}

void JBuilderProject :: 
    AddVariable( JBuilderProjectVariable *variable )
{
    unsigned short variable_index;

    if( variable->GetName().length() == 0 )
        return;

    variable_index = get_variable_index( variable->GetName() );
    if( variable_index == (unsigned short)-1 )
    {
        /*
         * New Variable
         */
        m_variables.push_back( variable );
    }
    else if( m_variables[variable_index] != variable )
    {
        delete m_variables[variable_index];
        m_variables[variable_index] = variable;
    }
}

JBuilderProjectVariable* JBuilderProject :: 
    GetVariable( const string &var_name ) const
{
    unsigned short variable_index;

    variable_index = get_variable_index( var_name );
    if( variable_index != (unsigned short)-1 )
        return m_variables[variable_index];
    else
        return NULL;
}

unsigned short JBuilderProject :: 
    get_variable_index( const string &var_name ) const
{
    unsigned short numb_variables;
    unsigned short i;

    numb_variables = m_variables.size();
    for( i=0; i<numb_variables; i++ ) {
        const char* tableName  = m_variables[i]->GetName().c_str();
		const char* searchName =  var_name.c_str();
        if( strcasecmp(searchName, tableName) == 0 ) return i;
	}

    return (unsigned short)-1;
}

void JBuilderProject :: 
    FPrintObjects( FILE *fp )
{
    unsigned short numb_objects;
    unsigned short i;

    numb_objects = m_objects.size();
    for( i=0; i<numb_objects; i++ )
    {
        if( m_objects[i]->GetName().length() > 0 )
        {
            fprintf( fp, "#%d=", i );
            m_objects[i]->FPrint( fp );
            fprintf( fp, "\n" );
        }
    }
}

void JBuilderProject :: 
    FPrintVariables( FILE *fp )
{
    unsigned short numb_variables;
    unsigned short i;

    numb_variables = m_variables.size();
    for( i=0; i<numb_variables; i++ )
    {
        m_variables[i]->FPrint( fp, "" );        
    }
}
