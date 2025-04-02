#define DHT_DEBUG

//Base Arduion Wifi
#include <WiFi.h>
#include <HTTPClient.h>

//Library CWW Morse Transmit by Ralph Iden V1.2.1
#include <cww_MorseTx.h>

//Library U8g2 by Oliver V2.35.30
#include <U8g2lib.h>

//Library DHT sensor library by Adafruit V1.4.6
#include <DHT.h>

#include "Icons.h"
#include "Menu.ino"

//Morse Relay config
#define CW_SPEED 15
#define CW_PIN 2

//Display Config
#define DISPLAY_CLOCK 22
#define DISPLAY_DATA 21

//Temp Sensor Config
#define DHT_PIN 19
#define DHT_TYPE DHT22

//Button config
#define BUTTON_UP_PIN 18
#define BUTTON_ENTER_PIN 5
#define BUTTON_DOWN_PIN 17

//WiFi Config
const struct WifiAp {
  char *name;
  char *ssid;
  char *password;
} wifiAps[] = {
  {"Home", "PrettyFlyForAWifi", "599a3c7d7ac9430fb8f4df0d338ee3a5"}
};

//State vars
const int textBufferSize = 13;
struct stateStruct {
  int expectedTemp = 68;
  int expectedTimer = 4;

  int currentWifiAP = 0;

  U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C displayInstance = 
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, DISPLAY_CLOCK, DISPLAY_DATA);

  DHT dhtInstance = DHT(DHT_PIN, DHT_TYPE);
  
} state;

const EnumMenu menu = EnumMenu("Main Menu", {
  IntegerSelectMenu("Set Temp (F)", state.expectedTemp, 40, 80),
  IntegerSelectMenu("Set Timer (H)", state.expectedTimer, 1, 12),
  Item("Exit", [](){ 
    Serial.println("Exit"); 
  })
});


const int relayPin = 23;
bool relayState = false;
void testRelay() {
  relayState =  !relayState;

  digitalWrite(relayPin, relayState ? HIGH : LOW);
  Serial.printf("Pin is set to %s.\n", relayState ? "true" : "false");
  delay(1000);
}

void setup() {
  Serial.begin(9600,SERIAL_8N1);
  WifiAp currentWifiAP = wifiAps[state.currentWifiAP];
  WiFi.begin(currentWifiAP.ssid, currentWifiAP.password);
  state.displayInstance.begin();
  state.dhtInstance.begin();

  pinMode(relayPin, OUTPUT);

  pinMode(BUTTON_UP_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_ENTER_PIN, INPUT_PULLDOWN);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLDOWN);
}

void readNewsState() {
  Serial.println("readNewsState");

  if (WiFi.status() != WL_CONNECTED) {
    //TODO state = connecting
    return;
  }

  HTTPClient http;
  http.begin(sites[state.currentNewsFeed].url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    //TODO, frequently returns -7, Why?
    char logBuffer[textBufferSize];
    sprintf(logBuffer, "httpCode = %d", httpCode);
    Serial.println(logBuffer);
      //TODO, cleaner error handling
  }

  WiFiClient *stream = http.getStreamPtr();

  char *startTag = "<title>";
  char *endTag = "</title>";
  char inputBuffer[textBufferSize] = {0};
  inputBuffer[textBufferSize-1] = 0;
  int bufferInputIndex = 0;
  int bufferOutputIndex = 0;
  bool writing = false;


  while (http.connected() && stream->available()) {

    //TODO parse out headlines
    char inputChar = stream->read();

    for ( int index = 0; index < textBufferSize - 2; index++) {
      inputBuffer[index] = inputBuffer[index + 1];
    }
    inputBuffer[textBufferSize-2] = inputChar;
    inputBuffer[textBufferSize-1] = 0;

    char logBuffer[100];
    sprintf(logBuffer, "writing: %d, input char %c, buffer '%s'", writing, inputChar, inputBuffer);
    Serial.println(logBuffer);

    if (strstr(inputBuffer, startTag) != NULL) {
      writing = true;
      memset(inputBuffer, ' ', textBufferSize-1);
    } else if (strstr(inputBuffer, endTag) != NULL) {
      writing = false;
      inputBuffer[textBufferSize-strlen(endTag)] = 0;
    
    } else if (writing) {
      displayBuffer(inputBuffer);
    }
  }
  http.end();
}

/*
 ***************************************************************************************
 * Display
 *   [height 8px][Logo] [Source Name] [justify left] SSID: [SSID] [wifi bargraph icon]
 *   [height 1px]
 *   [height 1px] [morse line out]
 *   [height 1px]
 *   [output or menu item]
 ***************************************************************************************
 */
void displayBuffer(char *logBuffer) {
//    Serial.println(logBuffer);
    
    state.morseInstance.send(logBuffer[textBufferSize-2]);

    state.displayInstance.clearBuffer();
    state.displayInstance.drawXBM(0, 0, 16, 8, logo);

    //TODO draw the feed name, draw the ap name

    long rssi = WiFi.RSSI();
    if (rssi < -90)
      state.displayInstance.drawXBM(111, 0, 16, 8, wifi0B);
    else if (rssi < -70)
      state.displayInstance.drawXBM(111, 0, 16, 8, wifi1B);
    else if (rssi < -65)
      state.displayInstance.drawXBM(111, 0, 16, 8, wifi2B);
    else if (rssi < -30)
      state.displayInstance.drawXBM(111, 0, 16, 8, wifi3B);
    else
      state.displayInstance.drawXBM(111, 0, 16, 8, wifi4B);

    state.displayInstance.setFont(u8g2_font_profont22_mf);
    state.displayInstance.drawStr(0, 25, logBuffer);
    state.displayInstance.sendBuffer();
}

void testTempSensor() {
  float hum = state.dhtInstance.readHumidity();
  float temp = state.dhtInstance.readTemperature();
  float temp2 = state.dhtInstance.convertCtoF(temp);

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print("%, Temp: ");
  Serial.print(temp);
  Serial.print(" C, ");
  Serial.print(temp2);
  Serial.println(" F.");
  delay(3000);
}

void testButtons() {
  int up = digitalRead(BUTTON_UP_PIN);
  int enter = digitalRead(BUTTON_ENTER_PIN);
  int down = digitalRead(BUTTON_DOWN_PIN);

  Serial.printf("Buttons, up: %s, down: %s, enter: %s.\n", 
    up == HIGH ? "HIGH" : "LOW",
    down == HIGH ? "HIGH" : "LOW",
    enter == HIGH ? "HIGH" : "LOW");

}

void loop() {

  //TODO input menu, three buttons, up down and select
  // - Wifi
  // +- [cycle through options]
  // +- back
  // - News Feed
  // +- [cycle through options]
  // +- back
  // - Mute/Unmute
  // - Exit

  // if state == inMenu
  //   waitButtonPush
  //   handleEvent
  // else if state == connecting && wifi not connected
  //   wifi.begin
  // else if state == connecing && wifi is connected
  //   state = readNews
  // else 
//  readNewsState();

  testTempSensor();
//  testRelay();
//    testButtons();
}

