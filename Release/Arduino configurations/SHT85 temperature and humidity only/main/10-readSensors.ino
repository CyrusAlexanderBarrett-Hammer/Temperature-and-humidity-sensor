//#include <RingBuf.h>

char* pythonRead()
{
  //digitalWrite(alarm, HIGH);
  char serialRead[3];
  serialRead[2] = '\0';
  
  serialRead[0] = Serial.read();
  serialRead[1] = Serial.read();
  if (Serial.available() >= 2) //Both digits recieved?
  {
      serialRead[0] = Serial.read();
      serialRead[1] = Serial.read();
      while (Serial.available() > 0) //Kicks in if digits still remain
      {
        //Error?
          Serial.read(); //Empty buffer
      }
  }
  //delay(500);
  //digitalWrite(alarm, LOW);

  //return serialRead[0];
  return serialRead;
}

bool readDs1307()                                               // Funksjon for å lese klokke
{
    //if(pythonRead[0] == '1' && pythonRead[1] == '0')
    //{
      //serialSetup();
    //}
    //else
    //{
      uint8_t tid;                                                // Definer generell variabel for tid 
      DateTime now = rtc.now();                                   // Les klokke til objekt "now" fra klasse "DateTime"
      Serial.print("02");                                         // Skriv kode; "02" til pc
      tid = now.day();                                            // Sett variabel "tid" til dag
      if (tid < 10) Serial.print('0');                            // Hvis dag er under 10, skriv "0" til pc 
      Serial.print(String(tid));                                  // Skriv dag til pc
      Serial.print('/');

      tid = now.month();                                          // Sett variabel "tid" til måned
      if (tid < 10) Serial.print('0');                            // Hvis måned er under 10(før oktober), skriv "0" til pc for at måneden alltid skal okupere 2 siffer i koden
      Serial.print(String(tid));                                  // Skriv måned til pc
      Serial.print('/');

      Serial.print(String(now.year()));                           // Skriv årstall til pc
      Serial.print(' ');

      tid = now.hour();                                           // Sett variabel "tid" til time
      if (tid < 10) Serial.print('0');                            // Hvis time er under 10, skriv "0" til pc 
      Serial.print(String(tid));                                  // Skriv time til pc
      Serial.print(':');

      tid = now.minute();                                         // Sett variabel "tid" til minutt
      if (tid < 10) Serial.print('0');                            // Hvis minutt er under 10, skriv "0" til pc 
      Serial.print(String(tid));                                  // Skriv minutt til pc
      Serial.print(':');

      tid = now.second();                                         // Sett variabel "tid" til sekund
      if (tid < 10) Serial.print('0');                            // Hvis sekund er under 10, skriv "0" til pc 
      Serial.print(String(tid));                                  // Skriv sekund til pc
      Serial.print('\n');
    //}
    return 1;                                                   // Gå ut av funksjon
}


