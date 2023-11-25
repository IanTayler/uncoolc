#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "symbol.h"
#include "token.h"
#include "tokenizer.h"

struct CliOptions {
  bool debug_output;
  std::filesystem::path debug_dir;

  bool verbose;
};

std::optional<TokenStream> run_tokenizer(std::istream *input,
                                         std::shared_ptr<SymbolTable> symbols,
                                         const CliOptions &options) {
  TokenStream tokens = tokenize(input, symbols);

  std::ostream *output = nullptr;
  std::fstream out_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    out_file.open(options.debug_dir / "tokenizer.log", std::ios::out);
    output = &out_file;
  }

  if (output != nullptr) {
    std::cout << "got here" << std::endl; // TODO remove
    Token token;

    const int position_width = 8;
    const int token_width = 13;

    // Print table header
    *output << "POSITION "
            << "|"
            << "   TOKEN TYPE  "
            << "|"
            << " STRING" << std::endl;
    *output << "---------"
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
      if (!options.verbose) {
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
            type == TokenType::CLOSE_COMMENT ||
            type == TokenType::LINE_COMMENT) {
          continue;
        }

        if (opened_comments > 0 || line_comment) {
          continue;
        }
      }

      *output << std::setw(position_width)
              << std::format("{}:{}", token.line(), token.column()) << " | "
              << std::setw(token_width) << token_type_str(type) << " | "
              << symbols->get_string(token.symbol()) << std::endl;
    }
  }
  if (output != nullptr) {
    out_file.flush();
    out_file.close();
  }
  return tokens;
}

int main(int argc, char *argv[]) {
  std::istream *stream = &std::cin;
  bool verbose = false;
  bool debug = true; // Default to debug mode while we develop
  std::filesystem::path debug_dir = "./coolc-debug";

  // Not used if reading from stdin
  std::fstream f;
  int arg_pos = 1;
  while (arg_pos < argc) {
    std::string arg{argv[arg_pos]};
    if (arg == "-v" || arg == "--verbose")
      verbose = true;

    else if (arg == "--debug")
      debug = true;

    else if (arg != "-") {
      f.open(arg, std::ios::in);
      debug_dir /= arg;
      stream = &f;
    }

    arg_pos++;
  }

  std::shared_ptr<SymbolTable> symbols = std::make_shared<SymbolTable>();

  CliOptions options = {
      .debug_output = debug, .debug_dir = debug_dir, .verbose = verbose};

  run_tokenizer(stream, symbols, options);
}
