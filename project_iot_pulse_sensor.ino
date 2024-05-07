#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#endif
#include <ArduinoJson.h>
#include <Wire.h>

WiFiClient wifiClient;


#define WIFI_SSID "VIVO 1906"
#define WIFI_PASSWORD "nggongapasit"

//----------------- Pulse Sensor -------------------//
#include <PulseSensorPlayground.h>
const int PulseWire = 0;
int Threshold = 550;
PulseSensorPlayground pulseSensor;
int myBPM;

//------------------ MAX30100 ----------------------//
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;
uint32_t tsLastReport = 0;
int bpm;
int spo2;

//------------------- MLX90614 ---------------------//
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
int temperature;

//----------------- Fetch Data ---------------------//
String namaData, nikData;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //-------------- Pulse Sensor ------------------//
  pulseSensor.analogInput(PulseWire);
  pulseSensor.setThreshold(Threshold);
  if (pulseSensor.begin()) {
    Serial.println("PulseSensor Object !");
  }

  //--------------- MAX30100 --------------------//
  if (!pox.begin()) {
    Serial.println("FAILED");
    for(;;);
  } else {
    Serial.println("SUCCESS");
  }

  //--------------- MLX90614 -------------------//
  while (!Serial);
  if (!mlx.begin()) {
    while (1);
  }
}

void loop(){
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    bpm = pox.getHeartRate();
    spo2 = pox.getSpO2();
    temperature = mlx.readObjectTempC();

    if (WiFi.status() == WL_CONNECTED)
    {
      fetchDataPasien();

      if (!namaData.isEmpty() && !nikData.isEmpty()) {
        postHearRateData();
        postSaturationData();
        postTemperatureData();
      }
    }
      tsLastReport = millis();
  }

}

void checkHeartbeat(){
  if (pulseSensor.sawStartOfBeat()) {           
    myBPM = pulseSensor.getBeatsPerMinute();                  
  }
  delay(20);                    
}

void fetchDataPasien(){
  HTTPClient http;
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/update");
  int httpCode = http.GET();

  if (httpCode > 0) 
  {
    const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(http.getString());

    int id = root["id"];
    namaData = root["nama"].asString();
    nikData = root["nik"].asString();

  } else {
    Serial.println("Tidak ada data");
  }
  http.end();
}

void postHearRateData() {
  String jsonData = "{\"nama\":\"" + namaData + "\",\"nik\":\"" + nikData + "\",\"sensor1\":" + String(bpm) + ",\"sensor2\":" + String(bpm) + "}";
  HTTPClient http;
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/heartrates");
  int httpCode = http.POST(jsonData);
  http.end();
}

void postSaturationData() {
  String jsonData = "{\"nama\":\"" + namaData + "\",\"nik\":\"" + nikData + "\",\"sensor1\":" + String(spo2) + ",\"sensor2\":" + String(spo2) + "}";
  HTTPClient http;
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/saturation");
  int httpCode = http.POST(jsonData);
  http.end();
}

void postTemperatureData() {
  String jsonData = "{\"nama\":\"" + namaData + "\",\"nik\":\"" + nikData + "\",\"sensor1\":" + String(temperature) + ",\"sensor2\":" + String(temperature) + "}";
  HTTPClient http;
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/suhu");
  int httpCode = http.POST(jsonData);
  http.end();
}
