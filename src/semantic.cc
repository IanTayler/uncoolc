#include "semantic.h"
#include <stdexcept>

/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/

Scopes::Scopes() {}

void Scopes::enter() { scopes.push_front(std::unordered_map<int, Symbol>()); }
void Scopes::exit() { scopes.pop_front(); }

void Scopes::assign(Symbol name, Symbol type) {
  scopes.front()[name.id] = type;
}

Symbol Scopes::get(Symbol name) const {
  for (auto &scope : scopes) {
    if (scope.find(name.id) != scope.end()) {
      return scope.at(name.id);
    }
  }
  return Symbol{};
}

Symbol Scopes::lookup(Symbol name) const {
  try {
    return scopes.front().at(name.id);
  } catch (const std::out_of_range &) {
    return Symbol{};
  }
}

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

ClassInfo::ClassInfo(ClassNode *cn, int d)
    : class_node(cn), depth_(d),
      methods(std::unordered_map<int, MethodNode *>()),
      attributes(std::unordered_map<int, AttributeNode *>()) {
  for (const auto &method_ptr : cn->methods) {
    Symbol name = method_ptr->name;
    MethodNode *raw_method_ptr = method_ptr.get();
    methods[name.id] = raw_method_ptr;
  }

  for (const auto &attr_ptr : cn->attributes) {
    Symbol object_id = attr_ptr->object_id;
    AttributeNode *raw_attr_ptr = attr_ptr.get();
    attributes[object_id.id] = raw_attr_ptr;
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

MethodNode *ClassInfo::method(Symbol name) { return methods[name.id]; }

AttributeNode *ClassInfo::attribute(Symbol name) { return attributes[name.id]; }

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/

// TODO(IT) take class hierarchies into consideration
bool TypeContext::match(Symbol type_a, Symbol type_b) const {
  if (type_a == type_b)
    return true;
  if (type_a == symbols.self_type && type_b == current_class)
    return true;

  return false;
}
