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
  case ValueType::Self:
    return "self";
  case ValueType::Var:
    return "var";
  case ValueType::Temp:
    return "temp";
  case ValueType::Acc:
    return "acc";
  case ValueType::Literal:
    return "literal";
  }
}

std::string to_string(Value value, const SymbolTable &symbols) {
  ValueType type = value.type;

  switch (type) {
  case ValueType::Self:
    return "[self]";
  case ValueType::Var:
    return std::format("[var: {}]", symbols.get_string(value.name));
  case ValueType::Temp:
    return std::format("[temp: {}]", value.id);
  case ValueType::Acc:
    return "[acc]";
  case ValueType::Literal:
    return std::format("{}", symbols.get_string(value.value));
  }
}

//
// Private constructors
//

Value::Value(ValueType t, int i) : type(t), id(i) {}

Value::Value(ValueType t, Symbol n) : type(t), name(n) {}

Value::Value(ValueType t) : type(t), name(Symbol{}) {}

//
// Public constructors
//

Value Value::self() { return Value(ValueType::Self); }

Value Value::var(Symbol name) { return Value(ValueType::Var, name); }

Value Value::temp(int id) { return Value(ValueType::Temp, id); }

Value Value::acc() { return Value(ValueType::Acc); }

Value Value::literal(Symbol value) { return Value(ValueType::Literal, value); }

/***********************
 *                     *
 *          Op         *
 *                     *
 **********************/

std::string to_string(Op op) {
  switch (op) {
  case Op::Add:
    return "add";
  case Op::Sub:
    return "sub";
  case Op::Mult:
    return "mult";
  case Op::Div:
    return "div";
  case Op::Equal:
    return "eq";
  case Op::LessEqual:
    return "leq";
  case Op::LessThan:
    return "lt";
  case Op::Neg:
    return "neg";
  case Op::Not:
    return "not";
  case Op::IsVoid:
    return "isvoid";
  case Op::New:
    return "new";
  case Op::AddArg:
    return "add_arg";
  case Op::Call:
    return "call";
  case Op::Branch:
    return "branch";
  case Op::Label:
    return "label";
  case Op::Mov:
    return "mov";
  }
}

/***********************
 *                     *
 *   BranchCondition   *
 *                     *
 **********************/

std::string to_string(const BranchCondition condition) {
  switch (condition) {
  case BranchCondition::Always:
    return "always";
  case BranchCondition::True:
    return "True";
  case BranchCondition::False:
    return "False";
  }
}

/***********************
 *                     *
 *       Position      *
 *                     *
 **********************/

Position::Position(int i, InstructionList::iterator ili) : idx(i), it(ili) {}

std::string to_string(Position position) {
  return std::format("[label {}]", position.idx);
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

Unary::Unary(Op o, Value d, Value c) : dest(d), child(c), Instruction(o) {}

void Unary::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.print(std::format("{} {}, {}", hlir::to_string(op),
                            hlir::to_string(dest, symbols),
                            hlir::to_string(child, symbols)));
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

AddArg::AddArg(Value a) : arg(a), Instruction(Op::AddArg) {}

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
    : target(t), method_name(n), Instruction(Op::Call) {}

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
    : value(v), condition(bc), target(l), Instruction(Op::Branch) {}

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

Label::Label(int i) : idx(i), Instruction(Op::Label) {}

void Label::print(Printer printer, const SymbolTable &symbols) const {
  printer.print(std::format("{}:", idx));
}

//
// Mov
//

Mov::Mov(Value d, Value s) : dest(d), src(s), Instruction(Op::Mov) {}

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
