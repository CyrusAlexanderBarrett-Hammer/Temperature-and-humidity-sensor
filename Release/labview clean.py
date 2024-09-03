"""
Temperature and Humidity Sensor Script

This script interfaces with the temperature and humidity sensor developed by Joakim Vigemyr. It includes two main functions: `setup` and `run`, which can be executed from LabVIEW or other program.
- `setup`: Initializes the sensor and should be called once.
- `run`: Returns humidity and time data on each call.

These functions are also utilized by the standalone.py script.

Requirements:
- Python 3.6.8
- pyserial library (compatible with Python 3.6.8)

Next Steps:
- Refactor to be more function-based and possibly adopt an object-oriented approach.
"""

import time
import re
import math
import random
import locale
import sys
import traceback

from datetime import datetime, timedelta
from types import SimpleNamespace

# Uncomment the following lines if serial communication is needed
# import serial
# import serial.tools.list_ports
# from serial import Serial, SerialException, PortNotOpenError


class StrayVariables:
    user_message = None

    connection_errors = SimpleNamespace(reconnect_failed = None)


class UserData:
    labview = None
    datalogging = None
    com_override = None

    first_run_time = None  # Startup time according to computer

    decimal = None
    log_name = None

    data_to_user = SimpleNamespace(start_time=None, port=None, setup_error_indicator=None, humidity=None, temperature=None, time=None, run_error_indicator=None, user_message=None)
    user_choices = SimpleNamespace(datalogging=None)


class TimeHandling:
    time_compensation = None  # Arduino time compensation


class Datalogging:
    
    @classmethod
    def setup(cls):
        #Labview takes care of the datalogging
        if(UserData.labview == True):
            UserData.datalogging = False

        cls.set_decimal()

    
    @classmethod
    def set_decimal(cls):
        langlocale = locale.getdefaultlocale()[0]
        locale.setlocale(locale.LC_ALL, langlocale)
        UserData.decimal = locale.localeconv()['decimal_point']


class Com:
    """
    Sets up COM communication, accurate and corrected time as recieved from Arduino, and datalogging

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
    
    
    ser = None

    @classmethod
    def setup_com(cls, this_one):
        print("Scary")


    @classmethod
    def com_setup_procedure(cls):

        if UserData.data_to_user.user_message is None:
            UserData.data_to_user.user_message = Com.setup_com(UserData.com_override)

        UserData.data_to_user.start_time = datetime.now()
    
        if UserData.data_to_user.user_message is None:
            UserData.data_to_user.user_message = "Setup successful"
            return(UserData.data_to_user.start_time, Com.ser.port, UserData.data_to_user.user_message, UserData.data_to_user.setup_error_indicator)  # Outputs example: 2023-08-21 15:34:17.4
        else:
            UserData.data_to_user.setup_error_indicator = True
            UserData.data_to_user.general_error_active = True
            return(UserData.data_to_user.start_time, None, UserData.data_to_user.user_message, UserData.data_to_user.setup_error_indicator)


    @classmethod
    def setup(cls, com_override = None, labview = None, datalogging = False): #Default arguments makes Labview usage easier
        UserData.com_override = com_override
        UserData.labview = labview
        UserData.datalogging = datalogging

        Datalogging.setup()

        cls.com_setup_procedure()
    


def setup():
    Com.setup()


Com.com_setup_procedure()