#pragma once

#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/data_structures/tree.h>
#include <vector>

namespace zc {
namespace rpn {

/// @brief represents a mathematical expression in reverse polish / postfix notation
using Token = std::variant<std::monostate,
                           parsing::tokens::Function,
                           parsing::tokens::Variable,
                           parsing::tokens::Number>;
using RPN = std::vector<Token>;

struct RpnMaker
{
  RPN operator () (std::monostate)
  {
    return RPN{std::monostate()};
  }

  RPN operator () (const ast::node::Function& func)
  {
    RPN res;
    for (const ast::Tree& sub_node: func.subnodes)
    {
      RPN tmp = std::visit(*this, sub_node);
      if (std::ranges::any_of(tmp, [](const Token& token){ return std::holds_alternative<std::monostate>(token); })) [[unlikely]]
        return RPN{std::monostate()};
      else [[likely]]
        std::ranges::move(tmp, std::back_inserter(res));
    }
    res.push_back(parsing::tokens::Function(func.name, substr_info(func)));
    return res;
  }

  RPN operator () (const ast::node::Variable& var)
  {
    return RPN(1, var);
  }

  RPN operator () (const ast::node::Number& number)
  {
    return RPN(1, number);
  }

};

inline RPN make_RPN(const ast::Tree& tree)
{
  return std::visit(RpnMaker{}, tree);
}

}
}
