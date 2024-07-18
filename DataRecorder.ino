#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
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

const int analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0

#define SCL_PIN D1
#define SDA_PIN D2
#define I2CADDRESS 0x76

#define SEALEVELPRESSURE_HPA 1013.25
#define DELAY 750

RTC_DS3231 rtc;

Adafruit_BME280 bme;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

File file;
String index_html = "";
ApplicationState appState;

void setup()
{
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

  initAccessPoint(IPAddress(10, 10, 10, 10), IPAddress(10, 10, 10, 10), IPAddress(255, 255, 255, 0), "GasCollector" + WiFi.macAddress());
  initWebServerWithSocket();
}

void loop()
{
  SensorData data = {};
  data.pressure = bme.readPressure() / 100.0;             // pressure in hPa
  data.altitude = bme.readAltitude(SEALEVELPRESSURE_HPA); // in m
  data.temperature = bme.readTemperature();               // in °C
  auto time_date = read_time(rtc);
  data.day = time_date.Day;
  data.month = time_date.Month;
  data.year = time_date.Year;
  data.hour = time_date.Hour;
  data.minute = time_date.Minute;
  data.second = time_date.Second;
  data.measurement_value = analogRead(analogInPin);
  sendDataToClient(data);
  printData(data);
  storeData(data, appState);
  ws.cleanupClients();
  delay(DELAY);
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
    if (strcmp((char *)data, "toggle") == 0)
    {
      appState.dataRecording = !appState.dataRecording;

      if (!appState.dataRecording)
      {
        Serial.println("File closed");
        ws.textAll("{\"state\":\"stop\"}");
      }
      else
      {
        file = LittleFS.open("data.csv", "w");
        file.print("Hoehe; Druck; Temp\n");
        Serial.println("File opened");
        ws.textAll("{\"state\":\"start\"}");
      }
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
            {
      file.close();
      request->send(LittleFS, "data.csv", String(), true); });
  server.begin();
}

String processor(const String &var)
{
  Serial.println(var);
  return String();
}

void sendDataToClient(const SensorData &data)
{
  ws.printfAll(getJSON(data).c_str());
}

void printData(const SensorData &data)
{
  // Serial.printf("%s\n", getJSON(data).c_str());
}

void storeData(const SensorData &data, const ApplicationState &appState)
{
  if (appState.dataRecording)
  {
    if (file.printf("%0.2f; %0.2f; %0.2f; %0.2f; %s;\n", data.altitude, data.pressure, data.temperature, data.measurement_value, formatISO8601(data.year, data.month, data.day, data.hour, data.minute, data.second).c_str()))
    {
    }
    else
    {
      Serial.println("- Schreiben fehlgeschlagen");
    }
  }
}

void initAccessPoint(const IPAddress &localIP, const IPAddress &gatewayIP, const IPAddress &netmask, const String &ssid)
{
  if (!WiFi.softAPConfig(localIP, gatewayIP, netmask))
  {
    Serial.println("WLAN Konfiguration nicht erfolgreich.");
  }
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  WiFi.softAP(ssid);
}
