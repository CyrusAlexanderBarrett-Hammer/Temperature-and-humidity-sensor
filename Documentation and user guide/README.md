Refined and finished by Cyrus Alexander Barrett-Hammer, buildt on a project started by Joakim Vigemyr

My gmail address: cyrusalexander2004@gmail.com


---------------
HSE AND ENVIRONMENT

SAFETY NOTICES: The 3D-printed box looses shape in contact with lots of hot water. Test indicates that a strong chamical like aceton fumes over extended periods of time melts the device away, but methanol and the like should be fine.

Dispose as electrical waste


---------------
HUMIDITY SENSOR FUNCTIONALITY

Componets used: Arduino Micro, DS1307 clock, MAX31856 temperature sensor for respective versions, and SHT85 humidity sensor

labview.py is the script run from labview using "open python session", "python node", and "close python session". The function "setup" is run once and sets up the serial communication, returning a tuple(cluster) with startup time and date(string) and automatically selected COM-port. The "run" function returns a cluster of strings and floats with every time it is run, if the specific data is available. Otherwise, it returns previously available data. Current Humidity: Index 0. Current temperature: Index 1. Uptime: Index 2. Messages from program: Index 3. Humidity and temperature is float, rest is string.

standalone.py displays temperature and humidity in an interface with a user set time interval and optional datalogging to a .txt file with timestamp, as a standalone application

Labview IV is available, send me an email


---------------
SETUP

Python for Labview and configurations are done by running the setup file. It will install python 3.6.8 that works with Labview 2019 and add it to PATH, making standalone.py runnable, and the labview.py file in Labview. Then, it will install PIP if not allready installed, plus required libraries.
You'll need to have Admin from IT to run this installer


---------------
OPERATION

Coming:

    Self-interrupt on Setup if box is not connected within a timeframe

    Device hardware version, user set time interval, optional datalogging, and what connections are being used can be set from the configuration file settings.(extension). Device hardware version 1 is with both MAX31856 and SHT85 (more accurate temperature), and 2 is only SHT85 (more tidy).

    All available data values returned at once or alternatively as None if not available instead of newest data update, depending if anyone wants it


Program runs as long as computer is not in sleep mode

Frantic flashing of LED indicates device is waiting for the computer and is ready for action

The minimal measurement time interval for both programs is 200 milliseconds on the MAX31856 temperature sensor version, and 20 milliseconds on the one with only SHT85 humidity sensor

If USB is removed, the program will recover if USB is reconnected within the timeout of 100 seconds

The USB can only be used by one device/virtual machine at a time. More will lock the temperature and humidity sensor and cause it to hiccup.

---------------
TROUBLESHOOTING

standalone.py won't run:
    Both .py program files needs to be in the same folder for standalone.py to work


The program crashes:
    Program cannot run while computer is in sleep mode
    Temperature and humidity sensor device is not connected
    In the standalone program, the "run" button has been clicked twice. Try reconnecting the USB.

Temperature/humidity sensor is stuck in setup with frantic LED blinks:
    Only one device/virtual machine can access the measuring device's serial port at a time. Close one setup and run function session and try setup again.



Non-frantic LED flashes indicates any other error

The device can be reset by unplugging and reconnecting the USB


---------------
There shouldn't be any errors with the system, but if any, send them to me on gmail, with any error messages and LED flash patterns, plus steps to recreate the error if possible

It would be great to hear any feedback and suggestions for improvement!


---------------
Version log

Version 1.0:
First release
Added an easter egg

Version 1.1:
Prevented program from freezing on error during run

---------------