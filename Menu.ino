#ifndef _menu_ino
#define _menu_ino

#include <string>
#include <vector>

class Menu {
  public: 
  // virtual bool isIdle() = 0;
  // virtual bool process();

  protected:
  Menu(std::string label): label(label) {}

  private:
  std::string label;

};

class EnumMenu: public Menu {
  public: 
  EnumMenu(std::string label, std::vector<Menu> items) : Menu(label) {
    this->items.reserve(items.size());
    for (const Menu item: items)
      items.push_back(item);
  }

  private:
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

#endif
