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
/* File:   JprParseMain.cpp                                                  */
/* Descr.: Main program for the JBuilder Project (,jpr) parser.              */
/*                                                                           */
/* 09-15-00    Guillermo    $$1   Created                                    */
/* 09-19-00    Guillermo    $$2   Several fixes and improvements             */
/* 09-21-00    Guillermo    $$3   Added processing of JBuilder Properties    */
/*                                file                                       */
/* 09-26-00    Guillermo    $$4   Use %USERPROFILE% env. variable instead of */
/*                                SDK function                               */
/* 09-27-00    Guillermo    $$5   Make sure jdk components exist             */
/* 09-27-00    Guillermo    $$6   Ignore CLASSPATH environment variable      */
/* 09-27-00    Guillermo    $$7   Make sure libraries exist					 */
/* 03-14-01	   Dmitry       $$8   JBuilder 4.0 support added                 */
/*---------------------------------------------------------------------------*/

#ifdef _WIN32
#pragma warning (disable:4786)
#endif

#include <stdio.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include "JprParser.h"
#include "msg.h"

#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#include <direct.h>
#ifdef _HAS_SDK
    #include <userenv.h>
#endif /*_HAS_SDK*/
#endif /*WIN32*/

static void display_usage()
{
    msg(
    "Usage: JprParser <jprFile> --output <outFile> --message_group <id> --lp_service <name>",
    catastrophe_sev) << eom;
}

static int getJBuilderVersion() {
    LONG     reg_status;
    HKEY     key1;
    HKEY     key2;
    HKEY     key3;
    HKEY     key4;
    BYTE     reg_data[1024];
    BYTE     versionStr[1024];
    DWORD    reg_data_size = 1023;
	int		 version = -1;

	char* jbuilderPath = getenv("CR_JBUILDER_PATH");
    reg_status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE", 0, 
                               KEY_READ, &key1 );
    if( reg_status == ERROR_SUCCESS ) {
        reg_status = RegOpenKeyEx( key1, "Borland", 0, KEY_READ, 
                                   &key2 );
		if( reg_status == ERROR_SUCCESS ) {
            reg_status = RegOpenKeyEx( key2, "JBuilder", 0,
                                       KEY_READ, &key3 );
            if( reg_status == ERROR_SUCCESS ){               
                reg_data_size = 1023;
				int keyNum = 0;
				while((reg_status = RegEnumKeyEx( key3, keyNum++, (char*)versionStr,
                                           &reg_data_size, 
                                           NULL, NULL, NULL, NULL )) == ERROR_SUCCESS) {
					reg_status = RegOpenKeyEx( key3, (char*)versionStr, 0, 
                                               KEY_READ, &key4 );
                    if( reg_status == ERROR_SUCCESS ) {
                        reg_data_size = 1023;
                        reg_status = RegQueryValueEx( key4, "PathName", 
                                                      NULL, NULL, reg_data, 
                                                      &reg_data_size );
						RegCloseKey(key4);
						if( reg_status == ERROR_SUCCESS ) {
							if(jbuilderPath==NULL || strcmpi(jbuilderPath,(const char*)reg_data)==0) {
								version = atof((const char*)versionStr)*10;
								RegCloseKey(key3);
								break;
							}
						}
					}
				}
				RegCloseKey(key2);
			}
		}
		RegCloseKey(key1);
	}
	return version;
}

static string get_jbuilder_home()
{
    string   jbuilder_home;
    LONG     reg_status;
    HKEY     key1;
    HKEY     key2;
    HKEY     key3;
    HKEY     key4;
    BYTE     reg_data[1024];
    DWORD    reg_data_size = 1023;

    jbuilder_home = "";

    reg_status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE", 0, 
                               KEY_READ, &key1 );

    if( reg_status == ERROR_SUCCESS )
    {
        reg_status = RegOpenKeyEx( key1, "Borland", 0, KEY_READ, 
                                   &key2 );

        if( reg_status == ERROR_SUCCESS )
        {
            reg_status = RegOpenKeyEx( key2, "JBuilder", 0,
                                       KEY_READ, &key3 );

            if( reg_status == ERROR_SUCCESS )
            {               
                reg_data_size = 1023;
                reg_status = RegEnumKeyEx( key3, 0, (char*)reg_data,
                                           &reg_data_size, 
                                           NULL, NULL, NULL, NULL );

                if( reg_status == ERROR_SUCCESS )
                {
                    reg_status = RegOpenKeyEx( key3, (char*)reg_data, 0, 
                                               KEY_READ, &key4 );

                    if( reg_status == ERROR_SUCCESS )
                    {
                        reg_data_size = 1023;
                        reg_status = RegQueryValueEx( key4, "PathName", 
                                                      NULL, NULL, reg_data, 
                                                      &reg_data_size );

                        if( reg_status == ERROR_SUCCESS )
                            jbuilder_home = (char*)reg_data;

                        RegCloseKey( key4 );
                    }                    
                }
                RegCloseKey( key3 );
            }
            RegCloseKey( key2 );
        }
        RegCloseKey( key1 );
    }

    if( jbuilder_home.length() == 0 )
        msg( "Can't locate JBuilder Home Directory", error_sev ) << eom;
            
    return jbuilder_home;
}

