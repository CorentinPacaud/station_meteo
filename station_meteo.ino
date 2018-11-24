/*
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


extern "C" {
  #include "user_interface.h" // this is for the RTC memory read/write functions
}
#define RTCMEMORYSTART 66
#define RTCMEMORYLEN 125

#include <GxEPD.h>
#include <GxFont_GFX.h>
#include <Adafruit_GFX.h>
#include <math.h>

#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w    // 4.2" b/w 400 x 300 px

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
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

//#include "DHT.h"
#include "DHTesp.h"

#define DHTPIN 5     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#include<SPI.h>

#define DEBUG true

  
typedef struct {
  // min, hour, d,m,y
  uint32_t minutes;
  uint32_t hours;
  uint32_t days;
  uint32_t months;
  uint32_t years;
} rtcStore;

rtcStore rtcValues;


GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD_Class display(io); // default selection of D4(=2), D2(=4)

const char* ssid = SSID;
const char* password = SSID_PASSWORD;

HTTPClient http;
HTTPClient https;
const char * headerKeys[] = {"Date"};

// TIME
//char timeUrl[] = TIME_URL;
const unsigned int GMT_D = 1;
unsigned int cHour = 0;
unsigned int cMin = 0;
unsigned int cDayStr = 0;
unsigned int cDay = 1;
unsigned int cMonth = 1;
unsigned int cYear = 1;

const String MONTHS[] = {"Jan", "Fev", "Mars","Avril", "Mai", "Juin", "Juil", "Aout", "Sept", "Oct", "Nov", "Dec"};
const String HEADERSMONTHS[]        = {"Jan" , "Feb" , "Mar" , "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const String DAYS[] = {"Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"};
const int daySpaces[] ={30,0,0,30,0,0,0}; // Espace pour centrer le text. 
const String HEADERSDAYS[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" , "Sun"};

bool fillRected = false;

const GFXfont* f = &FreeMonoBold9pt7b;

float tempIntCurr = 0.0f;
float tempIntMax = 0.0f;
float tempIntMin = 0.0f;

float tempExtCurr = 0.0f;
float tempExtMax = 0.0f;
float tempExtMin = 0.0f;

float humIntCurr = 0.0f;
float humExtCurr = 99.0f;

int weather = -1; //0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 = brouillard;  5 = nuages; 6 = soleil;
String sunset = "00:00";
String sunrise = "00:00";
bool startUp = true;

//DHT dht(DHTPIN, DHTTYPE);

DHTesp dht;
unsigned long currentMillis;

void setup() {

  // SWITCH OFF WIFI
  WiFi.forceSleepBegin();
  yield();
  
  #ifndef DEBUG
  currentMillis = millis();
  Serial.println(" PROD ");
  #endif
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

  display.init(115200); // enable diagnostic output on Serial

  //display.fillScreen(GxEPD_BLACK);
  
  int i = 0;
  //display.update();
  dht.setup(5, DHTesp::DHT22); // Connect DHT sensor to GPIO 17

  Serial.println("setup done");
  checkStart();

}

//TODO 
// TRY THIS :
// ESP.getResetReason()
// } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from deep-sleep
//      strcpy_P(buff, PSTR("Deep-Sleep Wake"));
      

void loop() {
   Serial.println("LOOP-----------------------------------------------------------------------");
   
  #ifdef DEBUG
    currentMillis = millis();
    Serial.println(" DEBUG ");
  #endif
  
    if(startUp){
      //getTime();
      Serial.println("STARTUP");
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
       
    // Display and Send temp every 5 min;
    if(cMin % 5 == 0 || startUp){
      initWifi();
      readTemperature();
      getInfo();
      
      WiFi.forceSleepBegin(); // Switch off wifi
      yield();
    }
   if(startUp || cMin == 0 || cMin % 5 == 0){
      Serial.println("Draw background");
      drawBackground();
      updateDate(false);
      displayTemperature(false);
      Serial.println("BIG UPDATE");
      displayTime(false);
      displayWeather(false);
      displaySunSetRise(false);
      display.update();
      startUp = false;
      //PRESET FOR PARTIAL UPDATE !
      display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
    }else{
      displayTime(true);
      //display.update();
    }
    saveData();
    
   #ifndef DEBUG
     ESP.deepSleep((60000-(millis()-currentMillis))*1000, WAKE_RFCAL);
     yield();
   #else
     int timeElapsed = millis()-currentMillis;
     timeElapsed = 60 - (timeElapsed / 1000);
     Serial.print("Time elapsed : ");
     Serial.println(timeElapsed);
     for(int i = 0;i<timeElapsed ; i++){
        delay(1000);
     }
   #endif
}

void checkStart(){
  String rstReason = ESP.getResetReason();
  Serial.print("Start reason: ");
  Serial.println(rstReason);
  if(rstReason == "External System"){
    // INIT RTC_MEM
    Serial.println("Init RTC MEM");
    startUp = true;
    rtcValues.minutes = cMin;
    rtcValues.hours = cHour;
    rtcValues.days = cDay;
    rtcValues.months = cMonth;
    rtcValues.years = cYear;
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcValues, sizeof(rtcValues));
  }else{
    //RETREIVE DATA FROM RTC.
    startUp = false;
    if(ESP.rtcUserMemoryRead(0,(uint32_t*) &rtcValues, sizeof(rtcValues))){
    Serial.println("READ RTC MEM");
      cMin = rtcValues.minutes;
      cHour = rtcValues.hours;
      cDay = rtcValues.days;
      cMonth = rtcValues.months;
      cYear = rtcValues.years;
    }
  }
}

void saveData(){
  rtcValues.minutes = cMin;
  rtcValues.hours = cHour;
  rtcValues.days = cDay;
  rtcValues.months = cMonth;
  rtcValues.years = cYear;
  #ifdef DEBUG
    Serial.print("Data saved: ");
    Serial.print(rtcValues.minutes);
    Serial.print(", ");
    Serial.print(rtcValues.hours);
    Serial.print(", ");
    Serial.print(rtcValues.days);
    Serial.print(", ");
    Serial.print(rtcValues.months);
    Serial.print(", ");
    Serial.println(rtcValues.years); 
  #endif
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcValues, sizeof(rtcValues));
}

void readTemperature(){
  
    humIntCurr = dht.getHumidity();
    Serial.println("H:");
    Serial.println(humIntCurr);
    // Read temperature as Celsius (the default)
    tempIntCurr = dht.getTemperature();
    // Check if any reads failed and exit early (to try again).
    if (isnan(tempIntCurr) || isnan(tempIntCurr) || isnan(tempIntCurr)) {
      Serial.println("Failed to read from DHT sensor!");
      tempIntCurr = 99;
      humIntCurr = 99;
      return;
    }
    Serial.println("T:");
    Serial.println(tempIntCurr);
    
    // POST TEMP
    
  http.setTimeout(5000);
  http.begin(String(THINGSPEAK_POST) + "&field1=" + tempIntCurr);
  http.collectHeaders(headerKeys, 1);
  int httpCode = http.GET();   //Send the request
  Serial.print("Status code : "); 
  Serial.println(httpCode);
  if (httpCode == 200) {
    // SUCCESS
    Serial.println("POST T SUCCESS");
    //drawWifi();
    String headerDate = http.header("Date");
    parseTime2(headerDate);
  } else {
    Serial.println("POST T Error : " + httpCode);
    //drawNoWifi();
  }
  http.end();  //Close connection
  delay(1000);
}

void getInfo(){
   HTTPClient http;

  http.setTimeout(5000);
  String url = String(HEROKU_INFO);
  Serial.println("URL: "+url);
  http.begin(url);
  int httpCode = http.GET();   //Send the request
  if (httpCode == HTTP_CODE_OK) {
    // SUCCESS
      Serial.println("Success get Last 24h");
      String payload = http.getString();
      Serial.println("RESULT: " +payload);
      parseLast24HInTemp(payload);
  } else {
    Serial.println("POST T Error : " + httpCode);
    //drawNoWifi();
  }
  http.end();  //Close connection
}

void parseLast24HInTemp(String data){
    const size_t bufferSize = JSON_ARRAY_SIZE(4) + 2*JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + 6*JSON_OBJECT_SIZE(3) + 450;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    
    JsonArray& root = jsonBuffer.parseArray(data);
    
    JsonArray& root_ = root;
    
    JsonObject& root_0_max_int_temp = root_[0]["max_int_temp"];
    //const char* root_0_max_int_temp_created_at = root_0_max_int_temp["created_at"]; // "2018-10-13T23:02:29Z"
    //int root_0_max_int_temp_entry_id = root_0_max_int_temp["entry_id"]; // 17487
    tempIntMax = atof(root_0_max_int_temp["field1"]); // "27.30"
    
    JsonObject& root_0_min_int_temp = root_[0]["min_int_temp"];
    //const char* root_0_min_int_temp_created_at = root_0_min_int_temp["created_at"]; // "2018-10-13T22:10:48Z"
    //int root_0_min_int_temp_entry_id = root_0_min_int_temp["entry_id"]; // 17483
    tempIntMin = atof(root_0_min_int_temp["field1"]); // "25.20"
    
    JsonObject& root_1_max_ext_temp = root_[1]["max_ext_temp"];
    //const char* root_1_max_ext_temp_created_at = root_1_max_ext_temp["created_at"]; // "2018-10-14T21:32:40Z"
    //int root_1_max_ext_temp_entry_id = root_1_max_ext_temp["entry_id"]; // 17526
    tempExtMax= atof(root_1_max_ext_temp["field2"]); // "99"
    
    JsonObject& root_1_min_ext_temp = root_[1]["min_ext_temp"];
    //const char* root_1_min_ext_temp_created_at = root_1_min_ext_temp["created_at"]; // "2018-10-14T21:32:40Z"
    //int root_1_min_ext_temp_entry_id = root_1_min_ext_temp["entry_id"]; // 17526
    tempExtMin = atof(root_1_min_ext_temp["field2"]); // "99"
    
    JsonObject& root_2_ext_temp = root_[2]["ext_temp"];
    //const char* root_2_ext_temp_created_at = root_2_ext_temp["created_at"]; // "2018-10-14T21:32:40Z"
    //int root_2_ext_temp_entry_id = root_2_ext_temp["entry_id"]; // 17526
    tempExtCurr = atof(root_2_ext_temp["field1"]); // "26.60"

    JsonObject& root_3_weather = root_[3]["weather"];
    weather = root_3_weather["weather"]; // "4"
    const char* cSunset = root_3_weather["sunset"];
    sunset = String(cSunset); // "17:02"
    const char* cSunrise = root_3_weather["sunrise"];
    sunrise = String(cSunrise); // "7:50"


    #ifdef DEBUG
      Serial.print(tempIntMax);
      Serial.print("    ");
      Serial.print(tempIntMin);
      Serial.print("    ");
      Serial.print(tempIntCurr);
      Serial.print("    ");
      Serial.print(tempExtCurr);
      Serial.print("    ");
      Serial.print(tempExtMax);
      Serial.print("    ");
      Serial.println(tempExtMin);
      Serial.print("weather: ");
      Serial.print(weather);
      Serial.print(" sunset: ");
      Serial.print(sunset);
      Serial.print("  sunrise: ");
      Serial.println(sunrise);
   #endif
}

void displayTemperature(bool update){
  // INTERIOR---------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------
  // CURRENT
  int cursor_x = 5;
  int cursor_y = 90;
  int width = 90;
  int height = 30;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempIntCurr)));
  display.print("*C");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);


  // MAX 
  cursor_x = 25;
  cursor_y = 130;
  width = 45;
  height = 20;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempIntMax)));
  display.print("*");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);


  
  // MIN 
  cursor_x = 25;
  cursor_y = 175;
  width = 45;
  height = 20;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempIntMin)));
  display.print("*");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);

  
  // Humidity 
  cursor_x = 30;
  cursor_y = 230;
  width = 60;
  height = 30;
  display.fillRect(cursor_x - 5,cursor_y - height, width + 15, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(humIntCurr)));
  display.print("%");
  if(update)
    display.updateWindow(cursor_x+3, cursor_y - height, width, height+2, true);

  
  // EXTERIOR --------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------------------

  // CURRENT
   cursor_x = 305;
   cursor_y = 90;
   width = 90;
   height = 30;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempExtCurr)));
  display.print("*C");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);


  // MAX 
  cursor_x = 330;
  cursor_y = 130;
  width = 50;
  height = 20;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempExtMax)));
  display.print("*");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);


  
  // MIN 
  cursor_x = 330;
  cursor_y = 175;
  width = 50;
  height = 20;
  display.fillRect(cursor_x-5,cursor_y - height, width+10, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(tempExtMin)));
  display.print("*");
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height+2, true);
  
  // Humidity 
  cursor_x = 310;
  cursor_y = 230;
  width = 55;
  height = 30;
  display.fillRect(cursor_x - 5,cursor_y - height, width + 15, height+2, GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(String(round(humExtCurr)));
  display.print("%");
  if(update)
    display.updateWindow(cursor_x+3, cursor_y - height, width, height+2, true);
    
}

void drawBackground(){
   display.drawBitmap(image_data_background,0,0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_WHITE);
  //delay(5000);
}

void drawWifi(){
  // display.drawBitmap(image_data_wifi,10, 10, 20,20, GxEPD_WHITE);
  //display.updateWindow(10, 10, 25, 25, true);
  //delay(5000);
}

void drawNoWifi(){
 //  display.drawBitmap(image_data_no_wifi,10, 10, 20,20, GxEPD_WHITE);
  // display.updateWindow(10, 10, 25, 25, true);
}


void displayTime(bool update){
    int cursor_x = 95;
    int cursor_y = 170;
    int width = 210;
    int height = 56;
  display.setFont(&Roboto_Black_72); // hauteur: 55
  display.setTextColor(GxEPD_BLACK);
    display.fillRect(cursor_x,cursor_y-height, width, height, GxEPD_WHITE);
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

  if(update){
    Serial.println("Update time");
    display.updateWindow(cursor_x,cursor_y-height, width, height, true);
  }

}

void displayWeather(bool update){
  //0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 = brouillard;  5 = nuages; 6 = soleil;
  int cursor_x = 115;
  int cursor_y = 260;
  int width = 220;
  int height = 30;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  String s;
  if(weather == 0)
    s = "Orage";
  else if(weather == 1)
    s = "Bruine";
  else if(weather == 2)
    s = "Pluie";
  else if(weather == 3)
    s = "Neige";
  else if(weather == 4)
    s = "Brouillard";
  else if(weather == 5)
    s = "Nuages";
  else if(weather == 6)
    s = "Soleil";
  else s = "Erreur";
  display.print(s);
}

void displaySunSetRise(bool update){
  //Sunrise
  int cursor_x = 5;
  int cursor_y = 282;
  int width = 70;
  int height = 20;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(sunrise);

  cursor_x = 290;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setCursor(cursor_x, cursor_y);
  display.print(sunset);
}

void updateDate(bool update){
  //DISPLAY DATE ( DD MMM YYYY )
  int cursor_x = 85;
  int cursor_y = 35;
  int width = 220;
  int height = 30;
  int negHeight = 10;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  if (cDay < 10) {
     display.print("0");
  }
  display.print(cDay);
  display.print(" ");  
//  if (cMonth < 10) {
//     display.print("0");
//  }
  display.print(MONTHS[cMonth]);
  display.print(" ");
  display.print(cYear);

  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height, true);

// UPDATE DAY NAME
   cursor_x = 115 + daySpaces[cDayStr] ; // init for "Dimanche"
   cursor_y = 85;
   width = 170;
   height = 25;
   negHeight = 10;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  display.print(DAYS[cDayStr]);
  if(update)
    display.updateWindow(cursor_x, cursor_y - height, width, height, true);
}


/*void getTime() {

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
}*/


