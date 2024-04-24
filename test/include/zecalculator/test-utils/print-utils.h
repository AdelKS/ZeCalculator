#pragma once

#include <ostream>

#include <boost/ut.hpp>
#include <zecalculator/error.h>
#include <zecalculator/math_objects/function.h>
#include <zecalculator/parsing/data_structures/rpn.h>
#include <zecalculator/parsing/data_structures/fast.h>
#include <zecalculator/parsing/data_structures/token.h>

namespace std {

std::ostream& operator << (std::ostream& os, const zc::parsing::Token& token);

std::ostream& operator << (std::ostream& os, const zc::parsing::fast::node::Node<zc::parsing::Type::FAST>& node);

std::ostream& operator << (std::ostream& os, const zc::parsing::fast::node::Node<zc::parsing::Type::RPN>& node);

std::ostream& operator << (std::ostream& os, const zc::parsing::AST& node);


std::ostream& operator<<(std::ostream &, const zc::parsing::tokens::Text&);

template <class Tok>
  requires zc::utils::is_any_of<Tok,
                                zc::parsing::tokens::Unkown,
                                zc::parsing::tokens::Number,
                                zc::parsing::tokens::Variable,
                                zc::parsing::tokens::Function,
                                zc::parsing::tokens::Operator<'-', 2>,
                                zc::parsing::tokens::Operator<'+', 2>,
                                zc::parsing::tokens::Operator<'*', 2>,
                                zc::parsing::tokens::Operator<'/', 2>,
                                zc::parsing::tokens::Operator<'^', 2>,
                                zc::parsing::tokens::OpeningParenthesis,
                                zc::parsing::tokens::ClosingParenthesis,
                                zc::parsing::tokens::FunctionCallStart,
                                zc::parsing::tokens::FunctionCallEnd,
                                zc::parsing::tokens::FunctionArgumentSeparator,
                                zc::parsing::tokens::EndOfExpression>
std::ostream& operator<<(std::ostream& os, const Tok& token)
{
  os << boost::ut::reflection::type_name<Tok>() << " " << static_cast<const zc::parsing::tokens::Text&>(token);
  return os;
}




std::ostream& operator << (std::ostream& os, zc::deps::ObjectType type);


std::ostream& operator << (std::ostream& os, const zc::Error& err);
std::ostream& operator << (std::ostream& os, zc::Ok);


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