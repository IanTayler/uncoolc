#include <iostream>
#include <string>

#include "./tokenizer.h"

int main() {
  std::string s;
  getline(std::cin, s);
  TokenStream tokens = tokenize(s);
  Token token;
  while (token.type() != TokenType::END) {
    token = tokens.next();
    std::cout << "TOKEN: " << token_type_str(token.type()) << " " << token.rep()
              << std::endl;
  }
}
