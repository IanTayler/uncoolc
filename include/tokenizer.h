#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include "symbol.h"
#include "token.h"

TokenStream tokenize(std::istream *, std::shared_ptr<SymbolTable>);

#endif // _TOKENIZER_H
