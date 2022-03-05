#include <Wire.h> //I2C lib
#include <pcf2129rtc.h> //RTC lib
#include <NTC_PCA9698.h> //Port Expander lib
#include "nixiedisplay.h" //Nixie Tube Driver Lib
#include "BluetoothSerial.h" //Bluetooth lib
#include <WS2812FX.h> //RGB LED lib

bool platformGPIOWrite(uint8_t pin, bool data);
void platformDelayMs(uint32_t ms);

/* Pin definitions */
#define rtcInt          2 //RTC Interrupt 
#define en170V          4 //170V enable - Pull to low to enable               
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


/* Nixie tube pinouts */
uint8_t pinout1[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
uint8_t pinout2[10] = {27, 10, 11, 12, 13, 14, 15, 24, 25, 26};
uint8_t pinout3[10] = {39, 28, 29, 30, 31, 34, 35, 36, 37, 38};
uint8_t pinout4[10] = {49, 40, 41, 42, 43, 44, 45, 46, 47, 48};
uint8_t pinout5[10] = {67, 50, 51, 52, 53, 54, 55, 64, 65, 66};
uint8_t pinout6[10] = {79, 68, 69, 70, 71, 74, 75, 76, 77, 78};

/* Nixie tube driver-specific parameters */
uint8_t active = 6; //6 active tubes
uint8_t offset = 0; //Do not offset

/* Objects */
pcf2129rtc pcf2129rtcInstance(twimIntSDA, twimIntSCL);
BluetoothSerial espBt;
WS2812FX ws2812fx = WS2812FX(6, ledBus, NEO_GRB + NEO_KHZ800);
NixieDisplay display(active, offset, pinout1, pinout2, pinout3, pinout4, pinout5, pinout6);
PCA9698 expanderChip0(0x20, twimIntSDA, twimIntSCL, 1000000); //(I2C_ADDR,SDA,SCL,SPEED)
PCA9698 expanderChip1(0x21, twimIntSDA, twimIntSCL, 1000000);

/* Global variables */
bool setHardwareMode = true; //true = Time Mode | false = Countdown Mode | Default = Time Mode
bool updateRtcFlag = false; //Alerts core 1 that RTC has to be updated
bool updateLedFlag = false;
bool timeInitFlag = false;
bool countdownInitFlag = false;
int rxHour = 0;
int rxMin = 0;
int rxSec = 0;
int ledModeNum = 0;
int ledBrightnessVar = 0;
int redVar = 0;
int greenVar = 0;
int blueVar = 0;
unsigned long catProInitTime = 0; //Cathode Protection Initial Time

/* Input data buffer */
char recDataBuf[20] = "";

/* RTC variables */
int rtcTimeConcat = 0;
int rtcTimeConcatPrev = 999999;

/* Countdown variables */
int rxTimeConcat = 0;
int rxTimeConcatPrev = 999999;

/* Seconds Interrupt Flag */
bool secIntFlag = false;

/* Secrets */
String ID = "7C0A";

/*! rtcIntISR() :: ISR
   @brief interrupt service routine for RTC, triggered every second
   @note none
   @param void
*/
void IRAM_ATTR rtcIntISR() {
  secIntFlag = true;
}


/*! setup() :: TASK
   @brief default setup function
   @note none
   @param void
*/
void setup() {

  //IO Definitions
  pinMode(rtcInt, INPUT_PULLUP); //***Interrupt pin has to be set as input pullup***
  pinMode(en170V, OUTPUT);
  pinMode(en5V, OUTPUT);
  pinMode(supervisor5V, INPUT);
  pinMode(ledBus, OUTPUT);
  pinMode(sysLed, OUTPUT);
  pinMode(opsLed, OUTPUT);
  pinMode(devLed, OUTPUT);
  pinMode(pwrLed, OUTPUT);
  pinMode(comLed, OUTPUT);

  //Turn on the 5V and 170V supplies (3.3V is auto turned on cuz ESP32 runs off it)
  digitalWrite(en5V, HIGH);
  digitalWrite(en170V, LOW);

  //Core 0 Config
  xTaskCreatePinnedToCore(
    btTask, //Task Function
    "BT Task",     //Name of Task
    10000,        //Stack size of task
    NULL,         //Parameter of the task
    2,            //Priority of the task
    NULL,       //Task handle to keep track of the created task
    0);           //Target Core

  //Core 1 Config
  xTaskCreatePinnedToCore(
    nixieTask, //Task Function
    "Nixie Task",     //Name of Task
    10000,        //Stack size of task
    NULL,         //Parameter of the task
    3,            //Priority of the task
    NULL,       //Task handle to keep track of the created task
    1);           //Target Core
    
}

void loop() {
  //Loop isn't used as all the code has been separated into tasks and run explicitly on core 0 and 1
}


//This function is called by the pwrSubSysMonitor() when there is a blackout on the 5V rail
//This function will switch off the 5V and 170V power supplies and turn off the 6 Nixie Tubes and RGB LEDs...
//This puts the hardware into safe mode, thereby protecting the hardware against further damage!
void disableSubsystems() {
  //Disable power sub systems
  digitalWrite(en170V, HIGH);
  digitalWrite(en5V, LOW);

  //Disable RGB LEDs
  ws2812fx.stop();
}



/*! btTask() :: TASK
   @brief enables bt, rx data from app, sets flags to inform nixieTask and ledTask
   @note none
   @param void
*/
void btTask(void * pvParameters) {

  //Name of BT signal
  //This will be the name shown to other BT devices in the BT network
  //ID is defined above and is unique to each clock (This is to prevent multiple clocks from having the same name on the BT network)
  espBt.begin("Nixie_Clock_" + ID);

  while (1) {
    //If app data has been received from app side...
    if (espBt.available()) {
      //Receive data from app via Bt
      String recDataString = espBt.readString();
      //Serial.println("Data Received From App: " + recDataString);
      //Serial.println("Processing Data Received...");

      //Clear the array
      for (int i = 0; i < 20; i++) {
        recDataBuf[i] = '\0';
      }

      recDataString.toCharArray(recDataBuf, (recDataString.length() + 1));

      //Process what kind of data it is (time/countdown/LED)and accordingly store to time/countdown/LED vars
      /*Standard format of data on Project Nixe Bluetooth Link:
         A:BC:DE:FG
           A = T (Time Mode) / C (Countdown Mode) / L (Config LED)
         B-C = Hours
         D-E = Mins
         F-G = Secs
      */

      //Time decoding
      //This decoding is necessary as the time sent by the android system varies in format for diff times
      if (recDataString.length() == 10) {
        //Format: T:HH:MM:SS or C:HH:MM:SS
        rxHour = (String(recDataBuf[2]) + String(recDataBuf[3])).toInt();
        rxMin = (String(recDataBuf[5]) + String(recDataBuf[6])).toInt();
        rxSec = (String(recDataBuf[8]) + String(recDataBuf[9])).toInt();
      }
      else if (recDataString.length() == 9) {
        if (recDataBuf[4] == ':') {
          if (recDataBuf[7] == ':') {
            //Format: T:HH:MM:S or C:HH:MM:S
            rxHour = (String(recDataBuf[2]) + String(recDataBuf[3])).toInt();
            rxMin = (String(recDataBuf[5]) + String(recDataBuf[6])).toInt();
            rxSec = (String(recDataBuf[8])).toInt();
          }
          else {
            //Format: T:HH:M:SS or C:HH:M:SS
            rxHour = (String(recDataBuf[2]) + String(recDataBuf[3])).toInt();
            rxMin = (String(recDataBuf[5])).toInt();
            rxSec = (String(recDataBuf[7]) + String(recDataBuf[8])).toInt();
          }
        }
        else if (recDataBuf[3] == ':') {
          //Format: T:H:MM:SS or C:H:MM:SS
          rxHour = (String(recDataBuf[2])).toInt();
          rxMin = (String(recDataBuf[4]) + String(recDataBuf[5])).toInt();
          rxSec = (String(recDataBuf[7]) + String(recDataBuf[8])).toInt();
        }
      }
      else if (recDataString.length() == 7) {
        //Format: T:H:M:S or C:H:M:S
        rxHour = (String(recDataBuf[2])).toInt();
        rxMin = (String(recDataBuf[4])).toInt();
        rxSec = (String(recDataBuf[6])).toInt();
      }
      if (recDataString.length() == 8) {
        if (recDataBuf[3] == ':') {
          if (recDataBuf[5] == ':') {
            //Format: T:H:M:SS or C:H:M:SS
            rxHour = (String(recDataBuf[2])).toInt();
            rxMin = (String(recDataBuf[4])).toInt();
            rxSec = (String(recDataBuf[6]) + String(recDataBuf[7])).toInt();
          }
          else {
            //Format: T:H:MM:S or C:H:MM:S
            rxHour = (String(recDataBuf[2])).toInt();
            rxMin = (String(recDataBuf[4]) + String(recDataBuf[5])).toInt();
            rxSec = (String(recDataBuf[7]) + String(recDataBuf[8])).toInt();
          }
        }
        else if (recDataBuf[4] == ':') {
          //Format: T:HH:M:S or C:HH:M:S
          //Serial.println("2");
          rxHour = (String(recDataBuf[2]) + String(recDataBuf[3])).toInt();
          rxMin = (String(recDataBuf[5])).toInt();
          rxSec = (String(recDataBuf[7])).toInt();
        }
      }

      //If Time Mode Initiated by App...
      if (recDataBuf[0] == 'T') {
        //Serial.println("Time Mode Initiated");

        //Set setHardwareMode to true indicating to core 1 that user has activated time mode
        setHardwareMode = true;

        //Set updateRtcFlag to alert core 1 to update the display
        updateRtcFlag = true;

        //set the timeInitFlag
        timeInitFlag = true;
      }

      //If Countdown Mode Initiated by App...
      else if (recDataBuf[0] == 'C') {
        //Serial.println("Countdown Mode Initiated");

        //Set setHardwareMode to false indicating to core 1 that user has activated countdown mode
        setHardwareMode = false;

        //Set updateRtcFlag to alert core 1 to update the display
        updateRtcFlag = true;

        //set the countdownInitFlag to run the countdownInit function once
        countdownInitFlag = true;
      }

      //If RGB LED Config Changed by App...
      else if (recDataBuf[0] == 'L') {
        //Serial.println("RGB LED Config");

        //Format: L:A:BCD:EFG:HIJ:KLM
        /*   A = LED Mode   = 1-8
           B-D = Brightness = 0-100
           E-G = R in RGB   = 0-255
           H-J = G in RGB   = 0-255
           K-M = B in RGB   = 0-255
        */

        //Read LED Mode
        if (recDataBuf[2] == '1') {
          //Mode 1 - Rainbow Cycle
          ledModeNum = 1;
        }
        else if (recDataBuf[2] == '2') {
          //Mode 2 - Breath
          ledModeNum = 2;
        }
        else if (recDataBuf[2] == '3') {
          //Mode 3 - Fade
          ledModeNum = 3;
        }
        else if (recDataBuf[2] == '4') {
          //Mode 4 - Theater Chase
          ledModeNum = 4;
        }
        else if (recDataBuf[2] == '5') {
          //Mode 5 - Theater Chase Rainbow
          ledModeNum = 5;
        }
        else if (recDataBuf[2] == '6') {
          //Mode 6 - Running Lights
          ledModeNum = 6;
        }
        else if (recDataBuf[2] == '7') {
          //Mode 7 - Merry Christmas
          ledModeNum = 7;
        }
        else if (recDataBuf[2] == '8') {
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
      digitalWrite(comLed, !digitalRead(comLed));
      vTaskDelay(pdMS_TO_TICKS(150));
    }
  }
}

/*! nixieTask() :: TASK
   @brief sets nixie tubes according to flags set by btTask and RTC time
   @note none
   @param void
*/
void nixieTask(void * pvParameters) {
  Wire.begin(twimIntSDA, twimIntSCL, 100000);
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
  attachInterrupt(digitalPinToInterrupt(rtcInt), rtcIntISR, FALLING);

  //Port expander chips config and port IO mode setting
  expanderChip0.configuration();
  expanderChip0.portMode(0, OUTPUT); //(PORT_NUM,INPUT/OUTPUT)
  expanderChip0.portMode(1, OUTPUT);
  expanderChip0.portMode(2, OUTPUT);
  expanderChip0.portMode(3, OUTPUT);
  expanderChip0.portMode(4, OUTPUT);

  expanderChip1.configuration();
  expanderChip1.portMode(0, OUTPUT);
  expanderChip1.portMode(1, OUTPUT);
  expanderChip1.portMode(2, OUTPUT);
  expanderChip1.portMode(3, OUTPUT);
  expanderChip1.portMode(4, OUTPUT);

  //whats this? found it in edward's ex... cathode protection?
  for (int i = 0 ; i < 40; i++) {
    expanderChip0.digitalWrite(i, 0);
    expanderChip1.digitalWrite(i, 0);
  }

  display.init(); //Initialize display
  //Start Nixie Clock in time mode from 000000
  int initTime = 0;
  display.write(initTime);

  catProInitTime = millis(); //Init catProInitTime
  //Core 1 Config
  xTaskCreatePinnedToCore(
    ledTask, //Task Function
    "LED Task",     //Name of Task
    2048,        //Stack size of task
    NULL,         //Parameter of the task
    0,            //Priority of the task
    NULL,       //Task handle to keep track of the created task
    tskNO_AFFINITY);           //Target Core

  while (1) {
    //If it has been 10mins since power on or the previous run of the cathode protection routine...
    if (millis() - catProInitTime > 600000) {
      //Run cathode protection and serial print result
      display.runProtection(CATHODE_PROTECTION_STYLE_SEQUENTIAL, 5000);
      catProInitTime = millis(); //Reset catProInitTime to current time
    }

    //If the updateRtcFlag has been set, it means that the hardware has received a command from the user
    //through the mobile app to update the display
    if (updateRtcFlag == true) {
      //If time mode is activated by user...
      if (setHardwareMode == true) {
        pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin, rxSec); //(HOUR,MIN,SEC)
      }
      //If countdown mode is activated by user...
      else {
        //-1 the rxSec cuz in the time taken by the rtc to update the nixie tube... 1s has already passed!!!
        if (rxSec > 0) {
          pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin, rxSec - 1); //(HOUR,MIN,SEC)
        }
        //Boundary Conditions
        //If rxSec is already 0, then rxSec-1 will be -1, so write 59
        else if (rxSec == 0) {
          if (rxMin > 0) {
            pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour, rxMin - 1, 59); //(HOUR,MIN,SEC)
          }
          //If rxMin and rxSec are already 0, then rxSec-1 and rxMin-1 will be -1, so write 59
          else if (rxMin == 0) {
            if (rxHour > 0) {
              pcf2129rtcInstance.updateCurrentTimeToRTC(rxHour - 1, 59, 59); //(HOUR,MIN,SEC)
            }
          }
        }
      }
      updateRtcFlag = false; //Reset the updateRtcFlag
    }

    //If RTC seconds interrupt triggered...
    if (secIntFlag == true) {
      //If time mode is activated by user...
      if (setHardwareMode == true) {
        if (timeInitFlag) {
          //display.clear();
          display.write(0);
          timeInitFlag = false;
        }
        rtcTimeConcat = (String(pcf2129rtcInstance.readRtcHourBCD1()) + String(pcf2129rtcInstance.readRtcHourBCD0()) + String(pcf2129rtcInstance.readRtcMinBCD1()) + String(pcf2129rtcInstance.readRtcMinBCD0()) + String(pcf2129rtcInstance.readRtcSecBCD1()) + String(pcf2129rtcInstance.readRtcSecBCD0())).toInt();
        if (rtcTimeConcat != rtcTimeConcatPrev) {
          //drive nixie
          display.write(rtcTimeConcat);
          rtcTimeConcatPrev = rtcTimeConcat;
        }
      }
      //If countdown mode is activated by user...
      else {
        //for later use
        int countdownHour;
        int countdownMin;
        int countdownSec;
        int countdownTime;
        int countdownTimePrev;
        String formattedCountdownHourString;
        String formattedCountdownMinString;
        String formattedCountdownSecString;

        if (countdownInitFlag) {
          //Clear tubes
          //display.clear();
          display.write(0);

          //saving rxHour, rxMin, rxSec into vars for later use
          countdownHour = rxHour;
          countdownMin = rxMin;
          countdownSec = rxSec;

          int countdownInitTimePrev = 0;
          String formattedRxHourString;
          String formattedRxMinString;
          String formattedRxSecString;
          int countdownInitTime;

          if (rxHour < 10) {
            formattedRxHourString = "0" + String(rxHour);
          }
          else {
            formattedRxHourString = String(rxHour);
          }
          if (rxMin < 10) {
            formattedRxMinString = "0" + String(rxMin);
          }
          else {
            formattedRxMinString = String(rxMin);
          }
          if (rxSec < 10) {
            formattedRxSecString = "0" + String(rxSec);
          }
          else {
            formattedRxSecString = String(rxSec);
          }

          countdownInitTime = (formattedRxHourString + formattedRxMinString + formattedRxSecString).toInt();

          display.write(countdownInitTime);

          if (countdownSec == 0) {
            if (countdownMin == 0) {
              if (countdownHour == 0) {
                display.write(0);
              } else {
                countdownHour -= 1;
                countdownMin = 59;
                countdownSec = 59;
              }
            } else {
              countdownMin -= 1;
              countdownSec = 59;
            }
          } else {
            countdownSec -= 1;
          }

          countdownTimePrev = countdownInitTime;

          //Reset the flag so that this loop doesn't run again
          countdownInitFlag = false;
        } else {
          if (countdownHour < 10) {
            formattedCountdownHourString = "0" + String(countdownHour);
          }
          else {
            formattedCountdownHourString = String(countdownHour);
          }
          if (countdownMin < 10) {
            formattedCountdownMinString = "0" + String(countdownMin);
          }
          else {
            formattedCountdownMinString = String(countdownMin);
          }
          if (countdownSec < 10) {
            formattedCountdownSecString = "0" + String(countdownSec);
          }
          else {
            formattedCountdownSecString = String(countdownSec);
          }

          countdownTime = (formattedCountdownHourString + formattedCountdownMinString + formattedCountdownSecString).toInt();
          display.write(countdownTime);
          //Serial.println(countdownTime);

          countdownTimePrev = countdownTime;

          if (countdownSec == 0) {
            if (countdownMin == 0) {
              if (countdownHour == 0) {
                display.write(0);
              } else {
                countdownHour -= 1;
                countdownMin = 59;
                countdownSec = 59;
              }
            } else {
              countdownMin -= 1;
              countdownSec = 59;
            }
          } else {
            countdownSec -= 1;
          }
        }
      }
      //Reset interrupt secIntFlag
      secIntFlag = false;
      pcf2129rtcInstance.clearMsf();
      //Flash OpsLed to indicate rtc seconds interrupt successful triggering
      digitalWrite(opsLed, !digitalRead(opsLed));
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}
/*! ledTask() :: TASK
   @brief sets leds based on flag set by nixieTask and runs led service()
   @note none
   @param void
*/
void ledTask(void * pvParameters) {

  //RGB LED Config (Default Settings)
  ws2812fx.init();
  ws2812fx.setBrightness(50);
  ws2812fx.setSpeed(5);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2812fx.start();
  while (1) {
    if (updateLedFlag == true) {
      //Check and Set LED Mode
      switch (ledModeNum) {
        case 1:
          //Serial.println("RGB LED set to Rainbow Mode");
          ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
          break;
        case 2:
          //Serial.println("RGB LED set to Breath Mode");
          ws2812fx.setMode(FX_MODE_BREATH);
          break;
        case 3:
          //Serial.println("RGB LED set to Fade Mode");
          ws2812fx.setMode(FX_MODE_FADE);
          break;
        case 4:
          //Serial.println("RGB LED set to Theater Chase Mode");
          ws2812fx.setMode(FX_MODE_THEATER_CHASE);
          break;
        case 5:
          //Serial.println("RGB LED set to Theater Chase Rainbow Mode");
          ws2812fx.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
          break;
        case 6:
          //Serial.println("RGB LED set to Running Lights Mode");
          ws2812fx.setMode(FX_MODE_RUNNING_LIGHTS);
          break;
        case 7:
          //Serial.println("RGB LED set to Merry Christmas Mode");
          ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS);
          break;
        case 8:
          //Serial.println("RGB LED set to Static Mode");
          ws2812fx.setMode(FX_MODE_STATIC);
          break;
      }

      //Check and Set LED Brightness
      //Serial.println("LED Brightness Set To: " + String(ledBrightnessVar));
      ws2812fx.setBrightness(ledBrightnessVar);

      //Check and Set LED Color
      //Serial.println("LED Color Set to RGB: " + String(redVar) + "," + String(greenVar) + "," + String(blueVar));
      ws2812fx.setColor(redVar, greenVar, blueVar);

      updateLedFlag = false; //Reset the updateLEDFlag
    }
    ws2812fx.service();
    vTaskDelay(5);
  }
}

//The function is required for the Nixie tube driver library
bool platformGPIOWrite(uint8_t pin, bool data)
{

  if (pin >= 40)
  {
    pin = pin - 40;
    expanderChip1.digitalWrite(pin, data);
    return true;
  }
  else
  {
    expanderChip0.digitalWrite(pin, data);
    return true;
  }

  return false;
}

//The function is required for the Nixie tube driver library
void platformDelayMs(uint32_t ms)
{
  vTaskDelay(ms);
}
