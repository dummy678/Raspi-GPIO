/*
         This file is part of freq_pi Copyright (c) Jan Panteltje 2013-always
 email: panteltje@yahoo.com
					
	This code contains parts of code from Pifm.c 


Start GPL license:
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

If you take this code and port it to that Redmond crap you STILL must release SOURCE code,
this program has secret beyond bit level encrypted bits that make it traceable and you will be bitten by the FSF if you violate these terms.
*/

                       /* set TABs to 4 for correct formatting of this file, or if you do not know what that is replace all TABs with 4 spaces, else you cannot READ this */

/*
Program name:
fre_pi

Function:
programmable frequency generator on GPIO_4 pin 7


To compile:
gcc -Wall -O4 -o test test.c -std=gnu99 -lm


To install:
cp freq_pi /usr/local/bin/

To run:
freq_pi
*/

#define PROGRAM_VERSION "0.1"



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>


#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
char *gpio_mem, *gpio_map;
char *spi0_mem, *spi0_map;


// I/O access
volatile unsigned *gpio;
volatile unsigned *allof7e;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GPIO_GET *(gpio+13)  // sets   bits which are 1 ignores bits which are 0

#define ACCESS(base) *(volatile int*)((int)allof7e+base-0x7e000000)
#define SETBIT(base, bit) ACCESS(base) |= 1<<bit
#define CLRBIT(base, bit) ACCESS(base) &= ~(1<<bit)

#define CM_GP0CTL (0x7e101070)
#define GPFSEL0 (0x7E200000)
#define CM_GP0DIV (0x7e101074)
#define CLKBASE (0x7E101000)
#define DMABASE (0x7E007000)
#define PWMBASE  (0x7e20C000) /* PWM controller */


struct GPCTL
{
char SRC         : 4;
char ENAB        : 1;
char KILL        : 1;
char             : 1;
char BUSY        : 1;
char FLIP        : 1;
char MASH        : 2;
unsigned int     : 13;
char PASSWD      : 8;
};



void getRealMemPage(void** vAddr, void** pAddr)
{
void* a = valloc(4096);
    
((int*)a)[0] = 1;  // use page to force allocation.
    
mlock(a, 4096);  // lock into ram.
    
*vAddr = a;  // yay - we know the virtual address
    
unsigned long long frameinfo;
    
int fp = open("/proc/self/pagemap", 'r');
lseek(fp, ((int)a)/4096*8, SEEK_SET);
read(fp, &frameinfo, sizeof(frameinfo));
    
*pAddr = (void*)((int)(frameinfo*4096));
}


void freeRealMemPage(void* vAddr)
{
munlock(vAddr, 4096);  // unlock ram.
    
free(vAddr);
}


void start_rf_output(int source)
{
/* open /dev/mem */
if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
	{
	printf("can't open /dev/mem \n");
	exit (-1);
	}
    
allof7e = (unsigned *)mmap( NULL, 0x01000000,  /*len */ PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0x20000000  /* base */ );

if ((int)allof7e==-1) exit(-1);

SETBIT(GPFSEL0 , 14);
CLRBIT(GPFSEL0 , 13);
CLRBIT(GPFSEL0 , 12);

/*
Clock source
0 = GND
1 = oscillator
2 = testdebug0
3 = testdebug1
4 = PLLA per
5 = PLLC per
6 = PLLD per
7 = HDMI auxiliary
8-15 = GND
*/

 
struct GPCTL setupword = {source/*SRC*/, 1, 0, 0, 0, 1,0x5a};

ACCESS(CM_GP0CTL) = *((int*)&setupword);
}


void modulate(int m)
{
ACCESS(CM_GP0DIV) = (0x5a << 24) + 0x4d72 + m;
}


struct CB
{
volatile unsigned int TI;
volatile unsigned int SOURCE_AD;
volatile unsigned int DEST_AD;
volatile unsigned int TXFR_LEN;
volatile unsigned int STRIDE;
volatile unsigned int NEXTCONBK;
volatile unsigned int RES1;
volatile unsigned int RES2;
};


struct DMAregs
{
volatile unsigned int CS;
volatile unsigned int CONBLK_AD;
volatile unsigned int TI;
volatile unsigned int SOURCE_AD;
volatile unsigned int DEST_AD;
volatile unsigned int TXFR_LEN;
volatile unsigned int STRIDE;
volatile unsigned int NEXTCONBK;
volatile unsigned int DEBUG;
};


struct PageInfo
{
void* p;  // physical address
void* v;   // virtual address
};


struct PageInfo constPage;   
struct PageInfo instrPage;
struct PageInfo instrs[1024];




void print_usage()
{
fprintf(stderr,\
"\nPanteltje freq_pi-%s\n\
Usage:\nfreq_pic -f frequency [-h]\n\
-f int        frequency to ouput on GPIO_4 pin 7.\n\
-h            help (this help).\n\
\n",\
PROGRAM_VERSION);
fprintf(stderr,\
"Example for 1 MHz:\n\
freq_pi -f 100000000\n\
\n"\
);

} /* end function print_usage */




