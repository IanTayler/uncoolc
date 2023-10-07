#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
  // Symbols
  L_PAREN,
  R_PAREN,
  ASSIGN,
  // IDs
  NAME,
  // Literals
  NUMBER,
  STRING,
  // Misc
  SPACE,
  END,
  INVALID,
};

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
