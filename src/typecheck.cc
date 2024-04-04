#include "error.h"
#include "semantic.h"
#include <format>
#include <unordered_set>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

bool ExpressionNode::typecheck(TypeContext &context) {
  fatal("INTERNAL: Calling typecheck on an abstract ExpressionNode is "
        "not permitted",
        start_token);
  return false; // fool the linters
}

bool AttributeNode::typecheck(TypeContext &context) {
  bool check = typecheck_inheritance(context);

  if (!initializer.has_value()) {
    return check;
  };

  check = initializer.value()->typecheck(context) && check;
  if (!initializer.value()->static_type.has_value()) {
    fatal("INTERNAL: Expression static_type is not set after calling "
          "typecheck",
          initializer.value()->start_token);
    return false; // fool the linters
  }

  if (!context.match(initializer.value()->static_type.value(), declared_type)) {
    error("Initializer type does not match declared type", start_token);
    check = false;
  }

  return check;
}

bool ParameterNode::typecheck(TypeContext &context) {
  warning("INTERNAL: Unnecessary call to typecheck for ParameterNode",
          start_token);
  return true;
}

bool MethodNode::typecheck(TypeContext &context) {
  // Check we have not already defined this method in a superclass with a
  // different signature
  bool check = typecheck_inheritance(context);

  context.scopes.enter();

  for (const auto &param : parameters) {
    context.scopes.assign(param->object_id, param->declared_type);
  }

  check = body->typecheck(context) && check;

  context.scopes.exit();

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

  return check;
}

bool ClassNode::typecheck(TypeContext &context) {
  bool check = true;

  context.scopes.enter();

  context.assign_attributes(superclass);

  for (const auto &attribute : attributes) {
    check = attribute->typecheck(context) && check;
    context.scopes.assign(attribute->object_id, attribute->declared_type);
  }

  for (const auto &method : methods) {
    check = method->typecheck(context) && check;
  }

  context.scopes.exit();

  return check;
}

bool ModuleNode::typecheck(TypeContext &context) {
  bool check = true;
  for (const auto &class_node : classes) {
    TypeContext class_context = context;
    class_context.current_class = class_node->name;
    check = class_node->typecheck(class_context) && check;
  }

  return check;
}

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

bool BuiltinNode::typecheck(TypeContext &context) {
  fatal(
      std::format(
          "INTERNAL: Calling typecheck on BuiltinNode ({}.{}) is not permitted",
          context.symbols.get_string(class_name),
          context.symbols.get_string(method_name)),
      start_token);
  return false; // fool the linters
}

bool LiteralNode::typecheck(TypeContext &context) {
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
                      to_string(start_token.type())),
          start_token);
    return false;
  }
  return true;
}

bool VariableNode::typecheck(TypeContext &context) {
  Symbol variable_type = context.scopes.get(name);

  if (variable_type.is_empty()) {
    fatal(std::format("Undefined variable {}. Cannot set type",
                      context.symbols.get_string(name)),
          start_token);
    return false; // fool linter
  }

  static_type = variable_type;
  return true;
}

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

bool UnaryOpNode::typecheck(TypeContext &context) {
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
                      to_string(start_token.type())),
          start_token);
    return false;
  }

  error(std::format("Unexpected type {} for child of UnaryOpNode with op {}",
                    context.symbols.get_string(child->static_type.value()),
                    context.symbols.get_string(op)),
        start_token);
  return false;
}

