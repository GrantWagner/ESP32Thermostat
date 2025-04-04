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
  std::function<void()> displayIdle;
};

bool lastUp = false;
bool lastEnter = false;
bool lastDown = false;
int lastInputMillis = 0;

class Menu {
  public: 
  virtual bool process(MenuConfig &config) = 0;
  
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

class ExitItem: public Menu {
  public:
  ExitItem(): Menu("Exit") {}

  bool process(MenuConfig &config) {
    lastInputMillis = 0;
    return true;
  }
} exitItem;

class EnumMenu: public Menu {
  public: 
  EnumMenu(std::string inputLabel, std::vector<Menu*> inputItems) : Menu(inputLabel) {
    items.reserve(inputItems.size() + 1);
    for (Menu *item: inputItems)
      items.push_back(item);
    items.push_back(&exitItem);
  }

  bool process(MenuConfig &config);

  private:
  Menu *childProcessing = NULL;
  int index = 0;
  std::vector<Menu*> items;
};

class IntegerSelectMenu: public Menu {
  public:
  IntegerSelectMenu(std::string label, int &value, int min, int max): Menu(label), value(value) {
    this->min = min;
    this->max = max;
  }

  bool process(MenuConfig &config);

  private:
  int &value;
  int min;
  int max;
};

bool debounce(int pin, bool &lastValue) {
    bool valueRaw = digitalRead(pin) == HIGH;
    bool value = valueRaw && !lastValue;
    lastValue = valueRaw;

    return value;
}

bool EnumMenu::process(MenuConfig &config) {
  if (this->childProcessing != NULL) {
    Serial.println("Child PRocessing.");
    bool isDone = childProcessing->process(config);
    if (isDone)
      childProcessing = NULL;
    return false;
  }

  Serial.println("Menu Processing.");
  bool up = debounce(config.buttonUpPin, lastUp);
  bool enter = debounce(config.buttonEnterPin, lastEnter);
  bool down = debounce(config.buttonDownPin, lastDown);
  Serial.printf("Buttons, up: %s, down: %s, enter: %s.\n", 
    up ? "HIGH" : "LOW",
    down ? "HIGH" : "LOW",
    enter ? "HIGH" : "LOW");

  unsigned long currentMillis = millis();
  if (up || enter || down) {
    lastInputMillis = currentMillis;
  } else if (currentMillis < lastInputMillis 
    || (currentMillis - lastInputMillis) > 15000) {

    config.displayIdle();

    return false;
  }  

  if (down) {
    Serial.println("Processing Down");
    index++;
    if (index >= items.size())
      index = items.size()-1;
  } else if (up) {
    Serial.println("Processing UP");
    index--;
    if (index < 0)
      index = 0;
  } else if (enter) {
    Serial.println("Processing Enter");
    childProcessing = items[index];
  }

  std::string minorText = getLabel();
  std::string majorText = items[index]->getLabel();
  config.display(minorText, majorText);
  return false;
}

bool IntegerSelectMenu::process(MenuConfig &config) {
  Serial.println("Integer Select Processing.");
  if (value < min)
    value = min;
  if (value > max)
    value = max;

  bool up = debounce(config.buttonUpPin, lastUp);
  bool enter = debounce(config.buttonEnterPin, lastEnter);
  bool down = debounce(config.buttonDownPin, lastDown);
  Serial.printf("Buttons, up: %s, down: %s, enter: %s.\n", 
    up ? "HIGH" : "LOW",
    down ? "HIGH" : "LOW",
    enter ? "HIGH" : "LOW");

  if (down) {
    Serial.println("Processing Down");
    if (value > min)
      value = value - 1;
  } else if (up) {
    Serial.println("Processing UP");
    if (value < max)
      value = value + 1;
  } else if (enter) {
    Serial.println("Processing Enter");
    return true;
  }

  std::string minorText = getLabel();
  char buffer[20] = {0};
  sprintf(buffer, "Value: %d", value);
  std::string majorText = std::string(buffer);
  config.display(minorText, majorText);
  return false;
}


#endif
