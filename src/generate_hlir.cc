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
  } else if (op == context.symbols.eq_op) {
    hlir_op = hlir::Op::Equal;
  } else if (op == context.symbols.lt_op) {
    hlir_op = hlir::Op::LessThan;
  } else if (op == context.symbols.leq_op) {
    hlir_op = hlir::Op::LessEqual;
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

hlir::InstructionList AssignNode::to_hlir(hlir::Context &context) const {
  auto instructions = expression->to_hlir(context);

  instructions.push_back(std::make_unique<hlir::Mov>(hlir::Value::var(variable),
                                                     hlir::Value::acc()));
  return instructions;
}

hlir::InstructionList DispatchNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  std::vector<hlir::Value> argument_temporaries;

  for (const auto &argument : arguments) {
    instructions.splice(instructions.end(), argument->to_hlir(context));

    hlir::Value temporary = context.create_temporary();
    argument_temporaries.push_back(temporary);

    instructions.push_back(
        std::make_unique<hlir::Mov>(temporary, hlir::Value::acc()));
  }

  if (target)
    instructions.splice(instructions.end(), target->to_hlir(context));
  else
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(), hlir::Value::var(context.symbols.self_var)));

  hlir::Value target_temp = context.create_temporary();

  // Add all the arguments before the call
  for (const auto &temporary : argument_temporaries) {
    instructions.push_back(std::make_unique<hlir::AddArg>(temporary));
  }

  instructions.push_back(std::make_unique<hlir::Call>(target_temp, method));

  return instructions;
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

hlir::InstructionList WhileNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();

  int condition_label_idx = context.create_label_idx();
  int exit_label_idx = context.create_label_idx();

  instructions.push_back(std::make_unique<hlir::Label>(
      condition_label_idx, context.symbols.while_kw));

  hlir::Position condition_position =
      hlir::Position{condition_label_idx, --instructions.end()};

  instructions.splice(instructions.end(), condition_expr->to_hlir(context));

  instructions.push_back(
      std::make_unique<hlir::Label>(exit_label_idx, context.symbols.pool_kw));

  hlir::Position exit_position =
      hlir::Position{exit_label_idx, --instructions.end()};

  // Insert the branch before the exit label
  instructions.insert(exit_position.it, std::make_unique<hlir::Branch>(
                                            hlir::BranchCondition::False,
                                            hlir::Value::acc(), exit_position));

  // Now put the while body before the exit label as well
  instructions.splice(exit_position.it, body_expr->to_hlir(context));

  // Finally, at the end of the while, we unconditionally return to the
  // condition evaluation
  instructions.insert(
      exit_position.it,
      std::make_unique<hlir::Branch>(hlir::BranchCondition::Always,
                                     hlir::Value::acc(), condition_position));

  return instructions;
}

hlir::InstructionList LetNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();

  for (const auto &declaration : declarations) {
    if (declaration->initializer.has_value())
      instructions.splice(instructions.end(),
                          declaration->initializer.value()->to_hlir(context));
    // TODO(IT) handle the else case: default initializer

    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::var(declaration->object_id), hlir::Value::acc()));
  }

  instructions.splice(instructions.end(), body_expr->to_hlir(context));

  return instructions;
}

// TODO(IT) fill in
hlir::InstructionList CaseBranchNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}

// TODO(IT) fill in
hlir::InstructionList CaseNode::to_hlir(hlir::Context &context) const {
  return hlir::InstructionList();
}