bool readSht85(int readingType)                                    // Funksjon for å lese fra fuktighetssensor
{
    //if(pythonRead[0] == '1' && pythonRead[1] == '0')
    //{
      //serialSetup();
    //}
    //else
    //{
    //Possible insurpetrator
    if(!timeRead){if (klokke) if(!readDs1307()) return 0;}         // Hvis klokke brukes og ikke allerede lest av, skriv dag/klokke til pc, hvis feil, gå ut av funksjon med error
      sht.read();

      //String finishedString = "";
      //finishedString += "03";
      if(readingType == 2){
        uint16_t sht85Temperatur = (sht.getTemperature()) * 100;  
        // Les temperatur fra fuktighetssensoren, gjør om til Kelvin og gang med 100, for å ikke få fortegn eller desimaler. Maks 382,2 grader celsius 
        // ( Maks temperatur for fuktighetssensoren er 105, anbefalt 10-50)
        //finishedString += String(sht85Temperatur);

        Serial.print("04");
        if (sht85Temperatur < 10000) Serial.print('0');
        if (sht85Temperatur < 1000) Serial.print('0');
        if (sht85Temperatur < 100) Serial.print('0');
        if (sht85Temperatur < 10) Serial.print('0');
        Serial.print(String(sht85Temperatur));                    // Skriv temperatur til pc
        Serial.print('\n'); 
      }

      else if (readingType == 1)
      {
        //Serial.print(String(sht85Temperatur));
        //if(sht85Fuktighet < 10000) finishedString += ('0');
        //if(sht85Fuktighet < 1000) finishedString += ('0');
        //if(sht85Fuktighet < 100) finishedString += ('0');
        //if(sht85Fuktighet < 10) finishedString += ('0');

        uint16_t sht85Fuktighet = sht.getHumidity() * 100;          // Les fuktighet fra fuktighettsensoren, og gang med 100 for å fjerne desimaler
      
        Serial.print("03");                                         // Skriv kode; "03" til pc
        if (sht85Fuktighet < 10000) Serial.print('0');              // Legg til en null hvis fuktighet er under 100%
        if (sht85Fuktighet < 1000) Serial.print('0');               // Legg til en null hvis fuktighet er under 10%
        if (sht85Fuktighet < 100) Serial.print('0');                // Legg til en null hvis fuktighet er under 1%
        if (sht85Fuktighet < 10) Serial.print('0');                 // Legg til en null hvis fukktighet er under 0,1%
        //finishedString += String(sht85Fuktighet);
        //finishedString += '\n';
        Serial.print(String(sht85Fuktighet));                       // Skriv fuktighet til pc
        Serial.print('\n');
        //myBuffer.push(finishedString);
      }
    return 1;                                                   // Gå ut av funksjon
}


bool readMax31856()                                             // Funksjon for å lese fra thermocouple
{   
      if(!timeRead){if (klokke) if(!readDs1307()) return 0;}         // Hvis klokke brukes og ikke allerede lest av, skriv dag/klokke til pc, hvis feil, gå ut av funksjon med error
      //uint32_t max31856Temperatur = maxthermo.readThermocoupleTemperature();
      //uint32_t max31856Temperatur = (maxthermo.readThermocoupleTemperature() + 273.15) * 100;
      uint32_t max31856Temperatur = (maxthermo.readThermocoupleTemperature() * 100);
      // Les temperatur fra thermocoupelen, gjør om til Kelvin og gang med 100, for å ikke få fortegn eller desimaler. Maks 42 949 399,8 grader celsius, like varmt som solen!

      //String finishedString = "";
      //finishedString += '04';
      //finishedString += String(max31856Temperatur);
      //finishedString += '\n';
      Serial.print("04");                                         // Skriv kode; "04" til pc
      Serial.print(String(max31856Temperatur));                   // Skriv temperatur til pc
      Serial.print('\n');                                       // Skriv newline til pc
      //myBuffer.push(finishedString);
    //}
    
    return 1;                                                   // Gå ut av funksjon
}


bool readPWM()                                                  // Funksjon for å lese status på pwm-signalene
{
    if (input2) if (!digitalRead(pwm2))                         // Hvis 2-5V inngangen er valgt at skal brukes, og låst høyt(inngang lav):
    {
        errorCode = 11;                                         // Sett error-kode til 10
        return 0;                                               // Gå ut av funksjon med error
    }
    if (input24) if (!digitalRead(pwm24))                       // Hvis 5-24V inngangen er valgt at skal brukes, og låst høyt(inngang lav):
    {
        errorCode = 12;                                         // Sett error-kode til 11
        return 0;                                               // Gå ut av funksjon med error
    }
    
    digitalWrite(srReset, HIGH);                                // Sett reset-pin høyt
    digitalWrite(srReset, LOW);                                 // Sett reset-pin lavt
    return 1;                                                   // Gå ut av funksjon
}
