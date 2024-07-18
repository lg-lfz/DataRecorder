#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include <LittleFS.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "bme280.h"
#include "rtc.h"
#include "applicationstate.h"
#include "data.h"
#include "file.h"
#include "html.h"

constexpr const auto SCL_PIN = D1;
constexpr const auto SDA_PIN = D2;
constexpr const auto I2CADDRESS = 0x76;
constexpr const auto SEALEVELPRESSURE_HPA = 1013.25;
constexpr const auto analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0
constexpr const auto DNS_PORT = 53;
constexpr const char *DATA_FILENAME = "data.csv";
constexpr const char *BASE_SSID = "DataRecorder";

constexpr const float constantA = 5.7331;
constexpr const float constantB = 1701.3;

constexpr const long interval1 = 500;
constexpr const long interval2 = 1000;

unsigned long previousMillis1 = 0; // Stores the last time an event occurred
unsigned long previousMillis2 = 0; // Stores the last time an event occurred



RTC_DS3231 rtc;

Adafruit_BME280 bme;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

File file;
String index_html = "";
ApplicationState appState;

void setup()
{
  appState.dataRecording = false;
  // Set A0 to INPUT mode for analog reading
  pinMode(analogInPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  setlocale(LC_ALL, "de_DE.UTF-8");
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(2000);
  Serial.println("Hello console...");

  initBME280(I2CADDRESS, bme);
  initRTC3231(rtc);

  initFileSystem();
  listDir("/", 0);

  initAccessPoint(IPAddress(10, 10, 10, 10), IPAddress(10, 10, 10, 10), IPAddress(255, 255, 255, 0), BASE_SSID);
  initWebServerWithSocket();
}

void loop()
{
  unsigned long currentMillis = millis();
  SensorData data;
  // Task 1 -> every 500ms
  if (currentMillis - previousMillis1 >= interval1)
  {
    // Save the last time task 1 was executed
    previousMillis1 = currentMillis;

    // Perform task 1
    auto data = collectData();
    sendDataToClient(data);
  }

  // Task 2 -> every 1000ms
  if (currentMillis - previousMillis2 >= interval2)
  {
    // Save the last time task 2 was executed
    previousMillis2 = currentMillis;

    // Perform task 2
    if (appState.dataRecording)
    {
      auto file_data = storeData(data, appState);
      sendDataToClient(file_data);
    }
  }
}

float getGasVolume(int current)
{
  float volume = ((constantA * current) - constantB);
  Serial.printf("Current value=%d Gas Volume: %0.2f mL\n", current, volume);
  return volume;
}

SensorData collectData()
{
  SensorData data = {};
  data.pressure = bme.readPressure() / 100.0;             // pressure in hPa
  data.altitude = bme.readAltitude(SEALEVELPRESSURE_HPA); // in m
  data.temperature = bme.readTemperature();               // in °C
  data.gas = getGasVolume(analogRead(analogInPin));       // in mL
  auto time_date = read_time(rtc);
  data.day = time_date.Day;
  data.month = time_date.Month;
  data.year = time_date.Year;
  data.hour = time_date.Hour;
  data.minute = time_date.Minute;
  data.second = time_date.Second;
  return data;
}

void print2digits(int number)
{
  if (number >= 0 && number < 10)
  {
    Serial.write('0');
  }
  Serial.print(number);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    DynamicJsonDocument doc(len);
    DeserializationError error = deserializeJson(doc, (char *)data);
    if (doc.containsKey("action") && doc["action"].as<String>().equals("toggle"))
    {
      appState.dataRecording = !appState.dataRecording;

      if (!appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stop\"}");
      }
      else
      {
        // Always create new file...
        writeFile(DATA_FILENAME, getFirstCSVLine().c_str());
        ws.textAll("{\"state\":\"start\"}");
      }
    }
    else if (doc.containsKey("action") && doc["action"].as<String>().equals("request"))
    {
      if (!appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stopped\"}");
      }
      else
      {
        ws.textAll("{\"state\":\"started\"}");
      }
    }
    else if (doc.containsKey("action") && doc["action"].as<String>().equals("format"))
    {
      // stop recording
      if (appState.dataRecording)
      {
        ws.textAll("{\"state\":\"stop\"}");
      }
      // format LittleFS Filesystem !Reboot!
      formatFileSystem();
    }
    else if (doc.containsKey("date_time"))
    {
      String iso8601DateTime = doc["date_time"].as<String>();
      setDateTimeFromISO8601(iso8601DateTime.c_str(), rtc);
      ws.textAll("{\"date_time\":\"success\"}");
    }
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    Serial.printf("Data received: %s\n", data);
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    Serial.printf("WebSocket Error: %s\n", (char *)arg);
    client->close();
    break;
  }
}

void initWebServerWithSocket()
{
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", htmlContent); });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, DATA_FILENAME, String(), true); });
  server.begin();
}

template <typename T>
void sendDataToClient(const T &data)
{
  ws.printfAll(getJSON(data).c_str());
}

FileData storeData(const SensorData &data, const ApplicationState &appState)
{
  FileData file_data;
  file_data.filename = DATA_FILENAME;
  digitalWrite(LED_BUILTIN, HIGH);
  char buffer[255];
  // Format the date and time into ISO 8601 format
  snprintf(buffer, sizeof(buffer), "%0.2f;%0.2f;%0.2f;%0.2f;%s;", data.altitude, data.pressure, data.temperature, data.gas, formatISO8601(data.year, data.month, data.day, data.hour, data.minute, data.second).c_str());
  file_data.filesize = appendFile(DATA_FILENAME, buffer);
  file_data.free_space = getAvalibleDiskSpace();
  // readFile(DATA_FILENAME); //just for debug purpose...
  digitalWrite(LED_BUILTIN, LOW);
  return file_data;
}

void initAccessPoint(const IPAddress &localIP, const IPAddress &gatewayIP, const IPAddress &netmask, const String &ssid)
{
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", localIP);
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAPConfig(localIP, gatewayIP, netmask))
  {
    Serial.println("Wifi configuration not successful.");
  }
  auto ip = WiFi.softAPIP();
  String mac = WiFi.macAddress();
  Serial.printf("Mac: %s, IP: %s\n", mac.c_str(), ip.toString().c_str());
  mac.replace(":", "");
  auto custom_ssid = ssid + "-" + mac;
  Serial.printf("SSID: %s\n", custom_ssid.c_str());
  WiFi.softAP(custom_ssid);
}
