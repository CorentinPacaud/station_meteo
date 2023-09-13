/*
 * To start the project, you need to create a config.h file containg the
 following param : *	 #define SSID "YOU_SSID" *	 #define SSID_PASSWORD
 "YOU_PASSWORD"

 *	 #define OPENWEATHER_URL
 "http://api.openweathermap.org/data/2.5/weather?q=<yourCity>&units=metric&APPID=<yourAppId>"
 *	 #define TIME_URL "http://worldtimeapi.org/api/ip"
 *
 *	 #define THINGSPEAK_POST
 "http://api.thingspeak.com/update?api_key=<yourKey>" *	 #define
 THINGSPEAK_GETLAST_FIELD2
 "http://api.thingspeak.com/channels/<channelId>/fields/2/last.csv?key=<yourKey>"
 // EXT TEMP *	 #define THINGSPEAK_GETLAST_FIELD4
 "http://api.thingspeak.com/channels/<channelId>/fields/4/last.csv?key=<yourKey>"
 // WET OUT *	 #define THINGSPEAK_GET24H_FIELD1
 "http://api.thingspeak.com/channels/<channelId>/fields/1.csv?key=<yourKey>&timezone=Europe/Paris&results=288"
 *	 #define THINGSPEAK_GET24H_FIELD2
 "http://api.thingspeak.com/channels/<channelId>/fields/2.csv?key=<yourKey>&timezone=Europe/Paris&results=288"
 *
 *
 * You also need to create an account on thingspeak and create 1 channel and 4
 field : 1: Indoor temp, 2: outdoor temp, 3: indoor humidity, 4: outdoor
 humidity
 *
 * SCREEN
 * BUSY (violet) => D2
 * RST (blanc)   => D4
 * DC (vert)     => D3
 * CS (orange)   => D8
 * CLK (jaune)   => D5
 * DIN (bleu)    => D7
 * GND (noir)    => gnd
 * VCC (rouge)   => 3.3v
 *
 * DHT22
 * DATA => D1
 *
 */

#include <Preferences.h>

#define RTCMEMORYSTART 66
#define RTCMEMORYLEN 125

#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD2_BW.h>
// #include <GxEPD2_3C.h>
// #include <GxEPD2_7C.h>
// #include <GxEPD.h>
#include <GxFont_GFX.h>
// #include <GxGDEW042T2/GxGDEW042T2.h> // 4.2" b/w    // 4.2" b/w 400 x 300 px
#include <GxIO/GxIO.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <math.h>

#include "DTime.h"
// FreeFonts from Adafruit_GFX
#include <ArduinoJson.h>

#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
// #include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <math.h>

#include "background.c"
#include "config.h"
#include "font36.h"

// #include "DHT.h"
#include "DHTesp.h"

#define DHTPIN 5      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // there are multiple kinds of DHT sensors

#include <SPI.h>

// #define DEBUG
#define DEBUG

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif // __arm__

int freeMemory()
{
     char top;
#ifdef __arm__
     return &top - reinterpret_cast<char *>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
     return &top - __brkval;
#else  // __arm__
     return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}
/*
typedef struct
{
     // min, hour, d,m,y
     uint32_t minutes;
     uint32_t hours;
     uint32_t days;
     uint32_t months;
     uint32_t years;
     uint32_t day;
     uint32_t weather;
     uint32_t tempIntCurr;
     uint32_t tempIntMax;
     uint32_t tempIntMin;
     uint32_t tempExtCurr;
     uint32_t tempExtMax;
     uint32_t tempExtMin;
     uint32_t humIntCurr;
     uint32_t humExtCurr;
     uint32_t sunriseHour;
     uint32_t sunriseMin;
     uint32_t sunsetHour;
     uint32_t sunsetMin;
     uint32_t weatherJ1;
     uint32_t weatherJ2;
} rtcStore;
 */
Preferences preferences;

typedef struct
{
     const uint8_t PIN;
     uint32_t numberKeyPresses;
     bool pressed;
} Button;

typedef struct
{
     int x;
     int y;
     int width;
     int height;
} PositionBox;

GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2),
                              // selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
// GxEPD_Class display(io); // default selection of D4(=2), D2(=4)

const char *ssid = SSID;
const char *password = SSID_PASSWORD;

const char *headerKeys[] = {"Date"};

// TIME
// char timeUrl[] = TIME_URL;
const unsigned int GMT_D = 1;
unsigned int cHour = 0;
unsigned int cMin = 0;
unsigned int cSec = 0;
unsigned int cDayStr = 0;
unsigned int cDay = 1;
unsigned int cMonth = 1;
unsigned int cYear = 1;

const String MONTHS[] = {"Jan", "Fev", "Mars", "Avril", "Mai", "Juin",
                         "Juil", "Aout", "Sept", "Oct", "Nov", "Dec"};
// const String HEADERSMONTHS[]        = {"Jan" , "Feb" , "Mar" , "Apr", "May",
// "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *DAYS[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI",
                      "JEUDI", "VENDREDI", "SAMEDI"};
const int daySpaces[] = {
    0, 30, 30, 15, 30, 10, 20}; // Espace pour centrer le text. lun,mar,...
// const String HEADERSDAYS[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ,
// "Sun"};

// const GFXfont *f = &FreeMonoBold9pt7b;

int tempIntCurr = 0;
int tempIntMax = 0;
int tempIntMin = 0;
int tempExtCurr = 0;
int tempExtMax = 0;
int tempExtMin = 0;
int humIntCurr = 0;
int humExtCurr = 99;

const int STORM = 0, RAIN = 1, SNOW = 2, FOG = 3, SUN = 4, SUNNY_CLOUD = 5, CLOUD = 6, DRIZZLE = 7;
int weather = -1;   // 0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 =
                    // brouillard;  5 = nuages; 6 = soleil;
int weatherJ1 = -1; // 0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 =
                    // brouillard;  5 = nuages; 6 = soleil;
int weatherJ2 = -1; // 0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 =
                    // brouillard;  5 = nuages; 6 = soleil;

int sunsetHour = 0;
int sunsetMin = 0;

int sunriseHour = 0;
int sunriseMin = 0;

bool startUp = true;

int firstRestart = 0;

// DHT dht(DHTPIN, DHTTYPE);

Button buttonResetWifi = {0, 0, false};
bool configSetting = false;

PositionBox timeBox, sunsetBox, sunriseBox, weatherBox, weatherBoxJ1, weatherBoxJ2;
PositionBox dayBox, dateBox;
PositionBox tempIntLogoBox, tempIntCurrBox, tempIntMaxLogoBox, tempIntMinLogoBox, tempIntMaxBox, tempIntMinBox;
PositionBox tempExtCurrBox, tempExtLogoBox, tempExtMaxLogoBox, tempExtMinLogoBox, tempExtMaxBox, tempExtMinBox;
PositionBox humIntCurrBox, humExtCurrBox, humLogoBox;

DHTesp dht;
unsigned long currentMillis;

int layout = 2;

