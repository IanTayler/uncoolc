#include "ast.h"
#include "constant_eval.h"
#include "error.h"
#include "hlir.h"
#include <format>

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

hlir::Universe ModuleNode::to_hlir_universe(SymbolTable &symbols) const {
  auto universe = hlir::Universe();
  for (const auto &cls : classes) {
    universe.classes.emplace(cls->name.id, cls->to_hlir_class(symbols));
  }

  return universe;
}

hlir::InstructionList default_initialize(Symbol object_id, Symbol type,
                                         const SymbolTable &symbols,
                                         Token token) {
  hlir::InstructionList instructions;

  if (type == symbols.int_type) {
    instructions.push_back(
        std::make_unique<hlir::Mov>(hlir::Value::var(object_id, type),
                                    hlir::Value::literal(0, type), token));

  } else if (type == symbols.bool_type) {
    instructions.push_back(
        std::make_unique<hlir::Mov>(hlir::Value::var(object_id, type),
                                    hlir::Value::literal(false, type), token));

  } else if (type == symbols.string_type) {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::var(object_id, type),
        hlir::Value::literal(symbols.string_empty, type), token));

  } else {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::var(object_id, type),
        hlir::Value::literal(symbols.void_value, type), token));
  }

  return instructions;
}

hlir::Class ClassNode::to_hlir_class(SymbolTable &symbols) const {
  auto cls = hlir::Class(name);

  auto initializer_context = hlir::Context(symbols);

  for (const auto &attribute : attributes) {
    if (attribute->initializer.has_value()) {

      cls.initializer.splice(
          cls.initializer.end(),
          attribute->initializer.value()->to_hlir(initializer_context));

      cls.initializer.push_back(std::make_unique<hlir::Mov>(
          hlir::Value::var(attribute->object_id, attribute->declared_type),
          hlir::Value::acc(attribute->initializer.value()->static_type.value()),
          start_token));

    } else {
      cls.initializer.splice(cls.initializer.end(),
                             default_initialize(attribute->object_id,
                                                attribute->declared_type,
                                                symbols, start_token));
    }
  }

  for (const auto &method : methods) {
    cls.methods.emplace(method->name.id, method->to_hlir_method(symbols));
  }
  return cls;
}

hlir::Method MethodNode::to_hlir_method(SymbolTable &symbols) const {
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
  Symbol literal_type = static_type.value();
  auto instructions = hlir::InstructionList();

  if (literal_type == context.symbols.int_type) {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(literal_type),
        hlir::Value::literal(int_eval(value, context.symbols), literal_type),
        start_token));

  } else if (literal_type == context.symbols.bool_type) {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(literal_type),
        hlir::Value::literal(bool_eval(value, context.symbols), literal_type),
        start_token));

  } else if (literal_type == context.symbols.bool_type) {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(literal_type),
        hlir::Value::literal(string_eval(value, context.symbols), literal_type),
        start_token));

  } else {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(literal_type),
        hlir::Value::literal(value, literal_type), start_token));
  }

  return instructions;
}

hlir::InstructionList VariableNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  instructions.push_back(std::make_unique<hlir::Mov>(
      hlir::Value::acc(static_type.value()),
      hlir::Value::var(name, static_type.value()), start_token));

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
  Symbol result_type;

  switch (start_token.type()) {
  case TokenType::NEG_OP:
    hlir_op = hlir::Op::NEG;
    result_type = context.symbols.int_type;
    break;
  case TokenType::KW_NOT:
    hlir_op = hlir::Op::NOT;
    result_type = context.symbols.bool_type;
    break;
  case TokenType::KW_ISVOID:
    hlir_op = hlir::Op::IS_VOID;
    result_type = context.symbols.bool_type;
    break;
  default:
    fatal("INTERNAL: unsupported token type {} in UnaryOpNode when translating "
          "to hlir");
  }

  instructions.push_back(std::make_unique<hlir::Unary>(
      hlir_op, hlir::Value::acc(result_type),
      hlir::Value::acc(child->static_type.value()), start_token));

  return instructions;
}

hlir::InstructionList BinaryOpNode::to_hlir(hlir::Context &context) const {
  hlir::Op hlir_op;
  Symbol result_type;
  if (op == context.symbols.add_op) {
    hlir_op = hlir::Op::ADD;
    result_type = context.symbols.int_type;
  } else if (op == context.symbols.sub_op) {
    hlir_op = hlir::Op::SUB;
    result_type = context.symbols.int_type;
  } else if (op == context.symbols.mult_op) {
    hlir_op = hlir::Op::MULT;
    result_type = context.symbols.int_type;
  } else if (op == context.symbols.div_op) {
    hlir_op = hlir::Op::DIV;
    result_type = context.symbols.int_type;
  } else if (op == context.symbols.eq_op) {
    hlir_op = hlir::Op::EQUAL;
    result_type = context.symbols.bool_type;
  } else if (op == context.symbols.lt_op) {
    hlir_op = hlir::Op::LESS_THAN;
    result_type = context.symbols.bool_type;
  } else if (op == context.symbols.leq_op) {
    hlir_op = hlir::Op::LESS_EQUAL;
    result_type = context.symbols.bool_type;
  } else {
    fatal(std::format(
        "INTERNAL: unsupported op {} in BinaryOpNode when translating to hlir.",
        context.symbols.get_string(op)));
  }

  auto instructions = left->to_hlir(context);

  hlir::Value left_temp = context.create_temporary(left->static_type.value());

  instructions.push_back(std::make_unique<hlir::Mov>(
      left_temp, hlir::Value::acc(left->static_type.value()), start_token));

  instructions.splice(instructions.end(), right->to_hlir(context));

  instructions.push_back(std::make_unique<hlir::Binary>(
      hlir_op, hlir::Value::acc(result_type), left_temp,
      hlir::Value::acc(right->static_type.value()), start_token));

  return instructions;
}

