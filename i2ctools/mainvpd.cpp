///
///simple util that reads AC2DC VPD info
///and prints it out.
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
#include "mainvpd.h"

using namespace std;
extern pthread_mutex_t i2c_mutex;

int usage(char * app ,int exCode ,const char * errMsg = NULL)
{
    if (NULL != errMsg )
    {
        fprintf (stderr,"==================\n%s\n\n==================\n",errMsg);
    }
    
    printf ("Usage: %s [-top|-bottom][[-q] [-a] [-p] [-s] [-r]] [-v vpd]\n\n" , app);

    printf ("       -top : top main board (default)\n");
    printf ("       -bottom : bottom main board\n");
    printf ("        default mode is read\n");
    printf ("       -q : quiet mode, values only w/o headers\n"); 
    printf ("       -a : print all VPD params as one string\n"); 
    printf ("       -p : print part number\n"); 
    printf ("       -r : print revision number\n"); 
    printf ("       -s : print serial number\n"); 
    printf ("       -v : vpd value to write\n");
    if (0 == exCode) // exCode ==0 means - just print usage and get back to app for business. other value - exit with it.
    {
        return 0;
    }
    else 
    {
        exit(exCode);
    }
}

int mainboard_set_vpd( mainboard_vpd_info_t *pVpd ,const char * vpd,int d00 , int d01 , int d02 , int d03) {
	int rc = 0;
	int err = 0;
	//01234567890123456789012
	//FL1420003205ELA-0005A02
	//pVpd->pnr , pVpd->revision , pVpd->serial
	//
	strncpy(pVpd->serial ,vpd,12);
	strncpy(pVpd->pnr ,vpd+12,8);
	strncpy(pVpd->revision,vpd+20,3);

	return rc;
}
int mainboard_get_vpd( mainboard_vpd_info_t *pVpd , int d00 , int d01 , int d02 , int d03) {
	int rc = 0;
	int err = 0;

	if (pVpd == NULL ) {
		psyslog("call mainboard_get_vpd performed without allocating sturcture first\n");
		return 1;
	}

	// validate 4 registers are properly used - i.e. marked with bit15=1
	if ((d00 & (1<<15)) != (1<<15) ||
		(d01 & (1<<15)) != (1<<15) ||
		(d02 & (1<<15)) != (1<<15) ||
		(d03 & (1<<15)) != (1<<15) ){
		fprintf(stderr, RED            "This Main Board does not have VPD info properly stored\n" RESET);
		return 2;
	}

	char CMs[3] = "NA";
	int CMi = (d00 >>12) & 3;
	if (CMi == 0){
		sprintf(CMs,"%s","FL");
	}

	char YYs[3];
	int YYi = 14 + ((d00 >>10) & 3);
	sprintf(YYs,"%d",YYi);

	char WWs[3];
	int WWi = (d00 >>4) & 63;
	sprintf(WWs,"%d",WWi);

	char SNs[7];
	int SNi = (d01>>4 & (0x3FF)) | ( (d02>>4 & (0x3FF)) << 10);
	sprintf(SNs,"%6.6d",SNi);

	char PNRs[9];
	int PNRi = (d03 >> 11) & 7;
	sprintf(PNRs,"ELA-000%d",PNRi);

	char REVCs[2];
	int REVCi = (d03 >> 8) & 7;
	sprintf (REVCs,"%c" , REVCi+char('A'));

	char REVNs[3];
	int REVNi = (d03 >> 4) & 0xF;
	sprintf (REVNs,"%2.2d" , REVNi);

//	fprintf(stdout, GREEN "CM - %s\n" RESET , CMs);
//	fprintf(stdout, GREEN "YY - %s\n" RESET , YYs);
//	fprintf(stdout, GREEN "WW - %s\n" RESET , WWs);
//	fprintf(stdout, GREEN "SN - %s\n" RESET , SNs);
//	fprintf(stdout, GREEN "PN - %s\n" RESET , PNRs);
//	fprintf(stdout, GREEN "RV - %s%s\n" RESET , REVCs,REVNs);
//	fprintf(stdout, RED "VPD - %s%s%s%s%s%s%s\n" RESET , CMs,YYs ,WWs ,SNs ,PNRs , REVCs,REVNs);

	sprintf(pVpd->pnr,"%s",PNRs);
	sprintf(pVpd->revision,"%s%s",REVCs,REVNs);
	sprintf(pVpd->serial,"%s%s%s%s",CMs,YYs ,WWs ,SNs );

	return rc;
/*
	VPD:FL1420003205ELA-0005A02
	SN = FL1420003205
	PN=ELA-0005
	REV=A02
	CM=00
	YY=00
	WW=0x14=010100
	SN=3205=0xC85=0000000011,0010000101
	PNR=101
	REV©=000
	Rev(n)=0010
*/
	if (err) {
		fprintf(stderr, RED            "Failed reading AC2DC eeprom (err %d)\n" RESET, err);
		rc =  err;
	}
	return rc;
}

