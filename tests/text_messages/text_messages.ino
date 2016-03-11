// Based on the FONA example code by Adafruit

// Libraries
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

// Pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// Buffer
char replybuffer[255];

// Instances
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Fona instance
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t type;

void setup() {

  // Initi serial
  while (!Serial);
  Serial.begin(115200);
  Serial.println(F("FONA reading SMS"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Setup GPRS settings
  //fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));

  // Read all SMS
  int8_t smsnum = fona.getNumSMS();
  uint16_t smslen;
  int8_t smsn;

  // Check if 3G
  if ( (type == FONA3G_A) || (type == FONA3G_E) ) {
    smsn = 0; // zero indexed
    smsnum--;
  } else {
    smsn = 1;  // 1 indexed
  }

  // Read all SMS
  for ( ; smsn <= smsnum; smsn++) {
    Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
    if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
      Serial.println(F("Failed!"));
      break;
    }
    // if the length is zero, its a special case where the index number is higher
    // so increase the max we'll look at!
    if (smslen == 0) {
      Serial.println(F("[empty slot]"));
      smsnum++;
      continue;
    }

    // Print SMS on Serial
    Serial.print(F("***** SMS #")); Serial.print(smsn);
    Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
    Serial.println(replybuffer);
    Serial.println(F("*****"));
  }

}
void loop() {
  
}