static vector<string> get_list_components( const string &path_list )
{
    char           *path_list_cstr;
    char           *path_list_cstr_ptr;
    char           *sep_ptr;
    vector<string>  components;

    if( path_list.length() > 0 )
    {
        path_list_cstr = new char [path_list.length()+1];
        strcpy( path_list_cstr, path_list.c_str() );

        path_list_cstr_ptr = path_list_cstr;

        do
        {
            sep_ptr = strchr( path_list_cstr, ';' );
            if( sep_ptr != NULL )
            {
                *sep_ptr = (char)0;
                components.push_back( path_list_cstr );
                path_list_cstr = sep_ptr + 1;
            }
            else
            {
                components.push_back( path_list_cstr );
                break;
            }
        }
        while( true );

        delete [] path_list_cstr_ptr;
    }

    return components;
}

/* Normalize the path 'rel_or_abs', copying the result to 'target'.
 * The resulting path will be absolute.
 * Remove trailing \ or /.
 */
static void normalize_path(const char *rel_or_abs, 
			   const char *abs_base, char *target)
{
    char *rel_or_abs_copy = new char[strlen(rel_or_abs) + 1];
    strcpy(rel_or_abs_copy, rel_or_abs);
    if(PathIsRelative(rel_or_abs_copy)) {
	strcpy(target, abs_base);
	PathAppend(target, rel_or_abs_copy);
    }
    else {
	strcpy(target, rel_or_abs_copy);
    }
    int last = strlen(target) - 1;
    char lastch = (last >= 0 ? target[last] : '\0');
    if (lastch == '\\' || lastch == '/') {
	char prevch = (last > 0 ? target[last-1] : '\0');
	if (prevch == ':' || prevch == '\0' || prevch == '\\') {
	    // Preserve the backslash for correctness,
	    // even though it may give aset_jfe a hard time.
	}
	else {
	    target[last] = '\0';
	}
    }
}

static string get_jbuilder_runtime_class_path2( 
    string           jdk_version,
    JBuilderProject *jpr_properties )
{
    string                        rt_class_path;
    JBuilderProjectVariable      *jdk_class_path_field;
    JBuilderProjectVariable      *jdk_current;
    JBuilderProjectVariableValue *jdk_class_path_value;
    JBuilderProjectVariableValue *jdk_current_value;
    string                        jdk_class_path;
    vector<string>                jdk_components;
    string                        jbuilder_home;
    unsigned short                i;
    char                          jdk_component[1024];
    FILE                         *fp;
    
	jdk_class_path_field = NULL;
	if(jdk_version.length()>0) {
        jdk_class_path_field = jpr_properties->GetVariable( 
            ((string)"JDK.classpath.") + jdk_version );
	} else {
		jdk_current = jpr_properties->GetVariable( (string)"jdk.names" );
		if(jdk_current!=NULL) {
		   jdk_current_value = jdk_current->GetValue( 0 );
		   if(jdk_current_value!=NULL) {
			   string currentJdk = jdk_current_value->GetValue();
			   if(currentJdk.length()>0) {
                  string stripJdk; 
                  int len = currentJdk.length();
				  for(int k=0;k<len;k++) {
					  if(currentJdk[k]!='\\') stripJdk.append(1,currentJdk[k]);
				  }
                  jdk_class_path_field = jpr_properties->GetVariable( 
                                         ((string)"JDK.classpath.") + stripJdk );
			   }
		   }
		}
	}

    if( jdk_class_path_field != NULL )
    {
        jdk_class_path_value = jdk_class_path_field->GetValue( 0 );

        if( jdk_class_path_value != NULL )
        {
            jdk_components = get_list_components( 
                                 jdk_class_path_value->GetValue() );

            jbuilder_home = get_jbuilder_home();

            for( i=0; i<jdk_components.size(); i++ )
            {
                if( jdk_components[i].length() > 0 )
                {
                    if( jdk_components[i][0] == '[' )
                        strcpy( jdk_component, &jdk_components[i].c_str()[1] );
                    else
                        strcpy( jdk_component, jdk_components[i].c_str() );

                    if( strlen(jdk_component) > 0 && 
                        jdk_component[strlen(jdk_component)-1] == ']' )
                        jdk_component[strlen(jdk_component)-1] = (char)0;

                    if( strlen(jdk_component) > 0 )
                    {
                        if( rt_class_path.length() > 0 )
                            rt_class_path += ";";

			normalize_path(jdk_component, jbuilder_home.c_str(),
				       jdk_component);

                        if( strchr(jdk_component, '*') == NULL )
                        {                            
                            /*
                             * No wildcards in the path, so make sure the file
                             * exists
                             */
                            fp = fopen( jdk_component, "r" );
                            if( fp != NULL )
                            {
                                rt_class_path += jdk_component;
                                fclose( fp );
                            }
                        }
                        else
                            rt_class_path += jdk_component;
                    }
                }
            }
        }
    }
   
    return rt_class_path;
}

