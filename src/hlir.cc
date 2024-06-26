#include "hlir.h"
#include "error.h"
#include "runtime.h"
#include <format>

namespace hlir {

static Value DUMMY_EMPTY_VALUE = Value::empty();

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
  case ValueKind::LOCAL:
    return "local";
  case ValueKind::ATTRIBUTE:
    return "attr";
  case ValueKind::TEMP:
    return "temp";
  case ValueKind::ACC:
    return "acc";
  case ValueKind::CONSTANT:
    return "constant";
  case ValueKind::EMPTY:
    return "empty";
  }
}

std::string to_string(Value value, const SymbolTable &symbols) {
  ValueKind kind = value.kind;

  switch (kind) {
  case ValueKind::SELF:
    return "[self]";

  case ValueKind::LOCAL:
    return std::format("[local: {}]", symbols.get_string(value.symbol));

  case ValueKind::ATTRIBUTE:
    return std::format("[attr: {}]", symbols.get_string(value.symbol));

  case ValueKind::TEMP:
    return std::format("[temp: {}]", value.num);

  case ValueKind::ACC:
    return "[acc]";

  case ValueKind::CONSTANT:
    if (value.static_type == symbols.bool_type)
      return std::format("{}", value.boolean);

    if (value.static_type == symbols.int_type)
      return std::format("{}", value.num);

    if (value.static_type == symbols.string_type)
      return std::format("\"{}\"", value.num);

    return std::format("{}", symbols.get_string(value.symbol));

  case ValueKind::EMPTY:
    return "[empty]";
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

Value Value::attr(Symbol name, Symbol static_type) {
  return Value(ValueKind::ATTRIBUTE, name, static_type);
}

Value Value::local(Symbol name, Symbol static_type) {
  return Value(ValueKind::LOCAL, name, static_type);
}

Value Value::temp(int id, Symbol static_type) {
  return Value(ValueKind::TEMP, id, static_type);
}

Value Value::acc(Symbol static_type) {
  return Value(ValueKind::ACC, static_type);
}

Value Value::constant(int value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::constant(bool value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::constant(Symbol value, Symbol static_type) {
  return Value(ValueKind::CONSTANT, value, static_type);
}

Value Value::empty() { return Value(ValueKind::EMPTY, Symbol{}); }

//
// Accessors
//

bool Value::is_empty() const { return kind == ValueKind::EMPTY; }

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
  case Op::CALL:
    return "call";
  case Op::BRANCH:
    return "branch";
  case Op::LABEL:
    return "label";
  case Op::MOV:
    return "mov";
  case Op::ERROR:
    return "error";
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
    return "true";
  case BranchCondition::FALSE:
    return "false";
  }
}

/***********************
 *                     *
 *       Position      *
 *                     *
 **********************/

Position::Position(int i) : label_idx(i) {}

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
  printer.println("__undefined_instruction_print__");
  printer.exit();
}

bool Instruction::has_dest() {
  switch (op) {
  case Op::ADD:
  case Op::SUB:
  case Op::MULT:
  case Op::DIV:
  case Op::EQUAL:
  case Op::LESS_EQUAL:
  case Op::LESS_THAN:
  case Op::NEG:
  case Op::NOT:
  case Op::IS_VOID:
  case Op::NEW:
  case Op::CALL:
  case Op::MOV:
  case Op::TYPE_ID_OF:
  case Op::SUPERCLASS:
    return true;

  case Op::BRANCH:
  case Op::LABEL:
  case Op::ERROR:
    return false;
  }
}

Value &Instruction::get_dest() {
  fatal("INTERNAL: trying to get_dest in unsupported instruction");

  // the following is just to fool the linter
  return DUMMY_EMPTY_VALUE;
}

int Instruction::num_args() {
  switch (op) {
  case Op::ADD:
  case Op::SUB:
  case Op::MULT:
  case Op::DIV:
  case Op::EQUAL:
  case Op::LESS_EQUAL:
  case Op::LESS_THAN:
    return 2;

  case Op::NEG:
  case Op::NOT:
  case Op::IS_VOID:
  case Op::NEW:
  case Op::MOV:
  case Op::TYPE_ID_OF:
  case Op::SUPERCLASS:
  case Op::ERROR:
  case Op::BRANCH:
    return 1;

  case Op::CALL:
    // TODO(IT) consider how to deal with CALL arguments
    return 1;

  case Op::LABEL:
    return 0;
  }
}

Value &Instruction::get_arg1() {
  fatal("INTERNAL: trying to get_arg1 in unsupported instruction");

  // the following is just to fool the linter
  return DUMMY_EMPTY_VALUE;
}

Value &Instruction::get_arg2() {
  fatal("INTERNAL: trying to get_arg2 in unsupported instruction");

  // the following is just to fool the linter
  return DUMMY_EMPTY_VALUE;
}

//
// Unary
//

Unary::Unary(Op o, Value d, Value c, Token t)
    : dest(d), arg(c), Instruction(o, t) {}

void Unary::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format("{} {}, {}", hlir::to_string(op),
                              hlir::to_string(dest, symbols),
                              hlir::to_string(arg, symbols)));
  printer.exit();
}

