#pragma once

#include <ostream>

#include <boost/ut.hpp>
#include <zecalculator/error.h>
#include <zecalculator/math_objects/function.h>
#include <zecalculator/parsing/data_structures/rpn.h>
#include <zecalculator/parsing/data_structures/fast.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace std {

template <class T>
std::ostream& operator << (std::ostream& os, const std::optional<T>& val)
{
  if (val)
    os << val.value();

  return os;
}

template <class T, class U>
std::ostream& operator << (std::ostream& os, const std::pair<T, U>& val)
{
  os << "{ " << val.first << ", " << val.second << " }";
  return os;
}

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

template <class T>
std::ostream& operator << (std::ostream& os, const std::unique_ptr<T>& ptr)
{
  ptr ? os << *ptr : os << "empty unique_ptr";
  return os;
}

template <class... T> requires (sizeof...(T) > 0)
std::ostream& operator << (std::ostream& os, const std::variant<T...>& var)
{
  std::visit(
    zc::utils::overloaded{
      [&](std::monostate) { os << "std::monostate"; },
      [&](auto&& alt) { os << alt; }},
    var);
  return os;
}

}

namespace zc {
  namespace parsing {

    std::ostream& operator << (std::ostream& os, const Token& token);

    std::ostream& operator << (std::ostream& os, const fast::node::Node<Type::FAST>& node);

    std::ostream& operator << (std::ostream& os, const fast::node::Node<Type::RPN>& node);

    std::ostream& operator << (std::ostream& os, const AST& node);

    namespace tokens {

    std::ostream& operator<<(std::ostream &, const tokens::Text&);

    template <class Tok>
      requires zc::utils::is_any_of<Tok,
                                    tokens::Unkown,
                                    tokens::Number,
                                    tokens::Variable,
                                    tokens::Function,
                                    tokens::Operator<'-', 2>,
                                    tokens::Operator<'+', 2>,
                                    tokens::Operator<'*', 2>,
                                    tokens::Operator<'/', 2>,
                                    tokens::Operator<'^', 2>,
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

    } // namespace tokens
  } // namespace parsing

  namespace deps {
    std::ostream& operator << (std::ostream& os, ObjectType type);
  }

  std::ostream& operator << (std::ostream& os, const Error& err);
  std::ostream& operator << (std::ostream& os, Ok);
}
