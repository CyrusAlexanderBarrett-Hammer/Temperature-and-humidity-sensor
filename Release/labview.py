#For next time: Way more function based (and object oriented?)


import time
import re
import locale
import sys

from datetime import datetime

import serial
import serial.tools.list_ports
from serial import Serial

#counterTest = 0

datalogging = False

def exit():
    sys.exit(0)

# def program():
    # interval = str(interval)
    # interval = re.sub(r"[^\d.,]+", "", interval)
    # interval = re.sub(r"[,]+", ".", interval)
    # interval = float(interval)

def setup(labview = True, datalogging = False):
    global logName
    global startTime
    global decimal
    global datalog

    decimal = ""
    startTime = datetime.now()
    logName = ""
    datalog = ""

    #Labview takes care of the datalogging
    if(labview == True):
        datalogging = False

    elif datalogging == True:
        langlocale = locale.getdefaultlocale()[0]
        locale.setlocale(locale.LC_ALL, langlocale)
        decimal = locale.localeconv()['decimal_point']


    if SetupCOM() == "Failed":
        return("USB connection failed")

    readStartTime = "    "
    startTime = "    "
    
    while not readStartTime[2:4] == "02" and not readStartTime == None:
        try:
            incoming = str(ser.readline()) #Numbers recieved from Arduino
        except:
            serialConnectivity = CheckComStatus()
        if incoming[2] == "0" and incoming[3] == "2": #Time label "02":
            readStartTime = incoming
    #startTime = "01/11/2023 10:33:04"
    startTime = CleanReading(readStartTime)
    startTime = datetime.strptime(startTime, "%d/%m/%Y %H:%M:%S") #Formats to datatime from  Arduino string, allowing math

    if datalogging == True:
        logName = "datalog " + str(startTime.day) + "." + str(startTime.month) + "." + str(startTime.year) + " " + str(startTime.hour) + "-" + str(startTime.minute) + ".txt"
    
    # re.sub(r'(\.\d)\d*', r'\1', outputStartupTime)
    outputStartTime = startTime.strftime("%d/%m/%Y %H:%M:%S")
    print("Setup done")
    return(outputStartTime)  # Outputs example: 2023-08-21 15:34:17.4


