#include "ast.h"
#include "error.h"
#include "hlir.h"
#include <format>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

hlir::Universe ModuleNode::to_hlir_universe(const SymbolTable &symbols) const {
  auto universe = hlir::Universe();
  for (const auto &cls : classes) {
    universe.classes.emplace(cls->name.id, cls->to_hlir_class(symbols));
  }

  return universe;
}

hlir::Class ClassNode::to_hlir_class(const SymbolTable &symbols) const {
  auto cls = hlir::Class(name);

  auto initializer_context = hlir::Context(symbols);

  for (const auto &attribute : attributes) {
    if (attribute->initializer.has_value())
      cls.initializer.splice(
          cls.initializer.end(),
          attribute->initializer.value()->to_hlir(initializer_context));
    // TODO(IT) default initializers in the else case

    cls.initializer.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::var(attribute->object_id), hlir::Value::acc()));
  }

  for (const auto &method : methods) {
    cls.methods.emplace(method->name.id, method->to_hlir_method(symbols));
  }
  return cls;
}

hlir::Method MethodNode::to_hlir_method(const SymbolTable &symbols) const {
  auto method = hlir::Method(name);
  auto context = hlir::Context(symbols);

  method.instructions = body->to_hlir(context);

  return method;
}

hlir::InstructionList ExpressionNode::to_hlir(hlir::Context &context) const {
  fatal("INTERNAL: Should not call to_hlir on bare ExpressionNode");
  return hlir::InstructionList(); // fool linter
}
/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

// TODO(IT) fill in
hlir::InstructionList BuiltinNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

hlir::InstructionList LiteralNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  instructions.push_back(std::make_unique<hlir::Mov>(
      hlir::Value::acc(), hlir::Value::literal(value)));

  return instructions;
}

hlir::InstructionList VariableNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  instructions.push_back(
      std::make_unique<hlir::Mov>(hlir::Value::acc(), hlir::Value::var(name)));

  return instructions;
}

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

hlir::InstructionList UnaryOpNode::to_hlir(hlir::Context &context) const {
  hlir::InstructionList instructions = child->to_hlir(context);

  hlir::Op hlir_op;
  switch (start_token.type()) {
  case TokenType::NEG_OP:
    hlir_op = hlir::Op::Neg;
    break;
  case TokenType::KW_NOT:
    hlir_op = hlir::Op::Not;
    break;
  case TokenType::KW_ISVOID:
    hlir_op = hlir::Op::IsVoid;
    break;
  default:
    fatal("INTERNAL: unsupported token type {} in UnaryOpNode when translating "
          "to hlir");
  }

  instructions.push_back(std::make_unique<hlir::Unary>(
      hlir_op, hlir::Value::acc(), hlir::Value::acc()));

  return instructions;
}

hlir::InstructionList BinaryOpNode::to_hlir(hlir::Context &context) const {
  hlir::Op hlir_op;
  if (op == context.symbols.add_op) {
    hlir_op = hlir::Op::Add;
  } else if (op == context.symbols.sub_op) {
    hlir_op = hlir::Op::Sub;
  } else if (op == context.symbols.mult_op) {
    hlir_op = hlir::Op::Mult;
  } else if (op == context.symbols.div_op) {
    hlir_op = hlir::Op::Div;
  } else {
    fatal(std::format(
        "INTERNAL: unsupported op {} in BinaryOpNode when translating to hlir.",
        context.symbols.get_string(op)));
  }

  auto instructions = left->to_hlir(context);

  hlir::Value left_temp = context.create_temporary();

  instructions.push_back(
      std::make_unique<hlir::Mov>(left_temp, hlir::Value::acc()));

  instructions.splice(instructions.end(), right->to_hlir(context));

  instructions.push_back(std::make_unique<hlir::Binary>(
      hlir_op, hlir::Value::acc(), left_temp, hlir::Value::acc()));

  return instructions;
}

hlir::InstructionList NewNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  instructions.push_back(std::make_unique<hlir::New>(
      hlir::Op::New, hlir::Value::acc(), created_type));
  return instructions;
}

// TODO(IT) fill in
hlir::InstructionList AssignNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList DispatchNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

hlir::InstructionList BlockNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  for (const auto &expression : expressions) {
    instructions.splice(instructions.end(), expression->to_hlir(context));
  }
  return instructions;
}

// TODO(IT) fill in
hlir::InstructionList IfNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList WhileNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList LetNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList CaseBranchNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList CaseNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}
