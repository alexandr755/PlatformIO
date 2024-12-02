/*
ESP32 OTA using PlatformIO
Refer to:
https://lonelybinary.com/blogs/learn/esp32-ota-using-platformio
*/
/*
    Reqruired Library: NTPClient

    https://github.com/arduino-libraries/NTPClient
*/
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-request-esp32-esp8266-nodemcu-sensor-readings/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <GyverTimer.h>   // подключаем библиотеку
#define PIN_LED 2  // 2-ый выходной цифровой контакт LED
#define WIFI_SSID "TP-LINK_17"
#define WIFI_PASS "rob17rob17"

//
GTimer myTimer1(MS); // создать миллисекундный таймер
GTimer myTimer2(MS); // создать миллисекундный таймер
GTimer myTimer24v(MS); // создать миллисекундный таймер
// Используйте @myidbot, чтобы получить ID пользователя или группы
// Помните, что бот сможет вам писать только после нажатия
// вами кнопки /start
#define CHAT_ID "489077210"
#define CHAT_ID_GROUP "-717637187"
///////
// Запустите бот Telegram
#define BOTtoken "5129566661:AAENLAD9pSK9eC13roIVjyYqwFrs6xUAlxA"  // укажите токен бота
///////
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//Каждую секунду проверяет новые сообщения
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
///////////
IPAddress local_IP(192, 168, 17, 18);
IPAddress gateway(192, 168, 17, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Create a WiFiMulti object
WiFiMulti WiFiMulti;
///////////////////NTP
// Define NTP Client to get time

WiFiUDP ntpUDP;

// NTP server, timezone offset +11 Sydney Time, update interval set to 1 hour
//NTPClient timeClient(ntpUDP, "ua.pool.ntp.org", 2 * 3600, 3600000); 
NTPClient timeClient(ntpUDP, "ua.pool.ntp.org", 2 * 3600, 3600000); 
//////////////////
// Запрос показаний датчика HTU21DF и запись их в переменную типа String
String getReadings(){
  float temperature, humidity;
  //temperature = bme.readTemperature();
  //humidity = bme.readHumidity();
  temperature = 25.77;
  humidity = 45.77;
  String message = "Temp Серверная К64: " + String(temperature) + " ºC \n";
  message += "Hum Серверная К64: " + String (humidity) + " % \n";
  return message;
}
/////////
//Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
      if (chat_id == CHAT_ID || chat_id == CHAT_ID_GROUP);
      else
     {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
     }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command to get current readings.\n";
      welcome += "/readings \n";
      welcome += "Use the /ON for turn on.\n";
      welcome += "/ON \n";
      welcome += "Use the /OFF for turn off .\n";
      welcome += "/OFF \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }
    if (text == "/ON") {
      digitalWrite(PIN_LED, HIGH);
      bot.sendMessage(chat_id, "Нагрузка включенна ON", "");
    }    
    if (text == "/OFF") {
      digitalWrite(PIN_LED, LOW);
      bot.sendMessage(chat_id, "Нагрузка выключенна OFF", "");
    } 
  }
  
}

void syncNTP();
/////////////////////////

void setup()
{
  Serial.begin(115200);
  myTimer1.setInterval(20000);   // настроить интервал 1 мин = 60000ms 5мин
  myTimer2.setInterval(43200000);   // (43200000) настроить интервал 1 мин = 60000ms  12ч
  myTimer24v.setInterval(60000);   // настроить интервал 1 мин = 60000ms  1m
  pinMode(PIN_LED, OUTPUT);
  //начальная установка релле
  digitalWrite(PIN_LED, HIGH); // реле выключено
  ///
  delay(10);

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

  Serial.print("\n\nWaiting for WiFi... ");

  // WIFI Connection, Reboot after 30 attempts
  uint8_t not_connected_counter = 0;
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
    not_connected_counter++;
    if (not_connected_counter > 30)
    {
      Serial.println("Resetting due to Wifi not connecting...");
      ESP.restart();
    }
  }

  if (WiFi.SSID() == WIFI_SSID)
  {
    // WiFi.config should after WiFi.begin or wifiMulti.addAP
    Serial.println("\nManual IP Configuration");
    WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("TEst is finished5");
  // Initialize NTP client but do not start syncing yet
  timeClient.begin();
  ///
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  // OTA Configiration and Enable OTA
  Serial.println("\nEnabling OTA Feature");
  ArduinoOTA.setPassword("vfyrtdbxf21");
  ArduinoOTA.begin();
}

void loop()
{
  // OTA Handle
  ArduinoOTA.handle();
  syncNTP();
  /*
  // Optional Display current time (last synced, updated every second)
  Serial.print("Current time (UTC+2): ");
  Serial.println(timeClient.getFormattedTime());
  delay(1000);  // Display time every second
  */
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  if (myTimer1.isReady()) { // Timer is complite
   float temp = 23.44;
    float rel_hum = 41.44;   
    if (temp < 23 || rel_hum < 60) {
      bot.sendMessage(CHAT_ID, "В серверной все ОК", "");
      String readings = getReadings();
      bot.sendMessage(CHAT_ID, readings, "");
      bot.sendMessage(CHAT_ID_GROUP, "В серверной все ОК", "");
      bot.sendMessage(CHAT_ID_GROUP, readings, "");
      }
  }
}

//////////////////////////////////////
void syncNTP()

{   
  // Tracks the last time the Wi-Fi was connected for NTP sync
  static unsigned long previousMillis = 0;       
  //static const unsigned long interval = 3600000; // sync with NTP every 1 hour
  static const unsigned long interval = 600000; // sync with NTP every 1 hour
  unsigned long currentMillis = millis();
  // Check if 1 hour has passed since the last Wi-Fi connection
  if (currentMillis - previousMillis >= interval)
  {
     previousMillis = currentMillis;
        // Connect to Wi-Fi
        Serial.print("Connecting to WiFi...");
        while (WiFiMulti.run() != WL_CONNECTED)
        {
            delay(1000);
            Serial.print(".");
        }
        Serial.println();
        Serial.println("Connected to WiFi");
        // Sync time with NTP
        timeClient.update();
        Serial.print("NTP synced time (UTC+2): ");
        Serial.println(timeClient.getFormattedTime());
        // Disconnect from Wi-Fi to save power
        ///WiFi.disconnect(true);
        ///WiFi.mode(WIFI_OFF);  // Ensure Wi-Fi is fully powered down
        ///Serial.println("WiFi disconnected.");

    }

}