#pragma once

#include <ostream>

#include <boost/ut.hpp>
#include <zecalculator/parsing/token.h>
#include <zecalculator/parsing/error.h>
#include <zecalculator/parsing/data_structures/tree.h>
#include <zecalculator/evaluation/error.h>

template <class T, class... U>
concept is_any_of = (std::is_same_v<T, U> or ...);

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

template <class... T> requires (sizeof...(T) > 0)
std::ostream& operator << (std::ostream& os, const std::variant<T...>& var)
{
  std::visit(overloaded{[&](std::monostate) { os << "std::monostate"; },
                        [&](auto&& alt) { os << alt; }},
             var);
  return os;
}

}

namespace zc {
namespace parsing {

std::ostream& operator << (std::ostream& os, const Token& token);
std::ostream& operator << (std::ostream& os, const Error& err);

namespace tokens {

std::ostream& operator<<(std::ostream &, const tokens::Text&);

template <class Tok>
  requires is_any_of<Tok,
                     tokens::Unkown,
                     tokens::Number,
                     tokens::Variable,
                     tokens::Function,
                     tokens::Operator,
                     tokens::OpeningParenthesis,
                     tokens::ClosingParenthesis,
                     tokens::FunctionCallStart,
                     tokens::FunctionCallEnd,
                     tokens::FunctionArgumentSeparator,
                     tokens::EndOfExpression>
std::ostream& operator<<(std::ostream& os, const Tok& token)
{
  os << boost::ut::reflection::type_name<Tok>() << " " << static_cast<const tokens::Text&>(token);
  return os;
}

}

}

namespace ast {

std::ostream& operator << (std::ostream& os, const Tree& node);

}

namespace eval {

std::ostream& operator << (std::ostream& os, const Error& err);

}

}
