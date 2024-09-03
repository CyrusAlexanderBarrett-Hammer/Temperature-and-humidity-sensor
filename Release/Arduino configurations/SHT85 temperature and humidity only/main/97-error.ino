void error ()                                                   // Funksjon for error
{
    digitalWrite(rele, LOW);                                    // Sett reléutgang lavt
    uint8_t a[2] = {0, errorCode};                              // Sett opp array for antall blink
    if (seriellKomunikasjon)                                    // Er seriell kommunikasjon slått på?

    {
        /*
        Serial.print("09");                                     //      Skriv kode; "09" til pc
        if (errorCode < 10) Serial.print('0');                  //      Hvis error-koden er under 10, skriv "0" til pc
        Serial.print(String(errorCode));                        //      Skriv error-kode til pc
        Serial.print('\n');                                     //      Skriv newline til pc
        */
    }
    while (a[1] > 5)                                            // Så lenge 2. del av blinkekoden er under 5:
    {
        a[0] ++;                                                //      Legg til 1 til 1. del av blinkekoden
        a[1] -= 5;                                              //      Trekk fra 5 fra 2. del av blinkekoden
    }
    uint32_t prevTimeE = millis();                              // Variabel for forrige blink
    bool errorPart = 0;                                         // Variabel for del av blinkekode
    uint16_t nextDelayE = 0;                                    // Variabel for lengde på delay
    uint8_t count = 0;                                          // Variabel for forrige blink i rekke

    while (1)                                                   // Loop
    {
        currentTime = millis();                                 // Hent tid siden oppstart
        //Serial.print("09");                                   //      Skriv kode; "09" til pc
        String fullError = "";
        if (errorCode < 10) fullError += '0';                   //      Hvis error-koden er under 10, skriv "0" til pc
        fullError += errorCode;
        alertPython(fullError, true);                              //      Skriv error-kode til pc

        //if (seriellKomunikasjon) {Serial.println("102");}
        uint32_t curTimeE = millis();                           //      Nåværende tid
        if (curTimeE - prevTimeE >= nextDelayE)                 //      Hvis tid siden forrige blink er større eller lik delay
        {
            if (count == 0)                                     //          Hvis første blink i rekke
            {
                nextDelayE = 200;                               //              Sett delay til 200ms
            }
            count ++;                                           //          Øk blink-nummer med 1
            if (count > a[errorPart] * 2)                       //          Hvis blink er mer enn 2 ganger antall blink i ledd av blinkekoden
            {
                nextDelayE = 1800;                              //              Sett delay til 1800ms
                if(!errorPart) nextDelayE = 400;                //              Hvis 1. del av error-kode: Sett delay til 400ms
                errorPart = !errorPart;                         //              Endre del av error-kode
                count = 0;                                      //              Sett blink-nummer til 0
            }
            else                                                //          Eller:
            {
                digitalWrite(alarm, !digitalRead(alarm));       //              Endre alarm - led
            }

            prevTimeE = curTimeE;                               //          Sett forrige tidspunkt til nåværende tidspunkt
        }
        if (
            (Serial.available() && exitError(1, curTimeE, &count)) || 
                                                                // Hvis riktig kode fra PC, og feil er rettet
            (digitalRead(alarmReset) && exitError(0, curTimeE, &count))
        )                                                       // Eller hvis knapp er trykket inn og feil er rettet
        {                                                       //
            digitalWrite(rele, HIGH);                           //      Sett reléutgang høyt
            return;                                             //      Gå ut av error
        }
    }
}