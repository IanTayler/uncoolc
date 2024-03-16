#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "ast.h"
#include "parser.h"
#include "symbol.h"
#include "token.h"
#include "tokenizer.h"

/**********************
 *                    *
 *      Helpers       *
 *                    *
 *********************/

const std::filesystem::path debug_dir_base = "./coolc-debug";

struct CliOptions {
  bool debug_output;
  std::filesystem::path debug_dir;
  bool verbose;
};

/**********************
 *                    *
 *    Tokenization    *
 *                    *
 *********************/

TokenStream run_tokenizer(std::istream *input, SymbolTable &symbols,
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

    Token token;
    TokenType type = token.type();
    while (type != TokenType::END) {
      token = tokens.next(!options.verbose);
      type = token.type();

      *output << std::setw(position_width)
              << std::format("{}:{}", token.line(), token.column()) << " | "
              << std::setw(token_width) << token_type_str(type) << " | "
              << symbols.get_string(token.symbol()) << std::endl;
    }

    tokens.reset_state();
  }
  return tokens;
}

/**********************
 *                    *
 *       Parsing      *
 *                    *
 *********************/

std::unique_ptr<ModuleNode> run_parser(TokenStream &tokens,
                                       const SymbolTable &symbols,
                                       const CliOptions &options) {

  std::unique_ptr<ModuleNode> node = Parser(tokens, symbols).parse();

  std::ostream *output = nullptr;
  std::fstream out_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    out_file.open(options.debug_dir / "parser.log", std::ios::out);
    output = &out_file;
  }

  if (output != nullptr) {
    AstPrinter printer{2, output};
    node->print(printer, symbols);
  }

  return node;
}

/**********************
 *                    *
 *     Entrypoint     *
 *                    *
 *********************/

int main(int argc, char *argv[]) {
  std::istream *stream = &std::cin;
  bool verbose = false;
  bool debug = true; // Default to debug mode while we develop
  std::filesystem::path debug_dir = debug_dir_base;

  // Not used if reading from stdin
  std::fstream input_file;
  int arg_pos = 1;
  while (arg_pos < argc) {
    std::string arg{argv[arg_pos]};
    if (arg == "-v" || arg == "--verbose")
      verbose = true;

    else if (arg == "--debug")
      debug = true;

    else if (arg != "-") {
      input_file.open(arg, std::ios::in);
      debug_dir /= arg;
      stream = &input_file;
    }

    arg_pos++;
  }

  std::unique_ptr<SymbolTable> symbols = std::make_unique<SymbolTable>();

  CliOptions options = {
      .debug_output = debug, .debug_dir = debug_dir, .verbose = verbose};

  TokenStream tokens = run_tokenizer(stream, *symbols, options);

  std::unique_ptr<ModuleNode> ast = run_parser(tokens, *symbols, options);
}
