#pragma once

#ifdef UNIT_TEST
#include <stdexcept>
#include <string>
#define ENGINE_ASSERT(expr, msg)                                               \
  do {                                                                         \
    if (!(expr)) {                                                             \
      throw std::logic_error(std::string(msg) + " [" + __FILE__ + ":" +        \
                             std::to_string(__LINE__) + "]");                   \
    }                                                                          \
  } while (0)
#else
#include <cassert>
#define ENGINE_ASSERT(expr, msg) assert((expr) && (msg))
#endif
