#pragma once

#include <ostream>

#include <zecalculator/utils/token.h>
#include <zecalculator/utils/error.h>

namespace zc {

std::ostream& operator << (std::ostream& os, const Token& token);

std::ostream& operator << (std::ostream& os, const Error& err);

template <class T>
std::ostream& operator << (std::ostream& os, const tl::expected<T, Error>& expected)
{
  if (not expected)
    os << expected.error();

  return os;
}

}
