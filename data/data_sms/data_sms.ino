// Based on the FONA example code by Adafruit

// Libraries
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
#include "DHT.h"

// DHT sensor
#define DHTPIN 7 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

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

// Threshold
float threshold = 26.0;

// Target number
char * number = "733382390";

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
  fona.setGPRSNetworkSettings(F("internet"));
  //fona.setGPRSNetworkSettings(F("your_APN"), F("your_username"), F("your_password"));

  // Init DHT
  dht.begin();

}
void loop() {

  // Measure data
  float temperature = dht.readTemperature(); 
  float humidity = dht.readHumidity();

  // Log
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Send SMS if threshold is crossed
  if (temperature < threshold) {

    // Build message
    char * message = "The temperature in your home is too low!";

    // Send    
    if (!fona.sendSMS(number, message)) {
      Serial.println(F("Failed to send SMS"));
    } else {
      Serial.println(F("SMS sent!"));
    }

    // Huge delay if alert is sent
    delay(60 * 60 * 1000);
    
  }

  // Wait
  delay(1000);
  
}

