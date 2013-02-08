#ifndef WD_TS75XX_H
#define WD_TS75XX_H

#include <stdlib.h>
#include "sbus.h"


//Enable watch dog
void enable_wd()
{
    //Enable the watch Dog
    int err = system("killall ts7500ctl");
    if(err != 0)
        perror("Error Killing 'ts7500ctl'.\n");

    err = system("/usr/local/bin/ts7500ctl -W");
    if(err != 0)
        perror("Error Running 'ts7500ctl -W'.\n");

//    err = system("/usr/local/bin/ts7500ctl -a 0x72 -w 2");
//    if(err != 0)
//        perror("Error Running 'ts7500ctl -a 0x72 -w 2'.\n");
}



//Feeds watch dog
//sec can be
//0	 feed watchdog for another .338s
//1	 feed watchdog for another 2.706s
//2	 feed watchdog for another 10.824s
//3	 disable watchdog
void feed_ts_dog(int sec)
{
        sbuslock();
        sbus_poke16(0x74, sec);
        sbusunlock();
}

//feed dog
void feed_dog()
{
    feed_ts_dog(2);
}

#endif // WD_TS75XX_H
