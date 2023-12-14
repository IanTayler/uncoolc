
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

SymbolTable::SymbolTable() {
  // No point in having to resize the buffers often when we know almost all
  // files will have quite a large number of different symbols.
  // Reserve a good starting size.
  const int MINIMUM_SYMBOLS_SIZE = 128;
  strings.reserve(MINIMUM_SYMBOLS_SIZE);
  id_map.reserve(MINIMUM_SYMBOLS_SIZE);

  true_const = from("true");
  false_const = from("false");
  self_var = from("self");
  self_type = from("SELF_TYPE");
  object_type = from("Object");
  io_type = from("IO");
  bool_type = from("Bool");
  int_type = from("Int");
  string_type = from("String");
  add_op = from("+");
  sub_op = from("-");
  div_op = from("/");
  mult_op = from("*");
  leq_op = from("<=");
  lt_op = from("<");
  eq_op = from("=");
  assign_op = from("<-");
  neg_op = from("~");
  if_kw = from("if");
  in_kw = from("in");
  fi_kw = from("fi");
  of_kw = from("of");
  let_kw = from("let");
  new_kw = from("new");
  not_kw = from("not");
  case_kw = from("case");
  else_kw = from("else");
  esac_kw = from("esac");
  then_kw = from("then");
  loop_kw = from("loop");
  pool_kw = from("pool");
  while_kw = from("while");
  class_kw = from("class");
  isvoid_kw = from("isvoid");
  inherits_kw = from("inherits");
}

Symbol SymbolTable::from(const std::string &str) {
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
