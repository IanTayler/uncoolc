#include "printer.h"

void Printer::print(std::string str) { *out << str; }

void Printer::println(std::string str) {
  beginln();
  print(str);
  endln();
}

void Printer::beginln() {
  for (int i = 0; i < current_depth; i++)
    for (int j = 0; j < indent; j++)
      *out << ' ';
}

void Printer::endln() { *out << std::endl; }

void Printer::enter() { current_depth++; }

void Printer::exit() { current_depth--; }