void setup()
{
     timeBox = {0, 0, 150, 200};
     dayBox = {0, 310, 300, 50};
     dateBox = {0, 360, 300, 40};
     sunriseBox = {150, 0, 74, 40};
     sunsetBox = {226, 0, 74, 40};
     weatherBox = {193, 50, 64, 64};
     weatherBoxJ1 = {155, 125, 64, 64};
     weatherBoxJ2 = {230, 125, 64, 64};
     tempIntLogoBox = {0, 200, 40, 40};
     tempIntCurrBox = {40, 200, 110, 40};
     tempIntMaxLogoBox = {0, 245, 11, 30};
     tempIntMinLogoBox = {0, 280, 11, 30};
     tempIntMaxBox = {15, 245, 50, 30};
     tempIntMinBox = {15, 280, 50, 30};
     humIntCurrBox = {70, 250, 65, 40};

     humLogoBox = {140, 260, 20, 29};

     tempExtLogoBox = {260, 200, 40, 40};
     tempExtCurrBox = {150, 200, 110, 40};
     tempExtMaxLogoBox = {280, 245, 11, 30};
     tempExtMinLogoBox = {280, 280, 11, 30};
     tempExtMaxBox = {230, 245, 50, 30};
     tempExtMinBox = {230, 280, 50, 30};
     humExtCurrBox = {165, 250, 65, 40};

     Serial.begin(115200);
     preferences.begin("save");

     pinMode(buttonResetWifi.PIN, INPUT);

     display.init(115200); // enable diagnostic output on Serial
     if (layout == 1)
          display.setRotation(0);
     else if (layout == 2)
          display.setRotation(1);
     // SWITCH OFF WIFI
     WiFi.forceSleepBegin();
     yield();

#ifndef DEBUG
     currentMillis = millis();
     Serial.println(" PROD ");
#endif

     Serial.println();
     Serial.println("setup");

     // display.fillScreen(GxEPD_BLACK);

     int i = 0;
     dht.setup(5, DHTesp::DHT22); // Connect DHT sensor to GPIO 17

     Serial.println("setup done");
     checkStart();
}

// TODO
// TRY THIS :
// ESP.getResetReason()
// } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from
// deep-sleep
//      strcpy_P(buff, PSTR("Deep-Sleep Wake"));

void loop()
{

     Serial.println(
         "LOOP--------------------------------------------------------------------"
         "---");
     if (digitalRead(buttonResetWifi.PIN) == LOW)
     {
          Serial.println("BUTTON PRESSED !!!!!!!!!!!!!!");
          resetParam();
     }

     currentMillis = millis();

#ifdef DEBUG
     Serial.println(" DEBUG ");
#endif

     // Display and Send temp every 5 min;

     if (startUp)
     {
          Serial.println("STARTUP");
     }
     else
     {
          addOneMinute();
     }

     bool bigRefresh = cMin % 5 == 0 || startUp;

     if (bigRefresh)
     {
          Serial.println("BIG UPDATE");
          getRemoteData();
          // display.update();
          startUp = false;
     }

     display.setFullWindow();
     loadDataToDisplay();
     display.firstPage();
     do
     {
          loadDataToDisplay();
     } while (display.nextPage());

     saveData();

     int timeElapsed = millis() - currentMillis;
     if (cMin % 5 == 0 || startUp)
     {
          timeElapsed = 60 - (timeElapsed / 1000) + (30 - cSec);
     }
     else
     {
          timeElapsed = 60 - (timeElapsed / 1000);
     }
     // timeElapsed = 5;
     Serial.print("Time elapsed : ");
     Serial.println(timeElapsed);

#ifndef DEBUG
     String rstReason = ESP.getResetReason();

     // Don't go to Deep sleep for first 5 min if external restart.
     // This is usefull to easily upload code when in deepsleep mod, just press the
     // restart btn and you have 5 min to upload code)
     if (rstReason == "External System" && firstRestart < 0)
     {
          Serial.println("First Restart");
          if (timeElapsed > 0)
               for (int i = 0; i < timeElapsed; i++)
               {
                    delay(1000);
               }
          firstRestart++;
     }
     else
     {
          Serial.println("Start deepSleep");
          display.powerOff();
          if (timeElapsed > 0)
               ESP.deepSleep(timeElapsed * 1000000, WAKE_RFCAL);
          yield();
     }
#else

     for (int i = 0; i < timeElapsed; i++)
     {
          delay(1000);
     }

#endif
}

void getRemoteData()
{
     saveData();
     if (initWifi())
     {
          readTemperature();

          getLastValue(THINGSPEAK_GETLAST_FIELD4, &humExtCurr);
          getLastValue(THINGSPEAK_GETLAST_FIELD2, &tempExtCurr);

          getMinMaxTemp(THINGSPEAK_GET24H_FIELD1, &tempIntMax, &tempIntMin);
          getMinMaxTemp(THINGSPEAK_GET24H_FIELD2, &tempExtMax, &tempExtMin);
          // getSunWeather();
          getOpenWeatherOneCall();
          // getTime();
     }

     WiFi.forceSleepBegin(); // Switch off wifi
     yield();
}

void loadDataToDisplay()
{
     Serial.println("Draw background");
     Serial.println("End Draw background");
     Serial.println("Draw date");
     if (layout == 1)
     {
          drawBackground();
          displayDate();
          // Serial.println("END Draw date");
          // Serial.println("Draw TEmp");
          displayTemperature();
          // Serial.println("End Draw TEmp");
          // Serial.println("Draw Time");
          displayTime();
          // Serial.println("End Draw TIME");
          // Serial.println("Draw weather");
          displayWeather();
          // Serial.println("End Draw weather");
          // Serial.println("Draw Sunsrise");
          displaySunSetRise();
     }
     else if (layout == 2)
     {
          drawBackground2();
          displayDate2();
          displayTemperature2();
          displayTime2();
          displayWeather2();
          displaySunSetRise2();
     }
     // Serial.println("End Draw Sunsrise");
     // Serial.println("UDPATE !");
}

void checkStart()
{
     String rstReason = ESP.getResetReason();
     Serial.print("Start reason: ");
     Serial.println(rstReason);
     if (rstReason == "External System")
     {
          // INIT RTC_MEM
          Serial.println("Init RTC MEM");
          startUp = true;
          saveData();
     }
     else
     {
          // RETREIVE DATA FROM RTC.
          startUp = false;

          readData();
     }
}

void readData()
{
     cMin = preferences.getInt("minutes");
     cHour = preferences.getInt("hours");
     cDay = preferences.getInt("days");
     cMonth = preferences.getInt("months");
     cYear = preferences.getInt("years");
     cDayStr = preferences.getInt("day");
     tempExtCurr = preferences.getInt("tempExtCurr");
     tempExtMax = preferences.getInt("tempExtMax");
     tempExtMin = preferences.getInt("tempExtMin");
     tempIntCurr = preferences.getInt("tempIntCurr");
     tempIntMax = preferences.getInt("tempIntMax");
     tempIntMin = preferences.getInt("tempIntMin");
     humExtCurr = preferences.getInt("humExtCurr");
     humIntCurr = preferences.getInt("humIntCurr");
     sunriseHour = preferences.getInt("sunriseHour");
     sunriseMin = preferences.getInt("sunriseMin");
     sunsetHour = preferences.getInt("sunsetHour");
     sunsetMin = preferences.getInt("sunsetMin");
     weather = preferences.getInt("weather");
}

