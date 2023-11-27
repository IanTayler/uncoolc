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
  printer.print(std::format("attr {}", symbols->get_string(object_id)));

  printer.enter();
  {
    printer.print(std::format("type {}", symbols->get_string(declared_type)));
    if (initializer) {
      printer.print("initializer");
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
  printer.print(std::format("method {}", symbols->get_string(name)));

  printer.enter();
  {
    for (auto &param : parameters) {
      param->print(printer, symbols);
    }
    printer.print(std::format("return {}", symbols->get_string(return_type)));

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

void BinaryOpNode::print(AstPrinter printer,
                         std::shared_ptr<SymbolTable> symbols) {
  std::string op_rep;
  switch (op) {
  case Operator::ADD:
    op_rep = "+";
    break;
  case Operator::SUB:
    op_rep = "-";
    break;
  case Operator::DIV:
    op_rep = "/";
    break;
  case Operator::MULT:
    op_rep = "*";
    break;
  case Operator::LEQ:
    op_rep = "<=";
    break;
  case Operator::LT:
    op_rep = "<";
    break;
  case Operator::EQ:
    op_rep = "=";
    break;
  default:
    op_rep = "__UNKNOWN_OPERATOR_TYPE__";
    break;
  }

  printer.print(std::format("BinaryOp {}", op_rep));

  printer.enter();
  {
    left->print(printer, symbols);
    right->print(printer, symbols);
  }
  printer.exit();
}
