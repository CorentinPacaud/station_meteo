/*
 * To start the project, you need to create a config.h file containg the following param :
 *	 #define SSID "YOU_SSID"
 *	 #define SSID_PASSWORD "YOU_PASSWORD"
                         
 *	 #define OPENWEATHER_URL "http://api.openweathermap.org/data/2.5/weather?q=<yourCity>&units=metric&APPID=<yourAppId>"
 *	 #define TIME_URL "http://worldtimeapi.org/api/ip"
 *	 
 *	 #define THINGSPEAK_POST "http://api.thingspeak.com/update?api_key=<yourKey>"
 *	 #define THINGSPEAK_GETLAST_FIELD2 "http://api.thingspeak.com/channels/<channelId>/fields/2/last.csv?key=<yourKey>" // EXT TEMP
 *	 #define THINGSPEAK_GETLAST_FIELD4 "http://api.thingspeak.com/channels/<channelId>/fields/4/last.csv?key=<yourKey>" // WET OUT
 *	 #define THINGSPEAK_GET24H_FIELD1 "http://api.thingspeak.com/channels/<channelId>/fields/1.csv?key=<yourKey>&timezone=Europe/Paris&results=288"
 *	 #define THINGSPEAK_GET24H_FIELD2 "http://api.thingspeak.com/channels/<channelId>/fields/2.csv?key=<yourKey>&timezone=Europe/Paris&results=288"
 *
 * 
 * You also need to create an account on thingspeak and create 1 channel and 4 field : 1: Indoor temp, 2: outdoor temp, 3: indoor humidity, 4: outdoor humidity
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

extern "C"
{
#include "user_interface.h" // this is for the RTC memory read/write functions
}
#define RTCMEMORYSTART 66
#define RTCMEMORYLEN 125

#include <GxEPD.h>
#include <GxFont_GFX.h>
#include <Adafruit_GFX.h>
#include <math.h>
#include "DTime.h"

#include <GxGDEW042T2/GxGDEW042T2.h> // 4.2" b/w    // 4.2" b/w 400 x 300 px

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

#define DHTPIN 5      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22 // there are multiple kinds of DHT sensors

#include <SPI.h>

//#define DEBUG
#define PROD

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
} rtcStore;

rtcStore rtcValues;

GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD_Class display(io); // default selection of D4(=2), D2(=4)

const char *ssid = SSID;
const char *password = SSID_PASSWORD;

const char *headerKeys[] = {"Date"};

// TIME
//char timeUrl[] = TIME_URL;
const unsigned int GMT_D = 1;
unsigned int cHour = 0;
unsigned int cMin = 0;
unsigned int cSec = 0;
unsigned int cDayStr = 0;
unsigned int cDay = 1;
unsigned int cMonth = 1;
unsigned int cYear = 1;

const String MONTHS[] = {"Jan", "Fev", "Mars", "Avril", "Mai", "Juin", "Juil", "Aout", "Sept", "Oct", "Nov", "Dec"};
//const String HEADERSMONTHS[]        = {"Jan" , "Feb" , "Mar" , "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const String DAYS[] = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
const int daySpaces[] = {0, 30, 30, 15, 30, 10, 20}; // Espace pour centrer le text. lun,mar,...
//const String HEADERSDAYS[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" , "Sun"};

const GFXfont *f = &FreeMonoBold9pt7b;

int tempIntCurr = 0;
int tempIntMax = 0;
int tempIntMin = 0;
int tempExtCurr = 0;
int tempExtMax = 0;
int tempExtMin = 0;
int humIntCurr = 0;
int humExtCurr = 99;

int weather = -1; //0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 = brouillard;  5 = nuages; 6 = soleil;

int sunsetHour = 0;
int sunsetMin = 0;

int sunriseHour = 0;
int sunriseMin = 0;

bool startUp = true;

int firstRestart = 0;

//DHT dht(DHTPIN, DHTTYPE);

DHTesp dht;
unsigned long currentMillis;

void setup()
{

	Serial.begin(115200);

	display.init(115200); // enable diagnostic output on Serial
	display.setRotation(0);
	// SWITCH OFF WIFI
	WiFi.forceSleepBegin();
	yield();

#ifndef DEBUG
	currentMillis = millis();
	Serial.println(" PROD ");
#endif

	Serial.println();
	Serial.println("setup");

	//display.fillScreen(GxEPD_BLACK);

	int i = 0;
	dht.setup(5, DHTesp::DHT22); // Connect DHT sensor to GPIO 17

	Serial.println("setup done");
	checkStart();
}

//TODO
// TRY THIS :
// ESP.getResetReason()
// } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from deep-sleep
//      strcpy_P(buff, PSTR("Deep-Sleep Wake"));

void loop()
{
	Serial.println("LOOP-----------------------------------------------------------------------");

	currentMillis = millis();
#ifdef DEBUG
	Serial.println(" DEBUG ");
#endif

	if (startUp)
	{
		//getTime();
		Serial.println("STARTUP");
	}
	else
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

	// Display and Send temp every 5 min;
	bool bigRefresh = cMin % 5 == 0 || startUp;
	//bool bigRefresh = startUp;
	if (bigRefresh)
	{
		saveData();
		initWifi();
		readTemperature();

		getLastValue(THINGSPEAK_GETLAST_FIELD4, &humExtCurr);
		getLastValue(THINGSPEAK_GETLAST_FIELD2, &tempExtCurr);

		getMinMaxTemp(THINGSPEAK_GET24H_FIELD1, &tempIntMax, &tempIntMin);
		getMinMaxTemp(THINGSPEAK_GET24H_FIELD2, &tempExtMax, &tempExtMin);
		getSunWeather();
		getTime();

		WiFi.forceSleepBegin(); // Switch off wifi
		yield();
	}
	if (bigRefresh)
	{
		Serial.println("BIG UPDATE");
		Serial.println("Draw background");
		drawBackground();
		Serial.println("End Draw background");
		Serial.println("Draw date");
		updateDate(false);
		Serial.println("END Draw date");
		Serial.println("Draw TEmp");
		displayTemperature(false);
		Serial.println("End Draw TEmp");
		Serial.println("Draw Time");
		displayTime(false);
		Serial.println("End Draw TIME");
		Serial.println("Draw weather");
		displayWeather(false);
		Serial.println("End Draw weather");
		Serial.println("Draw Sunsrise");
		displaySunSetRise(false);
		Serial.println("End Draw Sunsrise");
		Serial.println("UDPATE !");
		display.update();
		startUp = false;
	}
	else
	{
		drawBackground();
		updateDate(false);
		displayTemperature(false);
		displayWeather(false);
		displaySunSetRise(false);
		display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
		displayTime(true);
		display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
	}

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
	//timeElapsed = 5;
	Serial.print("Time elapsed : ");
	Serial.println(timeElapsed);

#ifndef DEBUG
	String rstReason = ESP.getResetReason();

	// Don't go to Deep sleep for first 5 min if external restart.
	// This is usefull to easily upload code when in deepsleep mod, just press the restart btn and you have 5 min to upload code)
	if (rstReason == "External System" && firstRestart < 0)
	{
		Serial.println("First Restart");
		for (int i = 0; i < timeElapsed; i++)
		{
			delay(1000);
		}
		firstRestart++;
	}
	else
	{
		Serial.println("Start deepSleep");
		display.powerDown();
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
		//RETREIVE DATA FROM RTC.
		startUp = false;
		if (ESP.rtcUserMemoryRead(0, (uint32_t *)&rtcValues, sizeof(rtcValues)))
		{
			Serial.println("READ RTC MEM");
			cMin = rtcValues.minutes;
			cHour = rtcValues.hours;
			cDay = rtcValues.days;
			cMonth = rtcValues.months;
			cYear = rtcValues.years;
			cDayStr = rtcValues.day;
			tempExtCurr = rtcValues.tempExtCurr;
			tempExtMax = rtcValues.tempExtMax;
			tempExtMin = rtcValues.tempExtMin;
			tempIntCurr = rtcValues.tempIntCurr;
			tempIntMax = rtcValues.tempIntMax;
			tempIntMin = rtcValues.tempIntMin;
			humExtCurr = rtcValues.humExtCurr;
			humIntCurr = rtcValues.humIntCurr;
			sunriseHour = rtcValues.sunriseHour;
			sunriseMin = rtcValues.sunriseMin;
			sunsetHour = rtcValues.sunsetHour;
			sunsetMin = rtcValues.sunsetMin;
			weather = rtcValues.weather;
		}
	}
}

void saveData()
{
	rtcValues.minutes = cMin;
	rtcValues.hours = cHour;
	rtcValues.days = cDay;
	rtcValues.months = cMonth;
	rtcValues.years = cYear;
	rtcValues.tempExtCurr = tempExtCurr;
	rtcValues.tempExtMax = tempExtMax;
	rtcValues.tempExtMin = tempExtMin;
	rtcValues.tempIntCurr = tempIntCurr;
	rtcValues.tempIntMax = tempIntMax;
	rtcValues.tempIntMin = tempIntMin;
	rtcValues.humExtCurr = humExtCurr;
	rtcValues.humIntCurr = humIntCurr;
	rtcValues.sunriseHour = sunriseHour;
	rtcValues.sunriseMin = sunriseMin;
	rtcValues.sunsetHour = sunsetHour;
	rtcValues.sunsetMin = sunsetMin;
	rtcValues.weather = weather;

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
	ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcValues, sizeof(rtcValues));
}

void readTemperature()
{
	Serial.println("Send temp int");
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
	http.begin(String(THINGSPEAK_POST) + "&field1=" + tempIntCurr + "&field3=" + humIntCurr);
	//http.collectHeaders(headerKeys, 1);
	int httpCode = http.GET(); //Send the request
	Serial.print("Status code : ");
	Serial.println(httpCode);
	if (httpCode == 200)
	{
		// SUCCESS
		Serial.println("POST T SUCCESS");
		//drawWifi();
		String headerDate = http.header("Date");
		//parseTime2(headerDate);
	}
	else
	{
		Serial.println("POST T Error : " + httpCode);
		//drawNoWifi();
	}
	http.end(); //Close connection
	delay(1000);
}

void displayTemperature(bool update)
{
	// INTERIOR---------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------
	// CURRENT
	int cursor_x = 5;
	int cursor_y = 85;
	int width = 90;
	int height = 30;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold18pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempIntCurr));
	display.print("*C");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// MAX
	cursor_x = 25;
	cursor_y = 130;
	width = 45;
	height = 20;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold12pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempIntMax));
	display.print("*");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// MIN
	cursor_x = 25;
	cursor_y = 175;
	width = 45;
	height = 20;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold12pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempIntMin));
	display.print("*");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// Humidity
	cursor_x = 30;
	cursor_y = 230;
	width = 60;
	height = 30;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 15, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold18pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(humIntCurr));
	display.print("%");
	if (update)
		display.updateWindow(cursor_x + 3, cursor_y - height, width, height + 2, true);

	// EXTERIOR --------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------

	// CURRENT
	cursor_x = 305;
	cursor_y = 85;
	width = 90;
	height = 30;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold18pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempExtCurr));
	display.print("*C");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// MAX
	cursor_x = 330;
	cursor_y = 130;
	width = 50;
	height = 20;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold12pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempExtMax));
	display.print("*");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// MIN
	cursor_x = 330;
	cursor_y = 175;
	width = 50;
	height = 20;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 10, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold12pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(tempExtMin));
	display.print("*");
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height + 2, true);

	// Humidity
	cursor_x = 310;
	cursor_y = 230;
	width = 55;
	height = 30;
	display.fillRect(cursor_x - 5, cursor_y - height, width + 15, height + 2, GxEPD_BLACK);
	display.setTextColor(GxEPD_WHITE);
	display.setFont(&FreeMonoBold18pt7b);
	display.setCursor(cursor_x, cursor_y);
	display.print(String(humExtCurr));
	display.print("%");
	if (update)
		display.updateWindow(cursor_x + 3, cursor_y - height, width, height + 2, true);
}

void drawBackground()
{
	display.fillRect(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK); // BACKGROUND BLACK
	display.fillRect(61, 0, 280, 50, GxEPD_WHITE);			// BACKGROUND TOP WHITE RECT (DATE)
	display.fillRect(100, 50, 200, 50, GxEPD_WHITE);		// BACKGROUND TOP WHITE RECT (DAY)
	display.fillRect(80, 100, 240, 97, GxEPD_WHITE);		// BACKGROUND TOP WHITE RECT (TIME)
	display.fillRect(100, 201, 200, 99, GxEPD_WHITE);		// BACKGROUND TOP WHITE RECT (METEO)
	display.fillRect(0, 250, GxEPD_WIDTH, 49, GxEPD_WHITE);		// BACKGROUND TOP WHITE RECT (SUNRISE, SUNSET)

	display.drawBitmap(image_data_home, 10, 10, 40, 40, GxEPD_WHITE);	// BACKGROUND ICON HOME
	display.drawBitmap(image_data_outdoor, 351, 10, 40, 40, GxEPD_WHITE);	// BACKGROUND ICON OUTDOOR
	display.drawBitmap(image_data_tempMax, 6, 111, 11, 30, GxEPD_WHITE);	// BACKGROUND ICON MAX TEMP LEFT
	display.drawBitmap(image_data_tempMin, 6, 158, 11, 30, GxEPD_WHITE);	// BACKGROUND ICON MIN TEMP LEFT
	display.drawBitmap(image_data_tempMax, 385, 111, 11, 30, GxEPD_WHITE);	// BACKGROUND ICON MAX TEMP RIGHT
	display.drawBitmap(image_data_tempMin, 385, 158, 11, 30, GxEPD_WHITE);	// BACKGROUND ICON MIN TEMP RIGHT
	display.drawBitmap(image_data_humidity, 375, 205, 20, 29, GxEPD_WHITE); // BACKGROUND ICON HUMIDITY RIGHT
	display.drawBitmap(image_data_humidity, 6, 205, 20, 29, GxEPD_WHITE);	// BACKGROUND ICON HUMIDITY LEFT
}

void drawWifi()
{
	// display.drawBitmap(image_data_wifi,10, 10, 20,20, GxEPD_WHITE);
	//display.updateWindow(10, 10, 25, 25, true);
	//delay(5000);
}

void drawNoWifi()
{
	//  display.drawBitmap(image_data_no_wifi,10, 10, 20,20, GxEPD_WHITE);
	// display.updateWindow(10, 10, 25, 25, true);
}

void displayTime(bool update)
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

	if (!update)
	{
		display.setFont(&Roboto_Black_72);
		display.setTextColor(GxEPD_BLACK);
		display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
		display.setCursor(box_x, cursor_y);
		display.print(s);
	}
	if (update)
	{
		display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
		display.setFont(&Roboto_Black_72);
		display.setTextColor(GxEPD_BLACK);
		display.updateWindow(box_x, box_y, box_w, box_h, true);
		//delay(500);
		display.setCursor(box_x, cursor_y);
		display.print(s);
		display.updateWindow(box_x, box_y, box_w, box_h, true);
		delay(500);
	}
}

void displayWeather(bool update)
{
	//0 =  orage; 1 = bruine; 2 = pluie; 3 = neige; 4 = brouillard;  5 = nuages; 6 = soleil;
	/*int cursor_x = 115;
  int cursor_y = 260;
  int width = 100;
  int height = 30;
  display.fillRect(cursor_x,cursor_y - height, width, height, GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);
  display.setCursor(cursor_x, cursor_y);
  String s;*/

	int width = 64;
	int height = 64;
	int posX = 168;
	int poxY = 215;

	if (weather == 0)
		display.drawBitmap(image_data_storm, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 1)
		display.drawBitmap(image_data_rain, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 2)
		display.drawBitmap(image_data_rain, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 3)
		display.drawBitmap(image_data_snow, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 4)
		display.drawBitmap(image_data_smogg, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 5)
		display.drawBitmap(image_data_cloud, posX, poxY, width, height, GxEPD_WHITE);
	else if (weather == 6)
		display.drawBitmap(image_data_sun, posX, poxY, width, height, GxEPD_WHITE);
	/*else s = "Erreur";
  display.print(");*/
}

