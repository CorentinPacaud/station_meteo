#include "clock.service.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "config.h"

const char* ssid = "CorentinP";
const char* password = "CorentinPBBox";

void ClockService::getData() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(10000);
    http.begin(client, TIME_URL);
    int httpCode = http.GET();
    if (httpCode > 0) {                  // Check the returning code
        String json = http.getString();  // Get the request response payload
        // Serial.println(json);            // Print the response payload

        const size_t capacity = JSON_OBJECT_SIZE(15) + 350;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, json);
        int day_of_week = doc["day_of_week"];  // 6
        int intcDayStr = day_of_week;
        const char* datetime = doc["datetime"];  // "2019-08-31T02:12:43.056352+02:00"
        String dateTimeStr = String(datetime);
        int indexYear = dateTimeStr.indexOf("-");
        int cYear = dateTimeStr.substring(0, indexYear).toInt();
        int indexMonth = dateTimeStr.indexOf("-", indexYear + 1);
        int cMonth = dateTimeStr.substring(indexYear + 1, indexMonth).toInt();
        int indexDay = dateTimeStr.indexOf("T", indexMonth + 1);
        int cDay = dateTimeStr.substring(indexMonth + 1, indexDay).toInt();
        int indexHour = dateTimeStr.indexOf(":", indexDay + 1);
        int cHour = dateTimeStr.substring(indexDay + 1, indexHour).toInt();
        int indexMin = dateTimeStr.indexOf(":", indexHour + 1);
        int cMin = dateTimeStr.substring(indexHour + 1, indexMin).toInt();
        int indexSec = dateTimeStr.indexOf(".", indexMin + 1);
        int intcSec = dateTimeStr.substring(indexMin + 1, indexSec).toInt();
        this->_clock->setTime(cHour, cMin);
        this->_clock->setDate(cYear, cMonth, cDay, day_of_week);
        Serial.println(this->_clock->timeToText().c_str());
    } else {
        Serial.print("Status ERROR : ");
        Serial.println(http.errorToString(httpCode).c_str());
        Serial.println(httpCode);
    }
    http.end();
}