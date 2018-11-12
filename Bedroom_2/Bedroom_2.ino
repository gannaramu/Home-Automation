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

char auth[] = "**"; //Get token from Blynk Bedroom1:  548aa0f4b76b4a018696e18af74c61dd
#define DHTPIN D4       // What digital pin we're connected to
#define DHTTYPE DHT11     // DHT 11
int localHum = 0;
int localTemp = 0;
int h = 0;
int t = 0;
DHT dht(DHTPIN, DHTTYPE);
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


int lightOneState = 0;
int lightTwoState = 0;
int lightThreeState = 0;
int lightFourState = 0;

int switchOneState = 0;
int switchTwoState = 0;
int switchThreeState = 0;
int switchFourState = 0;

bool Connected2Blynk = false;

int switchOne = D5;
int switchTwo = D6;
int switchThree = D7;
int switchFour = D9;


int relayOne = D0;
int relayTwo = D1;
int relayThree = D2;
int relayFour = D3;

WemoManager wemoManager;
WemoSwitch *Bedroomlight = NULL;
WemoSwitch *BedroomFan = NULL;
WemoSwitch *Bedlamp = NULL;
WemoSwitch *Radio = NULL;

const char* ssid = "***";
const char* password = "***";

void setup()
{
  Serial.begin(115200);
  WiFiManager wifiManager;
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(80);
  if (!wifiManager.autoConnect("Study Room")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  Bedroomlight = new WemoSwitch("Study Room Light", 91,  lightOneOff, lightOneOn);
  BedroomFan = new WemoSwitch("Study Room Fan", 92, lightTwoOff, lightTwoOn);
  Bedlamp = new WemoSwitch("Study Room Bed lamp", 93, lightThreeOff, lightThreeOn);
  Radio = new WemoSwitch("Balcony Light", 94, lightFourOff, lightFourOn);

  wemoManager.addDevice(*Bedroomlight);
  wemoManager.addDevice(*BedroomFan);
  wemoManager.addDevice(*Bedlamp);
  wemoManager.addDevice(*Radio);


  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  //Set switches to inputs
  pinMode(switchOne, INPUT_PULLUP);
  pinMode(switchTwo, INPUT_PULLUP);
  pinMode(switchThree, INPUT_PULLUP);
  pinMode(switchFour, INPUT_PULLUP);
  ArduinoOTA.setHostname("Bedroom ");
  ArduinoOTA.begin();

  Blynk.config(auth);

  Blynk.connect();


  timer.setInterval(1000L, sendSensor);
  timer.setInterval(100L, switchOneCheck);
  timer.setInterval(100L, switchTwoCheck);
  timer.setInterval(100L, switchThreeCheck);
  timer.setInterval(100L, switchFourCheck);
  timer.setInterval(11000L, CheckConnection);
}

void lightOneOn() {
  Serial.print("Switch 1 turn on ...");
  digitalWrite(relayOne, HIGH);   // sets relayOne on
  lightOneState = 1;
  Blynk.virtualWrite(V1, HIGH);
}

void lightOneOff() {
  Serial.print("Switch 1 turn off ...");
  digitalWrite(relayOne, LOW);   // sets relayOne off
  lightOneState = 0;
  Blynk.virtualWrite(V1, LOW);
}

void lightTwoOn() {
  Serial.print("Switch 2 turn on ...");
  digitalWrite(relayTwo, HIGH);   // sets relayTwo on
  lightTwoState = 1;
  Blynk.virtualWrite(V2, HIGH);
}

void lightTwoOff() {
  Serial.print("Switch 2 turn off ...");
  digitalWrite(relayTwo, LOW);   // sets relayTwo Off
  lightTwoState = 0;
  Blynk.virtualWrite(V2, LOW);
}

void lightThreeOn() {
  Serial.print("Socket 1 turn on ...");
  digitalWrite(relayThree, HIGH);   // sets relayThree on
  lightThreeState = 1;
  Blynk.virtualWrite(V3, HIGH);
}

void lightThreeOff() {
  Serial.print("Socket 1 turn off ...");
  digitalWrite(relayThree, LOW);   // sets relayThree off
  lightThreeState = 0;
  Blynk.virtualWrite(V3, LOW);
}

void lightFourOn() {
  Serial.print("Socket 2 turn on ...");
  digitalWrite(relayFour, HIGH);   // sets relayFour on
  lightFourState = 1;
  Blynk.virtualWrite(V4, HIGH);
}

void lightFourOff() {
  Serial.print("Socket 2 turn off ...");
  digitalWrite(relayFour, LOW);   // sets relayFour off
  lightFourState = 0;
  Blynk.virtualWrite(V4, LOW);
}

void switchOneCheck() {
  if (switchOneState != digitalRead(switchOne)) {
    if (lightOneState == 1) {
      lightOneOff();
    }
    else {
      lightOneOn();
    }
    switchOneState = digitalRead(switchOne);
  }
}

void switchTwoCheck() {
  if (switchTwoState != digitalRead(switchTwo)) {
    if (lightTwoState == 1) {
      lightTwoOff();
    }
    else {
      lightTwoOn();
    }
    switchTwoState = digitalRead(switchTwo);
  }
}

void switchThreeCheck() {
  if (switchThreeState != digitalRead(switchThree)) {
    if (lightThreeState == 1) {
      lightThreeOff();
    }
    else {
      lightThreeOn();
    }
    switchThreeState = digitalRead(switchThree);
  }
}

void switchFourCheck() {
  if (switchFourState != digitalRead(switchFour)) {
    if (lightFourState == 1) {
      lightFourOff();
    }
    else {
      lightFourOn();
    }
    switchFourState = digitalRead(switchFour);
  }
}


BLYNK_WRITE(V1) {
  int blynkSwitch = param.asInt();
  if (blynkSwitch) {
    lightOneOn();
  }
  else {
    lightOneOff();
  }
}
BLYNK_WRITE(V2) {
  int blynkSwitch = param.asInt();
  if (blynkSwitch) {
    lightTwoOn();
  }
  else {
    lightTwoOff();
  }
}
BLYNK_WRITE(V3) {
  int blynkSwitch = param.asInt();
  if (blynkSwitch) {
    lightThreeOn();
  }
  else {
    lightThreeOff();
  }
}
BLYNK_WRITE(V4) {
  int blynkSwitch = param.asInt();
  if (blynkSwitch) {
    lightFourOn();
  }
  else {
    lightFourOff();
  }
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi() {
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //delay(1000);
  }
  state = true;

  return state;
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

void sendSensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else if (h > 100 || t > 100) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V6, h);
  Blynk.virtualWrite(V7, t);
  return;
}
void loop()
{
  wemoManager.serverLoop();
  Blynk.run();

  //  if (!Blynk.connected()) {
  //    switchOneCheck();
  //    switchTwoCheck();
  //    switchThreeCheck();
  //    switchFourCheck();
  //  }
  ArduinoOTA.handle();
  timer.run();
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


