
#include "symbol.h"
#include <string>
#include <unordered_map>
#include <vector>

/**********************
 *                    *
 *        Symbol      *
 *                    *
 *********************/

Symbol::Symbol() : id(_EMPTY_SYMBOL_ID){};

Symbol::Symbol(int i) : id(i){};

bool Symbol::operator==(const Symbol &other) const { return other.id == id; }

bool Symbol::is_empty() { return id == _EMPTY_SYMBOL_ID; }

/**********************
 *                    *
 *     SymbolTable    *
 *                    *
 *********************/

Symbol SymbolTable::symbol_from(std::string str) {
  if (id_map.count(str) > 0) {
    return Symbol{id_map.at(str)};
  }

  int pos = strings.size();

  strings.push_back(str);
  id_map[str] = pos;

  return Symbol{pos};
}

const std::string _EMPTY_STRING = "";

const std::string &SymbolTable::get_string(Symbol symbol) const {
  if (symbol.is_empty()) {
    return _EMPTY_STRING;
  }
  return strings[symbol.id];
}
