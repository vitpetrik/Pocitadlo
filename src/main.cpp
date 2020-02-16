#include <Arduino.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFi.h>

#include "esp_wpa2.h"              //wpa2 library for connections to Enterprise networks
#define EAP_IDENTITY "sPetri61079" //if connecting from another corporation, use identity@organisation.domain in Eduroam
#define EAP_PASSWORD "7env8ifu"    //your Eduroam password
const char *ssid = "SPSCV-STUD";   // Eduroam SSID

#define NTP_OFFSET 1 * 60 * 60 // Zimni cas
//#define NTP_OFFSET 2 * 60 * 60 // Letni cas
#define NTP_INTERVAL 60 * 1000 // In miliseconds
#define NTP_ADDRESS "ntp.nic.cz"

struct timeStruct
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

#include <DS3231.h>
#include <LEDcontrol.h>

timeStruct time_RTC;
timeStruct time_NTP;

timeStruct timePoints[] = {
    {7, 00, 00},
    {7, 50, 00},
    {8, 00, 00},
    {8, 45, 00},
    {8, 50, 00},
    {9, 35, 00},
    {9, 45, 00},
    {10, 30, 00},
    {10, 50, 00},
    {11, 35, 00},
    {11, 40, 00},
    {12, 25, 00},
    {12, 35, 00},
    {13, 20, 00},
    {13, 25, 00},
    {14, 10, 00},
    {14, 15, 00},
    {15, 00, 00},
    {15, 05, 00},
    {15, 50, 00},
    {16, 00, 00},
    {16, 45, 00},
    {16, 50, 00},
    {17, 35, 00},
    {18, 00, 00},
    {19, 00, 00},
    {19, 45, 00},
    {19, 50, 00},
    {20, 35, 00},
    {21, 10, 00},
    {21, 15, 00},
    {21, 20, 00},
    {21, 25, 00},
    {21, 30, 00},
    {21, 35, 00},
    {21, 40, 00},
    {21, 45, 00},
    {21, 50, 00},
    {21, 55, 00},
    {22, 00, 00},
    {22, 05, 00},
    {22, 10, 00},
    {22, 15, 00},
    {22, 20, 00},
};

SemaphoreHandle_t i2c_mutex = xSemaphoreCreateMutex();

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

unsigned long convertToSeconds(timeStruct time)
{
  unsigned long out_seconds = 0;
  out_seconds = time.hours * 3600;
  out_seconds += time.minutes * 60;
  out_seconds += time.seconds * 1;
  /*Serial.println("-----------Konverze času na vteřiny-----------");
  Serial.print("Input time: ");
  Serial.print(time.hours);
  Serial.print(':');
  Serial.print(time.minutes);
  Serial.print(':');
  Serial.println(time.seconds);
  Serial.print("Output seconds: ");
  Serial.println(out_seconds);
  Serial.println("----------------------------------------------");*/
  return out_seconds;
}

void task_counting(void *parameters)
{
  unsigned long seconds;
  unsigned long diff;
  uint8_t startIndex = 0;
  char charsShow[2];
  while (1)
  {
    seconds = convertToSeconds(time_RTC);
    if (seconds < convertToSeconds(timePoints[0]))
    {
      diff = convertToSeconds(timePoints[0]) - seconds;
    }
    else if (seconds > convertToSeconds(timePoints[((sizeof(timePoints) / sizeof(timeStruct)) - 1)]))
    {
      diff = seconds - convertToSeconds(timePoints[((sizeof(timePoints) / sizeof(timeStruct)) - 1)]);
    }
    else
    {
      for (uint8_t i = 0; i < (sizeof(timePoints) / sizeof(timeStruct)) - 1; i++)
      {
        if (seconds > convertToSeconds(timePoints[i]) && seconds < convertToSeconds(timePoints[i + 1]))
        {
          if ((convertToSeconds(timePoints[i + 1]) - convertToSeconds(timePoints[i])) < (30 * 60))
          {
            startIndex += 1; /* motion speed */
            FillLEDsFromPaletteColors(startIndex);
          }
          else
          {
            fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
          }
          diff = convertToSeconds(timePoints[i + 1]) - seconds;
          break;
        }
      }
    }

    double minuty = (double)diff / 60;
    Serial.print("Zbývá minut: ");
    Serial.println(minuty);

    if (diff > 60)
    {
      int minuty2 = (minuty + 0.5);
      Serial.println(minuty2);
      Serial.println(((minuty2 / 10) % 10));
      Serial.println((minuty2 % 10));
      charsShow[0] = ((minuty2 / 10) % 10) + '0';
      charsShow[1] = (minuty2 % 10) + '0';

      segments(charsShow[0], 0);
      segments(charsShow[1], 1);

      Serial.print("1. pozice: ");
      Serial.println(charsShow[0]);
      Serial.print("2. pozice: ");
      Serial.println(charsShow[1]);
    }
    else
    {
      int vteriny = (int)diff;

      charsShow[0] = ((vteriny / 10) % 10) + '0';
      charsShow[1] = (vteriny % 10) + '0';

      fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));

      segments(charsShow[0], 0);
      segments(charsShow[1], 1);

      Serial.print("1. pozice: ");
      Serial.println(charsShow[0]);
      Serial.print("2. pozice: ");
      Serial.println(charsShow[1]);
    }

    vTaskPrioritySet(NULL, 10);
    FastLED.show();
    vTaskPrioritySet(NULL, 5);

    delay(20);
  }
}

