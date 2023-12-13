#include "ast.h"
#include <format>

/***********************
 *                     *
 *      AstPrinter     *
 *                     *
 **********************/

void AstPrinter::print(std::string str) {
  for (int i = 0; i < current_depth; i++)
    for (int j = 0; j < indent; j++)
      *out << ' ';
  *out << str << std::endl;
}

void AstPrinter::enter() { current_depth++; }

void AstPrinter::exit() { current_depth--; }

/***********************
 *                     *
 *    Node Printers    *
 *                     *
 **********************/

void AstNode::print(AstPrinter printer, std::shared_ptr<SymbolTable>) {
  printer.print("__NODE_PRINT_UNDEFINED__");
}

void ModuleNode::print(AstPrinter printer,
                       std::shared_ptr<SymbolTable> symbols) {
  for (auto &class_ : classes) {
    class_->print(printer, symbols);
  }
}

void ClassNode::print(AstPrinter printer,
                      std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("class {} inherits {}", symbols->get_string(name),
                            symbols->get_string(superclass)));

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

void AttributeNode::print(AstPrinter printer,
                          std::shared_ptr<SymbolTable> symbols) {
  if (initializer) {
    printer.print(std::format("attr {} : {} <-", symbols->get_string(object_id),
                              symbols->get_string(declared_type)));
    printer.enter();
    (*initializer)->print(printer, symbols);
    printer.exit();
  } else {
    printer.print(std::format("attr {} : {}", symbols->get_string(object_id),
                              symbols->get_string(declared_type)));
  }
}

void ParameterNode::print(AstPrinter printer,
                          std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("param {} : {}", symbols->get_string(object_id),
                            symbols->get_string(declared_type)));
}

void MethodNode::print(AstPrinter printer,
                       std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("method {} : {}", symbols->get_string(name),
                            symbols->get_string(return_type)));

  printer.enter();
  {
    for (auto &param : parameters) {
      param->print(printer, symbols);
    }
    printer.print("body");
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

void ExpressionNode::print(AstPrinter printer,
                           std::shared_ptr<SymbolTable> symbols) {
  printer.print("__EXPRESSION_PRINT_UNDEFINED__");
}

void LiteralNode::print(AstPrinter printer,
                        std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("Literal {}", symbols->get_string(value)));
}

void VariableNode::print(AstPrinter printer,
                         std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("Variable {}", symbols->get_string(name)));
}

void NewNode::print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("new {}", symbols->get_string(created_type)));
}

void UnaryOpNode::print(AstPrinter printer,
                        std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("UnaryOp {}", symbols->get_string(op)));

  printer.enter();
  if (child)
    child->print(printer, symbols);
  else
    printer.print("__missing_child__");
  printer.exit();
}

void BinaryOpNode::print(AstPrinter printer,
                         std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("BinaryOp {}", symbols->get_string(op)));

  printer.enter();
  {
    if (left)
      left->print(printer, symbols);
    else
      printer.print("__missing_left__");
    if (right)
      right->print(printer, symbols);
    else
      printer.print("__missing_right__");
  }
  printer.exit();
}

void AssignNode::print(AstPrinter printer,
                       std::shared_ptr<SymbolTable> symbols) {
  if (variable.is_empty())
    printer.print("__missing__variable__ <-");
  else
    printer.print(std::format("{} <-", symbols->get_string(variable)));

  printer.enter();
  {
    if (expression)
      expression->print(printer, symbols);
    else
      printer.print("__missing_expression__");
  }
  printer.exit();
}

void DispatchNode::print(AstPrinter printer,
                         std::shared_ptr<SymbolTable> symbols) {
  printer.print("Dispatch");

  printer.enter();
  {
    printer.print("target");

    printer.enter();
    {
      if (target_self)
        printer.print("self");
      else if (target)
        target->print(printer, symbols);
      else
        printer.print("__missing_target__");
    }
    printer.exit();

    if (dispatch_type)
      printer.print(std::format("@{}", symbols->get_string(*dispatch_type)));

    printer.print(std::format("method {}", symbols->get_string(method)));

    printer.print("arguments");
    printer.enter();
    for (auto &arg : arguments) {
      arg->print(printer, symbols);
    }
    printer.exit();
  }
  printer.exit();
}

void BlockNode::print(AstPrinter printer,
                      std::shared_ptr<SymbolTable> symbols) {
  printer.print("Block");

  printer.enter();
  {
    for (auto &expr : expressions) {
      expr->print(printer, symbols);
    }
  }
  printer.exit();
}

void IfNode::print(AstPrinter printer, std::shared_ptr<SymbolTable> symbols) {
  printer.print("If");

  printer.enter();
  {
    if (condition_expr) {
      printer.print("Condition");
      printer.enter();
      condition_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.print("__missing_condition__");
    }

    if (then_expr) {
      printer.print("Then");
      printer.enter();
      then_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.print("__missing_then_expr__");
    }

    if (else_expr) {
      printer.print("Else");
      printer.enter();
      else_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.print("__missing_else_expr__");
    }
  }
  printer.exit();
}

void WhileNode::print(AstPrinter printer,
                      std::shared_ptr<SymbolTable> symbols) {
  printer.print("While");

  printer.enter();
  {
    if (condition_expr) {
      printer.print("Condition");
      printer.enter();
      condition_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.print("__missing_condition__");
    }

    if (body_expr) {
      printer.print("Body");
      printer.enter();
      body_expr->print(printer, symbols);
      printer.exit();
    } else {
      printer.print("__missing_body_expr__");
    }
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
