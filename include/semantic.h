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
public:
  Symbol name;
  int depth;
  Symbol parent;
  std::vector<MethodNode *> methods;
  std::vector<AttributeNode *> attributes;
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
