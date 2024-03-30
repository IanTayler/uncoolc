#ifndef _CONSTANT_EVAL_H
#define _CONSTANT_EVAL_H

#include "symbol.h"

int int_eval(Symbol, SymbolTable &);
bool bool_eval(Symbol, SymbolTable &);
Symbol string_eval(Symbol, SymbolTable &);

#endif // !_CONSTANT_EVAL_H
