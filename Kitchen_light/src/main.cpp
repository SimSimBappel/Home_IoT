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
const int red_led = 19; //need to use ADC1 pins ONLY
const int green_led = 18; //need to use ADC1 pins ONLY
const int blue_led = 5; //need to use ADC1 pins ONLY
const int PIR = 21;
const int photoresistor = 34; //need to use ADC1 pins ONLY

unsigned long timer = 0;
unsigned long timer2 = 0;
int waitformotion = 10000;
bool light_state = false;
bool PIR_state = false;


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

  Serial.print(digitalRead(PIR));
  Serial.print(" ");
  Serial.println(analogRead(photoresistor));

  if(digitalRead(PIR)){
    timer2 = millis();
    if(!PIR_state){
      client.publish("koekken/motionsensor", "motion_detected");
      PIR_state = true;
    }
  }
  else if(!digitalRead(PIR) && PIR_state && millis() - timer2 > waitformotion){
    PIR_state = false;
    client.publish("koekken/motionsensor", "no motion (30s)");
  }

  if(digitalRead(PIR) && analogRead(photoresistor) < 100){
    light_state = fade_light(true, light_state, 3.0, red_led, green_led, blue_led);
    Serial.println("turn on");
    timer = millis();
  }
  else if(millis() - timer > waitformotion && light_state){
    // fade_light(false, 3.0);
    Serial.println("turn off");
    light_state = fade_light(false, light_state, 3.0, red_led, green_led, blue_led);
  }
  else{
    delay(100); //microcontroller sleep
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
    case 100:
      Serial.println("light on");
      digitalWrite(ledPin, HIGH);
      // fade_light(true, 0.0);
      fade_light(true, light_state, 3.0, red_led, green_led, blue_led);
      client.publish("koekken/light","light on");
      break;

    case 0:
      Serial.println("light off");
      digitalWrite(ledPin, LOW);
      // fade_light(false, 0.0);
      fade_light(false, light_state, 3.0, red_led, green_led, blue_led);
      client.publish("koekken/light","light off");
      break;

    case 25:
      // Serial.println("open blinds");
      client.publish("koekken/","25");
      break;

    case 1:
      // Serial.println("close blinds");
      client.publish("koekken/","1");
      break;

    default:
      blink_led(ledPin, 3, 300);
      Serial.println("wrong value");
      client.publish("koekken/error", "1");
      break;
  }
}