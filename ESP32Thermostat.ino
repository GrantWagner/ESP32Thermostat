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
  while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
    currentTime = millis();
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      Serial.write(c);                    // print it out the serial monitor
      header += c;
      if (c == '\n') {                    // if the byte is a newline character
        // if the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 0) {
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          
          // turns the GPIOs on and off
          if (header.indexOf("GET /26/on") >= 0) {
            Serial.println("GPIO 26 on");
            output26State = "on";
            // digitalWrite(output26, HIGH);
          } else if (header.indexOf("GET /26/off") >= 0) {
            Serial.println("GPIO 26 off");
            output26State = "off";
            // digitalWrite(output26, LOW);
          } else if (header.indexOf("GET /27/on") >= 0) {
            Serial.println("GPIO 27 on");
            output27State = "on";
            // digitalWrite(output27, HIGH);
          } else if (header.indexOf("GET /27/off") >= 0) {
            Serial.println("GPIO 27 off");
            output27State = "off";
            // digitalWrite(output27, LOW);
          }
          
          // Display the HTML web page
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          // CSS to style the on/off buttons 
          // Feel free to change the background-color and font-size attributes to fit your preferences
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
          client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
          client.println(".button2 {background-color: #555555;}</style></head>");
          
          // Web Page Heading
          client.println("<body><h1>ESP32 Web Server</h1>");
          
          // Display current state, and ON/OFF buttons for GPIO 26  
          client.println("<p>GPIO 26 - State " + output26State + "</p>");
          // If the output26State is off, it displays the ON button       
          if (output26State=="off") {
            client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
          } else {
            client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
          } 
              
          // Display current state, and ON/OFF buttons for GPIO 27  
          client.println("<p>GPIO 27 - State " + output27State + "</p>");
          // If the output27State is off, it displays the ON button       
          if (output27State=="off") {
            client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
          } else {
            client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
          }
          client.println("</body></html>");
          
          // The HTTP response ends with another blank line
          client.println();
          // Break out of the while loop
          break;
        } else { // if you got a newline, then clear currentLine
          currentLine = "";
        }
      } else if (c != '\r') {  // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      }
    }
  }

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

