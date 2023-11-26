#include "parser.h"
#include "ast.h"
#include <error.h>

std::unique_ptr<ModuleNode> Parser::parse_module() { return nullptr; }
std::unique_ptr<ClassNode> Parser::parse_class() { return nullptr; }
std::unique_ptr<MethodNode> Parser::parse_method() { return nullptr; }
std::unique_ptr<AttributeNode> Parser::parse_attribute() { return nullptr; }

ExpressionPtr Parser::parse_expression() {
  Token token = tokens.next();
  if (token.type() == TokenType::NUMBER || token.type() == TokenType::STRING) {
    return std::make_unique<LiteralNode>(token);
  }

  fatal("Could not parse expression", token);
  return nullptr;
}

std::unique_ptr<AstNode> Parser::parse() { return parse_expression(); }
