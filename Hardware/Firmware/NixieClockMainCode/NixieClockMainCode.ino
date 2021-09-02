//Use NodeMCU-32S for Nixie Hardware
//Use ESP32 DEV MODULE for esp32 dev board 

//Taskhandlers to handle the tasks for each core
TaskHandle_t Task0; //Runs on core 0
TaskHandle_t Task1; //Runs on core 1

//Headers
//#include <Wire.h> //I2C lib
#include <pcf2129rtc.h> //RTC lib
#include "NTC_PCA9698.h" //Port Expander lib
#include <getNixieExpanderPin.h> //Port Expander to Nixie Digit mapping lib
#include "BluetoothSerial.h" //Bluetooth lib
#include <WS2812FX.h> //RGB LED lib

//Pin Definitions 
#define bootControl     0 //ref to ds - not req atm                                 (NOT USED IN THIS CODE)
#define usbSerialTx     1 //Used for code upload from programmer                    (NOT USED IN THIS CODE)
#define rtcInt          2 //RTC Interrupt 
#define usbSerialRx     3 //Used for code upload from programmer                    (NOT USED IN THIS CODE)
#define en170V          4 //170V enable - Pull to low to enable               
#define errata1nc       5 //NC                                                      (NOT USED IN THIS CODE)
#define en5V           16 //5V enable - Pull to high to enable
#define supervisor5V   17 //5V rail blackout indicator - If low, 5V blackout!!!
#define twimIntSCL     18 //I2C SCK 
#define twimIntSDA     19 //I2C SDA
#define ledBus         21 //WS2812 RGBLED Control Bus
#define sysLed         22 //System LED - UNUSED FOR NOW 
#define opsLed         23 //Operation LED - Blinking if RTC interrupt is triggered and Nixie Tubes are updated
#define devLed         25 //Dev LED - UNUSED FOR NOW
#define pwrLed         26 //Power LED for power system (5V & 170V) status
#define comLed         27 //Fast Blinking if not connected to app... Slow Blinking if connected to app

//Defining cross-core var
//-----------------------
bool setHardwareMode= true; //true = Time Mode | false = Countdown Mode | Default = Time Mode

bool updateRtcFlag = false; //Alerts core 1 that RTC has to be updated
int rxHour; 
int rxMin;
int rxSec;

bool updateLedFlag = false;
int ledModeNum;
int ledBrightnessVar;
int redVar;
int greenVar;
int blueVar;

bool countdownInitFlag = false; //Prevents the countdown initialize function from running more than once

unsigned long catProInitTime; //Cathode Protection Initial Time

//Store the received data into a buffer for easy access!
char recDataBuf[20] = ""; 

//Init Nixie tube state vars:
//---------------------------
//Nixie tube digit's pin to be activated
int writeTube1pin;
int writeTube2pin;
int writeTube3pin; 
int writeTube4pin;
int writeTube5pin;
int writeTube6pin;
 
//Nixie tube digit's pin that is currently active
int currentTube1pin;
int currentTube2pin;
int currentTube3pin;
int currentTube4pin;
int currentTube5pin;
int currentTube6pin;

//Nixie tube digit (not pin!!!) that is currently on 
int currentTube1Digit;
int currentTube2Digit;
int currentTube3Digit;
int currentTube4Digit;
int currentTube5Digit;
int currentTube6Digit;

//Creating an instance of the PCF2129 RTC Lib
pcf2129rtc pcf2129rtcInstance(twimIntSDA, twimIntSCL); //(SDA,SCL)

//Seconds Interrupt Flag
bool secIntFlag = false; 
//RTC Seconds Interrupt ISR
void IRAM_ATTR rtcIntISR() {
  secIntFlag = true;
}

//Declare BT object
BluetoothSerial espBt;

//Nixie Clock ID (This ID is unique and is used in the clock's BT network display name)
//The Nixie Clock ID comprises the first and last byte of the ESP32's Bluetooth MAC address
String ID = "7C0A";

//Unique code that'll be known only to user or will be labelled on the hardware itself (security)
String uniqueCode = "UNIQUE_CODE";

//Create 2 instances of the port expander lib to control 2 port expanders via I2C
PCA9698 expanderChip0(0x20,twimIntSDA,twimIntSCL,1000000); //(I2C_ADDR,SDA,SCL,SPEED)
PCA9698 expanderChip1(0x21,twimIntSDA,twimIntSCL,1000000); 

