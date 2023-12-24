#include "doctest.h"
#include "tokenizer.h"
#include <sstream>

struct InputOutput {
  std::string inp;
  std::vector<Token> expected;
};

void check_cases(std::vector<InputOutput> cases,
                 std::shared_ptr<SymbolTable> symbs) {
  for (const auto &[inp, expected] : cases) {
    std::istringstream inp_stream(inp);
    auto tokens = tokenize(&inp_stream, symbs);
    for (auto &exp : expected) {
      CHECK(exp == tokens.next(true));
    }
    CHECK(Token::end() == tokens.next());
  }
}
TEST_SUITE("tokenize") {
  TEST_CASE("tokenize single token") {
    std::shared_ptr<SymbolTable> symbs = std::make_shared<SymbolTable>();

    std::vector<InputOutput> cases = {
        {"812", {Token(TokenType::NUMBER, symbs->from("812"))}},
        {"(", {Token(TokenType::L_PAREN, symbs->from("("))}},
        {"\"dance\"", {Token(TokenType::STRING, symbs->from("\"dance\""))}},
        {"inherits", {Token(TokenType::KW_INHERITS, symbs->from("inherits"))}},
        {"let", {Token(TokenType::KW_LET, symbs->from("let"))}},
        {"812", {Token(TokenType::NUMBER, symbs->from("812"))}},
        {"*", {Token(TokenType::SIMPLE_OP, symbs->from("*"))}},
        {"/", {Token(TokenType::SIMPLE_OP, symbs->from("/"))}},
        {"=", {Token(TokenType::SIMPLE_OP, symbs->from("="))}},
        {"<=", {Token(TokenType::SIMPLE_OP, symbs->from("<="))}},
        {"Main", {Token(TokenType::TYPE_NAME, symbs->from("Main"))}},
        {"MultiWordTypeName",
         {Token(TokenType::TYPE_NAME, symbs->from("MultiWordTypeName"))}},
        {"main", {Token(TokenType::OBJECT_NAME, symbs->from("main"))}},
        {"some_method_with_snake_case",
         {Token(TokenType::OBJECT_NAME,
                symbs->from("some_method_with_snake_case"))}},
        {"someMethodWithCamelCase",
         {Token(TokenType::OBJECT_NAME,
                symbs->from("someMethodWithCamelCase"))}},
    };
    check_cases(cases, symbs);
  }
}
