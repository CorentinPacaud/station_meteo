#include <CSV_Parser.h>

// ESP-32
#define EPD_CS SS
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <WiFi.h>
#include <math.h>

#include "clock.h"
#include "clock.service.h"
#include "config.h"
#include "screen.h"
#include "weather.h"

#define DHTPIN 22      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22  // there are multiple kinds of DHT sensors

Clock myClock;
Weather *weather = new Weather();
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(EPD_CS, 17, 16, 4));
Screen *myScreen = new Screen(&myClock, weather, display);
ClockService *clockService = new ClockService(&myClock, weather);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    myScreen->init();

    WiFi.begin(SSID, SSID_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    while (true) {
        readTemperature();
        clockService->getData();

        myScreen->refreshFull();
        for (int min = 0; min < 5; min++) {
            for (int i = 0; i < 60; i++) {
                delay(1000);
            }
            myClock.addOneMinute();
            myScreen->refreshClock();
        }
    }
}

void loop() {
    /*  Serial.println(
         "TEST"); */
}

void readTemperature() {
    dht.begin();

    weather->_indoorTemperature._humidity = int(round(dht.readHumidity()));
    // Read temperature as Celsius (the default)
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    // Check if any reads failed and exit early (to try again).
    Serial.print("TEMP: ");
    Serial.println(temp);
    Serial.print("HUM: ");
    Serial.println(hum);
    if (isnan(temp) || isnan(temp) || isnan(temp)) {
        Serial.println("Failed to read from DHT sensor!");
        weather->_indoorTemperature._currentTemperature = 99;
        weather->_indoorTemperature._humidity = 99;
        return;
    }
    weather->_indoorTemperature._currentTemperature = int(round(temp));
    weather->_indoorTemperature._humidity = int(round(hum));

    uploadTempHum(temp, hum);
    getMinMaxTemp(THINGSPEAK_GET24H_FIELD1, &weather->_indoorTemperature._maxTemperature, &weather->_indoorTemperature._minTemperature);
}

void uploadTempHum(float temperature, float humidity) {
    WiFiClient client;
    HTTPClient http;

    // POST TEMP
    http.setTimeout(5000);
    http.begin(client, String(THINGSPEAK_POST) + "&field1=" + temperature + "&field3=" + humidity);
    int httpCode = http.GET();  // Send the request
    if (httpCode == 200) {
        // SUCCESS
        Serial.println("POST T SUCCESS");

    } else {
        Serial.println("POST T Error : " + httpCode);
    }
    http.end();  // Close connection
}

void getMinMaxTemp(String url, int *maxTemp, int *minTemp) {
    Serial.print("getMinMaxTemp : ");
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(15000);
    // Serial.println("URL: "+THINGSPEAK_GET24H_FIELD1);
    http.begin(client, url);
    int httpCode = http.GET();  // Send the request
    if (httpCode == HTTP_CODE_OK) {
        // SUCCESS
        Serial.println("Success get Last 24h");
        getMaxMin(http, maxTemp, minTemp);
    } else {
        Serial.println("POST T Error : " + httpCode);
        // drawNoWifi();
    }
    http.end();  // Close connection
}

void getMaxMin(HTTPClient &http, int *tempMax, int *tempMin) {
    int maxTemp = -99;
    int minTemp = 99;

    /* StaticJsonDocument<48> filter;
    filter["feeds"][0]["field1"] = true;

    DynamicJsonDocument doc(4096);

    DeserializationError error = deserializeJson(doc, http.getString(), DeserializationOption::Filter(filter));

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    for (JsonObject feed : doc["feeds"].as<JsonArray>()) {
        const char *feed_field1 = feed["field1"];  // "29.70", "29.80", "29.80", "29.80", "29.90", "30.00", ...
        Serial.println(feed_field1);
        const int temp = atoi(feed_field1);
        if (temp < minTemp) minTemp = temp;
        if (temp > maxTemp) maxTemp = temp;
    } */
    String results = http.getString();
    /* char csv_str[results.length()];
    results.toCharArray(csv_str, results.length()); */

    CSV_Parser cp(results.c_str(), /*format*/ "--f", true);

    // retrieving parsed values (and casting them to correct types,
    // corresponding to format string provided in constructor above)
    // char **created_at = (char **)cp[0];
    // char **entry_id = (char **)cp[1];
    float *floats = (float *)cp["field1"];

    for (int i = 0; i < cp.getRowsCount(); i++) {
        if (floats[i] < minTemp) minTemp = floats[i];
        if (floats[i] > maxTemp) maxTemp = floats[i];
    }

    *tempMax = maxTemp;
    *tempMin = minTemp;
    Serial.println((String) "Max: " + maxTemp + " Min: " + minTemp);
}
