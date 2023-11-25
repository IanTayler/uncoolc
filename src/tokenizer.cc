#include "tokenizer.h"
#include "symbol.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/**********************
 *                    *
 *       Token        *
 *                    *
 *********************/

/// Full token initializer.
Token::Token(TokenType t, Symbol s) : type_(t), symb_(s), line_(0), col_(0) {}

/// Initializer for constant tokens.
Token::Token(TokenType t) : Token(t, Symbol{}) {}

/// Default token initializer.
Token::Token() : Token(TokenType::INVALID) {}

/// Special token marking an end of stream
Token Token::end() { return Token(TokenType::END); }

/// Return token type.
TokenType Token::type() { return type_; }

/// Return a representation. Empty string if no representation.
Symbol Token::symbol() { return symb_; }

/// Return the token's line number.
unsigned int Token::line() { return line_; }

/// Return the token's column number.
unsigned int Token::column() { return col_; }

/// Set the token's line number.
void Token::set_position(unsigned int l, unsigned int c) {
  line_ = l;
  col_ = c;
}

/**********************
 *                    *
 *     TokenStream    *
 *                    *
 *********************/

/// Create new TokenStream with no Tokens.
TokenStream::TokenStream() : pos_(0) { stream_ = std::vector<Token>(); }

/// Return position of the TokenStream pointer.
unsigned int TokenStream::position() { return pos_; }

/// Return Token at specific integer position.
Token TokenStream::at(unsigned int i) { return stream_.at(i); }

/// Move forward in the TokenStream.
Token TokenStream::next() {
  if (pos_ < stream_.size()) {
    return stream_.at(pos_++);
  }
  return Token::end();
}

/// Return next Token in stream without moving the pointer.
Token TokenStream::lookahead() { return stream_.at(pos_ + 1); }

