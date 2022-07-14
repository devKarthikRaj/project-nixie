#include <Arduino.h>
#include <FaBoRTC_PCF2129.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "NTC_PCA9698.h"
#include <WS2812FX.h>
#include <WiFi.h>
#include "time.h"
#include "nixiedisplay.h"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "DFRobot_SHT20.h"

void ifdb(void *pvParameters);
void tubes(void *pvParameters);
void leds(void *pvParameters);
bool platformGPIOWrite(uint8_t pin, bool data);
void platformDelayMs(uint32_t ms);
#ifdef __cplusplus
extern "C"
{
#endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

String hostname = "NIXIE TUBE CLOCK";

/* InfluxDB Connection Parameters */
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define INFLUXDB_URL ""
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN ""
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG ""
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET ""
#define TZ_INFO ""

/* InfluxDB Data Points */
Point Clock("Clock");

const char *ntpServer = "";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

#define LED_COUNT 6
/* Pin configuration */
#define LED1 23
#define LED2 25
#define LED3 26
#define LED4 27
#define LEDADDR 21
#define SDA 19
#define SCL 18
#define OE0 12
#define OE1 13
#define INT_RTC 2
#define nEN_170 4
#define EN_5 16

/* Comms configuration */
#define UART_BAUDRATE 115200
#define I2C_CLK_RATE 100000

uint8_t pinout1[10] = {5, 4, 3, 2, 1, 0, 9, 8, 7, 6};
uint8_t pinout2[10] = {15, 14, 13, 12, 11, 10, 27, 26, 25, 24};
uint8_t pinout3[10] = {35, 34, 31, 30, 29, 28, 39, 38, 37, 36};
uint8_t pinout4[10] = {45, 44, 43, 42, 41, 40, 49, 48, 47, 46};
uint8_t pinout5[10] = {55, 54, 53, 52, 51, 50, 67, 66, 65, 64};
uint8_t pinout6[10] = {75, 74, 71, 70, 69, 68, 79, 78, 77, 76};

/* Objects */
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LEDADDR, NEO_GRB + NEO_KHZ800);
FaBoRTC_PCF2129 faboRTC;
PCA9698 gp0(0x20, SDA, SCL, I2C_CLK_RATE); // instantiate PCA9698(I2C address, SDA, SCL, I2C speed)
PCA9698 gp1(0x21, SDA, SCL, I2C_CLK_RATE);
DFRobot_SHT20 sht20;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
NixieDisplay display(6, 0, pinout1, pinout2, pinout3, pinout4, pinout5, pinout6);

/* Timer handles */
TimerHandle_t ifdbTimer;

/* Global variables */
uint32_t prev_time = 0;
uint32_t now_time = 0;
uint32_t hr, mins, sec;
bool is_run = false;
bool post = false;
bool is_hour_chime = false;
bool is_night = false;
bool is_restrict = true;
bool is_inactive = false;

struct
{
  uint32_t IFDB_ERR_COUNT = 0;
  float BOARD_TEMP = 0;
  float BOARD_HUM = 0;
} info;

void vTimerCallback1(TimerHandle_t ifdbTimer)
{
  post = true;
}