static string get_jbuilder_runtime_class_path( 
    JBuilderProjectVariableValue *sys_0,
    JBuilderProject              *jpr_properties,
    bool			 &is_javasoft )
{
    JBuilderProjectVariable      *sys_0_jdk_field;
    JBuilderProjectVariableValue *sys_0_jdk_value;
    string                        sys_0_jdk;
    string                        rt_class_path;
    LONG                          reg_status;
    HKEY                          key1;
    HKEY                          key2;
    HKEY                          key3;
    HKEY                          key4;
    BYTE                          reg_data[1024];
    DWORD                         reg_data_size = 1023;

    /*
     * First attempt to use information contained in the project and in
     * the JBuilder Properties
     */

    sys_0_jdk       = "";
    sys_0_jdk_field = sys_0->GetField( "JDK" );
    if( sys_0_jdk_field != NULL )
    {
        sys_0_jdk_value = sys_0_jdk_field->GetValue( 0 );

        if( sys_0_jdk_value != NULL )
        {
            sys_0_jdk = sys_0_jdk_value->GetValue();

            if( sys_0_jdk.length() > 0 )
            {
                rt_class_path = get_jbuilder_runtime_class_path2(
                                    sys_0_jdk, jpr_properties );

                if( rt_class_path.length() > 0 ) {
                    return rt_class_path;
				} else {
					// If project contains JDK version which does not match
					// JBuilder properties version we will try to get 
					// default version of the JDK from java.names
                    rt_class_path = get_jbuilder_runtime_class_path2(
                                        (string)"", jpr_properties );
                    if( rt_class_path.length() > 0 ) {
                        return rt_class_path;
					}
				}
            }
        }
    }

    /*
     * If we get here our previous attempt didn't work, so we just try
     * the registry JavaSoft entries
     */
   
    reg_status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE", 0, 
                               KEY_READ, &key1 );

    if( reg_status == ERROR_SUCCESS )
    {
        reg_status = RegOpenKeyEx( key1, "JavaSoft", 0, KEY_READ, 
                                   &key2 );

        if( reg_status == ERROR_SUCCESS )
        {
            reg_status = RegOpenKeyEx( key2, "Java Runtime Environment", 0,
                                       KEY_READ, &key3 );

            if( reg_status == ERROR_SUCCESS )
            {               
                reg_status = RegQueryValueEx( key3, "CurrentVersion", 
                                              NULL, NULL, reg_data, 
                                              &reg_data_size );

                if( reg_status == ERROR_SUCCESS )
                {
                    reg_status = RegOpenKeyEx( key3, (LPCTSTR)reg_data, 0,
                                               KEY_READ, &key4 );

                    if( reg_status == ERROR_SUCCESS )
                    {
                        reg_data_size = 1023;
                        reg_status = RegQueryValueEx( key4, "JavaHome", 
                                                      NULL, NULL, reg_data,
                                                      &reg_data_size );

                        RegCloseKey( key4 );
                    }                    
                }                
                RegCloseKey( key3 );
            }                       
            RegCloseKey( key2 );
        }
        RegCloseKey( key1 );
    }

    if( reg_status == ERROR_SUCCESS )
    {
        WIN32_FIND_DATA  file_info;
        HANDLE           file_search_handle;
        char            *java_rtime_path;

        java_rtime_path = new char[strlen((LPTSTR)reg_data)+1];
        strcpy( java_rtime_path, (LPTSTR)reg_data );

        PathAppend( (LPTSTR)reg_data, "lib\\*.jar" );

        file_search_handle = FindFirstFile( (LPTSTR)reg_data, &file_info );
        
        if( file_search_handle != INVALID_HANDLE_VALUE )
        {
            do
            {
                if( rt_class_path.length() > 0 )
                    rt_class_path += ";";

                rt_class_path += ((string)java_rtime_path) + "\\lib\\" + 
                                 file_info.cFileName;
		is_javasoft = true;
            }
            while( FindNextFile(file_search_handle, &file_info) );

            FindClose( file_search_handle );
        }

        delete [] java_rtime_path;
    }
                                        
    return rt_class_path;
}

static string get_libraries_path( JBuilderProjectVariableValue *sys_0,
                                  JBuilderProject              *jpr_properties )
{
    JBuilderProjectVariable      *libraries_field;
    JBuilderProjectVariable      *jdk_field;
    JBuilderProjectVariableValue *libraries_field_value;
    JBuilderProjectVariableValue *jdk_field_value;
    string                        libraries;
    string                        jdk;
    vector<string>                libraries_vec;
    string                        libraries_path;
    unsigned short                i;
    JBuilderProjectVariable      *library_path_field;
    JBuilderProjectVariableValue *library_path_field_value;
    vector<string>                library_path_vec;
    unsigned short                j;
    char                          library_path[1024];
    string                        jbuilder_home;
    FILE                         *fp;

    libraries      = "";
    libraries_path = "";
    jbuilder_home  = get_jbuilder_home();

    libraries_field = sys_0->GetField( "Libraries" );
    if( libraries_field != NULL )
    {
        libraries_field_value = libraries_field->GetValue( 0 );
        if( libraries_field_value != NULL )        
            libraries = libraries_field_value->GetValue();
    }
       
    jdk_field = sys_0->GetField( "JDK" );
    if( jdk_field != NULL )
    {
        jdk_field_value = jdk_field->GetValue( 0 );
        if( jdk_field_value != NULL )        
            jdk = jdk_field_value->GetValue();
    }

    libraries_vec = get_list_components( libraries );
    libraries_vec.push_back(jdk);
	
    for( i=0; i<libraries_vec.size(); i++ )
    {
	msg("Obtaining classpath for $1 library.", normal_sev) << libraries_vec[i] << eoarg << eom;
        library_path_field = jpr_properties->GetVariable( 
                                 ((string)"library.classpath.") + 
                                 libraries_vec[i] );

        if( library_path_field != NULL )
        {
            library_path_field_value = library_path_field->GetValue( 0 );

            if( library_path_field_value != NULL )
            {
                library_path_vec = get_list_components( 
                    library_path_field_value->GetValue() );

                for( j=0; j<library_path_vec.size(); j++ )
                {
                    if( library_path_vec[j].length() > 0 )
                    {
                        if( library_path_vec[j][0] == '[' )
                            strcpy( library_path, 
                                    &library_path_vec[j].c_str()[1] );
                        else
                            strcpy( library_path, 
                                    library_path_vec[j].c_str() );

                        if( strlen(library_path) > 0 && 
                            library_path[strlen(library_path)-1] == ']' )
                            library_path[strlen(library_path)-1] = (char)0;

                        if( strlen(library_path) > 0 )
                        {
                            if( libraries_path.length() > 0 )
                                libraries_path += ";";

			    normalize_path(library_path, jbuilder_home.c_str(),
					   library_path);

                            if( strchr(library_path, '*') == NULL )
                            {
                                /*
                                 * Make sure the file exists
                                 */
                                fp = fopen( library_path, "r" );
                                if( fp != NULL )
                                {                                       
                                    libraries_path += library_path;
                                    fclose( fp );
                                }
				else if (_access(library_path, 00) != -1) { //had to include <io.h>
				    // Directory exists.
				    // Sorry, another Windows-specific call.
				    // See mbdriver/*/model_server_utils.C for portable code.
				    libraries_path += library_path;
				}
				else {
				    msg("The classpath for the $1 library refers to the nonexistent $2", warning_sev)
					<< libraries_vec[i] << eoarg << library_path << eoarg << eom;
				}
                            }
                            else
                                libraries_path += library_path;
                        }
			else {
			    msg("In the classpath for the $1 library, the path at index $2 was empty after adjustments.", warning_sev) << libraries_vec[i] << eoarg << j << eom;
			}
                    }
		    else {
			msg("In the classpath for the $1 library, the path at index $2 was empty.", warning_sev) << libraries_vec[i] << eoarg << j << eom;
		    }
                }
            }
	    else {
		msg("The project refers to the $1 library, but classpath information for the library was empty.", warning_sev) << libraries_vec[i] << eoarg << eom;
	    }
        }
	else {
	    msg("The project refers to the $1 library, but no classpath information for the library could be found.", warning_sev) << libraries_vec[i] << eoarg << eom;
	}
    }

    return libraries_path;
}

