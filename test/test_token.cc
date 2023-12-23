#include "doctest.h"
#include "token.h"

TEST_SUITE("TokenType") {
  TEST_CASE("token_type_str has correct values") {
    CHECK(token_type_str(TokenType::ARROW) == "=>");
    CHECK(token_type_str(TokenType::NEW_LINE) == "NEW_LINE");
    CHECK(token_type_str(TokenType::KW_LET) == "LET");
    CHECK(token_type_str(TokenType::OBJECT_NAME) == "OBJECT_NAME");
    CHECK(token_type_str(TokenType::TYPE_NAME) == "TYPE_NAME");
  }

  TEST_CASE("token_type_from_start gets correct TokenType") {
    CHECK(token_type_from_start('@') == TokenType::AT);
    CHECK(token_type_from_start('.') == TokenType::DOT);
    CHECK(token_type_from_start('1') == TokenType::NUMBER);
    CHECK(token_type_from_start('"') == TokenType::STRING);
    CHECK(token_type_from_start('(') == TokenType::L_PAREN_CLASS);
    CHECK(token_type_from_start(' ') == TokenType::SPACE);
  }
}

TEST_SUITE("Token") {
  TEST_CASE("end returns END type") {
    CHECK(Token::end().type() == TokenType::END);
  }

  TEST_CASE("constructor respects inputs") {
    Token token = Token{TokenType::COLON, Symbol(15)};
    CHECK(token.symbol() == Symbol(15));
    CHECK(token.type() == TokenType::COLON);

    token = Token{TokenType::L_PAREN, Symbol(21)};
    CHECK(token.symbol() == Symbol(21));
    CHECK(token.type() == TokenType::L_PAREN);

    token = Token{TokenType::OPEN_COMMENT, Symbol(0)};
    CHECK(token.symbol() == Symbol(0));
    CHECK(token.type() == TokenType::OPEN_COMMENT);

    token = Token{TokenType::OPEN_COMMENT, Symbol(0)};
    CHECK(token.symbol() == Symbol(0));
    CHECK(token.type() == TokenType::OPEN_COMMENT);
  }

  TEST_CASE("end returns empty symbol") {
    CHECK(Token::end().symbol() == Symbol());
  }

  TEST_CASE("set_position sets position correctly") {
    Token token = Token(TokenType::SPACE, Symbol(1));
    token.set_position(0, 1);
    CHECK(token.line() == 0);
    CHECK(token.column() == 1);

    token.set_position(2, 3);
    CHECK(token.line() == 2);
    CHECK(token.column() == 3);

    token.set_position(20, 1);
    CHECK(token.line() == 20);
    CHECK(token.column() == 1);
  }
}

TEST_SUITE("TokenStream") {
  TEST_CASE("constructor is correct") {
    TokenStream stream = TokenStream();
    CHECK(stream.position() == 0);
    CHECK(stream.next().type() == TokenType::END);
  }

  TEST_CASE("add adds what next reads") {
    TokenStream stream = TokenStream();

    Token t0 = Token(TokenType::L_PAREN, Symbol(0));
    Token t1 = Token(TokenType::NUMBER, Symbol(1));
    Token t2 = Token(TokenType::SIMPLE_OP, Symbol(2));
    Token t3 = Token(TokenType::NUMBER, Symbol(3));
    Token t4 = Token(TokenType::R_PAREN, Symbol(4));

    stream.add(t0);
    stream.add(t1);
    stream.add(t2);
    stream.add(t3);
    stream.add(t4);

    CHECK(stream.next() == t0);
    CHECK(stream.next() == t1);
    CHECK(stream.next() == t2);
    CHECK(stream.next() == t3);
    CHECK(stream.next() == t4);
  }
}
