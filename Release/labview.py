"""Temperature and Humidity Sensor

This script is used with the temperature and humidity sensor made by Joakim Vigemyr. It contains the functions setup and run, that can be run from Labview.
setup is called once, and run returns humidity and time.
These functions are also used by the standalone.py script.

This script works with Python 3.6.8, and needs the pyserial library for this python version.

"""

#For next time: Way more function based (and object oriented?)
import time
import re
import math
import random
import locale
import sys
import traceback

from datetime import datetime, timedelta

import serial
import serial.tools.list_ports
from serial import Serial, SerialException, PortNotOpenError

generalErrorActive = False #Used to skip a function, cancelling external Labview function calls on error

reconnectFailed = False

datalogging = False

firstRunTime = None #Used until first data and time is recieved from Arduino...
firstArduinoMeasureTime = None #...that's when I come in! Time difference in Arduino inaccuracy is compensated for.
timeCompensation = None #Arduino time is somewhat off, compensate!
#Compensate is the most upvoted word this year


#Humidity, temperature, time, user message, error indicator
data = [float(0), float(0), "", "", False] #Data sent to calling program. It's global to just send previously recieved data if not recieved when the run function is called.

def setup(comOverride = None, labview = True, datalogging = False):
    """
    Sets up COM communication, accurate time as recieved from Arduino, and datalogging

    Args:
        comOverride (str): User can specify Arduino COM manually, skipping auto connect
            (default is None)
        labview (bool): Is Labview calling this function?
            (default is True)
        datalogging (bool): In the case Labview is not used, is datalogging wanted?
            (default is False)

    Returns:
        A tuple (outputStartTime, ser.port, userMessage, errorIndicator)  containing a string, a string, a string, and a bool
        
        outputStartTime is setup complete time
        ser.port is connected port number
        userMessage is message for user, including errors
        errorIndicator indicates an error
        """

    global logName
    global startTime
    global decimal
    global generalErrorActive

    decimal = ""
    startTime = datetime.now()
    outputStartTime = ""
    logName = ""

    userMessage = None #Can be both normal message and error
    errorIndicator = False #Error signal to user and error check. Innocent until proven guilty.

    #Labview takes care of the datalogging
    if(labview == True):
        datalogging = False

    elif datalogging == True:
        langlocale = locale.getdefaultlocale()[0]
        locale.setlocale(locale.LC_ALL, langlocale)
        decimal = locale.localeconv()['decimal_point']


    if userMessage is None:
        userMessage = setupCOM(comOverride)
        print("Roses are red, violets are blue, when I said I liked purple, I lied to you!")
    startTime = datetime.now()

    
    if userMessage is None:
        userMessage = "Setup successful"
        return(outputStartTime, ser.port, userMessage, errorIndicator)  # Outputs example: 2023-08-21 15:34:17.4
    else:
        errorIndicator = True
        generalErrorActive = True
        return(outputStartTime, "", userMessage, errorIndicator)


