/* ==========================================================================
generating arbitrary timed waveforms on multiple user gpios simultaneously;
uses DMA and interrupts are not disabled;

To compile:
    gcc -o yourprog yourprog.c -lpigpio -lrt -lpthread -lcs50 -O3
==========================================================================  */

// To-Do : I am creating the wave! 

#include <stdio.h>
#include <pigpio.h>
#include <cs50.h>

//  GPIO port number is represented by their Boradcom Number (BCM)
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

int main(void)
{
	//Asking for how long will the LEDs to blink (wave to transmit)
	int secs = 0;
	printf("how long will the LEDs to blink (wave to transmit)?\n");
	do
	{
		secs = GetInt();
		printf("Second(s)");
		if (secs > 3600)
		{
			printf("The max time is 3600 seconds, please retry:\n");
		}
	} while (secs <= 0 || secs > 3600);
	printf("The LEDs will blink for %d seconds\n", secs);

	//Asking for frequncy input and convert it into ms
	float us[4], freq[4];
	int i = 0;
	for (i; i < 4; i++)
	{
		do
		{
			printf("Pls input a freq for port %d:\n", i);
			freq[i] = GetFloat();
			if (freq <= 0)
			{
				printf("error: the frequency must be greater than zero!\nplease retry:\n");
			}
		} while (freq <= 0);
		printf("Frequency: %.2f Hz\n", freq[i]);
		us[i] = 500.0 / freq[i];
		printf("Time is %.2f ms\n", us[i]);
	}

	//Initializing the "pigpio" library and check if initialization sucessful 
	printf("Initialzing GPIO...");
	if (gpioInitialise() < 0)
	{
		printf("error:Initializing failed!\n");
		return 1;
	}

	printf("Setting up GPIO ports...\n");
	// Set the port mode to "output" and into HIGH
	gpioSetMode(PORT0, PI_OUTPUT);
	gpioSetMode(PORT1, PI_OUTPUT);
	gpioSetMode(PORT2, PI_OUTPUT);
	gpioSetMode(PORT3, PI_OUTPUT);
	gpioWrite(PORT0, 1);
	gpioWrite(PORT1, 1);
	gpioWrite(PORT2, 1);
	gpioWrite(PORT3, 1);

	//While using NPN transistor HIGH on the GPIO pin will turn transistor on
	//while using a PNP transistor a LOW on the pin turn the transistor on
	
	//Creating a square wave
	printf("Creating the wave..\n");
	gpioPulse_t pulse[2]; // The original comment is misleading ---(only need two pulses for a square wave)
	
	/**************************************************************
		The fields specify

		1) the gpios to be switched on at the start of the pulse.
		2) the gpios to be switched off at the start of the pulse.
		3) the delay in microseconds before the next pulse. 
	***************************************************************/

	pulse[0].gpioOn = 0;
	pulse[0].usDelay = 31.25;
	pulse[0].gpioOff = (1 << PORT3);
	gpioWaveClear();
	gpioWaveAddNew();
	gpioWaveAddGeneric(2, pulse);

	int wave_id = gpioWaveCreate();
	printf("wave id : %d\n", wave_id);
	if (wave_id < 0)
	{
		printf("error:Failed to create wave!\n");
		return 2;
	}

	//Begin to transmitting the wave
	printf("Wave transmitting...\n");
	gpioWaveTxSend(wave_id, PI_WAVE_MODE_REPEAT);
	sleep(secs);
	gpioWaveTxStop();
	gpioTerminate();
	printf("error:Failed to create wave!\n");

	return 0;
}

