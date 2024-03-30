#ifndef _HLIR_H
#define _HLIR_H

#include "printer.h"
#include "symbol.h"
#include "token.h"
#include <list>

namespace hlir {

/***********************
 *                     *
 *        Value        *
 *                     *
 **********************/

enum class ValueType {
  SELF,
  VAR,
  TEMP,
  ACC,
  LITERAL,
  TYPE_ID,
};

std::string to_string(ValueType);

class Value {
private:
  Value(ValueType, int);
  Value(ValueType, Symbol);
  Value(ValueType);

public:
  ValueType type;
  union {
    int num;
    Symbol symbol;
  };

  static Value self();
  static Value var(Symbol);
  static Value temp(int);
  static Value acc();
  static Value literal(Symbol);
  static Value type_id(Symbol);
};

std::string to_string(Value, const SymbolTable &);

/***********************
 *                     *
 *          Op         *
 *                     *
 **********************/

enum class Op {
  ADD,
  SUB,
  MULT,
  DIV,
  EQUAL,
  LESS_THAN,
  LESS_EQUAL,
  NEG,
  NOT,
  IS_VOID,
  NEW,
  ADD_ARG,
  CALL,
  BRANCH,
  LABEL,
  MOV,
  TYPE_ID_OF,
  SUPERCLASS,
};

std::string to_string(Op op);

/***********************
 *                     *
 *   BranchCondition   *
 *                     *
 **********************/

enum class BranchCondition {
  ALWAYS,
  TRUE,
  FALSE,
};

std::string to_string(BranchCondition);

/***********************
 *                     *
 *       Position      *
 *                     *
 **********************/

class Instruction;

typedef std::list<std::unique_ptr<Instruction>> InstructionList;

class Position {
public:
  int label_idx;
  InstructionList::iterator label;

  Position(int, InstructionList::iterator);
};

std::string to_string(Position);

/***********************
 *                     *
 *    Instructions     *
 *                     *
 **********************/

class Instruction {
public:
  Op op;
  Token token;

  Instruction(Op o, Token t);

  virtual void print(Printer, const SymbolTable &) const;
};

class Unary : public Instruction {
private:
  Value dest;
  Value arg;

public:
  Unary(Op o, Value, Value, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class New : public Instruction {
private:
  Value dest;
  Symbol type;

public:
  New(Op o, Value, Symbol, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class Binary : public Instruction {
private:
  Value dest;
  Value left;
  Value right;

public:
  Binary(Op o, Value, Value, Value, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class AddArg : public Instruction {
private:
  Value arg;

public:
  AddArg(Value, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class Call : public Instruction {
private:
  Value target;
  Symbol method_name;

public:
  Call(Value, Symbol, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class Branch : public Instruction {
private:
  Value value;
  BranchCondition condition;
  Position target;

public:
  Branch(BranchCondition, Value, Position, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class Label : public Instruction {
private:
  int idx;
  Symbol name;

public:
  Label(int, Symbol, Token t);

  void print(Printer, const SymbolTable &) const override;
};

class Mov : public Instruction {
private:
  Value dest;
  Value src;

public:
  Mov(Value, Value, Token t);

  void print(Printer, const SymbolTable &) const override;
};

/***********************
 *                     *
 *       Context       *
 *                     *
 **********************/

class Context {
private:
  int temporaries;
  int labels;

public:
  const SymbolTable &symbols;

  Context(const SymbolTable &);

  Value create_temporary();
  int create_label_idx();
};

/***********************
 *                     *
 *        Method       *
 *                     *
 **********************/

class Method {
public:
  Symbol name;
  InstructionList instructions;

  Method(Symbol);

  void print(Printer, const SymbolTable &) const;
};

/***********************
 *                     *
 *        Class        *
 *                     *
 **********************/

class Class {
public:
  Symbol name;
  InstructionList initializer;
  std::unordered_map<int, Method> methods;

  Class(Symbol);

  void print(Printer, const SymbolTable &) const;
};

/***********************
 *                     *
 *      Universe       *
 *                     *
 **********************/

class Universe {
public:
  std::unordered_map<int, Class> classes;

  void print(Printer, const SymbolTable &) const;
};

} // namespace hlir

#endif
