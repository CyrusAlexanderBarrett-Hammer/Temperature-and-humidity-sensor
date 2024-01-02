void resetThermocoupleFunc(){
  //digitalWrite(10, LOW);  // Select the MAX31856
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));
  SPI.transfer(0x0F);  // Write address (0x0F) with write command
  SPI.transfer(0x00);  // Write any byte to the register to clear the fault
  SPI.endTransaction();
  //digitalWrite(10, HIGH);  // Deselect the MAX31856
}

void resetSHT85Func()
{
  sht.getError();
}

bool readMax31856Status()                                       // Funksjon for å lese status for temperaturkontroller
{
    thermocoupleFaultHandler(7);
  
    if (maxthermo.getThermocoupleType() != tcType)              // Hvis temperaturkontroller melder at den er satt opp for annen type tc enn den skal være
    {
        errorCode = 6;                                          // Sett error-kode til "6"
        return 0;                                               // Gå ut av funkson med error
    }
  
    //All good here, ignore this
    if (maxthermo.getThermocoupleType() != tcType)              // Hvis temperaturkontroller melder at den er satt opp for annen type tc enn den skal være
    {
        errorCode = 6;                                          // Sett error-kode til "6"
        return 0;                                               // Gå ut av funkson med error
    }

    return 1;                                                   // Gå ut av funksjon
}

bool readSht85Status()                                          // Funksjon for å lese om feil med fuktighetssensor
{
    SHT85FaultHandler(9);
    if (
        sht.readStatus() == 0xFFFF || sht.getError()
    )   // Hvis feil under funksjon for å lese status på fuktighetssensor
        // eller hvis fuktighetssensor melder feil
    {
        return 0;                                               // Gå ut av funksjon med error
    }
    return 1;                                                   // Gå ut av funksjon
}
