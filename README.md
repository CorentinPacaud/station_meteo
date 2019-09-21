To start the project on a ESP8266, you need to create a config.h file containg the following param :

	 #define SSID "YOU_SSID"
	 #define SSID_PASSWORD "YOU_PASSWORD"
                       
	 #define OPENWEATHER_URL "http://api.openweathermap.org/data/2.5/weather?q=<yourCity>&units=metric&APPID=<yourAppId>"
	 #define TIME_URL "http://worldtimeapi.org/api/ip"
	 
	 #define THINGSPEAK_POST "http://api.thingspeak.com/update?api_key=<yourKey>"
	 #define THINGSPEAK_GETLAST_FIELD2 "http://api.thingspeak.com/channels/<channelId>/fields/2/last.csv?key=<yourKey>" // EXT TEMP
	 #define THINGSPEAK_GETLAST_FIELD4 "http://api.thingspeak.com/channels/<channelId>/fields/4/last.csv?key=<yourKey>" // WET OUT
	 #define THINGSPEAK_GET24H_FIELD1 "http://api.thingspeak.com/channels/<channelId>/fields/1.csv?key=<yourKey>&timezone=Europe/Paris&results=288"
	 #define THINGSPEAK_GET24H_FIELD2 "http://api.thingspeak.com/channels/<channelId>/fields/2.csv?key=<yourKey>&timezone=Europe/Paris&results=288"

 
 You also need to create an account on thingspeak and create 1 channel and 4 field : 1: Indoor temp, 2: outdoor temp, 3: indoor humidity, 4: outdoor humidity
 
 SCREEN
 -	BUSY (violet) => D2
 -	RST (blanc)   => D4
 -	DC (vert)     => D3
 -	CS (orange)   => D8
 -	CLK (jaune)   => D5
 -	DIN (bleu)    => D7
 -	GND (noir)    => gnd
 -	VCC (rouge)   => 3.3v
 
 DHT22 :
 DATA => D1
 