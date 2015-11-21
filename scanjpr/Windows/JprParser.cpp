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
/* File:   JprParser.cpp                                                     */
/* Descr.: Implementation for the JBuilder Project (,jpr) Parser             */
/*                                                                           */
/* 09-14-00    Guillermo    $$1   Created                                    */
/* 09-19-00    Guillermo    $$2   Integrated with messaging system           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "JprParser.h"
#include "LibraryParser.h"
#include "msg.h"

/*--------------------------------------------------------------------*
 *           Support local data structures and functions              *
 *--------------------------------------------------------------------*/

#define MAX_LINE_LENGTH     4095

typedef struct input_file
{
    FILE           *fp;
    unsigned long   line_number;
    char            current_line[MAX_LINE_LENGTH+1];
    char           *current_line_pos;
} FILe;

typedef struct parse_error
{
    int         error_number;
    char       *error_msg;
    bool        show_error_line;
} ParseError;

static ParseError parse_error_table[] = 
{
    {   1,  "Numeric constant expected",                             true  },
    {   2,  "Identifier or number expected",                         true  },
    {   3,  "Closing square bracket expected",                       true  },
    {   4,  "Assignment operator expected",                          true  }  
};

static unsigned parse_error_table_size = sizeof( parse_error_table ) /
                                         sizeof( ParseError );

void parser_error( FILe               *fp_in, 
                   char              **string_in_out, 
                   char               *token_str,
                   FILE               *error_log,
                   int                 error_number,
                   const char         *additional_msg = NULL )
{
    ParseError *error;
    unsigned    i;

    if( error_log == NULL )
        return;

    error = NULL;

    for( i=0; i<parse_error_table_size; i++ )
    {
        if( error_number == parse_error_table[i].error_number )
        {
            error = &parse_error_table[i];
            break;
        }
    }

    if( error == NULL )
        return;

    if( fp_in != NULL && error->show_error_line )
    {
        /*
         * These are the only messages that we send to the messaging
         * system; the user would not be interested in looking at any of
         * the other stuff that we could show them
         */
        if( additional_msg == NULL )
            msg( "Line $1: $2", error_sev ) 
                << fp_in->line_number
                << eoarg
                << error->error_msg 
                << eoarg
                << eom;
        else
            msg( "Line $1: $2 $3", error_sev )
                << fp_in->line_number
                << eoarg 
                << error->error_msg
                << eoarg
                << additional_msg
                << eoarg
                << eom;
    }
    else
    {
        if( additional_msg == NULL )
            fprintf( error_log, "! %s\n", error->error_msg );
        else
            fprintf( error_log, "! %s %s\n", error->error_msg,
                     additional_msg );
    }

	if( error->show_error_line )
	{
		fprintf( error_log, "  Approximately before:  " );

        if( token_str == NULL )
		{
            if( *string_in_out != NULL )
                fprintf( error_log, "%s\n", *string_in_out );
            else if( fp_in != NULL && fp_in->current_line_pos != NULL )
                fprintf( error_log, "%s\n", fp_in->current_line_pos );
		}
        else
		{
            if( *string_in_out != NULL )
                fprintf( error_log, "%s %s\n", token_str, 
				         *string_in_out );
            else if( fp_in != NULL && fp_in->current_line_pos != NULL )
                fprintf( error_log, "%s %s\n", token_str, 
				         fp_in->current_line_pos );
            else
                fprintf( error_log, "%s\n", token_str );
		}
    }
}

static short get_token( FILe  *fp_in, 
                        char **string_in_out,
                        char **out_info, 
                        long  *out_info_size )
{
    short token;

    if( fp_in != NULL )
    {
        while( !feof(fp_in->fp) && 
                fp_in->current_line_pos[0] == (char)0 )
        {
            if( fgets(fp_in->current_line, MAX_LINE_LENGTH, fp_in->fp) != NULL )
            {
                fp_in->current_line_pos = fp_in->current_line;
                fp_in->line_number++;
            }
        }

        if( fp_in->current_line_pos[0] == (char)0 )
            token = TOKEN_EOF;
        else
        {
            token = _jpr_get_token( fp_in->current_line_pos, 
                                    &fp_in->current_line_pos,
                                    out_info, out_info_size );
            if( token == TOKEN_EOS )
                token = get_token( fp_in, string_in_out, out_info, 
                                   out_info_size );
        }
    }
    else
    {
        token = _jpr_get_token( *string_in_out, string_in_out, 
                                out_info, out_info_size );
        if( token == TOKEN_EOS )
            token = TOKEN_EOF;
    }
    return token;
}


