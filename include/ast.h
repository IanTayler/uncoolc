#ifndef _AST_H
#define _AST_H

#include "symbol.h"
#include "token.h"
#include <iostream>
#include <optional>
#include <vector>

/***********************
 *                     *
 *     PrettyPrint     *
 *                     *
 **********************/

class AstPrinter {
private:
  int current_depth;
  unsigned int indent;
  std::ostream *out;

public:
  AstPrinter() : AstPrinter(2, &std::cout){};

  AstPrinter(unsigned int i, std::ostream *o)
      : current_depth(0), indent(i), out(o){};

  void print(std::string str);

  void enter();

  void exit();
};

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

class AstNode {
public:
  AstNode(Token st, Token end) : start_token(st), end_token(end) {}
  Token start_token;
  Token end_token;

  virtual void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols);
  // TODO(IT): will need to add typecheck and generate_ir as virtual methods
};

class ExpressionNode : public AstNode {
public:
  ExpressionNode(Token st, Token end) : AstNode(st, end) {}
  std::optional<Symbol> static_type;

  virtual void print(AstPrinter printer,
                     std::shared_ptr<SymbolTable> symbols) override;
};

typedef std::unique_ptr<ExpressionNode> ExpressionPtr;

class AttributeNode : public AstNode {
public:
  Symbol variable;
  Symbol declared_type;
  std::optional<ExpressionPtr> initializer;
};

class ParameterNode : public AstNode {
public:
  Symbol variable;
  Symbol declared_type;
};

class MethodNode : public AstNode {
public:
  Symbol name;
  Symbol return_type;
  std::vector<std::unique_ptr<ParameterNode>> parameters;
};

class ClassNode : public AstNode {
public:
  Symbol name;
  Symbol superclass;

  std::vector<std::unique_ptr<AttributeNode>> attributes;
  std::vector<std::unique_ptr<MethodNode>> methods;
};

class ModuleNode : AstNode {
public:
  std::vector<std::unique_ptr<ClassNode>> classes;
};

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

class LiteralNode : public ExpressionNode {
private:
  Symbol value;

public:
  LiteralNode(Token t)
      : value(t.symbol()), ExpressionNode(t, t) {}

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class VariableNode : public ExpressionNode {
private:
  Symbol name;

public:
  VariableNode(Symbol n, Token st, Token end)
      : name(n), ExpressionNode(st, end) {}
};

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

class BinaryOpNode : public ExpressionNode {
public:
  enum class Operator {
    ADD,
    SUB,
    DIV,
    MULT,
    LT,
    LEQ,
    EQ,
  };
  BinaryOpNode(ExpressionPtr l, Operator o, ExpressionPtr r, Token st,
               Token end)
      : left(std::move(l)), op(o), right(std::move(r)),
        ExpressionNode(st, end) {}

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;

private:
  bool is_arithmetic() const {
    return op >= Operator::ADD && op <= Operator::MULT;
  };

  bool is_comparison() const {
    return op >= Operator::LT && op <= Operator::EQ;
  };

  ExpressionPtr left;
  Operator op;
  ExpressionPtr right;
};

class IsVoidNode : public ExpressionNode {
private:
  ExpressionPtr operand;

public:
  IsVoidNode(ExpressionPtr o, Token s, Token e)
      : operand(std::move(o)), ExpressionNode(s, e) {}
};

class NewNode : public ExpressionNode {
private:
  Symbol created_type;

public:
  NewNode(Symbol c, Token s, Token e) : created_type(c), ExpressionNode(s, e) {}
};

class AssignNode : public ExpressionNode {
private:
  Symbol variable;
  ExpressionPtr expression;

public:
  AssignNode(Symbol v, ExpressionPtr ex, Token s, Token e)
      : variable(v), expression(std::move(ex)), ExpressionNode(s, e) {}
};

class DispatchNode : public ExpressionNode {
private:
  ExpressionPtr target;
  std::optional<Symbol> dispatch_type;
  std::vector<ExpressionPtr> arguments;

public:
  DispatchNode(ExpressionPtr t, std::optional<Symbol> dt, Token s, Token e)
      : target(std::move(t)), dispatch_type(dt), ExpressionNode(s, e) {}
};

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT): LET, WHILE, IF, CASE, BLOCKS

#endif
