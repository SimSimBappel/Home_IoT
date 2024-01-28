# Home IoT
This repository holds all the embedded code for my home IoT projects. 
Using Homebridge on a raspberry pi to recive commands from siri, and send said commands on mqqt topics to esp32 micro controllers. Currently they have the ability to open/close blinds and turn on led lights based on either commands from siri, or environmental factors.

## TODO
- [ ] Make the blinds actuation non-blocking.
- [ ] Send PIR data to raspberry pi.
- [ ] Decrease power consumption.
- [ ] Fix jittering servo.
- [ ] Open blinds when wind speeds exceed Xm/s
 
- [x] Make the light fading non-blocking.
- [x] Enable OTA.
- [x] Automatic WIFI and MQTT reconnect.




### Remember
<pre>
mqtt server (pi) ip: 192.168.1.100
livingroom esp32 ip: 192.168.1.202
bedroom    esp32 ip: 192.168.1.203
kitchen    esp32 ip: 192.168.1.204
</pre>