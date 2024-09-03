bool readSerialCommand()                                        // Funksjon for å lese kode fra pc
{
    char commandIn[14];                                         // Variabel å lagre kode i
    for (uint8_t commandIndex = 0; commandIndex < 13; commandIndex ++)
    //For loop, gjentas for hvert tegn som har plass i variabel for kode
    {
        commandIn[commandIndex] = Serial.read();                // Les tegn fra seriell inn i variabel
        if (commandIn[commandIndex] == '\n')                    // hvis tegn fra seriell er newline:
        {
            commandIn[commandIndex] = '\0';                     //      Erstatt med "null" for å avslutte variabelen
            break;                                              //      Gå ut av loop
        }
        if (commandIndex >= 12)                                 // Hvis variabel for plass i tekst er større eller lik siste plass i tekst-variabel
        {                                                       // Og ugyldig innhold i kode fra PC skal gi error:
            if (CMD_UgyldigInnhold)
            {
                errorCode = 13;                                 //      Sett error-kode til 13
                return 0;                                       //      Gå ut av funksjon med error
            }
            return 1;
        }
    }
    if (commandIn[0] != '1')                                    // Hvis første tegn i kodeen ikke er "1" (Alle koder til Arduino begynner med "1")
    {                                                           // og ugyldig innhold i kode fra PC skal gi error:
        if (CMD_UgyldigInnhold)
        {
            errorCode = 14;                                     //      Sett error-kode til 13
            return 0;                                           //      Gå ut av funksjon med error
        }
        return 1;
    }

    if(commandIn[1] == '0'){serialSetup();}

    if (commandIn[1] == '1')                                    // Hvis Slå av/på funksjon
    {
        if (commandIn[2] == '1')                                //      Hvis "1" ("Slå seriell kommunikasjon av")
        {
            Serial.end();                                       //          Slå av seriell kommunikasjon
            seriellKomunikasjon = 0;                            //          Endre variabel for seriell kommunikasjon
            return 1;                                           //          Gå ut av funksjon
        }

        bool* sensorPtr;                                        //      Sette opp variabel som pointer
        if (commandIn[2] == '2') sensorPtr = &klokke;           //      Hvis "2"; sett variabel til minneadressen til variabel for klokke
        if (commandIn[2] == '3') sensorPtr = &fuktighet;        //      Hvis "3"; sett variabel til minneadressen til variabel for fuktighet
        if (commandIn[2] == '4') sensorPtr = &temperatur;       //      Hvis "4"; sett variabel til minneadressen til variabel for temperatur
        if (commandIn[2] == '5') sensorPtr = &input24;          //      Hvis "5"; sett variabel til minneadressen til variabel for 24V PWM
        if (commandIn[2] == '6') sensorPtr = &input2;           //      Hvis "6"; sett variabel til minneadressen til variabel for 2V PWM
        if (commandIn[2] == '7') sensorPtr = &CMD_Ugyldig;      //      Hvis "7"; sett variabel til minneadressen til variabel for error ved ugyldig kode
        if (commandIn[2] == '8') sensorPtr = &CMD_UgyldigInnhold; //      Hvis "8"; sett variabel til minneadressen til variabel for error ved ugyldig innhold i kode

        if (!sensorPtr)                                         //      Hvis pointer ikke er satt og :
        {
            if (CMD_UgyldigInnhold)                             //          Hvis ugyldig innhold i kode fra PC skal gi error:
            {
                errorCode = 14;                                 //              Sett error-kode til 14
                return 0;                                       //              Gå ut av funksjon med error
            }
            return 1;                                           //          Gå ut av funksjon
        }

        if(commandIn[3] == '0')                                 //      Hvis valgt at funksjon skal slås av
        {
            if (!*sensorPtr) return 1;                          //          Hvis allerede av: Gå ut av funksjon
            *sensorPtr = 0;                                     //          Endre variabel pointeren viser til
        }

        if(commandIn[3] == '1')                                 //      Hvis valgt at funksjon skal slås på
        {
            if(*sensorPtr) return 1;                            //          Hvis allerede på: gå ut av funksjon
            *sensorPtr = 1;                                     //          Endre variabel pointeren viser til
        }
        return 1;                                               //      Gå ut av funksjon
    }


    if (commandIn[1] == '2')                                    // Hvis nytt delay
    {
        char numBuffer[11];                                     //      Variabel for å lagre verdi som tekst
        for (uint8_t i = 0; i < (sizeof(commandIn) / sizeof(char) - 3); i++)
        {                                                       //      Gjenta for antall tegn i kode minus 3
            numBuffer[i] = commandIn[i + 3];                    //          flytt tegn tre plasser fremover i ny variabel
            if
            (
                (numBuffer[i] > '9' || numBuffer[i] < '0') &&   //          Hvis tegn ikke er tall
                numBuffer[i] != '\n' &&                         //          og ikke '\0' (0x0)
                CMD_UgyldigInnhold == 1
            )

            {
                errorCode = 15;                                 //                  Sett error-kode til 16
                return 0;                                       //                  Gå ut av funksjon med error
            }
        }

        uint32_t* delayPtr;                                     //          Sett opp pointer for nytt delay

        if (commandIn[2] == '0') delayPtr = &pwmDelay;          //          Hvis ny PWM-frekvens: Sett pointer til variabel for PWM-frekvens
        if (commandIn[2] == '1') delayPtr = &fuktighetDelay;    //          Hvis ny fuktighetsfrekvens: Sett pointer til variabel for fuktighetsfrekvens
        if (commandIn[2] == '2') delayPtr = &temperaturDelay;   //          Hvis ny temperaturfrekvens: Sett pointer til variabel for temperaturfrekvens
        if (commandIn[2] == '3') delayPtr = &SensorErrorDelay;  //          Hvis ny sensor-error-frekvens: Sett pointer til variabel for sensor-error-frekvens

        if (!delayPtr)                                          //          Hvis pointer ikke satt:
        {
            if (CMD_UgyldigInnhold)                             //              Hvis ugyldig innhold i kode fra PC skal gi error:
            {
                errorCode = 15;                                 //                  Sett error-kode til 14
                return 0;                                       //                  Gå ut av funksjon med error
            }
            return 1;                                           //              Gå ut av funksjon
        }
        *delayPtr = atol(numBuffer);                            //          Endre variabel pointeren viser til
        return 1;                                               //          Gå ut av funksjon
    }

    if (commandIn[1] == '3')                                    //      Hvis ny tc-type
    {
        if (commandIn[2] > '7' || commandIn[2] < '0')           //          Hvis utenfor rekkevidde
        {
            if (CMD_UgyldigInnhold)                             //              Hvis ugyldig innhold i kode skal gi error
            {
                errorCode = 15;                                 //                  Sett error-kode til 14
                return 0;                                       //                  Gå ut av funksjon med error
            }
            return 1;                                           //              Gå ut av funksjon
        }
        tcType = atoi(commandIn[2]);                            //          Gjør tctype i kode til int, og legg inn i variabel for tctype
        maxthermo.setThermocoupleType(tcType);                  //          Sett termocouple-type til verdi i variabel
        if (!readMax31856Status())                              //          Hvis tc-status gir feil
        {
            errorCode = 7;                                      //              Sett error-kode til 6
            return 0;                                           //              Gå ut av funksjon med error
        }
        return 1;                                               //          Gå ut av funksjon
    }

    if (commandIn[1] == '4')                                    //      Hvis "les feil fra sensorer"
    {
        if (temperatur)                                         //          Hvis temperatursesor er i bruk
        {
            if(mode == 1)
            {
              if(!readMax31856Status()) return 0;               //              Hvis feil med temperaturkontroller: Gå ut av funksjon med error;
            }
            return 1;
        }
        if (fuktighet)                                          //          Hvis fuktighetssensor er i bruk
        {
            if (!readSht85Status())                             //              Hvis feil med fuktighetssensor
            {
                errorCode = 8;                                  //                  Sett error-kode til 8
                return 0;                                       //                  Gå ut av funksjon med error
            }
            return 1;                                           //          Gå ut av funksjon
        }
    }

    if (CMD_Ugyldig)                                            //      Hvis feil hvis ugyldig kode
    {
        errorCode = 14;                                         //          Sett error-kode til 14
        return 0;                                               //          Gå ut av funksjon med error
    }
    return 1;                                                   //      Gå ut av funksjon
}