#include "semantic.h"
#include "error.h"
#include <format>
#include <stdexcept>

/***********************
 *                     *
 *       VarInfo       *
 *                     *
 **********************/

VarInfo::VarInfo(Symbol t, Lifetime l) : type(t), lifetime(l) {}

VarInfo VarInfo::undefined() { return VarInfo(Symbol{}, Lifetime::UNDEFINED); }

bool VarInfo::is_undefined() const { return lifetime == Lifetime::UNDEFINED; }

/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/

Scopes::Scopes() {}

void Scopes::enter() { scopes.push_front(std::unordered_map<int, VarInfo>()); }
void Scopes::exit() { scopes.pop_front(); }

void Scopes::assign(Symbol name, Symbol type, Lifetime kind) {
  scopes.front().emplace(name.id, VarInfo(type, kind));
}

VarInfo Scopes::get(Symbol name) const {
  for (auto &scope : scopes) {
    if (scope.find(name.id) != scope.end()) {
      return scope.at(name.id);
    }
  }
  return VarInfo::undefined();
}

VarInfo Scopes::lookup(Symbol name) const {
  try {
    return scopes.front().at(name.id);
  } catch (const std::out_of_range &) {
    return VarInfo::undefined();
  }
}

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

ClassInfo::ClassInfo(ClassNode *cn, int d)
    : class_node(cn), depth_(d),
      methods_(std::unordered_map<int, MethodNode *>()),
      attributes_(std::unordered_map<int, AttributeNode *>()) {
  for (const auto &method_ptr : cn->methods) {
    Symbol name = method_ptr->name;
    MethodNode *raw_method_ptr = method_ptr.get();
    methods_[name.id] = raw_method_ptr;
  }

  for (const auto &attr_ptr : cn->attributes) {
    Symbol object_id = attr_ptr->object_id;
    AttributeNode *raw_attr_ptr = attr_ptr.get();
    attributes_[object_id.id] = raw_attr_ptr;
  }
}

unsigned int ClassInfo::start_line() const {
  return class_node->start_token.line();
}

unsigned int ClassInfo::start_column() const {
  return class_node->start_token.column();
}

int ClassInfo::depth() const { return depth_; }

Symbol ClassInfo::name() const { return class_node->name; }

Symbol ClassInfo::superclass() const { return class_node->superclass; }

MethodNode *ClassInfo::method(Symbol name) { return methods_[name.id]; }

AttributeNode *ClassInfo::attribute(Symbol name) {
  return attributes_[name.id];
}

std::vector<Symbol> ClassInfo::methods() const {
  std::vector<Symbol> methods;
  for (const auto &[_, mn] : methods_) {
    methods.push_back(mn->name);
  }
  return methods;
}

std::vector<Symbol> ClassInfo::attributes() const {
  std::vector<Symbol> attributes;
  for (const auto &[_, an] : attributes_) {
    attributes.push_back(an->object_id);
  }
  return attributes;
}

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/

TypeContext::TypeContext(Scopes &scps, Symbol cc, const ClassTree &ct,
                         SymbolTable &st)
    : scopes(scps), current_class(cc), tree(ct), symbols(st) {}

bool TypeContext::match(Symbol type_a, Symbol type_b) const {
  if (type_a == symbols.self_type)
    type_a = current_class;

  if (type_b == symbols.self_type)
    type_b = current_class;

  return tree.is_subclass(type_a, type_b);
}
void TypeContext::assign_attributes(Symbol class_name) {
  while (class_name != symbols.tree_root_type) {
    std::optional<ClassInfo> cls = tree.get(class_name);
    if (!cls.has_value())
      fatal(
          std::format(
              "INTERNAL: class {} could not be found in ClassTree after checks",
              symbols.get_string(class_name)),
          Token{});

    for (const Symbol attr : cls->attributes()) {
      AttributeNode *attr_ptr = cls->attribute(attr);

      if (!attr_ptr)
        fatal(std::format("INTERNAL: attribute {}.{} could not be found but it "
                          "is declared in the AST",
                          symbols.get_string(class_name),
                          symbols.get_string(attr)),
              Token{});

      scopes.assign(attr, attr_ptr->declared_type, Lifetime::ATTRIBUTE);
    }

    class_name = cls->superclass();
  }
}