//Create an instance of the "port expander to nixie tube digit pin mapping lib"
//***This lib is specific to the Nixie hardware used in this project***
getNixieExpanderPin getNixieExpanderPinInstance; 

//Create an instance of the RGB LED lib
WS2812FX ws2812fx = WS2812FX(6, ledBus, NEO_GRB + NEO_KHZ800); //(LED_COUNT,LED_PIN,NEO_GRB + NEO_KHZ800)

void setup() {
  Serial.begin(115200); //Set baud rate

  //Core 0 Config
  xTaskCreatePinnedToCore(
    codeForTask0, //Task Function
    "Task 0",     //Name of Task
    10000,        //Stack size of task
    NULL,         //Parameter of the task
    1,            //Priority of the task
    &Task0,       //Task handle to keep track of the created task
    0);           //Target Core

  //Core 1 Config
  xTaskCreatePinnedToCore(
    codeForTask1, //Task Function
    "Task 1",     //Name of Task
    10000,        //Stack size of task
    NULL,         //Parameter of the task
    1,            //Priority of the task
    &Task1,       //Task handle to keep track of the created task
    1);           //Target Core

  //IO Definitions
  pinMode(rtcInt,INPUT_PULLUP); //***Interrupt pin has to be set as input pullup***
  pinMode(en170V,OUTPUT);
  pinMode(en5V,OUTPUT);
  pinMode(supervisor5V,INPUT);
  pinMode(ledBus,OUTPUT);
  pinMode(sysLed,OUTPUT);
  pinMode(opsLed,OUTPUT);
  pinMode(devLed,OUTPUT);
  pinMode(pwrLed,OUTPUT);
  pinMode(comLed,OUTPUT);

  //Turn on the 5V and 170V supplies (3.3V is auto turned on cuz ESP32 runs off it)
  enablePowerSupplies(); 

  //Name of BT signal
  //This will be the name shown to other BT devices in the BT network
  //ID is defined above and is unique to each clock (This is to prevent multiple clocks from having the same name on the BT network)
  espBt.begin("Nixie_Clock_" + ID);

  //RTC Initial Config
  pcf2129rtcInstance.rtcInitialConfig(); 

  //Update RTC with current time
  //Manually write the default hour, min and sec to RTC 
  //This will be the default start up time when the clock is reset
  int defaultHour = 0;
  int defaultMin = 0;
  int defaultSec = 0;
  pcf2129rtcInstance.updateCurrentTimeToRTC(defaultHour, defaultMin, defaultSec); //(HOUR,MIN,SEC)

  //Interrupt Definitions
  //attachInterrupt(digitalPinToInterrupt(PIN_NUM), ISR, mode)
  //Mode: LOW/CHANGE/RISING/FALLING/*HIGH(Only for Due,Zero,MKR1000 boards)* 
  attachInterrupt(digitalPinToInterrupt(supervisor5V), pwrSubSysMonitor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(rtcInt), rtcIntISR, FALLING);

  //Port expander chips config and port IO mode setting
  expanderChip0.configuration();
  expanderChip0.portMode(0,OUTPUT); //(PORT_NUM,INPUT/OUTPUT)
  expanderChip0.portMode(1,OUTPUT);
  expanderChip0.portMode(2,OUTPUT);
  expanderChip0.portMode(3,OUTPUT);
  expanderChip0.portMode(4,OUTPUT);
  
  expanderChip1.configuration();
  expanderChip1.portMode(0,OUTPUT);
  expanderChip1.portMode(1,OUTPUT);
  expanderChip1.portMode(2,OUTPUT);
  expanderChip1.portMode(3,OUTPUT);
  expanderChip1.portMode(4,OUTPUT);

  catProInitTime = millis(); //Init catProInitTime

  //RGB LED Config (Default Settings)
  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(5);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE); 
  ws2812fx.start();
}

void loop() {
  //Loop isn't used as all the code has been separated into tasks and run explicitly on core 0 and 1
}

//Function Definitions
//--------------------
void enablePowerSupplies() {
  digitalWrite(en5V, HIGH);
  digitalWrite(en170V, LOW); 
}

//THIS FUNCTION IS UNTESTED
//pwrSubSysMonitor is triggered when the 5V rail is activated or 5V rail blackout
IRAM_ATTR void pwrSubSysMonitor() {
  //If 5V Blackout...
  if(digitalRead(supervisor5V) == LOW) {
    //Initiate 5V Blackout Procedures
    disableSubsystems(); //This is done to protect the hardware
    digitalWrite(pwrLed,HIGH); //Turn on the pwrLed to indicate to user that there is a power fault
  }
  //If 5V rail is active...
  else {
    digitalWrite(pwrLed,LOW); //Turn off the pwrLed to indicate to the user that the pwr system is alive and kicking!!!
  }
}

