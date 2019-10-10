Read me
=====================
Hardware version 0.04 is the version that was first used at RobotChallenge 2012

NOTE: When programming, please use the following fusebit settings: E2D7FF when not using an external crystal.
Please visit http://www.engbedded.com/fusecalc to see check what this setting represents.

To program (using pololu stk500 programmer)
avrdude -c stk500 -P/dev/cu.usbmodem002130932 -pm168p -u -Uflash:w:remoteControl.hex -Ulfuse:w:0xe2:m -Uhfuse:w:0xd7:m -Uefuse:w:0xff:m

Note: You might need to change the port and progammer it to work for you