/*--------------------------------------------------------------------*
 *                   Actual parsing functions                         *
 *--------------------------------------------------------------------*/
static bool jpr_object_assignment_parse( 
    FILe             *fp_in, 
    char            **string_in_out, 
    FILE             *error_log,
    JBuilderProject  *jbuilder_proj,
    short            *in_out_token,
    char            **in_out_token_str,
    long             *in_out_token_str_size )
{
    bool                   status;
    unsigned short         obj_index;
    const char            *obj_name;
    JBuilderProjectObject *obj;

    status = true;

    if( *in_out_token == TOKEN_NUMBER )
    {
        obj_index = atoi( *in_out_token_str );

        *in_out_token = get_token( fp_in, string_in_out, 
                                   in_out_token_str, 
                                   in_out_token_str_size );

        if( *in_out_token == TOKEN_ASSIGN )
        {
            _jpr_set_treat_as_string_to_eol( true );

            *in_out_token = get_token( fp_in, string_in_out, 
                                       in_out_token_str, 
                                       in_out_token_str_size );

            _jpr_set_treat_as_string_to_eol( false );

            obj_name = *in_out_token_str;

            obj = new JBuilderProjectObject;
            obj->SetName( obj_name );

            jbuilder_proj->SetProjObject( obj_index, obj );

            *in_out_token = get_token( fp_in, string_in_out, 
                                       in_out_token_str, 
                                       in_out_token_str_size );
        }
        else
        {
            status = false;
            parser_error( fp_in, string_in_out, *in_out_token_str, error_log, 
                          4 );
        }
    }
    else
    {
        status = false;
        parser_error( fp_in, string_in_out, *in_out_token_str, error_log, 1 );
    }

    return status;
}

static bool jpr_variable_assignment_parse( 
    FILe             *fp_in, 
    char            **string_in_out, 
    FILE             *error_log,
    JBuilderProject  *jbuilder_proj,
    short            *in_out_token,
    char            **in_out_token_str,
    long             *in_out_token_str_size )
{
    bool                          status;
    JBuilderProjectVariable      *variable;
    unsigned short                variable_value_index;
    JBuilderProjectVariableValue *variable_value;
    JBuilderProjectVariable      *variable_field;
    const char                   *var_name;
    bool                          stop;
    bool                          variable_is_array;

    status               = true;
    variable             = NULL;
    variable_value_index = 0;
    stop                 = false;
    variable_is_array    = false;

    do
    {
        if( *in_out_token == TOKEN_IDENT || *in_out_token == TOKEN_NUMBER )
        {
            var_name = *in_out_token_str;

            if( variable == NULL )
            {
                /*
                 * We are at the project level, we need to search for a
                 * variable with the proper name in the project scope
                 */
                variable = jbuilder_proj->GetVariable( var_name );
                if( variable == NULL )
                {
                    /*
                     * First time we see this variable in the project
                     */
                    variable = new JBuilderProjectVariable;
                    variable->SetName( var_name );

                    jbuilder_proj->AddVariable( variable );
                }
            }
            else
            {
                /*
                 * We are within the scope of another variable, so this is
                 * just a field of this other variable
                 */
                if( variable_is_array )
                    variable->SetIsArray( true );

                variable_value = variable->GetValue( variable_value_index );

                if( variable_value == NULL )
                {
                    /*
                     * No such value for this variable yet
                     */
                    variable_value = new JBuilderProjectVariableValue;

                    variable->SetValue( variable_value_index, variable_value );
                }

                variable_field = variable_value->GetField( var_name );
                if( variable_field == NULL )
                {
                    /*
                     * No such field for this variable value yet
                     */
                    variable_field = new JBuilderProjectVariable;
                    variable_field->SetName( var_name );

                    variable_value->SetField( variable_field );
                }

                variable = variable_field;
            }

            *in_out_token = get_token( fp_in, string_in_out, 
                                       in_out_token_str, 
                                       in_out_token_str_size );

            if( *in_out_token == TOKEN_OPENSQB )
            {
                *in_out_token = get_token( fp_in, string_in_out, 
                                           in_out_token_str, 
                                           in_out_token_str_size );

                if( *in_out_token == TOKEN_NUMBER )
                {
                    variable_value_index = atoi( *in_out_token_str );

                    *in_out_token = get_token( fp_in, string_in_out, 
                                               in_out_token_str, 
                                               in_out_token_str_size );

                    if( *in_out_token == TOKEN_CLOSESQB )
                        *in_out_token = get_token( fp_in, string_in_out, 
                                                   in_out_token_str, 
                                                   in_out_token_str_size );
                    else
                    {
                        status = false;
                        parser_error( fp_in, string_in_out, *in_out_token_str, 
                                      error_log, 3 );
                    }
                }

                variable_is_array = true;
            }
            else
            {
                variable_value_index = 0;
                variable_is_array    = false;
            }

            if( status == true )
            {
                if( *in_out_token == TOKEN_DOT )            
                    *in_out_token = get_token( fp_in, string_in_out, 
                                               in_out_token_str, 
                                               in_out_token_str_size );
                else
                    stop = true;
            }
        }
        else
        {
            status = false;
            parser_error( fp_in, string_in_out, *in_out_token_str, error_log, 
                          2 );
        }
    }
    while( status == true && !stop );

    if( status == true )
    {
        if( *in_out_token == TOKEN_ASSIGN )
        {
            _jpr_set_treat_as_string_to_eol( true );

            *in_out_token = get_token( fp_in, string_in_out, 
                                       in_out_token_str, 
                                       in_out_token_str_size );

            _jpr_set_treat_as_string_to_eol( false );

            variable_value = variable->GetValue( variable_value_index );
            if( variable_value == NULL )
            {
                variable_value = new JBuilderProjectVariableValue;
                variable->SetValue( variable_value_index, variable_value );
            }

            variable_value->SetValue( *in_out_token_str );

            *in_out_token = get_token( fp_in, string_in_out, 
                                       in_out_token_str, 
                                       in_out_token_str_size );

        }
        else
        {
            status = false;
            parser_error( fp_in, string_in_out, *in_out_token_str, error_log, 
                          4 );
        }
    }

    return status;
}

