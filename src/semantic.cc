#include "semantic.h"
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

Symbol Scopes::get(Symbol name) {
  for (auto &scope : scopes) {
    if (scope.find(name.id) != scope.end()) {
      return scope[name.id];
    }
  }
  return Symbol{};
}

Symbol Scopes::lookup(Symbol name) { return scopes.front()[name.id]; }

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

int ClassInfo::depth() { return depth_; }

Symbol ClassInfo::name() { return class_node->name; }

Symbol ClassInfo::superclass() { return class_node->superclass; }

MethodNode *ClassInfo::method(Symbol name) { return methods[name.id]; }

AttributeNode *ClassInfo::attribute(Symbol name) { return attributes[name.id]; }

/***********************
 *                     *
 *      ClassTree      *
 *                     *
 **********************/
// TODO(IT) fill in
ClassInfo ClassTree::get(Symbol name) { return ClassInfo(nullptr, 0); }

// TODO(IT) fill in
ClassInfo ClassTree::get(NodeIdx idx) { return ClassInfo(nullptr, 0); }

// TODO(IT) fill in
ClassInfo ClassTree::common_ancestor(NodeIdx node_a, NodeIdx node_b) {
  return ClassInfo(nullptr, 0);
}

// TODO(IT) fill in
ClassInfo ClassTree::common_ancestor(Symbol name_a, Symbol name_b) {
  return ClassInfo(nullptr, 0);
}

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/
// TODO(IT) fill in
bool TypeContext::match(Symbol type_a, Symbol type_b) { return true; }

/***********************
 *                     *
 *     typechecks      *
 *                     *
 **********************/

// TODO(IT) fill in
bool AstNode::typecheck(TypeContext context) { return true; }
