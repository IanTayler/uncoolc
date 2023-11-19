#include "tokenizer.h"

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
    return "L_PAREN";
  case TokenType::R_PAREN:
    return "R_PAREN";
  case TokenType::L_BRACKET:
    return "L_BRACKET";
  case TokenType::R_BRACKET:
    return "R_BRACKET";
  case TokenType::L_SQ_BRACKET:
    return "L_SQ_BRACKET";
  case TokenType::R_SQ_BRACKET:
    return "R_SQ_BRACKET";
  case TokenType::SEMICOLON:
    return "SEMICOLON";
  case TokenType::SIMPLE_OP:
    return "SIMPLE_OP";
  // Complex operators
  case TokenType::MINUS_OP_CLASS:
    return "INEQ_OP";
  case TokenType::EQ_OP_CLASS:
    return "EQ_OP";
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
  case TokenType::KW_IF:
    return "KW_IF";
  case TokenType::KW_FI:
    return "KW_FI";
  case TokenType::KW_THEN:
    return "KW_THEN";
  case TokenType::KW_ELSE:
    return "KW_ELSE";
  case TokenType::KW_LET:
    return "KW_LET";
  case TokenType::KW_WHILE:
    return "KW_WHILE";
  case TokenType::KW_CASE:
    return "KW_CASE";
  case TokenType::KW_ESAC:
    return "KW_ESAC";
  case TokenType::KW_END:
    return "KW_END";
  case TokenType::KW_LOOP:
    return "KW_LOOP";
  case TokenType::KW_POOL:
    return "KW_POOL";
  case TokenType::KW_CLASS:
    return "KW_CLASS";
  case TokenType::KW_INHERITS:
    return "KW_INHERITS";
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
    return "__UNKNOWN_TOKEN_STR__";
  }
}

/**********************
 *                    *
 * TokenType analyzer *
 *                    *
 *********************/

// TODO(IT): name here weird
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
    return TokenType::L_PAREN;
  case ')':
    return TokenType::R_PAREN;
  case '{':
    return TokenType::L_BRACKET;
  case '}':
    return TokenType::R_BRACKET;
  case '[':
    return TokenType::L_SQ_BRACKET;
  case ']':
    return TokenType::R_SQ_BRACKET;
  case ';':
    return TokenType::SEMICOLON;
  case '=':
    return TokenType::EQ_OP_CLASS;
  case '<':
    return TokenType::MINUS_OP_CLASS;
  case '+':
  case '-':
  case '*':
  case '/':
  case '~':
    return TokenType::SIMPLE_OP;
  default:
    return TokenType::INVALID;
  }
}