//This function is called by the pwrSubSysMonitor() when there is a blackout on the 5V rail
//This function will switch off the 5V and 170V power supplies and turn off the 6 Nixie Tubes and RGB LEDs... 
//This puts the hardware into safe mode, thereby protecting the hardware against further damage!
void disableSubsystems() {
  //Disable power sub systems
  digitalWrite(en170V, HIGH);
  digitalWrite(en5V, LOW);

  //Off All Nixie Tubes
  offNixieTube1();
  offNixieTube2();
  offNixieTube3();
  offNixieTube4();
  offNixieTube5();
  offNixieTube6();
  
  //Disable RGB LEDs
  ws2812fx.stop();
}

//Run this function until it returns true
//Returns true = security verification successful
//Returns false = security verification unsuccessful
bool VerifyBtConnection() {
  int state = 0; //Set the starting state of the state machine
  for(int i=0; i<3; i++) {
    switch(state) {
      case 0:
      //Hang here till a remote device connects...
      Serial.println("Waiting for connection from remote device...");
      while(!espBt.hasClient()) {
        digitalWrite(comLed,HIGH);
        delay(50);
        digitalWrite(comLed,LOW);
        delay(50);
      }

      espBt.print("REQ_CHECK_1");

      //Hang here till level 1 verification key is received... (auto sent by hardware)
      Serial.println("Waiting for Level 1 verification key from remote device...");
      while(!espBt.available()) {
        digitalWrite(comLed,HIGH);
        delay(50);
        digitalWrite(comLed,LOW);
        delay(50);
      }
      
      if(espBt.readString() == "REQ_CONN") {
        Serial.println("Level 1 Pass");
        espBt.print("REQ_CHECK_2"); //Tell hardware that level 1 verification is successful and send level 2 verification
        state = 1; 
      }
      else {
        Serial.println("Level 1 Fail");
        state = 0;
      }
      break;
      
      case 1:
      //Hang here till Level 2 verification key is received... (keyed in by user)
      Serial.println("Waiting for Level 2 verification key from remote device");
      while(!espBt.available()) {
        digitalWrite(comLed,HIGH);
        delay(50);
        digitalWrite(comLed,LOW);
        delay(50);
      }

      if(espBt.readString() == uniqueCode) {
        Serial.println("Level 2 Pass");
        espBt.print("CONN_PASS"); //Tell the hardware that level 2 verification is successful 
        state = 2;
      }
      else {
        Serial.println("Level 2 Fail");
        espBt.print("CONN_FAIL"); //Tell the hardware that level 2 verification is unsuccessful
        state = 0;
      }
      break;
  
      case 2:
      return true;
      break;
    }
  }
}