static bool jpr_statement_parse( FILe             *fp_in, 
                                 char            **string_in_out, 
                                 FILE             *error_log,
                                 JBuilderProject  *jbuilder_proj,
                                 short            *in_out_token,
                                 char            **in_out_token_str,
                                 long             *in_out_token_str_size )
{
    bool                  status;

    status = true;

    if( *in_out_token == TOKEN_POUND )
    {
        /*
         * Object Assignment
         */
        *in_out_token = get_token( fp_in, string_in_out, 
                                   in_out_token_str, 
                                   in_out_token_str_size );

        status = jpr_object_assignment_parse( fp_in, string_in_out, error_log,
                                              jbuilder_proj, in_out_token,
                                              in_out_token_str,
                                              in_out_token_str_size );
    }
    else
    {
        /*
         * Variable assignment
         */
        status = jpr_variable_assignment_parse( fp_in, string_in_out, error_log,
                                                jbuilder_proj, in_out_token,
                                                in_out_token_str,
                                                in_out_token_str_size );
    }    
                                              
    return status;
}


bool jpr_parse( FILE             *fp_in, 
                char             *string_in, 
                FILE             *error_log,
                JBuilderProject **out_jbuilder_proj )   
{
    short                 token;
    bool                  status;
    FILe                 *fp_struct;
    char                 *token_str;
    long                  token_str_size;    

    if( (fp_in == NULL && string_in == NULL) || out_jbuilder_proj == NULL )
    {
        return false;
    }

    fp_struct = NULL;

    if( fp_in != NULL )
    {
        fp_struct                   = new FILe;
        fp_struct->fp               = fp_in;
        fp_struct->line_number      = 0;
        fp_struct->current_line[0]  = (char)0;
        fp_struct->current_line_pos = fp_struct->current_line;
    }

    *out_jbuilder_proj = new JBuilderProject;

    token_str      = NULL;
    token_str_size = 0;
    
    status = true;
    token  = get_token( fp_struct, &string_in, &token_str, &token_str_size );

    while( token != TOKEN_EOF )    
    {        
        status = jpr_statement_parse( fp_struct, &string_in, error_log,
                                      *out_jbuilder_proj, &token, &token_str, 
                                      &token_str_size );

        if( status == false )
            while( token != TOKEN_POUND && token != TOKEN_NUMBER && 
                   token != TOKEN_IDENT )
                token = get_token( fp_struct, &string_in, &token_str, 
                                   &token_str_size );
    }

    if( fp_struct != NULL )
        delete fp_struct;

    if( token_str != NULL )
        delete [] token_str;

    if( status == false )
    {
        delete *out_jbuilder_proj;

        *out_jbuilder_proj = NULL;
    }

    return status;
}

bool lib_parse( const char       *fp_in, 
                char             *string_in, 
                FILE             *error_log,
                JBuilderProject  *out_jbuilder_proj )   
{
    if((fp_in == NULL) || (out_jbuilder_proj == NULL)) {
        return false;
    }

	LibraryParser parser(fp_in);
	parser.parse(out_jbuilder_proj);
	return parser.success();
}
