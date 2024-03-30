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

std::string to_string(ValueType type) {
  switch (type) {
  case ValueType::SELF:
    return "self";
  case ValueType::VAR:
    return "var";
  case ValueType::TEMP:
    return "temp";
  case ValueType::ACC:
    return "acc";
  case ValueType::LITERAL:
    return "literal";
  case ValueType::TYPE_ID:
    return "type";
  }
}

std::string to_string(Value value, const SymbolTable &symbols) {
  ValueType type = value.type;

  switch (type) {
  case ValueType::SELF:
    return "[self]";
  case ValueType::VAR:
    return std::format("[var: {}]", symbols.get_string(value.symbol));
  case ValueType::TEMP:
    return std::format("[temp: {}]", value.id);
  case ValueType::ACC:
    return "[acc]";
  case ValueType::LITERAL:
    return std::format("{}", symbols.get_string(value.symbol));
  case ValueType::TYPE_ID:
    return std::format("[type: {}]", symbols.get_string(value.symbol));
  }
}

//
// Private constructors
//

Value::Value(ValueType t, int i) : type(t), id(i) {}

Value::Value(ValueType t, Symbol n) : type(t), symbol(n) {}

Value::Value(ValueType t) : type(t), symbol(Symbol{}) {}

//
// Public constructors
//

Value Value::self() { return Value(ValueType::SELF); }

Value Value::var(Symbol name) { return Value(ValueType::VAR, name); }

Value Value::temp(int id) { return Value(ValueType::TEMP, id); }

Value Value::acc() { return Value(ValueType::ACC); }

Value Value::literal(Symbol value) { return Value(ValueType::LITERAL, value); }

Value Value::type_id(Symbol type_name) {
  return Value(ValueType::TYPE_ID, type_name);
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

Instruction::Instruction(Op o) : op(o) {}

void Instruction::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print("__undefined_instruction_print__");
  printer.exit();
}

//
// Unary
//

Unary::Unary(Op o, Value d, Value c) : dest(d), arg(c), Instruction(o) {}

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

New::New(Op o, Value d, Symbol t) : dest(d), type(t), Instruction(o) {}

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

Binary::Binary(Op o, Value d, Value l, Value r)
    : dest(d), left(l), right(r), Instruction(o) {}

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

AddArg::AddArg(Value a) : arg(a), Instruction(Op::ADD_ARG) {}

void AddArg::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(
      std::format("{} {}", hlir::to_string(op), hlir::to_string(arg, symbols)));
  printer.exit();
}

//
// Call
//

Call::Call(Value t, Symbol n)
    : target(t), method_name(n), Instruction(Op::CALL) {}

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

Branch::Branch(BranchCondition bc, Value v, Position l)
    : value(v), condition(bc), target(l), Instruction(Op::BRANCH) {}

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

Label::Label(int i, Symbol n) : idx(i), name(n), Instruction(Op::LABEL) {}

void Label::print(Printer printer, const SymbolTable &symbols) const {
  printer.print(std::format("{}: // {}", idx, symbols.get_string(name)));
}

//
// Mov
//

Mov::Mov(Value d, Value s) : dest(d), src(s), Instruction(Op::MOV) {}

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

Context::Context(const SymbolTable &s)
    : temporaries(0), labels(0), symbols(s) {}

Value Context::create_temporary() { return Value::temp(temporaries++); }

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
