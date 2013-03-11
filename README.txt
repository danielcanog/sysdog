==== Introduction ====

'Sysdog'  is a 'watchdog' handler for Linux running systems, which attempts to ensure the 24/7 availability of the system.
This program is designed to run on embedded systems running Linux, systems with 'watchdog' module, systems with specific function that must be available 24/7.

==== Functionality ==== 

'sysdog' serves three functions.
1- Check that the system can access to a network.
2- Run a specific program and verify that this process.
2.1- be running
2.2- Don't crash. (is consuming processor)

When one of these conditions fails for a while, 'sysdog' reboots the system.

The restart is done in two ways.
1-First try to restart the system software using the 'reboot' command.
2-If for some reason the previous attempt fails, it stops feeding the 'watchdog' module for a hardware reboot.

!'Sysdog' also ensures that the operating system itself doesn't crash, since it activates and feeds the 'watchdog' module.

==== Features ==== 

* - 'Sysdog' is easily adaptable to the needs of each situation, since all functionality can be enabled or disabled individually.
* - 'Sysdog' is easily portable across various embedded systems, because source code has clearly defined the functions that needs to be reimplemented for each system.

Currently developed and compiled for the following systems.
* - 'Ts75xx' of "Technologic Systems"
* - 'Raspberry Pi'

==== Execution ====

'Sysdog' receives two arguments
1-web address to check network access.
2-Command to execute and survey.

Example:

 > sysdog "www.udea.edu.co" "ping www.google.com &> log.txt"

In this example the first argument ("www.udea.edu.co") says that this is the address that will be used to check the connection (network access)
The second argument ("ping www.google.com &> log.txt") is the command to be executed and will be check to be always working.

This program is designed to run when the operating system is starting with superuser permissions.
One way to do this is by adding the execution line to the '/etc/rc.local' script

Example:
(Assuming the file 'sysdog' (binary) is located in '/root/sysdog')
Add the following line in the script '/etc/rc.local'
 /root/sysdog "www.udea.edu.co" "ping www.google.com"

Warning: In the script '/etc/rc.local' is important to make sure the line to be executed before the line 'exit 0'

==== Other Examples ====

*-Check only network access
   >sysdog "www.udea.edu.co"
*-Survey only the execution of the program
   >sysdog 127.0.0.1 "ping www.google.com &> log.txt"
*-Check only internet access (access to google.com)
   >sysdog
* Enable single-watchdog module.
   >sysdog 127.0.0.1 

==== Precautions ====

*-The network access is tested using '/bin/ping'; The '-w'  option is used. Some implementations of '/bin/ping' does not have that option.
  To check if your implementation has this opecion run
   >ping --help
  And look for that option.
  If you do not have that option you can try the program update '/bin/ping' running
   >sudo apt-get install iputils-ping

*-To validate the command to run, the program 'file' is used.
  To check if your system has the program run
   >file
  And check if found.
  If you have not installed the program install it running
   >sudo apt-get install file

* If the address to test the network access is a direction that needs 'dns' resolve (the default address is) is extremely important have properly configured server 'dns' otherwise the hardware watchdog will reboot the system when activated (approx 30 sec after the boot).

==== Development ==== 
==== Compiling for new systems =====

1-Create a .h and implement in 'C' the following functions to work with your system.
      void enable_wd() / / Enable watch dog
      void feed_dog() / / feed dog

2-In the file './sysdog.c' edit the line that looks like this
include "wd_ts75xx.h" / / Include header That Implement 'enable_wd ()' and 'feed_dog ()' functions
  and then include the file created in the previous point

3-Edit the file './Makefile' and make the variable 'CC' to point to the compiler (or cross compiler) for your system
  CC = /mnt/NFS-RW/cross-compiler-tools/Ts/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-gcc

4-Run
   > Make
