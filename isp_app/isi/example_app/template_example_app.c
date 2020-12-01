#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>


/******************************************************************************
 * TYPEDEFS / DEFINES
 *****************************************************************************/
#define MAX_CHAR    32

typedef struct _PARAMETERS
{
    char ip[MAX_CHAR];
    char port[MAX_CHAR];

} Parameters;


/**
 * @brief   ParseCommandLine() parse the command line parameters and fill in the
 *          the Parameters structure
 *          search for the following parameters:
 *          -i   :   ip address
 *          -p   :   port number
 *          -h   :   display help file
 ******************************************************************************/
int ParseCommandLine(Parameters* params, int argc,char *argv[] )
{
    int ret = 1;
    int c;

    memset(params, 0, sizeof(*params));

    opterr = 0;

    while ((c = getopt (argc, argv, "hip:")) != -1)
    {
        switch (c)
        {
            case 'h':
                ret = 0;
                break;

            case 'i':
                strcpy(params->ip, optarg);
                break;

            case 'p':
                strcpy(params->port, optarg);
                break;

            default:
                ret = 0;
                abort ();
        }
    }

    return ret;
}


/**
 *
 * @brief   PrintHelpText() gives information about usage
 ******************************************************************************/
void PrintHelpText()
{
    printf( "\n   prog [-h] {[-i ip_addr][-p port]}\n\n");
    printf( "## Parameters\n\n");
    printf( "## Options\n");
    printf( "   -h              : Prints function usage\n");
    printf( "   -i <ip_addr>    : IP address.\n\n");
    printf( "   -p <port>       : Port number.\n\n");
    printf( "## Examples of usage:\n");
    printf( "   prog  -h                        //prints the help\n");
    printf( "   prog  -p 9999                   //listen on all ip's at port 9999\n");
    printf( "   prog  -i 192.168.40.100 -p 9999 //listen on selected ip at port 9999\n");
    return;
}


/**
 * @brief   Main function that starts the tests
 *****************************************************************************/
int main(int argc,char *argv[])
{
    Parameters              params;

    if( ParseCommandLine(&params,argc,argv) == 0)
    {
        PrintHelpText();
        return -1;
    }

    printf("Template test app called.\n");

    return 0;
}
