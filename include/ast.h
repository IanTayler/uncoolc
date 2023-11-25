#ifndef _AST_H
#define _AST_H

#include "symbol.h"
#include "token.h"
#include <optional>
#include <vector>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

class AstNode {
public:
  Token start_token;
  Token end_token;
  // TODO(IT): will need to add typecheck and generate_ir as virtual methods
};

class ExpressionNode : public AstNode {
public:
  std::optional<Symbol> static_type;
};

class AttributeNode : public AstNode {
public:
  Symbol variable;
  Symbol declared_type;
  std::optional<ExpressionNode> initializer;
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
  std::vector<ParameterNode> parameters;
};

class ClassNode : public AstNode {
public:
  Symbol name;
  Symbol superclass;

  std::vector<AttributeNode> attributes;
  std::vector<MethodNode> methods;
};

class ModuleNode : AstNode {
public:
  std::vector<ClassNode> classes;
};

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

class LiteralNode : public ExpressionNode {
private:
  Symbol value;
};

class VariableNode : public ExpressionNode {
private:
  Symbol name;
};

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

class BinaryOpNode : public ExpressionNode {
private:
  enum class Operator {
    ADD,
    SUB,
    DIV,
    MULT,
    LT,
    LEQ,
    EQ,
  };

  bool is_arithmetic() const {
    return op >= Operator::ADD && op <= Operator::MULT;
  };

  bool is_comparison() const {
    return op >= Operator::LT && op <= Operator::EQ;
  };

  ExpressionNode left;
  Operator op;
  ExpressionNode right;
};

class IsVoidNode : public ExpressionNode {
private:
  ExpressionNode operand;
};

class NewNode : public ExpressionNode {
private:
  Symbol created_type;
};

class AssignNode : public ExpressionNode {
private:
  Symbol variable;
  ExpressionNode initializer;
};

class DispatchNode : public ExpressionNode {
private:
  ExpressionNode target;
  std::optional<Symbol> dispatch_type;
  std::vector<ExpressionNode> arguments;
};

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT): LET, WHILE, IF, CASE, BLOCKS

#endif
