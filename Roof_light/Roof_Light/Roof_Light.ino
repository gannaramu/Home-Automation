/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Social networks:            http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <SimpleTimer.h>
#include "WemoSwitch.h"
#include "WemoManager.h"
#include "CallbackFunction.h"
#define VPIN V2  //Use a unique virtual pin for each device using the same token / dashboard

char auth[] = "******"; //Get token from Blynk

//on/off callbacks
void lightOn();
void lightOff();

WemoManager wemoManager;
WemoSwitch *light = NULL;

boolean LampState = 0;
boolean SwitchReset = true;   //Flag indicating that the hardware button has been released
int switchState = 0;

const int RelayPin = D4;      //Relay switching pin. Relay is pin 12 on the SonOff

const int RedPin1 = D0;
const int RedPin2 = D5;

const int GreenPin1 = D1;
const int GreenPin2 = D6;

const int BluePin1 = D2;
const int BluePin2 = D7;

int RedFadeLvl;
int GreenFadeLvl;
int BlueFadeLvl;


SimpleTimer timer;

int Red = 0;
int Green = 0;
int Blue = 0;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "****";
char pass[] = "*****";


BLYNK_WRITE(V4) // zeRGBa assigned to V4
{
  // get a RED channel value
  Red = param[0].asInt();
  // get a GREEN channel value
  Green = param[1].asInt();
  // get a BLUE channel value
  Blue = param[2].asInt();

  if (LampState)
  { analogWrite(RedPin1, Red);
    analogWrite(GreenPin1, Green);
    analogWrite(BluePin1, Blue);

    analogWrite(RedPin2, Red);
    analogWrite(GreenPin2, Green);
    analogWrite(BluePin2, Blue);
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(RelayPin, OUTPUT);
  WiFiManager wifi;   //WiFiManager intialization.
  wifi.autoConnect("Roof Light"); //Create AP, if necessary

  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  light = new WemoSwitch("Roof Light", 102, lightOn, lightOff);
  wemoManager.addDevice(*light);

  pinMode(RelayPin, OUTPUT);
  //  pinMode(LED, OUTPUT);
  //pinMode(TacSwitch, INPUT_PULLUP);
  delay(10);
  digitalWrite(RelayPin, LOW);
  //  digitalWrite(LED, LOW);

  ArduinoOTA.setHostname("Roof Light");
  Blynk.config(auth);
  ArduinoOTA.begin();

  Blynk.virtualWrite(VPIN, HIGH);
  Blynk.syncVirtual(VPIN);

}

BLYNK_WRITE(VPIN) {
  if (switchState !=  param.asInt()) {
    if (LampState == 1) {
      lightOff();
    }
    else {
      lightOn();
    }
    switchState = param.asInt();
  }
}


void FadeLEDUP()
{
  analogWrite(RedPin1, RedFadeLvl);  // LED on pin 6
  analogWrite(GreenPin1, GreenFadeLvl);
  analogWrite(BluePin1, BlueFadeLvl);

  analogWrite(RedPin2, RedFadeLvl);  // LED on pin 6
  analogWrite(GreenPin2, GreenFadeLvl);
  analogWrite(BluePin2, BlueFadeLvl);
  RedFadeLvl++;
  GreenFadeLvl++;
  BlueFadeLvl++;
}

void FadeLEDDOWN()
{
  analogWrite(RedPin1, RedFadeLvl);
  analogWrite(GreenPin1, GreenFadeLvl);
  analogWrite(BluePin1, BlueFadeLvl);

  analogWrite(RedPin2, RedFadeLvl);
  analogWrite(GreenPin2, GreenFadeLvl);
  analogWrite(BluePin2, BlueFadeLvl);


  RedFadeLvl--;
  GreenFadeLvl--;
  BlueFadeLvl--;
}


void lightOn() {


  Serial.println("Switch 1 turn on ...");
  digitalWrite(RelayPin, HIGH);
  // Setting it to a specific colour.
  RedFadeLvl=1023;
  GreenFadeLvl=1023;
  BlueFadeLvl=1023;
  timer.setTimer(10, FadeLEDUP, 1023);

  LampState = 1;
  Blynk.virtualWrite(VPIN, HIGH);     // Sync the Blynk button widget state
}



void lightOff() {
  digitalWrite(RelayPin, LOW);

  Serial.println("Switch 1 turn off ...");

  RedFadeLvl=1023;
  GreenFadeLvl=1023;
  BlueFadeLvl=1023;
  
  timer.setTimer(15, FadeLEDDOWN, 1023);
  LampState = 0;
  Blynk.virtualWrite(VPIN, LOW);      // Sync the Blynk button widget state
}



void ToggleRelay() {
  LampState = !LampState;

  if (LampState) {
    lightOn();
  }
  else lightOff();
}


void loop()
{
  wemoManager.serverLoop();
  Blynk.run();
  ArduinoOTA.handle();

}
