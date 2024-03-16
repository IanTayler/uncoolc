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
  std::forward_list<std::unordered_map<int, Symbol>> scopes;

public:
  Scopes();
  void enter();
  void exit();

  void assign(Symbol name, Symbol type);
  /// Read the full scope to find the latest definition of a symbol
  Symbol get(Symbol name) const;
  /// Check the definition of a symbol in the outermost scope
  Symbol lookup(Symbol name) const;
};

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

typedef int ClassIdx;

class ClassInfo {
private:
  std::unordered_map<int, MethodNode *> methods;
  std::unordered_map<int, AttributeNode *> attributes;
  ClassNode *class_node;
  int depth_;

public:
  ClassInfo(ClassNode *cn, int d);

  unsigned int start_line() const;
  unsigned int start_column() const;
  int depth() const;
  Symbol name() const;
  Symbol superclass() const;

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
  std::unordered_map<int, ClassIdx> classes_by_name;

public:

  bool exists(Symbol name) const;
  bool exists(ClassIdx idx) const;

  std::optional<ClassInfo> get(Symbol name) const;
  std::optional<ClassInfo> get(ClassIdx idx) const;

  std::optional<ClassInfo> common_ancestor(ClassIdx node_a,
                                           ClassIdx node_b) const;
  std::optional<ClassInfo> common_ancestor(Symbol name_a, Symbol name_b) const;
  std::optional<ClassInfo> common_ancestor(const ClassInfo &class_a,
                                           const ClassInfo &class_b) const;
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
  const ClassTree &tree;
  const SymbolTable &symbols;

  bool match(Symbol type_a, Symbol type_b) const;
};

#endif
