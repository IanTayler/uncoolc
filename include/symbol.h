#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string>
#include <unordered_map>
#include <vector>

const int _EMPTY_SYMBOL_ID = -1;

class Symbol {
public:
  int id;

  Symbol();
  Symbol(int i);

  bool is_empty();
  bool operator==(const Symbol &) const;
};

class SymbolTable {
private:
  std::vector<std::string> strings;
  std::unordered_map<std::string, int> id_map;

public:
  Symbol symbol_from(std::string);
  const std::string &get_string(Symbol) const;
};

#endif
