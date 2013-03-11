This file doesn't need to be placed in the TS!!

This file describes the content, the functionality and the use of the files in this folder.

The file netdog.c is the source code of the program 'netdog', the file 'Make' and 'sbus.h' are for compiling and develop purposes.

The file netdog is a executable that is designed to ensure a system be avaiable always and if it is lost, reboot the system.

This program uses the watchdog mechanism to make its work.

This program recives an argument (number) thet specify if this routine must have a delay before activating the watch dog system.
This program ensure the right working of the device, and decides when to stopp feeding the watch dog.

The file 'netdog' must be placed in the system and must be run at the beggining of the system load.

For example if the file is in the path '/root/netdog'
The the following entry must be added to the file "/etc/rc.local" 
'/root/netdog 30&'

!!! be sure to put the entry before the line "exit 0" !!!

Note:  The watchdog should not be feeded in the programs running over the system, otherwise the system conection wont be warranted

Note2:  This programs version is designed to run in the embeded system 'Ts75xx'
