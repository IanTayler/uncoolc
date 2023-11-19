#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
  // Symbols
  L_BRACKET,
  R_BRACKET,
  L_SQ_BRACKET,
  R_SQ_BRACKET,
  SEMICOLON,
  // Parentheses or comment tokens
  L_PAREN_CLASS,
  L_PAREN,
  OPEN_COMMENT,
  R_PAREN_CLASS,
  R_PAREN,
  CLOSE_COMMENT,
  // Operators
  MINUS_OP_CLASS, // <- (ASSIGN), < and <= (SIMPLE_OPs)
  ASSIGN,
  EQ_OP_CLASS,   // = (SIMPLE_OP) and => (ARROW)
  ARROW,
  SIMPLE_OP,
  // IDs
  OBJECT_NAME,
  TYPE_NAME,
  // Literals
  NUMBER,
  STRING,
  // Keywords
  KW_IF,
  KW_FI,
  KW_THEN,
  KW_ELSE,
  KW_FOR, // TODO(IT) not really a thing
  KW_WHILE,
  KW_CASE,
  KW_ESAC,
  KW_END,
  KW_LOOP,
  KW_POOL,
  // Misc
  SPACE,
  NEW_LINE,
  END,
  INVALID,
};

const TokenType TOKENTYPE_SYMBOLS_FIRST = TokenType::L_BRACKET;
const TokenType TOKENTYPE_SYMBOLS_LAST = TokenType::SEMICOLON;

std::string token_type_str(TokenType t);
TokenType token_type_from_start(char start);

class Token {
private:
  TokenType type_;
  std::optional<std::string> rep_;

public:
  Token();
  Token(TokenType, std::optional<std::string>);
  static Token end();
  TokenType type();
  std::string rep();
};

class TokenStream {
private:
  unsigned int pos_;
  std::vector<Token> stream_;

public:
  TokenStream();
  unsigned int position();
  Token at(unsigned int i);
  Token next();
  Token lookahead();
  void add(Token);
};

TokenStream tokenize(std::string input);

#endif
