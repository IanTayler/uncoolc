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

  TEST_CASE("tokenize class attributes examples") {
    std::shared_ptr<SymbolTable> symbs = std::make_shared<SymbolTable>();

    std::vector<InputOutput> cases = {
        {"class A inherits B {\n\tmyGreatObject : Int <- 12;\n};",
         {
             Token(TokenType::KW_CLASS, symbs->class_kw),
             Token(TokenType::TYPE_NAME, symbs->from("A")),
             Token(TokenType::KW_INHERITS, symbs->inherits_kw),
             Token(TokenType::TYPE_NAME, symbs->from("B")),
             Token(TokenType::L_BRACKET, symbs->from("{")),
             Token(TokenType::OBJECT_NAME, symbs->from("myGreatObject")),
             Token(TokenType::COLON, symbs->from(":")),
             Token(TokenType::TYPE_NAME, symbs->from("Int")),
             Token(TokenType::ASSIGN, symbs->from("<-")),
             Token(TokenType::NUMBER, symbs->from("12")),
             Token(TokenType::SEMICOLON, symbs->from(";")),
             Token(TokenType::R_BRACKET, symbs->from("}")),
             Token(TokenType::SEMICOLON, symbs->from(";")),
         }},
        {"class GoodClass{\n\tmyGreatObject : Int;\n\n\tanotherObject : "
         "String;\n};",
         {
             Token{TokenType::KW_CLASS, symbs->class_kw},
             Token{TokenType::TYPE_NAME, symbs->from("GoodClass")},
             Token{TokenType::L_BRACKET, symbs->from("{")},
             Token{TokenType::OBJECT_NAME, symbs->from("myGreatObject")},
             Token{TokenType::COLON, symbs->from(":")},
             Token{TokenType::TYPE_NAME, symbs->from("Int")},
             Token{TokenType::SEMICOLON, symbs->from(";")},
             Token{TokenType::OBJECT_NAME, symbs->from("anotherObject")},
             Token{TokenType::COLON, symbs->from(":")},
             Token{TokenType::TYPE_NAME, symbs->from("String")},
             Token{TokenType::SEMICOLON, symbs->from(";")},
             Token{TokenType::R_BRACKET, symbs->from("}")},
             Token{TokenType::SEMICOLON, symbs->from(";")},
         }},
    };
    check_cases(cases, symbs);
  }

  TEST_CASE("tokenize class long example") {
    std::shared_ptr<SymbolTable> symbs = std::make_shared<SymbolTable>();

    const char *example_long = R"""(
    class A {
      var : A; -- my favourite var
      call(v: Int, b: Int): Int {{
        if v < b then
          b <- var@A.call(v,v)
        else
          v <- var.call(b,b)
        fi;
        a;
      }};
    };

    (* Class B is *very* important!!!.
       Some more text in comment *)
    class B {
      myVar : A;
    };
    )""";

    const std::vector<Token> example_long_expected = {
        // Class header
        Token{TokenType::KW_CLASS, symbs->class_kw},
        Token{TokenType::TYPE_NAME, symbs->from("A")},
        Token{TokenType::L_BRACKET, symbs->from("{")},
        // var definition
        Token{TokenType::OBJECT_NAME, symbs->from("var")},
        Token{TokenType::COLON, symbs->from(":")},
        Token{TokenType::TYPE_NAME, symbs->from("A")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        // call method header
        Token{TokenType::OBJECT_NAME, symbs->from("call")},
        Token{TokenType::L_PAREN, symbs->from("(")},
        Token{TokenType::OBJECT_NAME, symbs->from("v")},
        Token{TokenType::COLON, symbs->from(":")},
        Token{TokenType::TYPE_NAME, symbs->from("Int")},
        Token{TokenType::COMMA, symbs->from(",")},
        Token{TokenType::OBJECT_NAME, symbs->from("b")},
        Token{TokenType::COLON, symbs->from(":")},
        Token{TokenType::TYPE_NAME, symbs->from("Int")},
        Token{TokenType::R_PAREN, symbs->from(")")},
        Token{TokenType::COLON, symbs->from(":")},
        Token{TokenType::TYPE_NAME, symbs->from("Int")},
        Token{TokenType::L_BRACKET, symbs->from("{")},
        // call method body
        Token{TokenType::L_BRACKET, symbs->from("{")},
        Token{TokenType::KW_IF, symbs->if_kw},
        Token{TokenType::OBJECT_NAME, symbs->from("v")},
        Token{TokenType::SIMPLE_OP, symbs->from("<")},
        Token{TokenType::OBJECT_NAME, symbs->from("b")},
        Token{TokenType::KW_THEN, symbs->then_kw},
        Token{TokenType::OBJECT_NAME, symbs->from("b")},
        Token{TokenType::ASSIGN, symbs->from("<-")},
        Token{TokenType::OBJECT_NAME, symbs->from("var")},
        Token{TokenType::AT, symbs->from("@")},
        Token{TokenType::TYPE_NAME, symbs->from("A")},
        Token{TokenType::DOT, symbs->from(".")},
        Token{TokenType::OBJECT_NAME, symbs->from("call")},
        Token{TokenType::L_PAREN, symbs->from("(")},
        Token{TokenType::OBJECT_NAME, symbs->from("v")},
        Token{TokenType::COMMA, symbs->from(",")},
        Token{TokenType::OBJECT_NAME, symbs->from("v")},
        Token{TokenType::R_PAREN, symbs->from(")")},
        Token{TokenType::KW_ELSE, symbs->else_kw},
        Token{TokenType::OBJECT_NAME, symbs->from("v")},
        Token{TokenType::ASSIGN, symbs->from("<-")},
        Token{TokenType::OBJECT_NAME, symbs->from("var")},
        Token{TokenType::DOT, symbs->from(".")},
        Token{TokenType::OBJECT_NAME, symbs->from("call")},
        Token{TokenType::L_PAREN, symbs->from("(")},
        Token{TokenType::OBJECT_NAME, symbs->from("b")},
        Token{TokenType::COMMA, symbs->from(",")},
        Token{TokenType::OBJECT_NAME, symbs->from("b")},
        Token{TokenType::R_PAREN, symbs->from(")")},
        Token{TokenType::KW_FI, symbs->fi_kw},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        Token{TokenType::OBJECT_NAME, symbs->from("a")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        Token{TokenType::R_BRACKET, symbs->from("}")},
        // end call method body
        Token{TokenType::R_BRACKET, symbs->from("}")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        // end A class
        Token{TokenType::R_BRACKET, symbs->from("}")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        // class B header
        Token{TokenType::KW_CLASS, symbs->class_kw},
        Token{TokenType::TYPE_NAME, symbs->from("B")},
        Token{TokenType::L_BRACKET, symbs->from("{")},
        // var definition
        Token{TokenType::OBJECT_NAME, symbs->from("myVar")},
        Token{TokenType::COLON, symbs->from(":")},
        Token{TokenType::TYPE_NAME, symbs->from("A")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
        Token{TokenType::R_BRACKET, symbs->from("}")},
        Token{TokenType::SEMICOLON, symbs->from(";")},
    };

    std::vector<InputOutput> cases = {{example_long, example_long_expected}};
    check_cases(cases, symbs);
  }
}
