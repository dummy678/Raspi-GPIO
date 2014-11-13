/* ==========================================================================
generating arbitrary timed waveforms on multiple user gpios simultaneously;
uses DMA and interrupts are not disabled;

To compile:
    gcc -o yourprog yourprog.c -lpigpio -lrt -lpthread -lcs50
==========================================================================  */

    #include <stdio.h>
    #include <pigpio.h>
	#include <cs50.h>

//  GPIO port number is represented by theoir Boradcom Number (BCM)
    #define PORT0 17
	#define PORT1 18
	#define PORT2 27 
	#define PORT3 22
/********************************************************************************
+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
|     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
|   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5V      |     |     |
|   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
|   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | ALT0 | TxD     | 15  | 14  |
|     |     |      0v |      |   |  9 || 10 | 1 | ALT0 | RxD     | 16  | 15  |
|  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
|  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
|  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
|     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
|  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
|   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
|  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
|     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
|   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
|   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
|   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
|  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
|  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
|  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
|     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+

*********************************************************************************/

    gpioPulse_t pulse[2]; /* only need two pulses for a square wave */

    int main(int argc, char *argv[])
    {
		printf("There is no sanity check, so be a good user!"\n)
		int secs, us = 0;
		printf("How many seconds would you like program to run?\n");
		secs = GetInt();

       if (argc>1) us   = atoi(argv[1]); /* square wave micros */
       if (argc>2) secs = atoi(argv[2]); /* program run seconds */

       if (us<2) us = 2; /* minimum of 2 micros per pulse */

       if ((secs<1) || (secs>3600)) secs = 3600;

	   if (gpioInitialise() < 0)
			{
			printf("Initializing failed!\n");
			return 1;
			}

       gpioSetMode(LED, PI_OUTPUT);

       pulse[0].gpioOn = (1<<LED); /* high */
       pulse[0].gpioOff = 0;
       pulse[0].usDelay = us;

       pulse[1].gpioOn = 0;
       pulse[1].gpioOff = (1<<LED); /* low */
       pulse[1].usDelay = us;

       gpioWaveClear();

       gpioWaveAddGeneric(2, pulse);

       gpioWaveTxStart(PI_WAVE_MODE_REPEAT);

       sleep(secs);

       gpioWaveTxStop();

       gpioTerminate();
    }
