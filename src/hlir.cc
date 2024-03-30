#include "hlir.h"
#include "error.h"
#include <format>

namespace hlir {

/***********************
 *                     *
 *        Value        *
 *                     *
 **********************/

//
// String representation
//

std::string to_string(ValueKind kind) {
  switch (kind) {
  case ValueKind::SELF:
    return "self";
  case ValueKind::VAR:
    return "var";
  case ValueKind::TEMP:
    return "temp";
  case ValueKind::ACC:
    return "acc";
  case ValueKind::CONSTANT:
    return "literal";
  case ValueKind::TYPE_ID:
    return "type";
  }
}

std::string to_string(Value value, const SymbolTable &symbols) {
  ValueKind kind = value.kind;

  switch (kind) {
  case ValueKind::SELF:
    return "[self]";
  case ValueKind::VAR:
    return std::format("[var: {}]", symbols.get_string(value.symbol));
  case ValueKind::TEMP:
    return std::format("[temp: {}]", value.num);
  case ValueKind::ACC:
    return "[acc]";
  case ValueKind::CONSTANT:
    if (value.static_type == symbols.bool_type ||
        value.static_type == symbols.int_type) {
      return std::format("{}", value.boolean);

      return std::format("{}", symbols.get_string(value.symbol));
    case ValueKind::TYPE_ID:
      return std::format("[type: {}]", symbols.get_string(value.symbol));
    }
  }
  return "__unknown_value_kind__";
}

//
// Private constructors
//

Value::Value(ValueKind k, int i, Symbol st)
    : kind(k), num(i), static_type(st) {}

Value::Value(ValueKind k, Symbol n, Symbol st)
    : kind(k), symbol(n), static_type(st) {}

Value::Value(ValueKind k, Symbol st) : kind(k), static_type(st) {}

//
// Public constructors
//

Value Value::self(Symbol static_type) {
  return Value(ValueKind::SELF, static_type);
}

Value Value::var(Symbol name, Symbol static_type) {
  return Value(ValueKind::VAR, name, static_type);
}

Value Value::temp(int id, Symbol static_type) {
  return Value(ValueKind::TEMP, id, static_type);
}

Value Value::acc(Symbol static_type) {
  return Value(ValueKind::ACC, static_type);
}

Value Value::literal(int value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::literal(bool value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::literal(Symbol value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::type_id(Symbol type_name, Symbol static_type) {
  return Value(ValueKind::TYPE_ID, type_name, static_type);
}

/***********************
 *                     *
 *          Op         *
 *                     *
 **********************/

std::string to_string(Op op) {
  switch (op) {
  case Op::ADD:
    return "add";
  case Op::SUB:
    return "sub";
  case Op::MULT:
    return "mult";
  case Op::DIV:
    return "div";
  case Op::EQUAL:
    return "eq";
  case Op::LESS_EQUAL:
    return "leq";
  case Op::LESS_THAN:
    return "lt";
  case Op::NEG:
    return "neg";
  case Op::NOT:
    return "not";
  case Op::IS_VOID:
    return "isvoid";
  case Op::NEW:
    return "new";
  case Op::ADD_ARG:
    return "add_arg";
  case Op::CALL:
    return "call";
  case Op::BRANCH:
    return "branch";
  case Op::LABEL:
    return "label";
  case Op::MOV:
    return "mov";
  case Op::TYPE_ID_OF:
    return "typeof";
  case Op::SUPERCLASS:
    return "superclass";
  }
}

/***********************
 *                     *
 *   BranchCondition   *
 *                     *
 **********************/

std::string to_string(const BranchCondition condition) {
  switch (condition) {
  case BranchCondition::ALWAYS:
    return "always";
  case BranchCondition::TRUE:
    return "True";
  case BranchCondition::FALSE:
    return "False";
  }
}

/***********************
 *                     *
 *       Position      *
 *                     *
 **********************/

Position::Position(int i, InstructionList::iterator ili)
    : label_idx(i), label(ili) {}

std::string to_string(Position position) {
  return std::format("[label {}]", position.label_idx);
}

/***********************
 *                     *
 *     Instructions    *
 *                     *
 **********************/

//
// Base Instruction
//

Instruction::Instruction(Op o, Token t) : op(o), token(t) {}

void Instruction::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print("__undefined_instruction_print__");
  printer.exit();
}

//
// Unary
//

Unary::Unary(Op o, Value d, Value c, Token t)
    : dest(d), arg(c), Instruction(o, t) {}

void Unary::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format("{} {}, {}", hlir::to_string(op),
                            hlir::to_string(dest, symbols),
                            hlir::to_string(arg, symbols)));
  printer.exit();
}

//
// New
//

New::New(Op o, Value d, Symbol ty, Token to)
    : dest(d), type(ty), Instruction(o, to) {}

void New::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format("{} {}, {}", hlir::to_string(op),
                            hlir::to_string(dest, symbols),
                            symbols.get_string((type))));
  printer.exit();
}

