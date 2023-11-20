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

  // Print table header
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

  int opened_comments = 0;
  bool line_comment = false;

  TokenType type = token.type();
  while (type != TokenType::END) {
    token = tokens.next();
    type = token.type();

    if (type == TokenType::OPEN_COMMENT)
      opened_comments++;

    // Ignore tokens inside comments and whitespace unless it's verbose mode
    if (!show_all) {
      if (opened_comments > 0 && type == TokenType::CLOSE_COMMENT) {
        opened_comments--;
      } else if (type == TokenType::LINE_COMMENT) {
        line_comment = true;
      }

      if (line_comment && type == TokenType::NEW_LINE) {
        line_comment = false;
        continue;
      }

      if (type == TokenType::NEW_LINE || type == TokenType::SPACE ||
          type == TokenType::END || type == TokenType::OPEN_COMMENT ||
          type == TokenType::CLOSE_COMMENT || type == TokenType::LINE_COMMENT) {
        continue;
      }

      if (opened_comments > 0 || line_comment) {
        continue;
      }
    }

    std::cout << std::setw(position_width)
              << std::format("{}:{}", token.line(), token.column()) << " | "
              << std::setw(token_width) << token_type_str(type) << " | "
              << token.rep() << std::endl;
  }
}
