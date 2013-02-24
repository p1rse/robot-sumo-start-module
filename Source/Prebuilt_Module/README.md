Read Me
=====================
Hardware version 0.16 is the version that was first sold at RobotChallenge 2012

======================
Programming the device
======================
For those who want to build their own modules based on this code and microcontroller, it is important that the fuse are correct when uploading the hex-file. Incorrect settings may  brick your chip.

Using the following avrdude command is used when programming the prebuilt modules. It utilizes the AvrISP MKII programmer, so if you use any other programmer, change the programming command accordingly. Make sure to be in the correct folder, or replace "StartModule.hex" to the correct path to your hex-file.

avrdude	-pt13 -cavrispmkII -Pusb -u -Uflash:w:StartModule.hex:a -Ulfuse:w:0x79:m-Uhfuse:w:0xef:m

These fuse bits sets the following. The ones marked (important) have the potential to brick your system if set incorrectly. Set them as below and you should be fine:
- No brownout detection
- No clock division by 8
- No Debug wire
- No Eeprom preserve through programming
- No reset disable (important)
- Self Programming enabled (important)
- SPI programming enabled (important)
- Clock source as 4.8MHz, 14CK 64ms startup time (important)
- No watchdog

You can check http://www.engbedded.com/fusecalc for further information about fusebtis.

When the flash has been written, the module will go into "Stopped" state, which means it will blink slowly. Cycle the power and it should be ready to use.