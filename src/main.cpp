#include <Arduino.h>

#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "ArduinoJson.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

const char* ssid = "----";     //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WiFi Name (SSID)
const char* password = "----------"; //!!!!!!!!!!!!!!!!!!!!!!!!! WIFI Password 

const uint8_t activeSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};
const uint8_t inactiveSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};
void update_screen(void* arg);

bool connected = false;

HTTPClient http;

// Initialize the OLED display using Wire library
//sda pin 4, scl pin 15

SSD1306  display(0x3c, 4, 15, GEOMETRY_128_64);

OLEDDisplayUi ui     ( &display );

const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 100; //Set size of the Json object
const String cryptoCompare = "https://min-api.cryptocompare.com/data/pricemultifull?fsyms=";

const String exchange = "Coinbase";
const uint8_t numOfCoins = 2;
String coinNames[numOfCoins] = {"BTC","ETH"};

struct cryptoCoin{
	String name;
	String price;
	String hr_percent_change;
	String day_percent_change;
};

//https://min-api.cryptocompare.com/data/pricemulti?fsyms=BTC,ETH&tsyms=USD&e=Coinbase&extraParams=your_app_name
class cryptoCoins{
		
	cryptoCoin coins[numOfCoins];

	public:
	bool updating;

	cryptoCoins(){		
		for(int x=0; x < numOfCoins; x++) coins[x] =  cryptoCoin{coinNames[x],"","",""};
		this->updating = false;		
	}	
  bool update(){

	  // Connect to WiFi
	  this->updating = true;
	  bool success = false;	  
	  if(WiFi.status() != WL_CONNECTED)  WiFi.begin(ssid, password);
	  	  
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(1000);
	  	Serial.println("Connecting to WiFi..");
	  }

	  Serial.println("Connected to SSID: " + WiFi.SSID());	  	  
	  String site = cryptoCompare;

	  for(int x=0; x < numOfCoins; x++) site += coinNames[x] + ",";
	  site += "&tsyms=USD";


	  site += "&e=";
	  site += exchange;


	  http.begin(site);
	  int httpCode = http.GET();

	  if (httpCode > 0) { //Check for the returning code

	        success = true;
	        String payload = http.getString();
		    
		    http.end(); //Free the resources
		    WiFi.disconnect(true);
		  	WiFi.mode(WIFI_OFF);
	       // Serial.println(httpCode);
	       // Serial.println(payload);

	    // Parse JSON object
	  	DynamicJsonBuffer jsonBuffer(capacity);
	 	JsonObject& root = jsonBuffer.parseObject(payload);
		  if (!root.success()) {
		    Serial.println(F("Parsing failed!"));		    
			}	
		

		/*Serial.print(F("BTC: "));
		Serial.println(root["USD"].as<char*>());
		Serial.print(F("ETH: "));	
		Serial.println(root["ETH"].as<char*>()); */

		for(int x=0; x < numOfCoins; x++) 	{
			this->coins[x].price = root["DISPLAY"][coinNames[x]]["USD"]["PRICE"].as<char*>();   
			this->coins[x].day_percent_change = root["DISPLAY"][coinNames[x]]["USD"]["CHANGEPCT24HOUR"].as<char*>();

 			}

	    }

	    else {
	      //Serial.println("Error on HTTP request");
	      http.end(); //Free the resources
		  WiFi.disconnect(true);
		  WiFi.mode(WIFI_OFF);	      
	    }

	  	this->updating = false;  	
	    return success;
	}
	cryptoCoin* getCoin(int coinIndex){
		return &this->coins[coinIndex];
	}

};
cryptoCoins crypto;

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  //display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  //display->drawString(128, 0, String(millis() / 1000));
  
  display->setTextAlignment(TEXT_ALIGN_RIGHT);   
  if(crypto.updating)	display->drawString(128, 0, "Updating");
  //Debug for the touch button
  //if(digitalRead(TOUCH_PIN))	display->drawString(0, 0, "Touched");
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 16 + y, "Crypto Ticker!"); 	

}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 10 + y,"BTC: " + crypto.getCoin(0)->price);
  display->drawString(0 + x, 30 + y,"24hr: " + crypto.getCoin(0)->day_percent_change  + "%");

}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 10 + y,"ETH: " + crypto.getCoin(1)->price);
  display->drawString(0 + x, 30 + y,"24hr: " + crypto.getCoin(1)->day_percent_change + "%");

}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Connected to SSID: " + WiFi.SSID());
}

// void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {drawFrame2, drawFrame3};

// how many frames are there?
int frameCount = 2;

//Remembers the last frame before it went to sleep.
RTC_DATA_ATTR uint8_t prevFrame = 0;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const uint8_t OLED_RESET_PIN = 16;


TaskHandle_t update_screen_handle = NULL;

void setup() {

  WiFi.begin(ssid, password);

	//Reset the OLED
  pinMode(OLED_RESET_PIN,OUTPUT);
  digitalWrite(OLED_RESET_PIN, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RESET_PIN, HIGH); // while OLED is running, must set GPIO16 in high

  //Initialize the serial terminal
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  //Lower the FPS to reduce power?
  ui.setTargetFPS(60);

	// Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);
  
  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  // display.flipScreenVertically();

  ui.setTimePerFrame(6000);

  //ui.disableAutoTransition();

  ui.switchToFrame(prevFrame);

  Serial.println("Initialized display");  

  //Create a new task to update the screen
  xTaskCreatePinnedToCore(
                    update_screen,   /* Function to implement the task */
                    "update_screen", /* Name of the task */
                    2000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    1,          /* Priority of the task */
                    &update_screen_handle,       /* Task handle. */
                    1);			/* Core number */
    
    crypto = cryptoCoins();
    crypto.update();    
   
}

int cryptoUpdate = 0;
const int updateInterval = 12000; //10 second update


void loop() {
 
  if(millis() - cryptoUpdate > updateInterval){
  	  cryptoUpdate = millis();  	  
  	  crypto.update();	  
  }	
  else{
	  vTaskDelay(5000/portTICK_PERIOD_MS);}
//vTaskDelete(NULL);

}

void update_screen(void* arg){
	while(true){
	int remainingTimeBudget = ui.update();

	if (remainingTimeBudget > 0) {
	vTaskDelay(remainingTimeBudget/portTICK_PERIOD_MS);
    }
	}
}