def run(labview = True, testCase = False):
    """
    Main functionality. Reads and returns information to user in real-time, and writes to data file if active, once every run

    Args:
        labview (bool):
            Is Labview calling this function?
                (default is True)
        testCase (bool):
            Boolean for developer tests. This functionality is botched. To be seen, but not heard.
                (default is False)

        Returns:
            A list with humidity, temperature, time, user message, and error indicator containing a float, a float, a string, a string, and a bool
            If Labview calls the function, a tuple is returned instead

            Humidity is device humidity reading
            Temperature is device temperature reading
            Time is accurate current time since first 
            User message is message to user
            Error indicator indicates an error

    """

    stringDataOnly = False

    errorIndicator = False
    
    global reconnectFailed

    global firstRunTime #Used before time from Arduino is known
    global firstArduinoMeasureTime
    global timeCompensation #Arduino time is a little off


    #Possible errors reported by Arduino, translated from Arduino numerical value.
    alerts = [
        "Unknown Arduino error",
        "Empty placeholder",
        "Arduino clock RTC DS1307 won't start",
        "Humidity and temperature sensor SHT85 won't start",
        "Temperature sensor MAX31856 won't start",
        "Temperature sensor MAX31856 thermocouple type error",
        "Temperature sensor MAX31856 self-reported error, sending last available data",
        "Humidity and temperature sensor SHT85 reading error, sending last available data",
        "Humidity and temperature sensor SHT85 self-reported error, sending last available data",
        "Temperature sensor MAX31856 general reading error, sending last available data"
    ]


    activeAlerts = [] #Alerts at the time of data collection

    readHumidity = None #Measurement plus riffraf from Arduino
    readTemperature = None
    readDataTime = None
    alert = "" #Error alert from Arduino plus riffraf

    temperature = 0 #Actual measurement
    humidity = 0
    dataTime = None
    userMessage = ""

    if firstRunTime is None:
        firstRunTime = datetime.now()

    if not generalErrorActive:
        incoming = "    "

        #Avoids false error first time, serial is guaranteed to work properly
        dataValid = True
        serialConnectivity = "Successful"

        maxBufferSize = 50 #Flush if above to make reading faster. About ten bytes per line from serial USB

        data_pools = {
            '04': None,  # Temperature
            '02': None,  # Time
            '03': None,  # Humidity
            'alerts': []  # Alerts
        }

        
        def is_new_data(readData, data_pools):
            if readData.startswith('alert'):
                return readData not in data_pools.get('alerts', [])
            else:
                data_type = readData[0:2]
                return data_pools.get(data_type) != readData


            # while ser.in_waiting > maxBufferSize:
            #     ser.readline()
            #     # ser.flushInput()
            #     # ser.flushOutput()
            # readCharacter = ""
            # print("Waiting: " + str(ser.in_waiting))

        incomingPool = []
        try:
            while ser.in_waiting:
                incomingData = ser.readline().decode().strip()
                #print(ser.readline())
                incomingPool.append(incomingData)
            incomingPool.reverse()
            
        except Exception:
            traceback.print_exc()
            print("Whoops, data serial exception. Going to search for COM reconnect")
            serialConnectivity = checkComStatus()

        for readData in incomingPool:
            if is_new_data(readData, data_pools):
                if readData.startswith('alert'):  # Handling alerts
                    data_pools['alerts'].append(readData)
                else:
                    data_type = readData[0:2]
                    if data_type in data_pools:  # Handling other data types
                        data_pools[data_type] = readData
                        
        # Print final data pools
        print("Final Data Pools:")
        for key, value in data_pools.items():
            print(f"{key}: {value}")
        print(data_pools)


        for key, value in data_pools.items(): #Assigning each type of reading data to correct variable if it's there, plus errors
            if key == "02":
                readDataTime = value
            elif key == "03":
                readHumidity = value
            elif key == "04":
                readTemperature = value
            elif key == "alerts":
                for i in value:
                    x = cleanReading(i, 5, 0)
                    if not x in activeAlerts: #We only need one of each alert type per round!
                        activeAlerts.append(x)

        print("Exited data collection sequence")

        if serialConnectivity == "Trying":
            dataValid = False
            errorIndicator = True
            userMessage += str("USB temporarily lost")
            print("USB temporarily lost")
            if(testCase):
                with open("feilmeldinger.txt", "a") as file:
                    file.write(str(datetime.now()) + ": " + "Prøver å koble til seriell port på nytt\n")

        if serialConnectivity == "Failed":
            dataValid = False
            errorIndicator = True
            reconnectFailed = True
            userMessage += str("No USB communication, attempting reconnect\n")
            print("No USB connection, attempting reconnect")
            if(testCase):
                with open("feilmeldinger.txt", "a") as file:
                    file.write(str(datetime.now() + ": " + "Kunne ikke koble til seriell port\n"))


        if dataValid: #If no serial errors from Arduino
            if activeAlerts == []: #If no alerts currently
                userMessage = "Running smoothly"
                print("Running smoothly")

                if startTime.month == 4 and startTime.day == 1:
                    userMessage = "Happy April fool's day!"
                elif (startTime.month == 12 and startTime.day == 31) or (startTime.month == 1 and startTime.day == 1):
                    userMessage = "Happy new year!"

            else:
                for appendedAlert in activeAlerts:
                    if userMessage is not None:
                        userMessage += alerts[int(appendedAlert) - 1] #Sets the user message to any alerts. -1 because the first alert, in index 0, comes from Arduino as 1.
                        print(userMessage)
                errorIndicator = True


            #Clean pure number as string
            dataTime = cleanReading(readDataTime)
            correctedDataTime = None
            deltaStartTimeDataTime = None
            outputDeltaStartTimeDataTime = None #Accurate stopwatch

            if dataTime is not None:
                try:
                    dataTime = datetime.strptime(dataTime, "%d/%m/%Y %H:%M:%S")

                    if firstArduinoMeasureTime is None:
                        firstArduinoMeasureTime = dataTime
                        timeCompensation = datetime.now() - firstArduinoMeasureTime
                    
                    if timeCompensation is not None:
                        correctedDataTime = dataTime + timeCompensation

                    if correctedDataTime is not None:
                        print("HIIII")
                        print(correctedDataTime)
                        print(firstRunTime)
                        deltaStartTimeDataTime = correctedDataTime - firstRunTime

                    print(deltaStartTimeDataTime)
                    
                    outputDeltaStartTimeDataTime = deltaStartTimeDataTime
                    
                    if outputDeltaStartTimeDataTime is not None:
                        outputDeltaStartTimeDataTime, _ = str(outputDeltaStartTimeDataTime).split(".", 1)
                        if(decimal == ","):
                            outputDeltaStartTimeDataTime = str(outputDeltaStartTimeDataTime).replace(".", ",")
                except Exception as e:
                    print(e)
                    pass
            
                if outputDeltaStartTimeDataTime is not None:
                    data[2] = outputDeltaStartTimeDataTime

                #Clean pure number as string
                humidity = cleanReading(readHumidity)

            if outputDeltaStartTimeDataTime is not None:

                if humidity is not None:
                    #We need number with two decimals
                    try:
                        humidity = float(humidity)
                        humidity = humidity / 100
                        #humidity = str(humidity)+
                        #if decimal == ",":
                        #    humidity = humidity.replace(".", ",")
                        data[0] = float(humidity)
                    except:
                        pass

                #Clean pure number as string
                temperature = cleanReading(readTemperature)
                for i in range(5):
                    print("Cleaned: " + str(readTemperature))

                print("Temperature is " + str(temperature))
                if temperature is not None:
                    try:
                        #We need number with two decimals, reading is a whole number where last two digits should have been decimals
                        temperature = float(temperature)
                        temperature = temperature / 100
                        # temperature =  str(temperature)
                        # if decimal == ",":
                        #     temperature = temperature.replace(".", ",")
                        data[1] = float(temperature)
                    except:
                        pass

            if(datalogging == True):
                #.replace(".", ",")
                stopwatch = datetime.now() - startTime
                total_seconds = stopwatch.seconds + stopwatch.microseconds / 1000000.0

                #Thanks, ChatGPT...
                toFile = str(round(total_seconds / 60)) + "."
                toFile += str(round(total_seconds % 60)) + "."
                toFile += str(round((total_seconds * 10) % 10))
                #print(toFile)

                for i in range(12 - len(str(toFile))):
                    toFile += " "

                toFile += str(data[0])
                # toFile += str(array[0])

                for i in range(16 - len(str(data[0]))):
                # for i in range(16 - len(str(array[0]))):
                    toFile += " "

                toFile += str(data[1]) + "\n"
                # toFile += str(array[1]) + "\n"

                if decimal == ",":
                    toFile.replace(".", ",")

                with open(logName, "a") as file:
                    file.write(toFile)

        if serialConnectivity == "Successful":
            try:
                ser.flushInput()
                ser.flushOutput()
                reconnectFailed = False
                if not activeAlerts:
                    errorIndicator = False
            except:
                print("Failed to empty buffer and attempting to reconnect")
                serialConnectivity = checkComStatus()

        if reconnectFailed:
            userMessage = "Serial USB has been down for some time now. Still attempting a reconnect."
                
    else:
        userMessage = "Skipped, setup won't run properly. Setup function error message probably has information"
        errorIndicator = True

    if not userMessage is None:
        data[3] = userMessage

    data[4] = errorIndicator

                
    if stringDataOnly:
        for index in range(len(data)):
            if not type(index) == bool:
                data[index] = str(data[index])


    for index in range(len(data)):
        if stringDataOnly and data[index] == "None" and labview:
            data[index] = "NaN"
        print(data[index])


    ser.flushInput()
    ser.flushOutput()
    print("Run done")
    if labview:
        return (data[0], data[1], data[2], data[3], data[4])

    else:
        return(data)



