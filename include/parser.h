#ifndef _PARSER_H
#define _PARSER_H

#include "ast.h"

enum class Associativity {
  LEFT,
  RIGHT,
};

class Parser {
private:
  bool has_error_;

  TokenStream &tokens;
  const SymbolTable &symbols;

  bool expect(TokenType);
  bool expect(Token, TokenType);
  void skip_until(TokenType type);

  std::unique_ptr<ModuleNode> parse_module();
  std::unique_ptr<ClassNode> parse_class_header();
  std::unique_ptr<ClassNode> parse_class();
  std::unique_ptr<MethodNode> parse_method();
  std::unique_ptr<AttributeNode> parse_attribute();

  // Expression parsers
  ExpressionPtr parse_expression();
  ExpressionPtr parse_expression_atom();
  ExpressionPtr parse_object_expression(Token);
  ExpressionPtr parse_parenthesised_expression();
  std::unique_ptr<BlockNode> parse_block(Token);
  std::unique_ptr<IfNode> parse_if(Token);
  std::unique_ptr<WhileNode> parse_while(Token);
  std::unique_ptr<LetNode> parse_let(Token);
  std::unique_ptr<CaseBranchNode> parse_case_branch();
  std::unique_ptr<CaseNode> parse_case(Token);

  std::vector<ExpressionPtr> parse_dispatch_args();
  std::unique_ptr<DispatchNode> parse_dynamic_dispatch();
  std::unique_ptr<DispatchNode> parse_static_dispatch();

  // Helpers for expression parsers
  inline int op_precedence(Token) const;
  inline Associativity op_associativity(Token) const;
  inline bool takes_left(Token) const;
  // Reducers
  bool reduce_stack(std::vector<ExpressionPtr> &node_stack, Token lookahead);

  void parser_error(std::string, Token);

public:
  Parser(TokenStream &ts, const SymbolTable &ss)
      : tokens(ts), symbols(ss), has_error_(false) {}

  bool get_error();

  std::unique_ptr<ModuleNode> parse();
};

#endif
