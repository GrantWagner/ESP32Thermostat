#include <Arduino.h>
#define DHT_DEBUG

//Base Arduino Wifi
#include <WiFi.h>
#include <HTTPClient.h>

//Library U8g2 by Oliver V2.35.30
//For the Display
#include <U8g2lib.h>

//Library DHT sensor library by Adafruit V1.4.6
//For the humidity and temp sensor
#include <DHT.h>

#include "../include/Secrets.h"
#include "../Icons.h"
#include "../include/Menu.h"

//TODO Created manually by running 'xxd -i WebPage.html > WebPage.html.h'
#include "../include/WebPage.html.h"

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

//TODO move to state
//TODO can this be SSH?
WiFiServer server(80);


void setup() {
  Serial.begin(9600,SERIAL_8N1);
  delay(500);
  Serial.println("Setup start.");

//TODO wifi stuff setup
  WifiAp currentWifiAP = wifiAps[state.currentWifiAP];
  WiFi.setHostname("GarageThermostat");
  WiFi.begin(currentWifiAP.ssid, currentWifiAP.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
//TODO end wifi

  state.displayInstance.begin();
  state.dhtInstance.begin();

  // pinMode(relayPin, OUTPUT);

  pinMode(menuConfig.buttonUpPin, INPUT_PULLDOWN);
  pinMode(menuConfig.buttonEnterPin, INPUT_PULLDOWN);
  pinMode(menuConfig.buttonDownPin, INPUT_PULLDOWN);
  Serial.println("Setup done.");
}



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


// Variable to store the HTTP request
String header;
String output26State = "off";
String output27State = "off";
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000; //ms = 2 seconds

//from https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
void httpServerProcess() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (!client)
    return;

  currentTime = millis();
  previousTime = currentTime;
  Serial.println("New Client.");          // print a message out in the serial port
  String currentLine = "";                // make a String to hold incoming data from the client

  boolean readingHeader = true;
  //TODO reading body
  boolean readingSecondBody = false;
  int contentLength = 0;
  while (client.connected() 
      && currentTime - previousTime <= timeoutTime
      && (!readingSecondBody
       || currentLine.length() <= contentLength)) {  // loop while the client's connected

    currentTime = millis();
    if (!client.available()) continue;

    char c = client.read();             // read a byte, then
    Serial.write(c);                    // print it out the serial monitor
    if (readingHeader)
      header += c;

    if (c == '\r') {
    } else if ((c == '\n') && (currentLine.length() != 0)) {

      for (char &c: currentLine)
        c = std::tolower(c);
      //TODO rename, correct spelling
      int contentLegthIdex = currentLine.indexOf("content-length: ");
      if (contentLegthIdex > 0) {
        sscanf(currentLine.c_str() + contentLegthIdex, "%d", &contentLength);
        Serial.printf("\nContent Length is %d\n", contentLength);
      }

      readingHeader = false;
      currentLine = "";
    } else if (c != '\n') {
      currentLine += c;      // add it to the end of the currentLine
    } else if ( header.indexOf("POST") > 0 && !readingSecondBody) {
      Serial.println("Handling body seperator");
      readingSecondBody = true;
//      contentLength = 101;
      currentLine = "";
    }
  }

  Serial.println();
  Serial.print("Current Line is ");
  Serial.println(currentLine);
      
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Connection: close");
  
  // turns the GPIOs on and off
  char * jsonBody = R"=({
                        "currentTemp": 45,
                        "targetTemp": 68,
                        "maxTimerSeconds": 14400,
                        "currentTimerSeconds": 5025,
                        "running": false
                      }
    )=";
  if (header.indexOf("GET /data") >= 0) {
    client.println("Content-type:application/json");
    client.println();
    client.println(jsonBody);
  } else if (header.indexOf("POST /data") >= 0 ) {
    client.println("Content-type:application/json");
    client.println();
    client.println(jsonBody);
  } else {
    // Display the HTML web page
    client.println("Content-type:text/html");
    client.println();
    client.println((char*)WebPage_html);
  }
  
  // The HTTP response ends with another blank line
  client.println();

  // Clear the header variable
  header = "";
  // Close the connection
  client.stop();
  Serial.println("Client disconnected.");
  Serial.println("");
}

void loop() {
  // Serial.println("");
  // Serial.println("");
  // Serial.println("Loop");
  // menu.process(menuConfig);

  httpServerProcess();

  //trigger relay

  // testTempSensor();
}

