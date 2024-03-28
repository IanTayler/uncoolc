#ifndef _HLIR_H
#define _HLIR_H

#include "printer.h"
#include "symbol.h"
#include <list>

namespace hlir {

/***********************
 *                     *
 *        Value        *
 *                     *
 **********************/

enum class ValueType {
  Self,
  Var,
  Temp,
  Acc,
  Literal,
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
    int id;
    Symbol name;
    Symbol value;
  };

  static Value self();
  static Value var(Symbol);
  static Value temp(int);
  static Value acc();
  static Value literal(Symbol);
};

std::string to_string(Value, const SymbolTable &);

/***********************
 *                     *
 *          Op         *
 *                     *
 **********************/

enum class Op {
  Add,
  Sub,
  Mult,
  Div,
  Neg,
  Not,
  IsVoid,
  New,
  Comparison,
  AddArg,
  Call,
  Branch,
  Label,
  Mov,
};

std::string to_string(Op op);

/***********************
 *                     *
 *   BranchCondition   *
 *                     *
 **********************/

enum class BranchCondition {
  Equal,
  NotEqual,
  LessThan,
  LessThanEqual,
  GreaterThan,
  GreaterThanEqual,
};

std::string to_string(BranchCondition);

/***********************
 *                     *
 *        Label        *
 *                     *
 **********************/

class Instruction;

typedef std::list<std::unique_ptr<Instruction>> InstructionList;

class Position {
public:
  int idx;
  InstructionList::iterator it;
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

  Instruction(Op o);

  virtual void print(Printer, const SymbolTable &) const;
};

class Unary : public Instruction {
private:
  Value dest;
  Value child;

public:
  Unary(Op o, Value, Value);

  void print(Printer, const SymbolTable &) const override;
};

class New : public Instruction {
private:
  Value dest;
  Symbol type;

public:
  New(Op o, Value, Symbol);

  void print(Printer, const SymbolTable &) const override;
};

class Binary : public Instruction {
private:
  Value dest;
  Value left;
  Value right;

public:
  Binary(Op o, Value, Value, Value);

  void print(Printer, const SymbolTable &) const override;
};

class Comparison : public Instruction {
private:
  Value left;
  Value right;

public:
  Comparison(Op o, Value, Value);

  void print(Printer, const SymbolTable &) const override;
};

class AddArg : public Instruction {
private:
  Value arg;

public:
  AddArg(Value);

  void print(Printer, const SymbolTable &) const override;
};

class Call : public Instruction {
private:
  Value target;
  Symbol method_name;

public:
  Call(Value, Symbol);

  void print(Printer, const SymbolTable &) const override;
};

class Branch : public Instruction {
private:
  BranchCondition condition;
  Position target;

public:
  Branch(BranchCondition, Position);

  void print(Printer, const SymbolTable &) const override;
};

class Label : public Instruction {
private:
  int idx;

public:
  Label(int);

  void print(Printer, const SymbolTable &) const override;
};

class Mov : public Instruction {
private:
  Value dest;
  Value src;

public:
  Mov(Value, Value);

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