void setup()
{

  /* Instantiate tubes */

  pinMode(nEN_170, OUTPUT);
  pinMode(EN_5, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  digitalWrite(LED3, HIGH);
  digitalWrite(EN_5, HIGH);
  digitalWrite(nEN_170, LOW);
  delay(50);
  Wire.begin(SDA, SCL, (uint32_t)I2C_CLK_RATE);
  delay(250);
  display.init();
  display.setCrossfade(true);
  display.setScrollback(true);

  if (faboRTC.searchDevice())
  {
    Serial.println("configuring FaBo RTC I2C Brick");
    faboRTC.configure();
  }
  Serial.begin(115200);
  Serial.println("[INIT] TEMP SENSOR OK");

  gp0.configuration();     // soft reset and configuration
  gp0.portMode(0, OUTPUT); // set port directions
  gp0.portMode(1, OUTPUT);
  gp0.portMode(2, OUTPUT);
  gp0.portMode(3, OUTPUT);
  gp0.portMode(4, OUTPUT);
  gp1.configuration();     // soft reset and configuration
  gp1.portMode(0, OUTPUT); // set port directions
  gp1.portMode(1, OUTPUT);
  gp1.portMode(2, OUTPUT);
  gp1.portMode(3, OUTPUT);
  gp1.portMode(4, OUTPUT);
  for (int i = 0; i < 40; i++)
  {
    gp0.digitalWrite(i, LOW);
    gp1.digitalWrite(i, LOW);
  }
  display.write(0);
  sht20.initSHT20(Wire);
  Serial.println("[INIT] TEMP SENSOR OK");

  xTaskCreatePinnedToCore(
      tubes,   // Task Function
      "tubes", // Name of Task
      10000,   // Stack size of task
      NULL,    // Parameter of the task
      1,       // Priority of the task
      NULL,    // Task handle to keep track of the created task
      1);      // Target Core

  xTaskCreatePinnedToCore(
      ifdb,   // Task Function
      "ifdb", // Name of Task
      75000,  // Stack size of task
      NULL,   // Parameter of the task
      1,      // Priority of the task
      NULL,   // Task handle to keep track of the created task
      0);     // Target Core

  xTaskCreatePinnedToCore(
      leds,            // Task Function
      "leds",          // Name of Task
      10000,           // Stack size of task
      NULL,            // Parameter of the task
      0,               // Priority of the task
      NULL,            // Task handle to keep track of the created task
      tskNO_AFFINITY); // Target Core

  vTaskDelete(NULL);
}

void loop()
{
}

void tubes(void *pvParameters)
{
  DateTime now = faboRTC.now();
  struct tm time;
  while (1)
  {
    digitalWrite(23, HIGH);
    now = faboRTC.now();
    time.tm_hour = now.hour();
    time.tm_min = now.minute();
    time.tm_sec = now.second();
    Serial.print(time.tm_hour);
    Serial.print(time.tm_min);
    Serial.println(time.tm_sec);
    if (time.tm_sec == 00 && time.tm_min == 00)
    {
      is_restrict = false;
      is_hour_chime = true;
    }
    else
    {
      is_restrict = true;
    }
    Serial.println(time.tm_hour);
    if (time.tm_hour > 19 || time.tm_hour < 8)
    {
      is_night = true;
    }
    else
    {
      is_night = false;
    }
    if (time.tm_hour > 23 || time.tm_hour < 6)
    {
      is_inactive = true;
    }
    else
    {
      is_inactive = false;
    }
    info.BOARD_TEMP = (float)sht20.readTemperature();
    info.BOARD_HUM = (float)sht20.readHumidity();
    Serial.println(info.BOARD_TEMP);
    if (time.tm_hour < 24 && time.tm_min < 60 && time.tm_sec < 60)
    {
      display.writeTime(&time);
      if ((time.tm_min % 10) != 6 && is_run)
      {
        is_run = false;
      }
    }

    digitalWrite(23, LOW);

    if ((time.tm_min % 10) == 6 && !is_run)
    {
      display.runProtection(CATHODE_PROTECTION_STYLE_SEQUENTIAL, 5000);
      is_run = true;
    }
    if (now_time == 031500 || now_time == 034500)
    {
      display.runProtection(CATHODE_PROTECTION_STYLE_SLOT, 120000, 50);
    }

    vTaskDelay(250);
  }
}
void ifdb(void *pvParameters)
{
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); // define hostname
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[INIT] ERROR CONNECTING TO WIFI");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(3000);
  }
  delay(500);
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo))
  {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);
  }

  faboRTC.setDate(2021, 5, 12, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  configTzTime("SGT-8", "pool.ntp.org", "time.nis.gov");
  client.setHTTPOptions(HTTPOptions().httpReadTimeout(200));
  client.setHTTPOptions(HTTPOptions().connectionReuse(true));
  // Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  Serial.println("[INIT] IFDB CONNECTION OK");
  ifdbTimer = xTimerCreate("Timer1", 5000, pdTRUE, (void *)0, vTimerCallback1);
  xTimerStart(ifdbTimer, 0);
  while (1)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[INIT] ERROR CONNECTING TO WIFI");
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      delay(2000);
    }
    if (post)
    {

      Clock.clearFields();
      Clock.clearTags();
      Clock.addTag("UID", "N/A");
      if (info.BOARD_TEMP < 100)
      {
        Clock.addField("Board Temp", info.BOARD_TEMP);
        Clock.addField("Board Hum", info.BOARD_HUM);
      }
      Clock.addField("InfluxDB Error Count", info.IFDB_ERR_COUNT);

      Serial.println(info.BOARD_TEMP);
      Serial.println("POSTED");
      digitalWrite(LED4, HIGH);
      if (!client.writePoint(Clock))
      {
        info.IFDB_ERR_COUNT++;
      }
      digitalWrite(LED4, LOW);
      post = false;
    }
    vTaskDelay(1);
  }
}