//
// Binary
//

Binary::Binary(Op o, Value d, Value l, Value r, Token t)
    : dest(d), left(l), right(r), Instruction(o, t) {}

void Binary::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format(
      "{} {}, {}, {}", hlir::to_string(op), hlir::to_string(dest, symbols),
      hlir::to_string(left, symbols), hlir::to_string(right, symbols)));
  printer.exit();
}

//
// AddArg
//

AddArg::AddArg(Value a, Token t) : arg(a), Instruction(Op::ADD_ARG, t) {}

void AddArg::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(
      std::format("{} {}", hlir::to_string(op), hlir::to_string(arg, symbols)));
  printer.exit();
}

//
// Call
//

Call::Call(Value ta, Symbol n, Token to)
    : target(ta), method_name(n), Instruction(Op::CALL, to) {}

void Call::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format("{} {}, {}", hlir::to_string(op),
                            hlir::to_string(target, symbols),
                            symbols.get_string(method_name)));
  printer.exit();
}

//
// Branch
//

Branch::Branch(BranchCondition bc, Value v, Position l, Token t)
    : value(v), condition(bc), target(l), Instruction(Op::BRANCH, t) {}

void Branch::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format(
      "{}.{} {} {}", hlir::to_string(op), hlir::to_string(condition),
      hlir::to_string(value, symbols), hlir::to_string(target)));
  printer.exit();
}

//
// Label
//

Label::Label(int i, Symbol n, Token t)
    : idx(i), name(n), Instruction(Op::LABEL, t) {}

void Label::print(Printer printer, const SymbolTable &symbols) const {
  printer.print(std::format("{}: // {}", idx, symbols.get_string(name)));
}

//
// Mov
//

Mov::Mov(Value d, Value s, Token t)
    : dest(d), src(s), Instruction(Op::MOV, t) {}

void Mov::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format("{} {}, {}", hlir::to_string(op),
                            hlir::to_string(dest, symbols),
                            hlir::to_string(src, symbols)));
  printer.exit();
}

/***********************
 *                     *
 *      Context        *
 *                     *
 **********************/

Context::Context(SymbolTable &s) : temporaries(0), labels(0), symbols(s) {}

Value Context::create_temporary(Symbol static_type) {
  return Value::temp(temporaries++, static_type);
}

int Context::create_label_idx() { return labels++; }

/***********************
 *                     *
 *        Method       *
 *                     *
 **********************/
Method::Method(Symbol n) : name(n) {}

void Method::print(Printer printer, const SymbolTable &symbols) const {
  printer.print(std::format("{} {{", symbols.get_string(name)));

  for (const auto &instruction : instructions) {
    instruction->print(printer, symbols);
  }

  printer.print("}");
}

/***********************
 *                     *
 *        Class        *
 *                     *
 **********************/

Class::Class(Symbol n) : name(n) {}

void Class::print(Printer printer, const SymbolTable &symbols) const {
  printer.print(symbols.get_string(name));
  printer.print("{");

  printer.enter();

  printer.print("__initializer__ {");
  for (const auto &instruction : initializer) {
    instruction->print(printer, symbols);
  }
  printer.print("}");

  for (const auto &[_, method] : methods) {
    method.print(printer, symbols);
  }
  printer.exit();
  printer.print("}");
  printer.print("");
}

/***********************
 *                     *
 *      Universe       *
 *                     *
 **********************/
void Universe::print(Printer printer, const SymbolTable &symbols) const {
  for (const auto &[_, cls] : classes) {
    cls.print(printer, symbols);
  }
}

}; // namespace hlir