int readD0ofL02L3(int topbot , int * l0 , int * l1 , int * l2 , int * l3)
{
	int boardOffset = 12 * topbot;

	*l0 = dc2dc_get_dcr_inductor_cat((boardOffset + 0) , true);
	*l1 = dc2dc_get_dcr_inductor_cat((boardOffset + 1) , true);
	*l2 = dc2dc_get_dcr_inductor_cat((boardOffset + 2) , true);
	*l3 = dc2dc_get_dcr_inductor_cat((boardOffset + 3) , true);

	//fprintf(stderr , "0 %d, 1 %d, 2 %d, 3 %d\n",*l0,*l1,*l2,*l3);

	if (*l0 < 0 || *l1 < 0 || *l2 < 0 || *l3 < 0){
		return 3;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int rc = 0;


	bool callUsage = false;
	bool badParm = false;
	bool quiet = false;
	bool print_all = false;
	bool print_pnr = false;
	bool print_rev = false;
	bool print_ser = false;
	bool write = false;
	char vpd_str[32];
	int topOrBottom = -1; // default willbe TOP, start with -1, to rule out ambiguity.

	const char * h_board[] = {"TOP MAIN BOARD ","BOTTOM MAIN BOARD "};
	const char * h_all = "VPD: ";
	const char * h_pnr = "PNR: ";
	const char * h_rev = "REV: ";
	const char * h_ser = "SER: ";



	for (int i = 1 ; i < argc ; i++){
	if ( 0 == strcmp(argv[i],"-h"))
	  callUsage = true;
	else if ( 0 == strcmp(argv[i],"-q"))
	  quiet = true;
	else if ( 0 == strcmp(argv[i],"-a"))
	  print_all = true;
	else if ( 0 == strcmp(argv[i],"-p"))
	  print_pnr = true;
	else if ( 0 == strcmp(argv[i],"-r"))
	  print_rev = true;
	else if ( 0 == strcmp(argv[i],"-s"))
	  print_ser = true;
	else if ( 0 == strcmp(argv[i],"-v")){
	  write = true;
	  i++;
	  strncpy(vpd_str,argv[i],sizeof(vpd_str));
	}
	else if ( 0 == strcmp(argv[i],"-top")){
	  if (topOrBottom == -1 || topOrBottom == TOP_BOARD)
		  topOrBottom = TOP_BOARD;
	  else
		  badParm = true;
	}
	else if ( 0 == strcmp(argv[i],"-bottom")){
	  if (topOrBottom == -1 || topOrBottom == BOTTOM_BOARD)
		  topOrBottom = BOTTOM_BOARD;
	  else
		  badParm = true;
	}
	else
	  badParm = true;
	}

	if (write && (print_ser | print_rev | print_pnr | print_all | quiet)){
		badParm = true;
	}


	// if no print spec was given (all are false, then set all sub fields (except for all)
	if (!write &&  false == (print_all || print_pnr || print_rev || print_ser) ){
	print_pnr = true;
	print_rev = true;
	print_ser = true;
	}

	if(badParm)
	{
		usage(argv[0],1,"Bad arguments");
	}

	// applying default as top.
	if (-1 == topOrBottom)
	  topOrBottom = TOP_BOARD;

	if (callUsage)
	return usage(argv[0] , 0);

	int err = 0;

	i2c_init(&err);

	if (err != 0)
	{
		fprintf(stderr,"FAILED to init i2c bus\n");
		return 4;
	}


	 mainboard_vpd_info_t vpd = {}; // allocte, and initializero


	 int L0D0 , L1D0 , L2D0 , L3D0;
	 L0D0 = L1D0 = L2D0 = L3D0 = 0;

	 if ( readD0ofL02L3(topOrBottom , &L0D0 , &L1D0 , &L2D0 , &L3D0) != 0)
	 {
		 fprintf(stderr , "Failed to read D0 Registers!\n");
		 return 5;
	 }

	 if (write){
		 fprintf(stderr , RED "vpd to set: %s\n" RESET , vpd_str);
		 rc  = mainboard_set_vpd(&vpd , vpd_str , L0D0 , L1D0 , L2D0 , L3D0);

		 if (0 == rc)
		 {
				 printf("%s%s%s%s%s\n",quiet?"":h_board[topOrBottom],quiet?"":h_all,vpd.serial,vpd.pnr,vpd.revision);
		 }

	 }
	 else{
		 rc  = mainboard_get_vpd(&vpd , L0D0 , L1D0 , L2D0 , L3D0);

		 if (0 == rc)
		 {
			 if (print_all)
				 printf("%s%s%s%s%s\n",quiet?"":h_board[topOrBottom],quiet?"":h_all,vpd.serial,vpd.pnr,vpd.revision);
			 if (print_pnr)
				 printf("%s%s%s\n",quiet?"":h_board[topOrBottom],quiet?"":h_pnr,vpd.pnr);
			 if (print_rev)
				 printf("%s%s%s\n",quiet?"":h_board[topOrBottom],quiet?"":h_rev,vpd.revision);
			 if (print_ser)
				 printf("%s%s%s\n",quiet?"":h_board[topOrBottom],quiet?"":h_ser,vpd.serial);
		 }
	 }
	 return rc;
}
