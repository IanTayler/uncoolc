#include <fstream>
#include <iostream>
#include <string>

#include "./tokenizer.h"

int main(int argc, char *argv[]) {
  std::string s;
  std::istream *stream = &std::cin;
  // Not used if reading from stdin
  std::fstream f;

  if (argc > 1) {
    std::string stream_name{argv[1]};
    if (stream_name != "-")
      f.open(stream_name, std::ios::in);
    stream = &f;
  }
  do {
    getline(*stream, s);
    TokenStream tokens = tokenize(s);
    Token token;
    while (token.type() != TokenType::END) {
      token = tokens.next();
      std::cout << "TOKEN: " << token_type_str(token.type()) << " "
                << token.rep() << std::endl;
    }
  } while (s.length() > 0);
}
