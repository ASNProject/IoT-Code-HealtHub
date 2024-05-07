## IoT Code HealtHub

### Library
- ArduinoJson
- HTTPClient
- PulseSensorPlayground
- MAX30100
- MLX90614

### How To Use
- Change Wifi SSID and Password
```
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "password"
```
- Change this code with your local ip address to all code 
```
  http.begin(wifiClient, "http://192.168.1.2/healthub/rest_api/api.php/update");
```
