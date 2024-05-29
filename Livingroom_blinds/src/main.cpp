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
IPAddress local_IP(192, 168, 1, 202);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

PubSubClient client(espClient);

enum class states 
{   down, 
    closed, 
    opened,
    up,
} blind_state; 

long actuation_time = 0;
unsigned long lastMsg_time = 0;
#define ledPin 2
#define servo_relay_pin 13

Servo myservo;
const int open_pos = 113;
const int neutral_pos = 56;
const int close_pos = 0;
const int swing_time = 500;


void blink_led(unsigned int, unsigned int);
void setup_wifi();
void connect_mqttServer();
void callback(char*, byte*, unsigned int);


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(servo_relay_pin, OUTPUT);
  pinMode(12, OUTPUT);
  // pinMode(15, INPUT_PULLDOWN);
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
  if (!client.connected()) {connect_mqttServer();}
  client.loop();
  ArduinoOTA.handle();
  unsigned long now = millis();
 
  
  if(now > actuation_time+lastMsg_time){
    myservo.write(neutral_pos);
    delay(swing_time);
    digitalWrite(servo_relay_pin, LOW);
  }
    
  if (lastMsg_time > millis()){
    lastMsg_time = millis();
  }
  delay(100);   
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

        if (client.connect("livingroom_client")) { // Change the name of client here if multiple ESP32 are connected
          //attempt successful
          Serial.println("connected");
          client.subscribe("stue/set_blind_pos");
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
  // test range of servo easily
  // int ass = map(intValue, 0, 100, 0, 180);

  // digitalWrite(servo_relay_pin, HIGH);
  // myservo.write(ass);
  // delay(1000);
  // myservo.write(neutral_pos);
  // delay(swing_time);
  // digitalWrite(servo_relay_pin, LOW);

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Check if the message is a valid integer
  bool isValidInteger = true;
  for (int i = 0; i < messageTemp.length(); i++) {
    if (!isDigit(messageTemp.charAt(i))) {
      isValidInteger = false;
      break;
    }
  }

  if (!isValidInteger) {
    Serial.println("error!");
    blink_led(5, 300);
    Serial.println("wrong value");
    digitalWrite(servo_relay_pin, HIGH);
    myservo.write(close_pos);
    delay(100);
    myservo.write(open_pos);
    delay(100);
    myservo.write(neutral_pos);
    delay(swing_time);
    digitalWrite(servo_relay_pin, LOW);
    client.publish("stue/blind_error", "ERROR! Invalid input");
    return;
  }

  int intValue = messageTemp.toInt();
  Serial.println(intValue);

  switch (intValue) {
    case 12:
      Serial.println("light on");
      digitalWrite(ledPin, HIGH);
      client.publish("stue/light","light on");
      break;

    case 21:
      Serial.println("light off");
      digitalWrite(ledPin, LOW);
      client.publish("stue/light","light off");
      break;

    case 25: //open blinds
      client.publish("stue/get_blind_pos","25");
      digitalWrite(servo_relay_pin, HIGH);
      delay(500);
      myservo.write(open_pos);
      actuation_time = 1500;
      lastMsg_time = millis();
      blind_state = states::opened;
      break;

    case 1: //close blinds
      client.publish("stue/get_blind_pos","1");
      digitalWrite(servo_relay_pin, HIGH);
      delay(swing_time);
      myservo.write(close_pos);
      actuation_time = 1500;
      lastMsg_time = millis();
      blind_state = states::closed;
      break;

    case 100: //blinds up
      if(blind_state==states::up){break;}
      client.publish("stue/get_blind_pos","100");
      digitalWrite(servo_relay_pin, HIGH);
      delay(500);
      myservo.write(open_pos);
      actuation_time = 70000;
      lastMsg_time = millis();
      blind_state = states::up;
      break;

    case 0: //blinds down
      client.publish("stue/get_blind_pos","0");
      digitalWrite(servo_relay_pin, HIGH);
      delay(500);
      myservo.write(close_pos);
      actuation_time = 70000;
      lastMsg_time = millis();
      blind_state = states::down;
      break;


    default:
      Serial.println("error!");
      blink_led(5, 300);
      Serial.println("wrong value");
      digitalWrite(servo_relay_pin, HIGH);
      delay(500);
      myservo.write(close_pos);
      delay(100);
      myservo.write(open_pos);
      delay(100);
      myservo.write(neutral_pos);
      delay(swing_time);
      digitalWrite(servo_relay_pin, LOW);
      client.publish("stue/blind_error", "ERROR!");
      break;
  } 

}

