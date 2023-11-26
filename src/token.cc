#include "token.h"

#include <string>
#include <vector>

/**********************
 *                    *
 * TokenType helpers  *
 *                    *
 *********************/

std::string token_type_str(TokenType t) {
  switch (t) {
  // Symbols
  case TokenType::L_PAREN:
    return "(";
  case TokenType::R_PAREN:
    return ")";
  case TokenType::L_BRACKET:
    return "{";
  case TokenType::R_BRACKET:
    return "}";
  case TokenType::L_SQ_BRACKET:
    return "[";
  case TokenType::R_SQ_BRACKET:
    return "]";
  case TokenType::DOT:
    return ".";
  case TokenType::COMMA:
    return ",";
  case TokenType::COLON:
    return ":";
  case TokenType::SEMICOLON:
    return ";";
  case TokenType::SIMPLE_OP:
    return "SIMPLE_OP";
  // Complex operators and token classes
  case TokenType::L_PAREN_CLASS:
    return "__L_PAREN_CLASS";
  case TokenType::MINOR_OP_CLASS:
    return "__MINOR_OP_CLASS";
  case TokenType::EQ_OP_CLASS:
    return "__EQ_OP_CLASS";
  case TokenType::ASTERISK_CLASS:
    return "__ASTERISK_CLASS";
  case TokenType::DASH_CLASS:
    return "__DASH_CLASS";
  case TokenType::ASSIGN:
    return "<-";
  case TokenType::ARROW:
    return "=>";
  // Comment related
  case TokenType::OPEN_COMMENT:
    return "(*";
  case TokenType::CLOSE_COMMENT:
    return "*)";
  case TokenType::LINE_COMMENT:
    return "--";
  // IDs
  case TokenType::OBJECT_NAME:
    return "OBJECT_NAME";
  case TokenType::TYPE_NAME:
    return "TYPE_NAME";
  // Literals
  case TokenType::NUMBER:
    return "NUMBER";
  case TokenType::STRING:
    return "STRING";
  // Keywords
  case TokenType::KW_TRUE:
    return "TRUE";
  case TokenType::KW_FALSE:
    return "FALSE";
  case TokenType::KW_ISVOID:
    return "ISVOID";
  case TokenType::KW_IF:
    return "IF";
  case TokenType::KW_FI:
    return "FI";
  case TokenType::KW_IN:
    return "IN";
  case TokenType::KW_OF:
    return "OF";
  case TokenType::KW_THEN:
    return "THEN";
  case TokenType::KW_ELSE:
    return "ELSE";
  case TokenType::KW_LET:
    return "LET";
  case TokenType::KW_NEW:
    return "NEW";
  case TokenType::KW_WHILE:
    return "WHILE";
  case TokenType::KW_CASE:
    return "CASE";
  case TokenType::KW_ESAC:
    return "ESAC";
  case TokenType::KW_LOOP:
    return "LOOP";
  case TokenType::KW_POOL:
    return "POOL";
  case TokenType::KW_CLASS:
    return "CLASS";
  case TokenType::KW_INHERITS:
    return "INHERITS";
  // Misc
  case TokenType::SPACE:
    return "SPACE";
  case TokenType::NEW_LINE:
    return "NEW_LINE";
  case TokenType::END:
    return "END";
  case TokenType::INVALID:
    return "INVALID";
  default:
    return "__UNKNOWN_TOKEN__";
  }
}

/**********************
 *                    *
 * TokenType analyzer *
 *                    *
 *********************/

TokenType token_type_from_start(char start) {
  switch (start) {
  case '\0':
    return TokenType::END;
  case '\n':
    return TokenType::NEW_LINE;
  case 'a' ... 'z':
    return TokenType::OBJECT_NAME;
  case 'A' ... 'Z':
    return TokenType::TYPE_NAME;
  case '0' ... '9':
    return TokenType::NUMBER;
  case ' ':
  case '\t':
    return TokenType::SPACE;
  case '"':
    return TokenType::STRING;
  case '(':
    return TokenType::L_PAREN_CLASS;
  case ')':
    return TokenType::R_PAREN;
  case '*':
    return TokenType::ASTERISK_CLASS;
  case '-':
    return TokenType::DASH_CLASS;
  case '{':
    return TokenType::L_BRACKET;
  case '}':
    return TokenType::R_BRACKET;
  case '[':
    return TokenType::L_SQ_BRACKET;
  case ']':
    return TokenType::R_SQ_BRACKET;
  case '.':
    return TokenType::DOT;
  case ',':
    return TokenType::COMMA;
  case ':':
    return TokenType::COLON;
  case ';':
    return TokenType::SEMICOLON;
  case '=':
    return TokenType::EQ_OP_CLASS;
  case '<':
    return TokenType::MINOR_OP_CLASS;
  case '+':
  case '/':
  case '~':
    return TokenType::SIMPLE_OP;
  default:
    return TokenType::INVALID;
  }
}

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
