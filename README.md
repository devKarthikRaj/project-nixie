# Project Nixie

<br> This is a Nixie Clock created by two engineering students. </br>
## Overview
Nixie Tubes were used back in the day to display numbers and text, until the world was taken over by LEDs. </br>

### Featuring our contraption...
This project aims to bring back a relic of the past, but packed with modern tech. This clock comes with features like Wi-Fi, BLE, onboard sensors, ARGB lighting etc., all of which is controlled through a mobile app.<br />

<img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Media/Clock%20Hardware%20v1.jpg" alt="Nixie Clock" width="50%"/><img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Media/Clock%20Hardware%20v2.jpg" alt="Nixie Clock" width="50%"/><br />
___

## Design

Our Nixie clock is designed from the ground up with individual drivers for every digit (no multiplexing), Wi-Fi/BLE, built-in timekeeping, and ruggedized power supply. The custom firmware drivers allow for full dimming and/or crossfading control of the digits. Users can also customize the firmware to suit their needs. <br>

The clock is housed in a 3D printed dark grey nylon enclosure, providing a high quality feel and look. <br>

![alt text](https://github.com/devKarthikRaj/project-nixie/blob/master/Media/Nixie%20Clock%20v2%20in%20Action%20gif.gif)

## Features
- Wi-Fi/BLE for NTP sync/smart home functionality
- Dual core microcontroller (ESP32) for complex tasks
- High accuracy RTC with TCXO to accurately keep track of time once sync'ed
- High efficiency, rugged power components to ensure long service life
- Individually addressible RGB tube illumination
- Individually driven digits which make crossfading or other such effects light on I/O as compared to multiplexed
- Replaceable tubes (trim leads to approx 8-10mm)<br />
<br><img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/psu.jpeg" alt="Power Supply Section" width="27.25%"/><img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/digital.png" alt="Digital Section" width="25%"/><br />

## Android App Control
Our Nixie Tube clock can be controlled through our Android app - NixieCon. NixieCon connects to the hardware via Bluetooth. Download the app [here](https://github.com/devKarthikRaj/project-nixie/blob/master/Software/app/release/app-release.apk).<br>

<div align="center">
<img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Home.jpg" alt="App Home" width="24%" />
<img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Time%20Mode.jpg" alt="App Time Mode" width="24%" />
<img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20Countdown%20Mode.jpg" alt="App Countdown Mode" width="24%" />
<img src="https://github.com/devKarthikRaj/project-nixie/blob/master/Pictures/App%20LED%20Control.jpg" alt="App LED Control" width="24%" />
</div>

## Links
[Hardware Documentation](https://github.com/devKarthikRaj/project-nixie/blob/master/Hardware/Nixie%20Tube%20Clock%20User%20Guide.pdf) <br>
[Nixie Tube HAL](https://github.com/edward62740/NixieDisplay-HAL) <br>

___ 
## License and Usage
This project and the resources contained therein may be used in accordance with the [LICENSE](https://github.com/devKarthikRaj/project-nixie/blob/master/LICENSE).

### Disclaimer (and read the license)

The reproduction and/or use of this project, and any resulting consequences thereof are completely at your own risk. The high-voltage supply contained in this design is capable of outputting up to 260 volts unadjusted, which can cause severe injury or death. There may be spurious emissions or other forms of EMI which may disrupt the usage of other appliances, from which a malfunction may cause severe injury or death. During normal operation, the power components radiate a non-trivial amount of heat, which may result in a fire hazard if poorly ventilated. IN-14 nixie tubes (used here) contain small amounts of mercury, which may be released if the tube is broken. Read the license.
___ 

## Developers
[Karthik Raj](https://github.com/devKarthikRaj) <br>
[Edward](https://github.com/edward62740)


Released under the AGPL-3.0 License