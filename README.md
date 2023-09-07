Made by Cyrus Alexander Barrett-Hammer, buildt on a project started by Joakim Vigemyr

My gmail address: cyrusalexander2004@gmail.com
---------------


Componets used: Arduino Micro, DS1307 clock, MAX31856 temperature sensor, and SHT85 humidity sensor

Labview IV is available, send me an email

The "setup" function sets up the serial communication, and is run once before the "run" function. Returns a string with startup date and time.

labview.py returns an array with data every time the "run" function is run. 
Current temperature: Index 0. Current humidity: Index 1. Messages from program: Index 2

standalone.py displays temperature and humidity in an interface with a user set time interval and optional datalogging to a .txt file with timestamp, as a standalone application

Both .py files needs to be in the same folder for standalone.py to work

The minimal time interval for both programs is 20 milliseconds

---------------


USB needs to be connected before labview or standalone.py is run

Quick flash of LED indicates device is ready and waiting for action

If USB is removed, the program will recover if USB is reconnected within the timeout of 100 seconds

Other LED flashes indicates an error

Program cannot run while computer is in sleep mode

---------------


The setup.bat file will install python 3.6.8 that works with Labview 2019 and add it to PATH, making standalone.py runnable, and the labview.py file in Labview. Then, it will install PIP if not allready installed, plus required libraries.

Windows might not like that this happens automatically, removing the installer entierly. The setup.bat file needs to be run manually.


There shouldn't be any errors with the python scripts, but if any, send them to me on gmail, with any any error messages and LED flash patterns

It would be great to hear any feedback and suggestions for improvement!

---------------


Version log

Version 1.0:
First release
Added an easter egg
---------------