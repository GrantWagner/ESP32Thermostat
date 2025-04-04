#ifndef _menu_ino
#define _menu_ino

#include <string>
#include <vector>
#include "Icons.h"

struct MenuConfig {
  uint8_t buttonUpPin;
  uint8_t buttonEnterPin;
  uint8_t buttonDownPin;

  std::function<void(std::string &minorText, std::string &majorText)> display;
};

class Menu {
  public: 
  // virtual bool isIdle() = 0;
  //TODO make virtual 
  bool process(MenuConfig &config) {
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

  void process(MenuConfig &config);

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

void EnumMenu::process(MenuConfig &config) {
  if (this->childProcessing != NULL) {
    Serial.println("Child PRocessing.");
    bool isDone = childProcessing->process(config);
    if (isDone)
      childProcessing = NULL;
  } else {
    //TODO debounce
    Serial.println("Menu Processing.");
    int up = digitalRead(config.buttonUpPin);
    int enter = digitalRead(config.buttonEnterPin);
    int down = digitalRead(config.buttonDownPin);

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

    //TODO if not idle
    std::string minorText = getLabel();
    std::string majorText = items[index].getLabel().c_str();
    config.display(minorText, majorText);
    //TODO else if idle?
  }
}

#endif
