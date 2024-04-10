#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "lifetime.h"
#include "symbol.h"
#include <forward_list>

/***********************
 *                     *
 *       VarInfo       *
 *                     *
 **********************/

class VarInfo {
public:
  Symbol type;
  Lifetime lifetime;

  VarInfo(Symbol, Lifetime);

  static VarInfo undefined();

  bool is_undefined() const;
};

/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/

class Scopes {
private:
  std::forward_list<std::unordered_map<int, VarInfo>> scopes;

public:
  Scopes();
  void enter();
  void exit();

  void assign(Symbol name, Symbol type, Lifetime kind);
  /// Read the full scope to find the latest definition of a symbol
  VarInfo get(Symbol name) const;
  /// Check the definition of a symbol in the outermost scope
  VarInfo lookup(Symbol name) const;
};

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

typedef int ClassIdx;

class ClassInfo {
private:
  std::unordered_map<int, MethodNode *> methods_;
  std::unordered_map<int, AttributeNode *> attributes_;
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

  std::vector<Symbol> methods() const;
  std::vector<Symbol> attributes() const;
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
  SymbolTable &symbols;

  // Nodes for builtin classes
  std::unique_ptr<ClassNode> objectClassNode;
  std::unique_ptr<ClassNode> ioClassNode;
  std::unique_ptr<ClassNode> stringClassNode;
  std::unique_ptr<ClassNode> intClassNode;
  std::unique_ptr<ClassNode> boolClassNode;

  void check_class_hierarchy(const std::unordered_map<int, ClassNode *> &,
                             ModuleNode *);

  std::unique_ptr<MethodNode>
  make_builtin_method(Symbol cls, Symbol name, Symbol return_type,
                      std::vector<Symbol> parameter_names,
                      std::vector<Symbol> parameter_types);

  std::unordered_map<int, ClassNode *> get_class_node_map(ModuleNode *) const;
  std::vector<Symbol> get_classes_by_depth(ModuleNode *) const;

  void add_default_classes();
  void add_class(ClassNode *, int depth);

public:
  ClassTree(ModuleNode *, SymbolTable &);

  bool exists(Symbol name) const;
  bool exists(ClassIdx idx) const;

  std::optional<ClassInfo> get(Symbol name) const;
  std::optional<ClassInfo> get(ClassIdx idx) const;

  std::optional<ClassInfo> common_ancestor(ClassIdx node_a,
                                           ClassIdx node_b) const;
  std::optional<ClassInfo> common_ancestor(Symbol name_a, Symbol name_b) const;
  std::optional<ClassInfo> common_ancestor(const ClassInfo &class_a,
                                           const ClassInfo &class_b) const;

  bool is_subclass(ClassIdx node_a, ClassIdx node_b) const;
  bool is_subclass(Symbol name_a, Symbol name_b) const;
  bool is_subclass(const ClassInfo &class_a, const ClassInfo &class_b) const;

  MethodNode *get_method(Symbol class_name, Symbol method_name) const;
  AttributeNode *get_attribute(Symbol class_name, Symbol attribute_name) const;

  void print(std::ostream *out);
};

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/

class TypeContext {
public:
  TypeContext(Scopes &, Symbol current_class, const ClassTree &, SymbolTable &);

  Scopes &scopes;
  Symbol current_class;
  const ClassTree &tree;
  SymbolTable &symbols;

  bool match(Symbol type_a, Symbol type_b) const;

  VarInfo get_var(Symbol name) const;

  void assign_attributes(Symbol class_name);
};

#endif
