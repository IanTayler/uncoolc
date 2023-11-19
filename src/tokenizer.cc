#include "tokenizer.h"

#include <string>
#include <vector>

/**********************
 *                    *
 *       Token        *
 *                    *
 *********************/

/// Full token initializer.
Token::Token(TokenType t, std::optional<std::string> r) : type_(t), rep_(r) {}

/// Initializer for constant tokens.
Token::Token(TokenType t) : type_(t), rep_(std::nullopt) {}

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
 *      Tokenizer     *
 *                    *
 *********************/
class Tokenizer {
private:
  unsigned int pos_;
  const std::string &s_;

  char current() { return s_[pos_]; }

  char consume() {
    char c = s_[pos_++];
    return c;
  }

  char lookahead() { return s_[pos_ + 1]; }

  bool is_alphanum(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
  }

  std::optional<Token> match_keyword(unsigned int start_pos,
                                     unsigned int end_pos) {
    int len = end_pos - start_pos;
    char c0, c1, c2, c3, c4;
    c0 = s_[start_pos];
    if (len > 1)
      c1 = s_[start_pos + 1];
    if (len > 2)
      c2 = s_[start_pos + 2];
    if (len > 3)
      c3 = s_[start_pos + 3];
    if (len > 4)
      c4 = s_[start_pos + 4];

    switch (len) {
    case 2: // if, in, fi
      if (c0 == 'i' && c1 == 'f')
        return Token(TokenType::KW_IF);

      if (c0 == 'i' && c1 == 'n')
        return Token(TokenType::KW_IN);

      if (c0 == 'f' && c1 == 'i')
        return Token(TokenType::KW_FI);

      break;
    case 3: // let, end
      if (c0 == 'l' && c1 == 'e' && c2 == 't')
        return Token(TokenType::KW_LET);

      if (c0 == 'e' && c1 == 'n' && c2 == 'd')
        return Token(TokenType::KW_END);

      break;
    case 4: // else, esac, then, loop and pool
      if (c0 == 'e' && c1 == 'l' && c2 == 's' && c3 == 'e')
        return Token(TokenType::KW_ELSE);
      if (c0 == 'e' && c1 == 's' && c2 == 'a' && c3 == 'c')
        return Token(TokenType::KW_ESAC);
      if (c0 == 't' && c1 == 'h' && c2 == 'e' && c3 == 'n')
        return Token(TokenType::KW_THEN);
      if (c0 == 'l' && c1 == 'o' && c2 == 'o' && c3 == 'p')
        return Token(TokenType::KW_LOOP);
      if (c0 == 'p' && c1 == 'o' && c2 == 'o' && c3 == 'l')
        return Token(TokenType::KW_POOL);
      break;
    case 5: // while, class
      if (c0 == 'w' && c1 == 'h' && c2 == 'i' && c3 == 'l' && c4 == 'e')
        return Token(TokenType::KW_WHILE);
      if (c0 == 'c' && c1 == 'l' && c2 == 'a' && c3 == 's' && c4 == 's')
        return Token(TokenType::KW_CLASS);
      break;
    case 8: // inherits
      char c5, c6, c7;
      c5 = s_[start_pos + 5];
      c6 = s_[start_pos + 6];
      c7 = s_[start_pos + 7];
      if (c0 == 'i' && c1 == 'n' && c2 == 'h' && c3 == 'e' && c4 == 'r' &&
          c5 == 'i' && c6 == 't' && c7 == 's')
        return Token(TokenType::KW_INHERITS);
      break;
    }
    return std::nullopt;
  }
  Token get_name(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (is_alphanum(c)) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;
    std::string substr = s_.substr(start_pos, end_pos - start_pos);
    std::optional<Token> kw_token = match_keyword(start_pos, end_pos);
    if (kw_token)
      return kw_token.value();
    // Didn't match a keyword
    return Token(t, substr);
  }

  Token get_symbol(TokenType t) {
    consume();
    return Token(t);
  }

  Token get_parenthesis(TokenType t) {
    if (t == TokenType::R_PAREN) {
      return Token(t);
    }
    // Consume initial parenthesis
    consume();
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
    return Token(TokenType::SIMPLE_OP, "-");
  }

  Token get_asterisk(TokenType t) {
    // Consume initial *
    consume();
    char c = current();
    if (c == ')') {
      consume();
      return Token(TokenType::CLOSE_COMMENT);
    }
    return Token(TokenType::SIMPLE_OP, "*");
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
      return Token(TokenType::SIMPLE_OP, "<=");
    }
    return Token(TokenType::SIMPLE_OP, "<");
  }

  Token get_eq_op(TokenType t) {
    // Consume initial =
    consume();
    char c = current();
    if (c == '>') {
      consume();
      return Token(TokenType::ARROW);
    }
    return Token(TokenType::SIMPLE_OP, "=");
  }

  Token get_space(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (token_type_from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_number(TokenType t) {
    unsigned int start_pos = pos_;
    char c = current();
    while (token_type_from_start(c) == t) {
      consume();
      c = current();
    }
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_string(TokenType t) {
    unsigned int start_pos = pos_;
    char c = consume(); // Consume the starting quotes.
    for (c = consume(); (c != '\00' && c != '"'); c = consume()) {
    }
    // TODO(IT) handle broken string here
    unsigned int end_pos = pos_;
    return Token(t, s_.substr(start_pos, end_pos - start_pos));
  }

  Token get_category(TokenType t) {
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
  explicit Tokenizer(const std::string &s) : pos_(0), s_(s) {}

  Token get() {
    TokenType t = token_type_from_start(current());
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