void displaySunSetRise(bool update)
{
	String sunset = (sunsetHour < 10 ? "0" : "") + String(sunsetHour) + ":" + (sunsetMin < 10 ? "0" : "") + String(sunsetMin);
	Serial.println("Sunset: " + sunset);
	String sunrise = (sunriseHour < 10 ? "0" : "") + String(sunriseHour) + ":" + (sunriseMin < 10 ? "0" : "") + String(sunriseMin);
	Serial.println("Sunrise: " + sunrise);
	//Sunrise
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

void updateDate(bool update)
{
	//DISPLAY DATE ( DD MMM YYYY )
	//display.fillRect(cursor_x, cursor_y - height, width, height, GxEPD_WHITE);
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
		//display.print("0");
	}
	s += String(cDay);
	s += " ";

	//display.print(cDay);
	//display.print(" ");
	//  if (cMonth < 10) {
	//     display.print("0");
	//  }
	s += MONTHS[cMonth - 1];
	s += " ";
	s += cYear;
	//display.print(MONTHS[cMonth - 1]);
	//display.print(" ");
	//display.print(cYear);

	int16_t x, y;
	uint16_t w, h;
	//61, 0, 280, 50
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
	//	if (update)
	//		display.updateWindow(cursor_x, cursor_y - height, width, height, true);

	// UPDATE DAY NAME
	int cursor_x = 115 + daySpaces[cDayStr]; // init for "Dimanche"
	int cursor_y = 85;
	int width = 170;
	int height = 25;
	int negHeight = 10;
	//display.fillRect(cursor_x, cursor_y - height, width, height, GxEPD_WHITE);
	display.setTextColor(GxEPD_BLACK);
	display.setFont(&FreeMonoBold18pt7b);
	//100, 50, 200, 50
	display.getTextBounds(DAYS[cDayStr], 100, 50, &x, &y, &w, &h);
	int cX2 = 100 + (200 - w) / 2;
	int cY2 = 100 - (50 - h) / 2 * 1.5;
	display.setCursor(cX2, cY2);
	display.print(DAYS[cDayStr]);
	if (update)
		display.updateWindow(cursor_x, cursor_y - height, width, height, true);
}

