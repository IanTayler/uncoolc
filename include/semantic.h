#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"
#include <forward_list>

/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/

class Scopes {
private:
  std::forward_list<std::unordered_map<Symbol, Symbol>> scopes;

  void enter();
  void exit();

  void assign(Symbol name, Symbol type);
  /// Read the full scope to find the latest definition of a symbol
  Symbol get(Symbol name);
  /// Check the definition of a symbol in the outermost scope
  Symbol lookup(Symbol name);
};

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

typedef int NodeIdx;

class ClassInfo {
private:
  std::unordered_map<Symbol, MethodNode *> methods;
  std::unordered_map<Symbol, AttributeNode *> attributes;
  ClassNode *class_node;
  int depth_;

public:
  ClassInfo(ClassNode *cn, int d);

  int depth();
  Symbol superclass();
  MethodNode *method(Symbol name);
  AttributeNode *attribute(Symbol name);
};

/***********************
 *                     *
 *      ClassTree      *
 *                     *
 **********************/

class ClassTree {
private:
  std::vector<ClassInfo> classes;
  std::unordered_map<Symbol, NodeIdx> classes_by_name;

public:
  ClassInfo get(Symbol name);
  ClassInfo get(NodeIdx idx);

  ClassInfo common_ancestor(NodeIdx node_a, NodeIdx node_b);
  ClassInfo common_ancestor(Symbol name_a, Symbol name_b);
};

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/

class TypeContext {
public:
  Scopes scopes;
  Symbol current_class;
  ClassTree tree;
};

#endif
