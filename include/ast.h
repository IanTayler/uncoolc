#ifndef _AST_H
#define _AST_H

#include "hlir.h"
#include "lifetime.h"
#include "printer.h"
#include "symbol.h"
#include "token.h"
#include <optional>
#include <vector>

/***********************
 *                     *
 *      ChildSide      *
 *                     *
 **********************/

/// Marks where an incomplete expression expects a child in parsing
enum class ChildSide {
  LEFT,
  RIGHT,
  NONE,
};

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

class TypeContext;

class AstNode {
public:
  AstNode(Token st) : start_token(st) {}
  Token start_token;

  virtual void print(Printer printer, const SymbolTable &symbols);
  /// Will typecheck and annotate the type of Expressions. Returns whether types
  /// are consistent.
  virtual bool typecheck(TypeContext &) = 0;
};

class ExpressionNode : public AstNode {
public:
  ExpressionNode(Token st) : AstNode(st) {}
  std::optional<Symbol> static_type;

  virtual void print(Printer printer, const SymbolTable &symbols) override;
  void print_type(Printer printer, const SymbolTable &symbols);

  virtual bool typecheck(TypeContext &) override;

  virtual hlir::InstructionList to_hlir(hlir::Context &) const;

  virtual int arity();
  virtual ChildSide child_side();
  virtual void add_child(std::unique_ptr<ExpressionNode> &new_child);
};

typedef std::unique_ptr<ExpressionNode> ExpressionPtr;

class AttributeNode : public AstNode {
private:
  bool typecheck_inheritance(const TypeContext &) const;

public:
  AttributeNode(Symbol v, Symbol ty, Token st)
      : object_id(v), declared_type(ty), AstNode(st) {}

  AttributeNode(Symbol v, Symbol ty, ExpressionPtr ex, Token st)
      : object_id(v), declared_type(ty), initializer(std::move(ex)),
        AstNode(st) {}

  Symbol object_id;
  Symbol declared_type;
  std::optional<ExpressionPtr> initializer;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;
};

class ParameterNode : public AstNode {
public:
  ParameterNode(Symbol o, Symbol dt, Token st)
      : object_id(o), declared_type(dt), AstNode(st) {}

  Symbol object_id;
  Symbol declared_type;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;
};

class MethodNode : public AstNode {
private:
  bool typecheck_inheritance(const TypeContext &) const;

public:
  MethodNode(Symbol n, Symbol rt,
             std::vector<std::unique_ptr<ParameterNode>> ps, ExpressionPtr b,
             Token st)
      : name(n), return_type(rt), parameters(std::move(ps)), body(std::move(b)),
        AstNode(st) {}

  Symbol name;
  Symbol return_type;

  std::vector<std::unique_ptr<ParameterNode>> parameters;
  ExpressionPtr body;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::Method to_hlir_method(SymbolTable &) const;
};

class ClassNode : public AstNode {
public:
  ClassNode(Symbol n, Symbol sc, Token st)
      : name(n), superclass(sc), AstNode(st) {}
  Symbol name;
  Symbol superclass;

  std::vector<std::unique_ptr<AttributeNode>> attributes;
  std::vector<std::unique_ptr<MethodNode>> methods;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::Class to_hlir_class(SymbolTable &) const;
};

class ModuleNode : public AstNode {
public:
  ModuleNode(Token st) : AstNode(st) {}

  std::vector<std::unique_ptr<ClassNode>> classes;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::Universe to_hlir_universe(SymbolTable &) const;
};

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

