
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
void lightTwoOn();
void lightTwoOff();
void lightThreeOn();
void lightThreeOff();
void lightFourOn();
void lightFourOff();

// Change this before you flash
const char* ssid = "Gannavarapu";
const char* password = "Sravani1";

WidgetBridge bridge1(V1);
static bool Roof_Light = true;

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
int relayOne = D0;
int relayTwo = D1;
int relayThree = D2;
int relayFour = D3;

// Set Switches
int switchOne = D5;
int switchTwo = D6;
int switchThree = D7;
int switchFour = D9;
int switchFive = D10;

//Blynk
char auth[] = "56e72f2810374be6b7f45758a485b472";
#define VPIN V1;
#define VPIN V2;
#define VPIN V3;
#define VPIN V4;
BlynkTimer timer;

void blynkAnotherDevice() // Here we will send HIGH or LOW once per second
{
  Roof_Light = lightOneState;
  // Send value to another device
  if (Roof_Light) {
    // bridge1.digitalWrite(9, HIGH);  // Digital Pin 9 on the second board will be set HIGH
    bridge1.virtualWrite(V2, 1); // Sends 1 value to BLYNK_WRITE(V5) handler on receiving side.

    /////////////////////////////////////////////////////////////////////////////////////////
    // Keep in mind that when performing virtualWrite with Bridge,
    // second board will need to process the incoming command.
    // It can be done by using this handler on the second board:
    //
    //    BLYNK_WRITE(V1){
    //    int pinData = param.asInt(); // pinData variable will store value that came via Bridge
    //    }
    //
    /////////////////////////////////////////////////////////////////////////////////////////
  } else {
    //bridge1.digitalWrite(9, LOW); // Digital Pin 9 on the second board will be set LOW
    bridge1.virtualWrite(V2, 0); // Sends 0 value to BLYNK_WRITE(V5) handler on receiving side.
  }
  // Toggle value
  // Roof_Light = !Roof_Light;
}

BLYNK_CONNECTED() {
  bridge1.setAuthToken("4eb0b7b4c9a540a19e5c10ed338f784f"); // Place the AuthToken of the second hardware here
}



void setup()
{
  Serial.begin(115200);
  WiFiManager wifi;   //WiFiManager intialization.
  wifi.autoConnect("Hall"); //Create AP, if necessary

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
  // wemoManager.begin();
  //Set each relay pin to Low
  digitalWrite(relayOne, HIGH);
  delay(50);
  digitalWrite(relayTwo, HIGH);
  delay(50);
  digitalWrite(relayThree, HIGH);
  delay(50);
  digitalWrite(relayFour, HIGH);
  delay(50);
  wifiConnected = connectWifi();
  Serial.print("WiFi Connected");

  if (WiFi.status() == WL_CONNECTED) {
    wemoManager.begin();
    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    lightOne = new WemoSwitch("Hall Light", 84, lightOneOff, lightOneOn );
    lightTwo = new WemoSwitch("Hall Fan", 85, lightTwoOff, lightTwoOn);
    lightThree = new WemoSwitch("Hall Bedlamp", 86, lightThreeOff, lightThreeOn);
    lightFour = new WemoSwitch(" Roof Light", 87, lightFourOff, lightFourOn);
    wemoManager.addDevice(*lightOne);
    wemoManager.addDevice(*lightTwo);
    wemoManager.addDevice(*lightThree);
    wemoManager.addDevice(*lightFour);

    //Serial.println("Adding switches upnp broadcast responder");

    Blynk.config(auth);
    Blynk.connect();
    while (Blynk.connect() == false) {
      switchOneCheck();
      switchTwoCheck();
      switchThreeCheck();
      switchFourCheck();
    }
    delay(100);
    Blynk.syncVirtual(V1);
    Blynk.syncVirtual(V2);
    Blynk.syncVirtual(V3);
    Blynk.syncVirtual(V4);

    timer.setInterval(1000L, blynkAnotherDevice);
    timer.setInterval(11000L, CheckConnection);
    timer.setInterval(1000L, sendSensor);

    ArduinoOTA.setHostname("Hall");
    ArduinoOTA.begin();


  }
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

void sendSensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
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

  if (Blynk.connected()) {
    Blynk.run();
  }
  switchOneCheck();
  switchTwoCheck();
  switchThreeCheck();
  switchFourCheck();

  ArduinoOTA.handle();
  timer.run();
}