void saveData()
{
     preferences.putInt("minutes", cMin);
     preferences.putInt("hours", cHour);
     preferences.putInt("days", cDay);
     preferences.putInt("months", cMonth);
     preferences.putInt("years", cYear);
     preferences.putInt("tempExtCurr", tempExtCurr);
     preferences.putInt("tempExtMax", tempExtMax);
     preferences.putInt("tempExtMin", tempExtMin);
     preferences.putInt("tempIntCurr", tempIntCurr);
     preferences.putInt("tempIntMax", tempIntMax);
     preferences.putInt("tempIntMin", tempIntMin);
     preferences.putInt("humExtCurr", humExtCurr);
     preferences.putInt("humIntCurr", humIntCurr);
     preferences.putInt("sunriseHour", sunriseHour);
     preferences.putInt("sunriseMin", sunriseMin);
     preferences.putInt("sunsetHour", sunsetHour);
     preferences.putInt("sunsetMin", sunsetMin);
     preferences.putInt("weather", weather);

#ifdef DEBUG
     /* Serial.print("Data saved: ");
     Serial.print(rtcValues.minutes);
     Serial.print(", ");
     Serial.print(rtcValues.hours);
     Serial.print(", ");
     Serial.print(rtcValues.days);
     Serial.print(", ");
     Serial.print(rtcValues.months);
     Serial.print(", ");
     Serial.println(rtcValues.years); */
#endif
     // ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcValues, sizeof(rtcValues));
}

void readTemperature()
{
     Serial.println("Send temp int");

     WiFiClient client;
     HTTPClient http;
     humIntCurr = int(round(dht.getHumidity()));
     Serial.println("H:");
     Serial.println(humIntCurr);
     // Read temperature as Celsius (the default)
     float temp = dht.getTemperature();
     // Check if any reads failed and exit early (to try again).
     if (isnan(temp) || isnan(temp) || isnan(temp))
     {
          Serial.println("Failed to read from DHT sensor!");
          tempIntCurr = 99;
          humIntCurr = 99;
          return;
     }
     tempIntCurr = int(round(temp));
     Serial.println("T:");
     Serial.println(tempIntCurr);

     // POST TEMP

     http.setTimeout(5000);
     http.begin(client, String(THINGSPEAK_POST) + "&field1=" + tempIntCurr +
                            "&field3=" + humIntCurr);
     // http.collectHeaders(headerKeys, 1);
     int httpCode = http.GET(); // Send the request
     Serial.print("Status code : ");
     Serial.println(httpCode);
     if (httpCode == 200)
     {
          // SUCCESS
          Serial.println("POST T SUCCESS");
          // drawWifi();
          String headerDate = http.header("Date");
          // parseTime2(headerDate);
     }
     else
     {
          Serial.println("POST T Error : " + httpCode);
          // drawNoWifi();
     }
     http.end(); // Close connection
     delay(1000);
}

void displayTemperature()
{
     // INTERIOR---------------------------------------------------------------------------------------------------------------------
     //------------------------------------------------------------------------------------------------------------------------------
     // CURRENT
     int cursor_x = 5;
     int cursor_y = 85;
     int width = 90;
     int height = 30;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold18pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(tempIntCurr));
     display.print("*C");

     // MAX
     cursor_x = 25;
     cursor_y = 130;
     width = 45;
     height = 20;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold12pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(tempIntMax));
     display.print("*");

     // MIN
     cursor_x = 25;
     cursor_y = 175;
     width = 45;
     height = 20;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold12pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(tempIntMin));
     display.print("*");

     // Humidity
     cursor_x = 30;
     cursor_y = 230;
     width = 60;
     height = 30;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 15, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold18pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(humIntCurr));
     display.print("%");

     // EXTERIOR
     // --------------------------------------------------------------------------------------------------------------------
     //------------------------------------------------------------------------------------------------------------------------------

     // CURRENT
     cursor_x = 305;
     cursor_y = 85;
     width = 90;
     height = 30;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold18pt7b);
     display.setCursor(cursor_x, cursor_y);
     if (tempExtCurr != 999)
     {
          display.print(String(tempExtCurr));
          display.print("*C");
     }
     else
     {
          display.print("--*C");
     }

     // MAX
     cursor_x = 330;
     cursor_y = 130;
     width = 50;
     height = 20;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold12pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(tempExtMax));
     display.print("*");

     // MIN
     cursor_x = 330;
     cursor_y = 175;
     width = 50;
     height = 20;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold12pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(String(tempExtMin));
     display.print("*");

     // Humidity
     cursor_x = 310;
     cursor_y = 230;
     width = 55;
     height = 30;
     display.fillRect(cursor_x - 5, cursor_y - height, width + 15, height + 2,
                      GxEPD_BLACK);
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold18pt7b);
     display.setCursor(cursor_x, cursor_y);
     if (humExtCurr != 999)
     {
          display.print(String(humExtCurr));
          display.print("%");
     }
     else
     {
          display.print("--%");
     }
}

