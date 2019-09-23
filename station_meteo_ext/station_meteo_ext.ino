#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "config.h"

//#define DEBUG true

DHTesp dht;
HTTPClient http;


unsigned long currentMillis;

void setup()
{
  currentMillis = millis();
  Serial.begin(115200);
  Serial.println("Start");
  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead: 
  dht.setup(5, DHTesp::DHT22); // Connect DHT sensor to GPIO 5
  
  initWifi();
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humExtCurr = dht.getHumidity();
  float tempExtCurr = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humExtCurr, 1);
  Serial.print("\t\t");
  Serial.print(tempExtCurr, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(tempExtCurr), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(tempExtCurr, humExtCurr, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(tempExtCurr), humExtCurr, true), 1);

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(String(THINGSPEAK_POST) + "&field2=" + tempExtCurr + "&field4=" + humExtCurr);
  int httpCode = http.GET();   //Send the request
  Serial.print("Status code : "); 
  Serial.println(httpCode);
  if (httpCode == 200) {
    // SUCCESS
    Serial.println("POST T SUCCESS");
  } else {
    Serial.println("POST T Error : " + httpCode);
  }
  http.end();  //Close connection
  delay(1000);
  
      
   #ifndef DEBUG
     ESP.deepSleep((60000-(millis()-currentMillis))*1000+240000*1000);
   #else
     int timeElapsed = millis()-currentMillis;
     timeElapsed = 60 - (timeElapsed / 1000);
     Serial.print("Time elapsed : ");
     Serial.println(timeElapsed);
     for(int i = 0;i<300 ; i++){
        delay(1000);
        if(i%60 == 0){
          Serial.print("--->min : ");
          Serial.println(i/60);
        }
     }
   #endif
}


void initWifi() {  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PASSWORD);

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
  delay(1000);
}

