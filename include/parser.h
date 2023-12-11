#ifndef _PARSER_H
#define _PARSER_H

#include "ast.h"

class Parser {
private:
  TokenStream &tokens;
  std::shared_ptr<SymbolTable> symbols;

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
  ExpressionPtr parse_parenthesised_expression();
  std::unique_ptr<DispatchNode> parse_dynamic_dispatch();
  std::unique_ptr<DispatchNode> parse_static_dispatch();

  // Reducers
  bool reduce_stack(std::vector<ExpressionPtr> &node_stack, Token lookahead);

public:
  Parser(TokenStream &ts, std::shared_ptr<SymbolTable> ss)
      : tokens(ts), symbols(ss) {}

  std::unique_ptr<ModuleNode> parse();
};

#endif
