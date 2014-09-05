ARM-CNC
=======

A CNC controller base on LPC1343 Cortex-M3 Chip

Installation
-------
1. LPC1343 Side

    This Require the LPCXpresso to Compile and program it onto the chip, project file is included in the LPC1343 Code         folder.
    
2. PC Side
    
    PC side Code is written in Python 3, should be able to run on any platform(Windows, Linux, MAC OSX).
    
    Dependience:
        - Python 3
        - pyserial
        - numpy
    
    You should be able to install pyserial and numpy through pip install.

Usage
------
The following procdure demonstrate how to use a svg file to control the machine.

1. I will start with Adobe Illastrator, draw any shape you like in mm as unit, 
    use file->Export to export as a .svg file.
2. open up jscut.org, Click Launch, and choose to import the svg file you just saved.
3. Click on the shape you drawed on jscut's web page.
4. Set the settings according to this
    - Left Upper Side, set the px per mm for Illastrator
    - Right Bottom Side, Click all in mm
    - Left Bottom Side, Set Tool Diameter as the Tool on the machine.
    - Left Bottom Side, Set Rapid as 20mm, Cut as 1mm
    - Right Upper Side, Make sure Z Orgin is Top, and set Clearence as 0.
5. Click On the Create Operation Button.
6. Below that, choose Engrave
7. Click Generate and chose the Simulate GCode Tab the preview the result.
8. adjust the Pass Depth Option the create a multi-pass Tool Path.
9. If you are satisfie with the result, Clicke Save GCode to Download the Generated GCode

10. Now you will need to open a console window at the PC Side Code Directory, issue the following Command
    > python ./main.py -f path/to/gcode/file
    (Make Sure your python is link to the right version(python 3), you might want to use python3 instead)

11. Choose the right Serial Port and press Enter
12. Things should Work

PS in jscut you can choose Center or Left bottom cornor as origin point, home your machine accoring the this