void task_RTC(void *parameters)
{
  while (1)
  {
    if (xSemaphoreTake(i2c_mutex, 100) == pdTRUE)
    {
      time_RTC = readTime();
      xSemaphoreGive(i2c_mutex);
      /*Serial.println("-------------RTC čas-------------");
      Serial.print("Hodiny: ");
      Serial.println(time_RTC.hours);
      Serial.print("Minuty: ");
      Serial.println(time_RTC.minutes);
      Serial.print("Sekundy: ");
      Serial.println(time_RTC.seconds);
      Serial.println("---------------------------------");*/
      delay(50);
    }
  }
}

void task_NTP(void *parameters)
{
  delay(1000);
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println();
    Serial.print("Connecting to network: ");
    Serial.println(ssid);
    WiFi.disconnect(true);                                                             //disconnect form wifi to set new wifi connection
    WiFi.mode(WIFI_STA);                                                               //init wifi mode
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide identity
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY)); //provide username --> identity and username is same
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD)); //provide password
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();                             //set config settings to default
    esp_wifi_sta_wpa2_ent_enable(&config);                                             //set config settings to enable function
    WiFi.begin(ssid);                                                                  //connect to wifi
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address set: ");
    Serial.println(WiFi.localIP()); //print LAN IP
  }
  timeClient.begin();
  while (1)
  {
    if (timeClient.update())
    {
      time_NTP.hours = (uint8_t)timeClient.getHours();
      time_NTP.minutes = (uint8_t)timeClient.getMinutes();
      time_NTP.seconds = (uint8_t)timeClient.getSeconds();
      /*Serial.println("-------------NTP čas-------------");
      Serial.print("Hodiny: ");
      Serial.println(time_NTP.hours);
      Serial.print("Minuty: ");
      Serial.println(time_NTP.minutes);
      Serial.print("Sekundy: ");
      Serial.println(time_NTP.seconds);
      Serial.println("---------------------------------");*/
      while (true)
      {
        if (xSemaphoreTake(i2c_mutex, 100) == pdTRUE)
        {
          uint8_t outputMsg = setDS3231time(time_NTP);
          xSemaphoreGive(i2c_mutex);
          /*Serial.println("-------------Zápis času do I2C-------------");
          switch (outputMsg)
          {
          case 0:
            Serial.println("Zápis času do I2C modulu proběhl úspěšně");
            break;
          case 1:
            Serial.println("data too long to fit in transmit buffer");
            break;
          case 2:
            Serial.println("received NACK on transmit of address");
            break;
          case 3:
            Serial.println("received NACK on transmit of data");
            break;
          case 4:
            Serial.println("Other error");
            break;
          default:
            Serial.println("Unknown error");
            break;
          }
          Serial.println("-------------------------------------------");*/
          break;
        }
      }
      delay(60000);
    }
    else
    {
      delay(1000);
    }
  }
}

void setup()
{
  delay(500); // power-up safety delay
  Serial.begin(115200);
  Wire.begin(4, 15);
  //setDS3231time(00,40,17,7,15,9,19);
  LEDinit();
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  segments('A', 0);
  segments('4', 1);
  FastLED.show();
  delay(3000);
  xTaskCreatePinnedToCore(task_RTC, "task_RTC", 10000, (void *)1, 5, NULL, 1);
  xTaskCreatePinnedToCore(task_NTP, "task_NTP", 10000, (void *)1, 5, NULL, 1);
  xTaskCreatePinnedToCore(task_counting, "task_counting", 10000, (void *)1, 5, NULL, 1);
  vTaskDelete(NULL);
}

void loop()
{
  vTaskDelete(NULL);
}