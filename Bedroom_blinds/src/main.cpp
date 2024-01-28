#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqtt_server = MQTT_IP;

WiFiClient espClient;
IPAddress local_IP(192, 168, 1, 203);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

PubSubClient client(espClient);

bool motion = false;
long lastMsg = 0;
#define ledPin 2

Servo myservo;
const int open_pos = 180;
const int neutral_pos = 90;
const int close_pos = 0;
const int swing_time = 400;

void blink_led(unsigned int, unsigned int);
void setup_wifi();
void connect_mqttServer();
void callback(char*, byte*, unsigned int);


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(15, INPUT_PULLDOWN);
  Serial.begin(115200);
  
  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(12);
  
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
  if(digitalRead(15)){
    motion = true;
    client.publish("sove/motionsensor", "motion Detected!"); 
    Serial.println("published motion!");
  }

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
  }
  delay(200); 
}



void blink_led(unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW); 
    delay(200);
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
    blink_led(2,200); 
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

        if (client.connect("bedroom_client")) { // Change the name of client here if multiple ESP32 are connected
          //attempt successful
          Serial.println("connected");
          client.subscribe("sove/blind_pos");
        } 
        else {
          //attempt not successful
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" trying again in 2 seconds");
    
          blink_led(3,200); //blink to show that MQTT server connection attempt failed
          // Wait 2 seconds before retrying
          delay(2000);
        }
  }
}

//this function will be executed whenever there is data available on subscribed topics
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
    case 12:
      Serial.println("light on");
      digitalWrite(ledPin, HIGH);
      client.publish("sove/light","light on");
      break;

    case 21:
      Serial.println("light off");
      digitalWrite(ledPin, LOW);
      client.publish("sove/light","light off");
      break;

    case 25:
      // Serial.println("open blinds");
      client.publish("sove/get_blind_pos","25");
      digitalWrite(13, HIGH);
      myservo.write(open_pos);
      delay(1500);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(13, LOW);
      break;

    case 1:
      // Serial.println("close blinds");
      client.publish("sove/get_blind_pos","1");
      digitalWrite(13, HIGH);
      myservo.write(close_pos);
      delay(1500);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(13, LOW);
      break;

    case 100:
      // Serial.println("blinds up");
      client.publish("sove/get_blind_pos","1000");
      digitalWrite(13, HIGH);
      myservo.write(open_pos);
      delay(70000);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(13, LOW);
      break;

    case 0:
      // Serial.println("blinds up");
      client.publish("sove/get_blind_pos","0");
      digitalWrite(13, HIGH);
      myservo.write(close_pos);
      delay(70000);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(13, LOW);
      break;


    default:
      Serial.println("error!");
      blink_led(5, 300);
      Serial.println("wrong value");
      digitalWrite(13, HIGH);
      myservo.write(close_pos);
      delay(100);
      myservo.write(open_pos);
      delay(100);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(13, LOW);
      client.publish("sove/blinds_error", "1");
      break;
  }
}