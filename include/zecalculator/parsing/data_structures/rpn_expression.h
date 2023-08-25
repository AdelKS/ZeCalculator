#pragma once

#include <zecalculator/parsing/data_structures/token.h>
#include <zecalculator/parsing/data_structures/tree.h>
#include <vector>

namespace zc {

/// @brief represents a mathematical expression in reverse polish / postfix notation
using rpn_token = std::variant<std::monostate,
                               parsing::tokens::Function,
                               parsing::tokens::Variable,
                               parsing::tokens::Number>;
using rpn_expression = std::vector<rpn_token>;

struct RpnMaker
{
  rpn_expression operator () (std::monostate)
  {
    return rpn_expression{std::monostate()};
  }

  rpn_expression operator () (const ast::node::Function& func)
  {
    rpn_expression res;
    for (const ast::Tree& sub_node: func.subnodes)
    {
      rpn_expression tmp = std::visit(*this, sub_node);
      if (std::ranges::any_of(tmp, [](const rpn_token& token){ return std::holds_alternative<std::monostate>(token); })) [[unlikely]]
        return rpn_expression{std::monostate()};
      else [[likely]]
        std::ranges::move(tmp, std::back_inserter(res));
    }
    res.push_back(parsing::tokens::Function(func.name, substr_info(func)));
    return res;
  }

  rpn_expression operator () (const ast::node::Variable& var)
  {
    return rpn_expression(1, var);
  }

  rpn_expression operator () (const ast::node::Number& number)
  {
    return rpn_expression(1, number);
  }

};

inline rpn_expression make_rpn_expression(const ast::Tree& tree)
{
  return std::visit(RpnMaker{}, tree);
}

}
