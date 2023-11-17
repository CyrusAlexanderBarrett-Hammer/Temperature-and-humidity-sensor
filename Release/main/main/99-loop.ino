void loop()                                                     // Arduino loop funksjon
{ 
    unsigned long currentTime = millis();                       // Hent tid siden oppstart
    
    if (seriellKomunikasjon)                                    // Hvis seriell komunikasjon er aktivert
    {
        timeRead = false;                                       // Ready to send time and date reading, only once per loop
        if (fuktighet && (fuktighetDelay <= currentTime - previousFukt))
        // Hvis fuktighetssensor er valgt at skal brukes, og tid siden start minus tid siden start under forrige fuktavlesing er mer eller lik variabel for tid mellom fuktavlesinger
        {
            if (!readSht85(1))                                   // Les fra fuktighetssensoren, hvis feil:
            {
                SHT85FaultHandler(8);                                        //      gå til error-funksjon
            }
            previousFukt = currentTime;                         // Sett variabel for forrige fuktavlesing til nåværende tid
            timeRead = true;                                    //Brap! Read once, avoid duplicates!
        }

        if(temperaturDelay <= currentTime - previousTemp)                
        // Hvis temperaturkontroller er valgt at skal brukes, og tid siden start minus tid siden start under forrige temperaturavlesing er mer eller lik variabel for tid mellom temperaturavlesinger
        {
          if (mode == 1 && temperatur == 1)
          {
              if (!readMax31856())                                // les fra thermocouple, hvis feil:
              {
                thermocoupleFaultHandler(7);
              }
          }

          else if (mode == 2 && shtTemperatur == 1)
          {
            if (!readSht85(2))
            {
              SHT85FaultHandler(8);
            }
          }
          previousTemp = currentTime;                         // Sett variabel for forrige temperaturavlesing til nåværende tid
          timeRead = true;                                    //Brap! Read once, avoid duplicates!
        }
        
        
        timeRead = false;                                       //Ready for next round

        //temperatur && (temperaturDelay <= currentTime - previousTemp)
        
        if (Serial.available() > 0)                             // Hvis data i buffer for seriell data
        {
            if (!readSerialCommand()) error();                  // Les kode fra pc, hvis feil: gå til error-funksjon
        }
    }
    
    if ((input2 || input24) && (pwmDelay <= currentTime - previousPWM))
    // Hvis 2-5V eller 5-24V inngangene er valgt at skal brukes, og tid siden start minus tid siden start under forrige PWM-avlesing er mer eller lik variabel for tid mellom PWM-avlesinger
    {
        if (!readPWM()) error();                                // Les PWM-status, hvis feil: gå til error-funksjon
        previousPWM = currentTime;                              // Sett variabel for forrige PWM-avlesing til nåværende tid
    }

    if (SensorErrorDelay <= currentTime - previousErrChk)       // Hvis tid siden forrige error-sjekk er større eller lik valgt tid mellom error-sjekk
    {
        if (fuktighet) if (!readSht85Status()) SHT85FaultHandler(8);         // Hvis fuktighetssensor er i bruk, hvis fuktighetssensor meldet feil, gå til error-funksjon
        if (temperatur) if (!readMax31856Status()) thermocoupleFaultHandler(7);     // Hvis temperaturkontroller er i bruk, hvis temperaturkontroller meldet feil, gå til error-funksjon
        previousErrChk = currentTime;
    }
}


void thermocoupleFaultHandler(uint8_t code)
{
  //while(true){
    //Serial.println(maxthermo.readThermocoupleTemperature());
    //resetFunc();
    //}
  //During a fault, the reading will freeze. faultReference and faultUpdated are updated and compared before and after freezes.
  uint32_t faultReference = 0;
  uint32_t faultUpdated = 0;
  
  uint8_t fault = maxthermo.readFault();
  if(fault){
        delay(failWaitTime);
        resetThermocoupleFunc();
        delay(200);

        uint8_t check;
        for(check = 1; check <= failChecks; check++)
        {
          faultReference = maxthermo.readThermocoupleTemperature() * 100;
          resetThermocoupleFunc();
          delay(200);
          faultUpdated = maxthermo.readThermocoupleTemperature() * 100;
          
          if(check >= failChecks && faultReference == faultUpdated)
          {
            errorCode = code;
            error();
          }
          else if(faultUpdated != faultReference){break;}
        }
        
        
        //Ignore, ignore!
        //uint8_t fault = maxthermo.readFault();                      // Les error fra temperaturkontroller
    }
}

void SHT85FaultHandler(uint8_t code)
{
  //During a fault, the reading will freeze. faultReference and faultUpdated are updated compared before and after freezes.
  uint32_t faultReference = 0;
  uint32_t faultUpdated = 0;
  
  uint8_t fault = sht.getError();
  if(fault){
        delay(failWaitTime);
        if(code == 4)
        {
          Wire.begin();
        }
        else
        {
          resetSHT85Func();
        }
        delay(200);

        uint8_t check;
        for(check = 1; check <= failChecks; check++)
        {
          sht.read();
          faultReference = sht.getHumidity() * 100;
          resetSHT85Func();
          delay(200);
          sht.read();
          faultUpdated = sht.getHumidity() * 100;
          
          if(check >= failChecks && faultReference == faultUpdated)
          {
            errorCode = code;
            error();
          }
          else if(faultUpdated != faultReference){break;}
        }
        
        
        //Ignore, ignore!
        //uint8_t fault = maxthermo.readFault();                      // Les error fra temperaturkontroller
    }
}