hlir::InstructionList NewNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  instructions.push_back(
      std::make_unique<hlir::New>(hlir::Op::NEW, hlir::Value::acc(created_type),
                                  created_type, start_token));
  return instructions;
}

hlir::InstructionList AssignNode::to_hlir(hlir::Context &context) const {
  auto instructions = expression->to_hlir(context);
  Symbol type = expression->static_type.value();

  instructions.push_back(std::make_unique<hlir::Mov>(
      hlir::Value::var(variable, type), hlir::Value::acc(type), start_token));
  return instructions;
}

hlir::InstructionList DispatchNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();
  std::vector<hlir::Value> argument_temporaries;

  for (const auto &argument : arguments) {
    instructions.splice(instructions.end(), argument->to_hlir(context));

    Symbol argument_type = argument->static_type.value();

    hlir::Value temporary = context.create_temporary(argument_type);
    argument_temporaries.push_back(temporary);

    instructions.push_back(std::make_unique<hlir::Mov>(
        temporary, hlir::Value::acc(argument_type), start_token));
  }

  Symbol target_type;

  if (target) {
    instructions.splice(instructions.end(), target->to_hlir(context));
    target_type = target->static_type.value();

  } else {
    instructions.push_back(std::make_unique<hlir::Mov>(
        hlir::Value::acc(context.symbols.self_type),
        hlir::Value::var(context.symbols.self_var, context.symbols.self_type),
        start_token));

    target_type = context.symbols.self_type;
  }

  auto call = hlir::Call(hlir::Value::acc(target_type), method, start_token);

  // Add all the arguments before the call
  for (const auto &temporary : argument_temporaries) {
    call.add_arg(temporary);
  }

  instructions.push_back(std::make_unique<hlir::Call>(call));

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

hlir::InstructionList IfNode::to_hlir(hlir::Context &context) const {
  int else_label_idx = context.create_label_idx();
  int exit_label_idx = context.create_label_idx();

  auto instructions = condition_expr->to_hlir(context);

  // else goes before the exit, we need to create it now to point to it from the
  // condition
  instructions.push_back(std::make_unique<hlir::Label>(
      else_label_idx, context.symbols.else_kw, else_expr->start_token));

  hlir::Position else_position =
      hlir::Position(else_label_idx, --instructions.end());

  // Insert the jump to the else block
  instructions.insert(else_position.label,
                      std::make_unique<hlir::Branch>(
                          hlir::BranchCondition::FALSE,
                          hlir::Value::acc(condition_expr->static_type.value()),
                          else_position, condition_expr->start_token));

  // Add the then part right after the check
  instructions.splice(else_position.label, then_expr->to_hlir(context));

  // Now add the else part after the else label
  instructions.splice(instructions.end(), else_expr->to_hlir(context));

  // Put an exit label right at the end
  instructions.push_back(std::make_unique<hlir::Label>(
      exit_label_idx, context.symbols.fi_kw, start_token));

  hlir::Position exit_position =
      hlir::Position(exit_label_idx, --instructions.end());

  // Add a jump to the exit after the then, skipping the else section
  instructions.insert(else_position.label,
                      std::make_unique<hlir::Branch>(
                          hlir::BranchCondition::ALWAYS,
                          hlir::Value::literal(true, context.symbols.bool_type),
                          exit_position, then_expr->start_token));

  return instructions;
}

hlir::InstructionList WhileNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();

  int condition_label_idx = context.create_label_idx();
  int exit_label_idx = context.create_label_idx();

  instructions.push_back(std::make_unique<hlir::Label>(
      condition_label_idx, context.symbols.loop_kw, start_token));

  hlir::Position condition_position =
      hlir::Position{condition_label_idx, --instructions.end()};

  instructions.splice(instructions.end(), condition_expr->to_hlir(context));

  instructions.push_back(std::make_unique<hlir::Label>(
      exit_label_idx, context.symbols.pool_kw, start_token));

  hlir::Position exit_position =
      hlir::Position{exit_label_idx, --instructions.end()};

  // Insert the branch after the condition evaluation
  instructions.insert(exit_position.label,
                      std::make_unique<hlir::Branch>(
                          hlir::BranchCondition::FALSE,
                          hlir::Value::acc(condition_expr->static_type.value()),
                          exit_position, body_expr->start_token));

  // Now put the while body right after that
  instructions.splice(exit_position.label, body_expr->to_hlir(context));

  // Finally, at the end of the while, we unconditionally return to the
  // condition evaluation
  instructions.insert(exit_position.label,
                      std::make_unique<hlir::Branch>(
                          hlir::BranchCondition::ALWAYS,
                          hlir::Value::literal(true, context.symbols.bool_type),
                          condition_position, body_expr->start_token));

  return instructions;
}

hlir::InstructionList LetNode::to_hlir(hlir::Context &context) const {
  auto instructions = hlir::InstructionList();

  for (const auto &declaration : declarations) {
    if (declaration->initializer.has_value()) {
      instructions.splice(instructions.end(),
                          declaration->initializer.value()->to_hlir(context));

      instructions.push_back(std::make_unique<hlir::Mov>(
          hlir::Value::var(declaration->object_id, declaration->declared_type),
          hlir::Value::acc(declaration->declared_type),
          declaration->start_token));

    } else {
      instructions.splice(
          instructions.end(),
          default_initialize(declaration->object_id, declaration->declared_type,
                             context.symbols, declaration->start_token));
    }
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
