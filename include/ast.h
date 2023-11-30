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
  AstNode(Token st) : start_token(st) {}
  Token start_token;

  virtual void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols);
  // TODO(IT): will need to add typecheck and generate_ir as virtual methods
};

class ExpressionNode : public AstNode {
public:
  ExpressionNode(Token st) : AstNode(st) {}
  std::optional<Symbol> static_type;

  virtual void print(AstPrinter printer,
                     std::shared_ptr<SymbolTable> symbols) override;
};

typedef std::unique_ptr<ExpressionNode> ExpressionPtr;

class AttributeNode : public AstNode {
public:
  AttributeNode(Symbol v, Symbol ty, Token st)
      : object_id(v), declared_type(ty), AstNode(st) {}

  AttributeNode(Symbol v, Symbol ty, ExpressionPtr ex, Token st)
      : object_id(v), declared_type(ty), initializer(std::move(ex)),
        AstNode(st) {}

  Symbol object_id;
  Symbol declared_type;
  std::optional<ExpressionPtr> initializer;

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class ParameterNode : public AstNode {
public:
  ParameterNode(Symbol o, Symbol dt, Token st)
      : object_id(o), declared_type(dt), AstNode(st) {}

  Symbol object_id;
  Symbol declared_type;

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class MethodNode : public AstNode {
public:
  MethodNode(Symbol n, Symbol rt,
             std::vector<std::unique_ptr<ParameterNode>> ps, ExpressionPtr b,
             Token st)
      : name(n), return_type(rt), parameters(std::move(ps)), body(std::move(b)),
        AstNode(st) {}

  Symbol name;
  Symbol return_type;

  std::vector<std::unique_ptr<ParameterNode>> parameters;
  ExpressionPtr body;

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class ClassNode : public AstNode {
public:
  ClassNode(Symbol n, Symbol sc, Token st)
      : name(n), superclass(sc), AstNode(st) {}
  Symbol name;
  Symbol superclass;

  std::vector<std::unique_ptr<AttributeNode>> attributes;
  std::vector<std::unique_ptr<MethodNode>> methods;

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class ModuleNode : public AstNode {
public:
  ModuleNode(Token st) : AstNode(st) {}

  std::vector<std::unique_ptr<ClassNode>> classes;

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
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
  LiteralNode(Token t) : value(t.symbol()), ExpressionNode(t) {}

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

class VariableNode : public ExpressionNode {
private:
  Symbol name;

public:
  VariableNode(Token t) : name(t.symbol()), ExpressionNode(t) {}

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;
};

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

public:

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;

private:
class BinaryOpNode : public ExpressionNode {
public:
  BinaryOpNode(Symbol o, Token st)
      : left(nullptr), op(o), right(nullptr), ExpressionNode(st) {}

  BinaryOpNode(ExpressionPtr l, Symbol o, ExpressionPtr r, Token st)
      : left(std::move(l)), op(o), right(std::move(r)), ExpressionNode(st) {}

  void print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) override;

private:
  ExpressionPtr left;
  Symbol op;
  ExpressionPtr right;
};

class IsVoidNode : public ExpressionNode {
private:
  ExpressionPtr operand;

public:
  IsVoidNode(ExpressionPtr o, Token s)
      : operand(std::move(o)), ExpressionNode(s) {}
};

class NewNode : public ExpressionNode {
private:
  Symbol created_type;

public:
  NewNode(Symbol c, Token s) : created_type(c), ExpressionNode(s) {}
};

class AssignNode : public ExpressionNode {
private:
  Symbol variable;
  ExpressionPtr expression;

public:
  AssignNode(Symbol v, ExpressionPtr ex, Token s)
      : variable(v), expression(std::move(ex)), ExpressionNode(s) {}
};

class DispatchNode : public ExpressionNode {
private:
  ExpressionPtr target;
  std::optional<Symbol> dispatch_type;
  std::vector<ExpressionPtr> arguments;

public:
  DispatchNode(ExpressionPtr t, std::optional<Symbol> dt, Token s)
      : target(std::move(t)), dispatch_type(dt), ExpressionNode(s) {}
};

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT): LET, WHILE, IF, CASE, BLOCKS

#endif
