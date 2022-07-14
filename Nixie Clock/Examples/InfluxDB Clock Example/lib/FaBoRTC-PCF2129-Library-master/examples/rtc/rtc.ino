/**
 @file rtc.ino
 @brief This is an Example for the FaBo RTC I2C Brick.

   http://fabo.io/215.html

   Released under APACHE LICENSE, VERSION 2.0

   http://www.apache.org/licenses/

 @author FaBo<info@fabo.io>
*/

#include <Wire.h>
#include <FaBoRTC_PCF2129.h>

FaBoRTC_PCF2129 faboRTC;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("RESET");

  // デバイス初期化
  Serial.println("Checking I2C device...");
  if (faboRTC.searchDevice()) {
    Serial.println("configuring FaBo RTC I2C Brick");
    faboRTC.configure();
  } else {
    Serial.println("device not found");
    while(1);
  }

  // 日付時刻の設定
  Serial.println("set date/time");
  faboRTC.setDate(2016,4,1,12,1,50);

}

void loop() {
  // 日付時刻の取得
  DateTime now = faboRTC.now();

  // 日付時刻の表示
  Serial.print("Time: ");
  Serial.print(now.year());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.print(now.second());
  Serial.println();

  delay(1000);
}
