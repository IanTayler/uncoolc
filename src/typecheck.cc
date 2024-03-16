#include "error.h"
#include "semantic.h"
#include <format>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

bool ExpressionNode::typecheck(TypeContext context) {
  fatal("INTERNAL: Calling typecheck on an abstract ExpressionNode is "
        "not permitted",
        start_token);
  return false; // fool the linters
}

bool AttributeNode::typecheck(TypeContext context) {
  if (!initializer.has_value()) {
    return true;
  };

  bool init_check = initializer.value()->typecheck(context);
  if (!initializer.value()->static_type.has_value()) {
    fatal("INTERNAL: Expression static_type is not set after calling "
          "typecheck",
          initializer.value()->start_token);
    return false; // fool the linters
  }

  if (!context.match(initializer.value()->static_type.value(), declared_type)) {
    error("Initializer type does not match declared type", start_token);
    return false;
  }

  return init_check;
}

bool ParameterNode::typecheck(TypeContext context) {
  warning("INTERNAL: Unnecessary call to typecheck for ParameterNode",
          start_token);
  return true;
}

bool MethodNode::typecheck(TypeContext context) {
  bool body_check = body->typecheck(context);

  Symbol body_type = body->static_type.value();
  if (!context.match(body->static_type.value(), return_type)) {
    error(std::format("Wrong body type {} in method {}, expected {}",
                      context.symbols.get_string(body_type),
                      context.symbols.get_string(name),
                      context.symbols.get_string(return_type)),
          start_token);
    return false;
  }
  return body_check;
}

bool ClassNode::typecheck(TypeContext context) {
  bool check = true;
  for (const auto &method : methods) {
    check = check && method->typecheck(context);
  }

  for (const auto &attribute : attributes) {
    check = check && attribute->typecheck(context);
  }

  return check;
}

bool ModuleNode::typecheck(TypeContext context) {
  bool check = true;
  for (const auto &class_node : classes) {
    TypeContext class_context = context;
    class_context.current_class = class_node->name;
    check = check && class_node->typecheck(class_context);
  }

  return check;
}

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

bool LiteralNode::typecheck(TypeContext context) {
  switch (start_token.type()) {
  case TokenType::STRING:
    static_type = context.symbols.string_type;
    break;
  case TokenType::NUMBER:
    static_type = context.symbols.int_type;
    break;
  case TokenType::KW_TRUE:
  case TokenType::KW_FALSE:
    static_type = context.symbols.bool_type;
    break;
  default:
    fatal(std::format("LiteralNode has unexpected token type {}",
                      token_type_str(start_token.type())),
          start_token);
    return false;
  }
  return true;
}

bool VariableNode::typecheck(TypeContext context) {
  Symbol variable_type = context.scopes.get(name);

  if (variable_type.is_empty()) {
    error(
        std::format("Undefined variable {}", context.symbols.get_string(name)),
        start_token);
    return false;
  }

  static_type = variable_type;
  return true;
}

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

bool UnaryOpNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool BinaryOpNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool NewNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool AssignNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool DispatchNode::typecheck(TypeContext context) { return true; }

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT) fill in
bool BlockNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool IfNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool WhileNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool LetNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool CaseBranchNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool CaseNode::typecheck(TypeContext context) { return true; }