void displayTemperature2()
{
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold18pt7b);

     int16_t tbx, tby;
     uint16_t tbw, tbh;
     display.getTextBounds(String(tempIntCurr) + "*", tempIntCurrBox.x, tempIntCurrBox.y, &tbx, &tby, &tbw, &tbh);
     display.setCursor(tempIntCurrBox.x + (tempIntCurrBox.width - tbw) / 2, tempIntCurrBox.y + tempIntCurrBox.height - 10);
     display.print(String(tempIntCurr) + "*");

     int16_t tbx2, tby2;
     uint16_t tbw2, tbh2;

     String sTempExt = "";
     if (tempExtCurr != 999)
     {
          sTempExt = String(tempExtCurr) + "*";
     }
     else
     {
          sTempExt = "--*";
     }

     display.getTextBounds(sTempExt, tempExtCurrBox.x, tempExtCurrBox.y, &tbx2, &tby2, &tbw2, &tbh2);
     display.setCursor(tempExtCurrBox.x + (tempExtCurrBox.width - tbw2) / 2, tempExtCurrBox.y + tempExtCurrBox.height - 10);
     display.print(sTempExt);

     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold12pt7b);

     int16_t tbx3, tby3;
     uint16_t tbw3, tbh3;
     String sTempIntMax = String(tempIntMax) + "*";
     display.getTextBounds(sTempIntMax, tempIntMaxBox.x, tempIntMaxBox.y, &tbx3, &tby3, &tbw3, &tbh3);
     display.setCursor(tempIntMaxBox.x + (tempIntMaxBox.width - tbw3) / 2, tempIntMaxBox.y + tempIntMaxBox.height - 7);
     display.print(sTempIntMax);

     int16_t tbx4, tby4;
     uint16_t tbw4, tbh4;
     String sTempIntMin = String(tempIntMin) + "*";
     display.getTextBounds(sTempIntMin, tempIntMinBox.x, tempIntMinBox.y, &tbx4, &tby4, &tbw4, &tbh4);
     display.setCursor(tempIntMinBox.x + (tempIntMinBox.width - tbw4) / 2, tempIntMinBox.y + tempIntMinBox.height - 7);
     display.print(sTempIntMin);

     int16_t tbx5, tby5;
     uint16_t tbw5, tbh5;
     String sTempExtMax = "";
     if (tempExtMax != -99)
     {
          sTempExtMax = String(tempExtMax) + "*";
     }
     else
     {
          sTempExtMax = "--*";
     }

     display.getTextBounds(sTempExtMax, tempExtMaxBox.x, tempExtMaxBox.y, &tbx5, &tby5, &tbw5, &tbh5);
     display.setCursor(tempExtMaxBox.x + (tempExtMaxBox.width - tbw5) / 2, tempExtMaxBox.y + tempExtMaxBox.height - 7);
     display.print(sTempExtMax);

     int16_t tbx6, tby6;
     uint16_t tbw6, tbh6;
     String sTempExtMin = "";
     if (tempExtMin != 99)
     {
          sTempExtMin = String(tempExtMin) + "*";
     }
     else
     {
          sTempExtMin = "--*";
     }
     display.getTextBounds(sTempExtMin, tempExtMinBox.x, tempExtMinBox.y, &tbx6, &tby6, &tbw6, &tbh6);
     display.setCursor(tempExtMinBox.x + (tempExtMinBox.width - tbw6) / 2, tempExtMinBox.y + tempExtMinBox.height - 7);
     display.print(sTempExtMin);

     // HumidityInt

     int16_t tbx7, tby7;
     uint16_t tbw7, tbh7;
     String sHumInt = "";
     if (humIntCurr != 999)
     {
          sHumInt = String(humIntCurr) + "%";
     }
     else
     {
          sHumInt = "--%";
     }
     display.setFont(&FreeMonoBold18pt7b);
     display.getTextBounds(sHumInt, humIntCurrBox.x, humIntCurrBox.y, &tbx7, &tby7, &tbw7, &tbh7);
     display.setCursor(humIntCurrBox.x + (humIntCurrBox.width - tbw7) / 2, humIntCurrBox.y + humIntCurrBox.height - 7);
     display.print(sHumInt);

     // HumidityInt

     int16_t tbx8, tby8;
     uint16_t tbw8, tbh8;
     String sHumExt = "";
     if (humExtCurr != 999)
     {
          sHumExt = String(humExtCurr) + "%";
     }
     else
     {
          sHumExt = "--%";
     }
     display.setFont(&FreeMonoBold18pt7b);
     display.getTextBounds(sHumExt, humExtCurrBox.x, humExtCurrBox.y, &tbx8, &tby8, &tbw8, &tbh8);
     display.setCursor(humExtCurrBox.x + (humExtCurrBox.width - tbw8) / 2, humExtCurrBox.y + humExtCurrBox.height - 7);
     display.print(sHumExt);
}

void drawBackground()
{
     display.fillRect(0, 0, display.width(), display.height(),
                      GxEPD_BLACK); // BACKGROUND BLACK
     display.fillRect(61, 0, 280, 50,
                      GxEPD_WHITE); // BACKGROUND TOP WHITE RECT (DATE)
     display.fillRect(100, 50, 200, 50,
                      GxEPD_WHITE); // BACKGROUND TOP WHITE RECT (DAY)
     display.fillRect(80, 100, 240, 97,
                      GxEPD_WHITE); // BACKGROUND TOP WHITE RECT (TIME)
     display.fillRect(100, 201, 200, 99,
                      GxEPD_WHITE); // BACKGROUND TOP WHITE RECT (METEO)
     display.fillRect(0, 250, display.width(), 50,
                      GxEPD_WHITE); // BACKGROUND TOP WHITE RECT (SUNRISE, SUNSET)

     display.drawBitmap(10, 10, image_data_home, 40, 40,
                        GxEPD_WHITE); // BACKGROUND ICON HOME
     display.drawBitmap(351, 10, image_data_outdoor, 40, 40,
                        GxEPD_WHITE); // BACKGROUND ICON OUTDOOR
     display.drawBitmap(6, 111, image_data_tempMax, 11, 30,
                        GxEPD_WHITE); // BACKGROUND ICON MAX TEMP LEFT
     display.drawBitmap(6, 158, image_data_tempMin, 11, 30,
                        GxEPD_WHITE); // BACKGROUND ICON MIN TEMP LEFT
     display.drawBitmap(385, 111, image_data_tempMax, 11, 30,
                        GxEPD_WHITE); // BACKGROUND ICON MAX TEMP RIGHT
     display.drawBitmap(385, 158, image_data_tempMin, 11, 30,
                        GxEPD_WHITE); // BACKGROUND ICON MIN TEMP RIGHT
     display.drawBitmap(375, 205, image_data_humidity, 20, 29,
                        GxEPD_WHITE); // BACKGROUND ICON HUMIDITY RIGHT
     display.drawBitmap(6, 205, image_data_humidity, 20, 29,
                        GxEPD_WHITE); // BACKGROUND ICON HUMIDITY LEFT
}

void drawBackground2()
{
     display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);                         // BACKGROUND WHITE
     display.fillRect(timeBox.x, timeBox.y, timeBox.width, timeBox.height, GxEPD_BLACK);             // TIME BKG BLACK
     display.fillRect(sunriseBox.x, sunriseBox.y, sunriseBox.width, sunriseBox.height, GxEPD_BLACK); // SUNRISE BKG BLACK
     display.fillRect(sunsetBox.x, sunsetBox.y, sunsetBox.width, sunsetBox.height, GxEPD_BLACK);     // SUNSET BKG BLACK

     display.drawBitmap(tempIntLogoBox.x, tempIntLogoBox.y, image_data_home, tempIntLogoBox.width, tempIntLogoBox.height, GxEPD_BLACK);                // BACKGROUND ICON HOME
     display.drawBitmap(tempIntMaxLogoBox.x, tempIntMaxLogoBox.y, image_data_tempMax, tempIntMaxLogoBox.width, tempIntMaxLogoBox.height, GxEPD_BLACK); // BACKGROUND ICON MAX TEMP LEFT
     display.drawBitmap(tempIntMinLogoBox.x, tempIntMinLogoBox.y, image_data_tempMin, tempIntMinLogoBox.width, tempIntMinLogoBox.height, GxEPD_BLACK); // BACKGROUND ICON MIN TEMP LEFT

     display.drawBitmap(tempExtLogoBox.x, tempExtLogoBox.y, image_data_outdoor, tempExtLogoBox.width, tempExtLogoBox.height, GxEPD_BLACK);             // BACKGROUND ICON OUTDOOR
     display.drawBitmap(tempExtMaxLogoBox.x, tempExtMaxLogoBox.y, image_data_tempMax, tempExtMaxLogoBox.width, tempExtMaxLogoBox.height, GxEPD_BLACK); // BACKGROUND ICON MAX TEMP LEFT
     display.drawBitmap(tempExtMinLogoBox.x, tempExtMinLogoBox.y, image_data_tempMin, tempExtMinLogoBox.width, tempExtMinLogoBox.height, GxEPD_BLACK); // BACKGROUND ICON MIN TEMP LEFT

     display.drawBitmap(humLogoBox.x, humLogoBox.y, image_data_humidity, humLogoBox.width, humLogoBox.height, GxEPD_BLACK); // BACKGROUND ICON HUMIDITY RIGHT

     // display.drawBitmap(385, 111, image_data_tempMax, 11, 30,
     //                    GxEPD_WHITE); // BACKGROUND ICON MAX TEMP RIGHT
     // display.drawBitmap(385, 158, image_data_tempMin, 11, 30,
     //                    GxEPD_WHITE); // BACKGROUND ICON MIN TEMP RIGHT
     // display.drawBitmap(375, 205, image_data_humidity, 20, 29,
     //                    GxEPD_WHITE); // BACKGROUND ICON HUMIDITY RIGHT
     // display.drawBitmap(6, 205, image_data_humidity, 20, 29,
     //                    GxEPD_WHITE); // BACKGROUND ICON HUMIDITY LEFT
}

