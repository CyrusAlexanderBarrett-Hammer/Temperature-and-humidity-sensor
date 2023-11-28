//Tid:
const uint32_t seriellTimeoutMillis = 0;                        // Timeout, hvis ikke kontakt med pc (0: ingen timeout, 120000:2 min)
uint32_t pwmDelay = 1000;                                       // Tid mellom pwm-avlesing
uint32_t fuktighetDelay = 20;                                 // Tid mellom avlesing fra fuktighetssensor
uint32_t temperaturDelay = 200;                                // Tid mellom avlesing fra temperaturkontroller
uint32_t messageDelay = 200;                                          //Spam filter timeout duration
uint32_t SensorErrorDelay = 6000;

unsigned long currentTime;


// Tilkoblede enheter:
bool seriellKomunikasjon = 1;                                   // Tilkoblet pc?
bool klokke = 1;                                                // Er klokke tilkoblet? Krever seriell
bool fuktighet = 1;                                             // Er fuktighetssensor tilkoblet? Krever seriell
bool temperatur = 0;                                            // Er temperatursensor tilkoblet? Krever seriell
bool shtTemperatur = 1;                                         // Print temperatur fra SHT av/på. Dette vil skje uansett om thermocouple brukes.
bool input24 = 0;                                               // Er 5-24V PWM-inngang koblet til
bool input2 = 0;                                                // Er 2-5V PWM-inngang koblet til

char mode = 2;                                                  // Mode 1 uses thermocouple for temperature and SHT85 for humidity, mode 2 uses SHT85 for both

// Skal Arduinoen gå til error ved veil i kode:
bool CMD_Ugyldig = 0;                                           // Hvis ugyldig kode
bool CMD_UgyldigInnhold = 0;                                    // Hvis innhold i kode er ugyldig eller for langt

uint8_t tcType = 3;                                             // Type thermocouple som tall (1-7)
    // Alternativer: B:0, E:1, J:2, K:3, N:4, R:5, S:6, T:7


// ---------------------------------------------------------------------------------------------------------------------


uint32_t braudRate = 115200;                                    // Bånnbredde på signal (bits per sekund)

// Definer I/O pinner
#define srReset     4                                           // Reset for sr-latch
#define pwm24       5                                           // Status pwm 3V-24V inngang, fra sr-latch
#define pwm2        6                                           // Status pwm 2,2V-5V inngang, fra sr-latch
#define rele        7                                           // Utgang for rele, skal brytes hvis feil med pwm
#define alarm       8                                           // Led for alarm, blinkekoder
#define alarmReset  9                                           // Knapp for å resette alarm


#include <SPI.h>;

// Sett opp for klokke
#include "RTClib.h"                                             // Inkluder bibliotek for klokke (I2C)
RTC_DS1307 rtc;                                                 // Opprett objekt fra klasse RTC_DS1307

// Sett opp for fuktighetssensor
#include <SHT85.h>                                              // Inkluder bibliotek for fuktighetssensor (I2C)
#define SHT85_ADDRESS 0x44                                      // Sett I2C-adresse for fuktighetssensoren 
SHT85 sht;                                                      // Opprett objekt fra klasse SHT85

// Sett opp for temperaturkontroller
#include <Adafruit_MAX31856.h>                                  // Inkluder bibliotek for temperaturkontroller (SPI)
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(SS);            // Temperaturkontroller, CS(SS) brukes for valg av temperaturkontroller

// Variabler for å lagre tid ved forrige avlesing
uint32_t previousPWM;                                           // Tid siden forrige pwm-avlesing
uint32_t previousFukt;                                          // Tid siden forrige avlesing av fuktighetssensor
uint32_t previousTemp;                                          // Tid siden forrige avlesing av temperaturkontroller
uint32_t previousMessage;                                       // Spam filter to avoid buffer overrun when sendimng messages
uint32_t previousErrChk;                                        // Tid siden siste error-avlesing av sensorer

//annet
uint8_t errorCode = 1;                                          // Variabel for feil m/standard error-kode
uint32_t failWaitTime = 1000;                                  // Temporary error pass time
uint8_t failChecks = 10;                                        // How many times to verify that error is not cleared
bool timeRead = false;                                          //Avoids double time and date writings