/// Add a Token to the end of the stream.
void TokenStream::add(Token token) { stream_.push_back(token); }

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
  std::shared_ptr<SymbolTable> symbols;

  // Buffer storing input text as we process it
  // TODO(IT) only keep a lookahead buffer instead of the entire string
  std::string s_;

  /// Load input characters to string. Returns False if there are no more
  /// characters to read
  bool load(unsigned int ahead) {
    const int read_size = 16;
    while (pos_ + ahead >= s_.length()) {
      char buf[read_size];

      input->read(buf, read_size);
      int read_bytes = input->gcount();

      if (read_bytes > 0)
        s_.append(buf, read_bytes);
      else // Got to the end, should add an end character
        return false;
    }
    return true;
  }
  char current() {
    if (!load(0))
      return '\0';
    return s_[pos_];
  }

  char consume() {
    char c = current();

    if (c != '\0') {
      pos_++;
      col_++;
    }

    return c;
  }

  void advance(unsigned int ahead) {
    load(ahead);
    pos_ += ahead;
    col_ += ahead;
  }

  char lookahead() { return lookahead(0); }

  char lookahead(unsigned int i) {
    if (!load(i))
      return '\0';
    return s_[pos_ + i];
  }

  bool is_alphanum(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_');
  }

  std::optional<Token> match_keyword(unsigned int start_pos,
                                     unsigned int end_pos) {
    int len = end_pos - start_pos;
    char c0, c1, c2, c3, c4;
    c0 = lookahead();
    if (len > 1)
      c1 = lookahead(1);
    if (len > 2)
      c2 = lookahead(2);
    if (len > 3)
      c3 = lookahead(3);
    if (len > 4)
      c4 = lookahead(4);

    switch (len) {
    case 2: // if, in, fi, of
      if (c0 == 'i' && c1 == 'f')
        return Token(TokenType::KW_IF);

      if (c0 == 'i' && c1 == 'n')
        return Token(TokenType::KW_IN);

      if (c0 == 'f' && c1 == 'i')
        return Token(TokenType::KW_FI);

      if (c0 == 'o' && c1 == 'f')
        return Token(TokenType::KW_OF);

      break;
    case 3: // let, end
      if (c0 == 'l' && c1 == 'e' && c2 == 't')
        return Token(TokenType::KW_LET);

      if (c0 == 'n' && c1 == 'e' && c2 == 'w')
        return Token(TokenType::KW_NEW);

      break;
    case 4: // else, esac, then, true, loop and pool
      if (c0 == 'e' && c1 == 'l' && c2 == 's' && c3 == 'e')
        return Token(TokenType::KW_ELSE);

      if (c0 == 'e' && c1 == 's' && c2 == 'a' && c3 == 'c')
        return Token(TokenType::KW_ESAC);

      if (c0 == 't' && c1 == 'h' && c2 == 'e' && c3 == 'n')
        return Token(TokenType::KW_THEN);

      if (c0 == 't' && c1 == 'r' && c2 == 'u' && c3 == 'e')
        return Token(TokenType::KW_TRUE);

      if (c0 == 'l' && c1 == 'o' && c2 == 'o' && c3 == 'p')
        return Token(TokenType::KW_LOOP);

      if (c0 == 'p' && c1 == 'o' && c2 == 'o' && c3 == 'l')
        return Token(TokenType::KW_POOL);

      break;
    case 5: // while, class, false
      if (c0 == 'w' && c1 == 'h' && c2 == 'i' && c3 == 'l' && c4 == 'e')
        return Token(TokenType::KW_WHILE);

      if (c0 == 'c' && c1 == 'l' && c2 == 'a' && c3 == 's' && c4 == 's')
        return Token(TokenType::KW_CLASS);

      if (c0 == 'f' && c1 == 'a' && c2 == 'l' && c3 == 's' && c4 == 'e')
        return Token(TokenType::KW_FALSE);

      break;
    case 8: // inherits
      char c5, c6, c7;
      c5 = lookahead(5);
      c6 = lookahead(6);
      c7 = lookahead(7);

      if (c0 == 'i' && c1 == 'n' && c2 == 'h' && c3 == 'e' && c4 == 'r' &&
          c5 == 'i' && c6 == 't' && c7 == 's')
        return Token(TokenType::KW_INHERITS);

      break;
    }
    return std::nullopt;
  }

  Token get_name(TokenType t) {
    unsigned int start_pos = pos_;

    unsigned int ahead = 0;
    char c = current();
    while (is_alphanum(c)) {
      ahead++;
      c = lookahead(ahead);
    }
    unsigned int end_pos = pos_ + ahead;

    std::optional<Token> kw_token = match_keyword(start_pos, end_pos);
    advance(ahead);

    if (kw_token)
      return kw_token.value();

    // Didn't match a keyword
    int len = end_pos - start_pos;
    return Token(t, symbols->symbol_from(s_.substr(start_pos, len)));
  }

  Token get_symbol(TokenType t) {
    consume();
    return Token(t);
  }

  Token get_parenthesis(TokenType t) {
    // Consume initial parenthesis
    consume();

    if (t == TokenType::R_PAREN) {
      return Token(t);
    }

    char c = current();
    if (c == '*') {
      consume();
      return Token(TokenType::OPEN_COMMENT);
    }

    return Token(TokenType::L_PAREN);
  }

  Token get_dash(TokenType t) {
    // Consume initial -
    consume();
    char c = current();
    if (c == '-') {
      consume();
      return Token(TokenType::LINE_COMMENT);
    }
    return Token(TokenType::SIMPLE_OP, symbols->symbol_from("-"));
  }

  Token get_asterisk(TokenType t) {
    // Consume initial *
    consume();
    char c = current();
    if (c == ')') {
      consume();
      return Token(TokenType::CLOSE_COMMENT);
    }
    return Token(TokenType::SIMPLE_OP, symbols->symbol_from("*"));
  }

  Token get_minor_op(TokenType t) {
    // Consume initial <
    consume();
    char c = current();
    if (c == '-') {
      consume();
      return Token(TokenType::ASSIGN);
    }
    if (c == '=') {
      consume();
      return Token(TokenType::SIMPLE_OP, symbols->symbol_from("<="));
    }
    return Token(TokenType::SIMPLE_OP, symbols->symbol_from("<"));
  }

  Token get_eq_op(TokenType t) {
    // Consume initial =
    consume();
    char c = current();
    if (c == '>') {
      consume();
      return Token(TokenType::ARROW);
    }
    return Token(TokenType::SIMPLE_OP, symbols->symbol_from("="));
  }

  Token get_space(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (token_type_from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;

    int len = end_pos - start_pos;
    return Token(t, symbols->symbol_from(s_.substr(start_pos, len)));
  }

  Token get_number(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (token_type_from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;

    int len = end_pos - start_pos;
    return Token(t, symbols->symbol_from(s_.substr(start_pos, len)));
  }

  Token get_string(TokenType t) {
    unsigned int start_pos = pos_;
    char c = consume(); // Consume the starting quotes.
    for (c = consume(); (c != '\00' && c != '"'); c = consume()) {
    }
    // TODO(IT) handle broken string here
    unsigned int len = start_pos - pos_;
    return Token(t, symbols->symbol_from(s_.substr(start_pos, len)));
  }

  Token get_in_category(TokenType t) {
    switch (t) {
    case TOKENTYPE_SYMBOLS_FIRST ... TOKENTYPE_SYMBOLS_LAST:
      return get_symbol(t);
    case TokenType::L_PAREN_CLASS:
    case TokenType::R_PAREN:
      return get_parenthesis(t);
    case TokenType::DASH_CLASS:
      return get_dash(t);
    case TokenType::ASTERISK_CLASS:
      return get_asterisk(t);
    case TokenType::MINOR_OP_CLASS:
      return get_minor_op(t);
    case TokenType::EQ_OP_CLASS:
      return get_eq_op(t);
    case TokenType::OBJECT_NAME:
    case TokenType::TYPE_NAME:
      return get_name(t);
    case TokenType::NUMBER:
      return get_number(t);
    case TokenType::SPACE:
      return get_space(t);
    case TokenType::STRING:
      return get_string(t);
    case TokenType::END:
      return Token::end();
    default:
      return get_symbol(t);
    }
  }

public:
  explicit Tokenizer(std::istream *inp, std::shared_ptr<SymbolTable> symbs)
      : pos_(0), input(inp), symbols(symbs), line_(1), col_(1) {}

  Token get() {
    unsigned int l = line_;
    unsigned int c = col_;
    TokenType t = token_type_from_start(current());
    Token token = get_in_category(t);
    token.set_position(l, c);
    if (token.type() == TokenType::NEW_LINE) {
      line_++;
      col_ = 1;
    }
    return token;
  }
};

/// Main exported function in the tokenizer. Return a stream of tokens.
TokenStream tokenize(std::istream *input,
                     std::shared_ptr<SymbolTable> symbols) {
  TokenStream tokens = TokenStream();
  Tokenizer tokenizer = Tokenizer(input, symbols);
  Token last_token;
  do {
    last_token = tokenizer.get();
    tokens.add(last_token);
  } while (last_token.type() != TokenType::END);

  return tokens;
}
