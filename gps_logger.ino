#include "Adafruit_GPS_small.h"
#include <SD.h>

uint32_t timer = millis();
Adafruit_GPS GPS(&Serial1);

bool cardInitialized = false;

#define GPSECHO false
void setup()  
{
  //Init Serial

  Serial.begin(9600);
  delay(5000);

  //Init red LED
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, INPUT_PULLUP);

  //Init SD Card
  if (!SD.begin(4)) {
    return;
  }
  cardInitialized = true;
  GPS.begin(9600);
  GPS.sendCommand(F("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"));//PMTK_SET_NMEA_OUTPUT_RMCGGA));
  GPS.sendCommand(F("$PMTK220,1000*1F"));
  delay(1000);

}

String padString(int in, int n)
{
  String out = String(in,DEC);
  int len = out.length();
  for (int i = 0; i < n-len; i++)
  {
    out = "0" + out;
  }
  return out;
}

void writeHeader(String fileName)
{
  File file = SD.open(fileName,FILE_WRITE);
  file.println(F("datetime,lat,lon,mph,heading,altitude,nSatellites"));
  file.close();
  Serial.println(F("Header Written"));
}

String createDataString(Adafruit_GPS *gps)
{
  String out = "";
  out += padString(gps->year,2)+ "/" + padString(gps->month,2) + "/" + padString(gps->day,2) + " ";
  out += padString(gps->hour,2) + ":" + padString(gps->minute,2) + ":" + padString(gps->seconds,2) + "." + padString(gps->milliseconds,3)+",";
  out += String(gps->latitudeDegrees,6) + "," + String(gps->longitudeDegrees,6) + ",";
  out += String(gps->speed*1.151,2) + "," + String((int)gps->angle) + ",";
  out += String((int)gps->altitude) + "," + String((int)gps->satellites);
  return out;   
}

String getFileName()
{
  return padString(GPS.month,2) + padString(GPS.day,2) + padString(GPS.hour,2) + ".csv";
}



void loop()                     // run over and over again
{

  String currentDataPoint;
  String fileName;
  File file;
  //Flash Red LED code
  
  
  char c = GPS.read();
  //if you want to debug, this is a good time to do it!
  if ((c) && (GPSECHO))
    Serial.write(c); 
  
  
  if (GPS.newNMEAreceived()) {
   
    if (!GPS.parse(GPS.lastNMEA()))  
      return;  
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 1000) { 
    timer = millis(); // reset the timer

    if (GPS.fix)
    {

      if (digitalRead(7) == LOW)
      {
        cardInitialized = false;
        Serial.println(F("No Card inserted"));
        return;
      }

      if (!cardInitialized)
      {
        if (!SD.begin(4)) {
          Serial.println(F("Can't initialize"));
        }
        cardInitialized = true;
      }
      
      fileName = getFileName();
      digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
      digitalWrite(8, HIGH);
      if (!SD.exists(fileName))
      {
        writeHeader(fileName);
      }
      file = SD.open(getFileName(),FILE_WRITE);
      
      if (file)
      {
        currentDataPoint = createDataString(&GPS);
      Serial.println(currentDataPoint);
      file.println(currentDataPoint);
      //Serial.println(F("Data point"));
      file.close(); 
      }
      else
      {
        
        Serial.println(F("Nope"));
      }
      digitalWrite(13, LOW); 
      digitalWrite(8,LOW);
     
  }
}
}
