
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h> 

#define DHTTYPE DHT11 

const char* WIFI_SSID     = "";
const char* WIFI_PASSWORD = "";

const int ECHO = 4;       // D2
const int TRIGGER = 5;    // D1
const int DHTPIN = 2;     // D4

const String OPENHAB_DISTANCE_URL = "http://openhab/rest/items/SoftenerTankDistance";
const String OPENHAB_TEMPERATURE_URL = "http://openhab/rest/items/UtilityRoom_Temperature";
const String OPENHAB_HUMIDITY_URL = "http://openhab/rest/items/UtilityRoom_Humidity";

DHT dhtSensor(DHTPIN, DHTTYPE);

void setup(void){
  pinMode(ECHO, INPUT);
  pinMode(TRIGGER, OUTPUT);
  
  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connected to:\t");
  Serial.print(WiFi.SSID());
  Serial.print("\nIP address:\t\n");
  Serial.print(WiFi.localIP());
}

void loop(void){
  Serial.print("Posting distance to endpoint\n");
  postToHttpEndpoint(OPENHAB_DISTANCE_URL, String(getDistance()));
  postToHttpEndpoint(OPENHAB_TEMPERATURE_URL, String(getTemperature()));
  postToHttpEndpoint(OPENHAB_HUMIDITY_URL, String(getHumidity()));
//  ESP.deepSleep(600e6);
  delay(600000);
}

float getDistance() {
  float distance;

  digitalWrite(TRIGGER, LOW);  
  delayMicroseconds(2); 
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10); 
  
  digitalWrite(TRIGGER, LOW);
  distance = pulseIn(ECHO, HIGH) / 58;

  Serial.print(distance);
  Serial.print("Centimeters\n");

  return distance;
}

float getTemperature(){
  float temperatureF = dhtSensor.readTemperature(true);
  if (isnan(temperatureF)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    return -1;  
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperatureF);
    Serial.print("\n");
    return temperatureF;
  }
}

float getHumidity() {
  float humidity = dhtSensor.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    return -1;  
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("\n");
    return humidity;
  }
}

void postToHttpEndpoint(String postURL, String postValue) {
  HTTPClient http;
  if (http.begin(postURL)) {
    Serial.print("HTTP POST\n");
    http.addHeader("Content-Type", "text/plain");
    http.addHeader("Accept", "application/json");

    int httpCode = http.POST(postValue);
    
    if (httpCode > 0) {
      Serial.printf("HTTP POST response code: %d\n", httpCode);
    
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.print(payload);
        Serial.print("\n");
      }
    } else {
      Serial.printf("HTTP POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.printf("HTTP Unable to connect\n");
  }
}
