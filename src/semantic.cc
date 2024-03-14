#include "semantic.h"
/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/
// TODO(IT) fill in
void Scopes::enter() {}
// TODO(IT) fill in
void Scopes::exit() {}
// TODO(IT) fill in
Symbol Scopes::get(Symbol name) { return Symbol{}; }
// TODO(IT) fill in
Symbol Scopes::lookup(Symbol name) { return Symbol{}; }
/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/
// TODO(IT) fill in initializing member maps
ClassInfo::ClassInfo(ClassNode *cn, int d)
    : class_node(cn), depth_(d),
      methods(std::unordered_map<int, MethodNode *>()),
      attributes(std::unordered_map<int, AttributeNode *>()) {}

int ClassInfo::depth() { return depth_; }

Symbol ClassInfo::superclass() { return class_node->superclass; }

// TODO(IT) fill in
MethodNode *ClassInfo::method(Symbol name) { return nullptr; }

// TODO(IT) fill in
AttributeNode *ClassInfo::attribute(Symbol name) { return nullptr; }

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
