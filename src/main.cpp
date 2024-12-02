/*
ESP32 OTA using PlatformIO
Refer to:
https://lonelybinary.com/blogs/learn/esp32-ota-using-platformio
*/
/*
    Reqruired Library: NTPClient

    https://github.com/arduino-libraries/NTPClient
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define WIFI_SSID "TP-LINK_17"
#define WIFI_PASS "rob17rob17"

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


void syncNTP();
/////////////////////////

void setup()
{
  Serial.begin(115200);
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
  // Optional Display current time (last synced, updated every second)
  Serial.print("Current time (UTC+2): ");
  Serial.println(timeClient.getFormattedTime());
  delay(1000);  // Display time every second
}


//////////////////////////////////////
void syncNTP()

{   
  // Tracks the last time the Wi-Fi was connected for NTP sync
  static unsigned long previousMillis = 0;       
  //static const unsigned long interval = 3600000; // sync with NTP every 1 hour
  static const unsigned long interval = 60000; // sync with NTP every 1 hour
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

        Serial.print("NTP synced time (UTC+3): ");
        Serial.println(timeClient.getFormattedTime());
        // Disconnect from Wi-Fi to save power
        ///WiFi.disconnect(true);
        ///WiFi.mode(WIFI_OFF);  // Ensure Wi-Fi is fully powered down
        ///Serial.println("WiFi disconnected.");

    }

}