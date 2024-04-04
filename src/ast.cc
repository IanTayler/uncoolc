#include "ast.h"
#include <format>

/***********************
 *                     *
 *    Node Printers    *
 *                     *
 **********************/

void AstNode::print(Printer printer, const SymbolTable &) {
  printer.println("__NODE_PRINT_UNDEFINED__");
}

void ModuleNode::print(Printer printer, const SymbolTable &symbols) {
  for (auto &class_ : classes) {
    class_->print(printer, symbols);
  }
}

void ClassNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("class {} inherits {}", symbols.get_string(name),
                              symbols.get_string(superclass)));

  printer.enter();
  {
    for (auto &attr : attributes) {
      attr->print(printer, symbols);
    }

    for (auto &method : methods) {
      method->print(printer, symbols);
    }
  }
  printer.exit();
}

void AttributeNode::print(Printer printer, const SymbolTable &symbols) {
  if (initializer) {
    printer.println(std::format("attr {} : {} <-",
                                symbols.get_string(object_id),
                                symbols.get_string(declared_type)));
    printer.enter();
    (*initializer)->print(printer, symbols);
    printer.exit();
  } else {
    printer.println(std::format("attr {} : {}", symbols.get_string(object_id),
                              symbols.get_string(declared_type)));
  }
}

void ParameterNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("param {} : {}", symbols.get_string(object_id),
                              symbols.get_string(declared_type)));
}

void MethodNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("method {} : {}", symbols.get_string(name),
                              symbols.get_string(return_type)));

  printer.enter();
  {
    for (auto &param : parameters) {
      param->print(printer, symbols);
    }
    printer.println("body");
    printer.enter();
    body->print(printer, symbols);
    printer.exit();
  }
  printer.exit();
}

/***********************
 *                     *
 * Expression Printers *
 *                     *
 **********************/

void ExpressionNode::print(Printer printer, const SymbolTable &symbols) {
  printer.print("__EXPRESSION_PRINT_UNDEFINED__");
  print_type(printer, symbols);
}

void ExpressionNode::print_type(Printer printer, const SymbolTable &symbols) {
  printer.enter();
  if (static_type.has_value())
    printer.println(
        std::format("type: {}", symbols.get_string(static_type.value())));
  else
    printer.println("type: __unset__");
  printer.exit();
}

void BuiltinNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("Builtin: {}.{}", symbols.get_string(class_name),
                              symbols.get_string(method_name)));
  print_type(printer, symbols);
}

void LiteralNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("Literal {}", symbols.get_string(value)));
  print_type(printer, symbols);
}

void VariableNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("Variable {}", symbols.get_string(name)));
  print_type(printer, symbols);
}

void NewNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("new {}", symbols.get_string(created_type)));
  print_type(printer, symbols);
}

void UnaryOpNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("UnaryOp {}", symbols.get_string(op)));

  print_type(printer, symbols);

  printer.enter();
  if (child)
    child->print(printer, symbols);
  else
    printer.println("__missing_child__");
  printer.exit();
}

void BinaryOpNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("BinaryOp {}", symbols.get_string(op)));

  print_type(printer, symbols);

  printer.enter();
  {
    if (left)
      left->print(printer, symbols);
    else
      printer.println("__missing_left__");
    if (right)
      right->print(printer, symbols);
    else
      printer.println("__missing_right__");
  }
  printer.exit();
}

void AssignNode::print(Printer printer, const SymbolTable &symbols) {
  if (variable.is_empty())
    printer.println("__missing__variable__ <-");
  else
    printer.println(std::format("{} <-", symbols.get_string(variable)));

  print_type(printer, symbols);

  printer.enter();
  {
    if (expression)
      expression->print(printer, symbols);
    else
      printer.println("__missing_expression__");
  }
  printer.exit();
}

void DispatchNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("Dispatch");

  print_type(printer, symbols);

  printer.enter();
  {
    printer.println("target");

    printer.enter();
    {
      if (target_self)
        printer.println("self");
      else if (target)
        target->print(printer, symbols);
      else
        printer.println("__missing_target__");
    }
    printer.exit();

    if (dispatch_type)
      printer.println(std::format("@{}", symbols.get_string(*dispatch_type)));

    printer.println(std::format("method {}", symbols.get_string(method)));

    printer.println("arguments");
    printer.enter();
    for (auto &arg : arguments) {
      arg->print(printer, symbols);
    }
    printer.exit();
  }
  printer.exit();
}

void BlockNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("Block");

  print_type(printer, symbols);

  printer.enter();
  {
    for (auto &expr : expressions) {
      expr->print(printer, symbols);
    }
  }
  printer.exit();
}

void IfNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("If");

  print_type(printer, symbols);

  printer.enter();
  {
    if (condition_expr) {
      printer.println("Condition");
      printer.enter();
      condition_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.println("__missing_condition__");
    }

    if (then_expr) {
      printer.println("Then");
      printer.enter();
      then_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.println("__missing_then_expr__");
    }

    if (else_expr) {
      printer.println("Else");
      printer.enter();
      else_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.println("__missing_else_expr__");
    }
  }
  printer.exit();
}

void WhileNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("While");

  print_type(printer, symbols);

  printer.enter();
  {
    if (condition_expr) {
      printer.println("Condition");
      printer.enter();
      condition_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.println("__missing_condition__");
    }

    if (body_expr) {
      printer.println("Body");
      printer.enter();
      body_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.println("__missing_body_expr__");
    }
  }
  printer.exit();
}

void LetNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("Let");

  print_type(printer, symbols);

  printer.enter();
  {
    printer.println("Declarations");

    printer.enter();
    for (auto &attr : declarations) {
      attr->print(printer, symbols);
    }
    printer.exit();

    printer.println("Body");

    printer.enter();
    body_expr->print(printer, symbols);
    printer.exit();
  }
  printer.exit();
}

void CaseBranchNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println(std::format("{} : {}", symbols.get_string(object_id),
                              symbols.get_string(declared_type)));

  print_type(printer, symbols);

  printer.enter();
  body_expr->print(printer, symbols);
  printer.exit();
}

void CaseNode::print(Printer printer, const SymbolTable &symbols) {
  printer.println("Case");

  print_type(printer, symbols);

  printer.enter();
  {
    printer.println("Eval");
    printer.enter();
    eval_expr->print(printer, symbols);
    printer.exit();

    printer.println("Branches");
    printer.enter();
    for (auto &branch : branches) {
      branch->print(printer, symbols);
    }
    printer.exit();
  }
  printer.exit();
}

/***********************
 *                     *
 *        Arity        *
 *                     *
 **********************/

int ExpressionNode::arity() { return 0; }

int BinaryOpNode::arity() {
  if (left == nullptr)
    return 2;
  if (right == nullptr)
    return 1;
  return 0;
}

int UnaryOpNode::arity() {
  if (child == nullptr)
    return 1;
  return 0;
}

int AssignNode::arity() {
  if (variable.is_empty())
    return 2;
  if (!expression)
    return 1;
  return 0;
}

int DispatchNode::arity() {
  if (!target_self && target == nullptr)
    return 1;
  return 0;
}

/***********************
 *                     *
 *      Add child      *
 *                     *
 **********************/

void ExpressionNode::add_child(std::unique_ptr<ExpressionNode> &new_child) {
  throw std::logic_error("not implemented");
}

void UnaryOpNode::add_child(std::unique_ptr<ExpressionNode> &new_child) {
  if (child == nullptr) {
    child = std::move(new_child);
  } else {
    throw std::logic_error("too many children in unary op");
  }
}

void BinaryOpNode::add_child(std::unique_ptr<ExpressionNode> &new_child) {
  if (left == nullptr)
    left = std::move(new_child);
  else if (right == nullptr)
    right = std::move(new_child);
  else
    throw std::logic_error("too many children in binary op");
}

void AssignNode::add_child(std::unique_ptr<ExpressionNode> &new_child) {
  if (variable.is_empty())
    variable = new_child->start_token.symbol();
  else if (expression == nullptr)
    expression = std::move(new_child);
  else
    throw std::logic_error("too many children in assignment op");
}

void DispatchNode::add_child(std::unique_ptr<ExpressionNode> &new_child) {
  if (!target_self && target == nullptr) {
    target = std::move(new_child);
  } else {
    throw std::logic_error("too many children in dispatch");
  }
}

/***********************
 *                     *
 *      ChildSide      *
 *                     *
 **********************/

ChildSide ExpressionNode::child_side() { return ChildSide::NONE; }

ChildSide BinaryOpNode::child_side() {
  if (left == nullptr)
    return ChildSide::LEFT;
  if (right == nullptr)
    return ChildSide::RIGHT;
  return ChildSide::NONE;
}

ChildSide UnaryOpNode::child_side() {
  if (child == nullptr)
    return ChildSide::RIGHT;
  return ChildSide::NONE;
}

ChildSide AssignNode::child_side() {
  if (variable.is_empty())
    return ChildSide::LEFT;
  if (expression == nullptr)
    return ChildSide::RIGHT;
  return ChildSide::NONE;
}

ChildSide DispatchNode::child_side() {
  if (!target_self && target == nullptr)
    return ChildSide::LEFT;
  return ChildSide::NONE;
}
