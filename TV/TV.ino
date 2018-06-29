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
int localHum = 0;
int localTemp = 0;
int h = 0;
int t = 0;
DHT dht(DHTPIN, DHTTYPE);
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


// Change this before you flash
const char* ssid = "****";
const char* password = "***";

boolean wifiConnected = false;

bool Connected2Blynk = false;
int lightOneState = 0;


int switchOneState = 0;


WemoManager wemoManager;
WemoSwitch *lightOne = NULL;


// Set Relay Pins
int relayOne = D5;
// Set Switches
int switchOne = D1;


//Blynk
char auth[] = "e5d5399a1751496da31b07bfbe170425";
#define VPIN V1;

BlynkTimer timer;
void setup()
{
  Serial.begin(115200);
  WiFiManager wifi;   //WiFiManager intialization.
  wifi.autoConnect("TV"); //Create AP, if necessary

  //Set relay pins to outputs
  pinMode(relayOne, OUTPUT);


  //Set switches to inputs
  pinMode(switchOne, INPUT_PULLUP);

  // wemoManager.begin();
  //Set each relay pin to Low
  digitalWrite(relayOne, HIGH);
  delay(50);

  // wifiConnected = connectWifi();


  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi Connected");
    wemoManager.begin();
    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    lightOne = new WemoSwitch("Tv", 101, lightOneOff, lightOneOn );
    wemoManager.addDevice(*lightOne);


    //Serial.println("Adding switches upnp broadcast responder");

    Blynk.config(auth);
    Blynk.connect();
    while (Blynk.connect() == false) {
      switchOneCheck();

    }
    delay(100);
    Blynk.syncVirtual(V1);


    ArduinoOTA.setHostname("TV");
    ArduinoOTA.begin();
  }
}

void loop()
{
  wemoManager.serverLoop();

  if (Blynk.connected()) {
    Blynk.run();
  }
  switchOneCheck();
  timer.setInterval(11000L, CheckConnection);
  ArduinoOTA.handle();
  timer.run();
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

BLYNK_WRITE(V1) {
  int blynkSwitch = param.asInt();
  if (blynkSwitch) {
    lightOneOn();
  }
  else {
    lightOneOff();
  }
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi() {
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
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