Value &Unary::get_dest() { return dest; }

Value &Unary::get_arg1() { return arg; }

//
// New
//

New::New(Op o, Value d, Symbol ty, Token to)
    : dest(d), type(ty), Instruction(o, to) {}

void New::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format("{} {}, {}", hlir::to_string(op),
                              hlir::to_string(dest, symbols),
                              symbols.get_string((type))));
  printer.exit();
}

Value &New::get_dest() { return dest; }

//
// Binary
//

Binary::Binary(Op o, Value d, Value l, Value r, Token t)
    : dest(d), left(l), right(r), Instruction(o, t) {}

void Binary::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format(
      "{} {}, {}, {}", hlir::to_string(op), hlir::to_string(dest, symbols),
      hlir::to_string(left, symbols), hlir::to_string(right, symbols)));
  printer.exit();
}

Value &Binary::get_dest() { return dest; }

Value &Binary::get_arg1() { return left; }

Value &Binary::get_arg2() { return right; }

//
// Call
//

Call::Call(Value d, Value ta, Symbol n, Token to)
    : dest(d), target(ta), method_name(n), Instruction(Op::CALL, to) {}

void Call::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();

  // We will form this line out of several parts
  printer.beginln();
  printer.print(std::format(
      "{} {}, {}, {}, (", hlir::to_string(op), hlir::to_string(dest, symbols),
      hlir::to_string(target, symbols), symbols.get_string(method_name)));

  for (int i = 0; i < args.size(); i++) {
    const auto &arg = args[i];

    if (i > 0)
      printer.print(" ");

    printer.print(hlir::to_string(arg, symbols));
  }

  printer.print(")");
  printer.endln();

  printer.exit();
}

void Call::add_arg(const Value &arg) { args.push_back(arg); }

Value &Call::get_dest() { return dest; }

Value &Call::get_arg1() { return target; }

//
// Branch
//

Branch::Branch(BranchCondition bc, Value v, Position l, Token t)
    : value(v), condition(bc), target(l), Instruction(Op::BRANCH, t) {}

void Branch::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format(
      "{}.{} {} {}", hlir::to_string(op), hlir::to_string(condition),
      hlir::to_string(value, symbols), hlir::to_string(target)));
  printer.exit();
}

Value &Branch::get_arg1() { return value; }
//
// Label
//

Label::Label(int i, Symbol n, Token t)
    : idx(i), name(n), Instruction(Op::LABEL, t) {}

void Label::print(Printer printer, const SymbolTable &symbols) const {
  printer.println(std::format("{}: // {}", idx, symbols.get_string(name)));
}

//
// Mov
//

Mov::Mov(Value d, Value s, Token t)
    : dest(d), src(s), Instruction(Op::MOV, t) {}

void Mov::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format("{} {}, {}", hlir::to_string(op),
                              hlir::to_string(dest, symbols),
                              hlir::to_string(src, symbols)));
  printer.exit();
}

Value &Mov::get_dest() { return dest; }

Value &Mov::get_arg1() { return src; }

//
// Error
//

Error::Error(BranchCondition ec, Value v, runtime::Error re, Token t)
    : error(re), condition(ec), check(v), Instruction(Op::ERROR, t) {}

void Error::print(Printer printer, const SymbolTable &symbols) const {
  printer.enter();
  printer.println(std::format(
      "{}.{} {} {}", hlir::to_string(op), hlir::to_string(condition),
      hlir::to_string(check, symbols), runtime::to_string(error)));
  printer.exit();
}

Value &Error::get_arg1() { return check; }

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
  printer.println(std::format("{} {{", symbols.get_string(name)));

  for (const auto &instruction : instructions) {
    instruction->print(printer, symbols);
  }

  printer.println("}");
}

/***********************
 *                     *
 *        Class        *
 *                     *
 **********************/

Class::Class(Symbol n) : name(n) {}

void Class::print(Printer printer, const SymbolTable &symbols) const {
  printer.println(symbols.get_string(name));
  printer.println("{");

  printer.enter();

  printer.println("__initializer__ {");
  for (const auto &instruction : initializer) {
    instruction->print(printer, symbols);
  }
  printer.println("}");

  for (const auto &[_, method] : methods) {
    method.print(printer, symbols);
  }
  printer.exit();
  printer.println("}");
  printer.println("");
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
