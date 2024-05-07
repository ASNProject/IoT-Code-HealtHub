#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#endif
#include <ArduinoJson.h>

WiFiClient wifiClient;


#define WIFI_SSID "VIVO 1906"
#define WIFI_PASSWORD "nggongapasit"

//----------------- Pulse Sensor -------------------//
#include <PulseSensorPlayground.h>
const int PulseWire = 0;
int Threshold = 550;
PulseSensorPlayground pulseSensor;
int myBPM;

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
}

void loop(){
  if (WiFi.status() == WL_CONNECTED)
  {
    fetchDataPasien();
    checkHeartbeat();
    if (!namaData.isEmpty() && !nikData.isEmpty()) {
      putData();
    }
    delay(1000);
  }
}

void checkHeartbeat(){
  if (pulseSensor.sawStartOfBeat()) {           
    myBPM = pulseSensor.getBeatsPerMinute();  
                                               
    // Serial.println("♥  A HeartBeat Happened ! "); 
    // Serial.print("BPM: ");                        
    // Serial.println(myBPM);                        
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

    // Serial.print("Nama: ");
    // Serial.println(namaData);
    // Serial.print("NIK: ");
    // Serial.println(nikData);
    
    // namaData = "";
    // nikData = "";

  } else {
    Serial.println("Tidak ada data");
  }
  http.end();
}

void putData() {
  String jsonData = "{\"nama\":\"" + namaData + "\",\"nik\":\"" + nikData + "\",\"sensor1\":" + String(myBPM) + ",\"sensor2\":" + String(myBPM) + "}";
  HTTPClient http;
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/heartrates");
  int httpCode = http.POST(jsonData);

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_NO_CONTENT) {
      Serial.println("Data berhasil diperbarui");
    } else {
      Serial.println("Gagal memperbarui data");
    }
  } 
  else 
  {
    Serial.println("Tidak dapat terhubung ke server");
  }

  http.end();
}
