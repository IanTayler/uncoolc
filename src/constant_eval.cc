#include "constant_eval.h"
#include "error.h"
#include <format>
#include <string>

int int_eval(Symbol literal, SymbolTable &symbols) {
  return std::stoi(symbols.get_string(literal));
}
bool bool_eval(Symbol literal, SymbolTable &symbols) {
  if (literal == symbols.true_const)
    return true;
  else if (literal == symbols.false_const)
    return false;
  else
    fatal(std::format("INTERNAL: tried to bool_eval a non-bool value {}",
                      symbols.get_string(literal)));
  return false; // fool linter
}

Symbol string_eval(Symbol literal, SymbolTable &symbols) {
  std::string full_string = symbols.get_string(literal);
  if (full_string.front() == '"' && full_string.back() == '"')
    return symbols.from(full_string.substr(1, full_string.size() - 2));
  else
    fatal(std::format("INTERNAL: tried to string_eval a non-string value {}",
                      symbols.get_string(literal)));
  return Symbol{}; // fool linter
}
