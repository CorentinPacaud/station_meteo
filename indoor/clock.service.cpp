#include "clock.service.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "DTime.h"
#include "config.h"

const char* ssid = "CorentinP";
const char* password = "CorentinPBBox";

void ClockService::getDate() {
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
        // Serial.println(this->_clock->timeToText().c_str());
    } else {
        Serial.print("Status ERROR : ");
        Serial.println(http.errorToString(httpCode).c_str());
        Serial.println(httpCode);
    }
    http.end();
}

const int STORM = 0, RAIN = 1, SNOW = 2, FOG = 3, SUN = 4, SUNNY_CLOUD = 5, CLOUD = 6, DRIZZLE = 7;

int getWeatherCode(int weather) {
    if (weather >= 200 && weather <= 232) return STORM;
    if (weather >= 300 && weather <= 321) return DRIZZLE;
    if (weather >= 500 && weather <= 531) return RAIN;
    if (weather >= 600 && weather <= 622) return SNOW;
    if (weather >= 700 && weather <= 781) return FOG;
    if (weather == 800) return SUN;
    if (weather >= 801 && weather <= 802) return SUNNY_CLOUD;
    if (weather >= 803 && weather <= 804) return CLOUD;
    return 7;
}

void deserializeJsonOpenWeather(Clock* clock, Weather* weather, String input) {
    StaticJsonDocument<304> filter;
    filter["timezone_offset"] = true;

    JsonObject filter_current = filter.createNestedObject("current");
    filter_current["dt"] = true;
    filter_current["sunrise"] = true;
    filter_current["sunset"] = true;
    filter_current["temp"] = true;
    filter_current["humidity"] = true;
    filter_current["weather"][0]["id"] = true;

    JsonObject filter_daily_0 = filter["daily"].createNestedObject();

    JsonObject filter_daily_0_temp = filter_daily_0.createNestedObject("temp");
    filter_daily_0_temp["day"] = true;
    filter_daily_0_temp["min"] = true;
    filter_daily_0_temp["max"] = true;
    filter_daily_0["weather"][0]["id"] = true;

    DynamicJsonDocument doc(1536);

    DeserializationError error = deserializeJson(doc, input, DeserializationOption::Filter(filter));

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // float lat = doc["lat"];                       // 46.3333
    // float lon = doc["lon"];                       // 5.1333
    // const char *timezone = doc["timezone"];       // "Europe/Paris"
    int timezone_offset = doc["timezone_offset"];  // 3600

    JsonObject current = doc["current"];
    long current_dt = current["dt"];            // 1612992998
    long current_sunrise = current["sunrise"];  // 1612939831
    long current_sunset = current["sunset"];    // 1612976221
    long tempExtCurr = current["temp"];         // 271.87

    // float current_feels_like = current["feels_like"]; // 267.61
    // int current_pressure = current["pressure"];       // 1016
    long humExtCurr = current["humidity"];  // 88

    // float current_dew_point = current["dew_point"];   // 270.34
    // int current_uvi = current["uvi"];                 // 0
    // int current_clouds = current["clouds"];           // 98
    // int current_visibility = current["visibility"];   // 10000
    // float current_wind_speed = current["wind_speed"]; // 2.68
    // int current_wind_deg = current["wind_deg"];       // 9
    // float current_wind_gust = current["wind_gust"];   // 5.36

    JsonObject current_weather_0 = current["weather"][0];
    int current_weather_0_id = current_weather_0["id"];  // 804
    // const char *current_weather_0_main = current_weather_0["main"];               // "Clouds"
    // const char *current_weather_0_description = current_weather_0["description"]; // "overcast clouds"
    // const char *current_weather_0_icon = current_weather_0["icon"];               // "04n"

    JsonArray daily = doc["daily"];

    JsonObject daily_0 = daily[1];
    // long daily_0_dt = daily_0["dt"];           // 1612954800
    // long daily_0_sunrise = daily_0["sunrise"]; // 1612939831
    // long daily_0_sunset = daily_0["sunset"];   // 1612976221

    JsonObject daily_0_temp = daily_0["temp"];
    float daily_0_temp_day = daily_0_temp["day"];  // 279.69
    float daily_0_temp_min = daily_0_temp["min"];  // 271.1
    float daily_0_temp_max = daily_0_temp["max"];  // 280.05
    // float daily_0_temp_night = daily_0_temp["night"]; // 271.1
    // float daily_0_temp_eve = daily_0_temp["eve"];     // 274.18
    // float daily_0_temp_morn = daily_0_temp["morn"];   // 278.77

    // JsonObject daily_0_feels_like = daily_0["feels_like"];
    // float daily_0_feels_like_day = daily_0_feels_like["day"];     // 277.51
    // float daily_0_feels_like_night = daily_0_feels_like["night"]; // 267.07
    // float daily_0_feels_like_eve = daily_0_feels_like["eve"];     // 270.4
    // float daily_0_feels_like_morn = daily_0_feels_like["morn"];   // 276.15

    // int daily_0_pressure = daily_0["pressure"];       // 1004
    // int daily_0_humidity = daily_0["humidity"];       // 90
    // float daily_0_dew_point = daily_0["dew_point"];   // 278.2
    // float daily_0_wind_speed = daily_0["wind_speed"]; // 1.51
    // int daily_0_wind_deg = daily_0["wind_deg"];       // 227

    JsonObject daily_0_weather_0 = daily_0["weather"][0];
    int daily_0_weather_0_id = daily_0_weather_0["id"];  // 501
    // const char *daily_0_weather_0_main = daily_0_weather_0["main"];               // "Rain"
    // const char *daily_0_weather_0_description = daily_0_weather_0["description"]; // "moderate rain"
    // const char *daily_0_weather_0_icon = daily_0_weather_0["icon"];               // "10d"

    // int daily_0_clouds = daily_0["clouds"]; // 100
    // int daily_0_pop = daily_0["pop"];       // 1
    // float daily_0_rain = daily_0["rain"];   // 6.73
    // float daily_0_uvi = daily_0["uvi"];     // 0.96

    JsonObject daily_1 = daily[2];
    JsonObject daily_1_weather_0 = daily_1["weather"][0];
    int daily_1_weather_0_id = daily_1_weather_0["id"];

    for (JsonObject elem : doc["alerts"].as<JsonArray>()) {
        const char* sender_name = elem["sender_name"];  // "METEO-FRANCE", "METEO-FRANCE"
        const char* event = elem["event"];              // "Moderate snow-ice warning", "Moderate flooding warning"
        long start = elem["start"];                     // 1612969200, 1612969200
        long end = elem["end"];                         // 1613055600, 1613055600
        const char* description = elem["description"];  // "Although rather usual in this region, locally or ...
    }

    //  Serial.printf("Demain, le temps sera %s, il fera %f, min %f et max %f\n", *daily_1_weather_0_main, daily_1_temp_day, daily_1_temp_min, daily_1_temp_max);
    // Serial.printf("Aujourd'hui, il fait %s, temp: %f\n", current_weather_0_description, tempExtCurr);
    // Serial.printf("Demain, il fera %s, temp: %f , min: %f , max: %f \n", daily_0_weather_0_description, daily_0_temp_day, daily_0_temp_min, daily_0_temp_max);
    Serial.printf("Il y a %d alertes météo.\n", doc["alerts"].as<JsonArray>().size());

    clock->setWeather(getWeatherCode(current_weather_0_id));
    clock->setWeatherJ1(getWeatherCode(daily_0_weather_0_id));
    clock->setWeatherJ1(getWeatherCode(daily_1_weather_0_id));

    Serial.print("Weahter: ");
    Serial.println(getWeatherCode(current_weather_0_id));
    JsonObject sys = doc["sys"];
    long sys_sunrise = sys["sunrise"];  // 1567227606
    long sys_sunset = sys["sunset"];    // 1567275757
    long timezone = doc["timezone"];    // 7200
    DTime sunsetT, sunriseT;
    sunsetT.setTimestamp(current_sunset + timezone_offset);
    int sunsetHour = (uint32_t)sunsetT.hour;
    int sunsetMin = (uint32_t)sunsetT.minute;
    clock->setSunset(sunsetHour, sunsetMin);

    sunriseT.setTimestamp(current_sunrise + timezone_offset);
    int sunriseHour = (uint32_t)sunriseT.hour;
    int sunriseMin = (uint32_t)sunriseT.minute;
    clock->setSunrise(sunriseHour, sunriseMin);

    weather->setOutdoorTemperature(tempExtCurr, static_cast<int>(std::round(daily_0_temp_max)), static_cast<int>(std::round(daily_0_temp_min)));
    weather->_outdoorTemperature._humidity = humExtCurr;
}

void ClockService::getWeather() {
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(10000);
    http.begin(client, OPENWEATHER_ONE_CALL);
    int httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0) {  // Check the returning code
        String json = http.getString();
        deserializeJsonOpenWeather(this->_clock, this->_weather, json);
    } else {
        Serial.println("ERROR open weather onecall");
    }
    http.end();
}

void ClockService::getData() {
    getDate();
    getWeather();
}