bool BinaryOpNode::typecheck(TypeContext &context) {
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

  } else if (op == symbols.lt_op || op == symbols.leq_op) {
    static_type = symbols.bool_type;

    if (left->static_type.value() == symbols.int_type &&
        right->static_type.value() == symbols.int_type)
      return true;

  } else if (op == symbols.eq_op) {
    static_type = symbols.bool_type;

    Symbol left_type = left->static_type.value();
    Symbol right_type = right->static_type.value();

    bool left_type_conforms =
        (left_type == symbols.bool_type || left_type == symbols.int_type ||
         left_type == symbols.string_type);

    bool right_type_conforms =
        (right_type == symbols.bool_type || right_type == symbols.int_type ||
         right_type == symbols.string_type);

    if (left_type_conforms && right_type_conforms)
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

bool NewNode::typecheck(TypeContext &context) {
  if (created_type == context.symbols.self_type)
    static_type = context.current_class;
  else
    static_type = created_type;
  return true;
}

bool AssignNode::typecheck(TypeContext &context) {

  bool check = expression->typecheck(context);
  if (!expression->static_type.has_value())
    fatal("INTERNAL: expression in assignment has unset type after checking",
          start_token);

  static_type = expression->static_type.value();

  Symbol variable_type = context.scopes.get(variable);

  if (variable_type.is_empty()) {
    error(std::format("Undefined variable {}",
                      context.symbols.get_string(variable)),
          start_token);
    check = false;
  }

  return check;
}

bool DispatchNode::typecheck(TypeContext &context) {
  const auto &symbols = context.symbols;

  Symbol target_type;
  if (target) {
    target->typecheck(context);

    if (!target->static_type.has_value())
      fatal("INTERNAL: target in dispatch has unset type after checking",
            start_token);

    target_type = target->static_type.value();
  } else {
    target_type = symbols.self_type;
  }

  if (dispatch_type.has_value())
    target_type = dispatch_type.value();

  Symbol parsed_target_type =
      target_type == symbols.self_type ? context.current_class : target_type;

  MethodNode *method_ptr = context.tree.get_method(parsed_target_type, method);

  if (!method_ptr) {
    error(std::format("Call to undefined method {}.{}",
                      symbols.get_string(target_type),
                      symbols.get_string(method)),
          start_token);
    return false;
  }

  static_type = method_ptr->return_type;

  if (method_ptr->parameters.size() != arguments.size()) {
    error(std::format("Wrong number of arguments in dispatch to {}.{}: "
                      "expected {} but got {}",
                      symbols.get_string(target_type),
                      symbols.get_string(method), method_ptr->parameters.size(),
                      arguments.size()),
          start_token);

    return false;
  }

  bool check = true;

  for (int i = 0; i < arguments.size(); i++) {
    auto &arg = arguments[i];
    auto &param = method_ptr->parameters[i];

    check = arg->typecheck(context) && check;

    if (!arg->static_type.has_value())
      fatal("INTERNAL: argument in dispatch has unset type after checking",
            arg->start_token);

    if (!context.match(arg->static_type.value(), param->declared_type)) {
      error(std::format("Argument type {} does not match parameter declared "
                        "type {} in method {}.{}",
                        symbols.get_string(arg->static_type.value()),
                        symbols.get_string(param->declared_type),
                        symbols.get_string(target_type),
                        symbols.get_string(method)),
            arg->start_token);
      check = false;
    }
  }

  return check;
}

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

bool BlockNode::typecheck(TypeContext &context) {
  Symbol last_type;
  bool check = true;

  for (const auto &expr : expressions) {
    check = expr->typecheck(context) && check;
    if (expr->static_type.has_value())
      last_type = expr->static_type.value();
    else
      fatal("INTERNAL: expression in block lacking type after checking",
            expr->start_token);
  }

  static_type = last_type;
  return check;
}

bool IfNode::typecheck(TypeContext &context) {
  condition_expr->typecheck(context);
  if (!condition_expr->static_type.has_value())
    fatal("INTERNAL: condition_expr in IfNode has unset type after checking",
          start_token);

  then_expr->typecheck(context);
  if (!then_expr->static_type.has_value())
    fatal("INTERNAL: then_expr in IfNode has unset type after checking",
          start_token);

  else_expr->typecheck(context);
  if (!else_expr->static_type.has_value())
    fatal("INTERNAL: else_expr in IfNode has unset type after checking",
          start_token);

  if (condition_expr->static_type.value() != context.symbols.bool_type) {
    error(std::format(
              "Unexpected type {} in condition for an if statement. "
              "Conditions should evaluate to Bool",
              context.symbols.get_string(condition_expr->static_type.value())),
          condition_expr->start_token);
    return false;
  }

  Symbol type_then = then_expr->static_type.value();
  Symbol type_else = else_expr->static_type.value();

  std::optional<ClassInfo> common_class =
      context.tree.common_ancestor(type_then, type_else);

  if (!common_class.has_value())
    fatal(std::format("INTERNAL: failed to get common class for {} and {}: "
                      "then and else clausses of an if statement respectively",
                      context.symbols.get_string(type_then),
                      context.symbols.get_string(type_else)),
          start_token);

  static_type = common_class.value().name();
  return true;
}

bool WhileNode::typecheck(TypeContext &context) {
  static_type = context.symbols.object_type;

  condition_expr->typecheck(context);
  if (!condition_expr->static_type.has_value())
    fatal("INTERNAL: condition_expr in IfNode has unset type after checking",
          start_token);

  body_expr->typecheck(context);
  if (!body_expr->static_type.has_value())
    fatal("INTERNAL: body_expr in IfNode has unset type after checking",
          start_token);

  if (condition_expr->static_type.value() != context.symbols.bool_type) {
    error(std::format(
              "Unexpected type {} in condition for a while statement. "
              "Conditions should evaluate to Bool",
              context.symbols.get_string(condition_expr->static_type.value())),
          condition_expr->start_token);
    return false;
  }
  return true;
}

bool LetNode::typecheck(TypeContext &context) {
  bool check = true;

  for (const auto &declaration : declarations) {

    if (declaration->initializer.has_value()) {
      auto &initializer = declaration->initializer.value();

      check = initializer->typecheck(context) && check;

      if (!initializer->static_type.has_value())
        fatal("INTERNAL: Initializer in LetNode has unset type after checking",
              initializer->start_token);

      if (!context.match(initializer->static_type.value(),
                         declaration->declared_type)) {
        error(std::format(
                  "Unexpected type of initializer in let statement. {} does "
                  "not match {}",
                  context.symbols.get_string(initializer->static_type.value()),
                  context.symbols.get_string(declaration->declared_type)),
              initializer->start_token);
        check = false;
      }
    }

    context.scopes.assign(declaration->object_id, declaration->declared_type);
  }

  check = body_expr->typecheck(context) && check;

  if (!body_expr->static_type.has_value())
    fatal("INTERNAL: Body in LetNode has unset type after checking",
          body_expr->start_token);

  static_type = body_expr->static_type;

  return check;
}

bool CaseBranchNode::typecheck(TypeContext &context) {
  context.scopes.enter();
  context.scopes.assign(object_id, declared_type);

  bool check = body_expr->typecheck(context);
  context.scopes.exit();

  if (!body_expr->static_type.has_value())
    fatal("INTERNAL: Case branch body static_type is not set after calling "
          "typecheck",
          start_token);

  static_type = body_expr->static_type.value();
  return check;
}

bool CaseNode::typecheck(TypeContext &context) {
  bool check = true;

  std::unordered_set<int> seen_types;
  Symbol common_type;

  eval_expr->typecheck(context);

  for (const auto &branch : branches) {
    if (seen_types.count(branch->declared_type.id) > 0) {
      error(std::format("Repeated type {} in case statement. Each type should "
                        "only be in one branch",
                        context.symbols.get_string(branch->declared_type)),
            branch->start_token);
      check = false;
    }

    seen_types.insert(branch->declared_type.id);

    check = branch->typecheck(context) && check;

    if (!branch->static_type.has_value())
      fatal("INTERNAL: Case branch static_type is not set after calling "
            "typecheck",
            branch->start_token);

    if (common_type.is_empty()) {
      common_type = branch->static_type.value();

    } else {
      std::optional<ClassInfo> opt_common_type = context.tree.common_ancestor(
          branch->static_type.value(), common_type);

      if (!opt_common_type.has_value())
        fatal("INTERNAL: failed to find ancestor for branch cases after "
              "hierarchy has been checked ",
              start_token);

      common_type = opt_common_type.value().name();
    }
  }

  static_type = common_type;

  return check;
}

/***********************
 *                     *
 * Inheritance Checks  *
 *                     *
 **********************/

bool MethodNode::typecheck_inheritance(const TypeContext &context) const {
  std::optional<ClassInfo> cls = context.tree.get(context.current_class);
  if (!cls.has_value())
    fatal(
        std::format("INTERNAL: clould not find class marked as current_class {}"
                    "in class tree inside MethodNode",
                    context.symbols.get_string(context.current_class)),
        start_token);

  const SymbolTable &symbols = context.symbols;

  bool check = true;

  Symbol superclass_name = cls->superclass();
  MethodNode *inherited_method = context.tree.get_method(superclass_name, name);
  if (inherited_method) {
    if (inherited_method->return_type != return_type) {
      error(std::format("Method {}.{} has return type {} but redefines an "
                        "inherited method with return type {}",
                        symbols.get_string(context.current_class),
                        symbols.get_string(name),
                        symbols.get_string(return_type),
                        symbols.get_string(inherited_method->return_type)),
            start_token);
      check = false;
    }

    else if (parameters.size() != inherited_method->parameters.size()) {
      error(std::format("Method {}.{} has {} parameters but redefines an "
                        "inherited method with {} parameters",
                        symbols.get_string(context.current_class),
                        symbols.get_string(name), parameters.size(),
                        inherited_method->parameters.size()),
            start_token);
      check = false;
    } else {
      for (int i = 0; i < parameters.size(); i++) {
        const auto &param = parameters[i];
        const auto &inherited_param = inherited_method->parameters[i];
        if (param->declared_type != inherited_param->declared_type) {
          error(std::format("Method {}.{}'s parameter number {} is declared as "
                            "{} but it redefines an inherited method in which "
                            "that parameter is declared as {} ",
                            symbols.get_string(context.current_class),
                            symbols.get_string(name), i,
                            symbols.get_string(param->declared_type),
                            symbols.get_string(inherited_param->declared_type)),
                start_token);
          check = false;
        }
      }
    }
  }

  return check;
}

bool AttributeNode::typecheck_inheritance(const TypeContext &context) const {
  std::optional<ClassInfo> cls = context.tree.get(context.current_class);
  if (!cls.has_value())
    fatal(
        std::format("INTERNAL: clould not find class marked as current_class {}"
                    "in class tree inside MethodNode",
                    context.symbols.get_string(context.current_class)),
        start_token);

  const SymbolTable &symbols = context.symbols;

  Symbol superclass_name = cls->superclass();
  AttributeNode *inherited_attribute =
      context.tree.get_attribute(superclass_name, object_id);

  if (inherited_attribute &&
      inherited_attribute->declared_type != declared_type) {
    error(std::format("Attribute {}.{} is declared type {} but inherits from a "
                      "class that declared it as {}",
                      symbols.get_string(cls->name()),
                      symbols.get_string(object_id),
                      symbols.get_string(declared_type),
                      symbols.get_string(inherited_attribute->declared_type)),
          start_token);
    return false;
  }
  return true;
}
