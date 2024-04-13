#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "ast.h"
#include "error.h"
#include "hlir.h"
#include "hlir_optimizer.h"
#include "optimizer_config.h"
#include "parser.h"
#include "semantic.h"
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
  unsigned int indent;
};

/**********************
 *                    *
 *    Tokenization    *
 *                    *
 *********************/

TokenStream run_tokenizer(std::istream *input, SymbolTable &symbols,
                          const CliOptions &options, int &steps) {
  TokenStream tokens = tokenize(input, symbols);

  std::ostream *output = nullptr;
  std::fstream out_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    out_file.open(options.debug_dir / std::format("{:03}_tokenizer.log", steps),
                  std::ios::out);
    output = &out_file;
  }

  steps++;

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
              << std::setw(token_width) << to_string(type) << " | "
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
                                       const CliOptions &options, int &steps) {

  Parser parser = Parser(tokens, symbols);
  std::unique_ptr<ModuleNode> node = parser.parse();

  std::ostream *output = nullptr;
  std::fstream out_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    out_file.open(options.debug_dir / std::format("{:03}_parser.log", steps),
                  std::ios::out);
    output = &out_file;
  }

  steps++;

  if (output != nullptr) {
    Printer printer{options.indent, output};
    node->print(printer, symbols);
  }

  if (parser.get_error())
    fatal("Syntax errors found. Aborting compilation.");

  return node;
}

/**********************
 *                    *
 *  Semantic Analysis *
 *                    *
 *********************/

std::unique_ptr<ClassTree>
run_semantic_analysis(ModuleNode *module, Scopes &scopes, SymbolTable &symbols,
                      const CliOptions &options, int &steps) {
  std::unique_ptr<ClassTree> class_tree =
      std::make_unique<ClassTree>(module, symbols);

  TypeContext context = TypeContext(scopes, Symbol{}, *class_tree, symbols);
  bool check = module->typecheck(context);

  std::ostream *tree_output = nullptr;
  std::fstream tree_file;

  std::ostream *type_output = nullptr;
  std::fstream type_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    tree_file.open(options.debug_dir /
                       std::format("{:03}_class_tree.log", steps),
                   std::ios::out);
    tree_output = &tree_file;
  }

  steps++;

  if (tree_output != nullptr) {
    class_tree->print(tree_output);
  }

  if (options.debug_output) {
    type_file.open(options.debug_dir /
                       std::format("{:03}_typed_ast.log", steps),
                   std::ios::out);
    type_output = &type_file;
  }

  steps++;

  if (type_output != nullptr) {
    Printer printer{options.indent, type_output};
    module->print(printer, symbols);
  }

  if (!check)
    fatal("Semantic analysis failed. Aborting compilation.", Token{});

  return class_tree;
}

/**********************
 *                    *
 *    High-level IR   *
 *                    *
 *********************/

hlir::Universe run_hlir_generation(ModuleNode *module, SymbolTable &symbols,
                                   const CliOptions &options, int &steps) {
  hlir::Universe universe = module->to_hlir_universe(symbols);

  std::ostream *output = nullptr;
  std::fstream out_file;

  if (options.debug_output) {
    std::filesystem::create_directories(options.debug_dir);
    out_file.open(options.debug_dir / std::format("{:03}_from_ast.hlir", steps),
                  std::ios::out);
    output = &out_file;
  }

  steps++;

  if (output != nullptr) {
    Printer printer{options.indent, output};
    universe.print(printer, symbols);
  }
  return universe;
}

/**********************
 *                    *
 *   HLIR Optimizers  *
 *                    *
 *********************/

void run_hlir_optimizers(hlir::Universe &universe,
                         const OptimizerConfig &optimizer_config,
                         const SymbolTable &symbols, const CliOptions &options,
                         int &steps) {
  hlir::PassManager pass_manager{universe, optimizer_config};

  while (!pass_manager.is_done()) {
    const hlir::Pass &pass = pass_manager.run_pass();

    std::ostream *output = nullptr;
    std::fstream out_file;

    if (options.debug_output) {
      std::filesystem::create_directories(options.debug_dir);
      out_file.open(options.debug_dir /
                        std::format("{:03}_{}_opt.hlir", steps, pass.name),
                    std::ios::out);
      output = &out_file;
    }

    if (output != nullptr) {
      Printer printer{options.indent, output};
      universe.print(printer, symbols);
    }

    steps++;
  }
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

      debug_dir /= std::filesystem::path(arg).filename();
      stream = &input_file;
    }

    arg_pos++;
  }

  int steps = 0;

  SymbolTable symbols = SymbolTable();

  CliOptions options = {.debug_output = debug,
                        .debug_dir = debug_dir,
                        .verbose = verbose,
                        .indent = 2};

  TokenStream tokens = run_tokenizer(stream, symbols, options, steps);

  std::unique_ptr<ModuleNode> ast = run_parser(tokens, symbols, options, steps);

  Scopes scopes = Scopes();

  std::unique_ptr<ClassTree> class_tree =
      run_semantic_analysis(ast.get(), scopes, symbols, options, steps);

  hlir::Universe universe =
      run_hlir_generation(ast.get(), symbols, options, steps);

  OptimizerConfig optimizer_config;
  run_hlir_optimizers(universe, optimizer_config, symbols, options, steps);
}
