#ifndef _menu_ino
#define _menu_ino

#include <string>
#include <vector>
#include "Icons.h"

//Button config
//TODO move to configs
#define BUTTON_UP_PIN 18
#define BUTTON_ENTER_PIN 5
#define BUTTON_DOWN_PIN 17

//Display Config
//TODO move to configs
#include <U8g2lib.h>
#define DISPLAY_CLOCK 22
#define DISPLAY_DATA 21
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C displayInstance = 
  U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, DISPLAY_CLOCK, DISPLAY_DATA);



class Menu {
  public: 
  // virtual bool isIdle() = 0;
  //TODO make virtual 
  bool process() {
    return true;
  };

  std::string getLabel() {
    return label;
  }

  protected:
  Menu(std::string inputLabel) {
    label = inputLabel;
  }

  private:
  std::string label;

};

class EnumMenu: public Menu {
  public: 
  EnumMenu(std::string inputLabel, std::vector<Menu> inputItems) : Menu(inputLabel) {
    items.reserve(inputItems.size());
    for (const Menu &item: inputItems)
      items.push_back(item);
  }

  void process();

  //Temp testing
  void begin() {
        displayInstance.begin();
  }

  private:
  Menu *childProcessing = NULL;
  int index = 0;
  std::vector<Menu> items;
};

class IntegerSelectMenu: public Menu {
  public:
  IntegerSelectMenu(std::string label, int &value, int min, int max): Menu(label), value(value) {
    this->min = min;
    this->max = max;
  }

  private:
  int &value;
  int min;
  int max;
};

class Item: public Menu {
  public:
  Item(std::string label, std::function<void()> body): Menu(label), body(body) {}

  private:
  std::function<void()> body;
};

void EnumMenu::process() {
  if (this->childProcessing != NULL) {
    Serial.println("Child PRocessing.");
    bool isDone = childProcessing->process();
    if (isDone)
      childProcessing = NULL;
  } else {
    Serial.println("Menu Processing.");
    int up = digitalRead(BUTTON_UP_PIN);
    int enter = digitalRead(BUTTON_ENTER_PIN);
    int down = digitalRead(BUTTON_DOWN_PIN);

    Serial.printf("Buttons, up: %s, down: %s, enter: %s.\n", 
      up == HIGH ? "HIGH" : "LOW",
      down == HIGH ? "HIGH" : "LOW",
      enter == HIGH ? "HIGH" : "LOW");

    if (down == HIGH) {
      Serial.println("Processing Down");
      index--;
      if (index < 0)
        index = 0;
    } else if (up == HIGH) {
      Serial.println("Processing UP");
      index--;
      if (index >= items.size())
        index = items.size()-1;
    } else if (enter == HIGH) {
      Serial.println("Processing Enter");
      childProcessing = &items[index];
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
    //TODO, make this a function passed in
    // char* minorText = this->getLable().c_str();
    // char* majorText = items[index].getLable().c_str();
    Serial.printf("Items size %d, index %d.\n", items.size(), index);

    std::string minorText = getLabel();
    Serial.printf("Label 1 are %s.\n", minorText.c_str());
    std::string majorText = items[index].getLabel().c_str();
    Serial.printf("Label 2 are %s, %s.\n", minorText.c_str(), majorText.c_str());

    displayInstance.clearBuffer();
    displayInstance.setFont(u8g2_font_bpixeldouble_tr);
    displayInstance.setFontPosTop();
    displayInstance.setFontMode(1);

    displayInstance.drawXBM(0, 0, 16, 8, logo);
    Serial.println("Wrote logo");

    int x = 128 - displayInstance.getStrWidth(minorText.c_str());
    x = x/2;
    displayInstance.drawStr(x, 0, minorText.c_str());
    Serial.println("Wrote minor");

    // long rssi = WiFi.RSSI();
    long rssi = -40;
    if (rssi < -90)
      displayInstance.drawXBM(111, 0, 16, 8, wifi0B);
    else if (rssi < -70)
      displayInstance.drawXBM(111, 0, 16, 8, wifi1B);
    else if (rssi < -65)
      displayInstance.drawXBM(111, 0, 16, 8, wifi2B);
    else if (rssi < -30)
      displayInstance.drawXBM(111, 0, 16, 8, wifi3B);
    else
      displayInstance.drawXBM(111, 0, 16, 8, wifi4B);
    Serial.println("Wrote wifi bars");

    x = 128 - displayInstance.getStrWidth(majorText.c_str());
    x = x/2;
    displayInstance.drawStr(x, 16, majorText.c_str());
    Serial.println("Wrote major ");

    displayInstance.sendBuffer();
  }
}

#endif
