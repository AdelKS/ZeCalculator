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

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::FAST<zc::parsing::Type::FAST>& node);

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::FAST<zc::parsing::Type::RPN>& node);

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::shared::Node<zc::parsing::Type::RPN>& node);

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::shared::Node<zc::parsing::Type::FAST>& node);

std::ostream& operator << (std::ostream& os, const zc::parsing::AST& node);


std::ostream& operator<<(std::ostream &, const zc::parsing::tokens::Text&);

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