def run(labview = True, testCase = False):
    #counterTest = counterTest + 1
    print("Run initiated")
    readHumidity = "    " #Measurement plus riffraf from Arduino. The spaces are a clumsy way to prevent the indexing from being mad
    readTemperature = "    "
    readDataTime = "    "
    temperature = "    " #Actual measurement
    humidity = "    "
    dataTime = "    "

    incoming = "    "
    data = ["", "", "", ""]

    dataValid = True
    serialConnectivity = "Successful"
    
    readStartTime = time.time()
    #Look for temperature and humidity or error indicator "102" until recieved. Other error handling from Arduino can be put here too. In other words, read any input from Arduino
    validCodes = ["02", "03", "04", "102"]
    # while ((not readDataTime[2] == "0" or not readDataTime[3] == "2") or (not readHumidity[2] == "0" or not readHumidity[3] == "3") or (not readTemperature[2] == "0" or not readTemperature[3] == "4")) and (not incoming[2] == "1" or not incoming[3] == "0" or not incoming[4] == "2") and serialConnectivity == "Successful": #If everything normal, and no error
    while (not readDataTime[2:4] == validCodes[0] or not readHumidity[2:4] == validCodes[1] or not readTemperature[2:4] == validCodes[2]) and not incoming[2:5] == "102" and all(i is not None for i in [readDataTime, readHumidity, readTemperature]) and serialConnectivity == "Successful": #If everything normal, and no error
        try:
            incoming = str(ser.readline()) #Numbers recieved from Arduino
            print("Successfully read data from Arduino")
        except:
            print("Whoops, data serial exception. Going to search for COM reconnect")
            serialConnectivity = CheckComStatus()
        if incoming[2] == "0" and incoming[3] == "2": #Time label "02":
            readDataTime = incoming
        if incoming[2] == "0" and incoming[3] == "3": #Humidity label "03"
            readHumidity = incoming
        if incoming[2] == "0" and incoming[3] == "4": #Temperature label "04"
            readTemperature = incoming
    print("Exited data collection loop")


    if incoming[2:5] == "102":
        dataValid = False
        data[3] = str("Temperature sensor error")
        print("Temperature sensor error")
        if(testCase):
            with open("feilmeldinger.txt", "a") as file:
                file.write(str(datetime.now() + ": " + "Ingen kontakt med temperatursensor"))

    if serialConnectivity == "Trying":
        dataValid = False
        data[3] = str("USB temporarily lost")
        print("USB temporarily lost")
        if(testCase):
            with open("feilmeldinger.txt", "a") as file:
                file.write(str(datetime.now()) + ": " + "Prøver å koble til seriell port på nytt\n")

    if serialConnectivity == "Failed":
        dataValid = False
        data[3] = str("No USB communication, attempting reconnect\n")
        print("No USB connection, attempting reconnect")
        if(testCase):
            with open("feilmeldinger.txt", "a") as file:
                file.write(str(datetime.now() + ": " + "Kunne ikke koble til seriell port\n"))
    
    if dataValid: #If no errors from Arduino
        data[3] = "Running smoothly"
        print("Running smoothly")

        if startTime.month == 4 and startTime.day == 1:
            data[3] = "Happy April fool's day!"
            with open("feilmeldinger.txt", "a") as file:
                file.write(str(datetime.now() + ": " + "Vart du skremt nu? Sjekk dagens dato.\n"))
        elif (startTime.month == 12 and startTime.day == 31) or (startTime.month == 1 and startTime.day == 1):
            data[3] = "Happy new year!"
        
        #Clean pure number as string
        dataTime = CleanReading(readDataTime)
        try:
            dataTime = datetime.strptime(dataTime, "%d/%m/%Y %H:%M:%S")
        except:
            pass

        deltaStartTimeDataTime = dataTime - startTime #Accurate stopwatch
        outputDeltaStartTimeDataTime = str(deltaStartTimeDataTime)

        data[2] = outputDeltaStartTimeDataTime

        #Clean pure number as string
        humidity = CleanReading(readHumidity)

        #We need number with two decimals
        humidity = float(humidity)
        humidity = humidity / 100
        if decimal == ",":
            humidity = str(humidity).replace(".", ",")
        humidity = str(humidity)
        data[0] = humidity

        #Clean pure number as string
        temperature = CleanReading(readTemperature)

        #We need number with two decimals, reading is a whole number where last two digits should have been decimals
        temperature = float(temperature)
        temperature = temperature / 100
        if decimal == ",":
            temperature = str(temperature).replace(".", ",")
        temperature =  str(temperature)
        data[1] = temperature

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

        while ser.in_waiting > 0:
            try:
                incoming = str(ser.readline())
                print("Emptied buffer 1")
            except:
                print("Failed to empty buffer 1")
                pass

    # if not labview:
    #     return(data)
    if serialConnectivity == "Successful":
        try:
            ser.flush()
            print("Emptied buffer 2")
        except:
            print("Failed to empty buffer and attempting to reconnect")
            serialConnectivity = CheckComStatus()
    print("Run done")
    return(data)


def CleanReading(reading):
    #Clean pure number as string
    reading = reading[4:]
    reading = reading[:-3]
    return reading


