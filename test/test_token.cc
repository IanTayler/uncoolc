#include "doctest.h"
#include "token.h"

TEST_SUITE("TokenType") {
  TEST_CASE("to_string(TokenType) has correct values") {
    CHECK(to_string(TokenType::ARROW) == "=>");
    CHECK(to_string(TokenType::NEW_LINE) == "NEW_LINE");
    CHECK(to_string(TokenType::KW_LET) == "LET");
    CHECK(to_string(TokenType::OBJECT_NAME) == "OBJECT_NAME");
    CHECK(to_string(TokenType::TYPE_NAME) == "TYPE_NAME");
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
  TEST_CASE("Token end returns END type") {
    CHECK(Token::end().type() == TokenType::END);
  }

  TEST_CASE("Token constructor respects inputs") {
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

  TEST_CASE("Token end returns empty symbol") {
    CHECK(Token::end().symbol() == Symbol());
  }

  TEST_CASE("Token set_position sets position correctly") {
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
  TEST_CASE("TokenStream constructor is correct") {
    TokenStream stream = TokenStream();
    CHECK(stream.position() == 0);
    CHECK(stream.next().type() == TokenType::END);
  }

  TEST_CASE("TokenStream add adds what next reads") {
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
    CHECK(stream.next() == Token::end());
  }

  TEST_CASE("TokenStream next skip_whitespace skips newline") {
    TokenStream stream = TokenStream();

    // non-whitespace
    Token t0 = Token(TokenType::L_PAREN, Symbol(0));
    // whitespace
    Token t1 = Token(TokenType::NEW_LINE, Symbol(1));
    Token t2 = Token(TokenType::NEW_LINE, Symbol(2));
    // non-whitespace
    Token t3 = Token(TokenType::NUMBER, Symbol(3));
    // whitespace
    Token t4 = Token(TokenType::NEW_LINE, Symbol(4));
    // non-whitespace
    Token t5 = Token(TokenType::R_PAREN, Symbol(5));

    stream.add(t0);
    stream.add(t1);
    stream.add(t2);
    stream.add(t3);
    stream.add(t4);
    stream.add(t5);

    CHECK(stream.next(false) == t0);
    CHECK(stream.next(false) == t1);
    CHECK(stream.next(false) == t2);
    CHECK(stream.next(false) == t3);
    CHECK(stream.next(false) == t4);
    CHECK(stream.next(false) == t5);
    CHECK(stream.next(false) == Token::end());

    stream.reset_state();

    CHECK(stream.next(true) == t0);
    CHECK(stream.next(true) == t3);
    CHECK(stream.next(true) == t5);
    CHECK(stream.next(true) == Token::end());
  }

  TEST_CASE("TokenStream next skip_whitespace skips space") {
    TokenStream stream = TokenStream();

    // non-whitespace
    Token t0 = Token(TokenType::L_PAREN, Symbol(0));
    // whitespace
    Token t1 = Token(TokenType::SPACE, Symbol(1));
    Token t2 = Token(TokenType::SPACE, Symbol(2));
    // non-whitespace
    Token t3 = Token(TokenType::NUMBER, Symbol(3));
    // whitespace
    Token t4 = Token(TokenType::SPACE, Symbol(4));
    // non-whitespace
    Token t5 = Token(TokenType::R_PAREN, Symbol(5));

    stream.add(t0);
    stream.add(t1);
    stream.add(t2);
    stream.add(t3);
    stream.add(t4);
    stream.add(t5);

    CHECK(stream.next(false) == t0);
    CHECK(stream.next(false) == t1);
    CHECK(stream.next(false) == t2);
    CHECK(stream.next(false) == t3);
    CHECK(stream.next(false) == t4);
    CHECK(stream.next(false) == t5);
    CHECK(stream.next(false) == Token::end());

    stream.reset_state();

    CHECK(stream.next(true) == t0);
    CHECK(stream.next(true) == t3);
    CHECK(stream.next(true) == t5);
    CHECK(stream.next(true) == Token::end());
  }

  TEST_CASE("TokenStream next skip_whitespace skips line comment") {
    TokenStream stream = TokenStream();

    // non-whitespace
    Token t0 = Token(TokenType::L_PAREN, Symbol(0));
    // whitespace
    Token t1 = Token(TokenType::LINE_COMMENT, Symbol(1));
    Token t2 = Token(TokenType::STRING, Symbol(2));
    Token t3 = Token(TokenType::NEW_LINE, Symbol(3));
    // non-whitespace
    Token t4 = Token(TokenType::NUMBER, Symbol(4));
    // whitespace
    Token t5 = Token(TokenType::LINE_COMMENT, Symbol(5));
    Token t6 = Token(TokenType::NEW_LINE, Symbol(6));
    // non-whitespace
    Token t7 = Token(TokenType::R_PAREN, Symbol(7));

    stream.add(t0);
    stream.add(t1);
    stream.add(t2);
    stream.add(t3);
    stream.add(t4);
    stream.add(t5);
    stream.add(t6);
    stream.add(t7);

    CHECK(stream.next(false) == t0);
    CHECK(stream.next(false) == t1);
    CHECK(stream.next(false) == t2);
    CHECK(stream.next(false) == t3);
    CHECK(stream.next(false) == t4);
    CHECK(stream.next(false) == t5);
    CHECK(stream.next(false) == t6);
    CHECK(stream.next(false) == t7);
    CHECK(stream.next(false) == Token::end());

    stream.reset_state();

    CHECK(stream.next(true) == t0);
    CHECK(stream.next(true) == t4);
    CHECK(stream.next(true) == t7);
    CHECK(stream.next(true) == Token::end());
  }

  TEST_CASE("TokenStream next skip_whitespace skips (* comment") {
    TokenStream stream = TokenStream();

    // non-whitespace
    Token t0 = Token(TokenType::L_PAREN, Symbol(0));
    // whitespace
    Token t1 = Token(TokenType::OPEN_COMMENT, Symbol(1));
    Token t2 = Token(TokenType::STRING, Symbol(2));
    Token t3 = Token(TokenType::NUMBER, Symbol(3));
    Token t4 = Token(TokenType::INVALID, Symbol(4));
    Token t5 = Token(TokenType::CLOSE_COMMENT, Symbol(5));
    // non-whitespace
    Token t6 = Token(TokenType::NUMBER, Symbol(6));
    // whitespace
    Token t7 = Token(TokenType::OPEN_COMMENT, Symbol(7));
    Token t8 = Token(TokenType::CLOSE_COMMENT, Symbol(8));
    // non-whitespace
    Token t9 = Token(TokenType::R_PAREN, Symbol(9));

    stream.add(t0);
    stream.add(t1);
    stream.add(t2);
    stream.add(t3);
    stream.add(t4);
    stream.add(t5);
    stream.add(t6);
    stream.add(t7);
    stream.add(t8);
    stream.add(t9);

    CHECK(stream.next(false) == t0);
    CHECK(stream.next(false) == t1);
    CHECK(stream.next(false) == t2);
    CHECK(stream.next(false) == t3);
    CHECK(stream.next(false) == t4);
    CHECK(stream.next(false) == t5);
    CHECK(stream.next(false) == t6);
    CHECK(stream.next(false) == t7);
    CHECK(stream.next(false) == t8);
    CHECK(stream.next(false) == t9);
    CHECK(stream.next(false) == Token::end());

    stream.reset_state();

    CHECK(stream.next(true) == t0);
    CHECK(stream.next(true) == t6);
    CHECK(stream.next(true) == t9);
    CHECK(stream.next(true) == Token::end());
  }
}
