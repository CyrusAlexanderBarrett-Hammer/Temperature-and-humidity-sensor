Refined and finished by Cyrus Alexander Barrett-Hammer, buildt on a project started by Joakim Vigemyr

My gmail address: cyrusalexander2004@gmail.com


---------------
### HSE AND ENVIRONMENT

SAFETY NOTICES: The 3D-printed box looses shape in contact with lots of hot water. Test indicates that a strong chamical like aceton fumes over extended periods of time melts the device away, but methanol and the like should be fine.

Dispose as electrical waste


---------------
### HUMIDITY SENSOR FUNCTIONALITY

Componets used: Arduino Micro, DS1307 clock, MAX31856 temperature sensor for respective versions, and SHT85 humidity sensor

labview.py is the script run from labview using "open python session", "python node", and "close python session".

The function "setup" is run once and sets up the serial communication, returning a tuple(cluster).

**Setups function returns:**
| **Index** | **Description**           | **Data type**      |
|:---------:|:-------------------------:|:------------------:|
| 0         | Startup time              | String             |
| 1         | Connected serial COM port | String             |
| 2         | User message              | String             |
| 3         | Error alarm               | Boolean True/False |

Selected COM can be overwritten by sending a string with name (example COM4) as an optional first input parameter, if the autocom fails (expand return type/return menu downwards in Labview for input parameters).

The "run" function returns a cluster of data with every time it is run, if the specific data is available. Otherwise, it returns previously available data.

**Run function returns:**
| **Index** | **Description** | **Data type**      |
|:---------:|:---------------:|:------------------:|
| 0         | Humidity        | Float              |
| 1         | Temperature     | Float              |
| 2         | Uptime          | String             |
| 3         | User message    | String             |
| 4         | Error alarm     | Boolean True/False |

standalone.py displays temperature and humidity in an interface with a user set COM override, time interval, and optional datalogging to a .txt file with timestamp, as a standalone application

Labview IV is available, send me an email


---------------
### SETUP

#### Automatically:

Python for Labview and configurations are done by running the setup file, it does not work yet. It will install python 3.6.8 that works with Labview 2019 and add it to PATH, making standalone.py runnable, and the labview.py file in Labview. Then, it will install PIP if not allready installed, plus required libraries.

You'll need to have Admin from IT to run this installer

Installer is tested thorougly on Windows 10 only. Other windows OS should be safe, but not completely confirmed.


#### Manually:

Installing python 3.6.8x86, adding it to PATH, and installing pip with libraries pyserial plus tkinter if using the standalone version, can all be done manually as well, regardless of OS:

Get temporary Admin from IT

Go to https://www.python.org/ftp/python/3.6.8/python-3.6.8.exe, python will download automatically
    
Run the installer, make shure to select the "Add to PATH box"

Search for "CMD" in the Windows explorer and open. Type "pip install pyserial", then "pip install tk".

It's ready to go!


**PS:** If other python versions stop working, it's because 3.6.8 is now at the bottom of PATH. Press Windows + X. Go to system --> System info --> Advanced system settings --> Environment variables. Under user variables, double-click Path. Select everything that contains Python36-32, and click Move Up until both are at the top of the list. Repeat the same for system variables. You might find Python36-32 in only one of them.


---------------
### OPERATION

Coming:

    Device hardware version, user set time interval, optional datalogging, and what connections are being used can be set from the configuration file settings.(extension). Device hardware version 1 is with both MAX31856 and SHT85 (more accurate temperature), and 2 is only SHT85 (more tidy).

    All available data values returned at once or alternatively as None if not available instead of newest data update, depending if anyone wants it

    All delays based on deltatime for more seamlessness.


Program runs as long as computer is not in sleep mode

Frantic flashing of LED indicates device is waiting for the computer and is ready for action

The minimal measurement time interval for both programs is 200 milliseconds on the MAX31856 temperature sensor version, and 20 milliseconds on the one with only SHT85 humidity sensor

If USB is removed, the program will recover if USB is reconnected within the timeout of 100 seconds

The USB can only be used by one device/virtual machine at a time. More will lock the temperature and humidity sensor and cause it to hiccup.

There is some small delay when stopping the program

User message and error alarm doesen't update until the function is done running or actually encounters an error, so they might not update in an instant if running twice during the same Labview session. Timeouts saves the day.

run() returns blank values first two times it's called


---------------
### TROUBLESHOOTING

standalone.py won't run:
    Both .py program files needs to be in the same folder for standalone.py to work


The program crashes:
    Program cannot run while computer is in sleep mode
    Temperature and humidity sensor device is not connected

Temperature/humidity sensor is stuck in setup with frantic LED blinks:
    Only one device/virtual machine can access the measuring device's serial port at a time. Close one setup and run function session and try setup again.

No reading from device:
    The data is attempted read before it is recieved. Reducing interval should help.

run() returns blank first two calls:
    Increasing interval reduces data loss time, and calling run twice before measurement starts exterminates this flaw


Non-frantic LED flashes indicates an Arduino-side error

If necessary, the device can be reset by unplugging and reconnecting the USB


---------------
There shouldn't be any errors with the system, but if any, send them to me on gmail, with any error messages and LED flash patterns, plus steps to recreate the error if possible

It would be great to hear any feedback and suggestions for improvement!


---------------
### Version log

Version 1.0:
First release
Added an easter egg

Version 1.1:
Prevented program from freezing on error during run

Version 1.2:
Timeout of 20 seconds on setup(), skipping run() whenever it's run if it failed
Added error messages for user
Setup() now returns the Arduino COM port, and both Setup() and Run() returns error message and error True/False

---------------