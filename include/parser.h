#ifndef _PARSER_H
#define _PARSER_H

#include "ast.h"

class Parser {
private:
  TokenStream &tokens;
  std::shared_ptr<SymbolTable> symbols;

  std::unique_ptr<ModuleNode> parse_module();
  std::unique_ptr<ClassNode> parse_class();
  std::unique_ptr<MethodNode> parse_method();
  std::unique_ptr<AttributeNode> parse_attribute();
  ExpressionPtr parse_expression();

public:
  Parser(TokenStream &ts, std::shared_ptr<SymbolTable> ss)
      : tokens(ts), symbols(ss) {}

  std::unique_ptr<AstNode> parse();
};

#endif