class BuiltinNode : public ExpressionNode {
private:
  Symbol class_name;
  Symbol method_name;

public:
  BuiltinNode(Symbol cn, Symbol mn)
      : class_name(cn), method_name(mn), ExpressionNode(Token{}) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class LiteralNode : public ExpressionNode {
private:
  Symbol value;

public:
  LiteralNode(Token t) : value(t.symbol()), ExpressionNode(t) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class VariableNode : public ExpressionNode {
private:
  Symbol name;
  Lifetime lifetime;

public:
  VariableNode(Token t)
      : name(t.symbol()), lifetime(Lifetime::UNKNOWN), ExpressionNode(t) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

class UnaryOpNode : public ExpressionNode {
public:
  UnaryOpNode(Token st) : op(st.symbol()), ExpressionNode(st) {}

  UnaryOpNode(ExpressionPtr ch, Token st)
      : op(st.symbol()), child(std::move(ch)), ExpressionNode(st) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;

  virtual int arity() override;
  virtual void add_child(std::unique_ptr<ExpressionNode> &new_child) override;
  virtual ChildSide child_side() override;

private:
  ExpressionPtr child;
  Symbol op;
};

class BinaryOpNode : public ExpressionNode {
public:
  BinaryOpNode(Token st)
      : left(nullptr), op(st.symbol()), right(nullptr), ExpressionNode(st) {}

  BinaryOpNode(ExpressionPtr l, ExpressionPtr r, Token st)
      : left(std::move(l)), op(st.symbol()), right(std::move(r)),
        ExpressionNode(st) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;

  virtual int arity() override;
  virtual void add_child(std::unique_ptr<ExpressionNode> &new_child) override;
  virtual ChildSide child_side() override;

private:
  ExpressionPtr left;
  Symbol op;
  ExpressionPtr right;
};

class NewNode : public ExpressionNode {
private:
  Symbol created_type;

public:
  NewNode(Symbol c, Token s) : created_type(c), ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class AssignNode : public ExpressionNode {
private:
  Symbol variable;
  Lifetime lifetime;
  ExpressionPtr expression;

public:
  AssignNode(Token s) : lifetime(Lifetime::UNKNOWN), ExpressionNode(s) {}

  virtual int arity() override;
  virtual void add_child(std::unique_ptr<ExpressionNode> &new_child) override;
  virtual ChildSide child_side() override;

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class DispatchNode : public ExpressionNode {
private:
  /// Set to true when the dispatch is known to have no target
  bool target_self;

  ExpressionPtr target;

  Symbol method;
  std::optional<Symbol> dispatch_type;
  std::vector<ExpressionPtr> arguments;

public:
  DispatchNode(std::optional<Symbol> dt, Symbol m,
               std::vector<ExpressionPtr> args, Token s)
      : target(nullptr), method(m), dispatch_type(dt),
        arguments(std::move(args)), target_self(false), ExpressionNode(s) {}
  DispatchNode(Symbol m, std::vector<ExpressionPtr> args, Token s)
      : DispatchNode(std::nullopt, m, std::move(args), s) {}

  virtual int arity() override;
  virtual void add_child(std::unique_ptr<ExpressionNode> &new_child) override;
  virtual ChildSide child_side() override;

  void set_dispatch_type(Symbol d) { dispatch_type = d; }
  void set_target_to_self() { target_self = true; }
  bool has_self_target() { return target_self; }

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

class BlockNode : public ExpressionNode {
private:
  std::vector<ExpressionPtr> expressions;

public:
  BlockNode(Token s) : ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;

  void add_expression(ExpressionPtr expr) {
    expressions.push_back(std::move(expr));
  }
};

class IfNode : public ExpressionNode {
private:
  ExpressionPtr condition_expr;
  ExpressionPtr then_expr;
  ExpressionPtr else_expr;

public:
  IfNode(ExpressionPtr &c, ExpressionPtr &t, ExpressionPtr &e, Token s)
      : condition_expr(std::move(c)), then_expr(std::move(t)),
        else_expr(std::move(e)), ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class WhileNode : public ExpressionNode {
private:
  ExpressionPtr condition_expr;
  ExpressionPtr body_expr;

public:
  WhileNode(ExpressionPtr &c, ExpressionPtr &b, Token s)
      : condition_expr(std::move(c)), body_expr(std::move(b)),
        ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class LetNode : public ExpressionNode {
private:
  std::vector<std::unique_ptr<AttributeNode>> declarations;
  ExpressionPtr body_expr;

public:
  LetNode(Token s) : body_expr(nullptr), ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;

  void add_declaration(std::unique_ptr<AttributeNode> attr) {
    declarations.push_back(std::move(attr));
  }

  void set_body(ExpressionPtr expr) { body_expr = std::move(expr); }
};

class CaseBranchNode : public ExpressionNode {
public:
  Symbol object_id;
  Symbol declared_type;
  ExpressionPtr body_expr;

  CaseBranchNode(Symbol o, Symbol t, ExpressionPtr b, Token s)
      : object_id(o), declared_type(t), body_expr(std::move(b)),
        ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;
};

class CaseNode : public ExpressionNode {
private:
  ExpressionPtr eval_expr;
  std::vector<std::unique_ptr<CaseBranchNode>> branches;

public:
  CaseNode(ExpressionPtr e, Token s)
      : eval_expr(std::move(e)), ExpressionNode(s) {}

  void print(Printer printer, const SymbolTable &symbols) override;

  bool typecheck(TypeContext &) override;

  hlir::InstructionList to_hlir(hlir::Context &) const override;

  void add_branch(std::unique_ptr<CaseBranchNode> branch) {
    branches.push_back(std::move(branch));
  }
};

#endif
