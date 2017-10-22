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

char auth[] = "7e957acad3224ed8a0913c637265a7f2"; //Get token from Blynk Bedroom1:  548aa0f4b76b4a018696e18af74c61dd
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

boolean lightState1 = 0;
boolean lightState2 = 0;
boolean lightState3 = 0;
boolean lightState4 = 0;

bool Connected2Blynk = false;

boolean SwitchReset = true;   //Flag indicating that the hardware button has been released
boolean SwitchReset1 = true;
boolean SwitchReset2 = true;
boolean SwitchReset3 = true;

const int TacSwitch1 = D9;      //Pin for hardware momentary switch. On when grounded. Pin 0 on Sonoff
const int TacSwitch12 = D5;
const int TacSwitch13 = D6;
const int TacSwitch14 = D7;


const int LightPin1 = D2;      //Relay switching pin. Relay is pin 12 on the SonOff
const int LightPin2 = D1;      //Relay switching pin. Relay is pin 12 on the SonOff
const int LightPin3 = D0;
const int LightPin4 = D3;

WemoManager wemoManager;
WemoSwitch *Bedroomlight = NULL;
WemoSwitch *BedroomFan = NULL;
WemoSwitch *Bedlamp = NULL;
WemoSwitch *Radio = NULL;

void setup()
{
  Serial.begin(115200);
  //  display.init();
  //  display.flipScreenVertically();
  WiFiManager wifi;   //WiFiManager intialization.
  wifi.autoConnect("Bedroom 1"); //Create AP, if necessary

  wemoManager.begin();
  // Format: Alexa invocation name, local port no, on callback, off callback
  Bedroomlight = new WemoSwitch("Bedroom Light", 80,  lightOneOff,lightOneOn);
  BedroomFan = new WemoSwitch("Bedroom Fan", 81, lightTwoOff, lightTwoOn);
  Bedlamp = new WemoSwitch("Bed lamp", 82, lightThreeOff, lightThreeOn);
  Radio = new WemoSwitch("Radio", 83, lightFourOff, lightFourOn);
  wemoManager.addDevice(*Bedroomlight);
  wemoManager.addDevice(*BedroomFan);
  wemoManager.addDevice(*Bedlamp);
  wemoManager.addDevice(*Radio);


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
  ArduinoOTA.setHostname("Bedroom1");
  Blynk.config(auth);
  ArduinoOTA.begin();
  Blynk.connect();
  Blynk.syncVirtual(VPIN);
  Blynk.syncVirtual(VPIN1);
  Blynk.syncVirtual(VPIN2);
  Blynk.syncVirtual(VPIN3);

  timer.setInterval(1000L, sendSensor);
  timer.setInterval(100, ButtonCheck);
  timer.setInterval(100, ButtonCheck1);
  timer.setInterval(100, ButtonCheck2);
  timer.setInterval(100, ButtonCheck3);
  timer.setInterval(11000L, CheckConnection);
}

void loop()
{

  wemoManager.serverLoop();
  if (Blynk.connected()) {
    Blynk.run();
  }
  else if (!Blynk.connected()) {
    ButtonCheck();
    ButtonCheck1();
    ButtonCheck2();
    ButtonCheck3();

  }
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

