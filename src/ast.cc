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

void ExpressionNode::print(AstPrinter printer,
                           std::shared_ptr<SymbolTable> symbols) {
  printer.print("__EXPRESSION_PRINT_UNDEFINED__");
}

void LiteralNode::print(AstPrinter printer,
                        std::shared_ptr<SymbolTable> symbols) {
  printer.print(std::format("Literal {}", symbols->get_string(value)));
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
