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
    std::cout << "TOKEN: " << static_cast<int>(token.type()) << " "
              << token.rep() << std::endl;
  }
}
