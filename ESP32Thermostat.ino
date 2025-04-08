#define DHT_DEBUG

//Base Arduion Wifi
#include <WiFi.h>
#include <HTTPClient.h>

//Library U8g2 by Oliver V2.35.30
#include <U8g2lib.h>

//Library DHT sensor library by Adafruit V1.4.6
#include <DHT.h>

#include "Secrets.h"
#include "Icons.h"
#include "Menu.ino"

//Display Config
#define DISPLAY_CLOCK 22
#define DISPLAY_DATA 21

//Temp Sensor Config
#define DHT_PIN 19
#define DHT_TYPE DHT22

//WiFi Config
const struct WifiAp {
  char *name;
  char *ssid;
  char *password;
} wifiAps[] = {
  {"Home", SECRETS_HOME_SSID, SECRET_HOME_PASS}
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
  unsigned long dhtLastRead = 0;  
} state;

/*
  ***************************************************************************************
  * Display
  *   [height 16px][Logo] [Justify Center] [Minor Text] [justify left] [wifi bargraph icon]
  *   [height 16px] [Justify Center] [Major Text]
  ***************************************************************************************
  */
void display(std::string &minorText, std::string &majorText) {
    state.displayInstance.clearBuffer();
    state.displayInstance.setFont(u8g2_font_bpixeldouble_tr);
    state.displayInstance.setFontPosTop();
    state.displayInstance.setFontMode(1);

    state.displayInstance.drawXBM(0, 0, 16, 8, logo);
    Serial.println("Wrote logo");

    int x = 128 - state.displayInstance.getStrWidth(minorText.c_str());
    x = x/2;
    state.displayInstance.drawStr(x, 0, minorText.c_str());
    Serial.println("Wrote minor");

    // long rssi = WiFi.RSSI();
    long rssi = -40;
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
    Serial.println("Wrote wifi bars");

    x = 128 - state.displayInstance.getStrWidth(majorText.c_str());
    x = x/2;
    state.displayInstance.drawStr(x, 16, majorText.c_str());
    Serial.println("Wrote major ");

    state.displayInstance.sendBuffer();
}

void displayIdle() {
  std::string idleText = std::string("Idle");
  display(idleText, idleText);
}

MenuConfig menuConfig = {
  .buttonUpPin = 18,
  .buttonEnterPin = 5,
  .buttonDownPin = 17,
  .display = display,
  .displayIdle = displayIdle
};

IntegerSelectMenu tempMenu = IntegerSelectMenu("Set Temp (F)", state.expectedTemp, 40, 80);
IntegerSelectMenu timerMenu = IntegerSelectMenu("Set Timer (H)", state.expectedTimer, 1, 12);
EnumMenu menu = EnumMenu("Main", {
  &tempMenu,
  &timerMenu
});

// const int relayPin = 23;
// bool relayState = false;
// void testRelay() {
//   relayState =  !relayState;

//   digitalWrite(relayPin, relayState ? HIGH : LOW);
//   Serial.printf("Pin is set to %s.\n", relayState ? "true" : "false");
//   delay(1000);
// }


void setup() {
  Serial.begin(9600,SERIAL_8N1);
  Serial.println("Setup start.");
  // WifiAp currentWifiAP = wifiAps[state.currentWifiAP];
  // WiFi.begin(currentWifiAP.ssid, currentWifiAP.password);
  state.displayInstance.begin();
  state.dhtInstance.begin();

  // pinMode(relayPin, OUTPUT);

  pinMode(menuConfig.buttonUpPin, INPUT_PULLDOWN);
  pinMode(menuConfig.buttonEnterPin, INPUT_PULLDOWN);
  pinMode(menuConfig.buttonDownPin, INPUT_PULLDOWN);
  Serial.println("Setup done.");
}

// void readNewsState() {
//   Serial.println("readNewsState");

//   if (WiFi.status() != WL_CONNECTED) {
//     //TODO state = connecting
//     return;
//   }

//   HTTPClient http;
//   http.begin(sites[state.currentNewsFeed].url);
//   int httpCode = http.GET();

//   if (httpCode != HTTP_CODE_OK) {
//     //TODO, frequently returns -7, Why?
//     char logBuffer[textBufferSize];
//     sprintf(logBuffer, "httpCode = %d", httpCode);
//     Serial.println(logBuffer);
//       //TODO, cleaner error handling
//   }

//   WiFiClient *stream = http.getStreamPtr();

//   char *startTag = "<title>";
//   char *endTag = "</title>";
//   char inputBuffer[textBufferSize] = {0};
//   inputBuffer[textBufferSize-1] = 0;
//   int bufferInputIndex = 0;
//   int bufferOutputIndex = 0;
//   bool writing = false;


//   while (http.connected() && stream->available()) {

//     //TODO parse out headlines
//     char inputChar = stream->read();

//     for ( int index = 0; index < textBufferSize - 2; index++) {
//       inputBuffer[index] = inputBuffer[index + 1];
//     }
//     inputBuffer[textBufferSize-2] = inputChar;
//     inputBuffer[textBufferSize-1] = 0;

//     char logBuffer[100];
//     sprintf(logBuffer, "writing: %d, input char %c, buffer '%s'", writing, inputChar, inputBuffer);
//     Serial.println(logBuffer);

//     if (strstr(inputBuffer, startTag) != NULL) {
//       writing = true;
//       memset(inputBuffer, ' ', textBufferSize-1);
//     } else if (strstr(inputBuffer, endTag) != NULL) {
//       writing = false;
//       inputBuffer[textBufferSize-strlen(endTag)] = 0;
    
//     } else if (writing) {
//       displayBuffer(inputBuffer);
//     }
//   }
//   http.end();
// }

void testTempSensor() {
  //Only run once ever 3 seconds, and properly handle the rollover
  unsigned long currentTimeStamp = millis();
  if (currentTimeStamp >= state.dhtLastRead 
    && currentTimeStamp - state.dhtLastRead <= 3000) {
    return;
  }
  state.dhtLastRead = currentTimeStamp;

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
}

void loop() {
  Serial.println("");
  Serial.println("");
  Serial.println("Loop");
  menu.process(menuConfig);

  //httpWeb process

  //trigger relay

  testTempSensor();
//  testRelay();
}