void offNixieTube1() {
  for(int i=0;i<10;i++) {
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(1,i),LOW);
  }
}
void offNixieTube2() {
  for(int i=0;i<10;i++) {
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(2,i),LOW);
  }
}
void offNixieTube3() {
  for(int i=0;i<10;i++) {
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,i),LOW);
  }
}
void offNixieTube4() {
  for(int i=0;i<10;i++) {
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,i),LOW);
  }
}
void offNixieTube5() {
  for(int i=0;i<10;i++) {
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,i),LOW);
  }
}
void offNixieTube6() {
  for(int i=0;i<10;i++) {
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,i),LOW);
  }
}
bool countdownInit() {
  if(countdownInitFlag == true) {
    //Get the pin numbers of the nixie digits to turn on for each tube
    //getNixieExpanderPinInstance.getPinNumber(TUBE_NUM,BCD_DIGIT)
    writeTube1pin = getNixieExpanderPinInstance.getPinNumber(1,pcf2129rtcInstance.readRtcHourBCD1());
    writeTube2pin = getNixieExpanderPinInstance.getPinNumber(2,pcf2129rtcInstance.readRtcHourBCD0());
    writeTube3pin = getNixieExpanderPinInstance.getPinNumber(3,pcf2129rtcInstance.readRtcMinBCD1());
    writeTube4pin = getNixieExpanderPinInstance.getPinNumber(4,pcf2129rtcInstance.readRtcMinBCD0());
    writeTube5pin = getNixieExpanderPinInstance.getPinNumber(5,(pcf2129rtcInstance.readRtcSecBCD1()));
    writeTube6pin = getNixieExpanderPinInstance.getPinNumber(6,(pcf2129rtcInstance.readRtcSecBCD0()));   

    //Update the current tube digit vars
    currentTube1Digit = pcf2129rtcInstance.readRtcHourBCD1();
    currentTube2Digit = pcf2129rtcInstance.readRtcHourBCD0();
    currentTube3Digit = pcf2129rtcInstance.readRtcMinBCD1();
    currentTube4Digit = pcf2129rtcInstance.readRtcMinBCD0();
    currentTube5Digit = pcf2129rtcInstance.readRtcSecBCD1();
    currentTube6Digit = pcf2129rtcInstance.readRtcSecBCD0();

    //Turn off all digits on that tube
    offNixieTube1(); 
    offNixieTube2();
    offNixieTube3();
    offNixieTube4();
    offNixieTube5();
    offNixieTube6();
    
    //Turn on the updated digit
    expanderChip0.digitalWrite(writeTube1pin,HIGH); 
    expanderChip0.digitalWrite(writeTube2pin,HIGH);
    expanderChip0.digitalWrite(writeTube3pin,HIGH); 
    expanderChip1.digitalWrite(writeTube4pin,HIGH);  
    expanderChip1.digitalWrite(writeTube5pin,HIGH);
    expanderChip1.digitalWrite(writeTube6pin,HIGH);

    //Update the current tube pin state vars
    currentTube1pin = writeTube1pin;
    currentTube2pin = writeTube2pin;
    currentTube3pin = writeTube3pin;
    currentTube4pin = writeTube4pin;
    currentTube5pin = writeTube5pin;
    currentTube6pin = writeTube6pin;

    //Reset the countdownInitFlag indicating that the countdownInit function has already run once
    countdownInitFlag = false;

    //Return true to indicate that the countdownInit function has just run
    return true;
  }
  //Return false to indicate that the coundownInit function has already run and will not run again unless MCU reset
  return false;
}

void cathodeProtection() {
  //A cathode protection a day keeps the cathode poisoning away!!!
  Serial.println("Initiating Routine Cathode Protection");
  //The inner most for loop takes approx 330ms to execute...
  //So for cathode protection to last approx 5s... The inner most for loop must be executed 15 times
  for(int i=0;i<15;i++) {
    for(int i=0; i<10; i++) {
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(1,i),HIGH);
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(2,i),HIGH);
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,i),HIGH);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,i),HIGH);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,i),HIGH);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,i),HIGH);
    delay(10);
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(1,i),LOW);
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(2,i),LOW);
    expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,i),LOW);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,i),LOW);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,i),LOW);
    expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,i),LOW);
    }
  }
  Serial.println("Cathode Protection Successfully Completed");
}

//Main Code for Core 0 and 1 

/* Core 0 
 *  Comms with Project Nixie android app
 *  Send data received from the app to core 1 so that the Nixies can be updated
 */
