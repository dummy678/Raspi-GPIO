/*
 * FREQUENCY:7.5 10 12 15 
 * DUTY   :66.7(67)  50  41.7(42)  33.3(33)
 * */

#include <wiringPi.h>
#include <cs50.h>
#include <stdio.h>
int main (void)
{
   // float freq_GPIO0,freq_GPIO1,freq_GPIO2,freq_GPIO3 = 0;
    int  du_time_GPIO0 = 67,du_time_GPIO1 = 50,du_time_GPIO2 = 42,du_time_GPIO3 = 33;
    
    /*激活GPIO口，使之可用*/
    wiringPiSetup () ; // Activate GPIO
    pinMode (0, OUTPUT) ;
    printf("GPIO now is active!\n");

    /*用delay进行延时，每次延时计数一次，*/

    int counter = 0;//计数器
    while(1)
    {  
        delay(1);
        counter++;
        /*LED灯是低电平亮，高电平不亮*/
        if(counter >= du_time_GPIO0 && (counter % (2 * du_time_GPIO0)) != 0)
        {
            digitalWrite (0, LOW);
        }
        else if(counter >= du_time_GPIO0 && (counter % (2 * du_time_GPIO0) ==0))
        {
            digitalWrite (0, HIGH);
        }
            
        if(counter >= du_time_GPIO1 && (counter % (2 * du_time_GPIO1)) != 0)
        {
            digitalWrite (1, LOW);
        }
        else if(counter >= du_time_GPIO1 && (counter % (2 * du_time_GPIO1) ==0))
        {
            digitalWrite (1, HIGH);
        }
        if(counter >= du_time_GPIO2 && (counter % (2 * du_time_GPIO2)) != 0)
        {
            digitalWrite (2, LOW);
        }
        else if(counter >= du_time_GPIO2 && (counter % (2 * du_time_GPIO2) ==0))
        {
            digitalWrite (2, HIGH);
        }
        if(counter >= du_time_GPIO3 && (counter % (2 * du_time_GPIO3)) != 0)
        {
            digitalWrite (3, LOW);
        }
        else if(counter >= du_time_GPIO3 && (counter % (2 * du_time_GPIO3) ==0))
        {
            digitalWrite (3, HIGH);
        }
        

        else
        {
            digitalWrite (0, HIGH);
            digitalWrite (1, HIGH);
            digitalWrite (2, HIGH);
            digitalWrite (3, HIGH);
        }

        if(counter == 134)
        {
            counter = 0;
        }

    }

    return 0 ;
}
