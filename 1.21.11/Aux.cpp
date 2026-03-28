#include "Aux.hpp"

std::string_view Extract(std::string_view source_code, BaseToken token) {
  return source_code.substr(token.start, token.end - token.start + 1);
}
