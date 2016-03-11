// Libraries
#include <Adafruit_SleepyDog.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_FONA.h"

// Pins
#define RELAY_PIN   7
#define FONA_RX     2
#define FONA_TX     3
#define FONA_RST    4

// Fona instances
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

// GPRS settings
#define FONA_APN       "internet"
#define FONA_USERNAME  ""
#define FONA_PASSWORD  ""

// Adafruit IO settings
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "name"
#define AIO_KEY         "key"

/************ Global State (you don't need to change this!) ******************/
// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = __TIME__ AIO_USERNAME;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the FONA MQTT class by passing in the FONA class and MQTT server and login details.
Adafruit_MQTT_FONA mqtt(&fona, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// FONAconnect is a helper function that sets up the FONA and connects to
// the GPRS network. See the fonahelper.cpp tab above for the source!
boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password);

// Create feed for relay
const char RELAY_FEED[] PROGMEM = AIO_USERNAME "/feeds/relay";
Adafruit_MQTT_Subscribe relay = Adafruit_MQTT_Subscribe(&mqtt, RELAY_FEED);

/*************************** Sketch Code ************************************/

void setup() {

  // Set relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200);

  // Initialise the FONA module
  while (! FONAconnect(F(FONA_APN), F(FONA_USERNAME), F(FONA_PASSWORD)))
    halt("Retrying FONA");

  Serial.println(F("Connected to Cellular!"));

  Watchdog.reset();
  delay(3000);  // wait a few seconds to stabilize connection
  Watchdog.reset();

  // Listen for events
  mqtt.subscribe(&relay);

  // connect to adafruit io
  connect();

}

void loop() {

  // Subscription
  Adafruit_MQTT_Subscribe *subscription;

  // Make sure to reset watchdog every loop iteration!
  Watchdog.reset();

  // Ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // Process incoming data
  while (subscription = mqtt.readSubscription(1000)) {

    // If it's a relay event
    if (subscription == &relay) {

      // Convert mqtt ascii payload to int
      char *value = (char *)relay.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      String relayStatus = String(value); 
      relayStatus.trim();

      // Set the status of the relay
      if (relayStatus == "ON") {
       digitalWrite(RELAY_PIN, HIGH);
      }
      if (relayStatus == "OFF") {
       digitalWrite(RELAY_PIN, LOW);
      }
      

    }

  }

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret, retries = 5;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    retries--;

    if (retries == 0)
      halt("Resetting system");

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