void codeForTask0(void * parameter) {
  while(1) {
    //Until hardware verification is successful... hang at this line...
    while(VerifyBtConnection()!=true){};  
    Serial.println("Hardware verification successful");
    
    //This is the time when user enters the mobile app's home page to config the clock
    unsigned long initTime = millis();

    //While there is an active BT connection to the app and it has not been more than 3mins since the user started to config the clock...
    //***User has 3mins to config clock, after which the user will have to verify Bt connection again (this is for security purposes)***
    while(espBt.hasClient() && millis() - initTime < 180000) {
      //digitalWrite(comLed,HIGH); 
      //If app data has been received from app side...
      if(espBt.available()) {
        //Receive data from app via Bt
        String recDataString = espBt.readString();
        Serial.println("Data Received From App: " + recDataString);
        Serial.println("Processing Data Received...");
        
        //Clear the array
        for(int i=0; i<20; i++) {
          recDataBuf[i] = '\0';
        }
        
        recDataString.toCharArray(recDataBuf,(recDataString.length()+1));
        
        //Process what kind of data it is (time/countdown/LED)and accordingly store to time/countdown/LED vars
        /*Standard format of data on Project Nixe Bluetooth Link:
         * A:BC:DE:FG
         *   A = T (Time Mode) / C (Countdown Mode) / L (Config LED)
         * B-C = Hours  
         * D-E = Mins
         * F-G = Secs
        */
        
        //Time decoding
        //This decoding is necessary as the time sent by the android system varies in format for diff times
        if(recDataString.length() == 10) {
          //Format: T:HH:MM:SS or C:HH:MM:SS
          rxHour = (String(recDataBuf[2])+String(recDataBuf[3])).toInt();
          rxMin = (String(recDataBuf[5])+String(recDataBuf[6])).toInt();
          rxSec = (String(recDataBuf[8])+String(recDataBuf[9])).toInt();
        }
        else if(recDataString.length() == 9) {
          if(recDataBuf[4] == ':') {
            if(recDataBuf[7] == ':') {
              //Format: T:HH:MM:S or C:HH:MM:S
              rxHour = (String(recDataBuf[2])+String(recDataBuf[3])).toInt();
              rxMin = (String(recDataBuf[5])+String(recDataBuf[6])).toInt();
              rxSec = (String(recDataBuf[8])).toInt();
            }
            else {
              //Format: T:HH:M:SS or C:HH:M:SS
              rxHour = (String(recDataBuf[2])+String(recDataBuf[3])).toInt();
              rxMin = (String(recDataBuf[5])).toInt();
              rxSec = (String(recDataBuf[7])+String(recDataBuf[8])).toInt();
            }
          }
          else if (recDataBuf[3] == ':') {
            //Format: T:H:MM:SS or C:H:MM:SS
            rxHour = (String(recDataBuf[2])).toInt();
            rxMin = (String(recDataBuf[4])+String(recDataBuf[5])).toInt();
            rxSec = (String(recDataBuf[7])+String(recDataBuf[8])).toInt();
          }
        }
        else if(recDataString.length() == 7) {
          //Format: T:H:M:S or C:H:M:S 
          rxHour = (String(recDataBuf[2])).toInt();
          rxMin = (String(recDataBuf[4])).toInt();
          rxSec = (String(recDataBuf[6])).toInt();
        }
        if(recDataString.length() == 8) {
          if(recDataBuf[3] == ':') {
            if(recDataBuf[5] == ':') {
              //Format: T:H:M:SS or C:H:M:SS  
              rxHour = (String(recDataBuf[2])).toInt();
              rxMin = (String(recDataBuf[4])).toInt();
              rxSec = (String(recDataBuf[6])+String(recDataBuf[7])).toInt();
            }
            else {
              //Format: T:H:MM:S or C:H:MM:S
              rxHour = (String(recDataBuf[2])).toInt();
              rxMin = (String(recDataBuf[4])+String(recDataBuf[5])).toInt();
              rxSec = (String(recDataBuf[7])+String(recDataBuf[8])).toInt();
            }
          }
          else if(recDataBuf[4] == ':') {
            //Format: T:HH:M:S or C:HH:M:S
            Serial.println("2");    
            rxHour = (String(recDataBuf[2])+String(recDataBuf[3])).toInt();
            rxMin = (String(recDataBuf[5])).toInt();
            rxSec = (String(recDataBuf[7])).toInt();
          }
        }
        
        //If Time Mode Initiated by App...
        if(recDataBuf[0]=='T') {  
          Serial.println("Time Mode Initiated");

          //Set setHardwareMode to true indicating to core 1 that user has activated time mode
          setHardwareMode = true;

          //Set updateRtcFlag to alert core 1 to update the display
          updateRtcFlag = true;
        }
        
        //If Countdown Mode Initiated by App...
        else if(recDataBuf[0]=='C') {
          Serial.println("Countdown Mode Initiated");

          //Set setHardwareMode to false indicating to core 1 that user has activated countdown mode
          setHardwareMode = false;

          //Set updateRtcFlag to alert core 1 to update the display
          updateRtcFlag = true;

          //set the countdownInitFlag to run the countdownInit function once
          countdownInitFlag = true;
        }
        
        //If RGB LED Config Changed by App...
        else if(recDataBuf[0]=='L') {
          Serial.println("RGB LED Config");

          //Format: L:A:BCD:EFG:HIJ:KLM
          /*   A = LED Mode   = 1-8
           * B-D = Brightness = 0-100
           * E-G = R in RGB   = 0-255 
           * H-J = G in RGB   = 0-255
           * K-M = B in RGB   = 0-255
           */
         
          //Read LED Mode 
          if(recDataBuf[2] == '1') {
            //Mode 1 - Rainbow Cycle
            ledModeNum = 1;
          }
          else if(recDataBuf[2] == '2') {
            //Mode 2 - Breath
            ledModeNum = 2;
          }
          else if(recDataBuf[2] == '3') {
            //Mode 3 - Fade
            ledModeNum = 3;
          }
          else if(recDataBuf[2] == '4') {
            //Mode 4 - Theater Chase
            ledModeNum = 4;
          }
          else if(recDataBuf[2] == '5') {
            //Mode 5 - Theater Chase Rainbow
            ledModeNum = 5;
          }
          else if(recDataBuf[2] == '6') {
            //Mode 6 - Running Lights
            ledModeNum = 6;
          }
          else if(recDataBuf[2] == '7') {
            //Mode 7 - Merry Christmas
            ledModeNum = 7;
          }
          else if(recDataBuf[2] == '8') {
            //Mode 8 - Static
            ledModeNum = 8;
          }
          
          //Read LED brightness
          ledBrightnessVar = (String(recDataBuf[4]) + String(recDataBuf[5]) + String(recDataBuf[6])).toInt();
          
          //Read LED Color
          redVar = (String(recDataBuf[8]) + String(recDataBuf[9]) + String(recDataBuf[10])).toInt(); 
          greenVar = (String(recDataBuf[12]) + String(recDataBuf[13]) + String(recDataBuf[14])).toInt(); 
          blueVar = (String(recDataBuf[16]) + String(recDataBuf[17]) + String(recDataBuf[18])).toInt();

          //set updateLedFlag to alert core 1 to update the RGB LEDs
          updateLedFlag = true;
        }
      }
      //If no data is received from app and core 0 is not busy processing that data...
      else {
        //Slow blink comLed to indicate to user that hardware is connected to app and waiting for commands from app side
        digitalWrite(comLed,HIGH);
        delay(200);
        digitalWrite(comLed,LOW);
        delay(200);
      }
    }
  }
}

