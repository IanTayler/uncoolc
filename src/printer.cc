#include "printer.h"

void Printer::print(std::string str) {
  for (int i = 0; i < current_depth; i++)
    for (int j = 0; j < indent; j++)
      *out << ' ';
  *out << str << std::endl;
}

void Printer::enter() { current_depth++; }

void Printer::exit() { current_depth--; }
