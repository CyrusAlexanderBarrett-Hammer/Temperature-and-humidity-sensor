void setup()                                                    // Arduino setup funksjon
{ 
    while(Serial.available() > 0){Serial.read();}

    //SHT-s temperature measurement is not in use, regardless. Avoid an error!
    //Overrides manual variable settings with relavant ones for the mode
    if(mode == 1)
    {
      shtTemperatur = 0;
    }

    if(mode == 2)
    {
      temperatur = 0;
    }

    previousPWM = millis();                                     // Setter variabel for tid siden forrige pwm-avlesing
    previousFukt = previousPWM;                                 // Setter variabel for tid siden forrige avlesing av fuktighetssensor
    previousTemp = previousPWM;                                 // Setter variabel for tid siden forrige avlesing av temperaturkontroller
    previousErrChk = previousPWM;                               // Setter variabel for tid siden forrige error-avlesing av sensorer

    pinMode(alarm, OUTPUT);                                     // Sett pinne for LED for alarm til output
    pinMode(alarmReset, INPUT);                                 // Sett pinne for reset av alarm til input

    pinMode(srReset, OUTPUT);                                   // Sett pinnen koblet til SR-latch reset til utgang
    pinMode(rele, OUTPUT);                                      // Sett pinnen koblet til Styring av relé til utgang

    pinMode(pwm24, INPUT);                                      // Set overvåkningspin for 5-24V inngangen til input
    pinMode(pwm2, INPUT);                                       // Set overvåkningspin for 2-5V inngangen til input

    if (seriellKomunikasjon) {                                  // Hvis kommunikasjon med pc er slått på
        Serial.begin(115200);                                   //      Start UART kommunikasjon
        serialSetup();

        /*
        char readMode[2];                                      //      Mode set by user from interface or Labview, going via Python
        readMode[1] = '\0';
        while(readMode[0] != '1' and readMode[0] != '2')
        {
            if (Serial.available() >= 1)
            {
                readMode[0] = Serial.read();
                while (Serial.available() > 0)
                {
                    Serial.read();
                }
            }
        }
        mode = readMode[0];
        */

    }
    if (klokke) {                                               // Hvis klokke er koblet til
        if (! rtc.begin())                                      //      Prøv å start I2C kommunikasjon med klokke, hvis feil:
        {
            errorCode = 3;                                      //          Sett error-kode til "3"
            error();                                            //          Gå til error-funksjon
        }
        if (! rtc.isrunning())                                  //      Hvis klokken ikke går
        {
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));     //          Sett klokke til klokkeslett ved kompilering av kode, ERSTATTES
        }
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Temperaturkontroller
    if (temperatur)                                             // Hvis temperaturkontroller er valgt at skal brukes
    {
      if(!maxthermo.begin())                                  //      Start kommunikasjon med temperaturkontroller, hvis feil:
      {
          errorCode = 5;                                      //          Sett error-kode til "2"
          error();                                            //          Gå til error-funksjon
      }            
      maxthermo.setThermocoupleType(tcType);                  //      Sett type thermocouple
      if (!readMax31856Status()) thermocoupleFaultHandler(7);                     //      Se om det er noe galt med temperaturkontroller, hvis feil: gå til error-funksjon  
    }

    // Fuktighetssensor
    if(fuktighet && shtTemperatur)
    {
      Wire.begin();                                               // Start I2C kommunikasjon for fuktighetssensor
      if (!sht.begin(SHT85_ADDRESS))                              //      Sett opp I2C kommunikasjon med fuktighetssensor, hvis feil:
      {
          //errorCode = 4;                                          //          Sett error-kode til 4                     
          //error();                                                //          Gå til error-funksjon
          SHT85FaultHandler(4);
      }
    }
    digitalWrite(rele, HIGH);                                   //      Sett reléutgang høyt
}