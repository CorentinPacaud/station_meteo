#include <GxEPD.h>
#include <GxFont_GFX.h>
#include <Adafruit_GFX.h>
#include <math.h>

#include <GxGDEW042T2/GxGDEW042T2.cpp>      // 4.2" b/w 400 x 300 px

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>


#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "font36.h"
#include "background.c"

#include "DHT.h"

#define DHTPIN 5     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#include<SPI.h>

GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD_Class display(io); // default selection of D4(=2), D2(=4)

const char* ssid = SSID;
const char* password = SSID_PASSWORD;

HTTPClient http;

// TIME
char timeUrl[] = TIME_URL;
int cHour = -1;
int cMin = -1;
int cDay = 1;
int cMonth = 1;
int cYear = 1;
int sleepSec = 0;
unsigned long millisTime = 0;

int i = 0;

bool fillRected = false;
  const GFXfont* f = &FreeMonoBold9pt7b;

bool startUp = true;

DHT dht(DHTPIN, DHTTYPE);
float currentTemp = 0.0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  initWifi();

  display.init(115200); // enable diagnostic output on Serial

  //display.fillScreen(GxEPD_BLACK);
  
  int i = 0;
  //display.update();

  Serial.println("setup done");
  drawBackground();
}

void loop() {
  unsigned long currentMillis = millis();
  // EVERY MINUTE -> TODO CHANGE TO ONCE A DAY
  if ((unsigned long)(currentMillis - millisTime) >= (1000 * 60) || millisTime == 0) {
   
    readTemperature();
    displayTemperature();
    if(cHour == -1 && cMin == -1){
      getTime();
    }else{
      cMin++;
      if(cMin == 60){
        cMin = 0;
        cHour++;
        if(cHour >23){
          cHour = 0;
        }
      }
    }
    millisTime = currentMillis;
    displayTime();
    if(startUp|| cMin%60==0){
        updateDate();
   //   display.update();
//      delay(5000);
      startUp = false;
   }
  }
  delay(60000-(millis()-currentMillis));
}

void readTemperature(){
   float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    currentTemp = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(currentTemp) || isnan(currentTemp) || isnan(currentTemp)) {
      Serial.println("Failed to read from DHT sensor!");
      
      return;
    }
    Serial.println("T:");
    Serial.println(currentTemp);
    
    // POST TEMP
  http.begin(String(THINGSPEAK_POST) + "&field1=" + currentTemp);
  int httpCode = http.GET();   //Send the request
  if (httpCode == 200) {
    // SUCCESS
    Serial.println("POST T SUCCESS");
  } else {
    Serial.println("POST T Error : " + httpCode);
  }
  http.end();  //Close connection

//  // GET EXT TEMP
//  http.begin(TSGetLastFiel2);
//  httpCode = http.GET();
//  if (httpCode > 0) { //Check the returning code
//    String json = http.getString();   //Get the request response payload
//    Serial.println(json);                     //Print the response payload
//    const size_t bufferSize = JSON_OBJECT_SIZE(3) + 70;
//    DynamicJsonBuffer jsonBuffer(bufferSize);
//    JsonObject& root = jsonBuffer.parseObject(json);
//
//    const char* created_at = root["created_at"]; // "2018-05-14T20:41:46Z"
//    int entry_id = root["entry_id"]; // 624
//    const char* field2 = root["field2"]; // "19.6"
//    extTemp = atof(field2);
//  } else {
//    Serial.println("GET xT Error : " + httpCode);
//  }
//  http.end();   //Close connection

}

void displayTemperature(){
  int cursor_x = 5;
  int cursor_y = 90;
  int width = 90;
  int height = 30;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(currentTemp)));
  display.print("*C");
  display.updateWindow(cursor_x, cursor_y - height, width, height, true);
  delay(2000);
}

void drawBackground(){
   display.drawPicture(image_data_background3, sizeof(image_data_background3));
  //delay(5000);
}

void displayTime(){
 //  display.fillRect(150, 100, 100, 200, GxEPD_WHITE);
    //display.fillRect(114,75, 210, 1, GxEPD_WHITE);
    int cursor_x = 95;
    int cursor_y = 170;
    int width = 210;
    int height = 56;
    display.fillRect(cursor_x,cursor_y-height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&Roboto_Black_72); // hauteur: 55
  display.setCursor(cursor_x, cursor_y);
  if (cHour < 10) {
      display.print("0");
    }
  display.print(cHour);
  display.print(":");
  if (cMin < 10) {
    display.print("0");
  }
  display.print(cMin);
 // if(cMin % 10 == 0){
 //   if(cMin != 0)
  //    display.updateWindow(cursor_x+120, cursor_y-height, width, height, true);
  //  else
      display.updateWindow(cursor_x, cursor_y-height, width, height, true);
 // }else{
  //  display.updateWindow(cursor_x+160, cursor_y-height, width, height, true);
  //}
  delay(2000);
}

void updateDate(){
  int cursor_x = 130;
  int cursor_y = 95;
  int width = 140;
  int height = 30;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(cursor_x, cursor_y);
  if (cDay < 10) {
     display.print("0");
  }
  display.print(cDay);
  display.print("-");  
  if (cMonth < 10) {
     display.print("0");
  }
  display.print(cMonth);
  display.print("-");
  display.print(cYear);
  display.updateWindow(cursor_x, cursor_y - height, width, height, true);
  delay(2000);
}


void getTime() {

  http.setTimeout(10000);
  http.begin(timeUrl);
  int httpCode = http.GET();
  if (httpCode > 0) { //Check the returning code
    String json = http.getString();   //Get the request response payload
    Serial.println(json);                     //Print the response payload
    const size_t bufferSize = JSON_OBJECT_SIZE(8) + 280;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(json);
    const char* localDate = root["localDate"];
    parseTime( reinterpret_cast<const char*>(localDate));
  }else{
    
    Serial.print("Status ERROR : ");
    Serial.println(http.errorToString(httpCode).c_str());
    Serial.println(httpCode);
  }
  http.end();
}

void parseTime(String date) {
  int index = date.indexOf(':');
  Serial.println("Date:" + date);
  if (index >= 0) {
    //Serial.println("Index:" + String(index));
    cHour = ((date.charAt(index - 2) - '0') * 10) + (date.charAt(index - 1) - '0');
    cMin = ((date.charAt(index + 1) - '0') * 10) + (date.charAt(index + 2) - '0');
    Serial.println("HOUR:" + String(cHour) + "  Min:" + String(cMin));
       sleepSec = (date.charAt(index + 4) - '0') * 10 + (date.charAt(index + 5) - '0');
    Serial.println("SleepSec:" + String(sleepSec));
  }
  int indexD = date.indexOf('.');
  if (indexD >= 0) {
    cDay = ((date.charAt(0) - '0') * 10) + (date.charAt(1) - '0');
    cMonth = ((date.charAt(3) - '0') * 10) + (date.charAt(4) - '0');
    cYear = ((date.charAt(6) - '0') * 1000) +((date.charAt(7) - '0') * 100) +((date.charAt(8) - '0') * 10)+ (date.charAt(9) - '0');
  }
}


void initWifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Starting the web server
  //server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  // delay(10000);

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
}
