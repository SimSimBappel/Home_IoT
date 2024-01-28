#include <Arduino.h>


struct light_feedback {
  bool light_state;
  bool fading;
};

light_feedback fade_light(bool light_state, float time, int red_led, int green_led, int blue_led);

bool switch_light(bool state,int red_led, int green_led, int blue_led);

void blink_led(int pin, unsigned int times, unsigned int duration);

