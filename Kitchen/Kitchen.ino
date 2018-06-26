/*

   Witnessmenow's ESP8266 Wemo Emulator library is an awesome way to build an IoT device on
   the cheap, and its direct integration with Alexa / Echo means that voice control can
   be done with natural sounding commands. However, it is limited by its reliance solely on
   voice control, as it doesnt work with the official Wemo app. It also provided no affordance
   for toggling the switch when an internet connection was unavailable.

   With just a bit of additional code, devices can be made controllable by the Blynk app, hardware
   switches, IFTTT event triggers,and of course, Alexa. Toss in OTA updates and Tzapulica's
   WiFiManager for easy provisioning, and you've got a really versatile, easy to use device.


   OTA updates are hardware dependent, but don't seem to cause any problems for devices
   that don't support it.

   Wemo Emulator and WiFi Manager libraries:
   https://github.com/witnessmenow/esp8266-alexa-wemo-emulator
   https://github.com/tzapu/WiFiManager

   In order to control a multitude of devices with a single Blynk dashboard, each ESP8266
   should be programmed with a unique virtual pin assignment, corresponding to a Blynk switch.

   The onboard LED is set to ON when the relay is off. This made sense to me, if you're looking
   for the physical switch in a dark room.

   For IFTTT control, use the Maker Channel with the following settings:
      URL: http://blynk-cloud.com:8080/YOUR_TOKEN/V1       Substitute your own token and vitual pin
      Method: PUT
      Content type: application/json
      Body: ["1"]                                          Use 1 for ON, 0 for OFF
*/
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>
/* Comment this out to disable prints and save space */

#include <SPI.h>
#include <DHT.h>

#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

#define VPIN V1  //Use a unique virtual pin for each device using the same token / dashboard
#define VPIN1 V2  //Use a unique virtual pin for each device using the same token / dashboard
#define VPIN2 V3  //Use a unique virtual pin for each device using the same token / dashboard
#define VPIN3 V4  //Use a unique virtual pin for each device using the same token / dashboard

char auth[] = "41bd986748e44d5da4fd6817a6bee869"; //Get token from Blynk Bedroom1:  548aa0f4b76b4a018696e18af74c61dd

int localHum = 0;
int localTemp = 0;
int h = 0;
int t = 0;
//DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;


//on/off callbacks
void lightOneOn();
void lightTwoOn();
void lightThreeOn();
void lightFourOn();
void lightOneOff();
void lightTwoOff();
void lightThreeOff();
void lightFourOff();

boolean lightState1 = 0;
boolean lightState2 = 0;
boolean lightState3 = 0;
boolean lightState4 = 0;

bool Connected2Blynk = false;

boolean SwitchReset = true;   //Flag indicating that the hardware button has been released
boolean SwitchReset1 = true;
boolean SwitchReset2 = true;
boolean SwitchReset3 = true;

const int TacSwitch1 = D5;      //Pin for hardware momentary switch. On when grounded. Pin 0 on Sonoff
const int TacSwitch12 = D6;
const int TacSwitch13 = D7;
const int TacSwitch14 = D9;


const int LightPin1 = D0;      //Relay switching pin. Relay is pin 12 on the SonOff
const int LightPin2 = D1;      //Relay switching pin. Relay is pin 12 on the SonOff
const int LightPin3 = D2;
const int LightPin4 = D3;

WemoManager wemoManager;
WemoSwitch *KitchenLight = NULL;
WemoSwitch *KitchenMini = NULL;
WemoSwitch *Balcony = NULL;

