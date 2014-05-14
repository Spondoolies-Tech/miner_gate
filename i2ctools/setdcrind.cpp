///
///simple util that sets DC2DCs DCR Inductor setting flag (0xD0) according to given input.
/// gets two parameters, first one is a bit mask (32bit) of what loops to set, second one is the value to set
/// alternatively, -top -bottom -both use as predefined group setting for loops 0-11 12-23 0-23
///


#include <stdio.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//#include "hammer.h"
#include <spond_debug.h>
#include "squid.h"
#include "i2c.h"
#include "dc2dc.h"
#include "setdcrind.h"
#include "mbtest.h"
#include "mainvpd.h"
#include "hammer_lib.h"
using namespace std;
extern pthread_mutex_t i2c_mutex;

int FIRSTLOOP = TOP_FIRST_LOOP ;
int LASTLOOP = BOTTOM_LAST_LOOP;

int LOOPSBITMASK = TOP_BITMASK; // default - top
int dcr_set_value = -1;

int usage(char * app ,int exCode ,const char * errMsg = NULL)
{
    if (NULL != errMsg )
    {
        fprintf (stderr,"==================\n%s\n\n==================\n",errMsg);
    }

    printf ("Usage: %s [-top|-bottom|-both|-l loops] [-r | -v value]\n\n" , app);

    printf ("       -top       : top main board(default)\n");
    printf ("       -bottom    : bottom main board\n");
    printf ("       -both      : both top & bottom main boards (default)\n");
    printf ("       -l (hex)   : specify what loops to set (zero based, e.g. bottom board is 0x00FFF000 )\n");
    printf ("       -r 		   : read the value in reg 0xD0 of specified DC2DC\n");
    printf ("       -v (dec)   : value to set into reg 0xD0 of DC2DC\n");


    if (0 == exCode) // exCode ==0 means - just print usage and get back to app for business. other value - exit with it.
    {
        return 0;
    }
    else
    {
        exit(exCode);
    }
}


int main(int argc, char *argv[])
{
	int rc = 0;
	int err = 0;


	bool callUsage = false;
	bool badParm = false;
	int ReadOrWrite = -1; // not set. read = 0 , write = 1

	int topOrBottom = -1; // default will be TOP, start with -1, to rule out ambiguity.

	char badloopsformattedstring[100];

	for (int i = 1 ; i < argc ; i++){
		if ( 0 == strcmp(argv[i],"-h"))
		  callUsage = true;

		else if ( 0 == strcmp(argv[i],"-both")){
		  if (topOrBottom == -1 || topOrBottom == BOTH_MAIN_BOARDS){
			  LOOPSBITMASK = BOTH_BITMASK;
			  topOrBottom = BOTH_MAIN_BOARDS;
		  }
		  else{
			  badParm = true;
		  }
		}
		else if ( 0 == strcmp(argv[i],"-top")){
		  if (topOrBottom == -1 || topOrBottom == TOP_BOARD){
			  LOOPSBITMASK = TOP_BITMASK;
			  topOrBottom = TOP_BOARD;
		  }
		  else{
			  badParm = true;
		  }
		}
		else if ( 0 == strcmp(argv[i],"-bottom")){
		  if (topOrBottom == -1 || topOrBottom == BOTTOM_BOARD){
			  LOOPSBITMASK = BOTTOM_BITMASK;
			  topOrBottom = BOTTOM_BOARD;
		  }
		  else{
			  badParm = true;
		  }
		}
		else if ( 0 == strcmp(argv[i],"-r")){

			if (ReadOrWrite == -1 || ReadOrWrite == 0) //read, or not set yet
			{
				ReadOrWrite = 0;
			}
			else {
				// both read and write ??
				badParm = true;
			}
		}
		else if ( 0 == strcmp(argv[i],"-v")){

			if (ReadOrWrite == -1 || ReadOrWrite == 1) //write, or not set yet
			{
				ReadOrWrite = 1;

				sscanf(argv[++i],"%d",&dcr_set_value);

				if (0 > dcr_set_value){
					fprintf(stderr,"CANT PARSE %s (dcr value) into positive integer\n",argv[i]);
					badParm =  true;
				}

			}
			else {
				// both read and write ??
				badParm = true;
			}
		}
		else if ( 0 == strcmp(argv[i],"-l")){
			int lups = -1;

			sscanf(argv[++i],"%x",&lups);

			if (lups > 0 && topOrBottom == -1) // valid value, and not ambiguous parms
			{
				topOrBottom = CUSTOM_LOOPS_SELECTION;
				LOOPSBITMASK = lups;
			}
			else{
				badParm = true;
			}
		}
		else{
		  badParm = true;
		}
	}
	if (-1 == dcr_set_value && badParm == false && ReadOrWrite == 1){
		fprintf(stderr,"no set value given");
		badParm =  true;
	}


	if(badParm)
	{
		usage(argv[0],1,"Bad arguments");
	}


	// applying default as top.
	if (-1 == topOrBottom){
	  topOrBottom = TOP_BITMASK;
	  LOOPSBITMASK = TOP_BITMASK;
	}

	LOOPSBITMASK = LOOPSBITMASK & 0x00FFFFFF;

	//fprintf(stdout,"loops bitmask is 0x%08X , set value is %d\n",LOOPSBITMASK,dcr_set_value);

	i2c_init(&err);
	if (err != 0)
	{
		fprintf(stderr,"FAILED to init i2c bus\n");
		return 4;
	}

	if (callUsage)
		return usage(argv[0] , 15);

	for (int i = 0 ; i < BITS_IN_INT ; i++ )
	{

		bool thisLoop = ( ((1 << i ) & LOOPSBITMASK ) == (1 << i));
		if (thisLoop){

			if (ReadOrWrite == 1) {//

				if (dc2dc_set_dcr_inductor_cat(i , dcr_set_value) != 0){
					fprintf(stderr,"Failed to set group %d DC2DC DCR inductor flag %d\n",(i+1),dcr_set_value);
					rc = rc | (1 << i) | 1; // the 1 will ensure failures.
					//break;
				}
			}
			else
			{
				fprintf (stdout , "DC2DC DCR inductor flag loop %d (0xD0) = %d\n",i, dc2dc_get_dcr_inductor_cat (i));
			}
		}
	}

	return rc;

}
