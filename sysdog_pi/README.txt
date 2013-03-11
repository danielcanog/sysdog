== sysdog for raspberry pi ==

==== intro ====

look at the main readme file "../README.txt" for information about sysdog.

== Raspberry pi cautions ==

==== enable watchdog module ====

The raspberry pi has a watchdog module implemented, but the system "raspbian" doesn't have it enabled by default, so be suere to have de module enable before running 'sysdog'.

To enable the module folllow the following actions.
  >sudo modprobe bcm2708_wdog
  >sudo nano /etc/modules
  # Add the line "bcm2708_wdog"
  # reboot to enable module

and you are good to go
