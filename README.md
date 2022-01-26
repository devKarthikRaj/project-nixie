# Project Nixie
This repository contains hardware and software documentation for a Nixie tube clock created by two engineering students! <br>

![Cover Image](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/Clock%20Hardware%20v1.jpg)
![Cover Image](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/Clock%20Hardware%20Latest.jpeg)

### A lil bit of history...
Nixie Tubes were used back in the day in most displays until the world was taken over by LEDs (Light Emitting Diodes). Nixies were invented in Hungary and largely used in the Soviet Union in the 1950s. Nixies are not mass produced anymore! <br>

### Featuring our contraption...
Fret not! We have brought this relic back to life with our custom-built Nixie hardware. Our contraption can display time and also countdown from a set time. It comes with multi-color backlighting too! You can control the time, countdown, and lighting through a mobile app developed by us as well! <br>

Please contact the developers for information regarding purchase of the clock <br>

___

## Description

Our Nixie clock is designed from the ground up with individual drivers for every digit (no multiplexing), Wi-Fi/BLE, built-in timekeeping, and ruggedized power supply. The custom firmware drivers allow for full dimming and/or crossfading control of the digits. Users can also customize the firmware to suit their needs. <br>

The clock is housed in a 3D printed dark grey nylon enclosure, providing a high quality feel and look. <br>



### Notable Features
- Wi-Fi/BLE for NTP sync/smart home functionality
- Dual core RISC CPU (ESP32) for complex tasks
- High accuracy RTC with TCXO to accurately keep track of time once sync'ed
- High efficiency, rugged power components to ensure long service life
- Individually addressible RGB tube illumination
- Individually driven digits which make crossfading or other such effects light on I/O as compared to multiplexed

### Links
[Hardware](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware) 
[Firmware](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware/Firmware) <br>
[Hardware Documentation](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware/Hardware%20Documentation) <br>
[Software/App](https://github.com/devKarthikRaj/project-nixie/tree/master/Software) <br>





### Android App Control
Our Nixie Tube clock can be controlled through our Android app - NixieCon. NixeCon connects to the hardware via Bluetooth.<br>

![App Home](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Home.jpg)
![App Time Mode](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Time%20Mode.jpg)
![App Countdown Mode](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Countdown%20Mode.jpg)
![App LED Control](https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20LED%20Control.jpg)

___ 

## What's in it for you? <br>
If you are developing anything Nixie related, feel free use the hardware and software documentation here!
The hardware and software can be easily reused/adapted for similar projects.
___ 

## Developers
[Karthik Raj](https://github.com/devKarthikRaj) <br>
[Edward Tan](https://github.com/edward62740)

Released under the AGPL-3.0 License
