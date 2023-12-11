#include "parser.h"
#include "ast.h"
#include "error.h"
#include <format>

/***********************
 *                     *
 *     Parser Utils    *
 *                     *
 **********************/

/// Skip Tokens until we find one of the appropriate type
void Parser::skip_until(TokenType type) {
  Token next_token = tokens.lookahead();
  while (next_token.type() != type && next_token.type() != TokenType::END) {
    tokens.next();
    next_token = tokens.lookahead();
  }
}

/// Consume a token, checking that it matches the expected type
bool Parser::expect(TokenType type) {
  Token token = tokens.next();
  return expect(token, type);
}

/// Check that a token matches some type and log an error if it doesn't.
bool Parser::expect(Token token, TokenType type) {
  if (token.type() != type) {
    error(std::format("Expected {}, but got {} {}", token_type_str(type),
                      token_type_str(token.type()),
                      symbols->get_string(token.symbol())),
          token);
    return false;
  }
  return true;
}

/***********************
 *                     *
 *  Top-level Parsers  *
 *                     *
 **********************/

std::unique_ptr<ModuleNode> Parser::parse_module() {
  Token next = tokens.lookahead();
  auto module_ = std::make_unique<ModuleNode>(next);

  while (next.type() != TokenType::END) {
    auto class_ = parse_class();
    module_->classes.push_back(std::move(class_));
    next = tokens.lookahead();
  }

  return module_;
}

/// Parse the beginning line of a class declaration
std::unique_ptr<ClassNode> Parser::parse_class_header() {
  Token start_token = tokens.next();
  if (start_token.type() != TokenType::KW_CLASS) {
    error("Expected class identifier", start_token);
    return parse_class();
  }

  Token token = tokens.next();
  Symbol class_name = token.symbol();
  if (token.type() != TokenType::TYPE_NAME) {
    fatal("Invalid class name", token);
    return nullptr;
  }

  Symbol parent_class = symbols->object_type;

  if (tokens.lookahead().type() == TokenType::KW_INHERITS) {
    tokens.next();
    Token parent_token = tokens.next();

    if (expect(parent_token, TokenType::TYPE_NAME)) {
      parent_class = parent_token.symbol();
    } else {
      skip_until(TokenType::L_BRACKET);
    }
  }

  if (!expect(TokenType::L_BRACKET)) {
    skip_until(TokenType::OBJECT_NAME);
  }

  return std::make_unique<ClassNode>(class_name, parent_class, start_token);
}

/// Parse one class definition
std::unique_ptr<ClassNode> Parser::parse_class() {
  std::unique_ptr<ClassNode> class_ = parse_class_header();

  // Get attributes and methods in a loop;
  Token lookahead, next_up;
  do {
    next_up = tokens.lookahead(1);

    switch (next_up.type()) {
    case TokenType::COLON:
      class_->attributes.push_back(parse_attribute());
      break;
    case TokenType::L_PAREN:
      class_->methods.push_back(parse_method());
      break;
    default:
      error("Expected : in attribute definition", next_up);
      skip_until(TokenType::SEMICOLON);
      tokens.next();
    }
  } while (tokens.lookahead().type() != TokenType::R_BRACKET);

  expect(TokenType::R_BRACKET);
  expect(TokenType::SEMICOLON);

  return class_;
}

/// Parse a method definition in a class
std::unique_ptr<MethodNode> Parser::parse_method() {
  Token method_name = tokens.next();

  if (!expect(method_name, TokenType::OBJECT_NAME)) {
    skip_until(TokenType::R_BRACKET);
    skip_until(TokenType::SEMICOLON);
    tokens.next();
    return nullptr;
  };

  // (
  expect(TokenType::L_PAREN);

  std::vector<std::unique_ptr<ParameterNode>> parameters;
  while (tokens.lookahead().type() != TokenType::R_PAREN) {
    Token object_name = tokens.next();
    // object
    expect(object_name, TokenType::OBJECT_NAME);

    // :
    expect(TokenType::COLON);

    // Type
    Token type_name = tokens.next();
    expect(type_name, TokenType::TYPE_NAME);

    // ,
    if (tokens.lookahead().type() == TokenType::COMMA) {
      tokens.next();
    }

    parameters.push_back(std::make_unique<ParameterNode>(
        object_name.symbol(), type_name.symbol(), object_name));
  }

  // )
  expect(TokenType::R_PAREN);

  // :
  expect(TokenType::COLON);

  // Return type
  Token return_type = tokens.next();
  expect(return_type, TokenType::TYPE_NAME);

  // {
  expect(TokenType::L_BRACKET);

  // method body
  ExpressionPtr expr = parse_expression();

  // }
  expect(TokenType::R_BRACKET);
  // ;
  expect(TokenType::SEMICOLON);

  return std::make_unique<MethodNode>(
      method_name.symbol(), return_type.symbol(), std::move(parameters),
      std::move(expr), method_name);
}

/// Parse an attribute defintion
std::unique_ptr<AttributeNode> Parser::parse_attribute() {
  // object id
  Token start_token = tokens.next();
  if (!expect(start_token, TokenType::OBJECT_NAME)) {
    skip_until(TokenType::SEMICOLON);
    return nullptr;
  }

  // :
  if (!expect(TokenType::COLON)) {
    skip_until(TokenType::SEMICOLON);
    return nullptr;
  }

  // type id
  Token type_token = tokens.next();
  if (!expect(type_token, TokenType::TYPE_NAME)) {
    skip_until(TokenType::SEMICOLON);
    return nullptr;
  }

  // either ; or <-
  Token next = tokens.next();
  ExpressionPtr expr;

  switch (next.type()) {
  case TokenType::SEMICOLON:
    return std::make_unique<AttributeNode>(start_token.symbol(),
                                           type_token.symbol(), start_token);
  case TokenType::ASSIGN:
    expr = parse_expression();
    expect(TokenType::SEMICOLON);
    return std::make_unique<AttributeNode>(start_token.symbol(),
                                           type_token.symbol(), std::move(expr),
                                           start_token);
  default:
    error("Expected : or <-", next);
    skip_until(TokenType::SEMICOLON);
  }
  return nullptr;
}