static string get_class_path( JBuilderProjectVariableValue *sys_0,
                              JBuilderProject              *jpr_properties )
{
    JBuilderProjectVariable      *class_path_field;
    JBuilderProjectVariableValue *class_path_field_value;
    string                        class_path;
    string			  jbuilder_class_path;
    string                        libraries_path;
    unsigned short                len;
    unsigned short                i;
    bool			  jbuilder_class_path_is_javasoft = false;

    jbuilder_class_path = get_jbuilder_runtime_class_path( sys_0, jpr_properties,
					       jbuilder_class_path_is_javasoft );
    libraries_path = get_libraries_path( sys_0, jpr_properties );

    if (!jbuilder_class_path_is_javasoft) {
	// Put jbuilder class path first if it's really jbuilder; else last.
	class_path = jbuilder_class_path;
    }

    if( libraries_path.length() > 0 )
    {
        if( class_path.length() > 0 )
        {
            class_path += ";";
            class_path += libraries_path;
        }
        else
            class_path = libraries_path;
    }

    /*
     * Get sys[0].ClassPath
     */
    class_path_field = sys_0->GetField( "ClassPath" );
    if( class_path_field != NULL )
    {
        class_path_field_value = class_path_field->GetValue( 0 );

        if( class_path_field_value != NULL && 
            class_path_field_value->GetValue().length() > 0 )
        {
            if( class_path.length() > 0 )
                class_path += ";";

            class_path += class_path_field_value->GetValue();
        }
    }

    if( false /*getenv("CLASSPATH") != NULL*/ ) //ignore CLASSPATH env. variable
    {
        if( class_path.length() > 0 )
            class_path += ";";

        class_path += getenv( "CLASSPATH" );
    }    

    if (jbuilder_class_path_is_javasoft) {
	if( jbuilder_class_path.length() > 0 )
	{
	    if( class_path.length() > 0 )
	    {
		class_path += ";";
		class_path += jbuilder_class_path;
	    }
	    else
		class_path = jbuilder_class_path;
	}
    }

    /* 
     * Replace commas with semicolons,
     * which must be used as command line arg to java
     */
    len = class_path.length();
    for( i=0; i<len; i++ )
        if( class_path[i] == ',')
            class_path[i] = ';';    

    return class_path;
}    

static vector<string> get_project_source_path( 
    JBuilderProjectVariableValue *sys_0 )
{
    vector<string>                source_path;
    JBuilderProjectVariable      *source_path_field;
    JBuilderProjectVariableValue *source_path_field_value;

    source_path_field = sys_0->GetField( "SourcePath" );
    if( source_path_field != NULL )
    {
        source_path_field_value = source_path_field->GetValue( 0 );
        if( source_path_field_value != NULL )
        {
            source_path = get_list_components( 
                              source_path_field_value->GetValue() );
        }        
    }

    return source_path;
}

static string get_project_name( JBuilderProject *jpr_project )
{
    string proj_name;

    /*
     * Seems like Object #0 in the .jpr file is the project
     */

    proj_name = "";

    if( jpr_project->GetProjObject(0) != NULL )
        proj_name = jpr_project->GetProjObject(0)->GetName();

    return proj_name;
}

static void generate_output_line( const string &proj_name,
                                  const string &proj_directory,
                                  const string &file_name,
                                  const string &class_path,
                                  FILE         *out_fp )
{
    fprintf( out_fp, "%s\t%s\t%s\tjbuilder\t-classpath %s\n",
             proj_name.c_str(), 
             proj_directory.c_str(),
             file_name.c_str(),
             class_path.c_str() );
}

