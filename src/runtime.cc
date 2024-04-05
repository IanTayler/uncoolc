#include "runtime.h"

namespace runtime {
std::string to_string(Error error) {
  switch (error) {
  case Error::CASE_VOID:
    return "case_void";
  case Error::CASE_UNMATCHED:
    return "case_unmatched";
  default:
    return "__unknown_runtime_error__";
  }
}

} // namespace runtime
