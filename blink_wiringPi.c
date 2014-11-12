#include <wiringPi.h>
#include <cs50.h>
#include <stdio.h>

int main (void)
{
   float freq, m_sec = 0;
   int i = 0;
   for (i = 0; i < 4; i++)
   {
      do
      {
         printf("Pls input a freq for GPIO %d:\n", i);
         freq = GetFloat();
      }
      while (freq < 0);
      printf("Frequency for GPIO %d is: %.2f Hz\n", i, freq);
      m_sec = 500.0/freq;
      printf("Time is %.2f ms\n", m_sec);
   }

/*
   printf("GPIO now is active!\n");
   wiringPiSetup () ; // Activate GPIO
   pinMode (0, OUTPUT) ;

   while(1)
   {
      digitalWrite (0, HIGH);
      delay (m_sec[2]) ;
      digitalWrite (0,  LOW);
      delay (m_sec[2]) ;
   }
*/

    return 0 ;
}