static bool generate_package_output( const string   &proj_name,
                                     const string   &proj_directory,
                                     const string   &package_parent_directory,
                                     const string   &package_name,
                                     const string   &class_path,
                                     FILE           *out_fp )
{
    bool             found_it;
    char             search_path[1024];
    char             search_path2[1024];
    char             java_file_abs_path[1024];
    WIN32_FIND_DATA  file_info;
    HANDLE           file_search_handle;

    found_it = false;

    normalize_path(package_parent_directory.c_str(),
		   proj_directory.c_str(),
		   search_path);

    PathAppend( search_path, package_name.c_str() );

    file_search_handle = FindFirstFile( search_path, &file_info );

    if( file_search_handle != INVALID_HANDLE_VALUE )
    {
        /*
         * Got it!
         */
        found_it = true;
        FindClose( file_search_handle );

        strcpy( search_path2, search_path );

//        PathAppend( search_path2, "*.java" );
        PathAppend( search_path2, "*.*" );

        file_search_handle = FindFirstFile( search_path2, &file_info );
        if( file_search_handle != INVALID_HANDLE_VALUE )
        {
            do
            {
				if((file_info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY) {
					if(strcmp(file_info.cFileName,".")!=NULL && 
					   strcmp(file_info.cFileName,"..")!=NULL) {
						string subPackageName = package_name+"\\"+file_info.cFileName;
						generate_package_output(proj_name,proj_directory,
							package_parent_directory,subPackageName,class_path,out_fp);
					}
				} else {
					string strFileName = file_info.cFileName;
					int extPos = strFileName.rfind(".java");
					// make sure that file name ends with ".java"
					if((extPos!=string::npos) && 
					   ((extPos+5)==strFileName.length())) { 
						strcpy( java_file_abs_path, search_path );
						PathAppend( java_file_abs_path, file_info.cFileName );

						generate_output_line( proj_name, proj_directory, 
											  string(java_file_abs_path), class_path,
											  out_fp );
					}
				}
            }
            while( FindNextFile(file_search_handle, &file_info) );

            FindClose( file_search_handle );
        }
    }

    return found_it;
}

static void generate_package_output( const string   &proj_name,
                                     const string   &proj_directory,
                                     vector<string> &proj_source_path,
                                     const string   &package_name,
                                     const string   &class_path,
                                     FILE           *out_fp )
{
    bool           found_it;
    unsigned short numb_of_directories;
    unsigned short i;
    string         directory;

    found_it = false;

    numb_of_directories = proj_source_path.size();
    for( i=0; !found_it && i<numb_of_directories; i++ )
        found_it = generate_package_output( proj_name, proj_directory, 
                                            proj_source_path[i], 
                                            package_name, class_path, 
                                            out_fp );
}

// This parses a comment and returns true if one is detected,
// and is supposed to backtrack if it returns false, but doesn't
// quite achieve that all the time.
//
static bool get_comment(istream &input)
{
    char ch;
    input.get(ch);
    if (input.bad()) {
	return false;
    }
    if (input.eof()) {
	return false;
    }
    if (ch != '/') {
	input.putback(ch);
	return false;
    }
    input.get(ch);
    if (input.bad()) {
	return false;
    }
    if (input.eof()) {
	return false;
    }
    if (ch == '/') {
	for (;;) {
	    char ch;
	    input.get(ch);
	    if (input.bad() || input.eof()) {
		break;
	    }
	    else if (ch == '\n') {
		break;
	    }
	}
	return true;
    }
    else if (ch == '*') {
	bool had_asterisk = false;
	for (;;) {
	    char ch;
	    input.get(ch);
	    if (input.bad() || input.eof()) {
		break;
	    }
	    if (had_asterisk && ch == '/') {
		break;
	    }
	    had_asterisk = (ch == '*');
	}
	return true;
    }
    else {
	input.putback(ch);
	return false;
    }
}

static bool get_ident(istream &input, string &str)
{
    bool have_ident = false;
    for (;;) {
	char ch;
	input.get(ch);
	if (input.bad()) {
	    return false;
	}
	else if (input.eof()) {
	    break;
	}
	else if (!isalnum(ch)) {
	    input.putback(ch);
	    break;
	}
	if (!have_ident) {
	    str = "";
	    have_ident = true;
	}
	str.append(1, ch);
    }
    return have_ident;
}

static string package_path(vector<string> &pkg) {
    string result;
    vector<string>::iterator iter;
    for (iter = pkg.begin(); iter != pkg.end(); ++iter) {
	if (iter != pkg.begin()) {
	    result.append(".");
	}
	result.append(*iter);
    }
    return result;
}

// Looks in the .java file for the package statement,
// setting pkg to the sequence of names in its package statement,
// or to an empty vector if no package statement is found.
// Returns false if file could not be opened.
static bool get_package(string file, vector<string> &pkg)
{
    ifstream f(file.c_str(), ios::in);
    if (!f.is_open()) {
	return false;
    }
    string str;
    bool in_package = false;
    bool have_dot = false;
    pkg.clear();
    for (;;) {
	if (get_ident(f, str)) {
	    if (!in_package) {
		if (str == "package") {
		    in_package = true;
		}
	    }
	    else {
		pkg.push_back(str);
	    }
	}
	else if (get_comment(f)) {
	}
	else {
	    char ch;
	    f.get(ch);
	    if (f.bad()) {
		return false;
	    }
	    if (f.eof()) {
		return pkg.size() == 0;
	    }
	    if (ch == ';' && in_package) {
		return true;
	    }
	    if (ch == '.' && in_package) {
		have_dot = true;
	    }
	}
    }
    return true;
}

// Looks in the .java file for the package statement,
// and verifies that it matches the path given by pkgs.
static bool is_of_pkg(string file, vector<string> &pkgs, bool dflt)
{
    vector<string> file_pkgs;
    if (get_package(file, file_pkgs)) {
	if (pkgs.size() != file_pkgs.size()) {
	    return false;
	}
	else {
	    vector<string>::iterator iter;
	    vector<string>::iterator file_iter = file_pkgs.begin();
	    for (iter = pkgs.begin(); iter != pkgs.end();  ++iter, ++file_iter) {
		if (_stricmp(iter->c_str(), file_iter->c_str()) != 0) {
		    return false;
		}
	    }
	    return true;
	}
    }
    else {
	return dflt;
    }
}

// Return true if any file in the current directory is a valid
// .java source file of the package given by the pkgs list.
static bool dir_valid_in_class_path(string dir, vector<string> &pkgs, string &reason)
{
    WIN32_FIND_DATA FileData; 
    HANDLE hSearch; 
 
    // Search for .java files in the current directory. 
    char *file_pattern = new char[dir.size() + 10];
    strcpy(file_pattern, dir.c_str());
    PathAppend(file_pattern, "*.java");
    hSearch = FindFirstFile(file_pattern, &FileData);

    bool found_valid = false;
    if (hSearch != INVALID_HANDLE_VALUE) {
	for (;;) { 
	    // FileData.cFileName is the local (relative) name.
	    char *file_path = new char[dir.size() + strlen(FileData.cFileName) + 4];
	    strcpy(file_path, dir.c_str());
	    PathAppend(file_path, FileData.cFileName);
	    found_valid = is_of_pkg(file_path, pkgs, false);
	    if (found_valid) {
		reason = "Found ";
		reason.append(file_path);
		reason.append(".");
	    }
	    else {
		reason.append(", examined ");
		reason.append(file_path);
	    }
	    delete [] file_path;
	    if (found_valid) {
		break;
	    }
	    if (!FindNextFile(hSearch, &FileData)) {
		break;
	    }
	} 

	// Close the search handle. 
	FindClose(hSearch);
    }
    delete [] file_pattern;
    if (found_valid) {
	return true;
    }

    // Search nested directories.
    file_pattern = new char[dir.size() + 5];
    strcpy(file_pattern, dir.c_str());
    PathAppend(file_pattern, "*.*");
    hSearch = FindFirstFile(file_pattern, &FileData);
    if (hSearch != INVALID_HANDLE_VALUE) {
	for (;;) { 
	    if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
		&& strcmp(FileData.cFileName, ".") != 0
		&& strcmp(FileData.cFileName, "..") != 0) {
		// FileData.cFileName is the local (relative) name.
		pkgs.push_back(string(FileData.cFileName));
		char *nested_dir = new char[dir.size() + strlen(FileData.cFileName) + 4];
		strcpy(nested_dir, dir.c_str());
		PathAppend(nested_dir, FileData.cFileName);
		found_valid = dir_valid_in_class_path(nested_dir, pkgs, reason);
		pkgs.pop_back();
		delete [] nested_dir;
		if (found_valid) {
		    break;
		}
	    }
	    if (!FindNextFile(hSearch, &FileData)) {
		break;
	    }
	} 

	// Close the search handle. 
	FindClose(hSearch);
    }
    delete [] file_pattern;
    return found_valid;
}

