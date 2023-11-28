void serialSetup()
{
  char serialRead[3];
        serialRead[2] = '\0';
        while (serialRead[0] != '1' || serialRead[1] != '0')    //      Vent til seriell kommunikasjon er opprettet, mens ikke:
        {
            digitalWrite(alarm, HIGH);
            delay(100);
            digitalWrite(alarm, LOW);
            delay(100);
            if(millis() - previousPWM >= seriellTimeoutMillis && seriellTimeoutMillis != 0)
            // Hvis tid siden start minus lagret tid er mer eller lik timeot (Linje 2), og timeout ikke er 0: Gå til error-funksjon med error-kode "4"            
            {
                errorCode = 2;                                  //              Sett error-kode til 2
                error();                                        //              Gå til error-funksjon
            }
            if (Serial.available() >= 2)
            {
                //Leser tegnene vi er interessert i, som er IDEELT først i rekken, før resten tømmes. En gang blir rekkefølgen riktig.
                serialRead[0] = Serial.read();
                serialRead[1] = Serial.read();
                while (Serial.available() > 0)
                {
                    Serial.read();
                }
            }
        }
        Serial.print("01\n");                                   //      Svar på oppstart (01)
}