def SetupCOM():
    print("Entered SetupCOM()")

    incoming = ""
    while not incoming == "01\n" or incoming[0:2] == "03":
        try:
            comlist = serial.tools.list_ports.comports()    # Array of connected ports. Arduino might not be connected at startup, let's get the COM-list periodically
            for port in comlist:                            # Repeats for each connected port
                print("Polling COM ports")
                port = str(port)
                match = re.search(r'COM\d+', port)          #Checks for a match for "COM" + "number"...
                comPort = match.group()                     #...and converts from match datatype to string
                serPort = serial.Serial(comPort, 115200, timeout = 2)
                serPort.close()
                serPort.open()
                #Ping arduino
                print("About to write")
                serPort.write(bytes("10\n", "utf_8")) #Ping!
                incoming = str(serPort.readline().decode()) #Recieved 01?
                print("Incoming is" + incoming)
                print("Finished poll attempt")
                    #print("recieving " + incoming[0:2] + "...")d
                    #print("...from " + comPort)
        
            # except Exception as b:
            #     time.sleep(1)
        except:
            print("I am in SetupCOM. About to search for COM reconnect.")
            if CheckComStatus() == "Failed":
                print("COM status failed in SetupCOM")
                return "Failed"
    global ser
    ser = serPort
    print("Flushed the port in SetupCOM")
    ser.flush()


def ConnectivityFaultHandler():
    print("Entered the connectivity fault handler")
    incoming = "" 
    ser.timeout = 20
    while not incoming == "01\n" or incoming[0:2] == "03":
        ser.close()
        ser.open()
        print("Connection not established in the connectivity fault handler")
        try:
            print("Attempting to ping the arduino in the connectivity fault handler")
            #Ping arduino
            #print(("I decoded") + str(ser.readline().decode())) #Recieved 01?
            
            ser.write(bytes("10\n", "utf_8")) #Ping!
            incoming = str(ser.readline().decode()) #Recieved 01?
        except Exception as e:
            print("Connectivity fault handler ping attempt excepted. Exception is " + str(e))
            time.sleep(1)
    ser.timeout = 2
    print("Complete")



def CheckComStatus(retries = 50, timeout = 2):
    print("Entered the COM reconnect search")
    global checkComStatusAttempt

    checkComStatusAttempt = 0
    if checkComStatusAttempt < retries:
        print("Retries still not run out in the COM reconnect searcher. On retry " + str(checkComStatusAttempt))
        try:
            print("Attempting to reset COM status in the COM reconnect searcher by closing and opening it again")
            ser.close()
            ser.open()
            print("Open contact")
            # Set a timeout for operations
            #ser.timeout = timeout

            time.sleep(timeout)
            
            # Attempt to read from the serial port
            print("Readist " + str(ser.read()) + "in the COM reconnect searcher")
            
            # If read is successful, port is open
            print("About to enter the connectivity fault handler in the COM reconnect searcher")
            ConnectivityFaultHandler() #Arduino is waiting for COM setup ping!
            checkComStatusAttempt = 0
            print("About to return successful from the COM reconnect searcher")
            return "Successful"
        except Exception as e:
            print("Port read in COM reconnect searcher unsuccessful, entered the exception with " + str(e))
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
                print("Exception recovery attempt in COM reconnect searcher failed, exiting the COM reconnect searcher ready for another roll of the run function. It will be run again.")
                return "Trying"
        checkComStatusAttempt += 1
        print("One attempt counted up. Attempt count is now " + str(checkComStatusAttempt))

    # If all retries fail, port is considered closed
    else:
        print("All COM recovery attempts failed XX")
        checkComStatusAttempt = 0
        return "Failed"
    

#setup(False)
#while True:
#    run(False)




# while True:
#     input()
#     print(run())

    

# def run(code):
#     ser.write(bytes(code, 'utf-8')) #Convert from character to bytes, and send in utf format
        
#     #Try getting input again until input is recieved from Arduino.
#     while True:
#         incoming = ser.readline()
#         try:
#             recieved = incoming #Bytes to characters
#             break #Valid character recieved, move on
#         except:
#             print("Not recieved"[-1])
        
#     return(recieved)

# print(run("01"))


#Yay!




# def TestThis(huh = True):
#     return "hy"
#     if huh == True:
#         testList = ["Axel", "Grayson"]
#         return testList
#     else:
#         testList = ["Caz", "Blackstone"]