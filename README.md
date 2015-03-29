
Lasersaur - Open Source Laser cutter
-------------------------------------

This is a port of the Lasersaur firmware to run on a K40-III 40W Chinese Laser. It runs on a Cortex-M4 and takes g-code files to control the stepper motors, laser pulse and aux outputs.


**DISCLAIMER:** Please be aware that operating a DIY laser cutter can be dangerous and requires full awareness of the risks involved. You build the machine and you will have to make sure it is safe. The instructions of the Lasersaur project and related software come without any warranty or guarantees whatsoever. All information is provided as-is and without claims to mechanical or electrical fitness, safety, or usefulness. You are fully responsible for doing your own evaluations and making sure your system does not burn, blind, or electrocute people.


Grbl - An embedded g-code interpreter and motion-controller for the Arduino/AVR328 microcontroller
--------------

For more information [on Grbl](https://github.com/simen/grbl)


Development Environment
-----------------------

- Linux Mint 17
- gcc-arm-none-eabi 4.9.3 (https://launchpad.net/~terry.guo/+archive/ubuntu/gcc-arm-embedded)
- Eclipse 3.8.1
- lm4tools (https://github.com/utzig/lm4tools.git) for flashing
- openocd 0.7.0 for debug

Once these components are installed in Eclipse:
- Import... 
- General -> Existing Projects into Workspace
- Point it to the LasaurGrbl.git clone

Stellaris vs. Tiva-C
--------------------

TI seem to have had a re-shuffle internally. 
For us, this means a little confusion with the part numbers. 
The TI Stellaris board is now obsolete. The LM4F120 previously used has been renamed as TM4C1233H6PM.
The replacement board is called Tiva-C and uses a TM4C123GH6PM (note the missing 3).
From what I can see the boards are the same other than a silk screen and the MCU.

The code base now supports both boards via build settings in Eclipse. 
- Right click on the project
- Build Configurations -> Set Active -> Select LM4F120 or TM4C123G.