void drawWifi()
{
     // display.drawBitmap(image_data_wifi,10, 10, 20,20, GxEPD_WHITE);
     // display.updateWindow(10, 10, 25, 25, true);
     // delay(5000);
}

void drawNoWifi()
{
     //  display.drawBitmap(image_data_no_wifi,10, 10, 20,20, GxEPD_WHITE);
     // display.updateWindow(10, 10, 25, 25, true);
}

void displayTime()
{
     String s = "";
     if (cHour < 10)
     {
          s = s + "0";
     }
     s = s + String(cHour);
     s = s + ":";
     if (cMin < 10)
     {
          s = s + "0";
     }
     s = s + String(cMin);

     uint16_t box_x = 95;
     uint16_t box_y = 120;
     uint16_t box_w = 210;
     uint16_t box_h = 56;
     uint16_t cursor_y = box_y + box_h - 1;

     display.setFont(&Roboto_Black_72);
     display.setTextColor(GxEPD_BLACK);
     display.setCursor(box_x, cursor_y);
     display.print(s);
}

void displayTime2()
{
     // 0, 0, 200, 250
     String sHour = "";
     if (cHour < 10)
     {
          sHour = "0" + String(cHour);
     }
     else
     {
          sHour = String(cHour);
     }

     String sMin = "";
     if (cMin < 10)
     {
          sMin = "0" + String(cMin);
     }
     else
     {
          sMin = String(cMin);
     }

     display.setFont(&Roboto_Black_72);
     display.setTextColor(GxEPD_WHITE);
     display.setCursor(30, 85);
     display.print(sHour);
     display.setCursor(30, 170);
     display.print(sMin);
}

void displayWeather()
{
     // 0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 = brouillard;  5 = nuages;
     // 6 = soleil;
     int width = 64;
     int height = 64;
     int posX = 168;
     int poxY = 215;

     displayWeatherItem(weather, posX, poxY, width, height);
}

void displayWeather2()
{
     displayWeatherItem(weather, weatherBox.x, weatherBox.y, weatherBox.width, weatherBox.height);
     displayWeatherItem(weatherJ1, weatherBoxJ1.x, weatherBoxJ1.y, weatherBoxJ1.width, weatherBoxJ1.height);
     displayWeatherItem(weatherJ2, weatherBoxJ2.x, weatherBoxJ2.y, weatherBoxJ2.width, weatherBoxJ2.height);

     int16_t tbx, tby;
     uint16_t tbw, tbh;
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold12pt7b);
     display.getTextBounds("J+1", weatherBoxJ1.x, weatherBoxJ1.y, &tbx, &tby, &tbw, &tbh);
     display.fillRect(weatherBoxJ1.x, weatherBoxJ1.y, tbw + 2, tbh + 2, GxEPD_WHITE);
     display.setCursor(weatherBoxJ1.x, weatherBoxJ1.y + tbh);
     // display.setCursor(weatherBoxJ1.x, tby);
     display.print("J+1");
     display.fillRect(weatherBoxJ2.x, weatherBoxJ2.y, tbw + 2, tbh + 2, GxEPD_WHITE);
     display.setCursor(weatherBoxJ2.x, weatherBoxJ2.y + tbh);
     // display.setCursor(weatherBoxJ2.x, tby);
     display.print("J+2");
}

void displayWeatherItem(int weath, int posX, int posY, int width, int height)
{
     if (weath == STORM)
          display.drawInvertedBitmap(posX, posY, image_data_storm, width, height,
                                     GxEPD_BLACK);
     else if (weath == DRIZZLE)
          display.drawInvertedBitmap(posX, posY, image_data_drizzle, width, height, GxEPD_BLACK);
     else if (weath == RAIN)
          display.drawInvertedBitmap(posX, posY, image_data_rain, width, height, GxEPD_BLACK);
     else if (weath == SNOW)
          display.drawInvertedBitmap(posX, posY, image_data_snow, width, height, GxEPD_BLACK);
     else if (weath == FOG)
          display.drawInvertedBitmap(posX, posY, image_data_fog, width, height,
                                     GxEPD_BLACK);
     else if (weath == SUNNY_CLOUD)
          display.drawInvertedBitmap(posX, posY, image_data_sunny_cloud, width, height,
                                     GxEPD_BLACK);
     else if (weath == CLOUD)
          display.drawInvertedBitmap(posX, posY, image_data_cloud, width, height,
                                     GxEPD_BLACK);
     else if (weath == SUN)
          display.drawInvertedBitmap(posX, posY, image_data_sun, width, height, GxEPD_BLACK);
}

void displaySunSetRise()
{
     String sunset = (sunsetHour < 10 ? "0" : "") + String(sunsetHour) + ":" +
                     (sunsetMin < 10 ? "0" : "") + String(sunsetMin);
     Serial.println("Sunset: " + sunset);
     String sunrise = (sunriseHour < 10 ? "0" : "") + String(sunriseHour) + ":" +
                      (sunriseMin < 10 ? "0" : "") + String(sunriseMin);
     Serial.println("Sunrise: " + sunrise);
     // Sunrise
     int cursor_x = 5;
     int cursor_y = 282;
     int width = 70;
     int height = 20;
     display.fillRect(cursor_x, cursor_y - height, width, height, GxEPD_WHITE);
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold18pt7b);
     display.setCursor(cursor_x, cursor_y);
     display.print(sunrise);

     cursor_x = 290;
     display.fillRect(cursor_x, cursor_y - height, width, height, GxEPD_WHITE);
     display.setCursor(cursor_x, cursor_y);
     display.print(sunset);
}

