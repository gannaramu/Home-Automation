#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include <BlynkSimpleEsp8266.h>

#include <SPI.h>
#include <DHT.h>
#define DHTPIN D4       // What digital pin we're connected to
#define DHTTYPE DHT11     // DHT 11
#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"

#include <ArduinoOTA.h>
#include <SimpleTimer.h>
// prototypes
boolean connectWifi();

//on/off callbacks
void lightOneOn();
void lightOneOff();
void lightTwoOn();
void lightTwoOff();
void lightThreeOn();
void lightThreeOff();
void lightFourOn();
void lightFourOff();

// Change this before you flash
const char* ssid = "Gannavarapu";
const char* password = "Sravani1";

//WidgetBridge bridge1(V1);
//static bool Roof_Light = true;
int localHum = 0;
int localTemp = 0;
int h = 0;
int t = 0;
DHT dht(DHTPIN, DHTTYPE);

boolean wifiConnected = false;

bool Connected2Blynk = false;
int lightOneState = 0;
int lightTwoState = 0;
int lightThreeState = 0;
int lightFourState = 0;

int switchOneState = 0;
int switchTwoState = 0;
int switchThreeState = 0;
int switchFourState = 0;

WemoManager wemoManager;
WemoSwitch *lightOne = NULL;
WemoSwitch *lightTwo = NULL;
WemoSwitch *lightThree = NULL;
WemoSwitch *lightFour = NULL;

// Set Relay Pins
int relayOne = D2;
int relayTwo = D1;
int relayThree = D0;
int relayFour = D3;

// Set Switches
int switchOne = D9;
int switchTwo = D5;
int switchThree = D6;
int switchFour = D7;


//Blynk
char auth[] = "7e957acad3224ed8a0913c637265a7f2";
#define VPIN V1;
#define VPIN V2;
#define VPIN V3;
#define VPIN V4;
BlynkTimer timer;



void setup()
{

  Serial.begin(115200);
  WiFiManager wifiManager;
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout
  (180);
  if (!wifiManager.autoConnect("Bedroom")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  //Set relay pins to outputs
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  //Set switches to inputs
  pinMode(switchOne, INPUT_PULLUP);
  pinMode(switchTwo, INPUT_PULLUP);
  pinMode(switchThree, INPUT_PULLUP);
  pinMode(switchFour, INPUT_PULLUP);

  //Set each relay pin to Low
  digitalWrite(relayOne, HIGH);
  digitalWrite(relayTwo, HIGH);
  digitalWrite(relayThree, HIGH);
  digitalWrite(relayFour, HIGH);
  //wifiConnected = connectWifi();
  Serial.print("WiFi Connected");

  if (WiFi.status() == WL_CONNECTED) {
    // wemoManager.begin();
    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    lightOne = new WemoSwitch("Bedroom Light", 80,  lightOneOff, lightOneOn);
    lightTwo = new WemoSwitch("Bedroom Fan", 81, lightTwoOff, lightTwoOn);
    lightThree = new WemoSwitch("Bed lamp", 82, lightThreeOff, lightThreeOn);
    lightFour = new WemoSwitch("Radio", 83, lightFourOff, lightFourOn);
    wemoManager.addDevice(*lightOne);
    wemoManager.addDevice(*lightTwo);
    wemoManager.addDevice(*lightThree);
    wemoManager.addDevice(*lightFour);

    ArduinoOTA.setHostname("Bedroom");
    ArduinoOTA.begin();
    //Serial.println("Adding switches upnp broadcast responder");

    Blynk.config(auth);
    Blynk.connect();
    while (Blynk.connect() == false) {
      switchOneCheck();
      switchTwoCheck();
      switchThreeCheck();
      switchFourCheck();
    }
    Blynk.syncVirtual(V1);
    Blynk.syncVirtual(V2);
    Blynk.syncVirtual(V3);
    Blynk.syncVirtual(V4);

    //timer.setInterval(1000L, blynkAnotherDevice); // modified on 25-12-17
    //    timer.setInterval(11000L, CheckConnection);
    timer.setInterval(1000L, sendSensor);
    timer.setInterval(100L, switchOneCheck);
    timer.setInterval(100L, switchTwoCheck);
    timer.setInterval(100L, switchThreeCheck);
    timer.setInterval(100L, switchFourCheck);

    ArduinoOTA.setHostname("Hall");
    ArduinoOTA.begin();


  }
  timer.setInterval(1000L, sendSensor);
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

BLYNK_WRITE(V12)
{
  // You'll get uptime value here as result of syncAll()
  int uptime = param.asInt();
}

