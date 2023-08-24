#pragma once

#include <ostream>

#include <zecalculator/utils/token.h>
#include <zecalculator/utils/parsing_error.h>
#include <zecalculator/utils/syntax_tree.h>
#include <zecalculator/utils/evaluation_error.h>

namespace std {

template <class T, class U>
std::ostream& operator << (std::ostream& os, const tl::expected<T, U>& expected)
{
  if (expected)
    os << expected.value();
  else os << expected.error();

  return os;
}

template <class T>
std::ostream& operator << (std::ostream& os, const std::vector<T>& vec)
{
  os << "{ ";
  for (const auto& val: vec)
    os << val << ", ";
  os << " }" << std::endl;
  return os;
}

}

namespace zc {

std::ostream& operator << (std::ostream& os, const Token& token);

std::ostream& operator << (std::ostream& os, const SyntaxTree& node);

std::ostream& operator << (std::ostream& os, const ParsingError& err);

std::ostream& operator << (std::ostream& os, const EvaluationError& err);

}