void displaySunSetRise2()
{
     String sunset = (sunsetHour < 10 ? "0" : "") + String(sunsetHour) + ":" +
                     (sunsetMin < 10 ? "0" : "") + String(sunsetMin);
     Serial.println("Sunset: " + sunset);
     String sunrise = (sunriseHour < 10 ? "0" : "") + String(sunriseHour) + ":" +
                      (sunriseMin < 10 ? "0" : "") + String(sunriseMin);
     Serial.println("Sunrise: " + sunrise);
     // Sunrise
     display.setTextColor(GxEPD_WHITE);
     display.setFont(&FreeMonoBold12pt7b);

     int16_t tbx, tby;
     uint16_t tbw, tbh;
     display.getTextBounds(sunrise, sunriseBox.x, sunriseBox.y, &tbx, &tby, &tbw, &tbh);
     display.setCursor(sunriseBox.x + (sunriseBox.width - tbw) / 2, sunriseBox.y + sunriseBox.height - 13);
     display.print(sunrise);

     int16_t tbx2, tby2;
     uint16_t tbw2, tbh2;
     display.getTextBounds(sunset, sunsetBox.x, sunsetBox.y, &tbx2, &tby2, &tbw2, &tbh2);
     display.setCursor(sunsetBox.x + (sunsetBox.width - tbw2) / 2, sunsetBox.y + sunsetBox.height - 13);
     display.print(sunset);
}

void displayDate()
{
     // DISPLAY DATE ( DD MMM YYYY )
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold18pt7b);
     Serial.println(cDay);
     Serial.println(cMonth);
     Serial.println(cYear);
     Serial.println(cDayStr);
     String s = "";
     if (cDay < 10)
     {
          s += "0";
     }
     s += String(cDay);
     s += " ";

     s += MONTHS[cMonth - 1];
     s += " ";
     s += cYear;

     int16_t x, y;
     uint16_t w, h;
     display.getTextBounds(s, 61, 50, &x, &y, &w, &h);
     Serial.print(x);
     Serial.print(" ");
     Serial.print(y);
     Serial.print(" ");
     Serial.print(w);
     Serial.print(" ");
     Serial.println(h);
     int cX = 61 + (280 - w) / 2;
     int cY = 50 - (50 - h) / 2 * 1.5;
     Serial.println(cY);
     display.setCursor(cX, cY);

     display.print(s);

     // UPDATE DAY NAME
     int cursor_x = 115 + daySpaces[cDayStr]; // init for "Dimanche"
     int cursor_y = 85;
     int width = 170;
     int height = 25;
     int negHeight = 10;
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold18pt7b);
     display.getTextBounds(DAYS[cDayStr], 100, 50, &x, &y, &w, &h);
     int cX2 = 100 + (200 - w) / 2;
     int cY2 = 100 - (50 - h) / 2 * 1.5;
     display.setCursor(cX2, cY2);
     display.print(DAYS[cDayStr]);
}

void displayDate2()
{

     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold18pt7b);

     int16_t tbx, tby;
     uint16_t tbw, tbh;
     display.getTextBounds(DAYS[cDayStr], dayBox.x, dayBox.y, &tbx, &tby, &tbw, &tbh);
     display.setCursor(dayBox.x + (dayBox.width - tbw) / 2, dayBox.y + dayBox.height - 5);
     display.print(DAYS[cDayStr]);

     String s = "";
     if (cDay < 10)
     {
          s += "0";
     }
     s += String(cDay);
     s += " ";

     s += MONTHS[cMonth - 1];
     s += " ";
     s += cYear;

     display.setFont(&FreeMonoBold18pt7b);
     int16_t tbx2, tby2;
     uint16_t tbw2, tbh2;
     display.getTextBounds(s, dateBox.x, dateBox.y, &tbx2, &tby2, &tbw2, &tbh2);
     display.setCursor(dateBox.x + (dateBox.width - tbw2) / 2, dateBox.y + dateBox.height - 5);
     display.print(s);
}

void getTime()
{
     WiFiClient client;
     HTTPClient http;
     http.setTimeout(10000);
     http.begin(client, TIME_URL);
     int httpCode = http.GET();
     if (httpCode > 0)
     {                                    // Check the returning code
          String json = http.getString(); // Get the request response payload
          Serial.println(json);           // Print the response payload

          const size_t capacity = JSON_OBJECT_SIZE(15) + 350;
          DynamicJsonDocument doc(capacity);
          deserializeJson(doc, json);
          int day_of_week = doc["day_of_week"]; // 6
          cDayStr = day_of_week;
          const char *datetime =
              doc["datetime"]; // "2019-08-31T02:12:43.056352+02:00"
          String dateTimeStr = String(datetime);
          int indexYear = dateTimeStr.indexOf("-");
          cYear = dateTimeStr.substring(0, indexYear).toInt();
          int indexMonth = dateTimeStr.indexOf("-", indexYear + 1);
          cMonth = dateTimeStr.substring(indexYear + 1, indexMonth).toInt();
          int indexDay = dateTimeStr.indexOf("T", indexMonth + 1);
          cDay = dateTimeStr.substring(indexMonth + 1, indexDay).toInt();
          int indexHour = dateTimeStr.indexOf(":", indexDay + 1);
          cHour = dateTimeStr.substring(indexDay + 1, indexHour).toInt();
          int indexMin = dateTimeStr.indexOf(":", indexHour + 1);
          cMin = dateTimeStr.substring(indexHour + 1, indexMin).toInt();
          int indexSec = dateTimeStr.indexOf(".", indexMin + 1);
          cSec = dateTimeStr.substring(indexMin + 1, indexSec).toInt();
     }
     else
     {
          Serial.print("Status ERROR : ");
          Serial.println(http.errorToString(httpCode).c_str());
          Serial.println(httpCode);
     }
     http.end();
}

void getLastValue(String url, int *value)
{
     Serial.print("getLast : ");
     HTTPClient http;
     WiFiClient client;
     http.setTimeout(15000);
     // Serial.println("URL: "+THINGSPEAK_GET24H_FIELD1);
     http.begin(client, url);
     int httpCode = http.GET(); // Send the request
     if (httpCode == HTTP_CODE_OK)
     {
          // SUCCESS
          Serial.println("Success get Last ");
          int lastI = http.getString().lastIndexOf(',');
          *value = -99;
          if (lastI != http.getString().length())
          {
               String tmp = http.getString()
                                .substring(lastI + 1, http.getString().length());
               if (!tmp.startsWith("nan"))
               {
                    *value = int(round(tmp.toFloat()));
               }
               else
               {
                    *value = 999;
               }
          }
     }
     else
     {
          Serial.println("POST T Error : " + httpCode);
     }
     http.end(); // Close connection
}

