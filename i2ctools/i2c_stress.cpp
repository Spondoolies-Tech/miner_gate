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

int sleep_time = -1;
int addr = -1;
int command = -1;


int usage(char * app ,int exCode ,const char * errMsg = NULL)
{
    if (NULL != errMsg )
    {
        fprintf (stderr,"==================\n%s\n\n==================\n",errMsg);
    }

    printf ("Read word in i2c in loop.\nUsage: %s -u microsecs -a addr -c command\n\n" , app);

    printf ("       -u (dec)   : how long to sleep between reads\n");
    printf ("       -a (hex)   : i2c register address\n");
    printf ("       -c (hex)   : i2c command\n");	


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


	for (int i = 1 ; i < argc ; i++){
		if ( 0 == strcmp(argv[i],"-h")) {
		  callUsage = true;
		} else if ( 0 == strcmp(argv[i],"-u")){
				sscanf(argv[++i],"%x",&sleep_time);
				if (0 > sleep_time){
					fprintf(stderr,"CANT PARSE %s (sleep) into positive integer\n",argv[i]);
					badParm =  true;
				}
		}
		else if ( 0 == strcmp(argv[i],"-a")){
				sscanf(argv[++i],"%x",&addr);
				if (0 > addr){
					fprintf(stderr,"CANT PARSE %s (addr) into positive integer\n",argv[i]);
					badParm =  true;
				}

		} else if ( 0 == strcmp(argv[i],"-c")){
				sscanf(argv[++i],"%d",&command);
				if (0 > command){
					fprintf(stderr,"CANT PARSE %s (command) into positive integer\n",argv[i]);
					badParm =  true;
				}
		}
		else{
		  badParm = true;
		}
	}
	if ((-1 == sleep_time) || (-1 == addr) || (-1 == command)){
		fprintf(stderr,"no value given");
		badParm =  true;
	}


	if(badParm)
	{
		usage(argv[0],1,"Bad arguments");
	}


	// applying default as top.
	
	i2c_init(&err);
	while (1) {
		int error;
		i2c_read_word(addr,command,&error);
		if (error) {
			printf("Error!!\n");
			return 0;
		} else {
			//printf("*");
		}
		usleep(sleep_time);
	}
}
