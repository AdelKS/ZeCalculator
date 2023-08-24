#pragma once

#include <zecalculator/utils/token.h>
#include <zecalculator/utils/syntax_tree.h>
#include <vector>

namespace zc {

/// @brief represents a mathematical expression in reverse polish / postfix notation
using rpn_token = std::variant<std::monostate, tokens::Function, tokens::Variable, tokens::Number>;
using rpn_expression = std::vector<rpn_token>;

struct RpnMaker
{
  rpn_expression operator () (std::monostate)
  {
    return rpn_expression{std::monostate()};
  }

  rpn_expression operator () (const FunctionNode& func)
  {
    rpn_expression res;
    for (const SyntaxTree& sub_node: func.subnodes)
    {
      rpn_expression tmp = std::visit(*this, sub_node);
      if (std::ranges::any_of(tmp, [](const rpn_token& token){ return std::holds_alternative<std::monostate>(token); })) [[unlikely]]
        return rpn_expression{std::monostate()};
      else [[likely]]
        std::ranges::move(tmp, std::back_inserter(res));
    }
    res.push_back(tokens::Function(func.name, substr_info(func)));
    return res;
  }

  rpn_expression operator () (const VariableNode& var)
  {
    return rpn_expression(1, var);
  }

  rpn_expression operator () (const NumberNode& number)
  {
    return rpn_expression(1, number);
  }

};

inline rpn_expression make_rpn_expression(const SyntaxTree& tree)
{
  return std::visit(RpnMaker{}, tree);
}

}
