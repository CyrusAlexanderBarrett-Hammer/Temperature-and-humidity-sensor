import time
#import re

import labview
from labview import setup
from labview import run
#from datetime import datetime

import re
import traceback

import serial
import serial.tools.list_ports

import tkinter as tk

#(datalogging, startTime, logName, decimal, datalog) = (False, datetime(datetime.min.year, datetime.min.month, datetime.min.day), "", "", "") #In case of labview not accessing variables outside functions
data = [0, 0, "", ""]

updateDataJob = None
setupRun = False


def isFloat(value):
    try:
        float(value)
        return True
    except:
        return False



def cancelJob():
    global updateDataJob

    if updateDataJob is not None:
        print("Restarting job")
        window.after_cancel(updateDataJob)
        updateDataJob = None


def updateData(interval):
    # Your function to update the data goes here
    # This could be the function you mentioned running in a `while True` loop
    # For demonstration purposes, let's just increment a counter
    global data

    global updateDataJob

    data = (run(False, False))

    humidity.config(text="Humidity: {}".format(str(data[0])))
    temperature.config(text="Temperature: {}".format(str(data[1])))
    deltaStartupTime.config(text="Time since startup: {}".format(data[2]))
    message.config(text="Message: {}".format(data[3]))

    # Schedule the next update
    updateDataJob = window.after(int(interval * 1000), updateData, interval)# Update every 1000 milliseconds (1 second). Tkinter handles the recursive to avoid memory leak.

def submit():
    global updateDataJob

    global setupRun
    cancelJob()

    interval = ""

    try:
        comOverride = comOverrideIn.get()
        interval = intervalIn.get()
        datalogging = dataloggingIn.get()
        
        interval = str(interval)
        interval = interval.replace(",", ".")
        interval = float(interval)
        comOverride = comOverride.upper()
        datalogging = datalogging.lower()
        if comOverride == "":
            comOverride = None
        
        comOverrideValid = True
        if not comOverride == None: #Override detect
            comOverrideValid = False
            comlist = serial.tools.list_ports.comports()
            for port in comlist:                            # Repeats for each connected port
                port = str(port)
                match = re.search(r'COM\d+', port)          #Checks for a match for "COM" + "number"...
                comPort = match.group()                     #...and converts from match datatype to string
                if comOverride == comPort:
                    comOverrideValid = True
                    break
        
        if (datalogging == "y" or datalogging == "n") and isFloat(interval) and comOverrideValid: #If all user input is valid, we can begin
            #Check if user input contains a y or n, and set the datalogging variables accordingly
            if datalogging == "y":
                datalogging = True

            elif datalogging == "n":
                datalogging = False

            #time.sleep(60)
            labview.interuptSetup = False
            print("Running setup")
            if not setupRun:
                setup(comOverride, False, datalogging)
                setupRun = True
            updateData(interval)

    except Exception as e:
        print(traceback.print_exc())



window = tk.Tk()

# Create input fields
comOverrideInText = tk.Label(window, text="    Manual COM override in case of Setup unsuccessful (example COM4)? Leave empty for auto.    ") #Does the decimal point conversion across regions work in the labview.py script?
comOverrideInText.pack()
comOverrideIn = tk.Entry(window)
comOverrideIn.pack()

intervalInText = tk.Label(window, text="    What sensor reading time interval do you want in seconds (minimum 0.02s with only SHT85, 0.2s with MAX31856)?    ") #Does the decimal point conversion across regions work in the labview.py script?
intervalInText.pack()
intervalIn = tk.Entry(window)
intervalIn.pack()

dataloggingInText = tk.Label(window, text="Datalogging? (y/n)")
dataloggingInText.pack()
dataloggingIn = tk.Entry(window)
dataloggingIn.pack()

# Create submit button
submitButton = tk.Button(window, text = "Run", command = submit)
submitButton.pack()

stopButton = tk.Button(window, text = "Stop", command = cancelJob)
stopButton.pack()


humidity = tk.Label(window, text="Humidity: {}".format(data[0]))
humidity.pack()

temperature = tk.Label(window, text="Temperature: {}".format(data[1]))
temperature.pack()

deltaStartupTime = tk.Label(window, text="Time since startup: {}".format(data[2]))
deltaStartupTime.pack()

message = tk.Label(window, text="Message: {}".format(data[3]))
message.pack()

window.mainloop()