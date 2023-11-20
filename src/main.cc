#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "./tokenizer.h"

int main(int argc, char *argv[]) {
  std::istream *stream = &std::cin;
  bool show_all = false;

  // Not used if reading from stdin
  std::fstream f;
  int arg_pos = 1;
  while (arg_pos < argc) {
    std::string arg{argv[arg_pos]};
    if (arg == "-v" || arg == "--verbose")
      show_all = true;

    else if (arg != "-") {
      f.open(arg, std::ios::in);
      stream = &f;
    }

    arg_pos++;
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
    TokenType type = token.type();
    if (!show_all && (type == TokenType::NEW_LINE || type == TokenType::SPACE ||
                      type == TokenType::END))
      continue;
    std::cout << std::setw(position_width)
              << std::format("{}:{}", token.line(), token.column()) << " | "
              << std::setw(token_width) << token_type_str(type) << " | "
              << token.rep() << std::endl;
  }
}
