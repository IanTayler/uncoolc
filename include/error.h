#ifndef _ERROR_H
#define _ERROR_H

#include "token.h"
#include <string>

void warning(std::string, Token);

void error(std::string, Token);

void fatal(std::string, Token, int);
void fatal(std::string, int);
void fatal(std::string);

void fatal(std::string, Token);

#endif // !_ERROR_H