void setup()
{
  Serial.begin(115200);
  //  display.init();
  //  display.flipScreenVertically();
  WiFiManager wifiManager;
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(80);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Kitchen")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  KitchenLight = new WemoSwitch("Kitchen Light", 95,  lightOneOff, lightOneOn);
  KitchenMini = new WemoSwitch("Kithchen Mini Light", 96, lightTwoOff, lightTwoOn);
  Balcony = new WemoSwitch("Balcony Light", 97, lightThreeOff, lightThreeOn);

  wemoManager.addDevice(*KitchenLight);
  wemoManager.addDevice(*KitchenMini);
  wemoManager.addDevice(*Balcony);


  pinMode(LightPin1, OUTPUT);
  pinMode(LightPin2, OUTPUT);
  pinMode(LightPin3, OUTPUT);
  pinMode(LightPin4, OUTPUT);
  pinMode(TacSwitch1, INPUT_PULLUP);
  pinMode(TacSwitch12, INPUT_PULLUP);
  pinMode(TacSwitch13, INPUT_PULLUP);
  pinMode(TacSwitch14, INPUT_PULLUP);
  delay(10);
  digitalWrite(LightPin1, HIGH);
  digitalWrite(LightPin2, HIGH);
  digitalWrite(LightPin3, HIGH);
  digitalWrite(LightPin4, HIGH);
  ArduinoOTA.setHostname("Kitchen");
  Blynk.config(auth);
  ArduinoOTA.begin();
  Blynk.connect();
  Blynk.syncVirtual(VPIN);
  Blynk.syncVirtual(VPIN1);
  Blynk.syncVirtual(VPIN2);
  Blynk.syncVirtual(VPIN3);

  timer.setInterval(100, ButtonCheck);
  timer.setInterval(100, ButtonCheck1);
  timer.setInterval(100, ButtonCheck2);
  timer.setInterval(100, ButtonCheck3);
  timer.setInterval(11000L, CheckConnection);
}

void loop()
{

  wemoManager.serverLoop();
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}

// Toggle the relay on
void lightOneOn() {
  Serial.println("Switch 1 turn on ...");
  digitalWrite(LightPin1, HIGH);
  lightState1 = 1;
  Blynk.virtualWrite(VPIN, HIGH);     // Sync the Blynk button widget state
}
void lightTwoOn() {
  Serial.println("Switch 2 turn on ...");
  digitalWrite(LightPin2, HIGH);
  lightState2 = 1;
  Blynk.virtualWrite(VPIN1, HIGH);     // Sync the Blynk button widget state
}
void lightThreeOn() {
  Serial.println("Switch 3 turn on ...");
  digitalWrite(LightPin3, HIGH);
  lightState3 = 1;
  Blynk.virtualWrite(VPIN2, HIGH);     // Sync the Blynk button widget state
}

void lightFourOn() {
  Serial.println("Switch 4 turn on ...");
  digitalWrite(LightPin4, HIGH);
  lightState4 = 1;
  Blynk.virtualWrite(VPIN3, HIGH);     // Sync the Blynk button widget state
}


// Toggle the relay off
void lightOneOff() {
  Serial.println("Switch 1 turn off ...");
  digitalWrite(LightPin1, LOW);
  lightState1 = 0;
  Blynk.virtualWrite(VPIN, LOW);      // Sync the Blynk button widget state
}

void lightTwoOff() {
  Serial.println("Switch 2 turn off ...");
  digitalWrite(LightPin2, LOW);
  lightState2 = 0;
  Blynk.virtualWrite(VPIN1, LOW);     // Sync the Blynk button widget state
}

void lightThreeOff() {
  Serial.println("Switch 3 turn off ...");
  digitalWrite(LightPin3, LOW);
  lightState3 = 0;
  Blynk.virtualWrite(VPIN2, LOW);     // Sync the Blynk button widget state
}


void lightFourOff() {
  Serial.println("Switch 4 turn off ...");
  digitalWrite(LightPin4, LOW);
  lightState4 = 0;
  Blynk.virtualWrite(VPIN3, LOW);     // Sync the Blynk button widget state
}


// Handle switch changes originating on the Blynk app
BLYNK_WRITE(VPIN) {
  int SwitchStatus = param.asInt();

  Serial.println("Blynk switch activated");

  // For use with IFTTT, toggle the relay by sending a "2"
  if (SwitchStatus == 2) {
    ToggleRelay1();
  }
  else if (SwitchStatus) {
    lightOneOn();
  }
  else lightOneOff();
}
// Handle switch changes originating on the Blynk app
BLYNK_WRITE(VPIN1) {
  int SwitchStatus1 = param.asInt();

  Serial.println("Blynk switch activated");

  // For use with IFTTT, toggle the relay by sending a "2"
  if (SwitchStatus1 == 2) {
    //    ToggleRelay();
  }
  else if (SwitchStatus1) {
    lightTwoOn();
  }
  else lightTwoOff();
}

