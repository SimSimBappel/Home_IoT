#include "light_control.h"

int light_val = 0;

light_feedback fade_light(bool light_state, float time, int red_led, int green_led, int blue_led){
  if(!light_state){
      light_val++;
    }
    else{
      light_val--;
    }
  int wait = (int)time/0.255;
  delay(wait);
  if(light_val <= 255 && light_val >= 0){
    analogWrite(red_led, light_val);
    analogWrite(green_led, light_val);
    analogWrite(blue_led, light_val);
    light_feedback result = {light_state, true};
    return result;
  }
  else{
    light_feedback result = {!light_state, false};
    return result;
  }
}

bool switch_light(bool state,int red_led, int green_led, int blue_led){
  digitalWrite(red_led, state);
  digitalWrite(green_led, state);
  digitalWrite(blue_led, state);
  return state;
}


void blink_led(int pin, unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW); 
    delay(200);
  }
}