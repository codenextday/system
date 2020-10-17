/*****************************************************************************
* This is an unpublished work, the copyright in which vests in sci-worx GmbH.
*
* The information contained herein is the property of sci-worx GmbH and is
* supplied without liability for errors or omissions. No part may be
* reproduced or used except as authorised by contract or other written
* permission.
*
* Copyright (c) 2006 sci-worx GmbH. All rights reserved.
*
*****************************************************************************/
/*!
* \file unitTestRunner.c
*
*/
/*****************************************************************************/

//#include "ebase/types.h"
#include "embUnit/embUnit.h"
#include "textui/Outputter.h"
#include "textui/TextUIRunner.h"
#include "textui/TextOutputter.h"
#include "textui/XMLOutputter.h"
#include "textui/ExtXMLOutputter.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

extern TestRef isi_tests_api(void);

/******************************************************************************
* TYPEDEFS / DEFINES
*****************************************************************************/
#define MAX_STRING_LENGTH 128

typedef struct _PARAMETERS
{
    char        haveFile;
    char        filename[MAX_STRING_LENGTH];
    char        label[MAX_STRING_LENGTH];
    char        TimeStamp[MAX_STRING_LENGTH];
    int			time;
    char        *drivername;
} Parameters;


/******************************************************************************
* PROTOTYPES
*****************************************************************************/
int ParseCommandLine(Parameters* params, int argc,char *argv[]);
void FindTimeParameter(Parameters* params,int argc,char *argv[]);


static FILE  *h_XMLOutput = NULL;
static const char* g_logfile_path    = "";//for example insert: "//hajsd101/project1/2203_hajcine/shared/test/sw_mvdu_drv/";
static const char* g_logfile         = "TemplateUnitTest";
static const char* g_logfile_ext     = ".xml";


/**
 *
 * @brief   PrintHelpText() gives information about usage
******************************************************************************/
void PrintHelpText()
{
    printf( "\n   DriverTest [-h] {[-o output.xml][-t][-l label]}\n\n");
    printf( "\n   First parameter is -o and the second parameter is -l or:");
    printf( "\n   use -h or -o without -l, see example\n\n");
    printf( "## Parameters\n\n");
    printf( "## Options\n");
    printf( "   -h              : Prints function usage\n");
    printf( "   -o <path/name>  : Output XML Logfile name. Use your own path and filename\n");
    printf( "   -t              : Add suffix to file name (e.g. fileyymmdd_hhmmss.xml)\n");
    printf( "   -l <label name> : Label name.\n\n");
    printf( "   -d <path/name>  : Sensor driver to use.\n\n");
    printf( "## Examples of usage:\n");
    printf( "   DriverTest\n");
    printf( "   DriverTest  -h\n");
    printf( "   DriverTest  -o c:/Test/output_logfile.xml\n");
    printf( "   DriverTest  -o c:/Test/output_logfile.xml\n");
    printf( "   DriverTest  -o (without specifying a folder/file a standard file called");
    printf( "                    %s.%s will be created in the standard path:\n", g_logfile, g_logfile_ext);
    printf( "                    %s)", g_logfile_path);
    printf( "   DriverTest  -o -l UNITTEST_LABEL\n");
    printf( "   DriverTest  -d ov14825.drv\n");
    return;
}


/**
 * @brief   Main function that starts the tests
 *****************************************************************************/
int main(int argc,char *argv[])
{
    Parameters params;
    params.haveFile = 0;
    sprintf( params.label, "none");
    params.time  = 0;
    params.drivername = NULL;

    if( ParseCommandLine(&params,argc,argv) == 0)
    {
        PrintHelpText();
        return -1;
    }

    // 'pass' driver to unit tests
    extern char *libName;
    if (params.drivername != NULL)
    {
        libName = params.drivername;

    }
    // print driver name
    printf("Using driver '%s'\n", libName);

    // select ouput as specified
    if(params.haveFile == 0)
    {
        // the user does not want to get an output file - print out the outputter file
        TextUIRunner_setOutputter(TextOutputter_outputter());
    }
    else
    {
        //open output file for XML output
        if ((h_XMLOutput = fopen(&params.filename[0], "w"))==NULL)
        {
            printf("Error open file %s.\n",params.filename);
        }

        TextUIRunner_setOutputter(ExtXMLOutputter_outputter());
        // configure ExtXmlOutputter
        ExtXMLOutputter_setTestExecutionTime( params.TimeStamp);
        ExtXMLOutputter_setStyleSheet("embUnitTests.xsl");
        ExtXMLOutputter_setFileHandle(h_XMLOutput);
        ExtXMLOutputter_setConfigSpecLabel(params.label);
#ifdef _DEBUG
            ExtXMLOutputter_setBuildMode( "Debug" );
#else
            ExtXMLOutputter_setBuildMode( "Release" );
#endif
    }

    TextUIRunner_start();

    TextUIRunner_runTest( isi_tests_api() );

    TextUIRunner_end();

    if(h_XMLOutput)
        fclose(h_XMLOutput);

    return 0;

}




