下面的函数提供了用来实现对GPIO单独控制的支持，麻烦看一看。
"Timer"文件夹里面有各种定时器，请看一看“GPIO_Pi.c”.

int gpioSetAlertFunc(unsigned user_gpio, gpioAlertFunc_t f)
Registers a function to be called (a callback) when the specified gpio changes state.

user_gpio: 0-31
        f: the callback function


Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

One function may be registered per gpio.

The function is passed the gpio, the new level, and the tick.

The alert may be cancelled by passing NULL as the function.

The gpios are sampled at a rate set when the library is started.

If a value isn't specifically set the default of 5 us is used.

The number of samples per second is given in the following table.

              samples
              per sec

         1  1,000,000
         2    500,000
sample   4    250,000
rate     5    200,000
(us)     8    125,000
        10    100,000


Level changes shorter than the sample rate may be missed.

The thread which calls the alert functions is triggered nominally 1000 times per second. The active alert functions will be called once per level change since the last time the thread was activated. i.e. The active alert functions will get all level changes but there will be a latency.

The tick value is the time stamp of the sample in microseconds, see gpioTick for more details.

Example

void aFunction(int gpio, int level, uint32_t tick)
{
   printf("gpio %d became %d at %d\n", gpio, level, tick);
}

// call aFunction whenever gpio 4 changes state

gpioSetAlertFunc(4, aFunction);
int gpioSetAlertFuncEx(unsigned user_gpio, gpioAlertFuncEx_t f, void *userdata)
Registers a function to be called (a callback) when the specified gpio changes state.

user_gpio: 0-31
        f: the callback function
 userdata: pointer to arbitrary user data


Returns 0 if OK, otherwise PI_BAD_USER_GPIO.

One function may be registered per gpio.

The function is passed the gpio, the new level, the tick, and the userdata pointer.

Only one of gpioSetAlertFunc or gpioSetAlertFuncEx can be registered per gpio.

See gpioSetAlertFunc for further details. 