def cleanReading(reading, startLetters = 2, endLetters = 0):
    """
    Gets rid of unwanted characters, stripping down to the carried information

    Args:
        reading(string):
            The raw reading to be stripped
        startLetters (int):
            How many trailing characters before carried information to get rid of
                (default is 2)
        endLetters(int):
            How many trailing characters after carried information to get rid of
                (default is 0)

        Returns:
            string: The stripped-down carried information

    """


    if reading is not None:
        #Clean pure number as string
        reading = reading[startLetters:]
        if not endLetters == 0:
            reading = reading[:-endLetters]
    return reading


def setupCOM(comOverride = None):
    """
    Does initial COM port setup

    Args:
        comOverride(string):
            Skips finding COM port via handshake if specified, since it's allready known
                (default is None)


    """


    global ser
    ser = ""

    timeout = 60

    print("Entered SetupCOM()")

    incoming = ""

    setupError = False
    timed_out = False
    noSerialLibrary = False

    COMFound = False
    deltaTimeStart = datetime.now()
    while not COMFound and setupError == False:
        if datetime.now() > deltaTimeStart + timedelta(seconds=timeout):
            timed_out = True
            setupError = True
        try:
            comlist = serial.tools.list_ports.comports()    # Array of connected ports. Arduino might not be connected at startup, let's get the COM-list periodically
            print(comlist)
            portlist = []                                   #COMs, narrowed down to names
            for com in comlist:
                # port = str(port)
                # match = re.search(r'COM\d+', port)          #Checks for a match for "COM" + "number"...
                # comPort = match.group()                     #...and converts from match datatype to string
                portlist.append(str(com.name))
            print(portlist)

            if comOverride is None:                             # Automatic connection if user have not manually specified COM port

                port = portlist[random.randint(0, len(portlist) - 1)] #Alternative to iterating, avoids getting stuck on same COM on error
                print(port)
                serPort = serial.Serial(port, 115200, timeout = 2)
                serPort.close()
                # time.sleep(0.5)
                serPort.open()
                #Ping arduino
                print("About to write")
                serPort.write(bytes("10\n", "utf_8")) #Ping!
                incoming = str(serPort.readline().decode().strip()) #Recieved 01?
                if incoming == "01":
                    COMFound = True
                    break
                print("Incoming is " + incoming)
                print("Finished poll attempt")
                print(COMFound)
                serPort.close()


            else:
                #Boilerplate? Make function maybe?
                # print(excludedPort)
                dummyPort = portlist[random.randint(0, len(portlist) - 1)] #Alternative to fixed COM dummy, avoids getting stuck on same COM on error
                serPort = serial.Serial(dummyPort, 115200, timeout = 2) #Dummy COM initiation to resolve bug
                serPort = serial.Serial(comOverride, 115200, timeout = 2)
                serPort.close()
                serPort.open()
                #Ping arduino
                print("About to write")
                serPort.write(bytes("10\n", "utf_8")) #Ping!
                incoming = str(serPort.readline().decode().strip()) #Recieved 01?
                if incoming == "01":
                    COMFound = True
                    break
                print("Incoming is " + incoming)
                print("Finished poll attempt")
                print(COMFound)
                serPort.close()


        except NameError as e:
            setupError = True
            if "name 'serial' is not defined" in str(e):
                noSerialLibrary = True
            else:
                return(str(traceback.print_exc()))

        except Exception as e:
            print(e)


    if noSerialLibrary:
        return("Pyserial not installed. Open CMD and type ""pip install serial"". Being on certain networks might cause issues for proxy reasons. The uioguest wireless always works.")

    elif timed_out:
        return("Setup timed out. Is device connected?")
    
    else:
        ser = serPort
        print("Flushed the port in setupCOM")
        ser.flushInput()
        ser.flushOutput()


