#ifndef _TOKEN_H
#define _TOKEN_H

#include "symbol.h"
#include <optional>
#include <string>
#include <vector>

enum class TokenType {
  // Symbols
  L_BRACKET,
  R_BRACKET,
  L_SQ_BRACKET,
  R_SQ_BRACKET,
  AT,
  DOT,
  COMMA,
  COLON,
  SEMICOLON,
  // Parentheses or comment tokens
  L_PAREN_CLASS,
  L_PAREN,
  OPEN_COMMENT,
  R_PAREN,
  ASTERISK_CLASS, // *) (CLOSE_COMMENT) and * (SIMPLE_OP)
  CLOSE_COMMENT,
  DASH_CLASS, // -- (LINE_COMMENT) and - (SIMPLE_OP)
  LINE_COMMENT,
  // Operators
  MINOR_OP_CLASS, // <- (ASSIGN), < and <= (SIMPLE_OPs)
  ASSIGN,
  EQ_OP_CLASS, // = (SIMPLE_OP) and => (ARROW)
  ARROW,
  NEG_OP,
  SIMPLE_OP,
  // IDs
  OBJECT_NAME,
  TYPE_NAME,
  // Literals
  NUMBER,
  STRING,
  // Keywords
  KW_TRUE,
  KW_FALSE,
  KW_ISVOID,
  KW_IF,
  KW_FI,
  KW_THEN,
  KW_ELSE,
  KW_LET,
  KW_NEW,
  KW_NOT,
  KW_IN,
  KW_WHILE,
  KW_CASE,
  KW_ESAC,
  KW_OF,
  KW_LOOP,
  KW_POOL,
  KW_CLASS,
  KW_INHERITS,
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
  Symbol symb_;
  unsigned int line_;
  unsigned int col_;

public:
  Token();
  Token(TokenType);
  Token(TokenType, Symbol);
  static Token end();
  TokenType type() const;
  Symbol symbol() const;
  unsigned int line();
  unsigned int column();
  void set_position(unsigned int, unsigned int);

  bool operator==(const Token &) const;
};

class TokenStream {
private:
  struct State {
    unsigned int pos_;
    int opened_comments;
    bool line_comment;
  };

  unsigned int pos_;
  int opened_comments;
  bool line_comment;

  std::vector<Token> stream_;

  Token next_raw();
  State get_state() const;
  void restore_state(State);

public:
  TokenStream();

  unsigned int position();

  Token at(unsigned int i);
  Token next();
  Token next(bool skip_whitespace);
  Token lookahead();
  Token lookahead(unsigned int k);

  void add(Token);

  void reset_state();
};

#endif
