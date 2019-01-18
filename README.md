# ESP8266 WiFi car

Firmware for the ESP8266/NodeMCU WiFi car.  
Embedded web server to live stream sonars sensors values via WebSockets.  
Optimised web contents on the SPIFFS flash.  
Towards Reinforcement Learning to learn an obstacle avoidance algorithm in simulation, then apply it on the real car.  
It will be used as teaching material for an Applied Machine Learning engineering course.   

## Quickstart

### Install requirements

```bash
npm install gulp-cli -g
cd gulp_minify
npm install
```

### Minifying the embedded web server files  
mini+ugl    gzipped  
  381 Ko -> 149ko  

```bash
cd gulp_minify  
gulp  
```

### Build and upload the firmware

```bash
platformio run --environment nodemcuv2_USB -t buildfs -t uploadfs  
platformio run --environment nodemcuv2_USB -t upload -t monitor  
```

## Ressources
[PlatformIO](https://platformio.org/):  
- [ESPurna by Xose Pérez](https://github.com/xoseperez/espurna)  
ESP8266 Motor shield:  
- [Squix78](ttps://github.com/squix78/esp8266-projects/tree/master/arduino-ide/wifi-car)  
- [ric96](https://github.com/ric96/esp8266car)  
- [nouyang](https://github.com/nouyang/robot_esp8266-12E/tree/master/wireless_motor_gui_AP)  
- [Alictronix](https://github.com/Alictronix/ESP8266-12E-Robot)  

HC-SR04 sonar:  
- [jshaw](https://github.com/jshaw/NewPingESP8266/)  
- [szormpas](https://community.thinger.io/t/echogarage-project/339)  

WebSockets:
- [NeoCat](https://gist.github.com/NeoCat/a57f73b8db0605e1763d3ca1a1f75941)  