void getSunWeather()
{
     Serial.print("getSunWeather : ");
     HTTPClient http;
     WiFiClient client;
     http.setTimeout(15000);
     http.begin(client, OPENWEATHER_URL);
     int httpCode = http.GET(); // Send the request
     if (httpCode == HTTP_CODE_OK)
     {
          // SUCCESS
          Serial.println("Success get Sun and Weather :" + http.getString());
          const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) +
                                  2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) +
                                  JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) +
                                  JSON_OBJECT_SIZE(13) + 270;
          DynamicJsonDocument doc(capacity);

          // const char* json =
          // "{\"coord\":{\"lon\":4.83,\"lat\":45.76},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear
          // sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":32.76,\"pressure\":1012,\"humidity\":30,\"temp_min\":31,\"temp_max\":35},\"visibility\":10000,\"wind\":{\"speed\":4.1,\"deg\":150},\"clouds\":{\"all\":0},\"dt\":1567263192,\"sys\":{\"type\":1,\"id\":6505,\"message\":0.0061,\"country\":\"FR\",\"sunrise\":1567227606,\"sunset\":1567275757},\"timezone\":7200,\"id\":2996944,\"name\":\"Lyon\",\"cod\":200}";

          deserializeJson(doc, http.getString());

          JsonObject weather_0 = doc["weather"][0];
          int weather_0_id = weather_0["id"]; // 800

          int code = weather_0_id;
          Serial.print("Code: ");
          Serial.println(code);

          if (code >= 200 && code <= 232)
               weather = 0;

          if (code >= 300 && code <= 321)
               weather = 1;

          if (code >= 500 && code <= 531)
               weather = 2;

          if (code >= 600 && code <= 622)
               weather = 3;

          if (code >= 700 && code <= 781)
               weather = 4;

          if (code >= 801 && code <= 804)
               weather = 5;

          if (code == 800)
               weather = 6;

          Serial.print("Weahter: ");
          Serial.println(weather);
          JsonObject sys = doc["sys"];
          long sys_sunrise = sys["sunrise"]; // 1567227606
          long sys_sunset = sys["sunset"];   // 1567275757
          long timezone = doc["timezone"];   // 7200
          DTime sunsetT, sunriseT;
          sunsetT.setTimestamp(sys_sunset + timezone);
          sunsetHour = (uint32_t)sunsetT.hour;
          sunsetMin = (uint32_t)sunsetT.minute;

          sunriseT.setTimestamp(sys_sunrise + timezone);
          sunriseHour = (uint32_t)sunriseT.hour;
          sunriseMin = (uint32_t)sunriseT.minute;

          Serial.printf("Sun : %d:%d - %d:%d", sunsetHour, sunsetMin, sunriseHour,
                        sunriseMin);
     }
     else
     {
          Serial.println("POST T Error : " + httpCode);
     }
     http.end(); // Close connection
}

void getMinMaxTemp(String url, int *maxTemp, int *minTemp)
{
     Serial.print("getMinMaxTemp : ");
     WiFiClient client;
     HTTPClient http;
     http.setTimeout(15000);
     // Serial.println("URL: "+THINGSPEAK_GET24H_FIELD1);
     http.begin(client, url);
     int httpCode = http.GET(); // Send the request
     if (httpCode == HTTP_CODE_OK)
     {
          // SUCCESS
          Serial.println("Success get Last 24h");
          getMaxMin(http, maxTemp, minTemp);
     }
     else
     {
          Serial.println("POST T Error : " + httpCode);
          // drawNoWifi();
     }
     http.end(); // Close connection
}

void getMaxMin(HTTPClient &http, int *tempMax, int *tempMin)
{
     // int ssize =  http.getString().length();
     // int line = 0;
     // int index = 0;
     int minTemp = 99;
     int maxTemp = -99;
     int iLine = http.getString().indexOf("\n");
     int startS = iLine + 1;
     int nbLine = 0;
     iLine = http.getString().indexOf("\n", startS);
     while (iLine != -1)
     {
          // get line.
          String line = http.getString().substring(startS, iLine);
          // get only temp.
          int lastDelimiter = line.lastIndexOf(",");
          if (lastDelimiter + 1 != line.length())
          { // check if value exists
               String sTemp = line.substring(lastDelimiter + 1, iLine);
               if (!sTemp.equals("nan"))
               {
                    float temp = int(round(sTemp.toFloat()));
                    minTemp = minTemp < temp ? minTemp : temp;
                    maxTemp = maxTemp > temp ? maxTemp : temp;
               }
          }
          nbLine++;
          startS = iLine + 1;
          iLine = http.getString().indexOf("\n", startS);
     }
     *tempMax = maxTemp;
     *tempMin = minTemp;
     Serial.println((String) "Max: " + maxTemp + " Min: " + minTemp);
     Serial.println(nbLine);
}

void addOneMinute()
{
     cMin++;
     if (cMin == 60)
     {
          cMin = 0;
          cHour++;
          if (cHour > 23)
          {
               cHour = 0;
          }
     }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
     configSetting = true;
     Serial.println("Entered config mode");
     Serial.println(WiFi.softAPIP());

     Serial.println(myWiFiManager->getConfigPortalSSID());

     displayWifiManager(myWiFiManager);
}

void displayWifiManager(WiFiManager *myWiFiManager)
{
     Serial.println("Display connection config");
     // WiFi.mode(WIFI_AP);
     drawConnectionPage();

     display.firstPage();
     do
     {
          drawConnectionPage();
     } while (display.nextPage());
}

void drawConnectionPage()
{
     display.fillRect(0, 0, display.width(), display.height(),
                      GxEPD_WHITE);
     display.setTextColor(GxEPD_BLACK);
     display.setFont(&FreeMonoBold9pt7b);
     display.setCursor(15, 100);
     display.print("Initialisation de la station meteo");
     display.setCursor(15, 130);
     display.print("Connectez-vous sur cette borne wifi :\nStation meteto\nMot de passe :\nmaStation");
     display.setCursor(15, 230);
     display.print("Puis ouvrez un navigateur internet a cette adresse :\n192.168.4.1");
}

bool initWifi()
{
     Serial.println("Restart wifi");
     // WiFi.forceSleepWake();
     // WiFi.mode(WIFI_AP_STA);

     WiFiManager wifiManager;
     // wifiManager.softAPConfig(IPAddress(192, 168, 10, 1), IPAddress(192, 168, 10, 1), IPAddress(255, 255, 255, 0));
     wifiManager.setAPCallback(configModeCallback);
     // wifiManager.setConfigPortalTimeout(30);                            //set timeout portal.
     bool res = wifiManager.autoConnect("Station météto", "maStation"); // password protected ap
                                                                        // getSSID
     if (!res)
     {
          Serial.println("Failed to connect");
          addOneMinute();
          return false;
     }
     else
     {
          Serial.println("");
          Serial.println("WiFi connected");
     }

     // Printing the ESP IP address
     Serial.println(WiFi.localIP());
     return true;
}

void resetParam()
{
     Serial.println("Button has been pressed.");

     WiFiManager wifiManager;
     wifiManager.resetSettings();
     delay(1000);
     initWifi();
}

void getOpenWeatherOneCall()
{
     WiFiClient client;
     HTTPClient http;
     http.setTimeout(10000);
     http.begin(client, OPENWEATHER_ONE_CALL);
     int httpCode = http.GET();
     Serial.println(httpCode);
     if (httpCode > 0)
     { // Check the returning code
          String json = http.getString();
          deserializeJsonOpenWeather(json);
     }
     else
     {
          Serial.println("ERROR open weather onecall");
     }
     http.end();
}

