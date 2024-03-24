#include <iostream>

class Printer {
private:
  int current_depth;
  unsigned int indent;
  std::ostream *out;

public:
  Printer() : Printer(2, &std::cout){};

  Printer(unsigned int i, std::ostream *o)
      : current_depth(0), indent(i), out(o){};

  void print(std::string str);

  void enter();

  void exit();
};
