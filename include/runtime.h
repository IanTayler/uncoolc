#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <string>

namespace runtime {
enum class Error {
  CASE_VOID,
  CASE_UNMATCHED,
};

std::string to_string(Error error);

} // namespace runtime

#endif // !_RUNTIME_H