void deserializeJsonOpenWeather(String input)
{
     // String input;

#pragma region Deserialize JSON
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

     if (error)
     {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
     }

     // float lat = doc["lat"];                       // 46.3333
     // float lon = doc["lon"];                       // 5.1333
     // const char *timezone = doc["timezone"];       // "Europe/Paris"
     int timezone_offset = doc["timezone_offset"]; // 3600

     JsonObject current = doc["current"];
     long current_dt = current["dt"];           // 1612992998
     long current_sunrise = current["sunrise"]; // 1612939831
     long current_sunset = current["sunset"];   // 1612976221
     tempExtCurr = current["temp"];             // 271.87

     // float current_feels_like = current["feels_like"]; // 267.61
     // int current_pressure = current["pressure"];       // 1016
     humExtCurr = current["humidity"]; // 88
     // float current_dew_point = current["dew_point"];   // 270.34
     // int current_uvi = current["uvi"];                 // 0
     // int current_clouds = current["clouds"];           // 98
     // int current_visibility = current["visibility"];   // 10000
     // float current_wind_speed = current["wind_speed"]; // 2.68
     // int current_wind_deg = current["wind_deg"];       // 9
     // float current_wind_gust = current["wind_gust"];   // 5.36

     JsonObject current_weather_0 = current["weather"][0];
     int current_weather_0_id = current_weather_0["id"]; // 804
     // const char *current_weather_0_main = current_weather_0["main"];               // "Clouds"
     // const char *current_weather_0_description = current_weather_0["description"]; // "overcast clouds"
     // const char *current_weather_0_icon = current_weather_0["icon"];               // "04n"

     JsonArray daily = doc["daily"];

     JsonObject daily_0 = daily[1];
     // long daily_0_dt = daily_0["dt"];           // 1612954800
     // long daily_0_sunrise = daily_0["sunrise"]; // 1612939831
     // long daily_0_sunset = daily_0["sunset"];   // 1612976221

     JsonObject daily_0_temp = daily_0["temp"];
     float daily_0_temp_day = daily_0_temp["day"]; // 279.69
     float daily_0_temp_min = daily_0_temp["min"]; // 271.1
     float daily_0_temp_max = daily_0_temp["max"]; // 280.05
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
     int daily_0_weather_0_id = daily_0_weather_0["id"]; // 501
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

     for (JsonObject elem : doc["alerts"].as<JsonArray>())
     {

          const char *sender_name = elem["sender_name"]; // "METEO-FRANCE", "METEO-FRANCE"
          const char *event = elem["event"];             // "Moderate snow-ice warning", "Moderate flooding warning"
          long start = elem["start"];                    // 1612969200, 1612969200
          long end = elem["end"];                        // 1613055600, 1613055600
          const char *description = elem["description"]; // "Although rather usual in this region, locally or ...
     }

#pragma endregion

     //  Serial.printf("Demain, le temps sera %s, il fera %f, min %f et max %f\n", *daily_1_weather_0_main, daily_1_temp_day, daily_1_temp_min, daily_1_temp_max);
     // Serial.printf("Aujourd'hui, il fait %s, temp: %f\n", current_weather_0_description, tempExtCurr);
     // Serial.printf("Demain, il fera %s, temp: %f , min: %f , max: %f \n", daily_0_weather_0_description, daily_0_temp_day, daily_0_temp_min, daily_0_temp_max);
     Serial.printf("Il y a %d alertes météo.\n", doc["alerts"].as<JsonArray>().size());

     weather = getWeatherCode(current_weather_0_id);
     weatherJ1 = getWeatherCode(daily_0_weather_0_id);
     weatherJ2 = getWeatherCode(daily_1_weather_0_id);

     /*  if (current_weather_0_id >= 200 && current_weather_0_id <= 232)
          weather = 0;
     if (current_weather_0_id >= 300 && current_weather_0_id <= 321)
          weather = 1;
     if (current_weather_0_id >= 500 && current_weather_0_id <= 531)
          weather = 2;
     if (current_weather_0_id >= 600 && current_weather_0_id <= 622)
          weather = 3;
     if (current_weather_0_id >= 700 && current_weather_0_id <= 781)
          weather = 4;
     if (current_weather_0_id >= 801 && current_weather_0_id <= 804)
          weather = 5;
     if (current_weather_0_id == 800)
          weather = 6; */

     Serial.print("Weahter: ");
     Serial.println(weather);
     JsonObject sys = doc["sys"];
     long sys_sunrise = sys["sunrise"]; // 1567227606
     long sys_sunset = sys["sunset"];   // 1567275757
     long timezone = doc["timezone"];   // 7200
     DTime sunsetT, sunriseT;
     sunsetT.setTimestamp(current_sunset + timezone_offset);
     sunsetHour = (uint32_t)sunsetT.hour;
     sunsetMin = (uint32_t)sunsetT.minute;

     sunriseT.setTimestamp(current_sunrise + timezone_offset);
     sunriseHour = (uint32_t)sunriseT.hour;
     sunriseMin = (uint32_t)sunriseT.minute;

     Serial.printf("Sun : %d:%d - %d:%d\n", sunsetHour, sunsetMin, sunriseHour,
                   sunriseMin);

     DTime cDate;
     cDate.setTimestamp(current_dt + timezone_offset);
     // String dateTimeStr = String(datetime);
     cDayStr = cDate.weekday;
     cYear = cDate.year;
     cMonth = cDate.month;
     cDay = cDate.day;
     cHour = cDate.hour;
     cMin = cDate.minute;
     cSec = cDate.second;
     Serial.printf("Nous somme le %s %d %d %d et il est %d:%d:%d\n", DAYS[cDayStr], cDay, cMonth, cYear, cHour, cMin, cSec);
     /*int indexYear = dateTimeStr.indexOf("-");
     cYear = dateTimeStr.substring(0, indexYear).toInt();
     int indexMonth = dateTimeStr.indexOf("-", indexYear + 1);
     cMonth = dateTimeStr.substring(indexYear + 1, indexMonth).toInt();
     int indexDay = dateTimeStr.indexOf("T", indexMonth + 1);
     cDay = dateTimeStr.substring(indexMonth + 1, indexDay).toInt();
     int indexHour = dateTimeStr.indexOf(":", indexDay + 1);
     cHour = dateTimeStr.substring(indexDay + 1, indexHour).toInt();
     int indexMin = dateTimeStr.indexOf(":", indexHour + 1);
     cMin = dateTimeStr.substring(indexHour + 1, indexMin).toInt();
     int indexSec = dateTimeStr.indexOf(".", indexMin + 1);
     cSec = dateTimeStr.substring(indexMin + 1, indexSec).toInt();*/
}

int getWeatherCode(int weather)
{
     if (weather >= 200 && weather <= 232)
          return STORM;
     if (weather >= 300 && weather <= 321)
          return DRIZZLE;
     if (weather >= 500 && weather <= 531)
          return RAIN;
     if (weather >= 600 && weather <= 622)
          return SNOW;
     if (weather >= 700 && weather <= 781)
          return FOG;
     if (weather == 800)
          return SUN;
     if (weather >= 801 && weather <= 802)
          return SUNNY_CLOUD;
     if (weather >= 803 && weather <= 804)
          return CLOUD;
     return 7;
}