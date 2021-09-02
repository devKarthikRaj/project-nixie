# Project Nixie
This repository contains hardware and software documentation for a Nixie tube clock created by two engineer wannabes! <br>

![Product Image](https://drive.google.com/uc?export=view&id=1ISCZ87VOQUT8qHJg4DFFtxaFQR4g-VNW)

### A lil bit of history...
Nixie Tubes were used back in the day in most displays until the world was taken over by LEDs (Light Emitting Diodes). Nixies were invented in Hungary and largely used in the Soviet Union in the 1950s. Nixies are not mass produced anymore! <br>

### Featuring our contraption...
Fret not! We have brought this relic back to life with our custom-built Nixie hardware. Our contraption can display time and also countdown from a set time. It comes with multi-color backlighting too! You can control the time, countdown, and lighting through a mobile app developed by us as well! <br>

[Our product is currently unavailable for purchase] <br>

___

## Contents
### [Hardware](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware) 
[Firmware](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware/Firmware) <br>
[Hardware Documentation](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware/Hardware%20Documentation) <br>

Our custom built Nixie hardware is built around the ESP32 chip on a 4-layer PCB. The firmware uses interrupts and also takes advantage of the ESP32's dual core functionality. The enclosure for the PCB is 3d-printed PA 12. <br>

##### Additional Notes
Libraries for the firmware used in this project are located [here](https://github.com/devKarthikRaj/project-nixie/tree/master/Hardware/Firmware/Libraries) and have to be put into the Arduino's library folder before compiling the code. <br>

This project contains a custom written library specific to our custom-built Nixie hardware as well as a few open source libraries downloaded off the internet.

### [Software](https://github.com/devKarthikRaj/project-nixie/tree/master/Software) <br>
[Android App](https://github.com/devKarthikRaj/project-nixie/tree/master/Software) <br>

Our Nixie Tube clock can be controlled through our Android app - NixieCon. NixeCon connects to the hardware via Bluetooth.

#### Get the app!
<div align="center">
	<div class="clearfix">
	  <div class="img-container">
		<img src="https://drive.google.com/uc?export=view&id=1HyYBXJ0fQLPbphzMWAkY_Zig-mV50mGT" height="300"> 
		<img src="https://drive.google.com/uc?export=view&id=1I7a_C3IQ5gpD9yG2XNPnXy6p5Sw_wXa3" height="300"> 
		<img src="https://drive.google.com/uc?export=view&id=1HyexAPifqvR2obPhjKel27XPyoyMqOuk" height="300">
      </div>
	</div>
	<div>
		<a href="https://play.google.com/store/apps/details?id=com.raj.projectnixiev3">
			<img src="https://drive.google.com/uc?export=view&id=1hqWEkSeNjhhtVZcWNqooTvevPk3sEfvX" alt="Get it on the play store" class="center">
		</a>
	</div>
</div>

___ 

## What's in it for you? <br>
If you are developing anything Nixie related, feel free use the hardware and software documentation here!

___ 

## Developers
[Edward Tan](https://github.com/edward62740) <br>
[Karthik Raj](https://github.com/devKarthikRaj)