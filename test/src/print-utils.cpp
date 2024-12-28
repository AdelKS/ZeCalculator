#include "zecalculator/parsing/data_structures/decl/ast.h"
#include <boost/ut.hpp>
#include <zecalculator/test-utils/magic_enum.h>
#include <zecalculator/test-utils/print-utils.h>
#include <zecalculator/utils/utils.h>

namespace std {

std::ostream& operator<<(std::ostream& os, const zc::parsing::tokens::Text& txt_token)
{
  os << txt_token.substr << " at " << txt_token.begin << " ";
  return os;
}

std::ostream &operator<<(std::ostream &os, const zc::parsing::Token &token)
{
  os << magic_enum::enum_name(token.type) << " " << token.substr;
  return os;
}

template <zc::parsing::Type world_type>
void shared_node_printer(std::ostream& os,
                         const zc::parsing::shared::Node<world_type>& node,
                         size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  using zc::Function,
        zc::parsing::tokens::Text,
        zc::Sequence,
        zc::Data,
        zc::CppFunction,
        zc::parsing::shared::node::Add,
        zc::parsing::shared::node::Subtract,
        zc::parsing::shared::node::Multiply,
        zc::parsing::shared::node::Divide,
        zc::parsing::shared::node::Power,
        zc::parsing::shared::node::UnaryMinus,
        zc::parsing::shared::node::InputVariable,
        zc::parsing::shared::node::Number,
        zc::GlobalConstant;

  os << padding_str;

  std::visit(
    zc::utils::overloaded{
      [&](const Function<world_type>* f)
      {
        os << "Function<" << f->args_num() << "> ";
      },
      [&](const Sequence<world_type>*)
      {
        os << "Sequence ";
      },
      [&](const Data<world_type>*)
      {
        os << "Data ";
      },
      [&]<size_t args_num>(const CppFunction<args_num>*)
      {
        os << "CppFunction<" << args_num << "> ";
      },
      [&](Add)        { os << "+ "; },
      [&](Subtract)   { os << "- "; },
      [&](Multiply)   { os << "× "; },
      [&](Divide)     { os << "÷ "; },
      [&](Power)      { os << "^ "; },
      [&](UnaryMinus) { os << "unary - "; },
      [&](const InputVariable &v)
      {
        os << "InputVariable: index: " << v.index;
      },
      [&](const GlobalConstant *c)
      {
        os << "GlobalConstant "
          << " value: " << c->value;
      },
      [&](const Number &n)
      {
        os << "Number " << n.value;
      }},
    node);
}

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::shared::Node<zc::parsing::Type::RPN>& node)
{
  shared_node_printer(os, node);
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const zc::parsing::shared::Node<zc::parsing::Type::FAST>& node)
{
  shared_node_printer(os, node);
  return os;
}

template <zc::parsing::Type type>
void fast_printer(std::ostream& os, const zc::parsing::FAST<type>& node, size_t padding = 0)
{
  shared_node_printer(os, node.node, padding);
  if (not node.subnodes.empty())
  {
    os << "{\n";
    for (auto&& subnode: node.subnodes)
      fast_printer(os, subnode, padding + 2);
    os << std::string(padding, ' ') << "}";
  }
  os << std::endl;
}

std::ostream &operator<<(std::ostream &os,
                         const zc::parsing::FAST<zc::parsing::Type::FAST> &node)
{
  fast_printer(os, node);
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const zc::parsing::FAST<zc::parsing::Type::RPN> &node)
{
  fast_printer(os, node);
  return os;
}

void syntax_node_print_helper(std::ostream& os, const zc::parsing::AST& node, size_t padding = 0)
{
  const std::string padding_str(padding, ' ');

  return std::visit(
    zc::utils::overloaded{
      [&](const zc::parsing::AST::Func &func)
      {
        if (func.type == zc::parsing::AST::Func::FUNCTION)
          os << padding_str << "Function<" << func.subnodes.size()
              << "> ";
        else
          os << padding_str << magic_enum::enum_name(func.type);

        os << " at " << node.name.begin
            << " subexpr: " << func.full_expr << " {" << std::endl;
        for (auto &&operand : func.subnodes)
          syntax_node_print_helper(os, operand, padding + 2);
        os << padding_str << "}" << std::endl;
      },
      [&](const zc::parsing::AST::InputVariable &input_var)
      {
        os << padding_str << "InputVariable " << node.name
            << "index: " << size_t(input_var.index) << std::endl;
      },
      [&](const zc::parsing::AST::Number &)
      {
        os << padding_str << "Number " << node.name << std::endl;
      },
      [&](zc::parsing::AST::Variable)
      {
        os << padding_str << "Variable " << node.name << std::endl;
      }},
      node.dyn_data);
}

std::ostream& operator << (std::ostream& os, const zc::parsing::AST& node)
{
  os << std::endl;
  syntax_node_print_helper(os, node);
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

std::ostream& operator << (std::ostream& os, zc::deps::Dep::ObjectType type)
{
  os << magic_enum::enum_name(type);
  return os;
}

std::ostream& operator << (std::ostream& os, const zc::deps::Dep& dep)
{
  os << dep.type << " " << dep.indexes;
  return os;
}

} // namespace std