/***********************
 *                     *
 * Expression parsers  *
 *                     *
 **********************/

bool is_expression_end(TokenType type) {
  switch (type) {
  case TokenType::SEMICOLON:
  case TokenType::R_BRACKET:
  case TokenType::R_PAREN:
  case TokenType::KW_CLASS:
    return true;
  default:
    return false;
  }
}

ExpressionPtr Parser::parse_expression_atom() {
  Token token = tokens.next();

  // This second token will only be filled for expressions that need it
  Token second_token;

  switch (token.type()) {
  case TokenType::NUMBER:
  case TokenType::STRING:
  case TokenType::KW_TRUE:
  case TokenType::KW_FALSE:
    return std::make_unique<LiteralNode>(token);
  case TokenType::OBJECT_NAME:
    return std::make_unique<VariableNode>(token);
  case TokenType::SIMPLE_OP:
    return std::make_unique<BinaryOpNode>(token.symbol(), token);
  case TokenType::NEG_OP:
  case TokenType::KW_NOT:
  case TokenType::KW_ISVOID:
    return std::make_unique<UnaryOpNode>(token.symbol(), token);
  case TokenType::KW_NEW:
    second_token = tokens.next();
    expect(second_token.type(), TokenType::TYPE_NAME);
    return std::make_unique<NewNode>(second_token.symbol(), token);
  case TokenType::L_PAREN:
    return parse_parenthesised_expression();
  case TokenType::DOT:
    return parse_dynamic_dispatch();
  case TokenType::AT:
    return parse_static_dispatch();
  default:
    error("Could not parse expression", token);
    return nullptr;
  }
}

ExpressionPtr Parser::parse_expression() {
  std::vector<ExpressionPtr> node_stack;
  Token lookahead = tokens.lookahead();
  // TODO: parse most kinds of expressions
  while (!is_expression_end(lookahead.type()) || node_stack.size() > 1) {
    if (!reduce_stack(node_stack, lookahead)) {
      node_stack.push_back(parse_expression_atom());
      lookahead = tokens.lookahead();
    }
  }
  if (node_stack.size() != 1) {
    error("could not parse expression nearby", lookahead);
  }
  return std::move(node_stack.back());
}

ExpressionPtr Parser::parse_object_expression(Token object_token) {
  if (tokens.lookahead().type() != TokenType::L_PAREN)
    return std::make_unique<VariableNode>(object_token);

  std::unique_ptr<DispatchNode> dispatch = std::make_unique<DispatchNode>(
      object_token.symbol(), parse_dispatch_args(), object_token);
  dispatch->set_target_to_self();

  return dispatch;
}

ExpressionPtr Parser::parse_parenthesised_expression() {
  ExpressionPtr expr = parse_expression();
  expect(TokenType::R_PAREN);
  return expr;
}

std::vector<ExpressionPtr> Parser::parse_dispatch_args() {
  expect(TokenType::L_PAREN);

  std::vector<ExpressionPtr> args;
  Token lookahead = tokens.lookahead();
  while (!is_expression_end(lookahead.type())) {
    args.push_back(parse_expression());
    lookahead = tokens.lookahead();
    if (lookahead.type() == TokenType::COMMA)
      tokens.next();
  }

  expect(TokenType::R_PAREN);
  return args;
}

std::unique_ptr<DispatchNode> Parser::parse_dynamic_dispatch() {
  Token method_token = tokens.next();
  expect(method_token.type(), TokenType::OBJECT_NAME);

  return std::make_unique<DispatchNode>(method_token.symbol(),
                                        parse_dispatch_args(), method_token);
}

std::unique_ptr<DispatchNode> Parser::parse_static_dispatch() {
  Token type_token = tokens.next();
  expect(type_token.type(), TokenType::TYPE_NAME);
  expect(type_token.type(), TokenType::DOT);

  std::unique_ptr<DispatchNode> dispatch = parse_dynamic_dispatch();
  dispatch->static_type = type_token.symbol();

  return dispatch;
}
/***********************
 *                     *
 *      Reducers       *
 *                     *
 **********************/

bool Parser::reduce_stack(std::vector<ExpressionPtr> &node_stack,
                          Token lookahead) {
  // TODO(IT): consider precedence
  int stack_size = node_stack.size();
  if (stack_size > 1) {
    if (node_stack[stack_size - 1]->arity() == 2 &&
        node_stack[stack_size - 2]->arity() == 0) {
      ExpressionPtr op = std::move(node_stack[stack_size - 1]);
      op->add_child(node_stack[stack_size - 2]);
      node_stack.pop_back();
      node_stack.pop_back();
      node_stack.push_back(std::move(op));
      return true;
    }

    if (node_stack[stack_size - 1]->arity() == 0 &&
        node_stack[stack_size - 2]->arity() == 1) {
      node_stack[stack_size - 2]->add_child(node_stack[stack_size - 1]);
      node_stack.pop_back();
      return true;
    }
  }
  return false;
}

/***********************
 *                     *
 *      Exported       *
 *                     *
 **********************/

std::unique_ptr<ModuleNode> Parser::parse() { return parse_module(); }
