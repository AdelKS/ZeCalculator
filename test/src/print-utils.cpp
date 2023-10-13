#include <boost/ut.hpp>
#include <zecalculator/test-utils/magic_enum.h>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/parsing/data_structures/node.h>
#include <zecalculator/utils/utils.h>

namespace zc {

namespace parsing {
namespace tokens {

std::ostream& operator<<(std::ostream& os, const tokens::Text& txt_token)
{
  os << txt_token.name << " at (" << txt_token.substr_info.begin  << ", " << txt_token.substr_info.size << ") ";
  return os;
}

} // namespace tokens

std::ostream &operator<<(std::ostream &os, const Token &token)
{
  os << static_cast<const TokenType&>(token);
  return os;
}

template <Type world_type>
void syntax_node_print_helper(std::ostream& os, const node::ast::Node<world_type>& node, size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  std::visit(overloaded{[&](std::monostate)
                        {
                          os << padding_str << "empty tree " << std::endl;
                        },
                        [&]<size_t args_num>(const node::ast::Function<world_type, args_num> &f)
                        {
                          os << padding_str << "Function<" << args_num << "> " << tokens::Text(f)
                             << " {" << std::endl;
                          for (const auto &operand : f.operands)
                            syntax_node_print_helper(os, *operand, padding + 2);
                        },
                        [&](const node::ast::Sequence<world_type> &u)
                        {
                          os << padding_str << "Sequence " << tokens::Text(u) << " {"
                             << std::endl;
                          syntax_node_print_helper(os, *u.operand, padding + 2);
                        },
                        [&]<size_t args_num>(const node::ast::CppFunction<world_type, args_num> &f)
                        {
                          os << padding_str << "CppFunction<" << args_num << "> " << tokens::Text(f) << " {"
                             << std::endl;
                          for (auto&& operand: f.operands)
                            syntax_node_print_helper(os, *operand, padding + 2);
                        },
                        [&](const node::InputVariable &v)
                        {
                          os << padding_str << "InputVariable " << tokens::Text(v) << " index: " << v.index
                             << std::endl;
                        },
                        [&](const node::GlobalConstant &c)
                        {
                          os << padding_str << "GlobalConstant " << tokens::Text(c) << " value: " << c.constant->value
                             << std::endl;
                        },
                        [&](const node::GlobalVariable<world_type> &v)
                        {
                          os << padding_str << "GlobalVariable " << tokens::Text(v)
                             << std::endl;
                        },
                        [&](const node::Number &n)
                        {
                          os << padding_str << "Number " << n
                             << std::endl;
                        }},
             node);
}

std::ostream &operator<<(std::ostream &os, const node::ast::Node<Type::AST> &node) {
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

std::ostream &operator<<(std::ostream &os, const node::ast::Node<Type::RPN> &node) {
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

} // namespace parsing

std::ostream& operator << (std::ostream& os, const Error& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.token;

  return os;
}

namespace eval {

std::ostream& operator << (std::ostream& os, const Error& err)
{
  os << magic_enum::enum_name(err.error_type)
     << " at "
     << err.token;

  return os;
}

} // namespace eval

namespace deps {
  std::ostream& operator << (std::ostream& os, ObjectType type)
  {
    os << magic_enum::enum_name(type);
    return os;
  }
} // namespace deps

} // namespace zc
