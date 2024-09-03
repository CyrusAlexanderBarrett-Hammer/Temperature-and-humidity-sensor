All serial handling shall be done in their own classes!


Skips labview function call on stall/freeze to avoid blocking

Datalogs all data on request from user

Gets startup time from PC, and uses PC time as reference to adjust Arduino time input since it's a bit off.

Keeps track of humidity, temperature, time, user message, and error indicator (HTTUE) for user

Sets up system by:
    
    Records start time from computer

    Forces datalogging off if Labview is used, it should be done from there in that case

    Gets system decimal point ("." or ","), for later output format

    Sends any error to calling application, with an error indication boolean

    Calls COM setup function, if no errors

    Data to calling application (CAD):
        start time, port object, status, and error indication status

    Sets status to "Setup successful" and returns CAD, if no errors

    Sounds the alarms with status and indicator indication, tells the run function not to run, and returns CAD, if error