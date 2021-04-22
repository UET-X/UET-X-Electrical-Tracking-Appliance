#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WifiLocation.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BLYNK_PRINT Serial 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

/*Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)*/
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

/*Comment this out to disable prints and save space*/
char auth[] = "WqCybI54VKA8FnSL94pC4BDxkXhPOGwm"; //Blynk app auth
const char* googleApiKey = "AIzaSyCExI0EEOHrIN5q6_Kksp-G13QAeq8WQmY"; //Google API 

/* WiFi credentials */
char ssid[] = "UET-Wifi"; //Wifi name
char pass[] = "12345689"; //Wifi password
 
SimpleTimer timer;
 
WidgetMap myMap(V0); //Pin to V0

/*DS18B20 on arduino pin2 corresponds to D4 on physical board "D4 pin on the nodemcu Module"*/
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature DS18B20(&oneWire);

WifiLocation location(googleApiKey);

float temp;
int move_index = 1; //Index in Blynk app
float zonex1 = 22 , zoney1 = 105 , zonex2 = 23 , zoney2 = 106;  //Coordinate of quarantine area 

void setup() 
{
	Serial.begin(9600);
	Serial.println("Start");
	/*Set WiFi to station mode and disconnect from an AP if it was previously connected*/
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.display();
	display.clearDisplay();
	Blynk.begin(auth, ssid, pass);
	DS18B20.begin();
	delay(500);
	timer.setInterval(1000L, getSendData);
	Serial.println("Setup done");
}
 
void loop() 
{
	timer.run(); // Initiates SimpleTimer
	Blynk.run();
	displayOnScreen();
}
 
/*Send Sensor data to Blynk*/
void getSendData()
{
	DS18B20.requestTemperatures(); 
	temp = DS18B20.getTempCByIndex(0); // Celcius
	Serial.println(temp);
	Blynk.virtualWrite(V3, temp); //virtual pin V3
	if(temp < 30)
	{
		Blynk.email("CAUTIONS!", "Device has been removed!"); //Warning when patient remove the device
	}
	/*Get location using Google API*/
	location_t loc = location.getGeoFromWiFi();
	float latitude = loc.lat;
	float longitude = loc.lon;
	myMap.location(move_index, latitude, longitude, "WiFi_Location");
	if(latitude < zonex1 || latitude > zonex2 || longitude < zoney1 || longitude > zoney2)
	{
		Blynk.email( "WARNING!","Patient has left the quarantine area"); //Warning when patient leave the quarantine zone
	}
}

void displayOnScreen()
{
	display.clearDisplay();
	display.setTextSize(1);             // Normal 1:1 pixel scale
	display.setTextColor(SSD1306_WHITE);        // Draw white text
	display.setCursor(0,0);             // Start at top-left corner
	display.println("Your Temperature:");
	display.println("-----------------");
	display.setTextSize(3);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(28, 27);
	display.print(temp, 1);
	display.print((char)247);
	display.display();
	delay(1000);
}