void getTime()
{
	HTTPClient http;
	http.setTimeout(10000);
	http.begin(TIME_URL);
	int httpCode = http.GET();
	if (httpCode > 0)
	{					//Check the returning code
		String json = http.getString(); //Get the request response payload
		Serial.println(json);		//Print the response payload

		const size_t capacity = JSON_OBJECT_SIZE(15) + 350;
		DynamicJsonDocument doc(capacity);
		deserializeJson(doc, json);
		int day_of_week = doc["day_of_week"]; // 6
		cDayStr = day_of_week;
		rtcValues.day = cDayStr;
		const char *datetime = doc["datetime"]; // "2019-08-31T02:12:43.056352+02:00"
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
	http.setTimeout(15000);
	// Serial.println("URL: "+THINGSPEAK_GET24H_FIELD1);
	http.begin(url);
	int httpCode = http.GET(); //Send the request
	if (httpCode == HTTP_CODE_OK)
	{
		// SUCCESS
		Serial.println("Success get Last ");
		int lastI = http.getString().lastIndexOf(',');
		*value = -99;
		if (lastI != http.getString().length())
		{
			*value = int(round(http.getString().substring(lastI + 1, http.getString().length()).toFloat()));
			Serial.print("TempCurr");
			Serial.println(*value);
		}
	}
	else
	{
		Serial.println("POST T Error : " + httpCode);
		//drawNoWifi();
	}
	http.end(); //Close connection
}

void getSunWeather()
{
	Serial.print("getSunWeather : ");
	HTTPClient http;
	http.setTimeout(15000);
	http.begin(OPENWEATHER_URL);
	int httpCode = http.GET(); //Send the request
	if (httpCode == HTTP_CODE_OK)
	{
		// SUCCESS
		Serial.println("Success get Sun and Weather :" + http.getString());
		const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13) + 270;
		DynamicJsonDocument doc(capacity);

		//const char* json = "{\"coord\":{\"lon\":4.83,\"lat\":45.76},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":32.76,\"pressure\":1012,\"humidity\":30,\"temp_min\":31,\"temp_max\":35},\"visibility\":10000,\"wind\":{\"speed\":4.1,\"deg\":150},\"clouds\":{\"all\":0},\"dt\":1567263192,\"sys\":{\"type\":1,\"id\":6505,\"message\":0.0061,\"country\":\"FR\",\"sunrise\":1567227606,\"sunset\":1567275757},\"timezone\":7200,\"id\":2996944,\"name\":\"Lyon\",\"cod\":200}";

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

		Serial.printf("Sun : %d:%d - %d:%d", sunsetHour, sunsetMin, sunriseHour, sunriseMin);
	}
	else
	{
		Serial.println("POST T Error : " + httpCode);
		//drawNoWifi();
	}
	http.end(); //Close connection
}

void getMinMaxTemp(String url, int *maxTemp, int *minTemp)
{
	Serial.print("getMinMaxTemp : ");
	HTTPClient http;
	http.setTimeout(15000);
	// Serial.println("URL: "+THINGSPEAK_GET24H_FIELD1);
	http.begin(url);
	int httpCode = http.GET(); //Send the request
	if (httpCode == HTTP_CODE_OK)
	{
		// SUCCESS
		Serial.println("Success get Last 24h");
		getMaxMin(http, maxTemp, minTemp);
	}
	else
	{
		Serial.println("POST T Error : " + httpCode);
		//drawNoWifi();
	}
	http.end(); //Close connection
}

void getMaxMin(HTTPClient &http, int *tempMax, int *tempMin)
{
	//int ssize =  http.getString().length();
	//int line = 0;
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

void mylog(String a)
{
	Serial.println("LOG");
}

void initWifi()
{
	Serial.println("Restart wifi");
	WiFi.forceSleepWake();
	WiFi.mode(WIFI_STA);

	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
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