void parseTime2(String date){
  //Tue, 02 Oct 2018 21:22:57 GMT
  char * pch;
  char pDate[20];
  Serial.println("HEADERS : ");
  Serial.println(date);
  date.toCharArray(pDate, date.length());
  Serial.println(pDate);
  pch = strtok (pDate," :");
  int i = 0;
  Serial.println(pch);
  Serial.println("SPLIT");
  while (pch != NULL)
  {
    
  Serial.println(pch);
    if(i == 0){// jourStr
      String dayStr = String(pch);
      dayStr.remove(3);
      int j = 0;
      while(j<7){
        if(HEADERSDAYS[j] == dayStr){
          cDayStr = j;
          break;
        }
        j++;
      }
    }
    if(i == 1) // jour;
      cDay = atoi(pch);
    if(i == 2){// MOIS
      //TODO
      int j = 0;
      while(j<12){
        if(HEADERSMONTHS[j] == pch){
          cMonth = j;
          break;
        }
        j++;
      }
    }
    if(i == 3) //année
      cYear = atoi(pch);
    if(i == 4) // hours
      cHour = (atoi(pch)+GMT_D)%24;
    if(i == 5)
      cMin = atoi(pch);
    
      
    i++;
   pch = strtok (NULL, " :");
  }
  Serial.println("HOUR:" + String(cHour) + "  Min:" + String(cMin));
  Serial.println("Day:"+DAYS[cDayStr]+"  DAY:" + String(cDay) + "  Month:" + String(cMonth) + "  YEAR:" + String(cYear));
}


void initWifi() {
  Serial.println("Restart wifi");
  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);
  
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
  //Serial.println("Web server running. Waiting for the ESP IP...");
  // delay(10000);

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  delay(2000);
}