/* Core 1
 *  Drive Nixies when seconds interrupt is triggered by RTC
 *  Drive RGB LEDs
 *  Nixie Tube Cathode Protection
 */
void codeForTask1(void * parameter) {
  while(1) {
    //If it has been 10mins since power on or the previous run of the cathode protection routine...
    if(millis() - catProInitTime > 600000) {
      cathodeProtection(); //Lasts 5s 
      catProInitTime = millis(); //Reset catProInitTime to current time

      //Revert all 6 tubes to what they were displaying right before cathode protection started...
      expanderChip0.digitalWrite(currentTube1pin,HIGH); //Turn on the updated digit
      expanderChip0.digitalWrite(currentTube2pin,HIGH); //Turn on the updated digit
      expanderChip0.digitalWrite(currentTube3pin,HIGH); //Turn on the updated digit
      expanderChip1.digitalWrite(currentTube4pin,HIGH); //Turn on the updated digit
      expanderChip1.digitalWrite(currentTube5pin,HIGH); //Turn on the updated digit
      expanderChip1.digitalWrite(currentTube6pin,HIGH); //Turn on the updated digit
    }

    //If RTC seconds interrupt triggered...
    if(secIntFlag == true) {
      //If time mode is activated by user...
      if(setHardwareMode == true) {
        //Get the pin numbers of the nixie digits to turn on for each tube
        //getNixieExpanderPinInstance.getPinNumber(TUBE_NUM,BCD_DIGIT)
        writeTube1pin = getNixieExpanderPinInstance.getPinNumber(1,pcf2129rtcInstance.readRtcHourBCD1());
        writeTube2pin = getNixieExpanderPinInstance.getPinNumber(2,pcf2129rtcInstance.readRtcHourBCD0());
        writeTube3pin = getNixieExpanderPinInstance.getPinNumber(3,pcf2129rtcInstance.readRtcMinBCD1());
        writeTube4pin = getNixieExpanderPinInstance.getPinNumber(4,pcf2129rtcInstance.readRtcMinBCD0());
        writeTube5pin = getNixieExpanderPinInstance.getPinNumber(5,pcf2129rtcInstance.readRtcSecBCD1());
        writeTube6pin = getNixieExpanderPinInstance.getPinNumber(6,pcf2129rtcInstance.readRtcSecBCD0());      
        
        //If the tube has to be updated...
        if(writeTube1pin != currentTube1pin) {
          offNixieTube1(); //Turn off all digits on that tube
          expanderChip0.digitalWrite(writeTube1pin,HIGH); //Turn on the updated digit
          //Turning off the tube and then updating the tube is done to prevent double digit display on the tube
        }
        if(writeTube2pin != currentTube2pin) {
          offNixieTube2();
          expanderChip0.digitalWrite(writeTube2pin,HIGH);
        }
        if(writeTube3pin != currentTube3pin) {
          offNixieTube3();
          expanderChip0.digitalWrite(writeTube3pin,HIGH); 
        }
        if(writeTube4pin != currentTube4pin) {
          offNixieTube4();
          expanderChip1.digitalWrite(writeTube4pin,HIGH);  
        }
        if(writeTube5pin != currentTube5pin) {
          offNixieTube5();
          expanderChip1.digitalWrite(writeTube5pin,HIGH);
        }
        if(writeTube6pin != currentTube6pin) {
          offNixieTube6();
          expanderChip1.digitalWrite(writeTube6pin,HIGH);
        }
    
        //Update the current tube pin state vars
        currentTube1pin = writeTube1pin;
        currentTube2pin = writeTube2pin;
        currentTube3pin = writeTube3pin;
        currentTube4pin = writeTube4pin;
        currentTube5pin = writeTube5pin;
        currentTube6pin = writeTube6pin;
      }
      
      //If countdown mode is activated by user...
      else {
        //Run countdownInit function once (This is done to display the countdown starting time set by the user)
        //Countdown mode uses the RTC only in the countdownInit function, after which it does not use the RTC
        //Once initialized... Countdown mode relies on tube pin state vars to countdown
        bool initRunFlag = countdownInit(); //Returns 1 if function has just run | Returns 0 if function has already run previously
        
        //Countdown using tube pin state vars... No RTC involved!!!
        //If countdown mode is initialized...
        if(initRunFlag == false) {
          //If Tube 6 Digit is more than 0
          if(currentTube6Digit > 0) {
            offNixieTube6();
            expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,currentTube6Digit-1),HIGH);
            currentTube6Digit -= 1;
          }

          //If tube 6 is 0'ed
          else if(currentTube6Digit == 0) {
            //If tube 1,2,3,4,5,6 are 0'ed...
            if(currentTube1Digit == 0 && currentTube2Digit == 0 && currentTube3Digit == 0 && currentTube4Digit == 0 && currentTube5Digit == 0 && currentTube6Digit == 0) {
              //END OF COUNTDOWN!!!
              while(updateRtcFlag == false) {
                cathodeProtection();
              }
            }

            //If tube 2,3,4,5,6 are 0'ed...
            else if(currentTube2Digit == 0 && currentTube3Digit == 0 && currentTube4Digit == 0 && currentTube5Digit == 0 && currentTube6Digit == 0) {
              //stuff
              offNixieTube1();
              offNixieTube2();
              offNixieTube3();
              offNixieTube4();
              offNixieTube5();
              offNixieTube6();
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(1,currentTube1Digit-1),HIGH);
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(2,9),HIGH);
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,5),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,9),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,5),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,9),HIGH);
              currentTube1Digit -= 1;
              currentTube2Digit = 9;
              currentTube3Digit = 5;
              currentTube4Digit = 9;
              currentTube5Digit = 5;
              currentTube6Digit = 9;
            }

            //If tube 3,4,5,6 are 0'ed...
            else if(currentTube3Digit == 0 && currentTube4Digit == 0 && currentTube5Digit == 0 && currentTube6Digit == 0) {
              //stuff
              offNixieTube2();
              offNixieTube3();
              offNixieTube4();
              offNixieTube5();
              offNixieTube6();
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(2,currentTube2Digit-1),HIGH);
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,5),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,9),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,5),HIGH); //ITS SHOWING 4 THO!!!
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,9),HIGH);
              currentTube2Digit -= 1;
              currentTube3Digit = 5;
              currentTube4Digit = 9;
              currentTube5Digit = 5;
              currentTube6Digit = 9;
            }

            //If tube 4,5,6 are 0'ed...
            else if(currentTube4Digit == 0 && currentTube5Digit == 0 && currentTube6Digit == 0) {
              //stuff
              offNixieTube3();
              offNixieTube4();
              offNixieTube5();
              offNixieTube6();
              expanderChip0.digitalWrite(getNixieExpanderPinInstance.getPinNumber(3,currentTube3Digit-1),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,9),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,5),HIGH); //ITS SHOWING 4 THO!!!
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,9),HIGH);
              currentTube3Digit -= 1;
              currentTube4Digit = 9;
              currentTube5Digit = 5;
              currentTube6Digit = 9;
            }

            //If tube 5,6 are 0'ed...
            else if(currentTube5Digit == 0 && currentTube6Digit == 0) {
              offNixieTube4();
              offNixieTube5();
              offNixieTube6();
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(4,currentTube4Digit-1),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,5),HIGH); //ITS SHOWING 4 THO!!!
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,9),HIGH);
              currentTube4Digit -= 1;
              currentTube5Digit = 5;
              currentTube6Digit = 9;
              
            }
            //If only tube 6 is 0'ed...
            else {
              offNixieTube5();
              offNixieTube6();
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(5,currentTube5Digit-1),HIGH);
              expanderChip1.digitalWrite(getNixieExpanderPinInstance.getPinNumber(6,9),HIGH);
              currentTube5Digit -= 1;
              currentTube6Digit = 9;
            }
          }
        }
      }
  
      //Reset interrupt secIntFlag
      secIntFlag = false;
      pcf2129rtcInstance.clearMsf();
      
      //Flash OpsLed to indicate rtc seconds interrupt successful triggering
      digitalWrite(opsLed,HIGH);
      delay(50);
      digitalWrite(opsLed,LOW);
    }
    //Code inside the else loop will execute if the core is not busy driving the nixies
    //Priority is given to driving the nixies and cathode protection!!!
    else {
      //If the updateRtcFlag has been set, it means that the hardware has received a command from the user 
      //through the mobile app to update the display
      if(updateRtcFlag == true) {
        //If time mode is activated by user...
        if(setHardwareMode == true) {
          pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin, rxSec); //(HOUR,MIN,SEC)
        }
        //If countdown mode is activated by user...
        else {
          //-1 the rxSec cuz in the time taken by the rtc to update the nixie tube... 1s has already passed!!!
          if(rxSec > 0) {
            pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin, rxSec-1); //(HOUR,MIN,SEC)
          }
          //Boundary Conditions
          //If rxSec is already 0, then rxSec-1 will be -1, so write 59
          else if(rxSec == 0){
            if(rxMin > 0) {
              pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin-1, 59); //(HOUR,MIN,SEC)
            }
            //If rxMin and rxSec are already 0, then rxSec-1 and rxMin-1 will be -1, so write 59
            else if(rxMin == 0) {
              if(rxHour > 0) {
                pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour-1, 59, 59); //(HOUR,MIN,SEC)
              }
            }
          }
        }
        updateRtcFlag = false; //Reset the updateRtcFlag
      }
      
      //Drive the RGB LEDs
      //If the updateLedFlag has been set, it means that the hardware has received a command from the user
      //through the mobile app to update the RGB LEDs
      if(updateLedFlag == true) {
        //Check and Set LED Mode
        switch(ledModeNum) {
          case 1:
                  Serial.println("RGB LED set to Rainbow Mode");
                  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);  
                  break;
          case 2:
                  Serial.println("RGB LED set to Breath Mode");
                  ws2812fx.setMode(FX_MODE_BREATH);
                  break;
          case 3:
                  Serial.println("RGB LED set to Fade Mode");
                  ws2812fx.setMode(FX_MODE_FADE);  
                  break;
          case 4:
                  Serial.println("RGB LED set to Theater Chase Mode");
                  ws2812fx.setMode(FX_MODE_THEATER_CHASE);
                  break;
          case 5:
                  Serial.println("RGB LED set to Theater Chase Rainbow Mode");
                  ws2812fx.setMode(FX_MODE_THEATER_CHASE_RAINBOW);  
                  break;
          case 6:
                  Serial.println("RGB LED set to Running Lights Mode");
                  ws2812fx.setMode(FX_MODE_RUNNING_LIGHTS);
                  break;
          case 7:
                  Serial.println("RGB LED set to Merry Christmas Mode");
                  ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS);  
                  break;
          case 8:
                  Serial.println("RGB LED set to Static Mode");
                  ws2812fx.setMode(FX_MODE_STATIC);
                  break;      
        }
        
        //Check and Set LED Brightness 
        Serial.println("LED Brightness Set To: " + String(ledBrightnessVar));  
        ws2812fx.setBrightness(ledBrightnessVar);
        
        //Check and Set LED Color
        Serial.println("LED Color Set to RGB: " + String(redVar) + "," + String(greenVar) + "," + String(blueVar));  
        ws2812fx.setColor(redVar,greenVar,blueVar);
        
        updateLedFlag = false; //Reset the updateLEDFlag
      }
      ws2812fx.service();
    }
  }
}
