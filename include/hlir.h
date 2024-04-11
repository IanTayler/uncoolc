#ifndef _HLIR_H
#define _HLIR_H

#include "printer.h"
#include "runtime.h"
#include "symbol.h"
#include "token.h"
#include <list>

namespace hlir {

/***********************
 *                     *
 *        Value        *
 *                     *
 **********************/

enum class ValueKind {
  SELF,
  ATTRIBUTE,
  LOCAL,
  TEMP,
  ACC,
  CONSTANT,
  EMPTY,
};

std::string to_string(ValueKind);

class Value {
private:
  Value(ValueKind, int, Symbol);
  Value(ValueKind, Symbol, Symbol);
  Value(ValueKind, Symbol);

public:
  ValueKind kind;
  Symbol static_type;

  // Which of these fields is used depends on the kind and static_type
  // - SELF ignores these values
  // - LOCAL, ATTR use symbol for the variable name.
  // - TEMP uses num as the temp id
  // - ACC ignores these values
  // - CONSTANT uses the field matching their type, with strings using Symbol
  union {
    int num;
    Symbol symbol;
    bool boolean;
  };

  static Value self(Symbol);
  static Value attr(Symbol, Symbol);
  static Value local(Symbol, Symbol);
  static Value temp(int, Symbol);
  static Value acc(Symbol);
  static Value constant(int, Symbol);
  static Value constant(bool, Symbol);
  static Value constant(Symbol, Symbol);
  static Value empty();

  bool is_empty() const;
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
  CALL,
  BRANCH,
  LABEL,
  MOV,
  ERROR,
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

  Position(int);
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

  bool has_dest();
  virtual Value &get_dest();

  int num_args();
  virtual Value &get_arg1();
  virtual Value &get_arg2();

  virtual void print(Printer, const SymbolTable &) const;
};

class Unary : public Instruction {
private:
  Value dest;
  Value arg;

public:
  Unary(Op o, Value, Value, Token t);

  Value &get_dest() override;
  Value &get_arg1() override;

  void print(Printer, const SymbolTable &) const override;
};

class New : public Instruction {
private:
  Value dest;
  Symbol type;

public:
  New(Op o, Value, Symbol, Token t);

  Value &get_dest() override;

  void print(Printer, const SymbolTable &) const override;
};

class Binary : public Instruction {
private:
  Value dest;
  Value left;
  Value right;

public:
  Binary(Op o, Value, Value, Value, Token t);

  Value &get_dest() override;
  Value &get_arg1() override;
  Value &get_arg2() override;

  void print(Printer, const SymbolTable &) const override;
};

class Call : public Instruction {
private:
  Value dest;
  Value target;
  Symbol method_name;
  std::vector<Value> args;

public:
  Call(Value, Value, Symbol, Token t);

  Value &get_dest() override;
  Value &get_arg1() override;

  void add_arg(const Value &);

  void print(Printer, const SymbolTable &) const override;
};

class Branch : public Instruction {
private:
  Value value;
  BranchCondition condition;
  Position target;

public:
  Branch(BranchCondition, Value, Position, Token t);

  Value &get_arg1() override;

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

  Value &get_dest() override;
  Value &get_arg1() override;

  void print(Printer, const SymbolTable &) const override;
};

class Error : public Instruction {
public:
  BranchCondition condition;
  Value check;
  runtime::Error error;

  Error(BranchCondition, Value, runtime::Error, Token t);

  Value &get_arg1() override;

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
  SymbolTable &symbols;

  Context(SymbolTable &);

  Value create_temporary(Symbol);
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
