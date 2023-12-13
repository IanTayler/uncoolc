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

bool is_class_end(TokenType type) {
  switch (type) {
  case TokenType::SEMICOLON:
  case TokenType::R_BRACKET:
  case TokenType::R_PAREN:
  case TokenType::KW_CLASS:
  case TokenType::END:
  case TokenType::INVALID:
    return true;
  default:
    return false;
  }
}

bool is_expression_end(TokenType type) {
  switch (type) {
  case TokenType::SEMICOLON:
  case TokenType::COMMA:
  case TokenType::R_BRACKET:
  case TokenType::R_PAREN:
  case TokenType::KW_CLASS:
  case TokenType::END:
  case TokenType::INVALID:
  case TokenType::KW_THEN:
  case TokenType::KW_ELSE:
  case TokenType::KW_FI:
  case TokenType::KW_POOL:
  case TokenType::KW_LOOP:
    return true;
  default:
    return false;
  }
}

void dump_node_stack(std::vector<ExpressionPtr> &node_stack,
                     std::shared_ptr<SymbolTable> symbols) {
  AstPrinter printer{2, &std::cerr};
  printer.print("-- node_stack dump --");

  for (auto &node : node_stack) {
    node->print(printer, symbols);
  }
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
  if (!expect(start_token, TokenType::KW_CLASS)) {
    skip_until(TokenType::KW_CLASS);
    start_token = tokens.next();
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
    expect(TokenType::SEMICOLON);
  } while (!is_class_end(tokens.lookahead().type()));

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
  Token lookahead = tokens.lookahead();
  ExpressionPtr expr;

  switch (lookahead.type()) {
  case TokenType::SEMICOLON:
  case TokenType::COMMA:
    return std::make_unique<AttributeNode>(start_token.symbol(),
                                           type_token.symbol(), start_token);
  case TokenType::ASSIGN:
    tokens.next(); // Consume <-
    expr = parse_expression();
    return std::make_unique<AttributeNode>(start_token.symbol(),
                                           type_token.symbol(), std::move(expr),
                                           start_token);
  default:
    error("Expected ';', ',' or '<-'", lookahead);
    skip_until(TokenType::SEMICOLON);
  }
  return nullptr;
}

/***********************
 *                     *
 * Operator attributes *
 *                     *
 **********************/

inline int Parser::op_precedence(Token t) const {
  Symbol s = t.symbol();

  if (s == symbols->not_kw || s == symbols->neg_op)
    return 10;
  if (s == symbols->mult_op || s == symbols->div_op)
    return 8;
  if (s == symbols->add_op || s == symbols->sub_op)
    return 6;
  if (s == symbols->leq_op || s == symbols->lt_op || s == symbols->eq_op)
    return 4;
  if (s == symbols->assign_op)
    return 2;
  if (s == symbols->isvoid_kw)
    return 1;

  return 0;
}

inline bool Parser::takes_left(Token t) const {
  if (t.type() == TokenType::SIMPLE_OP)
    return true;
  return false;
}

/***********************
 *                     *
 * Expression parsers  *
 *                     *
 **********************/

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
    return parse_object_expression(token);
  case TokenType::SIMPLE_OP:
    return std::make_unique<BinaryOpNode>(token);
  case TokenType::ASSIGN:
    return std::make_unique<AssignNode>(token);
  case TokenType::NEG_OP:
  case TokenType::KW_NOT:
  case TokenType::KW_ISVOID:
    return std::make_unique<UnaryOpNode>(token);
  case TokenType::KW_NEW:
    second_token = tokens.next();
    expect(second_token.type(), TokenType::TYPE_NAME);
    return std::make_unique<NewNode>(second_token.symbol(), token);
  case TokenType::L_PAREN:
    return parse_parenthesised_expression();
  case TokenType::L_BRACKET:
    return parse_block(token);
  case TokenType::KW_IF:
    return parse_if(token);
  case TokenType::KW_WHILE:
    return parse_while(token);
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
      // Failed reduction at the end of an expression will continue to fail
      if (is_expression_end(lookahead.type())) {
        // Dump the unreducible node stack before fatal message
        dump_node_stack(node_stack, symbols);
        fatal("Could not reduce expression", lookahead);
      }

      ExpressionPtr expr = parse_expression_atom();
      if (expr)
        node_stack.push_back(std::move(expr));

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
  while (!is_expression_end(tokens.lookahead().type())) {
    args.push_back(parse_expression());
    if (tokens.lookahead().type() == TokenType::COMMA) {
      tokens.next();
    }
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

std::unique_ptr<BlockNode> Parser::parse_block(Token start_token) {
  auto block = std::make_unique<BlockNode>(start_token);
  while (!is_class_end(tokens.lookahead().type())) {
    block->add_expression(parse_expression());
    expect(TokenType::SEMICOLON);
  }
  expect(TokenType::R_BRACKET);
  return std::move(block);
}

std::unique_ptr<IfNode> Parser::parse_if(Token start_token) {
  ExpressionPtr cond_expr = parse_expression();

  expect(TokenType::KW_THEN);
  ExpressionPtr then_expr = parse_expression();

  expect(TokenType::KW_ELSE);
  ExpressionPtr else_expr = parse_expression();

  expect(TokenType::KW_FI);

  return std::make_unique<IfNode>(cond_expr, then_expr, else_expr, start_token);
}

std::unique_ptr<WhileNode> Parser::parse_while(Token start_token) {
  ExpressionPtr cond_expr = parse_expression();

  expect(TokenType::KW_LOOP);
  ExpressionPtr body_expr = parse_expression();

  expect(TokenType::KW_POOL);
  return std::make_unique<WhileNode>(cond_expr, body_expr, start_token);
}
/***********************
 *                     *
 *      Reducers       *
 *                     *
 **********************/

bool Parser::reduce_stack(std::vector<ExpressionPtr> &node_stack,
                          Token lookahead) {
  int stack_size = node_stack.size();
  if (stack_size > 1) {
    ExpressionPtr &top = node_stack[stack_size - 1];
    ExpressionPtr &second = node_stack[stack_size - 2];

    if (top->arity() > 0 && top->child_side() == ChildSide::LEFT &&
        second->arity() == 0) {
      // preserve top;
      ExpressionPtr top_node = std::move(top);
      top_node->add_child(second);
      node_stack.pop_back();
      node_stack.pop_back();
      node_stack.push_back(std::move(top_node));
      return true;
    }

    if (top->arity() == 0 && second->arity() > 0 &&
        second->child_side() == ChildSide::RIGHT) {
      // TODO(IT): consider associativity
      int second_precedence = op_precedence(second->start_token);
      bool is_competing = takes_left(lookahead);
      if (!is_competing || second_precedence >= op_precedence(lookahead)) {
        second->add_child(top);
        node_stack.pop_back();
        return true;
      }
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