/**
* @brief   ParseCommandLine() parse the command line parameters and fill in the
*          the Parameters structure
*          search for the following parameters:
*          -o   :   output xml file
*          -l   :   label name
*          -t   :   use timestamp into xml outputfile
*          -h   :   display help file
******************************************************************************/
int ParseCommandLine(Parameters* params, int argc,char *argv[] )
{
    char timebuf[25] = {0};
    time_t actual_time;
    struct tm *pLocalTime;
    int paramsCount = 1;

    /* no params */
    if( argc == 1)
    {
        return 1;/* nothing to do */
    }

    /* check if we need the timestamp */
    FindTimeParameter(params, argc, argv);

    while (paramsCount < argc)
    {
        /* search -o */
        if (0 == strncmp (argv[paramsCount], "-o", 2))
        {
            params->haveFile = 1;
            time(&actual_time);
            pLocalTime = localtime(&actual_time);
            strftime( timebuf, 24,"%d.%m.%Y %H:%M:%S", pLocalTime );
            sprintf( &params->TimeStamp[0], "%s", timebuf );
            /* prepare timebuf for filename*/
            memset( timebuf, 0, 25);
            strftime( timebuf, 24,"_%Y%m%d_%H%M%S", pLocalTime );
            if( (paramsCount+1) < argc) {

                if(0 == strncmp (argv[paramsCount+1], "-", 1))
                {
                    /*use the default filename*/
                    if(params->time != 0)
                    {
                        sprintf( &params->filename[0], "%s%s%s%s",g_logfile_path, g_logfile, timebuf, g_logfile_ext );
                    }
                    else
                    {
                        sprintf( &params->filename[0], "%s%s%s",g_logfile_path, g_logfile, g_logfile_ext );
                    }
                    paramsCount++;
                }
                else
                {
                    /*use the filename from parameter list*/
                    sprintf(params->filename, "%s", argv[paramsCount+1]);
                    /*check if we need the timestamp */
                    if(params->time != 0)
                    {
                        *(params->filename+strlen(params->filename) -4) = '\0';
                        sprintf(&params->filename[0], "%s%s%s", params->filename, timebuf, g_logfile_ext );
                    }
                    paramsCount += 2;
                }
            }
            else
            {
                if(params->time != 0)
                {
                    *(params->filename+sizeof(params->filename) -4) = '\0';
                    sprintf( &params->filename[0], "%s%s%s%s",g_logfile_path, g_logfile, timebuf, g_logfile_ext );
                }
                else
                {
                    *(params->filename+sizeof(params->filename) -4) = '\0';
                    sprintf( &params->filename[0], "%s%s%s",g_logfile_path, g_logfile, g_logfile_ext );
                }
                return 1;
            }
        }
        /* search -l */
        else if (0 == strncmp (argv[paramsCount], "-l", 2))
        {
            if( (paramsCount+1) < argc) {

                if(0 == strncmp (argv[paramsCount+1], "-", 1))
                {
                    paramsCount++;
                }
                else
                {
                    /*use the label from parameter list*/
                    sprintf( &params->label[0], "%s", argv[paramsCount+1]);
                    paramsCount += 2;
                }
            }
            else
            {
                return 1;
            }
        }
        /* search -t */
        else if (0 == strncmp (argv[paramsCount], "-t", 2))
        {
            paramsCount++;
        }
        else if (0 == strncmp (argv[paramsCount], "-h", 2))
        {
            return 0;
        }
        /* search -d */
        else if (0 == strncmp (argv[paramsCount], "-d", 2))
        {
            if( (paramsCount+1) < argc)
            {
                /*directly use the filename from parameter list*/
                params->drivername = argv[paramsCount+1];
                paramsCount += 2;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            printf("Invalid syntax. Use DriverTest -h for proper usage\n\n");
            return 0;
        }
    }
    return 1;
}

/**
* @brief   FindTimeParameter() search for "-t" into the command line parameters
******************************************************************************/
void FindTimeParameter(Parameters* params,int argc,char *argv[])
{
    int i;
    params->time = 0;

    //search "-t" in parameter list
    for(i = 1; i < argc; i++)
    {
        if( 0 == strncmp (argv[i], "-t", 2) )
        {
            params->time = 1;
        }
    }
}