// Check that path is valid, because JBuilder mixes in the source path
//    (1) directories which must be searched
//    (2) directories which should not be searched
static bool valid_for_class_path(const char *path)
{
    vector<string> pkgs;
    string reason = "no reason";
    bool result = dir_valid_in_class_path(path, pkgs, reason);
    return result;
}
       
static bool generate_output( JBuilderProject *jpr_project, 
                             JBuilderProject *jpr_properties,
                             const string    &proj_directory,
                             FILE            *out_fp )
{    
    bool                          success;
    JBuilderProjectVariable      *sys;
    JBuilderProjectVariableValue *sys_0;
    string                        class_path;
    string                        proj_name;
    vector<string>                proj_source_path;
    const JBuilderProjectObject  *proj_obj;
    unsigned short                i;
    JBuilderProjectVariableValue *obj_sys;
    JBuilderProjectVariable      *obj_sys_type;
    JBuilderProjectVariableValue *obj_sys_type_value;
    char                          path[MAX_PATH];

    success = true;

    sys = jpr_project->GetVariable( "sys" );
    if( sys != NULL )
    {
        sys_0 = sys->GetValue( 0 );

        if( sys_0 != NULL )
        {
            class_path       = get_class_path( sys_0, jpr_properties );
            proj_name        = get_project_name( jpr_project );
            proj_source_path = get_project_source_path( sys_0 );

            for( i=proj_source_path.size(); i>0; i-- )
            {
		normalize_path(proj_source_path[i-1].c_str(),
			       proj_directory.c_str(),
			       path);
		if (valid_for_class_path(path)) {
		    if( class_path.length() > 0 )                  
			class_path.insert( 0, ";" );
                    class_path.insert( 0, path );
		}
                
            }
            
            if( class_path.length() > 0 )
                class_path.insert( 0, ";" );

            class_path.insert( 0, proj_directory );

            if( strchr(class_path.c_str(), ' ') != NULL )
            {
                class_path.insert( 0, "\"" );
                class_path.append( "\"" );
            }

            /* 
             * class_path MUST NOT be modified after this point!
             */

            i = 1;
            proj_obj = jpr_project->GetProjObject( i );
            while( proj_obj != NULL )
            {
		/* Figure out whether the object is Package type. */
		bool is_a_package = false;
		obj_sys = sys->GetValue( i );
		if( obj_sys != NULL )
		{
		    obj_sys_type = obj_sys->GetField( "Type" );
		    if( obj_sys_type != NULL )
		    {
			obj_sys_type_value = obj_sys_type->GetValue( 0 );
			if( obj_sys_type_value != NULL )
			{
			    if( strcasecmp(
				    obj_sys_type_value->GetValue().c_str(),
				    "Package") == 0 )
			    {
				is_a_package = true;
			    }
			}
		    }
		}

		if (is_a_package) {
		    if( proj_obj->GetName().length() > 0 )
		    {
			/*
			 * If the object represents a package, we need to 
			 * include all the java files in the directory
			 */
			string packageName = proj_obj->GetName();
			int dotPos = string::npos;
			while((dotPos = packageName.find("."))!=string::npos) {
				packageName.replace(dotPos,1,"\\");
			}	
			generate_package_output( 
			    proj_name, proj_directory, 
			    proj_source_path, 
			    packageName, class_path, 
			    out_fp );
		    }
		}
                else if( proj_obj->IsJavaSourceFile() )
                    generate_output_line( proj_name, proj_directory, 
                                          proj_obj->GetName(), class_path,
                                          out_fp );
                
                i++;
                proj_obj = jpr_project->GetProjObject( i );
            }
        }
        else
        {
            success = false;
            fprintf( stderr, "Can't access sys[0] in JBuilder Project File" );
        }
    }
    else
    {
        success = false;
        fprintf( stderr, "Can't access sys in JBuilder Project File" );
    }    

    return success;
}