int main(int argc, char **argv)
{
int a;
uint32_t frequency = 0; // -Wall
uint32_t ua;

/* defaults */



/* end defaults */



/* proces any command line arguments */
while(1)
	{
	a = getopt(argc, argv, "f:h");
	if(a == -1) break;

	switch(a)
		{
		case 'f':	// frequency
			a = atoi(optarg);


			frequency = a;
			break;
		case 'h': // help
			print_usage();
			exit(1);
			break;
			break;
        case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 				{
 				fprintf(stderr, "send_iq: unknown option `-%c'.\n", optopt);
 				}
			else
				{
				fprintf(stderr, "send_iq: unknown option character `\\x%x'.\n", optopt);
				}
			print_usage();

			exit(1);
			break;			
		default:
			print_usage();

			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */


/* set frequeny */

/*
From ducument BCM2835-ARM-Peripherals.pdf
 page 105
   6.3 General Purpose GPIO Clocks


The General Purpose clocks can be output to GPIO pins.
They run from the peripherals clock sources and use clock generators with noise-shaping MASH dividers.
These allow the GPIO clocks to be used to drive audio devices.
The fractional divider operates by periodically dropping source clock pulses, therefore the output frequency will periodically switch between:
source_frequency / DIVI, and   source_frequency / (DIVI + 1)

Jitter is therefore reduced by increasing the source clock frequency.
In applications where jitter is a concern, the fastest available clock source should be used.
The General Purpose clocks have MASH noise-shaping dividers which push this fractional divider jitter out of the audio band.
MASH noise-shaping is incorporated to push the fractional divider jitter out of the audio band if required.
The MASH can be programmed for 1, 2 or 3-stage filtering. MASH filter, the frequency is spread around the requested frequency and the user must ensure that the module is not exposed to frequencies higher than 25MHz.
Also, the MASH filter imposes a low limit on the range of DIVI.
                                                                                                              
MASH           min DIVI min output freq       average output freq             max output freq
0 (int divide) 1        source / ( DIVI )     source / ( DIVI )               source / ( DIVI )
1              2        source / ( DIVI )     source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 1 )
2              3        source / ( DIVI - 1 ) source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 2 )
3              5        source / ( DIVI - 3 ) source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 4 )
Table 6-32 Effect of MASH Filter on Frequency

The following example illustrates the spreading of output clock frequency resulting from the
use of the MASH filter. Note that the spread is greater for lower divisors.
                   PLL  target                          min    ave   max
                   freq   freq                          freq   freq  freq
                  (MHz) (MHz)  MASH  divisor DIVI DIVF (MHz)  (MHz) (MHz) error
                   650   18.32  0    35.480   35  492  18.57  18.57 18.57   ok
                   650   18.32  1    35.480   35  492  18.06  18.32 18.57   ok
                   650   18.32  2    35.480   35  492  17.57  18.32 19.12   ok
                   650   18.32  3    35.480   35  492  16.67  18.32 20.31   ok
                   400   18.32  0    21.834   21  854  19.05  19.05 19.05   ok
                   400   18.32  1    21.834   21  854  18.18  18.32 19.05   ok
                   400   18.32  2    21.834   21  854  17.39  18.32 20.00   ok
                   400   18.32  3    21.834   21  854  16.00  18.32 22.22   ok
                   200   18.32  0    10.917   10  939  20.00  20.00 20.00   ok
                   200   18.32  1    10.917   10  939  18.18  18.32 20.00   ok
                   200   18.32  2    10.917   10  939  16.67  18.32 22.22   ok
                   200   18.32  3    10.917   10  939  14.29  18.32 28.57 error
            Table 6-33 Example of Frequency Spread when using MASH Filtering


Operating Frequency
The maximum operating frequency of the General Purpose clocks is ~125MHz at 1.2V but this will be reduced if the GPIO pins are heavily loaded or have a capacitive load.


Register Definitions
    Clock Manager General Purpose Clocks Control (CM_GP0CTL, GP1CTL &
                                                 GP2CTL)
Address         0x 7e10 1070 CM_GP0CTL
                0x 7e10 1078 CM_GP1CTL
                0x 7e10 1080 CM_GP2CTL
            Bit      Field                                                            Read/
                                                   Description                              Reset
         Number     Name                                                              Write
          31-24    PASSWD    Clock Manager password "5a"                                W     0
          23-11        -     Unused                                                     R     0
           10-9     MASH     MASH control                                              R/W    0
                             0 = integer division
                             1 = 1-stage MASH (equivalent to non-MASH dividers)
                             2 = 2-stage MASH
                             3 = 3-stage MASH
                             To avoid lock-ups and glitches do not change this
                             control while BUSY=1 and do not change this control
                             at the same time as asserting ENAB.
             8       FLIP    Invert the clock generator output                         R/W    0
                             This is intended for use in test/debug only. Switching
                             this control will generate an edge on the clock
                             generator output. To avoid output glitches do not
                             switch this control while BUSY=1.
             7      BUSY     Clock generator is running                                 R     0
                             Indicates the clock generator is running. To avoid
                             glitches and lock-ups, clock sources and setups must
                             not be changed while this flag is set.
             6         -     Unused                                                     R     0
             5       KILL    Kill the clock generator                                  R/W    0
                             0 = no action
                             1 = stop and reset the clock generator
                             This is intended for test/debug only. Using this control
                             may cause a glitch on the clock generator output.
             4      ENAB     Enable the clock generator                                R/W    0
                             This requests the clock to start or stop without
                             glitches. The output clock will not stop immediately
                             because the cycle must be allowed to complete to
                             avoid glitches. The BUSY flag will go low when the
                             final cycle is completed.
            3-0      SRC     Clock source                                              R/W    0
                             0 = GND
                             1 = oscillator
                             2 = testdebug0
                             3 = testdebug1
                             4 = PLLA per
                             5 = PLLC per
                             6 = PLLD per
                             7 = HDMI auxiliary
                             8-15 = GND
                             To avoid lock-ups and glitches do not change this
                             control while BUSY=1 and do not change this control
                             at the same time as asserting ENAB.
06 February 2012 Broadcom Europe Ltd. 406 Science Park Milton Road Cambridge CB4 0WW              Page 107
                            2012 Broadcom Corporation. All rights reserved


 Clock Manager General Purpose Clock Divisors (CM_GP0DIV, CM_GP1DIV &
                                          CM_GP2DIV)
Address       0x 7e10 1074 CM_GP0DIV
              0x 7e10 107c CM_GP1DIV
              0x 7e10 1084 CM_GP2DIV
 
         Bit      Field                                                         Read/
                                                Description                           Reset
       Number    Name                                                           Write
        31-24   PASSWD    Clock Manager password "5a"                             W     0
        23-12     DIVI    Integer part of divisor                                R/W    0
                          This value has a minimum limit determined by the
                          MASH setting. See text for details. To avoid lock-ups
                          and glitches do not change this control while BUSY=1.
        11-0      DIVF    Fractional part of divisor                             R/W    0
                          To avoid lock-ups and glitches do not change this
                          control while BUSY=1.
                        Table 6-35 General Purpose Clock Divisors
*/




//struct GPCTL setupword = {6 /* clock source */, 1 /* enable */,  0 /* not kill */, 0 , 0, 1 /* 1 stage MASH (equivalent to no MASH */, 0x5a /* password */ };

//ACCESS(CM_GP0CTL) = *((int*)&setupword);


//ACCESS(CM_GP0DIV) = (0x5a << 24) + 0x4d72 + m

uint16_t divi; // integer part divider [23:12]		12 bits wide, max 4095
uint16_t divf; // fractional part divider [11:0]	12 bits wide, max 4095


/*
clock sources are 650 MHz, 400 MHz, and 200 MHz

So the lowest frequency we can make is 200,000,000 / 4095.4095 = 48,835.165323516 Hz 

But I get 61,043 Hz

4095.4095 * 61043 = 249,996,082.108499999 Hz....

But for MASH 1,
 MASH           min DIVI min output freq       average output freq             max output freq
 0 (int divide) 1        source / ( DIVI )     source / ( DIVI )               source / ( DIVI )
* 1              2        source / ( DIVI )     source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 1 )
 2              3        source / ( DIVI - 1 ) source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 2 )
 3              5        source / ( DIVI - 3 ) source / ( DIVI + DIVF / 1024 ) source / ( DIVI + 4 )

200,000,000 / (4095 = 48840.048840048
200,000,000 / (4095 + (4095/1024) ) = 48792.400011912


So 
61043  * 4099

61043 * (4095 + .3999023437) = 249995496.238766479 Hz, looks liike we have a 250 MHz clock.

Lowest frequency then is 61,043 Hz,
highest frequency then is 250,000,000 / 1 = 250,000,000 Hz
*/

//#define PLL0_FREQUENCY 250000000.0
#define PLL0_FREQUENCY 500000000.0

//if(frequency 

int clock_source;
/*
Clock source
0 = GND
1 = oscillator
2 = testdebug0
3 = testdebug1
4 = PLLA per
5 = PLLC per
6 = PLLD per
7 = HDMI auxiliary
8-15 = GND
*/

/* init hardware */

clock_source = 6; /* this GPIO_4 pin 7 allows only the 200 MHz clock????? as source, the other clock GPIO lines are not on a pin in revision 2 board, so we have to work with 200 MHz, and that seems to be 250 MHz */

start_rf_output(clock_source);

/* calculate divider */
double da; 

da = PLL0_FREQUENCY / (double) frequency;

divi = (int) da;
divf = 4096.0 *(da - (double)divi);

//fprintf(stderr, "frequency=%d da=%f divi=%d divf=%d\n", frequency, da, divi, divf);

if( (divi > 4095.0) || (divi < 1.0) )
	{
	fprintf(stderr, "freq_pi: requested frequency out of range, aborting.\n");

	exit(1);
	}


if(divf > 4095.0)
	{
	fprintf(stderr, "freq_pi: requested frequency out of range, aborting.\n");

	exit(1);
	}

ua = (0x5a << 24) + (divi << 12) + divf;

ACCESS(CM_GP0DIV) = ua;

//fprintf(stderr, "WAS here\n");

exit(0);
} /* end function  main */

