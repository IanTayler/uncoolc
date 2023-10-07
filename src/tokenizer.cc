#include "./tokenizer.h"

#include <string>
#include <vector>

/**********************
 *                    *
 *       Token        *
 *                    *
 *********************/
/// Full token initializer.
Token::Token(TokenType t, std::optional<std::string> r) : type_(t), rep_(r) {}

/// Default token initializer.
Token::Token() : type_(TokenType::INVALID), rep_(std::nullopt) {}

/// Special token marking an end of stream
Token Token::end() { return Token(TokenType::END, std::nullopt); }

/// Return token type.
TokenType Token::type() { return type_; }

/// Return a representation. Empty string if no representation.
std::string Token::rep() { return rep_.value_or(""); }

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
 *      TokenType     *
 *                    *
 *********************/
// TODO(IT): name here weird
TokenType from_start(char start) {
  switch (start) {
    case '\0':
      return TokenType::END;
    case '\n':
      return TokenType::END;
    case 'a' ... 'z':
      return TokenType::NAME;
    case 'A' ... 'Z':
      return TokenType::NAME;
    case '0' ... '9':
      return TokenType::NUMBER;
    case ' ':
      return TokenType::SPACE;
    case '"':
      return TokenType::STRING;
    case '(':
      return TokenType::L_PAREN;
    case ')':
      return TokenType::R_PAREN;
    case '=':
      return TokenType::ASSIGN;
    default:
      return TokenType::INVALID;
  }
}

/**********************
 *                    *
 *      Functions     *
 *                    *
 *********************/
class Tokenizer {
 private:
  unsigned int pos_;
  std::string s_;

  char current() { return s_[pos_]; }

  char consume() {
    char c = s_[pos_];
    pos_++;
    return c;
  }

  char lookahead() { return s_[pos_ + 1]; }

  bool is_alphanum(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
  }

  Token get_name(TokenType t) {
    unsigned int start_pos = pos_;
    for (char c = consume(); is_alphanum(c); c = consume()) {
    }
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_symbol(TokenType t) {
    consume();
    return Token(t, std::nullopt);
  }

  Token get_space(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_number(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    // TODO(IT) support floating point and separators
    while (from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_string(TokenType t) {
    unsigned int start_pos = pos_;
    char c = consume();  // Consume the starting quotes.
    for (c = consume(); (c != '\00' && c != '"'); c = consume()) {
    }
    // TODO(IT) handle broken string here
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_category(TokenType t) {
    switch (t) {
      case TokenType::L_PAREN... TokenType::R_PAREN:
        return get_symbol(t);
      case TokenType::NAME:
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
  explicit Tokenizer(std::string s) : pos_(0), s_(s) {}

  Token get() {
    TokenType t = from_start(current());
    Token token = get_category(t);
    return token;
  }
};

/// Main exported function in the tokenizer. Return a stream of tokens.
TokenStream tokenize(std::string input) {
  TokenStream tokens = TokenStream();
  Tokenizer tokenizer = Tokenizer(input);
  Token last_token;
  do {
    last_token = tokenizer.get();
    tokens.add(last_token);
  } while (last_token.type() != TokenType::END);

  return tokens;
}