static bool parse( FILE *jpr_fp, const string &project_directory, FILE *out_fp )
{
    bool             success;
    JBuilderProject *jpr_project;
    JBuilderProject *jpr_properties;
    FILE            *jpr_properties_fp;
    char             jpr_properties_file[1024];
    
    int jbuilderVersion = getJBuilderVersion();

    jpr_properties = NULL; 
    jpr_properties_file[0] = (char)0;

    #ifdef _HAS_SDK
    {
        HANDLE           process_handle;
        HANDLE           user_token;
        DWORD            buff_size = 1023;

        process_handle = OpenProcess( PROCESS_ALL_ACCESS, TRUE, 
                                      GetCurrentProcessId() );
    
        if( process_handle != NULL )
        {
            if( OpenProcessToken(process_handle, TOKEN_QUERY, &user_token) != 0 )
            {       
                GetUserProfileDirectory(user_token, jpr_properties_file, 
                                            &buff_size);
            }
            CloseHandle( process_handle );
        }   
    }
    #else /*_HAS_SDK*/
    {
        if( getenv("USERPROFILE") != NULL )
        {
            strcpy( jpr_properties_file, getenv("USERPROFILE") );
        }
    }
    #endif /*_HAS_SDK*/

	char longName[1024];
	strcpy(longName,jpr_properties_file);
	GetShortPathName(longName,jpr_properties_file,1024);

	if(jbuilderVersion<40) {
		if( jpr_properties_file[0] != (char)0 )
		{
			PathAppend( jpr_properties_file, ".jbuilder\\library.properties" );
			jpr_properties_fp = fopen( jpr_properties_file, "r" );
			if( jpr_properties_fp != NULL )
			{
				_jpr_set_pound_starts_comment( true );
				_jpr_set_allow_dots_in_idents( true );
				success = jpr_parse( jpr_properties_fp, NULL, stderr, 
									 &jpr_properties );
				_jpr_set_allow_dots_in_idents( false );
				_jpr_set_pound_starts_comment( false );

				fclose( jpr_properties_fp );
				//jpr_properties->FPrintVariables( stdout );
			}
			else
				msg( "Can't open JBuilder Properties file: $1", error_sev )
					<< jpr_properties_file << eoarg << eom;
		}
		else
			msg( "Can't locate User's Profile Directory", error_sev ) << eom;
	} else {
		if(jbuilderVersion == 40 || jbuilderVersion == 50) {
			struct _finddata_t lib_file;
			long hFile;
			char filesMask[1024];
			JBuilderProject** out_jbuilder_proj = NULL;

			if(jbuilderVersion == 40)
				PathAppend(jpr_properties_file,".jbuilder4");
			else
				PathAppend(jpr_properties_file,".jbuilder5");
			strcpy(filesMask,jpr_properties_file);
			PathAppend( filesMask, "*.library" );
			// Find all .library files in JBuilder preferences directory
			if( (hFile = _findfirst( filesMask, &lib_file )) != -1L ) {
				char libraryFile[1024];
				out_jbuilder_proj = &jpr_properties;
				*out_jbuilder_proj = new JBuilderProject;
				do {
				    strcpy(libraryFile,jpr_properties_file);
				    PathAppend( libraryFile, lib_file.name );				
				    success = lib_parse( libraryFile, NULL, stderr, 
									     *out_jbuilder_proj );
				} while (_findnext( hFile, &lib_file ) == 0);
			   _findclose( hFile );
			}
			
			// Find all .library files in JBuilder home lib directory.
			// If both user preference and JBuilder home define the same
			// library, the user's version takes precedence.
			string jbuilder_home = get_jbuilder_home();
			char *home_files_mask = new char[jbuilder_home.size() + 30];
			strcpy(home_files_mask, jbuilder_home.c_str());
			PathAppend(home_files_mask, "lib");
			PathAppend(home_files_mask, "*.library");
			if( (hFile = _findfirst( home_files_mask, &lib_file )) != -1L ) {
				if (out_jbuilder_proj == NULL) {
				    out_jbuilder_proj = &jpr_properties;
				    *out_jbuilder_proj = new JBuilderProject;
				}
				while( _findnext( hFile, &lib_file ) == 0 ){
			            char *library_file = new char[jbuilder_home.size() + sizeof(lib_file.name) + 10];
				    strcpy(library_file, jbuilder_home.c_str());
				    PathAppend(library_file, "lib");
				    PathAppend( library_file, lib_file.name );
				    success = lib_parse( library_file, NULL, stderr, 
									     *out_jbuilder_proj );
				    delete [] library_file;
				}
			   _findclose( hFile );
			}

			delete [] home_files_mask;
		}
	}

    if( jpr_properties == NULL )
        /*
         * Just create an empty, dummy one
         */
         jpr_properties = new JBuilderProject;

    success = jpr_parse( jpr_fp, NULL, stderr, &jpr_project );

    if( success )
    {
        generate_output( jpr_project, jpr_properties, project_directory, 
                         out_fp );

        delete jpr_project;
    }
    
    delete jpr_properties;

    return success;
}