BLYNK_WRITE(VPIN2) {
  int SwitchStatus2 = param.asInt();

  Serial.println("Blynk switch activated");

  // For use with IFTTT, toggle the relay by sending a "2"
  if (SwitchStatus2 == 2) {
    //    ToggleRelay();
  }
  else if (SwitchStatus2) {
    lightThreeOn();
  }
  else lightThreeOff();
}
BLYNK_WRITE(VPIN3) {
  int SwitchStatus3 = param.asInt();

  Serial.println("Blynk switch activated");

  // For use with IFTTT, toggle the relay by sending a "2"
  if (SwitchStatus3 == 2) {
    //    ToggleRelay();
  }
  else if (SwitchStatus3) {
    lightFourOn();
  }
  else lightFourOff();
}




// Handle hardware switch activation
void ButtonCheck() {
  // look for new button press
  boolean SwitchState = (digitalRead(TacSwitch1));

  // toggle the switch if there's a new button press
  if (SwitchState == LOW && SwitchReset == true) {
    Serial.println("Hardware switch activated");

    ToggleRelay1();

    SwitchReset = false;  // Flag that indicates the physical button hasn't been released
    delay(50);            //debounce
  }
  else if (SwitchState) {
    SwitchReset = true;   // reset flag the physical button release
  }
}

void ButtonCheck1() {
  // look for new button press
  boolean SwitchState1 = (digitalRead(TacSwitch12));

  // toggle the switch if there's a new button press
  if (SwitchState1 == LOW && SwitchReset1 == true) {
    Serial.println("Hardware switch activated");
    ToggleRelay2();
    SwitchReset1 = false;  // Flag that indicates the physical button hasn't been released
    delay(50);            //debounce
  }
  else if (SwitchState1) {
    SwitchReset1 = true;   // reset flag the physical button release
  }
}

void ButtonCheck2() {
  // look for new button press
  boolean SwitchState2 = (digitalRead(TacSwitch13));

  // toggle the switch if there's a new button press
  if (SwitchState2 == LOW && SwitchReset2 == true) {
    Serial.println("Hardware switch activated");

    ToggleRelay3();

    SwitchReset2 = false;  // Flag that indicates the physical button hasn't been released
    delay(50);            //debounce
  }
  else if (SwitchState2) {
    SwitchReset2 = true;   // reset flag the physical button release
  }
}

void ButtonCheck3() {
  // look for new button press
  boolean SwitchState3 = (digitalRead(TacSwitch14));

  // toggle the switch if there's a new button press
  if (SwitchState3 == LOW && SwitchReset3 == true) {
    Serial.println("Hardware switch activated");

    ToggleRelay4();

    SwitchReset3 = false;  // Flag that indicates the physical button hasn't been released
    delay(50);            //debounce
  }
  else if (SwitchState3) {
    SwitchReset3 = true;   // reset flag the physical button release
  }
}

void ToggleRelay1() {
  lightState1 = !lightState1;

  if (lightState1) {
    lightOneOn();
  }
  else lightOneOff();
}

void ToggleRelay2() {
  lightState2 = !lightState2;

  if (lightState2) {
    lightTwoOn();
  }
  else lightTwoOff();
}

void ToggleRelay3() {
  lightState3 = !lightState3;

  if (lightState3) {
    lightThreeOn();
  }
  else lightThreeOff();
}

void ToggleRelay4() {
  lightState4 = !lightState4;

  if (lightState4) {
    lightFourOn();
  }
  else lightFourOff();
}


void CheckConnection() {
  Connected2Blynk = Blynk.connected();
  if (!Connected2Blynk) {
    Serial.println("Not connected to Blynk server");
    if (WiFi.status() == WL_CONNECTED)
    {

      Blynk.connect();
    }
    else {
      Serial.println("Still connected to Blynk server");
    }
  }
}


BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();

  // You can also update individual virtual pins like this:
  //Blynk.syncVirtual(V0, V2);

  // Let's write your hardware uptime to Virtual Pin 2
  int value = millis() / 1000;
  Blynk.virtualWrite(V12, value);
}
