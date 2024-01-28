#include <Arduino.h>


bool fade_light(bool state, bool light_state, float time, int red_led, int green_led, int blue_led);

bool fade_light_semiblocking(bool state, bool light_state, float time, int red_led, int green_led, int blue_led);

void blink_led(int pin, unsigned int times, unsigned int duration);

