#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "./tokenizer.h"

int main(int argc, char *argv[]) {
  std::istream *stream = &std::cin;

  // Not used if reading from stdin
  std::fstream f;
  if (argc > 1) {
    std::string stream_name{argv[1]};
    if (stream_name != "-") {
      f.open(stream_name, std::ios::in);
      stream = &f;
    }
  }

  TokenStream tokens = tokenize(stream);
  Token token;
  const int position_width = 8;
  const int token_width = 13;

  std::cout << "POSITION "
            << "|"
            << "   TOKEN TYPE  "
            << "|"
            << " STRING" << std::endl;
  std::cout << "---------"
            << "|"
            << "---------------"
            << "|"
            << "---------" << std::endl;

  while (token.type() != TokenType::END) {
    token = tokens.next();
    std::cout << std::setw(position_width)
              << std::format("{}:{}", token.line(), token.column()) << " | "
              << std::setw(token_width) << token_type_str(token.type()) << " | "
              << token.rep() << std::endl;
  }
}
