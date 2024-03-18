#include "error.h"
#include "semantic.h"
#include <format>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

bool ExpressionNode::typecheck(const TypeContext &context) {
  fatal("INTERNAL: Calling typecheck on an abstract ExpressionNode is "
        "not permitted",
        start_token);
  return false; // fool the linters
}

bool AttributeNode::typecheck(const TypeContext &context) {
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

bool ParameterNode::typecheck(const TypeContext &context) {
  warning("INTERNAL: Unnecessary call to typecheck for ParameterNode",
          start_token);
  return true;
}

bool MethodNode::typecheck(const TypeContext &context) {
  bool body_check = body->typecheck(context);

  if (!body->static_type.has_value())
    fatal(std::format(
              "INTERNAL: method body of {} has unset type after typechecking",
              context.symbols.get_string(name)),
          start_token);

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

bool ClassNode::typecheck(const TypeContext &context) {
  bool check = true;
  for (const auto &method : methods) {
    check = check && method->typecheck(context);
  }

  for (const auto &attribute : attributes) {
    check = check && attribute->typecheck(context);
  }

  return check;
}

bool ModuleNode::typecheck(const TypeContext &context) {
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

bool BuiltinNode::typecheck(const TypeContext &context) {
  fatal(
      std::format(
          "INTERNAL: Calling typecheck on BuiltinNode ({}.{}) is not permitted",
          context.symbols.get_string(class_name),
          context.symbols.get_string(method_name)),
      start_token);
  return false; // fool the linters
}

bool LiteralNode::typecheck(const TypeContext &context) {
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

bool VariableNode::typecheck(const TypeContext &context) {
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

bool UnaryOpNode::typecheck(const TypeContext &context) {
  child->typecheck(context);
  if (!child->static_type.has_value())
    fatal("INTERNAL: child of UnaryOpNode has unset type after checking",
          start_token);

  switch (start_token.type()) {
  case TokenType::NEG_OP:
    static_type = context.symbols.int_type;
    if (child->static_type.value() == context.symbols.int_type)
      return true;
    break;
  case TokenType::KW_NOT:
    static_type = context.symbols.bool_type;
    if (child->static_type.value() == context.symbols.bool_type)
      return true;
    break;
  case TokenType::KW_ISVOID:
    static_type = context.symbols.bool_type;
    // Any type is good for isvoid
    return true;
  default:
    fatal(std::format("INTERNAL: UnaryOpNode with unknown token type {}",
                      token_type_str(start_token.type())),
          start_token);
    return false;
  }

  error(std::format("Unexpected type {} for child of UnaryOpNode with op {}",
                    context.symbols.get_string(child->static_type.value()),
                    context.symbols.get_string(op)),
        start_token);
  return false;
}

bool BinaryOpNode::typecheck(const TypeContext &context) {
  const SymbolTable &symbols = context.symbols;

  left->typecheck(context);
  if (!left->static_type.has_value())
    fatal("INTERNAL: left child of BinaryOpNode has type unset after checking",
          start_token);

  right->typecheck(context);
  if (!right->static_type.has_value())
    fatal("INTERNAL: right child of BinaryOpNode has type unset after checking",
          start_token);

  if (op == symbols.add_op || op == symbols.sub_op || op == symbols.mult_op ||
      op == symbols.div_op) {
    static_type = symbols.int_type;

    if (left->static_type.value() == symbols.int_type &&
        right->static_type.value() == symbols.int_type)
      return true;
  } else if (op == symbols.eq_op || op == symbols.lt_op ||
             op == symbols.leq_op) {
    static_type = symbols.bool_type;

    if (left->static_type.value() == symbols.int_type &&
        right->static_type.value() == symbols.int_type)
      return true;
  } else
    fatal(std::format("INTERNAL: Unexpected op {} in BinaryOpNode",
                      symbols.get_string(op)),
          start_token);

  error(std::format("Unexpected types {} and {} for sides of BinaryOpNode {}",
                    symbols.get_string(left->static_type.value()),
                    symbols.get_string(right->static_type.value()),
                    symbols.get_string(op)),
        start_token);
  return false;
}
// TODO(IT) fill in
bool NewNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool AssignNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool DispatchNode::typecheck(const TypeContext &context) { return true; }

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT) fill in
bool BlockNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool IfNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool WhileNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool LetNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool CaseBranchNode::typecheck(const TypeContext &context) { return true; }
// TODO(IT) fill in
bool CaseNode::typecheck(const TypeContext &context) { return true; }
