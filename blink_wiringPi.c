#include <wiringPi.h>
#include <cs50.h>
#include <stdio.h>

int main (void)
{
	float ms[4], freq[4];
	int i = 0;
/*
	for (; i < 4; i++)
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
		ms[i] = 500.0 / freq[i];
		printf("Time is %.2f ms\n", ms[i]);
	}
*/
	wiringPiSetup(); // Activate GPIO
	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	digitalWrite(0, HIGH);
	digitalWrite(1, HIGH);
	digitalWrite(2, HIGH);
	digitalWrite(3, HIGH);
	printf("GPIO now is active!\n");
	delay(200);

   while(1)
   {
		delay(31.25);
		digitalWrite(3, LOW);
		delay(10.42);
		digitalWrite(2, LOW);
		delay(8.33);
		digitalWrite(1, LOW);
		delay(12.5);
		digitalWrite(3, HIGH);
		delay(4.17);
		digitalWrite(0, LOW);
		delay(16.67);
		digitalWrite(3, HIGH);
   }

return 0 ;
}
