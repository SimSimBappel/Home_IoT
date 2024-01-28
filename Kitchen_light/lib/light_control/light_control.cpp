#include "light_control.h"

bool fade_light(bool state, bool light_state, float time, int red_led, int green_led, int blue_led){
  if(light_state == state){
    return light_state;
  }
  int wait = (int)time/0.255; // .255 comes from step
//   Serial.print("Wait time: ");
//   Serial.println(wait);
  if(state){
    for(int i = 0; i < 254; i++){
      analogWrite(red_led, i);
      analogWrite(green_led, i);
      analogWrite(blue_led, i);
      delay(wait);
    }
  }
  else{
    for(int i = 254; i > 0; i--){
      analogWrite(red_led, i);
      analogWrite(green_led, i);
      analogWrite(blue_led, i);
      delay(wait);
    }
  }
  
  digitalWrite(red_led, state);
  digitalWrite(green_led, state);
  digitalWrite(blue_led, state);
  light_state = state;
  return light_state;
}


//This function attempts to fade in a non-blocking manner by timing the looprate
bool fade_light_semiblocking(bool state, bool light_state, float time, int red_led, int green_led, int blue_led){
  if(light_state == state){
    return light_state;
  }
  int wait = (int)time/0.255; // .255 comes from step
//   Serial.print("Wait time: ");
//   Serial.println(wait);
  if(state){
    for(int i = 0; i < 254; i++){
      analogWrite(red_led, i);
      analogWrite(green_led, i);
      analogWrite(blue_led, i);
      delay(wait);
    }
  }
  else{
    for(int i = 254; i > 0; i--){
      analogWrite(red_led, i);
      analogWrite(green_led, i);
      analogWrite(blue_led, i);
      delay(wait);
    }
  }
  
  digitalWrite(red_led, state);
  digitalWrite(green_led, state);
  digitalWrite(blue_led, state);
  light_state = state;
  return light_state;
}


void blink_led(int pin, unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW); 
    delay(200);
  }
}