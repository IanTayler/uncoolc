#include "error.h"
#include "token.h"
#include <iostream>

enum LogLevel {
  WARNING,
  ERROR,
  FATAL,
};

void _message(std::string message, LogLevel level, Token token) {
  std::string level_name;
  switch (level) {
  case WARNING:
    level_name = "WARNING";
    break;
  case ERROR:
    level_name = "ERROR";
    break;
  case FATAL:
    level_name = "FATAL";
    break;
  }

  std::cerr << level_name << ": " << token.line() << ":" << token.column()
            << " " << message << std::endl;
}

void warning(std::string message, Token token) {
  _message(message, WARNING, token);
}

void error(std::string message, Token token) {
  _message(message, ERROR, token);
}

void fatal(std::string message, Token token, int errorcode) {
  _message(message, FATAL, token);
  exit(errorcode);
}

void fatal(std::string message, Token token) { fatal(message, token, 1); }
