#ifndef WD_PI_H
#define WD_PI_H

#include <stdlib.h>
#include <fcntl.h>

//In order to make this functions work propertly, the watchdog module must be enabled in the 'raspberri pi'
//To do that make the following actions.
//>sudo modprobe bcm2708_wdog
//>sudo nano /etc/modules
//# Add the line "bcm2708_wdog"
//# reboot to enable module


int _wd_fd = 0;

//Enable watch dog
void enable_wd()
{
    //Enable the watch Dog
    _wd_fd = open("/dev/watchdog", O_WRONLY);
    if(_wd_fd <= 0)
    {
        perror("Error enableing watchdog. (/dev/watchdog could not be open).\n");
        _wd_fd = 0;
    }
}



//feed dog (about 16 seconds)
void feed_dog()
{
    if(_wd_fd != 0)
          write(_wd_fd, ":)", 2);
}

#endif // WD_TS75XX_H
