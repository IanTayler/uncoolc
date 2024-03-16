#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include "symbol.h"
#include "token.h"

/**********************
 *                    *
 *      Tokenizer     *
 *                    *
 *********************/

class Tokenizer {
private:
  // Internal state indicating where we are standing
  unsigned int pos_;
  unsigned int line_;
  unsigned int col_;

  // Handlers to external resources we're composing here
  std::istream *input;
  SymbolTable &symbols;

  // Buffer storing input text as we process it
  // TODO(IT) only keep a lookahead buffer instead of the entire string
  std::string s_;

  bool load(unsigned int ahead);
  char current();
  char consume();
  void advance(unsigned int ahead);
  char lookahead();
  char lookahead(unsigned int i);
  bool is_alphanum(char c);

  std::optional<Token> match_keyword(unsigned int start_pos,
                                     unsigned int end_pos);

  Token get_name(TokenType t);
  Token get_symbol(TokenType t);
  Token get_parenthesis(TokenType t);
  Token get_dash(TokenType t);
  Token get_asterisk(TokenType t);
  Token get_minor_op(TokenType t);
  Token get_eq_op(TokenType t);
  Token get_space(TokenType t);
  Token get_number(TokenType t);
  Token get_string(TokenType t);
  Token get_in_category(TokenType t);

public:
  explicit Tokenizer(std::istream *inp, SymbolTable &symbs)
      : pos_(0), input(inp), symbols(symbs), line_(1), col_(1) {}

  Token get();
};

/**********************
 *                    *
 *  Useful Functions  *
 *                    *
 *********************/

TokenStream tokenize(std::istream *, SymbolTable &);

#endif // _TOKENIZER_H
