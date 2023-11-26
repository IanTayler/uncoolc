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
  SymbolTable();
  Symbol from(std::string);
  const std::string &get_string(Symbol) const;

  Symbol true_const;
  Symbol false_const;
  Symbol self_var;
  Symbol self_type;
  Symbol object_type;
  Symbol io_type;
  Symbol bool_type;
  Symbol int_type;
  Symbol string_type;
  Symbol add_op;
  Symbol sub_op;
  Symbol div_op;
  Symbol mult_op;
  Symbol leq_op;
  Symbol lt_op;
  Symbol eq_op;
};

#endif
