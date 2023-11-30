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
  printer.print(std::format("attr {} : {}", symbols->get_string(object_id),
                            symbols->get_string(declared_type)));

  printer.enter();
  {
    if (initializer) {
      printer.print("<-");
      printer.enter();
      (*initializer)->print(printer, symbols);
      printer.exit();
    }
  }
  printer.exit();
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
int UnaryOpNode::arity() { if (child == nullptr) return 1; return 0; }
int ExpressionNode::arity() { return 0; }

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
