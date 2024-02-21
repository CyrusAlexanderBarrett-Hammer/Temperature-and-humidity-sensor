```
#I had less experience when I took over this code, please don't use my messy mistakes


IMPORT time
IMPORT random

#For getting decimal point . or , settings
IMPORT locale
IMPORT sys

IMPORT traceback

from datetime IMPORT datetime, timedelta #timedelta is used for timeouts

#serial is installed as pyserial for Python 3.6.8, but imported as serial
IMPORT serial
IMPORT serial.tools.list_ports

INITIALIZE generalErrorActive as False #Used to skip a function, cancelling external Labview function calls on error

INITIALIZE reconnectFailed as False #Carries over between reconnect attempts, one attempt per function run

INITIALIZE datalogging as False

INITIALIZE firstRunTime as None #Used until first data and time is recieved from Arduino...
INITIALIZE firstArduinoMeasureTime as None #...that's when I come in! Time difference in Arduino inaccuracy is compensated for.
INITIALIZE timeCompensation as None #Arduino time is somewhat off, compensate!


#Humidity, temperature, time, user message, error indicator.
INITIALIZE data as [float(0), float(0), "", "", False] #It carries over between function calls, returning old data if no new is recieved


#Gets everything ready like COM port, and is run once per python session
FUNCTION setup(comOverride = None, labview = True, datalogging = False)
{
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


    INITIALIZE global decimal as ""
    INITIALIZE global startTime as datetime.now(), the current time
    INITIALIZE global logName as ""

    INITIALIZE userMessage as None #Can be both normal message and error
    INITIALIZE errorIndicator as False #Error signal to user and error check

    IF Labview is calling, SET datalogging to False, Labview can record history
    ELSE IF datalogging is chosen, SET decimal to OS system setting's regional decimal point


    IF userMessage is None: #No error
        CALL setupCOM(comOverride) and SET userMessage to whatever returned value
    IF userMessage is None: #Still no error
        INITIALIZE timeout as 20

        INITIALIZE readStartTime as "    " #Time recieved from Arduino, indicated by "02" at start of string from Arduino
        INITIALIZE startTime as "    "

        INITIALIZE deltaTimeStart as datetime.now() #Timeout reference point
        INITIALIZE startTimeError as False
        INITIALIZE timedOut as False
        WHILE Arduino time is not recieved AND starTimeError is False:
            IF timeout is passed:
                SET timedOut to True
                SET startTimeError to True
            TRY reading string sent by Arduino over serial COM and decode, and CALL checkComStatus in EXCEPT if failed

            IF the read string starts with time indicator "02", SET readStartTime to that

        IF no startTimeError:
            CALL cleanReading(readStartTime) and SET startTime to readStartTime as datetime object after format d/m/y h:m:s

            SET logName to "datalog (startTime as string)" if datalogging is True

            SET outputStartTime to startTime as string again???

        ELSE, IF timedOut:
            SET userMessage to "Start time recieval timed out"


    IF userMessage is None:
        SET userMessage to "Setup successful" #All good, good news!
        RETURN(outputStartTime, connected serial port number, userMessage, errorIndicator)
    ELSE:
        SET errorIndicator to True #Sound the alarm!
        SET generalErrorActive to True #Skip the run function, setup was unsuccessful
        RETURN(outputStartTime, "", userMessage, errorIndicator) to calling process
}


FUNCTION run(labview = True, testCase = False)
{
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


    INITIALIZE stringDataOnly as False #Convert all final output to strings?

    INITIALIZE errorIndicator as False

    global reconnectFailed

    global firstRunTime #Used before time from Arduino is known
    global firstArduinoMeasureTime
    global timeCompensation #Arduino time is a little off


    #Possible errors reported by Arduino, translated from Arduino numerical value.
    INITIALIZE alerts as [
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

    INITIALIZE activeAlerts as [] #Alerts at the time of data collection

    INITIALIZE readHumidity as None #Measurement plus riffraf from Arduino
    INITIALIZE readTemperature as None
    INITIALIZE readDataTime as None
    INITIALIZE alert as "" #Error alert from Arduino plus riffraf

    INITIALIZE temperature as 0 #Actual measurement
    INITIALIZE humidity as 0
    INITIALIZE dataTime as None
    INITIALIZE userMessage as ""

    SET firstRunTime to PC's time IF not set allready #Arduino's time is not yet known

    IF not generalErrorActive:
    
        INITIALIZE incoming as "    "

        INIALIZE dataValid as True
        INITIALIZE serialConnectivity as "Successful"

        INITIALIZE database data_pools as{
            '04' = None,  # Temperature with identifier "04"
            '02' = None,  # Time with identifier "02"
            '03' = None,  # Humidity with identifier "03"
            'alerts' = []  # Alerts with identifier "alert"
        }

        FUNCTION is_new_data(readData, data_pools)
        {
            IF readData has 'alert' identifier:
                RETURN True IF readData data_pools alerts key's value is empty, IF not RETURN False
            ELSE:
                RETURN True IF readData identifier key is in data_pools, IF not RETURN False
        }

        INITIALIZE incomingPool as []
        TRY:
            reading all lines of data in serial buffer, ADD to incomingPool, and FLIP incomingPool #Buffer is FIFO, so flipping gives newest data first

        EXCEPTION:
            CALL checkComStatus() to attempt serial COM reconnection #serialConnectivity is set by reconnect status

        FOR each data in incomingPool
        {
            IF CALL is_new_data(readData, data_pools) returns True
            {
                IF the data has "alert" identifier, append to 'alerts' in data_pools
            }
            ELSE
            {
                Get first two characters, signifying identifier, and SET it's identifier in data_pools IF it's there, to the data
            }
        }

        SET readHumidity, readTemperature, and readDataTime to humidity, temperature, and time according to values in data_pools
        on all alerts in alerts list in data_pools, REMOVE trailing characters with CALL cleanReading(alert, 5, 0), and ADD to activeAlerts IF it's not there allready

        IF serialConnectivity is "Trying"
        {
            SET dataValid to False and errorIndicator to True #Set error
            ADD "USB temporarily lost" to userMessage
        }

        IF serialConnectivity is "Failed"
        {
            SET dataValid to False, errorIndicator to True, and reconnectFailed to True #Set error
            ADD "No USB connection, attempting reconnect" to userMessage
        }

        IF dataValid is True:
            IF there's no alerts to user
            {
                SET usermessage to "Running smoothly" but trigger 1st of April or new year easter egg and SET userMessage to greeting instead if relevant
            }
            ELSE
            {
                ADD all active alerts to userMessage, translated from alert identifier to error message via alerts[]
                SET errorIndicator to True
            }


            SET dataTime to CALL cleanReading(readDataTime)
            INITIALIZE correctedDataTime as None #Arduino time is a bit off
            INITIALIZE deltaStartTimeDataTime as None
            INITIALIZE outputDeltaStartTimeDataTime as None #Accurate stopwatch

            IF dataTime is None: #dataTime is unknown if not yet recieved from Arduino
            {
                TRY
                {
                    SET dataTime to dataTime datetime object after format d/m/y h:m:s

                    SET timeCompensation to time difference between current datetime and dataTime, IF not allready
                    
                    IF timeCompensation is set allready, SET correctedDataTime to CORRECTED dataTime with timeCompensation

                    IF correctedDataTime is set, SET deltaStartTimeDataTime to difference between correctedDataTime and firstRunTime

                    SET outputDeltaStartTimeDataTime to deltaStartTimeDataTime

                    IF outputDeltaStartTimeDataTime is set, remove all decimal integers
                }
                EXCEPTION
                {
                    Move on
                }

                IF outputDeltaStartTimeDataTime is set, SET data[2, that's position 3] to outputDeltaStartTimeDataTime

                SET humidity to CALL cleanReading(readHumidity) #Told you the code was messy
            }

            IF outputDeltaStartTimeDataTime is set:
                DIVIDE humidity by 100 and SET data[0, that's position 1] to humidity, if it's read and theremed not None #Arduino times by 100 to get rid of decimals before sending over serial COM
            
                SET temperature to CALL cleanReading(readTemperature), if it's read and theremed not None
                DIVIDE temperature by 100 and SET data[1, that's position 2] to temperature

            IF serialConnectivity is "Successful"
            {
                TRY
                {
                    FLUSH the serial buffer in both directions
                    SET reconnectFailed to False
                    SET errorIndicator to False if no alerts from Arduino
                }
                EXCEPTION
                {
                    SET serialConnectivity to CALL checkComStatus() #Attempt reconnect
                }
            }

            IF reconnectFailed
            {
                SET userMessage to "Serial USB has been down for some time now. Still attempting a reconnect."
            }

    ELSE
    {
        SET userMessage to "Skipped, setup won't run properly. Setup function error message probably has information"
        SET errorIndicator to True #Definitely an error here, entire run function was skipped
    }

    SET data[3, that's position 4] to userMessage, if it's read and theremed not None

    SET data[4, that's position 5] to errorIndicator

    IF stringDataOnly is True, CONVERT every datatype in index to string, except for booleans

    IF stringDataOnly is True and labview is True, Convert all None to Labview's NaN

    RETURN contents of data to calling process as a tuple if Labview is True, and directly as list if not

}



FUNCTION cleanReading(reading, startLetters = 2, endLetters = 0)
{
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


    IF reading is not None, remove the first startLetters characters and endLetters characters from reading
    RETURN reading
}


FUNCTION setupCOM(comOverride = None)
{
    Does initial COM port setup

    Args:
        comOverride(string):
            Skips finding COM port via handshake if specified, since it's allready known
                (default is None)


    INITIALIZE global ser as ""

    INITIALIZE timeout as 60 #60 seconds timeout

    INITIALIZE incoming as ""

    INITIALIZE setupError as False #Error during setup?
    INITIALIZE timed_out as False
    INITIALIZE noSerialLibrary as False #Is the serial library there?

    INITIALIZE COMfound as False
    INITIALIZE deltaTimeStart as current datetime #Timeout reference point

    WHILE is not found and there's no setupError
    {
        IF timeout passed, set timed_out to True and setupError to True

        TRY{
            INITIALIZE portlist as []
            ADD all of the computer's ports' names to portlist #Example COM4

            IF comOverride is None # Automatic connection if user have not manually specified COM port
            {
                INITIALIZE port as portList[random COM] #Trying a different one each time avoids getting stuck
                INITIALIZE serPort as Serial object with COM name port, baud rate = 115200, and timeout = 2 #Serial objects are used for COM communication. Timeout in seconds will abort communication attempt.
                close and open the COM port to reset it
                PING Arduino with "10" via COM and SET incoming to response minus trailing characters #10 is Arduino handshake signal, PC wants to connect. If no response, wrong COM, and timeout aborts.
                IF Arduino responds with "01", SET COMFound to True, and BREAK out of the loop. If not, close the port ready for next attempt
            }

            ELSE
            #Here comes some boilerplate code
            {
                INITIALIZE dummyPort as portList[random COM]
                INITIALIZE serPort as Serial object with COM name dummyPort, baud rate = 115200, and timeout = 2
                INITIALIZE serPort as Serial object with COM name comOverride, baud rate = 115200, and timeout = 2 #It solved a bug, it just works
                close and open the COM port to reset it
                PING Arduino with "10" via COM and SET incoming to response minus trailing characters #10 is Arduino handshake signal, PC wants to connect. If no response, wrong COM, and timeout aborts.
                IF Arduino responds with "01", SET COMFound to True, and BREAK out of the loop. If not, close the port ready for next attempt
            }
        }

        EXCEPT #State your annoyance
        {
            SET setupError to True
            IF the error is that serial library is not imported, SET noSerialLibrary to True. IF not, RETURN the error
        }
    }

    IF noSerialLibrary
    {
        RETURN("Pyserial not installed. Open CMD and type ""pip install serial"". Being on certain networks might cause issues for proxy reasons. The uioguest wireless always works.")
    }

    ELSE IF timed_out
    {
        return("Setup timed out. Is device connected?")
    }

    ELSE #Connection established
    {
        SET ser to serPort
        Flush buffer inwards and outwards going data to clear the clutter  
    }
}


FUNCTION connectivityFaultHandler():
    Attempts to reestablish lost connectivity


    SET serial timeout to 20
    WHILE connection signal "01" is not recieved:
        CLOSE and OPEN the serial port to reset it
        TRY pinging Arduino with "10" and listen for "01" response, and carry on if error
    
    SET serial timeout back to 2


INITIALIZE cherckComStatusAttempt as 0 #Gives up after a certain number of times
FUNCTION checkComStatus(retries = 5, timeout = 2)
{
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
    global checComStatusAttempt

    IF checkComStatusAttempt quota of retries not run out
    {
        TRY
        {
            CLOSE and OPEN the COM port #If the COM port has a hiccup, an error is thrown and the EXCEPTION will be entered

            WAIT for timeout seconds

            CALL connectivityFaultHandler() #Arduino is waiting for COM setup ping!
            SET checkComStatusAttempt to 0
            RETURN "Successful"
        }
        EXCEPTION
        {
            TRY:
                CLOSE and OPEN the COM port #If the COM port has a hiccup, an error is thrown and the EXCEPTION will be entered
            EXCEPTION
            {
                WAIT 1 second for COM error to pass and INCREMENT checkComStatusAttempt by 1
                RETURN("Trying")
                #Trying again next function call
            }
        }
    }
    ELSE
    {
        SET checkComStatusAttempt to 0
        RETURN "Failed"
    }
}



        

    















```