// Data logger for Arduino Uno, Adafruit data logger shield, K-30 CO2 sensor, BME2800 sensor
// Based on Adafruit data logger example file lighttemplogger.ino; K30_basic from CO2meter.com
// RToledo-Crow, NGENS-ASRC-CUNY, 7.15.18

#include <SPI.h> 
#include <SD.h> 
#include <Wire.h>
#include "RTClib.h"
#include "SoftwareSerial.h"

#define LOG_INTERVAL  3000 // mills between entries (reduce to take more/faster data)
// SYNC_INTERVAL: how many milliseconds before writing the logged data permanently to disk
// SYNC_INTERVAL = LOG_INTERVAL to write each time (safest)
// SYNC_INTERVAL = 10*LOG_INTERVAL write data every 10 datareads 
// ... can lose 10 reads, uses less power, is much faster
#define SYNC_INTERVAL 3000 // mills between calls to flush() - to write data to the card
#define ECHO_TO_SERIAL 1 // echo data to serial port
#define WAIT_TO_START 0 // Wait for serial input in setup()
#define redLEDpin 2    // the digital pins that connect to the LEDs
#define greenLEDpin 3
#define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!

RTC_PCF8523 RTC; // Real Time Clock for RevB Adafruit logger shield
SoftwareSerial K_30_Serial(8,9); // Virtual serial port for k30 on pinS 8,9 (Rx,Tx). Logger Shield uses 10,12,13
File logfile;  // the logging file

const int chipSelect = 10; // for the data logging shield, we use digital pin 10 for the SD cs line
uint32_t syncTime = 0; // time of last sync()
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response
int valMultiplier = 1; // multiplier, default 1, 3 for K-30 3%, 10 for K-33 ICB

void setup(void) {
  Serial.begin(9600);
  K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600
#if ECHO_TO_SERIAL
    Serial.println("serial up, k30 up");
#endif

  pinMode(redLEDpin, OUTPUT);   // use debugging LEDs
  pinMode(greenLEDpin, OUTPUT);
  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif

  Serial.print("Initializing SD card...");   // initialize the SD card
  pinMode(10, OUTPUT); // default chip select pin set to output, even if you don't use it:
  if (!SD.begin(chipSelect)) {  // see if the card is present and can be initialized:
    error("Card failed, or not present");
  }
#if ECHO_TO_SERIAL  
  Serial.println("card initialized.");
#endif
  
  char filename[] = "LOGGER00.TXT";  // create a new file
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (!SD.exists(filename)) {  // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop
    }
  }
  if (!logfile) 
    error("couldnt create file");
  
  Serial.print("Logging to file: ");
  Serial.println(filename);
  Wire.begin();  // connect to RTC
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  analogReference(EXTERNAL); // If you want to set the aref to something other than 5v
}

void loop(void)  {
  DateTime now;
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL)); // delay between readings
  digitalWrite(greenLEDpin, HIGH);

  sendRequest(readCO2); // read the k30
  unsigned long valCO2 = getValue(response); // interpret response
    
//  uint32_t m = millis(); // log milliseconds since starting
//  logfile.print(m);      // milliseconds since start
//  logfile.print(", ");   
//   
//#if ECHO_TO_SERIAL
//  Serial.print(m);       // milliseconds since start
//  Serial.print(", ");  
//#endif
  
  now = RTC.now(); // fetch the time
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print("  CO2 ppm = ");
  logfile.print(valCO2, DEC);
  logfile.println("");
  
#if ECHO_TO_SERIAL
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print("  CO2 ppm = ");
  Serial.print(valCO2, DEC);
  Serial.println("");
#endif //ECHO_TO_SERIAL

  digitalWrite(greenLEDpin, LOW);
  // Now write to disk. Uses 2048 bytes of I/O to SD card, power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);
}
