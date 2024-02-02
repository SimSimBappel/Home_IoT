#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"
#include "light_control.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqtt_server = MQTT_IP;

WiFiClient espClient;
IPAddress local_IP(192, 168, 1, 204);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

PubSubClient client(espClient);

const int ledPin = 2;
const int red_led = 16; 
const int green_led = 17;
const int blue_led = 18; 
const int PIR = 35;
const int photoresistor = 33; //need to use ADC1 pins ONLY

unsigned long timer1 = 0;
unsigned long timer2 = 0;
const int waitformotion = 30000; 
bool light_state = false;
bool PIR_state = false;

bool fading = false;

void setup_wifi();
void connect_mqttServer();
void callback(char*, byte*, unsigned int);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(blue_led, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(photoresistor, INPUT_PULLDOWN);

  digitalWrite(red_led, LOW);
  digitalWrite(green_led, LOW);
  digitalWrite(blue_led, LOW);
 

  setup_wifi();
  ArduinoOTA.setPassword("admin"); 
  ArduinoOTA.begin();
  client.setServer(mqtt_server,1883); 
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
  }
  client.loop();
  ArduinoOTA.handle();

  //reset timers at rollover
  if(timer1 > millis()){
    timer1 = 0;
  }
  if(timer2 > millis()){
    timer2 = 0;
  }

  int analogValue = analogRead(photoresistor);
  char msg[10];
  sprintf(msg, "%d", analogValue);
  client.publish("koekken/LDRsensor", msg);

  if(digitalRead(PIR)){
    timer2 = millis();
    if(!PIR_state){
      client.publish("koekken/PIRsensor", "motion_detected");
      PIR_state = true;
    }
  }
  else if(PIR_state && millis() - timer2 > waitformotion){
    PIR_state = false;
    client.publish("koekken/PIRsensor", "no motion (30s)");
  }


  if(fading){
    light_feedback result = fade_light(light_state, 3.0, red_led, green_led, blue_led);
    light_state = result.light_state;
    fading = result.fading;
  }
  else if(digitalRead(PIR) && analogRead(photoresistor) < 30){
    fading = true;
    // Serial.println("turn on");
    timer1 = millis();
  }
  else if(light_state && digitalRead(PIR)){
    timer1 = millis();
  }
  else if(millis() - timer1 > waitformotion && light_state){
    fading = true;
    // Serial.println("turn off");
  }
  else{
    delay(100); //TODO microcontroller sleep
  }

}





void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    blink_led(ledPin, 2, 200); 
    delay(1000);
    Serial.print(".");
    c=c+1;
    if(c>10){
        ESP.restart(); 
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
}

void connect_mqttServer() {
  while (!client.connected()) {
        if(WiFi.status() != WL_CONNECTED){
          //if not connected, then first connect to wifi
          setup_wifi();
        }

        Serial.print("Attempting MQTT connection...");

        if (client.connect("koekken_client")) { // Change the name of client here if multiple ESP32 are connected
          //attempt successful
          Serial.println("connected");
          client.subscribe("koekken/set_light");
        } 
        else {
          //attempt not successful
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" trying again in 2 seconds");
    
          blink_led(ledPin, 3, 200); //blink to show that MQTT server connection attempt failed
          // Wait 2 seconds before retrying
          delay(2000);
        }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  int intValue = messageTemp.toInt();

  Serial.println(intValue);

  switch (intValue) {
    case 1:
      Serial.println("light on");
      digitalWrite(ledPin, HIGH);
      light_state = switch_light(true, red_led, green_led, blue_led);
      client.publish("koekken/light","light on");
      break;

    case 0:
      Serial.println("light off");
      digitalWrite(ledPin, LOW);
      light_state = switch_light(false, red_led, green_led, blue_led);
      client.publish("koekken/light","light off");
      break;

    default:
      blink_led(ledPin, 3, 300);
      Serial.println("wrong value");
      client.publish("koekken/error", "1");
      break;
  }
}