static string get_project_directory( const char *proj_file_path )
{
    char  proj_directory[1024];
    char  current_directory[1024];

    strcpy( proj_directory, proj_file_path );
	const char* srcPtr = proj_file_path;
	char*destPtr = proj_directory;
	for(;*srcPtr!=0;srcPtr++,destPtr++) {
#ifdef _WIN32
		if(*srcPtr=='/')
			*destPtr = '\\';
		else
			*destPtr=*srcPtr;
#else
		if(*srcPtr=='\\')
			*destPtr = '/';
		else
			*destPtr=*srcPtr;
#endif
	}

    PathRemoveFileSpec( proj_directory );

    /*
     * A relative path must be combined with current working directory 
     * to establish the absolute path
     */
    _getcwd( current_directory, 1023 );

    normalize_path(proj_directory, current_directory, proj_directory);
    
    return string( proj_directory );
}

class CommandLine
{
    private:
        string       m_jpr_file;
        string       m_out_file;
        string       m_msg_group;
        string       m_lp_service;

    public:
        CommandLine( int argc, char *argv[] )
        {
            int i;

            m_jpr_file = "";
            m_out_file = "";

            for( i=1; i<argc; i++ )
            {
                if( strcasecmp(argv[i], "--output") == 0 )
                {
                    if( i < argc-1 )           
                    {
                        m_out_file = argv[i+1];
                        i++;
                    }
                }
                else if( strcasecmp(argv[i], "--message_group") == 0 )
                {
                    if( i < argc-1 )
                    {
                        m_msg_group = argv[i+1];
                        i++;
                    }
                }
                else if( strcasecmp(argv[i], "--lpservice") == 0 )
                {
                    if( i < argc-1 )
                    {
                        m_lp_service = argv[i+1];
                        i++;
                    }
                }
                else if( m_jpr_file.length() == 0 )
                    m_jpr_file = argv[i];
            }
        }

        ~CommandLine() {};

        const string& GetJPRFile() const   { return m_jpr_file; }
        const string& GetOutFile() const   { return m_out_file; }        
        const string& GetMsgGroup() const  { return m_msg_group; }
        const string& GetLPService() const { return m_lp_service; }
};
        
int main( int argc, char* argv[] )
{
    int          exit_code;
    CommandLine *cmd_line;
    FILE        *jpr_fp;
    FILE        *out_fp;
    bool         success;
    string       project_directory;

    exit_code = 0;

    cmd_line = new CommandLine( argc, argv );

    if( cmd_line->GetLPService().length() > 0 && 
        cmd_line->GetMsgGroup().length() > 0 )
    {
        init_lp_service( cmd_line->GetLPService().c_str(),
                         cmd_line->GetMsgGroup().c_str() );
    }

    if( cmd_line->GetJPRFile().length() == 0 || 
        cmd_line->GetOutFile().length() == 0 )
    {
        display_usage();
        exit_code = 1;
    }
    else
    {
        jpr_fp = fopen( cmd_line->GetJPRFile().c_str(), "r" );
        if( jpr_fp == NULL )
        {
            msg( "Can't open file $1", catastrophe_sev ) 
                     << cmd_line->GetJPRFile().c_str()
                     << eoarg << eom;

            exit_code = 2;
        }
        else
        {
            out_fp = fopen( cmd_line->GetOutFile().c_str(), "w" );
            if( out_fp == NULL )
            {
                msg( "Can't create output file $1", catastrophe_sev )                    
                    << cmd_line->GetOutFile().c_str()
                    << eoarg << eom;

                exit_code = 3;
            }
            else
            {
                project_directory = get_project_directory( 
                    cmd_line->GetJPRFile().c_str() );

                success = parse( jpr_fp, project_directory, out_fp );

                if( !success )
                    exit_code = 4;

                fclose( out_fp );
            }

            fclose( jpr_fp );
        }
    }

    delete cmd_line;
    
    return exit_code;
}
