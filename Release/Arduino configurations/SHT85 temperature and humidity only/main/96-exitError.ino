bool exitError(bool serialRecive, uint32_t tid, uint8_t* countPtr)
// Funksjon for å gå ut av finksjon, m/ variabel for om funsjon er kalt fra seriell + tid når kode ble mottatt + referanse til hvor i blinkekoden koden ble stoppet.
{
    if (serialRecive)                                           //      Hvis melding fra seriell
    {
        delay(10);                                              //          10ms delay for at data skal ha tid til å komme inn i buffer for seriell
        char serialRead[2];                                     //          2 byte tekststreng for seriell data
        serialRead[0] = Serial.read();                          //          Les data fra buffer til første byte i variabel
        serialRead[1] = Serial.read();                          //          Les data fra buffer til andre byte i variabel
        while (Serial.available()) Serial.read();               //          Tøm bufferet
        if (serialRead[0] != '1' || serialRead[1] != '9')       //          Hvis mottatt verdi ikke er "19"
        {
            Serial.print("09");                                 //              Skriv kode; "09" til pc
            if (errorCode < 10) Serial.print('0');              //              Hvis error-koden er under 10, skriv "0" til pc
            Serial.print(String(errorCode));                    //              Skriv error-koden til pc
            Serial.print('\n');                                 //              Skriv newline til pc
            return 0;                                           //              Gå ut av funksjon med error
        }
    }
    digitalWrite(alarm, LOW);                                   //      Sett alarm lavt
    *countPtr = 0;                                              //      Gå til start av blinkekode

    switch(errorCode)                                           //      Sammenlign error-koden med verdier 
    {
        case 2:                                                 //          Hvis lik 2
        {
            Serial.end();                                       //              Stopp kommunikasjon med pc
            Serial.begin(braudRate);                            //              Start kommunikasjon med pc
            Serial.print("00\n");                               //              Skriv "00" til pc m/ newline
            char serialRead[2];                                 //              2 byte variabel for data fra seriell
            while (serialRead[0] != '1' || serialRead[1] != '0')
                                                                //              Gjenta så lenge "10" ikke er mottatt fra pc
            {
                if(millis() - tid >= seriellTimeoutMillis && seriellTimeoutMillis != 0)
                // Hvis tid siden start minus lagret tid er mer eller lik timeot (Linje 2), og timeout ikke er 0: Gå til error-funksjon med error-kode "4"            
                {
                    return 0;                                   //                  Gå til error-funksjon
                }

                if (Serial.available())                         //              Hvis data mottatt fra seriell
                {
                    delay(10);                                  //                  10ms delay for at data skal ha tid til å komme inn i buffer
                    serialRead[0] = Serial.read();              //                  Les data fra seriell til første byte i variabel
                    serialRead[1] = Serial.read();              //                  Les data fra seriell til andre byte i variabel
                    while (Serial.available()) Serial.read();   //                  Tøm bufferet
                }
            }
            Serial.print("01\n");                               //              Svar på oppstart
            return 1;                                           //              Gå ut av funksjon
        }

        case 3:                                                 //          Hvis lik 3
        {
            if (!rtc.begin()) return 0;                         //              Hvis fortsatt under oppstart av kommunikasjon med klokke, gå ut av funksjon med error
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 4:                                                 //          Hvis lik 4
        {
            if (!sht.begin(SHT85_ADDRESS)) return 0;            //              Hvis fortsatt feil under oppstart av kommunikasjon for fuktighetssensor, gå ut av funksjon med error
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 5:                                                 //          Hvis lik 5
        {
            if (!maxthermo.begin()) return 0;                   //              Hvis fortsatt feil under oppstart av kommunikasjon for temperaturkontroller, gå ut av funksjon med error
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 6:                                                 //          Hvis lik 6 
        case 7:                                                 //          Eller hvis lik 7
        {
            if (!readMax31856Status()) return 0;                //              Hvis temperaturkontroller fortsatt melder feil, gå ut av funksjon med error
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 8:                                                 //          Hvis lik 8
        {
            if(mode == 1)
            {
              if (!readSht85(1)) return 0;                      //              Hvis fortsatt feil under avlesing av fuktighetssensor
            }

            if(mode == 2)
            {
              if (!readSht85(1)) return 0;                      //              Hvis fortsatt feil under avlesing av fuktighetssensor
              if (!readSht85(2)) return 0;
            }
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 9:                                                 //          Hvis lik 9
        {
            if (!readSht85Status()) return 0;                   //              Hvis fortsatt feil under avlesing av status på fuktighetssensor
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 10:                                                //          Hvis lik 10
        {
            if(!readMax31856()) return 0;                       //              Hvis fortsatt feil under avlesing av thermocouple
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 11:                                                //          Hvis lik 11
        case 12:                                                //          Eller hvis lik 12
        {
            if (!readPWM()) return 0;                           //              Hvis fortatt PWM er låst høyt
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }

        case 13:                                                //          Hvis lik 13
        case 14:                                                //          Eller hvis lik 14
        case 15:                                                //          Eller hvis lik 15
        case 16:                                                //          Eller hvis lik 16
        case 20:                                                //          Eller hvis lik 20
        {
            if(seriellKomunikasjon) Serial.print("08");         //              Hvis seriell, skriv ok til PC
            return 1;                                           //              Gå ut av funksjon
        }
    }
    return 0;                                                   //      Gå ut av funksjon med error (Ugyldig kode)
}