void leds(void *pvParameters)
{
  ws2812fx.init();
  ws2812fx.setBrightness(255);

  ws2812fx.setSegment(0, 0, 6 - 1, 44, 0xFFFFFF, 200, NO_OPTIONS);
  ws2812fx.start();
  for (int i = 0; i < 90; i++)
  {
    ws2812fx.service();
    delay(1);
  }
  ws2812fx.setSegment(0, 0, 6 - 1, 44, 0xcc6600, 200, REVERSE);
  ws2812fx.start();
  for (int i = 0; i < 90; i++)
  {
    ws2812fx.service();
    delay(1);
  }

  ws2812fx.setSegment(0, 0, 6 - 1, 12, 0xcc6600, 255, REVERSE);
  ws2812fx.start();
  while (!WiFi.isConnected())
  {
    ws2812fx.service();
    delay(1);
  }

  ws2812fx.setBrightness(155);
  ws2812fx.setSegment(0, 0, 6 - 1, 48, 0xF1722E, 300, NO_OPTIONS);
  ws2812fx.start();

  if (is_night)
  {
    ws2812fx.setColor(RED);
  }
  else
  {
    ws2812fx.setColor(BLUE);
  }

  while (1)
  {

    ws2812fx.service();
    if (is_hour_chime && !is_restrict)
    {
      ws2812fx.setBrightness(255);

      ws2812fx.setSegment(0, 0, 6 - 1, 44, 0xFFFFFF, 200, NO_OPTIONS);
      ws2812fx.start();
      for (int i = 0; i < 90; i++)
      {
        ws2812fx.service();
        delay(1);
      }
      ws2812fx.setSegment(0, 0, 6 - 1, 44, 0xcc6600, 200, REVERSE);
      ws2812fx.start();
      for (int i = 0; i < 90; i++)
      {
        ws2812fx.service();
        delay(1);
      }
      ws2812fx.setBrightness(155);
      ws2812fx.setSegment(0, 0, 6 - 1, 48, 0xF1722E, 300, NO_OPTIONS);
      ws2812fx.start();
      if (is_night)
      {
        ws2812fx.setColor(RED);
      }
      else
      {
        ws2812fx.setColor(BLUE);
      }
      is_hour_chime = false;
    }
    vTaskDelay(1);
  }
}

bool platformGPIOWrite(uint8_t pin, bool data)
{

  if (pin >= 40)
  {
    pin = pin - 40;
    gp1.digitalWrite(pin, data);
    return true;
  }
  else
  {
    gp0.digitalWrite(pin, data);
    return true;
  }

  return false;
}

void platformDelayMs(uint32_t ms)
{
  digitalWrite(LED2, LOW);
  vTaskDelay(ms);
  digitalWrite(LED2, HIGH);
}
