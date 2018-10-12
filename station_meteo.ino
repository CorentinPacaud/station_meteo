extern "C" {
  #include "user_interface.h" // this is for the RTC memory read/write functions
}

#define RTCMEMORYSTART 66
#define RTCMEMORYLEN 125

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
#include "no_wifi.c"
#include "wifi.c"

#include "DHT.h"

#define DHTPIN 5     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

#include<SPI.h>

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
const char * headerKeys[] = {"Date"};

// TIME
char timeUrl[] = TIME_URL;
const unsigned int GMT_D = 2;
unsigned int cHour = 0;
unsigned int cMin = 0;
unsigned int cDay = 1;
unsigned int cMonth = 1;
unsigned int cYear = 1;

const String MONTHS[] = {"Jan", "Fev", "Mars","Avril", "Mai", "Juin", "Juil", "Aout", "Sept", "Oct", "Nov", "Dec"};
const String HEADERSMONTHS[]        = {"Jan" , "Feb" , "Mar" , "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

bool fillRected = false;

const GFXfont* f = &FreeMonoBold9pt7b;

bool startUp = true;

DHT dht(DHTPIN, DHTTYPE);
float currentTemp = 0.0;
  unsigned long currentMillis;

void setup() {
  currentMillis = millis();
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  initWifi();

  display.init(115200); // enable diagnostic output on Serial

  //display.fillScreen(GxEPD_BLACK);
  
  int i = 0;
  //display.update();

  Serial.println("setup done");
  checkStart();
}

//TODO 
// TRY THIS :
// ESP.getResetReason()
// } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from deep-sleep
//      strcpy_P(buff, PSTR("Deep-Sleep Wake"));
      

void loop() {
   
    if(startUp){
      //getTime();
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
    if(startUp || cMin == 0 || cMin == -1){
      drawBackground();
    }
    // Display and Send temp every 5 min;
    if(cMin % 5 == 0 || startUp){
      readTemperature();
      displayTemperature();
    }
    if(startUp|| cMin%60==0){
      updateDate();
      startUp = false;
   }
   
    if(cHour == 0 && cMin == 0){
      //getTime();
    }
    displayTime();
    saveData();
   ESP.deepSleep((60000-(millis()-currentMillis))*1000);
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
  ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcValues, sizeof(rtcValues));
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
    
  http.setTimeout(5000);
  http.begin(String(THINGSPEAK_POST) + "&field1=" + currentTemp);
  http.collectHeaders(headerKeys, 1);
  int httpCode = http.GET();   //Send the request
  if (httpCode == 200) {
    // SUCCESS
    Serial.println("POST T SUCCESS");
    drawWifi();
    String headerDate = http.header("Date");
    parseTime2(headerDate);
  } else {
    Serial.println("POST T Error : " + httpCode);
    drawNoWifi();
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
}

void drawBackground(){
   display.drawPicture(image_data_background3, sizeof(image_data_background3));
  //delay(5000);
}

void drawWifi(){
   display.drawBitmap(image_data_wifi,10, 10, 20,20, GxEPD_WHITE);
  display.updateWindow(10, 10, 25, 25, true);
  //delay(5000);
}

void drawNoWifi(){
   display.drawBitmap(image_data_no_wifi,10, 10, 20,20, GxEPD_WHITE);
   display.updateWindow(10, 10, 25, 25, true);
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

  display.updateWindow(cursor_x, cursor_y-height, width, height, true);

}

void updateDate(){
  int cursor_x = 115;
  int cursor_y = 95;
  int width = 170;
  int height = 30;
  int negHeight = 10;
  display.fillRect(cursor_x,cursor_y - height, width, height+negHeight, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);
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
  display.updateWindow(cursor_x, cursor_y - height, width, height + negHeight, true);
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
  }
  int indexD = date.indexOf('.');
  if (indexD >= 0) {
    cDay = ((date.charAt(0) - '0') * 10) + (date.charAt(1) - '0');
    cMonth = ((date.charAt(3) - '0') * 10) + (date.charAt(4) - '0');
    cYear = ((date.charAt(6) - '0') * 1000) +((date.charAt(7) - '0') * 100) +((date.charAt(8) - '0') * 10)+ (date.charAt(9) - '0');
  }
}


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
  Serial.println("DAY:" + String(cDay) + "  Month:" + String(cMonth) + "  YEAR:" + String(cYear));
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