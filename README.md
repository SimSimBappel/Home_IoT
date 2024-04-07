# Home IoT
This repository holds all the embedded code for my home IoT projects. 
Using Homebridge on a raspberry pi to recive commands from siri, and send said commands on mqqt topics to esp32 micro controllers. Currently they have the ability to open/close blinds and turn on led lights based on either commands from siri, or environmental factors.

## CAD Drawings
Onshape models for the [blinds](https://cad.onshape.com/documents/c204b57e2b652dffe0f4cb2f/w/db689c71abd3ed45c39d807c/e/455f4c33dcff05f660a28a3a)

Onshape models for the [kitchen light](https://cad.onshape.com/documents/a2c710457d6a04e9bad3d419/w/4b1b59d44ccb7fe8d8f181df/e/12f839e1797edc566d70ef96)

## TODO
- [ ] Make the blinds actuation non-blocking.
- [x] Send PIR data to raspberry pi.
- [ ] Decrease power consumption.
- [ ] Fix jittering servo.
- [x] Open blinds when wind speeds exceed Xm/s.
- [ ] Sync blinds with calendar.
- [ ] Add color temperature scaling.
 
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