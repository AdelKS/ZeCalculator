#include <boost/ut.hpp>
#include <zecalculator/test-utils/magic_enum.h>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/utils/utils.h>

namespace std {

std::ostream& operator<<(std::ostream& os, const zc::parsing::tokens::Text& txt_token)
{
  os << txt_token.substr;
  if (txt_token.substr_info)
    os << " at (" << txt_token.substr_info->begin  << ", " << txt_token.substr_info->size << ") ";
  return os;
}

std::ostream &operator<<(std::ostream &os, const zc::parsing::Token &token)
{
  os << static_cast<const zc::parsing::TokenType&>(token);
  return os;
}

template <zc::parsing::Type world_type>
void syntax_node_print_helper(std::ostream &os,
                              const zc::parsing::fast::node::Node<world_type> &node,
                              size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  using zc::parsing::fast::node::Function,
        zc::parsing::tokens::Text,
        zc::parsing::fast::node::Sequence,
        zc::parsing::fast::node::CppFunction,
        zc::parsing::fast::node::Operator,
        zc::parsing::shared::node::InputVariable,
        zc::parsing::shared::node::Number,
        zc::parsing::shared::node::GlobalConstant;

  std::visit(
    zc::utils::overloaded{
      [&]<size_t args_num>(const zc::parsing::fast::node::Function<world_type, args_num> &f)
      {
        os << padding_str << "Function<" << args_num << "> "
          << Text(f) << " {" << std::endl;
        for (const auto &operand : f.operands)
          syntax_node_print_helper(os, *operand, padding + 2);
      },
      [&](const Sequence<world_type> &u)
      {
        os << padding_str << "Sequence " << Text(u) << " {"
          << std::endl;
        syntax_node_print_helper(os, *u.operand, padding + 2);
      },
      [&]<size_t args_num>(const CppFunction<world_type, args_num> &f)
      {
        os << padding_str << "CppFunction<" << args_num << "> "
          << Text(f) << " {" << std::endl;
        for (auto &&operand : f.operands)
          syntax_node_print_helper(os, *operand, padding + 2);
      },
      [&]<char op, size_t args_num>(const Operator<world_type, op, args_num> &f)
      {
        os << padding_str << "Operator<" << op << ", " << args_num
          << "> " << Text(f) << " {" << std::endl;
        for (auto &&operand : f.operands)
          syntax_node_print_helper(os, *operand, padding + 2);
      },
      [&](const InputVariable &v)
      {
        os << padding_str << "InputVariable " << Text(v)
          << " index: " << v.index << std::endl;
      },
      [&](const GlobalConstant<world_type> &c)
      {
        os << padding_str << "GlobalConstant " << Text(c)
          << " value: " << c.constant->value() << std::endl;
      },
      [&](const Number &n)
      {
        os << padding_str << "Number " << n << std::endl;
      }},
    node);
}

std::ostream &operator<<(std::ostream &os,
                         const zc::parsing::fast::node::Node<zc::parsing::Type::FAST> &node)
{
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const zc::parsing::fast::node::Node<zc::parsing::Type::RPN> &node)
{
  os << std::endl;
  syntax_node_print_helper(os, node);
  return os;
}

void syntax_node_print_helper(std::ostream &os,
                              const zc::parsing::ast::node::Node &node,
                              size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  using zc::parsing::ast::node::Function,
        zc::parsing::tokens::Text,
        zc::parsing::ast::node::Operator,
        zc::parsing::ast::node::Variable,
        zc::parsing::shared::node::InputVariable,
        zc::parsing::shared::node::Number,
        zc::parsing::shared::node::GlobalConstant;

  std::visit(
    zc::utils::overloaded{
      [&](const Function &f)
      {
        os << padding_str << "Function<" << f.subnodes.size() << "> "
          << f.name_token << "subexpr: " << Text(f) << " {" << std::endl;
        for (const auto &operand : f.subnodes)
          syntax_node_print_helper(os, *operand, padding + 2);
        os << padding_str << "}" << std::endl;
      },
      [&]<char op, size_t args_num>(const Operator<op, args_num> &f)
      {
        os << padding_str << "Operator<" << op << ", " << args_num
          << "> " << f.name_token << "subexpr: " << Text(f) << " {" << std::endl;
        for (auto &&operand : f.operands)
          syntax_node_print_helper(os, *operand, padding + 2);
        os << padding_str << "}" << std::endl;
      },
      [&](const InputVariable &v)
      {
        os << padding_str << "InputVariable " << Text(v)
          << " index: " << v.index << std::endl;
      },
      [&](const Variable &c)
      {
        os << padding_str << "Variable " << Text(c) << std::endl;
      },
      [&](const Number &n)
      {
        os << padding_str << "Number " << n << std::endl;
      }},
    node);
}

std::ostream& operator << (std::ostream& os, const zc::parsing::AST& node)
{
  os << std::endl;
  syntax_node_print_helper(os, *node);
  return os;
}

std::ostream& operator << (std::ostream& os, const zc::Error& err)
{
  os << magic_enum::enum_name(err.type)
     << " at "
     << err.token;

  return os;
}

std::ostream& operator << (std::ostream& os, zc::Ok)
{
  os << "Ok";
  return os;
}

std::ostream& operator << (std::ostream& os, zc::deps::ObjectType type)
{
  os << magic_enum::enum_name(type);
  return os;
}

} // namespace std