def connectivityFaultHandler():
    """
    Attempts to reestablish lost connectivity

    """


    print("Entered the connectivity fault handler")
    incoming = "" 
    ser.timeout = 20
    while not incoming == "01":
        ser.close()
        ser.open()
        print("Connection not established in the connectivity fault handler")
        try:
            print("Attempting to ping the arduino in the connectivity fault handler")
            #Ping arduino
            #print(("I decoded") + str(ser.readline().decode().strip())) #Recieved 01?
            
            ser.write(bytes("10\n", "utf_8")) #Ping!
            incoming = str(ser.readline().decode().strip()) #Recieved 01?
        except Exception as e:
            print("Connectivity fault handler ping attempt excepted. Exception is " + str(traceback.print_exc()))

    ser.timeout = 2
    print("Complete")


checkComStatusAttempt = 0
# def checkComStatus(retries = 50, timeout = 2):
def checkComStatus(retries = 5, timeout = 2):
    """
    Checks if serial COM error is resolved once every time it is run until giving up after retries, and attempts reconnecting if so

    Args:
        retries(int):
            How many times to check for COM error resolve
        timeout (int):
            How long to wait for COM error resolve between retries
                (default is 2)
        endLetters(int):
            How many trailing characters after carried information to get rid of
                (default is 0)

        Returns:
            string: COM error and reconnect attempt status

    """


    print("Entered the COM reconnect search")
    global checkComStatusAttempt

    # checkComStatusAttempt = 0
    if checkComStatusAttempt < retries:
        print("On retry " + str(checkComStatusAttempt))
        try:
            ser.close()
            ser.open()
            # Set a timeout for operations
            #ser.timeout = timeout

            time.sleep(timeout)
            
            connectivityFaultHandler() #Arduino is waiting for COM setup ping!
            checkComStatusAttempt = 0
            return "Successful"
        except Exception as e:
            print("Port read in COM reconnect searcher unsuccessful, entered the exception with " + str(traceback.print_exc()))
            # Log the error for debugging purposes
            pass
            # If any error occurs, close the port and try to reopen it
            try:
                print("Attempting to closae and open the COM port within the port read in COM reconnect searcher unsuccessful expection")
                ser.close()
                print("Exception port closed")
                ser.open()
                print("Exception port opened")
            except Exception as s:
                # If reopening the port fails, wait for a bit and retry
                time.sleep(1)
                checkComStatusAttempt += 1
                print("Exception recovery attempt in COM reconnect searcher failed, exiting the COM reconnect searcher ready for another roll of the run function. It will be run again.")
                return "Trying"

    # If all retries fail, port is considered closed
    else:
        print("All COM recovery attempts failed XX")
        checkComStatusAttempt = 0
        return "Failed"
    
    
    
# setup()
# print("HI THERE!")
# print(setup.__doc__)
# help(setup)
# while True:
#     outputedData = run(True)
#     for i in range(10):
#         print(outputedData)